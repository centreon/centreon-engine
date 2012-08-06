/*
** Copyright 2011-2012 Merethis
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <cstdlib>
#include <list>
#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/engine/commands/connector.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/version.hh"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::commands;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Constructor.
 *
 *  @param[in] connector_name  The connector name.
 *  @param[in] connector_line  The connector command line.
 *  @param[in] name            The command name.
 *  @param[in] command_line    The command line.
 *  @param[in] listener        The listener who catch events.
 */
connector::connector(
             std::string const& connector_name,
             std::string const& connector_line,
             std::string const& command_name,
             std::string const& command_line,
             command_listener* listener)
  : command(command_name, command_line, listener),
    _connector_line(connector_line),
    _connector_name(connector_name),
    _is_running(false),
    _query_quit_ok(false),
    _query_version_ok(false),
    _process(this) {
  // Disable stderr.
  _process.enable_stream(process::err, false);
}

/**
 *  Copy constructor
 *
 *  @param[in] right Object to copy.
 */
connector::connector(connector const& right)
  : command(right),
    process_listener(right) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
connector::~connector() throw() {
  // Close connector properly.
  _connector_close();
}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
connector& connector::operator=(connector const& right) {
  _internal_copy(right);
  return (*this);
}

/**
 *  Get a pointer on a copy of the same object.
 *
 *  @return Return a pointer on a copy object.
 */
com::centreon::engine::commands::command* connector::clone() const {
  return (new connector(*this));
}

/**
 *  Get the connector command line.
 *
 *  @return The connector command line.
 */
std::string const& connector::connector_line() const throw () {
  return (_connector_line);
}

/**
 *  Get the connector name.
 *
 *  @return The connector name.
 */
std::string const& connector::connector_name() const throw () {
  return (_connector_name);
}

/**
 *  Run a command.
 *
 *  @param[in] args    The command arguments.
 *  @param[in] macros  The macros data struct.
 *  @param[in] timeout The command timeout.
 *
 *  @return The command id.
 */
unsigned long connector::run(
                           std::string const& processed_cmd,
                           nagios_macros& macros,
                           unsigned int timeout) {
  (void)macros;

  unsigned long command_id(get_uniq_id());
  shared_ptr<query_info> info(new query_info);
  info->processed_cmd = processed_cmd;
  info->start_time = timestamp::now();
  info->timeout = timeout;
  info->waiting_result = false;

  {
    concurrency::locker lock(&_lock);

    // Check if the connector run and start it if necessary.
    if (!_is_running)
      _connector_start();

    // Send check to the connector.
    _send_query_execute(
      info->processed_cmd,
      command_id,
      info->start_time,
      info->timeout);
    _queries[command_id] = info;
  }

  logger(dbg_commands, basic)
    << "connector \"" << _connector_name << "\" start (id="
    << command_id << ") '" << processed_cmd << "'";

  // XXX: todo timeout command!
  return (command_id);
}

/**
 *  Run a command and wait the result.
 *
 *  @param[in]  args    The command arguments.
 *  @param[in]  macros  The macros data struct.
 *  @param[in]  timeout The command timeout.
 *  @param[out] res     The result of the command.
 */
void connector::run(
                  std::string const& processed_cmd,
                  nagios_macros& macros,
                  unsigned int timeout,
                  result& res) {
  (void)macros;

  unsigned long command_id(get_uniq_id());
  shared_ptr<query_info> info(new query_info);
  info->processed_cmd = processed_cmd;
  info->start_time = timestamp::now();
  info->timeout = timeout;
  info->waiting_result = true;

  concurrency::locker lock(&_lock);

  // Check if the connector run and start it if necessary.
  if (!_is_running)
    _connector_start();

  // Send check to the connector.
  _send_query_execute(
    info->processed_cmd,
    command_id,
    info->start_time,
    info->timeout);
  _queries[command_id] = info;

  logger(dbg_commands, basic)
    << "connector \"" << _connector_name << "\" start (id="
    << command_id << ") '" << processed_cmd << "'";

  // XXX: todo timeout command!

  // Waiting result.
  while (true) {
    _cv_query.wait(&_lock);
    umap<unsigned long, result>::iterator
      it(_results.find(command_id));
    if (it != _results.end()) {
      res = it->second;
      _results.erase(it);
      break;
    }
  }
  return;
}

/**
 *  Provide by process_listener interface to get data on stdout.
 *
 *  @param[in] p  The process to get data on stdout.
 */
void connector::data_is_available(process& p) throw () {
  typedef void (connector::*recv_query)(char const*);
  static recv_query tab_recv_query[] = {
    NULL,
    &connector::_recv_query_version,
    NULL,
    &connector::_recv_query_execute,
    NULL,
    &connector::_recv_query_quit,
    &connector::_recv_query_error,
    NULL
  };

  try {
    // Read process output.
    std::string data;
    p.read(data);

    concurrency::locker lock(&_lock);

    // Split outpout into queries responses.
    std::list<std::string> responses;
    {
      std::string const& ending(_query_ending());
      _data_available.append(data);
      while (_data_available.size() > 0) {
        size_t pos(_data_available.find(ending));
        if (pos == std::string::npos)
          break;
        responses.push_back(_data_available.substr(0, pos));
        _data_available.erase(0, pos + ending.size());
      }
    }

    // Parse queries responses.
    for (std::list<std::string>::const_iterator
           it(responses.begin()), end(responses.end());
         it != end;
         ++it) {
      char const* data(it->c_str());
      char* endptr(NULL);
      unsigned int id(strtol(data, &endptr, 10));
      // Invalid query.
      if (data == endptr
          || id >= sizeof(tab_recv_query) / sizeof(*tab_recv_query)
          || !tab_recv_query[id])
        logger(log_runtime_warning, basic)
          << "connector \"" << _connector_name << "\" bad request id"
          ": id=" << id;
      // Valid query, so execute it.
      else
        (this->*tab_recv_query[id])(endptr + 1);
    }
  }
  catch (std::exception const& e) {
    logger(log_runtime_warning, basic)
      << "connector \"" << _connector_name << "\" error: " << e.what();
  }
  return;
}

/**
 *  Provide by process_listener interface but not used.
 *
 *  @param[in] p  Unused param.
 */
void connector::data_is_available_err(process& p) throw () {
  (void)p;
  return;
}

/**
 *  Provide by process_listener interface. Call at the end
 *  of the process execution.
 *
 *  @param[in] p  The process to finished.
 */
void connector::finished(process& p) throw () {
  (void)p;

  concurrency::locker lock(&_lock);
  _is_running = false;
  _data_available.clear();
  return;
}

/**
 *  Close connection with the process.
 */
void connector::_connector_close() {
  {
    concurrency::locker lock(&_lock);

    // Reset variables.
    _query_quit_ok = false;

    // Ask connector to quit properly.
    _send_query_quit();

    // Waiting connector quit, or 1 seconds.
    if (!_cv_query.wait(&_lock, 1000)) {
      _process.kill();
      logger(log_runtime_warning, basic)
        << "connector \"" << _connector_name << "\" "
        "query quit failed: timeout";
    }
    else if (!_query_quit_ok) {
      _process.kill();
      logger(log_runtime_warning, basic)
        << "connector \"" << _connector_name << "\" "
        "query quit failed: invalid response";
    }
  }
  _process.wait();
  return;
}

/**
 *  Start connection with the process.
 */
void connector::_connector_start() {
  concurrency::locker lock(&_lock);

  // Reset variables.
  _query_quit_ok = false;
  _query_version_ok = false;
  _is_running = false;

  // Start connector execution.
  _process.exec(_connector_line);

  // Ask connector version.
  _send_query_version();

  // Waiting connector version, or 1 seconds.
  if (!_cv_query.wait(&_lock, 1000)) {
    _process.kill();
    throw (engine_error() << "query version failed: timeout");
  }
  if (!_query_version_ok) {
    _process.kill();
    throw (engine_error() << "query version failed: invalid version");
  }
  _is_running = true;

  logger(log_info_message, basic)
    << "connector \"" << _connector_name << "\" start";

  // Resend commands.
  for (umap<unsigned long, shared_ptr<query_info> >::iterator
         it(_queries.begin()), end(_queries.end());
       it != end;
       ++it) {
    unsigned long command_id(it->first);
    shared_ptr<query_info> info(it->second);
    _send_query_execute(
      info->processed_cmd,
      command_id,
      info->start_time,
      info->timeout);
  }
  return;
}

/**
 *  Internal copy.
 *
 *  @param[in] right  The object to copy.
 */
void connector::_internal_copy(connector const& right) {
  if (this != &right) {
    _connector_line = right._connector_line;
    _connector_name = right._connector_name;
    _data_available.clear();
    _is_running = false;
    _queries.clear();
    _query_quit_ok = false;
    _query_version_ok = false;
    _results.clear();
  }
  return;
}

/**
 *  Get the ending string for connector protocole.
 *
 *  @return The ending string.
 */
std::string const& connector::_query_ending() const throw () {
  static std::string ending(3, '\0');
  return (ending);
}

/**
 *  Receive an error from the connector.
 */
void connector::_recv_query_error(char const* data) {
  try {
    char* endptr(NULL);
    int code(strtol(data, &endptr, 10));
    if (data == endptr)
      throw (engine_error() << "invalid query version: "
             "invalid number of parameters");
    char const* message(endptr + 1);

    switch (code) {
      // Information message.
    case 0:
      logger(log_info_message, basic)
        << "connector \"" << _connector_name << "\": " << message;
      break;

      // Warning message.
    case 1:
      logger(log_runtime_warning, basic)
        << "connector \"" << _connector_name << "\": " << message;
      break;

      // Error message.
    case 2:
      logger(log_runtime_error, basic)
        << "connector \"" << _connector_name << "\": " << message;
      break;
    }
  }
  catch (std::exception const& e) {
    logger(log_runtime_warning, basic)
      << "connector \"" << _connector_name << "\": " << e.what();
  }
  return;
}

/**
 *  Receive response to the query execute.
 */
void connector::_recv_query_execute(char const* data) {
  try {
    // Get query informations.
    char* endptr(NULL);
    unsigned long command_id(strtol(data, &endptr, 10));
    if (data == endptr)
      throw (engine_error() << "invalid query execute: "
             "invalid command_id");
    data = endptr + 1;
    bool is_executed(strtol(data, &endptr, 10));
    if (data == endptr)
      throw (engine_error() << "invalid query execute: "
             "invalid is_executed");
    data = endptr + 1;
    int exit_code(strtol(data, &endptr, 10));
    if (data == endptr)
      throw (engine_error() << "invalid query execute: "
             "invalid exit_code");
    char const* stderr(endptr + 1);
    char const* stdout(stderr + strlen(stderr) + 1);

    // Get query information with the command_id.
    umap<unsigned long, shared_ptr<query_info> >::iterator
      it(_queries.find(command_id));
    if (it == _queries.end()) {
      logger(dbg_commands, basic)
        << "recv query failed: command_id(" << command_id << ") "
        "not found into queries";
      return;
    }
    // Get data.
    shared_ptr<query_info> info(it->second);
    // Remove query from queries.
    _queries.erase(it);

    // Initialize result.
    result res;
    res.command_id = command_id;
    res.end_time = timestamp::now();
    res.exit_code = STATE_CRITICAL;
    res.exit_status = process::normal;
    res.start_time = info->start_time;

    // Check if the check timeout.
    if (info->timeout > 0
        && (res.end_time - res.start_time).to_seconds() > info->timeout) {
      res.exit_status = process::timeout;
      res.output = "(Process Timeout)";
    }
    // If the check was not executed correctly.
    else if (!is_executed) {
      res.exit_status = process::crash;
      res.output = stderr;
    }
    // If the check was executed correctly.
    else {
      if (exit_code < -1 || exit_code > 3)
        res.exit_code = STATE_UNKNOWN;
      else
        res.exit_code = exit_code;
      res.output = stdout;
    }

    logger(dbg_commands, basic)
      << "connector \"" << _connector_name << "\" finished: "
      "id=" << command_id;

    if (!info->waiting_result) {
      // Forward result to the listener.
      if (_listener)
        (_listener->finished)(res);
    }
    else {
      // Push result into list of results.
      _results[command_id] = res;
      _cv_query.wake_all();
    }
  }
  catch (std::exception const& e) {
    logger(log_runtime_warning, basic)
      << "connector \"" << _connector_name << "\": " << e.what();
  }
  return;
}

/**
 *  Receive response to the query quit.
 *
 *  @param[in] data  Unused param.
 */
void connector::_recv_query_quit(char const* data) {
  (void)data;
  _query_quit_ok = true;
  _cv_query.wake_all();
  return;
}

/**
 *  Receive response to the query version.
 *
 *  @param[in] data  Has version of engine to use with the connector.
 */
void connector::_recv_query_version(char const* data) {
  _query_version_ok = false;
  try {
    int version[2];
    char* endptr(NULL);
    for (unsigned int i(0); i < 2; ++i) {
      version[i] = strtol(data, &endptr, 10);
      if (data == endptr)
        throw (engine_error() << "invalid query version: "
               "invalid number of parameters");
      data = endptr + 1;
    }

    if (version[0] < CENTREON_ENGINE_VERSION_MAJOR
        || (version[0] == CENTREON_ENGINE_VERSION_MAJOR
            && version[1] <= CENTREON_ENGINE_VERSION_MINOR))
      _query_version_ok = true;
  }
  catch (std::exception const& e) {
    logger(log_runtime_warning, basic)
      << "connector \"" << _connector_name << "\": " << e.what();
  }
  _cv_query.wake_all();
  return;
}

/**
 *  Send query execute. To ask connector to execute.
 *
 *  @param[in]  cmdline     The command to execute.
 *  @param[in]  command_id  The command id.
 *  @param[in]  start       The start time.
 *  @param[in]  timeout     The timeout.
 */
void connector::_send_query_execute(
                  std::string const& cmdline,
                  unsigned int command_id,
                  timestamp const& start,
                  unsigned int timeout) {
  std::ostringstream oss;
  oss << "2" << '\0'
      << command_id << '\0'
      << timeout << '\0'
      << start.to_seconds() << '\0'
      << cmdline << '\0'
      << _query_ending();
  _process.write(oss.str());
}

/**
 *  Send query quit. To ask connector to quit properly.
 */
void connector::_send_query_quit() {
  std::string query("4\0", 2);
  _process.write(query + _query_ending());
}

/**
 *  Send query verion. To ask connector version.
 */
void connector::_send_query_version() {
  std::string query("0\0", 2);
  _process.write(query + _query_ending());
}


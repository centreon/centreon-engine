/*
 * Copyright 2011-2013,2015,2019 Centreon (https://www.centreon.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information : contact@centreon.com
 *
 */
#include "com/centreon/engine/commands/connector.hh"
#include <unistd.h>
#include <cstdlib>

#include <list>
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/my_lock.hh"
#include "com/centreon/engine/version.hh"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::commands;

//#define DEBUG_CONFIG

#ifdef DEBUG_CONFIG
#define LOCK_GUARD(lck, m) \
  my_lock_guard<std::mutex> lck(m, __FILE__ ":" #m, __LINE__)
#define UNIQUE_LOCK(lck, m) \
  my_unique_lock<std::mutex> lck(m, __FILE__ ":" #m, __LINE__)
#define UNLOCK(lck) lck.unlock(__LINE__)
#define LOCK(lck) lck.lock(__LINE__)
#else
#define LOCK_GUARD(lck, m) std::lock_guard<std::mutex> lck(m)
#define UNIQUE_LOCK(lck, m) std::unique_lock<std::mutex> lck(m)
#define UNLOCK(lck) lck.unlock()
#define LOCK(lck) lck.lock()
#endif

connector_map connector::connectors;

/**
 *  Constructor.
 *
 *  @param[in] connector_name  The connector name.
 *  @param[in] connector_line  The connector command line.
 *  @param[in] listener        The listener who catch events.
 */
connector::connector(const std::string& connector_name,
                     const std::string& connector_line,
                     command_listener* listener)
    : command(connector_name, connector_line, listener),
      process_listener(),
      _is_running(false),
      _query_quit_ok(false),
      _query_version_ok(false),
      _process(this, true, true, false),  // Disable stderr.
      _try_to_restart(true),
      _thread_running(false),
      _thread_action(none) {
  // Set use setpgid.
  {
    UNIQUE_LOCK(lck, _thread_m);
    _restart = std::thread(&connector::_restart_loop, this),
    _thread_cv.wait(lck, [this] { return _thread_running; });
  }
  {
    UNIQUE_LOCK(lck, _lock);
    _process.setpgid_on_exec(config->use_setpgid());
  }
  if (config->enable_environment_macros())
    logger(log_runtime_warning, basic)
        << "Warning: Connector does not enable environment macros";
}

/**
 *  Destructor.
 */
connector::~connector() noexcept {
  // Close connector properly.
  try {
    _connector_close();
  } catch (const std::exception& e) {
    logger(log_runtime_error, basic)
        << "Error: could not stop connector properly: " << e.what();
  }

  // Wait restart thread.
  {
    UNIQUE_LOCK(lck, _thread_m);
    _thread_action = stop;
    _thread_cv.notify_all();
    UNLOCK(lck);
    _restart.join();
  }
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
uint64_t connector::run(const std::string& processed_cmd,
                        nagios_macros& macros,
                        uint32_t timeout) {
  (void)macros;

  logger(dbg_commands, basic)
      << "connector::run: connector='" << _name << "', cmd='" << processed_cmd
      << "', timeout=" << timeout;

  // Set query informations.
  uint64_t command_id(get_uniq_id());
  std::shared_ptr<query_info> info(new query_info);
  info->processed_cmd = processed_cmd;
  info->start_time = timestamp::now();
  info->timeout = timeout;
  info->waiting_result = false;

  logger(dbg_commands, basic) << "connector::run: id=" << command_id;

  try {
    {
      UNIQUE_LOCK(lock, _lock);

      // Start connector if is not running.
      if (!_is_running) {
        if (!_try_to_restart)
          throw engine_error()
              << "Connector '" << _name << "' failed to restart";
        _queries[command_id] = info;
        UNLOCK(lock);
        _connector_start();
        LOCK(lock);
      }

      // Send check to the connector.
      _send_query_execute(info->processed_cmd, command_id, info->start_time,
                          info->timeout);
      _queries[command_id] = info;
    }

    logger(dbg_commands, basic)
        << "connector::run: start command success: id=" << command_id;
  } catch (...) {
    logger(dbg_commands, basic)
        << "connector::run: start command failed: id=" << command_id;
    throw;
  }
  return command_id;
}

/**
 *  Run a command and wait the result.
 *
 *  @param[in]  args    The command arguments.
 *  @param[in]  macros  The macros data struct.
 *  @param[in]  timeout The command timeout.
 *  @param[out] res     The result of the command.
 */
void connector::run(const std::string& processed_cmd,
                    nagios_macros& macros,
                    uint32_t timeout,
                    result& res) {
  (void)macros;

  logger(dbg_commands, basic)
      << "connector::run: connector='" << _name << "', cmd='" << processed_cmd
      << "', timeout=" << timeout;

  // Set query informations.
  uint64_t command_id(get_uniq_id());
  std::shared_ptr<query_info> info(new query_info);
  info->processed_cmd = processed_cmd;
  info->start_time = timestamp::now();
  info->timeout = timeout;
  info->waiting_result = true;

  logger(dbg_commands, basic) << "connector::run: id=" << command_id;

  try {
    {
      UNIQUE_LOCK(lock, _lock);

      // Start connector if is not running.
      if (!_is_running) {
        if (!_try_to_restart)
          throw engine_error()
              << "Connector '" << _name << "' failed to restart";
        UNLOCK(lock);
        _connector_start();
        LOCK(lock);
      }

      // Send check to the connector.
      _send_query_execute(info->processed_cmd, command_id, info->start_time,
                          info->timeout);
      _queries[command_id] = info;
    }

    logger(dbg_commands, basic)
        << "connector::run: start command success: id=" << command_id;
  } catch (...) {
    logger(dbg_commands, basic)
        << "connector::run: start command failed: id=" << command_id;
    throw;
  }

  // Waiting result.
  UNIQUE_LOCK(lock, _lock);
  for (;;) {
    auto it = _results.find(command_id);
    if (it != _results.end()) {
      res = it->second;
      _results.erase(it);
      break;
    }
    _cv_query.wait(lock);
  }
}

/**
 *  Set connector command line.
 *
 *  @param[in] command_line The new command line.
 */
void connector::set_command_line(const std::string& command_line) {
  // Close connector properly.
  _connector_close();
  _try_to_restart = true;

  // Change command line.
  {
    LOCK_GUARD(lck, _lock);
    command::set_command_line(command_line);
  }
}

/**
 *  Provide by process_listener interface to get data on stdout.
 *
 *  @param[in] p  The process to get data on stdout.
 */
void connector::data_is_available(process& p) noexcept {
  typedef void (connector::*recv_query)(char const*);
  static recv_query tab_recv_query[] = {nullptr,
                                        &connector::_recv_query_version,
                                        nullptr,
                                        &connector::_recv_query_execute,
                                        nullptr,
                                        &connector::_recv_query_quit,
                                        &connector::_recv_query_error,
                                        nullptr};

  try {
    logger(dbg_commands, basic)
        << "connector::data_is_available: process=" << (void*)&p;

    // Read process output.
    std::string data;
    p.read(data);

    // Split output into queries responses.
    std::list<std::string> responses;
    {
      std::string ending(_query_ending());
      ending.append("\0", 1);

      {
        LOCK_GUARD(lock, _lock);
        _data_available.append(data);
        while (_data_available.size() > 0) {
          size_t pos(_data_available.find(ending));
          if (pos == std::string::npos)
            break;
          responses.push_back(_data_available.substr(0, pos));
          _data_available.erase(0, pos + ending.size());
        }
      }

      logger(dbg_commands, basic)
          << "connector::data_is_available: responses.size="
          << responses.size();
    }

    // Parse queries responses.
    for (std::list<std::string>::const_iterator it(responses.begin()),
         end(responses.end());
         it != end; ++it) {
      char const* data(it->c_str());
      char* endptr(nullptr);
      uint32_t id(strtol(data, &endptr, 10));
      logger(dbg_commands, basic)
          << "connector::data_is_available: request id=" << id;
      // Invalid query.
      if (data == endptr ||
          id >= sizeof(tab_recv_query) / sizeof(*tab_recv_query) ||
          !tab_recv_query[id])
        logger(log_runtime_warning, basic) << "Warning: Connector '" << _name
                                           << "' "
                                              "received bad request ID: "
                                           << id;
      // Valid query, so execute it.
      else
        (this->*tab_recv_query[id])(endptr + 1);
    }
  } catch (std::exception const& e) {
    logger(log_runtime_warning, basic)
        << "Warning: Connector '" << _name << "' error: " << e.what();
  }
}

/**
 *  Provide by process_listener interface but not used.
 *
 *  @param[in] p  Unused param.
 */
void connector::data_is_available_err(process& p) noexcept {
  (void)p;
}

/**
 *  Provide by process_listener interface. Call at the end
 *  of the process execution.
 *
 *  @param[in] p  The process to finished.
 */
void connector::finished(process& p) noexcept {
  try {
    logger(dbg_commands, basic) << "connector::finished: process=" << &p;

    UNIQUE_LOCK(lock, _lock);
    _is_running = false;
    _data_available.clear();

    // The connector is stop, restart it if necessary.
    if (_try_to_restart && !sigshutdown) {
      restart_connector();
    }
    // Connector probably quit without sending exit return.
    else {
      _cv_query.notify_all();
      UNLOCK(lock);
    }

  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
        << "Error: Connector '" << _name
        << "' termination routine failed: " << e.what();
  }
}

/**
 *  Close connection with the process.
 */
void connector::_connector_close() {
  UNIQUE_LOCK(lock, _lock);

  // Exit if connector is not running.
  if (!_is_running)
    return;

  logger(dbg_commands, basic)
      << "connector::_connector_close: process=" << &_process;

  // Set variable to dosn't restart connector.
  {
    LOCK_GUARD(lck, _thread_m);
    _try_to_restart = false;
    _thread_cv.notify_all();
  }

  // Reset variables.
  _query_quit_ok = false;

  // Ask connector to quit properly.
  _send_query_quit();

  // Waiting connector quit.
  bool is_timeout{
      _cv_query.wait_for(
          lock, std::chrono::seconds(config->service_check_timeout())) ==
      std::cv_status::timeout};
  if (is_timeout || !_query_quit_ok) {
    _process.kill();
    if (is_timeout)
      logger(log_runtime_warning, basic)
          << "Warning: Cannot close connector '" << _name << "': Timeout";
  }
  UNLOCK(lock);

  // Waiting the end of the process.
  _process.wait();
}

/**
 *  Start connection with the process.
 */
void connector::_connector_start() {
  logger(dbg_commands, basic)
      << "connector::_connector_start: process=" << &_process;

  {
    LOCK_GUARD(lock, _lock);

    // Reset variables.
    _query_quit_ok = false;
    _query_version_ok = false;
    _is_running = false;
  }

  // Start connector execution.
  _process.exec(_command_line);

  {
    UNIQUE_LOCK(lock, _lock);

    // Ask connector version.
    _send_query_version();

    // Waiting connector version, or 1 seconds.
    bool is_timeout{
        _cv_query.wait_for(
            lock, std::chrono::seconds(config->service_check_timeout())) ==
        std::cv_status::timeout};
    if (is_timeout || !_query_version_ok) {
      _process.kill();
      _try_to_restart = false;
      _thread_cv.notify_all();

      if (is_timeout)
        throw engine_error()
            << "Cannot start connector '" << _name << "': Timeout";
      throw engine_error() << "Cannot start connector '" << _name
                           << "': Bad protocol version";
    }
    _is_running = true;
  }

  logger(log_info_message, basic) << "Connector '" << _name << "' has started";

  {
    LOCK_GUARD(lock, _lock);
    logger(dbg_commands, basic)
        << "connector::_connector_start: resend queries: queries.size="
        << _queries.size();

    // Resend commands.
    for (std::unordered_map<uint64_t, std::shared_ptr<query_info> >::iterator
             it(_queries.begin()),
         end(_queries.end());
         it != end; ++it) {
      uint64_t command_id(it->first);
      std::shared_ptr<query_info> info(it->second);
      _send_query_execute(info->processed_cmd, command_id, info->start_time,
                          info->timeout);
    }
  }
}

/**
 *  Get the ending string for connector protocole.
 *
 *  @return The ending string.
 */
const std::string& connector::_query_ending() const noexcept {
  const static std::string ending(3, '\0');
  return ending;
}

/**
 *  Receive an error from the connector.
 *
 *  @param[in] data  The query to parse.
 */
void connector::_recv_query_error(char const* data) {
  try {
    logger(dbg_commands, basic) << "connector::_recv_query_error";

    char* endptr(nullptr);
    int code(strtol(data, &endptr, 10));
    if (data == endptr)
      throw engine_error() << "Invalid query for connector '" << _name
                           << "': Bad number of arguments";
    char const* message(endptr + 1);

    switch (code) {
        // Information message.
      case 0:
        logger(log_info_message, basic)
            << "Info: Connector '" << _name << "': " << message;
        break;
        // Warning message.
      case 1:
        logger(log_runtime_warning, basic)
            << "Warning: Connector '" << _name << "': " << message;
        break;
        // Error message.
      case 2:
        logger(log_runtime_error, basic)
            << "Error: Connector '" << _name << "': " << message;
        break;
    }
  } catch (std::exception const& e) {
    logger(log_runtime_warning, basic)
        << "Warning: Connector '" << _name << "': " << e.what();
  }
}

/**
 *  Receive response to the query execute.
 *
 *  @param[in] data  The query to parse.
 */
void connector::_recv_query_execute(char const* data) {
  try {
    logger(dbg_commands, basic) << "connector::_recv_query_execute";

    // Get query informations.
    char* endptr(nullptr);
    uint64_t command_id(strtol(data, &endptr, 10));
    if (data == endptr)
      throw engine_error() << "Invalid execution result: Invalid command ID";
    data = endptr + 1;
    bool is_executed(strtol(data, &endptr, 10));
    if (data == endptr)
      throw engine_error() << "Invalid execution result: Invalid executed flag";
    data = endptr + 1;
    int exit_code(strtol(data, &endptr, 10));
    if (data == endptr)
      throw engine_error() << "Invalid execution result: Invalid exit code";
    char const* std_err(endptr + 1);
    char const* std_out(std_err + strlen(std_err) + 1);

    logger(dbg_commands, basic)
        << "connector::_recv_query_execute: id=" << command_id;

    std::shared_ptr<query_info> info;
    {
      LOCK_GUARD(lock, _lock);

      // Get query information with the command_id.
      std::unordered_map<uint64_t, std::shared_ptr<query_info> >::iterator it(
          _queries.find(command_id));
      if (it == _queries.end()) {
        logger(dbg_commands, basic)
            << "recv query failed: command_id(" << command_id
            << ") "
               "not found into queries";
        return;
      }
      // Get data.
      info = it->second;
      // Remove query from queries.
      _queries.erase(it);
    }

    // Initialize result.
    result res;
    res.command_id = command_id;
    res.end_time = timestamp::now();
    res.exit_code = service::state_unknown;
    res.exit_status = process::normal;
    res.start_time = info->start_time;

    uint32_t execution_time((res.end_time - res.start_time).to_mseconds());

    // Check if the check timeout.
    if (info->timeout > 0 && execution_time > info->timeout * 1000) {
      res.exit_status = process::timeout;
      res.output = "(Process Timeout)";
    }
    // The check result was properly returned.
    else {
      if (exit_code < 0 || exit_code > 3)
        res.exit_code = service::state_unknown;
      else
        res.exit_code = exit_code;
      res.output = (is_executed ? std_out : std_err);
    }

    logger(dbg_commands, basic) << "connector::_recv_query_execute: "
                                   "id="
                                << command_id
                                << ", "
                                   "start_time="
                                << res.start_time.to_mseconds()
                                << ", "
                                   "end_time="
                                << res.end_time.to_mseconds()
                                << ", "
                                   "exit_code="
                                << res.exit_code
                                << ", "
                                   "exit_status="
                                << res.exit_status
                                << ", "
                                   "output='"
                                << res.output << "'";

    if (!info->waiting_result) {
      // Forward result to the listener.
      if (_listener)
        (_listener->finished)(res);
    } else {
      LOCK_GUARD(lock, _lock);
      // Push result into list of results.
      _results[command_id] = res;
      _cv_query.notify_all();
    }
  } catch (std::exception const& e) {
    logger(log_runtime_warning, basic)
        << "Warning: Connector '" << _name << "': " << e.what();
  }
}

/**
 *  Receive response to the query quit.
 *
 *  @param[in] data  Unused param.
 */
void connector::_recv_query_quit(char const* data) {
  (void)data;
  logger(dbg_commands, basic) << "connector::_recv_query_quit";

  LOCK_GUARD(lock, _lock);
  _query_quit_ok = true;
  _cv_query.notify_all();
}

/**
 *  Receive response to the query version.
 *
 *  @param[in] data  Has version of engine to use with the connector.
 */
void connector::_recv_query_version(char const* data) {
  logger(dbg_commands, basic) << "connector::_recv_query_version";

  bool version_ok(false);
  try {
    // Parse query version response to get major and minor
    // engine version supported by the connector.
    int version[2];
    char* endptr(nullptr);
    for (uint32_t i(0); i < 2; ++i) {
      version[i] = strtol(data, &endptr, 10);
      if (data == endptr)
        throw engine_error() << "Invalid version query: Bad format";
      data = endptr + 1;
    }

    logger(dbg_commands, basic) << "connector::_recv_query_version: "
                                   "major="
                                << version[0] << ", minor=" << version[1];

    // Check the version.
    if (version[0] < CENTREON_ENGINE_VERSION_MAJOR ||
        (version[0] == CENTREON_ENGINE_VERSION_MAJOR &&
         version[1] <= CENTREON_ENGINE_VERSION_MINOR))
      version_ok = true;
  } catch (std::exception const& e) {
    logger(log_runtime_warning, basic)
        << "Warning: Connector '" << _name << "': " << e.what();
  }

  LOCK_GUARD(lock, _lock);
  _query_version_ok = version_ok;
  _cv_query.notify_all();
}

/**
 *  Send query execute. To ask connector to execute.
 *
 *  @param[in]  cmdline     The command to execute.
 *  @param[in]  command_id  The command id.
 *  @param[in]  start       The start time.
 *  @param[in]  timeout     The timeout.
 */
void connector::_send_query_execute(const std::string& cmdline,
                                    uint64_t command_id,
                                    timestamp const& start,
                                    uint32_t timeout) {
  logger(dbg_commands, basic) << "connector::_send_query_execute: "
                                 "id="
                              << command_id
                              << ", "
                                 "cmd='"
                              << cmdline
                              << "', "
                                 "start="
                              << start.to_seconds()
                              << ", "
                                 "timeout="
                              << timeout;

  std::ostringstream oss;
  oss << "2" << '\0' << command_id << '\0' << timeout << '\0'
      << start.to_seconds() << '\0' << cmdline << '\0' << _query_ending();

  _process.write(oss.str());
}

/**
 *  Send query quit. To ask connector to quit properly.
 */
void connector::_send_query_quit() {
  logger(dbg_commands, basic) << "connector::_send_query_quit";

  std::string query("4\0", 2);
  _process.write(query + _query_ending());
}

/**
 *  Send query verion. To ask connector version.
 */
void connector::_send_query_version() {
  logger(dbg_commands, basic) << "connector::_send_query_version";

  std::string query("0\0", 2);
  query.append(_query_ending());
  _process.write(query);
}

/**
 * @brief This function is useful to restart the connector. This is the first
 * step to then execute a check.
 */
void connector::restart_connector() {
  LOCK_GUARD(lck, _thread_m);
  _thread_action = start;
  _thread_cv.notify_all();
}

/**
 * @brief The restart loop used to restart in background the connector.
 */
void connector::_restart_loop() {
  UNIQUE_LOCK(lck, _thread_m);
  _thread_running = true;
  _thread_cv.notify_all();
  for (;;) {
    _thread_cv.wait(lck, [this] { return _thread_action != none; });
    UNLOCK(lck);

    if (_thread_action == stop) {
      _try_to_restart = false;
      return;
    }

    _thread_action = none;
    _run_restart();
    LOCK(lck);
  }
}

/**
 *  Execute restart.
 */
void connector::_run_restart() {
  try {
    _connector_start();
  } catch (std::exception const& e) {
    logger(log_runtime_warning, basic)
        << "Warning: Connector '" << _name << "': " << e.what();

    std::unordered_map<uint64_t, std::shared_ptr<query_info> > tmp_queries;
    {
      LOCK_GUARD(lck, _lock);
      _try_to_restart = false;
      _thread_cv.notify_all();
      std::swap(tmp_queries, _queries);
    }

    // Resend commands.
    for (std::unordered_map<uint64_t, std::shared_ptr<query_info> >::iterator
             it(tmp_queries.begin()),
         end(tmp_queries.end());
         it != end; ++it) {
      uint64_t command_id(it->first);
      std::shared_ptr<query_info> info(it->second);

      result res;
      res.command_id = command_id;
      res.end_time = timestamp::now();
      res.exit_code = service::state_unknown;
      res.exit_status = process::normal;
      res.start_time = info->start_time;
      res.output = "(Failed to execute command with connector '" + _name + "')";

      logger(dbg_commands, basic) << "connector::_recv_query_execute: "
                                     "id="
                                  << command_id
                                  << ", "
                                     "start_time="
                                  << res.start_time.to_mseconds()
                                  << ", "
                                     "end_time="
                                  << res.end_time.to_mseconds()
                                  << ", "
                                     "exit_code="
                                  << res.exit_code
                                  << ", "
                                     "exit_status="
                                  << res.exit_status
                                  << ", "
                                     "output='"
                                  << res.output << "'";

      if (!info->waiting_result) {
        // Forward result to the listener.
        if (_listener)
          (_listener->finished)(res);
      } else {
        LOCK_GUARD(lock, _lock);
        // Push result into list of results.
        _results[command_id] = res;
        _cv_query.notify_all();
      }
    }
  }
}

/**
 *  Dump connector content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The connector to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, connector const& obj) {
  os << "connector {\n"
        "  name:         "
     << obj.get_name()
     << "\n"
        "  command_line: "
     << obj.get_command_line()
     << "\n"
        "}\n";
  return os;
}

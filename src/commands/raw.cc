/*
** Copyright 2011-2015 Merethis
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

#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
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
 *  @param[in] name         The command name.
 *  @param[in] command_line The command line.
 *  @param[in] listener     The listener who catch events.
 */
raw::raw(
       std::string const& name,
       std::string const& command_line,
       command_listener* listener)
  : command(name, command_line, listener), process_listener() {}

/**
 *  Copy constructor
 *
 *  @param[in] right Object to copy.
 */
raw::raw(raw const& right) : command(right), process_listener(right) {}

/**
 *  Destructor.
 */
raw::~raw() throw () {
  try {
    concurrency::locker lock(&_lock);
    while (!_processes_busy.empty()) {
      process* p(_processes_busy.begin()->first);
      lock.unlock();
      p->wait();
      lock.relock();
    }
    for (std::list<process*>::const_iterator
           it(_processes_free.begin()), end(_processes_free.end());
         it != end;
         ++it)
      delete *it;
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: Raw command destructor failed: " << e.what();
  }
}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
raw& raw::operator=(raw const& right) {
  if (this != &right)
    command::operator=(right);
  return (*this);
}

/**
 *  Get a pointer on a copy of the same object.
 *
 *  @return Return a pointer on a copy object.
 */
commands::command* raw::clone() const {
  return (new raw(*this));
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
unsigned long raw::run(
                     std::string const& processed_cmd,
                     nagios_macros& macros,
                     unsigned int timeout) {
  logger(dbg_commands, basic)
    << "raw::run: cmd='" << processed_cmd << "', timeout=" << timeout;

  // Get process and put into the busy list.
  process* p(NULL);
  unsigned long command_id(get_uniq_id());
  {
    concurrency::locker lock(&_lock);
    p = _get_free_process();
    _processes_busy[p] = command_id;
  }

  logger(dbg_commands, basic)
    << "raw::run: id=" << command_id << ", process=" << p;

  try {
    // Start process.
    p->exec(processed_cmd.c_str(), NULL, timeout);
    logger(dbg_commands, basic)
      << "raw::run: start process success: id=" << command_id;
  }
  catch (...) {
    logger(dbg_commands, basic)
      << "raw::run: start process failed: id=" << command_id;

    concurrency::locker lock(&_lock);
    _processes_busy.erase(p);
    throw;
  }
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
void raw::run(
            std::string const& processed_cmd,
            nagios_macros& macros,
            unsigned int timeout,
            result& res) {
  logger(dbg_commands, basic)
    << "raw::run: cmd='" << processed_cmd << "', timeout=" << timeout;

  // Get process.
  process p;
  unsigned long command_id(get_uniq_id());

  logger(dbg_commands, basic)
    << "raw::run: id=" << command_id << ", process=" << &p;

  // Start process.
  try {
    p.exec(processed_cmd.c_str(), NULL, timeout);
    logger(dbg_commands, basic)
      << "raw::run: start process success: id=" << command_id;
  }
  catch (...) {
    logger(dbg_commands, basic)
      << "raw::run: start process failed: id=" << command_id;
    throw;
  }

  // Wait for completion.
  p.wait();

  // Get process output.
  p.read(res.output);

  // Set result informations.
  res.command_id = command_id;
  res.start_time = p.start_time();
  res.end_time = p.end_time();
  res.exit_code = p.exit_code();
  res.exit_status = p.exit_status();

  if (res.exit_status == process::timeout) {
    res.exit_code = STATE_UNKNOWN;
    res.output = "(Process Timeout)";
  }
  else if ((res.exit_status == process::crash)
           || (res.exit_code < -1)
           || (res.exit_code > 3))
    res.exit_code = STATE_UNKNOWN;

  logger(dbg_commands, basic)
    << "raw::run: end process: "
    "id=" << command_id << ", "
    "start_time=" << res.start_time.to_mseconds() << ", "
    "end_time=" << res.end_time.to_mseconds() << ", "
    "exit_code=" << res.exit_code << ", "
    "exit_status=" << res.exit_status << ", "
    "output='" << res.output << "'";
  return;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Provide by process_listener interface but not used.
 *
 *  @param[in] p  Unused.
 */
void raw::data_is_available(process& p) throw () {
  (void)p;
  return;
}

/**
 *  Provide by process_listener interface but not used.
 *
 *  @param[in] p  Unused.
 */
void raw::data_is_available_err(process& p) throw () {
  (void)p;
  return;
}

/**
 *  Provide by process_listener interface. Call at the end
 *  of the process execution.
 *
 *  @param[in] p  The process to finished.
 */
void raw::finished(process& p) throw () {
  try {
    logger(dbg_commands, basic)
      << "raw::finished: process=" << &p;

    unsigned long command_id(0);
    {
      concurrency::locker lock(&_lock);
      // Find process from the busy list.
      umap<process*, unsigned long>::iterator
        it(_processes_busy.find(&p));
      if (it == _processes_busy.end()) {
        // Put the process into the free list.
        _processes_free.push_back(&p);
        lock.unlock();

        logger(log_runtime_warning, basic)
          << "Warning: Invalid process pointer: "
             "process not found into process busy list";
        return ;
      }
      // Get command_id and remove the process from the busy list.
      command_id = it->second;
      _processes_busy.erase(it);
    }

    logger(dbg_commands, basic)
      << "raw::finished: id=" << command_id;

    // Build check result.
    result res;

    // Get process output.
    p.read(res.output);

    // Release process, put into the free list.
    {
      concurrency::locker lock(&_lock);
      _processes_free.push_back(&p);
    }

    // Set result informations.
    res.command_id = command_id;
    res.start_time = p.start_time();
    res.end_time = p.end_time();
    res.exit_code = p.exit_code();
    res.exit_status = p.exit_status();

    if (res.exit_status == process::timeout) {
      res.exit_code = STATE_UNKNOWN;
      res.output = "(Process Timeout)";
    }
    else if ((res.exit_status == process::crash)
             || (res.exit_code < -1)
             || (res.exit_code > 3))
      res.exit_code = STATE_UNKNOWN;

    logger(dbg_commands, basic)
      << "raw::finished: "
      "id=" << command_id << ", "
      "start_time=" << res.start_time.to_mseconds() << ", "
      "end_time=" << res.end_time.to_mseconds() << ", "
      "exit_code=" << res.exit_code << ", "
      "exit_status=" << res.exit_status << ", "
      "output='" << res.output << "'";

    // Forward result to the listener.
    if (_listener)
      (_listener->finished)(res);
  }
  catch (std::exception const& e) {
    logger(log_runtime_warning, basic)
      << "Warning: Raw process termination routine failed: "
      << e.what();

    // Release process, put into the free list.
    concurrency::locker lock(&_lock);
    _processes_free.push_back(&p);
  }
  return;
}

/**
 *  Get one process to execute command.
 *
 *  @return A process.
 */
process* raw::_get_free_process() {
  // If any process are available, create new one.
  if (_processes_free.empty()) {
    process* p(new process(this));
    p->enable_stream(process::in, false);
    p->enable_stream(process::err, false);
    p->setpgid_on_exec(config->use_setpgid());
    return (p);
  }
  // Get a free process.
  process* p(_processes_free.front());
  _processes_free.pop_front();
  return (p);
}

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

#include <QCoreApplication>
#include <QMutexLocker>
#include <QTimer>
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

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
 */
raw::raw(std::string const& name, std::string const& command_line)
  : command(name, command_line) {}

/**
 *  Copy constructor
 *
 *  @param[in] right Object to copy.
 */
raw::raw(raw const& right) : command(right) {}

/**
 *  Destructor.
 */
raw::~raw() throw () {
  QMutexLocker lock(&_mutex);
  while (!_processes.empty()) {
    // XXX MK : code below seems broken to me, as vtable is not
    // guaranteed upon destruction. Also, the event loop might not catch
    // signal of process between checking of emptiness and connection of
    // signal.
    process_info info(_processes.begin().value());
    QEventLoop loop;
    connect(
      this,
      SIGNAL(command_executed(commands::result const&)),
      &loop,
      SLOT(quit()));
    lock.unlock();
    loop.exec();
    lock.relock();
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
                     nagios_macros const& macros,
                     unsigned int timeout) {
  // Debug.
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;

  // Create process.
  process_info info;
  info.proc = QSharedPointer<process>(
                new process(macros, timeout),
                &_deletelater_process);

  // Connect to process.
  if (!connect(&(*info.proc),
  	      SIGNAL(process_ended()),
  	      this,
  	      SLOT(raw_ended())))
    throw (engine_error() << "connect process to commands::raw failed.");

  // Store process information.
  {
    QMutexLocker lock(&_mutex);
    info.cmd_id = get_uniq_id();
    _processes.insert(&(*info.proc), info);
  }

  // Start process.
  logger(dbg_commands, basic)
    << "raw command (id=" << info.cmd_id
    << ") start '" << processed_cmd << "'.";
  info.proc->start(processed_cmd);

  // Debug.
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
  return (info.cmd_id);
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
            nagios_macros const& macros,
            unsigned int timeout,
            result& res) {
  // Debug.
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;

  // Get process ID.
  unsigned long id;
  {
    QMutexLocker lock(&_mutex);
    id = get_uniq_id();
  }

  // Create and run process.
  process proc(macros, timeout);
  logger(dbg_commands, basic)
    << "raw command (id=" << id
    << ") start '" << processed_cmd << "'.";
  proc.start(processed_cmd);

  // Wait for completion.
  proc.wait();

  // Provide result on process execution.
  res.set_command_id(id);
  res.set_start_time(proc.get_start_time());
  res.set_end_time(proc.get_end_time());
  res.set_exit_code(proc.get_exit_code());
  res.set_is_timeout(proc.get_is_timeout());
  res.set_stdout(proc.get_stdout());
  res.set_stderr(proc.get_stderr());
  res.set_is_executed(proc.get_is_executed());

  // Debug.
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
  return ;
}

/**
 *  Slot to catch the end of processes et send the result by signal.
 */
void raw::raw_ended() {
  // Debug.
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;

  // Find process info.
  process_info info;
  {
    QMutexLocker lock(&_mutex);
    std::map<QObject*, process_info>::iterator
      it(_processes.find(sender()));
    if (it == _processes.end()) {
      logger(log_runtime_warning, basic) << "sender not found in processes.";
      return ;
    }
    info = it.value();
    _processes.erase(it);
  }

  // Build check result.
  result res(
           info.cmd_id,
           info.proc->get_stdout(),
           info.proc->get_stderr(),
           info.proc->get_start_time(),
           info.proc->get_end_time(),
           info.proc->get_exit_code(),
           info.proc->get_is_timeout(),
           info.proc->get_is_executed());

  // Debug.
  logger(dbg_commands, basic)
    << "raw command (id=" << info.cmd_id << ") finished.";

  // Command finished executing.
  emit command_executed(res);
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
  return ;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Set the object to delete later.
 *
 *  @param[in] obj The object to delete later.
 */
void raw::_deletelater_process(process* obj) {
  obj->deleteLater();
  return ;
}

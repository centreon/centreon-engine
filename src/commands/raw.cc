/*
** Copyright 2011      Merethis
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
#include <QTimer>
#include "engine.hh"
#include "globals.hh"
#include "error.hh"
#include "logging/logger.hh"
#include "commands/raw.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::commands;

/**
 *  Default constructor.
 *
 *  @param[in] name         The command name.
 *  @param[in] command_line The command line.
 */
raw::raw(QString const& name,
	 QString const& command_line)
  : command(name, command_line) {
}

/**
 *  Default copy constructor
 *
 *  @param[in] right The copy class.
 */
raw::raw(raw const& right)
  : command(right) {
  operator=(right);
}

/**
 *  Default destructor.
 */
raw::~raw() throw() {
  _mutex.lock();
  while (_processes.empty() == false) {
    process_info info = _processes.begin().value();

    QEventLoop loop;
    connect(this, SIGNAL(command_executed(commands::result const&)), &loop, SLOT(quit()));
    _mutex.unlock();
    loop.exec();
    _mutex.lock();
  }
  _mutex.unlock();
}

/**
 *  Default copy operator.
 *
 *  @param[in] right The copy class.
 *
 *  @return This object.
 */
raw& raw::operator=(raw const& right) {
  if (this != &right) {
    command::operator=(right);
  }
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
unsigned long raw::run(QString const& processed_cmd,
		       nagios_macros const& macros,
		       unsigned int timeout) {
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;

  process_info info;
  info.proc = QSharedPointer<process>(new process(macros, timeout),
                                      &_deletelater_process);

  if (connect(&(*info.proc),
  	      SIGNAL(process_ended()),
  	      this,
  	      SLOT(raw_ended())) == false) {
    throw (engine_error() << "connect process to commands::raw failed.");
  }

  _mutex.lock();
  info.cmd_id = get_uniq_id();
  _processes.insert(&(*info.proc), info);
  _mutex.unlock();

  logger(dbg_commands, basic)
    << "raw command (id=" << info.cmd_id
    << ") start '" << processed_cmd << "'.";

  info.proc->start(processed_cmd);

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
void raw::run(QString const& processed_cmd,
	      nagios_macros const& macros,
	      unsigned int timeout,
	      result& res) {
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;

  _mutex.lock();
  unsigned long id = get_uniq_id();
  _mutex.unlock();

  process proc(macros, timeout);

  logger(dbg_commands, basic)
    << "raw command (id=" << id
    << ") start '" << processed_cmd << "'.";

  proc.start(processed_cmd);
  proc.wait();

  res.set_command_id(id);
  res.set_start_time(proc.get_start_time());
  res.set_end_time(proc.get_end_time());
  res.set_exit_code(proc.get_exit_code());
  res.set_is_timeout(proc.get_is_timeout());
  res.set_stdout(proc.get_stdout());
  res.set_stderr(proc.get_stderr());
  res.set_is_executed(proc.get_is_executed());

  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
}

/**
 *  Slot to catch the end of processes et send the result by signal.
 */
void raw::raw_ended() {
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;
  _mutex.lock();
  QHash<QObject*, process_info>::iterator it = _processes.find(sender());
  if (it == _processes.end()) {
    _mutex.unlock();
    logger(log_runtime_warning, basic) << "sender not found in processes.";
    return;
  }
  process_info info = it.value();
  _processes.erase(it);
  _mutex.unlock();

  result res(info.cmd_id,
  	     info.proc->get_stdout(),
  	     info.proc->get_stderr(),
  	     info.proc->get_start_time(),
  	     info.proc->get_end_time(),
  	     info.proc->get_exit_code(),
  	     info.proc->get_is_timeout(),
  	     info.proc->get_is_executed());

  logger(dbg_commands, basic)
    << "raw command (id=" << info.cmd_id << ") finished.";

  emit command_executed(res);
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
}

/**
 *  Set the object to delete later.
 *
 *  @param[in] obj The object to delete later.
 */
void raw::_deletelater_process(process* obj) {
  obj->deleteLater();
}

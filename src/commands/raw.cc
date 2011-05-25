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
 *  @param[in] args   The command arguments.
 *  @param[in] macros The macros data struct.
 *
 *  @return The command id.
 */
unsigned long raw::run(QString const& processed_cmd,
		       nagios_macros const& macros,
		       int timeout) {
  timeout = (timeout <= 0 ? -1 : timeout * 1000);

  proc_info proc;
  proc.process = QSharedPointer<eprocess>(new eprocess(macros, timeout));

  if (connect(&(*proc.process),
	      SIGNAL(error(QProcess::ProcessError)),
	      this,
	      SLOT(process_error(QProcess::ProcessError))) == false
      || connect(&(*proc.process),
		 SIGNAL(finished(int, QProcess::ExitStatus)),
		 this,
		 SLOT(process_finished(int, QProcess::ExitStatus))) == false
      || connect(&(*proc.process),
		 SIGNAL(started()),
		 this,
		 SLOT(process_started())) == false
      || connect(&(*proc.process),
		 SIGNAL(readyReadStandardOutput()),
		 this,
		 SLOT(process_stdout())) == false
      || connect(&(*proc.process),
		 SIGNAL(readyReadStandardError()),
		 this,
		 SLOT(process_stderr())) == false) {
    throw (engine_error() << "connect process to commands::raw failed.");
  }

  proc_info& rproc = _processes.insert(&(*proc.process), proc).value();
  rproc.cmd_id = ++_id;
  rproc.process->start(processed_cmd);

  return (_id);
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
	      int timeout,
	      result& res) {
  timeout = (timeout <= 0 ? -1 : timeout * 1000);

  proc_info proc;
  proc.process = QSharedPointer<eprocess>(new eprocess(macros, timeout));

  QHash<QObject*, proc_info>::iterator it =_processes.insert(&(*proc.process), proc);
  proc_info& rproc = it.value();
  rproc.cmd_id = ++_id;

  rproc.process->start(processed_cmd);

  rproc.process->waitForStarted(-1);
  gettimeofday(&rproc.start_time, NULL);

  rproc.process->waitForFinished(timeout);
  gettimeofday(&rproc.end_time, NULL);

  if (rproc.process->state() == QProcess::Running) {
    rproc.process->kill();
    rproc.process->waitForFinished(-1);

    res.set_retval(STATE_CRITICAL);
    res.set_is_timeout(true);
    res.set_stdout("");
    res.set_stderr("Process Timeout");
  }
  else {
    if (rproc.process->exitCode() < -1 || rproc.process->exitCode() > 3) {
      res.set_retval(STATE_UNKNOWN);
    }
    res.set_is_timeout(false);
    res.set_stdout(rproc.process->readAllStandardOutput());
    res.set_stderr(rproc.process->readAllStandardError());
  }

  res.set_cmd_id(rproc.cmd_id);
  res.set_start_time(rproc.start_time);
  res.set_end_time(rproc.end_time);
  res.set_exited_ok(res.get_retval() != STATE_CRITICAL);

  _processes.erase(it);
}

/**
 *  Slot to catch QProcess error.
 *
 *  @param[in] error Unused.
 */
void raw::process_error(QProcess::ProcessError error) {
  (void)error;

  QHash<QObject*, proc_info>::iterator it = _processes.find(sender());
  if (it == _processes.end()) {
    logger(log_runtime_warning, basic) << "sender not found in processes.";
    return;
  }
  proc_info& proc = it.value();
  proc.stdout = "";
  proc.stderr = proc.process->errorString();
}

/**
 *  Slot to catch QProcess ending.
 *
 *  @param[in] exit_code   The exit code of the process.
 *  @param[in] exit_status The exit status.
 */
void raw::process_finished(int exit_code, QProcess::ExitStatus exit_status) {
  (void)exit_status;

  QHash<QObject*, proc_info>::iterator it = _processes.find(sender());
  if (it == _processes.end()) {
    logger(log_runtime_warning, basic) << "sender not found in processes.";
    return;
  }
  proc_info& proc = it.value();
  gettimeofday(&proc.end_time, NULL);

  if (exit_status == QProcess::CrashExit) {
    exit_code = STATE_CRITICAL;
  }
  else if (exit_code < -1 || exit_code > 3) {
    exit_code = STATE_UNKNOWN;
  }

  bool is_timeout = false;
  if (exit_code == STATE_CRITICAL
      && proc.process->get_timeout() != -1
      && proc.end_time.tv_sec - proc.start_time.tv_sec >= proc.process->get_timeout() / 1000) {
    is_timeout = true;
    proc.stderr = "Process Timeout";
  }

  result res(proc.cmd_id,
	     proc.stdout,
	     proc.stderr,
	     proc.start_time,
	     proc.end_time,
	     exit_code,
	     is_timeout,
	     exit_code != STATE_CRITICAL);
  emit command_executed(res);
  _processes.erase(it);
}

/**
 *  Slot to catch QProcess starting.
 */
void raw::process_started() {
  QHash<QObject*, proc_info>::iterator it = _processes.find(sender());
  if (it == _processes.end()) {
    logger(log_runtime_warning, basic) << "sender not found in processes.";
    return;
  }
  proc_info& proc = it.value();
  gettimeofday(&proc.start_time, NULL);
  if (proc.process->get_timeout() != -1) {
    QTimer::singleShot(proc.process->get_timeout(), sender(), SLOT(kill()));
  }
}

/**
 *  Slot to catch standard output.
 */
void raw::process_stdout() {
  QHash<QObject*, proc_info>::iterator it = _processes.find(sender());
  if (it == _processes.end()) {
    logger(log_runtime_warning, basic) << "sender not found in processes.";
    return;
  }
  proc_info& proc = it.value();
  proc.stdout += proc.process->readAllStandardOutput();
}

/**
 *  Slot to catch error output.
 */
void raw::process_stderr() {
  QHash<QObject*, proc_info>::iterator it = _processes.find(sender());
  if (it == _processes.end()) {
    logger(log_runtime_warning, basic) << "sender not found in processes.";
    return;
  }
  proc_info& proc = it.value();
  proc.stderr += proc.process->readAllStandardOutput();
}

/**
 *  Default constructor of private implementation of QProcess.
 *
 *  @param[in] macros The specific macros to build user environment.
 */
raw::eprocess::eprocess(nagios_macros const& macros, int timeout)
  : _macros(macros), _timeout(timeout) {

}

/**
 *  Default destructor.
 */
raw::eprocess::~eprocess() {

}

/**
 *  Get the timeout value.
 *
 *  @return The timeout value, if no timeout return -1.
 */
int raw::eprocess::get_timeout() {
  return (_timeout);
}

/**
 *  Overload of QProcess::setupChildPorcess to build user environment before
 *  create process.
 */
void raw::eprocess::setupChildProcess() {
  set_all_macro_environment_vars(&_macros, true);
}

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

#include <QEventLoop>
#include <QTimer>
#include "com/centreon/engine/commands/process.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine::commands;
using namespace com::centreon::engine::logging;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default constructor.
 *
 *  @param[in] macros  The macros data struct.
 *  @param[in] timeout The timeout time, -1 disable timeout.
 */
process::process(nagios_macros const& macros, unsigned int timeout)
  : basic_process(),
    _executed_time(0),
    _exit_code(0),
    _is_executed(false),
    _is_timeout(false),
    _macros(macros),
    _timeout(timeout * 1000) {
  connect(
    this,
    SIGNAL(error(QProcess::ProcessError)),
    this,
    SLOT(_error(QProcess::ProcessError)));
  connect(
    this,
    SIGNAL(finished(int, QProcess::ExitStatus)),
    this,
    SLOT(_finished(int, QProcess::ExitStatus)));
  connect(this, SIGNAL(started()), this, SLOT(_started()));
}

/**
 *  Default copy constructor.
 *
 *  @param[in] right Object to copy.
 */
process::process(process const& right) : basic_process() {
  connect(
    this,
    SIGNAL(error(QProcess::ProcessError)),
    this,
    SLOT(_error(QProcess::ProcessError)));
  connect(
    this,
    SIGNAL(finished(int, QProcess::ExitStatus)),
    this,
    SLOT(_finished(int, QProcess::ExitStatus)));
  connect(this, SIGNAL(started()), this, SLOT(_started()));
  _internal_copy(right);
}

/**
 *  Destructor.
 */
process::~process() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
process& process::operator=(process const& right) {
  if (this != &right)
    _internal_copy(right);
  return (*this);
}

/**
 *  Get the end time of the process.
 *
 *  @return The time.
 */
QDateTime const& process::get_end_time() const throw () {
  return (_end_time);
}

/**
 *  Get the execution time of the process.
 *
 *  @return The time on msec.
 */
unsigned int process::get_executed_time() const throw () {
  return (_executed_time);
}

/**
 *  Get the exit code of the process.
 *
 *  @return The exit code.
 */
int process::get_exit_code() const throw () {
  return (_exit_code);
}

/**
 *  Get if the process execute.
 *
 *  @return True if the process execute.
 */
bool process::get_is_executed() const throw () {
  return (_is_executed);
}

/**
 *  Get if the process stop by timeout.
 *
 *  @return True if the process stop by timeout.
 */
bool process::get_is_timeout() const throw () {
  return (_is_timeout);
}

/**
 *  Get the start time of the process.
 *
 *  @return The Time.
 */
QDateTime const& process::get_start_time() const throw () {
  return (_start_time);
}

/**
 *  Get the standard error of the process.
 *
 *  @return The standard error string.
 */
std::string const& process::get_stderr() const throw () {
  return (_stderr);
}

/**
 *  Get the standard output of the process.
 *
 *  @return The standard output string.
 */
std::string const& process::get_stdout() const throw () {
  return (_stdout);
}

/**
 *  Get the timeout value.
 *
 *  @return The timeout value.
 */
unsigned int process::get_timeout() const throw () {
  return (_timeout);
}

/**
 *  Wait the end of the process (blocking).
 */
void process::wait() {
  QEventLoop loop;
  connect(this, SIGNAL(process_ended()), &loop, SLOT(quit()));
  if (state() != QProcess::NotRunning)
    loop.exec();
  return ;
}

/**************************************
*                                     *
*          Protected Methods          *
*                                     *
**************************************/

/**
 *  Overload of QProcess::setupChildPorcess to build user environment
 *  before creating process.
 */
void process::setupChildProcess() {
  set_all_macro_environment_vars_r(&_macros, true);
  return ;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Slot to catch QProcess error.
 *
 *  @param[in] error Error type of QProcess.
 */
void process::_error(QProcess::ProcessError error) {
  // Debug.
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;

  // Handle error.
  if (QProcess::FailedToStart == error) {
    _start_time = QDateTime::currentDateTime();
    _is_timeout = false;
    emit finished(exitCode(), exitStatus());
  }

  // Debug.
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
  return ;
}

/**
 *  Slot to catch QProcess ending.
 *
 *  @param[in] exit_code   The exit code of the process.
 *  @param[in] exit_status The exit status.
 */
void process::_finished(
                int exit_code,
                QProcess::ExitStatus exit_status) {
  // Debug.
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;

  // Set timing information about process.
  _end_time = QDateTime::currentDateTime();
  _executed_time = _end_time.toTime_t() - _start_time.toTime_t();

  // Handle errors.
  if (error() == QProcess::FailedToStart) {
    _exit_code = STATE_CRITICAL;
    _stderr = "(" + errorString().toStdString() + ")";
    _stdout = "";
    _is_executed = false;
  }
  // Timeout elapsed.
  else if (_is_timeout == true) {
    _exit_code = STATE_CRITICAL;
    _stderr = "(Process Timeout)";
    _stdout = "";
    _is_executed = true;
  }
  // Get termination information.
  else {
    if (exit_status == QProcess::CrashExit)
      _exit_code = STATE_CRITICAL;
    else if ((exit_code < -1) || (exit_code > 3))
      _exit_code = STATE_UNKNOWN;
    else
      _exit_code = exit_code;
    _stderr = readAllStandardError();
    _stdout = readAllStandardOutput();
    _is_executed = (exit_status != QProcess::CrashExit);
  }

  // Notify of termination.
  emit process_ended();

  // Debug.
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
  return ;
}

/**
 *  Copy internal data members.
 *
 *  @param[in] right Object to copy.
 */
void process::_internal_copy(process const& right) {
  _start_time = right._start_time;
  _end_time = right._end_time;
  _stderr = right._stderr;
  _stdout = right._stdout;
  _macros = right._macros;
  _executed_time = right._executed_time;
  _timeout = right._timeout;
  _exit_code = right._exit_code;
  _is_timeout = right._is_timeout;
  _is_executed = right._is_executed;
  return ;
}

/**
 *  Slot to catch QProcess starting.
 */
void process::_started() {
  // Debug.
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;

  // Set information.
  _start_time = QDateTime::currentDateTime();
  _is_timeout = false;
  if (_timeout > 0)
    QTimer::singleShot(_timeout, this, SLOT(_timedout()));

  // We never send information on process' stdin.
  closeWriteChannel();

  // Debug.
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
  return ;
}

/**
 *  Slot to catch process timeout.
 */
void process::_timedout() {
  // Debug.
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;

  // Timeout occurs, kill process.
  if (state() == QProcess::Running) {
    kill();
    _is_timeout = true;
  }

  // Debug.
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
  return ;
}

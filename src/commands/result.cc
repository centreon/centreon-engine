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

#include "commands/result.hh"

using namespace com::centreon::engine::commands;

/**
 *  Default constructor.
 *
 *  @param[in] cmd_id         The command id.
 *  @param[in] stdout         The standard output.
 *  @param[in] stderr         The error output.
 *  @param[in] start_time     The command start time.
 *  @param[in] end_time       The command end time.
 *  @param[in] retval         The return value.
 *  @param[in] timeout        Command run and timeout.
 *  @param[in] exited_ok      Command run and exit normaly.
 */
result::result(unsigned long cmd_id,
	       QString const& stdout,
	       QString const& stderr,
	       timeval const& start_time,
	       timeval const& end_time,
	       int retval,
	       bool is_timeout,
	       bool exited_ok)
  : _stdout(stdout),
    _stderr(stderr),
    _start_time(start_time),
    _end_time(end_time),
    _cmd_id(cmd_id),
    _retval(retval),
    _is_timeout(is_timeout),
    _exited_ok(exited_ok) {
}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The copy class.
 */
result::result(result const& right) {
  operator=(right);
}

/**
 *  Default desctructor.
 */
result::~result() throw() {

}

/**
 *  Default copy operator.
 *
 *  @param[in] right The copy class.
 *
 *  @return This object.
 */
result& result::operator=(result const& right) {
  if (this != &right) {
    _stdout = right._stdout;
    _stderr = right._stderr;
    _start_time = right._start_time;
    _end_time = right._end_time;
    _cmd_id = right._cmd_id;
    _retval = right._retval;
    _exited_ok = right._exited_ok;
    _is_timeout = right._is_timeout;
  }
  return (*this);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the same value.
 */
bool result::operator==(result const& right) const throw() {
  return (_stdout == right._stdout
	  && _stderr == right._stderr
	  && _start_time.tv_sec == right._start_time.tv_sec
	  && _start_time.tv_usec == right._start_time.tv_usec
	  && _end_time.tv_sec == right._end_time.tv_sec
	  && _end_time.tv_usec == right._end_time.tv_usec
	  && _cmd_id == right._cmd_id
	  && _retval == right._retval
	  && _exited_ok == right._exited_ok);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the different value.
 */
bool result::operator!=(result const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Get the command id.
 *
 *  @return The command id.
 */
unsigned long result::get_cmd_id() const throw() {
  return (_cmd_id);
}

/**
 *  Get the return value.
 *
 *  @return The return value.
 */
int result::get_retval() const throw() {
  return (_retval);
}

/**
 *  Get the execution time.
 *
 *  @return The execution time.
 */
double result::get_execution_time() const throw() {
  double res = (double)(_end_time.tv_sec - _start_time.tv_sec)
    + (double)(_end_time.tv_usec - _start_time.tv_usec);
  return (res < 0.0 ? 0.0 : res);
}

/**
 *  Get the execution start time.
 *
 *  @return The execution start time.
 */
timeval const& result::get_start_time() const throw() {
  return (_start_time);
}

/**
 *  Get the execution end time.
 *
 *  @return The execution end time.
 */
timeval const& result::get_end_time() const throw() {
  return (_end_time);
}

/**
 *  Get the standard output.
 *
 *  @return The standard output.
 */
QString const& result::get_stdout() const throw() {
  return (_stdout);
}

/**
 *  Get the error output.
 *
 *  @return The error output.
 */
QString const& result::get_stderr() const throw() {
  return (_stderr);
}

/**
 *  Get if the execution failed.
 *
 *  @return True if the execution success, false otherwise.
 */
bool result::get_exited_ok() const throw() {
  return (_exited_ok);
}

/**
 *  Get if the execution timeout.
 *
 *  @return True if the execution timedout, false otherwise.
 */
bool result::get_is_timeout() const throw() {
  return (_is_timeout);
}

/**
 *  Set the command id.
 *
 *  @param[in] id The command id.
 */
void result::set_cmd_id(unsigned long id) throw() {
  _cmd_id = id;
}

/**
 *  Set the return value.
 *
 *  @param[in] retval The return value.
 */
void result::set_retval(int retval) throw() {
  _retval = retval;
}

/**
 *  Set the start time.
 *
 *  @param[in] tv The start time.
 */
void result::set_start_time(timeval const& tv) throw() {
  _start_time = tv;
}

/**
 *  Set the end time.
 *
 *  @param[in] tv The end time.
 */
void result::set_end_time(timeval const& tv) throw() {
  _end_time = tv;
}

/**
 *  Set the standard output.
 *
 *  @param[in] str The standard output.
 */
void result::set_stdout(QString const& str) {
  _stdout = str;
}

/**
 *  Set the error output.
 *
 *  @param[in] str The error output.
 */
void result::set_stderr(QString const& str) {
  _stderr = str;
}

/**
 *  Set the exited value.
 *
 *  @param[in] value The exited value.
 */
void result::set_exited_ok(bool value) throw() {
  _exited_ok = value;
}

/**
 *  Set the timeout value.
 *
 *  @param[in] value The timeout value.
 */
void result::set_is_timeout(bool value) throw() {
  _is_timeout = value;
}

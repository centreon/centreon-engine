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

#include "com/centreon/engine/commands/result.hh"
#include "com/centreon/timestamp.hh"

using namespace com::centreon;
using namespace com::centreon::engine::commands;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Constructor.
 *
 *  @param[in] cmd_id         The command id.
 *  @param[in] stdout         The standard output.
 *  @param[in] stderr         The error output.
 *  @param[in] start_time     The command start time.
 *  @param[in] end_time       The command end time.
 *  @param[in] exit_code      The return value.
 *  @param[in] timeout        Command run and timeout.
 *  @param[in] is_executed    Command run and exit normaly.
 */
result::result(
          unsigned long cmd_id,
          std::string const& stdout,
          std::string const& stderr,
          timestamp const& start_time,
          timestamp const& end_time,
          int exit_code,
          bool is_timeout,
          bool is_executed)
  : _cmd_id(cmd_id),
    _exit_code(exit_code),
    _is_executed(is_executed),
    _is_timeout(is_timeout),
    _stderr(stderr),
    _stdout(stdout) {
  if (start_time != 0)
    set_start_time(start_time);
  if (end_time != 0)
    set_end_time(end_time);
}

/**
 *  Copy constructor.
 *
 *  @param[in] right The copy class.
 */
result::result(result const& right) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
result::~result() throw () {}

/**
 *  Default copy operator.
 *
 *  @param[in] right The copy class.
 *
 *  @return This object.
 */
result& result::operator=(result const& right) {
  if (this != &right)
    _internal_copy(right);
  return (*this);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the same value.
 */
bool result::operator==(result const& right) const throw () {
  return ((_cmd_id == right._cmd_id)
          && (_exit_code == right._exit_code)
          && (_is_executed == right._is_executed)
          && (_is_timeout == right._is_timeout)
          && (_end_time == right._end_time)
          && (_start_time == right._start_time)
          && (_stderr == right._stderr)
          && (_stdout == right._stdout));
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the different value.
 */
bool result::operator!=(result const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Get the command id.
 *
 *  @return The command id.
 */
unsigned long result::get_command_id() const throw () {
  return (_cmd_id);
}

/**
 *  Get the execution end time.
 *
 *  @return The execution end time.
 */
timestamp const& result::get_end_time() const throw () {
  return (_end_time);
}

/**
 *  Get the execution time.
 *
 *  @return The execution time.
 */
unsigned int result::get_execution_time() const throw () {
  if (_end_time < _start_time)
    return (0);
  return (_end_time.to_seconds() - _start_time.to_seconds());
}

/**
 *  Get the return value.
 *
 *  @return The return value.
 */
int result::get_exit_code() const throw () {
  return (_exit_code);
}

/**
 *  Get if the execution failed.
 *
 *  @return True if the execution success, false otherwise.
 */
bool result::get_is_executed() const throw () {
  return (_is_executed);
}

/**
 *  Get if the execution timeout.
 *
 *  @return True if the execution timedout, false otherwise.
 */
bool result::get_is_timeout() const throw () {
  return (_is_timeout);
}

/**
 *  Get the execution start time.
 *
 *  @return The execution start time.
 */
timestamp const& result::get_start_time() const throw () {
  return (_start_time);
}

/**
 *  Get the error output.
 *
 *  @return The error output.
 */
std::string const& result::get_stderr() const throw () {
  return (_stderr);
}

/**
 *  Get the standard output.
 *
 *  @return The standard output.
 */
std::string const& result::get_stdout() const throw () {
  return (_stdout);
}

/**
 *  Set the command id.
 *
 *  @param[in] id The command id.
 */
void result::set_command_id(unsigned long id) throw () {
  _cmd_id = id;
  return ;
}

/**
 *  Set the end time.
 *
 *  @param[in] tv The end time.
 */
void result::set_end_time(timestamp const& t) throw () {
  _end_time = t;
  return ;
}

/**
 *  Set the return value.
 *
 *  @param[in] exit_code The return value.
 */
void result::set_exit_code(int exit_code) throw () {
  _exit_code = exit_code;
  return ;
}

/**
 *  Set the exited value.
 *
 *  @param[in] value The exited value.
 */
void result::set_is_executed(bool value) throw () {
  _is_executed = value;
  return ;
}

/**
 *  Set the timeout value.
 *
 *  @param[in] value The timeout value.
 */
void result::set_is_timeout(bool value) throw () {
  _is_timeout = value;
  return ;
}

/**
 *  Set the start time.
 *
 *  @param[in] t The start time.
 */
void result::set_start_time(timestamp const& t) throw () {
  _start_time = t;
  return ;
}

/**
 *  Set the error output.
 *
 *  @param[in] str The error output.
 */
void result::set_stderr(std::string const& str) {
  _stderr = str;
  return ;
}

/**
 *  Set the standard output.
 *
 *  @param[in] str The standard output.
 */
void result::set_stdout(std::string const& str) {
  _stdout = str;
  return ;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Copy internal data members.
 *
 *  @param[in] right Object to copy.
 */
void result::_internal_copy(result const& right) {
  _cmd_id = right._cmd_id;
  _end_time = right._end_time;
  _exit_code = right._exit_code;
  _is_executed = right._is_executed;
  _is_timeout = right._is_timeout;
  _start_time = right._start_time;
  _stderr = right._stderr;
  _stdout = right._stdout;
  return ;
}

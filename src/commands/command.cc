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

#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/engine/commands/command.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

static concurrency::mutex _lock_id;
static unsigned long      _id = 0;

/**
 *  Default constructor
 *
 *  @param[in] name         The command name.
 *  @param[in] command_line The command line.
 *  @param[in] listener     The command listener to catch events.
 */
commands::command::command(
                     std::string const& name,
                     std::string const& command_line,
                     command_listener* listener)
  : _command_line(command_line),
    _listener(listener),
    _name(name) {

}

/**
 *  Destructor.
 */
commands::command::~command() throw () {

}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the same value.
 */
bool commands::command::operator==(command const& right) const throw() {
  return (_name == right._name && _command_line == right._command_line);
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the different value.
 */
bool commands::command::operator!=(command const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Get the command line.
 *
 *  @return The command line.
 */
std::string const& commands::command::get_command_line() const throw() {
  return (_command_line);
}

/**
 *  Get the command listener.
 *
 *  @return The listener who catch events.
 */
commands::command_listener* commands::command::get_listener() const throw() {
  return (_listener);
}

/**
 *  Get the command name.
 *
 *  @return The command name.
 */
std::string const& commands::command::get_name() const throw() {
  return (_name);
}

/**
 *  Set the command line.
 *
 *  @param[in] command_line The command line.
 */
void commands::command::set_command_line(
                          std::string const& command_line) {
  _command_line = command_line;
  return;
}

/**
 *  Set the command listener.
 *
 *  @param[in] listener  The listener who catch events.
 */
void commands::command::set_listener(
                          commands::command_listener* listener) throw () {
  _listener = listener;
  return;
}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The copy class.
 */
commands::command::command(commands::command const& right) {
  operator=(right);
}

/**
 *  Default copy operatro.
 *
 *  @param[in] right The copy class.
 *
 *  @return This object.
 */
commands::command& commands::command::operator=(commands::command const& right) {
  if (this != &right) {
    _command_line = right._command_line;
    _listener = right._listener;
    _name = right._name;
  }
  return (*this);
}

/**
 *  Get the processed command line.
 *
 *  @param[in] macros The macros list.
 *
 *  @return The processed command line.
 */
std::string commands::command::process_cmd(nagios_macros* macros) const {
  char* command_line = NULL;
  process_macros_r(macros, _command_line.c_str(), &command_line, 0);
  std::string processed_cmd(command_line);
  delete[] command_line;
  return (processed_cmd);
}

/**
 *  Get the unique command id.
 *
 *  @return The unique command id.
 */
unsigned long commands::command::get_uniq_id() {
  concurrency::locker locker(&_lock_id);
  return (++_id);
}

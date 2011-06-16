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

#include "commands/command.hh"

using namespace com::centreon::engine;

unsigned long commands::command::_id = 0;

/**
 *  Default constructor
 *
 *  @param[in] name         The command name.
 *  @param[in] command_line The command line.
 */
commands::command::command(QString const& name,
			   QString const& command_line)
  : QObject(),
    _name(name),
    _command_line(command_line) {

}

/**
 *  Default destructor.
 */
commands::command::~command() throw() {

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
 *  Get the command name.
 *
 *  @return The command name.
 */
QString const& commands::command::get_name() const throw() {
  return (_name);
}

/**
 *  Get the command line.
 *
 *  @return The command line.
 */
QString const& commands::command::get_command_line() const throw() {
  return (_command_line);
}

/**
 *  Set the command name.
 *
 *  @param[in] name The command name.
 */
void commands::command::set_name(QString const& name) {
  QString old_name = _name;
  _name = name;
  emit name_changed(old_name, _name);
}

/**
 *  Set the command line.
 *
 *  @param[in] command_line The command line.
 */
void commands::command::set_command_line(QString const& command_line) {
  _command_line = command_line;
}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The copy class.
 */
commands::command::command(commands::command const& right)
  : QObject() {
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
    _name = right._name;
    _command_line = right._command_line;
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
QString commands::command::process_cmd(nagios_macros* macros) const {
  char* command_line = NULL;
  process_macros_r(macros, qPrintable(_command_line), &command_line, 0);
  QString processed_cmd(command_line);
  delete[] command_line;
  return (processed_cmd);
}

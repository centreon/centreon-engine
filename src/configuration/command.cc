/*
** Copyright 2011-2013 Merethis
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

#include "com/centreon/engine/configuration/command.hh"
#include <memory>
#include "com/centreon/engine/exceptions/error.hh"

using namespace com::centreon;
using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<command, type, &command::method>::generic

std::unordered_map<std::string, command::setter_func> const command::_setters{
    {"command_line", SETTER(std::string const&, _set_command_line)},
    {"command_name", SETTER(std::string const&, _set_command_name)},
    {"connector", SETTER(std::string const&, _set_connector)}};

/**
 *  Constructor.
 *
 *  @param[in] key The object key.
 */
command::command(key_type const& key)
    : object(object::command), _command_name(key) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The command to copy.
 */
command::command(command const& right) : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
command::~command() throw() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The command to copy.
 *
 *  @return This command.
 */
command& command::operator=(command const& right) {
  if (this != &right) {
    object::operator=(right);
    _command_line = right._command_line;
    _command_name = right._command_name;
    _connector = right._connector;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The command to compare.
 *
 *  @return True if is the same command, otherwise false.
 */
bool command::operator==(command const& right) const throw() {
  return (object::operator==(right) && _command_line == right._command_line &&
          _command_name == right._command_name &&
          _connector == right._connector);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The command to compare.
 *
 *  @return True if is not the same command, otherwise false.
 */
bool command::operator!=(command const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Less-than operator.
 *
 *  @param[in] right Object to compare to.
 *
 *  @return True if this object is strictly less than right.
 */
bool command::operator<(command const& right) const throw() {
  if (_command_name != right._command_name)
    return (_command_name < right._command_name);
  return (_command_line < right._command_line);
}

/**
 *  @brief Check if the object is valid.
 *
 *  If the object is not valid, an exception is thrown.
 */
void command::check_validity() const {
  if (_command_name.empty())
    throw(engine_error() << "Command has no name (property 'command_name')");
  if (_command_line.empty())
    throw(engine_error() << "Command '" << _command_name
                         << "' has no command line (property 'command_line')");
  return;
}

/**
 *  Get the command key.
 *
 *  @return The command name.
 */
command::key_type const& command::key() const throw() {
  return (_command_name);
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void command::merge(object const& obj) {
  if (obj.type() != _type)
    throw(engine_error() << "Cannot merge command with '" << obj.type() << "'");
  command const& tmpl(static_cast<command const&>(obj));

  MRG_DEFAULT(_command_line);
  MRG_DEFAULT(_command_name);
  MRG_DEFAULT(_connector);
}

/**
 *  Parse and set the command property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool command::parse(char const* key, char const* value) {
  std::unordered_map<std::string, command::setter_func>::const_iterator it{
      _setters.find(key)};
  if (it != _setters.end())
    return (it->second)(*this, value);
  return false;
}

/**
 *  Get command_line.
 *
 *  @return The command_line.
 */
std::string const& command::command_line() const throw() {
  return (_command_line);
}

/**
 *  Get command_name.
 *
 *  @return The command_name.
 */
std::string const& command::command_name() const throw() {
  return (_command_name);
}

/**
 *  Get connector.
 *
 *  @return The connector.
 */
std::string const& command::connector() const throw() {
  return (_connector);
}

/**
 *  Set command_line value.
 *
 *  @param[in] value The new command_line value.
 *
 *  @return True on success, otherwise false.
 */
bool command::_set_command_line(std::string const& value) {
  _command_line = value;
  return (true);
}

/**
 *  Set command_name value.
 *
 *  @param[in] value The new command_name value.
 *
 *  @return True on success, otherwise false.
 */
bool command::_set_command_name(std::string const& value) {
  _command_name = value;
  return (true);
}

/**
 *  Set connector value.
 *
 *  @param[in] value The new connector value.
 *
 *  @return True on success, otherwise false.
 */
bool command::_set_connector(std::string const& value) {
  _connector = value;
  return (true);
}

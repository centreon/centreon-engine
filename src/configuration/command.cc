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

#include <memory>
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/commands/forward.hh"
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/configuration/command.hh"
#include "com/centreon/engine/error.hh"

using namespace com::centreon::engine;

#define SETTER(type, method) \
  &configuration::object::setter< \
     configuration::command, \
     type, \
     &configuration::command::method>::generic

static struct {
  std::string const name;
  bool (*func)(configuration::command&, std::string const&);
} gl_setters[] = {
  { "command_line", SETTER(std::string const&, _set_command_line) },
  { "command_name", SETTER(std::string const&, _set_command_name) },
  { "connector",    SETTER(std::string const&, _set_connector) }
};

/**
 *  Default constructor.
 */
configuration::command::command()
  : object(object::command, "command") {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The command to copy.
 */
configuration::command::command(command const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
configuration::command::~command() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The command to copy.
 *
 *  @return This command.
 */
configuration::command& configuration::command::operator=(
                          command const& right) {
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
bool configuration::command::operator==(
       command const& right) const throw () {
  return (object::operator==(right)
          && _command_line == right._command_line
          && _command_name == right._command_name
          && _connector == right._connector);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The command to compare.
 *
 *  @return True if is not the same command, otherwise false.
 */
bool configuration::command::operator!=(
       command const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Create new command.
 *
 *  @return The new command.
 */
::command* configuration::command::create() const {
  if (!is_valid())
    throw (engine_error() << "configuration: invalid command name "
           << _command_name << " property missing");

  commands::set& cmd_set(commands::set::instance());

  // Without connector.
  if (_connector.empty()) {
    shared_ptr<commands::command>
      cmd(new commands::raw(
                _command_name,
                _command_line,
                &checks::checker::instance()));
    cmd_set.add_command(cmd);
  }
  // With connector.
  else {
    shared_ptr<commands::command>
      cmd_forward(cmd_set.get_command(_connector));

    shared_ptr<commands::command>
      cmd(new commands::forward(
                _command_name,
                _command_line,
                *cmd_forward));
    cmd_set.add_command(cmd);
  }

  std::auto_ptr< ::command> obj(new ::command);
  memset(obj.get(), 0, sizeof(*obj));
  obj->name = my_strdup(_command_name.c_str());
  obj->command_line = my_strdup(_command_line.c_str());
  return (obj.release());
}

/**
 *  Get the unique object id.
 *
 *  @return The object id.
 */
std::size_t configuration::command::id() const throw () {
  return (_id);
}

/**
 *  Check if the object is valid.
 *
 *  @return True if is a valid object, otherwise false.
 */
bool configuration::command::is_valid() const throw () {
  return (!_command_name.empty()
          && !_command_line.empty());
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void configuration::command::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
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
bool configuration::command::parse(
       std::string const& key,
       std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  return (false);
}

/**
 *  Set command_line value.
 *
 *  @param[in] value The new command_line value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::command::_set_command_line(
       std::string const& value) {
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
bool configuration::command::_set_command_name(
       std::string const& value) {
  _command_name = value;
  _id = _hash(value);
  return (true);
}

/**
 *  Set connector value.
 *
 *  @param[in] value The new connector value.
 *
 *  @return True on success, otherwise false.
 */
bool configuration::command::_set_connector(
       std::string const& value) {
  _connector = value;
  return (true);
}

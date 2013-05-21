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
#include "com/centreon/engine/error.hh"

using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<command, type, &command::method>::generic

static struct {
  std::string const name;
  bool (*func)(command&, std::string const&);
} gl_setters[] = {
  { "command_line", SETTER(std::string const&, _set_command_line) },
  { "command_name", SETTER(std::string const&, _set_command_name) },
  { "connector",    SETTER(std::string const&, _set_connector) }
};

/**
 *  Default constructor.
 */
command::command()
  : object("command") {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The command to copy.
 */
command::command(command const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
command::~command() throw () {

}

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
bool command::operator==(command const& right) const throw () {
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
bool command::operator!=(command const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void command::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
  command const& tmpl(static_cast<command const&>(obj));

  MRG_STRING(_command_line);
  MRG_STRING(_command_name);
  MRG_STRING(_connector);
}

/**
 *  Parse and set the command property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool command::parse(std::string const& key, std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  return (false);
}

bool command::_set_command_line(std::string const& value) {
  _command_line = value;
  return (true);
}

bool command::_set_command_name(std::string const& value) {
  _command_name = value;
  return (true);
}

bool command::_set_connector(std::string const& value) {
  _connector = value;
  return (true);
}

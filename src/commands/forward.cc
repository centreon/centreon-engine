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

#include <cstdlib>
#include <list>
#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/engine/commands/forward.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/version.hh"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::commands;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Constructor.
 *
 *  @param[in] command_name  The command name.
 *  @param[in] command_line  The command command line.
 *  @param[in] command       The command to forward command.
 */
forward::forward(
             std::string const& command_name,
             std::string const& command_line,
             command& cmd)
  : command(command_name, command_line, nullptr),
    _command(&cmd) {
  if (_name.empty())
    throw (engine_error()
      << "Could not create a command with an empty name");
  if (_command_line.empty())
    throw (engine_error()
      << "Could not create '"
      << _name << "' command: command line is empty");
}

/**
 *  Copy constructor
 *
 *  @param[in] right Object to copy.
 */
forward::forward(forward const& right)
  : command(right),
    _command(right._command) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
forward::~forward() throw() {

}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
forward& forward::operator=(forward const& right) {
  _internal_copy(right);
  return (*this);
}

/**
 *  Get a pointer on a copy of the same object.
 *
 *  @return Return a pointer on a copy object.
 */
com::centreon::engine::commands::command* forward::clone() const {
  return (new forward(*this));
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
unsigned long forward::run(
                         std::string const& processed_cmd,
                         nagios_macros& macros,
                         unsigned int timeout) {
  return (_command->run(processed_cmd, macros, timeout));
}

/**
 *  Run a command and wait the result.
 *
 *  @param[in]  args    The command arguments.
 *  @param[in]  macros  The macros data struct.
 *  @param[in]  timeout The command timeout.
 *  @param[out] res     The result of the command.
 */
void forward::run(
                std::string const& processed_cmd,
                nagios_macros& macros,
                unsigned int timeout,
                result& res) {
  _command->run(processed_cmd, macros, timeout, res);
  return;
}

/**
 *  Internal copy.
 *
 *  @param[in] right  The object to copy.
 */
void forward::_internal_copy(forward const& right) {
  if (this != &right) {
    command::operator=(right);
    _command = right._command;
  }
  return;
}

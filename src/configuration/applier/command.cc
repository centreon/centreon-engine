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

#include <cstring>
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/commands/connector.hh"
#include "com/centreon/engine/commands/forward.hh"
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::command::command() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::command::command(applier::command const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::command::~command() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::command& applier::command::operator=(
                                      applier::command const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new command.
 *
 *  @param[in] obj The new command to add into the monitoring engine.
 */
void applier::command::add_object(
                         shared_ptr<configuration::command> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new command '" << obj->command_name() << "'.";

  // Add command to the global configuration set.
  config->commands().insert(obj);

  // Create compatibility command.
  command_struct* c(add_command(
                      obj->command_name().c_str(),
                      obj->command_line().c_str()));
  if (!c)
    throw (engine_error() << "Error: Could not register command '"
           << obj->command_name() << "'.");

  // Create real command object.
  _create_command(obj);

  return ;
}

/**
 *  @brief Expand command.
 *
 *  Command configuration objects do not need expansion. Therefore this
 *  method does nothing.
 *
 *  @param[in] obj Unused.
 *  @param[in] s   Unused.
 */
void applier::command::expand_object(
                         shared_ptr<configuration::command> obj,
                         configuration::state& s) {
  (void)obj;
  (void)s;
  return ;
}

/**
 *  Modified command.
 *
 *  @param[in] obj The new command to modify into the monitoring engine.
 */
void applier::command::modify_object(
                         shared_ptr<configuration::command> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying command '" << obj->command_name() << "'.";

  // XXX : modify global configuration set

  // Modify command.
  shared_ptr<command_struct>&
    c(applier::state::instance().commands()[obj->command_name()]);
  modify_if_different(c->command_line, obj->command_line().c_str());

  // Command will be temporarily removed from the command set but will
  // be added back right after with _create_command. This does not
  // create dangling pointers since commands::command object are not
  // referenced anywhere, only ::command objects are.
  commands::set::instance().remove_command(obj->command_name());
  _create_command(obj);

  return ;
}

/**
 *  Remove old command.
 *
 *  @param[in] obj The new command to remove from the monitoring engine.
 */
void applier::command::remove_object(
                         shared_ptr<configuration::command> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing command '" << obj->command_name() << "'.";

  // Unregister command.
  unregister_object<command_struct, &command_struct::name>(
    &command_list,
    obj->command_name().c_str());

  // Remove command objects.
  applier::state::instance().commands().erase(obj->command_name());
  commands::set::instance().remove_command(obj->command_name());

  // Remove command from the global configuration set.
  config->commands().erase(obj);

  return ;
}

/**
 *  Resolve command.
 *
 *  @param[in] obj Unused.
 */
void applier::command::resolve_object(
                         shared_ptr<configuration::command> obj) {
  // XXX : resolution should occur, command objects should check for
  //       the validity of their connector (if any). However there is
  //       currently no support code to do that in the commands::set
  //       and other classes of the commands namespace.

  return ;
}

/**
 *  @brief Find real command object.
 *
 *  Create the commands::command object. This can be either a
 *  commands::raw object or a commands::forward object.
 *
 *  @param[in] obj Command configuration object.
 */
void applier::command::_create_command(
                         shared_ptr<configuration::command> obj) {
  // Command set.
  commands::set& cmd_set(commands::set::instance());

  // Raw command.
  if (obj->connector().empty()) {
    shared_ptr<commands::command>
      cmd(new commands::raw(
                          obj->command_name(),
                          obj->command_line(),
                          &checks::checker::instance()));
    cmd_set.add_command(cmd);
  }
  // Connector command.
  else {
    shared_ptr<commands::command>
      cmd(new commands::forward(
                          obj->command_name(),
                          obj->command_line(),
                          *cmd_set.get_command(obj->connector())));
    cmd_set.add_command(cmd);
  }

  return ;
}

/**
 *  Compare a commandsmember list with a list of string.
 *
 *  @param[in] left  First list.
 *  @param[in] right Second list.
 *
 *  @return True if both lists contain the same entries.
 */
bool operator==(
       commandsmember_struct const* left,
       std::list<std::string> const& right) {
  std::list<std::string>::const_iterator
    it(right.begin()),
    end(right.end());
  while (left && (it != end)) {
    if (strcmp(left->cmd, it->c_str()))
      return (false);
    left = left->next;
    ++it;
  }
  return (!left && (it == end));
}

/**
 *  Compare a list of string with a commandsmember list.
 *
 *  @param[in] left  First list.
 *  @param[in] right Second list.
 *
 *  @return True if both lists contain the same entries.
 */
bool operator==(
       std::list<std::string> const& left,
       commandsmember_struct const* right) {
  return (operator==(right, left));
}

/**
 *  Compare a commandsmember list with a list of string.
 *
 *  @param[in] left  First list.
 *  @param[in] right Second list.
 *
 *  @return False if both lists contain the same entries.
 */
bool operator!=(
       commandsmember_struct const* left,
       std::list<std::string> const& right) {
  return (!operator==(left, right));
}

/**
 *  Compare a list of string with a commandsmember list.
 *
 *  @param[in] left  First list.
 *  @param[in] right Second list.
 *
 *  @return False if both lists contain the same entries.
 */
bool operator!=(
       std::list<std::string> const& left,
       commandsmember_struct const* right) {
  return (!operator==(left, right));
}

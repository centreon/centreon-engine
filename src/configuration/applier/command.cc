/*
** Copyright 2011-2013,2017 Centreon
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
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/commands/connector.hh"
#include "com/centreon/engine/commands/forward.hh"
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
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
 *  Destructor.
 */
applier::command::~command() throw () {}

/**
 *  Add new command.
 *
 *  @param[in] obj  The new command to add into the monitoring engine.
 */
void applier::command::add_object(configuration::command const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new command '" << obj.command_name() << "'.";

  // Add command to the global configuration set.
  config->commands().insert(obj);

  if (obj.connector().empty()) {
    std::shared_ptr<commands::raw> raw{
      new commands::raw(obj.command_name(), obj.command_line(), &checks::checker::instance())};
    commands::command::commands[raw->get_name()] = raw;
  }
  else {
    connector_map::iterator found_con{commands::connector::connectors.find(obj.connector())};
    if (found_con != commands::connector::connectors.end() && found_con->second) {
      std::shared_ptr<commands::forward> forward{
        new commands::forward(obj.command_name(), obj.command_line(), *found_con->second)};
      commands::command::commands[forward->get_name()] = forward;
    }
    else
      throw engine_error() << "Could not register command '"
                           << obj.command_name() << "': unable to find '"
                           << obj.connector() << "'";
  }
}

/**
 *  @brief Expand command.
 *
 *  Command configuration objects do not need expansion. Therefore this
 *  method does nothing.
 *
 *  @param[in] s  Unused.
 */
void applier::command::expand_objects(configuration::state& s) {
  (void)s;
}

/**
 *  Modified command.
 *
 *  @param[in] obj The new command to modify into the monitoring engine.
 */
void applier::command::modify_object(
                         configuration::command const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying command '" << obj.command_name() << "'.";

  // Find old configuration.
  set_command::iterator it_cfg(config->commands_find(obj.key()));
  if (it_cfg == config->commands().end())
    throw (engine_error() << "Cannot modify non-existing "
           << "command '" << obj.command_name() << "'");

  // Find command object.
  command_map::iterator
    it_obj(commands::command::commands.find(obj.key()));
  if (it_obj == commands::command::commands.end())
    throw (engine_error() << "Could not modify non-existing "
           << "command object '" << obj.command_name() << "'");
  commands::command* c(it_obj->second.get());

  // Update the global configuration set.
  config->commands().erase(it_cfg);
  config->commands().insert(obj);

  // Modify command.
  if (c->get_command_line() != obj.command_line())
    c->set_command_line(obj.command_line());

  // Command will be temporarily removed from the command set but
  // will be added back right after with _create_command. This does
  // not create dangling pointers since commands::command object are
  // not referenced anywhere, only ::command objects are.
  commands::command::commands.erase(obj.command_name());
  if (obj.connector().empty()) {
    std::shared_ptr<commands::raw> raw{
      new commands::raw(obj.command_name(), obj.command_line(), &checks::checker::instance())};
    commands::command::commands[raw->get_name()] = raw;
  }
  else {
    connector_map::iterator found_con{commands::connector::connectors.find(obj.connector())};
    if (found_con != commands::connector::connectors.end() && found_con->second) {
      std::shared_ptr<commands::forward> forward{
        new commands::forward(obj.command_name(), obj.command_line(), *found_con->second)};
      commands::command::commands[forward->get_name()] = forward;
    }
    else
      throw engine_error() << "Could not register command '"
                           << obj.command_name() << "': unable to find '"
                           << obj.connector() << "'";
  }
  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_command_data(
    NEBTYPE_COMMAND_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    c,
    &tv);
}

/**
 *  Remove old command.
 *
 *  @param[in] obj The new command to remove from the monitoring engine.
 */
void applier::command::remove_object(
                         configuration::command const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing command '" << obj.command_name() << "'.";

  // Find command.
  std::unordered_map<std::string, std::shared_ptr<commands::command> >::iterator
    it(commands::command::commands.find(obj.key()));
  if (it != commands::command::commands.end()) {
    commands::command* cmd(it->second.get());

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_command_data(
      NEBTYPE_COMMAND_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      cmd,
      &tv);

    // Erase command (will effectively delete the object).
    commands::command::commands.erase(it);
  }
  else
    throw engine_error() << "Could not remove command '"
        << obj.key() << "': it does not exist";

  // Remove command from the global configuration set.
  config->commands().erase(obj);
}

/**
 *  @brief Resolve command.
 *
 *  This method will check for its connector's existence, if command is
 *  configured to use one.
 *
 *  @param[in] obj  Command object.
 */
void applier::command::resolve_object(
                         configuration::command const& obj) {
  if (!obj.connector().empty()) {
    connector_map::iterator found{
      commands::connector::connectors.find(obj.connector())};
    if(found == commands::connector::connectors.end() || !found->second)
      throw (engine_error() << "unknow command " << obj.connector());
  }
}

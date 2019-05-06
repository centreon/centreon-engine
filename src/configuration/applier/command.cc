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
 *  @param[in] obj  The new command to add into the monitoring engine.
 */
void applier::command::add_object(configuration::command const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new command '" << obj.command_name() << "'.";

  // Add command to the global configuration set.
  config->commands().insert(obj);

  // Create compatibility command.
  command_struct* c(add_command(
                      obj.command_name().c_str(),
                      obj.command_line().c_str()));
  if (!c)
    throw (engine_error() << "Could not register command '"
           << obj.command_name() << "'");

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
 *  @param[in] s  Unused.
 */
void applier::command::expand_objects(configuration::state& s) {
  (void)s;
  return ;
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
  umap<std::string, std::shared_ptr<command_struct> >::iterator
    it_obj(applier::state::instance().commands_find(obj.key()));
  if (it_obj == applier::state::instance().commands().end())
    throw (engine_error() << "Could not modify non-existing "
           << "command object '" << obj.command_name() << "'");
  command_struct* c(it_obj->second.get());

  // Update the global configuration set.
  config->commands().erase(it_cfg);
  config->commands().insert(obj);

  // Modify command.
  modify_if_different(c->command_line, obj.command_line().c_str());

  // Command will be temporarily removed from the command set but
  // will be added back right after with _create_command. This does
  // not create dangling pointers since commands::command object are
  // not referenced anywhere, only ::command objects are.
  commands::set::instance().remove_command(obj.command_name());
  _create_command(obj);

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_command_data(
    NEBTYPE_COMMAND_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    c,
    &tv);

  return ;
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
  umap<std::string, std::shared_ptr<command_struct> >::iterator
    it(applier::state::instance().commands_find(obj.key()));
  if (it != applier::state::instance().commands().end()) {
    command_struct* cmd(it->second.get());

    // Remove command from its list.
    unregister_object<command_struct>(&command_list, cmd);

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_command_data(
      NEBTYPE_COMMAND_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      cmd,
      &tv);

    // Erase command (will effectively delete the object).
    applier::state::instance().commands().erase(it);
  }

  // Remove command objects.
  commands::set::instance().remove_command(obj.command_name());

  // Remove command from the global configuration set.
  config->commands().erase(obj);

  return ;
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
  if (!obj.connector().empty())
    commands::set::instance().get_command(obj.connector());
  return ;
}

/**
 *  @brief Find real command object.
 *
 *  Create the commands::command object. This can be either a
 *  commands::raw object or a commands::forward object.
 *
 *  @param[in] obj  Command configuration object.
 */
void applier::command::_create_command(
                         configuration::command const& obj) {
  // Command set.
  commands::set& cmd_set(commands::set::instance());

  // Raw command.
  if (obj.connector().empty()) {
    std::shared_ptr<commands::command>
      cmd(new commands::raw(
                          obj.command_name(),
                          obj.command_line(),
                          &checks::checker::instance()));
    cmd_set.add_command(cmd);
  }
  // Connector command.
  else {
    std::shared_ptr<commands::command>
      cmd(new commands::forward(
                          obj.command_name(),
                          obj.command_line(),
                          *cmd_set.get_command(obj.connector())));
    cmd_set.add_command(cmd);
  }

  return ;
}

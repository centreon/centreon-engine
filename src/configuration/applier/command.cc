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

#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

static applier::command* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::command::apply(configuration::state const& config) {
  _diff(::config->commands(), config.commands());
}

/**
 *  Get the singleton instance of command applier.
 *
 *  @return Singleton instance.
 */
applier::command& applier::command::instance() {
  return (*_instance);
}

/**
 *  Load command applier singleton.
 */
void applier::command::load() {
  if (!_instance)
    _instance = new applier::command;
}

/**
 *  Unload command applier singleton.
 */
void applier::command::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::command::command() {

}

/**
 *  Destructor.
 */
applier::command::~command() throw () {

}

/**
 *  Add new command.
 *
 *  @param[in] obj The new command to add into the monitoring engine.
 */
void applier::command::_add_object(command_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new command '" << obj->command_name() << "'.";

  // Create command.
  shared_ptr<command_struct> c(new command_struct);
  memset(c.get(), 0, sizeof(*c));
  c->name = my_strdup(obj->command_name().c_str());
  c->command_line = my_strdup(obj->command_line().c_str());

  // Register command.
  c->next = command_list;
  applier::state::instance().commands()[obj->command_name()] = c;
  command_list = c.get();

  return ;
}

/**
 *  Modified command.
 *
 *  @param[in] obj The new command to modify into the monitoring engine.
 */
void applier::command::_modify_object(command_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying command '" << obj->command_name() << "'.";

  // Modify command.
  shared_ptr<command_struct>&
    c(applier::state::instance().commands()[obj->command_name()]);
  modify_if_different(c->command_line, obj->command_line().c_str());

  // XXX: todo.

  return ;
}

/**
 *  Remove old command.
 *
 *  @param[in] obj The new command to remove from the monitoring engine.
 */
void applier::command::_remove_object(command_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing command '" << obj->command_name() << "'.";

  // Unregister command.
  for (command_struct** cs(&command_list); *cs; cs = &(*cs)->next)
    if (!strcmp((*cs)->name, obj->command_name().c_str())) {
      (*cs) = (*cs)->next;
      break ;
    }
  applier::state::instance().commands().erase(obj->command_name());

  return ;
}

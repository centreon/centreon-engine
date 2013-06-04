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
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/misc/difference.hh"

using namespace com::centreon::engine::configuration;

static applier::command* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::command::apply(state const& config) {
  // list_command const& old_commands(::config->commands());
  // list_command const& new_commands(config.commands());

  // misc::difference<list_command> diff(old_commands, new_commands);
  // list_command const& added(diff.added());
  // list_command const& deleted(diff.deleted());
  // list_command const& modified(diff.modified());

  // _add_commands(added);
  // _remove_commands(deleted);
  // _modify_commands(modified);
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
 *  Add new commands.
 *
 *  @param[in] data All command to add into the monitoring engine.
 */
void applier::command::_add_commands(list_command const& data) {

}

/**
 *  Modified commands.
 *
 *  @param[in] data All command to modify into the monitoring engine.
 */
void applier::command::_modify_commands(list_command const& data) {

}

/**
 *  Remove old commands.
 *
 *  @param[in] data All command to remove from the monitoring engine.
 */
void applier::command::_remove_commands(list_command const& data) {

}

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

#include "com/centreon/engine/configuration/applier/hostgroup.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

static applier::hostgroup* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::hostgroup::apply(configuration::state const& config) {
  _diff(::config->hostgroups(), config.hostgroups());
}

/**
 *  Get the singleton instance of hostgroup applier.
 *
 *  @return Singleton instance.
 */
applier::hostgroup& applier::hostgroup::instance() {
  return (*_instance);
}

/**
 *  Load hostgroup applier singleton.
 */
void applier::hostgroup::load() {
  if (!_instance)
    _instance = new applier::hostgroup;
}

/**
 *  Unload hostgroup applier singleton.
 */
void applier::hostgroup::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::hostgroup::hostgroup() {

}

/**
 *  Destructor.
 */
applier::hostgroup::~hostgroup() throw () {

}

/**
 *  Add new hostgroup.
 *
 *  @param[in] obj The new hostgroup to add into the monitoring engine.
 */
void applier::hostgroup::_add_object(hostgroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new hostgroup '" << obj->hostgroup_name() << "'.";

}

/**
 *  Modified hostgroup.
 *
 *  @param[in] obj The new hostgroup to modify into the monitoring engine.
 */
void applier::hostgroup::_modify_object(hostgroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying hostgroup '" << obj->hostgroup_name() << "'.";

}

/**
 *  Remove old hostgroup.
 *
 *  @param[in] obj The new hostgroup to remove from the monitoring engine.
 */
void applier::hostgroup::_remove_object(hostgroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing hostgroup '" << obj->hostgroup_name() << "'.";

  // Unregister host.
  unregister_object<hostgroup_struct, &hostgroup_struct::group_name>(
    &hostgroup_list,
    obj->hostgroup_name().c_str());
  applier::state::instance().hostgroups().erase(obj->hostgroup_name());
}

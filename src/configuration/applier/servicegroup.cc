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

#include "com/centreon/engine/configuration/applier/servicegroup.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

static applier::servicegroup* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::servicegroup::apply(configuration::state const& config) {
  _diff(::config->servicegroups(), config.servicegroups());
}

/**
 *  Get the singleton instance of servicegroup applier.
 *
 *  @return Singleton instance.
 */
applier::servicegroup& applier::servicegroup::instance() {
  return (*_instance);
}

/**
 *  Load servicegroup applier singleton.
 */
void applier::servicegroup::load() {
  if (!_instance)
    _instance = new applier::servicegroup;
}

/**
 *  Unload servicegroup applier singleton.
 */
void applier::servicegroup::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::servicegroup::servicegroup() {

}

/**
 *  Destructor.
 */
applier::servicegroup::~servicegroup() throw () {

}

/**
 *  Add new servicegroup.
 *
 *  @param[in] obj The new servicegroup to add into the monitoring engine.
 */
void applier::servicegroup::_add_object(servicegroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new servicegroup '" << obj->servicegroup_name() << "'.";

}

/**
 *  Modified servicegroup.
 *
 *  @param[in] obj The new servicegroup to modify into the monitoring engine.
 */
void applier::servicegroup::_modify_object(servicegroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying servicegroup '" << obj->servicegroup_name() << "'.";

}

/**
 *  Remove old servicegroup.
 *
 *  @param[in] obj The new servicegroup to remove from the monitoring engine.
 */
void applier::servicegroup::_remove_object(servicegroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing servicegroup '" << obj->servicegroup_name() << "'.";

  // Unregister host.
  unregister_object<servicegroup_struct, &servicegroup_struct::group_name>(
    &servicegroup_list,
    obj->servicegroup_name().c_str());
  applier::state::instance().servicegroups().erase(obj->servicegroup_name());
}

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

#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

static applier::contactgroup* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::contactgroup::apply(state const& config) {
  _diff(::config->contactgroups(), config.contactgroups());
}

/**
 *  Get the singleton instance of contactgroup applier.
 *
 *  @return Singleton instance.
 */
applier::contactgroup& applier::contactgroup::instance() {
  return (*_instance);
}

/**
 *  Load contactgroup applier singleton.
 */
void applier::contactgroup::load() {
  if (!_instance)
    _instance = new applier::contactgroup;
}

/**
 *  Unload contactgroup applier singleton.
 */
void applier::contactgroup::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::contactgroup::contactgroup() {

}

/**
 *  Destructor.
 */
applier::contactgroup::~contactgroup() throw () {

}

/**
 *  Add new contactgroup.
 *
 *  @param[in] obj The new contactgroup to add into the monitoring engine.
 */
void applier::contactgroup::_add_object(contactgroup_ptr obj) {

}

/**
 *  Modified contactgroup.
 *
 *  @param[in] obj The new contactgroup to modify into the monitoring engine.
 */
void applier::contactgroup::_modify_object(contactgroup_ptr obj) {

}

/**
 *  Remove old contactgroup.
 *
 *  @param[in] obj The new contactgroup to remove from the monitoring engine.
 */
void applier::contactgroup::_remove_object(contactgroup_ptr obj) {

}

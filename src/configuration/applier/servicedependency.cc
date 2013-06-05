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

#include "com/centreon/engine/configuration/applier/servicedependency.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

static applier::servicedependency* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::servicedependency::apply(state const& config) {
  _diff(::config->servicedependencies(), config.servicedependencies());
}

/**
 *  Get the singleton instance of servicedependency applier.
 *
 *  @return Singleton instance.
 */
applier::servicedependency& applier::servicedependency::instance() {
  return (*_instance);
}

/**
 *  Load servicedependency applier singleton.
 */
void applier::servicedependency::load() {
  if (!_instance)
    _instance = new applier::servicedependency;
}

/**
 *  Unload servicedependency applier singleton.
 */
void applier::servicedependency::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::servicedependency::servicedependency() {

}

/**
 *  Destructor.
 */
applier::servicedependency::~servicedependency() throw () {

}

/**
 *  Add new servicedependency.
 *
 *  @param[in] obj The new servicedependency to add into the monitoring engine.
 */
void applier::servicedependency::_add_object(servicedependency_ptr obj) {

}

/**
 *  Modified servicedependency.
 *
 *  @param[in] obj The new servicedependency to modify into the monitoring engine.
 */
void applier::servicedependency::_modify_object(servicedependency_ptr obj) {

}

/**
 *  Remove old servicedependency.
 *
 *  @param[in] obj The new servicedependency to remove from the monitoring engine.
 */
void applier::servicedependency::_remove_object(servicedependency_ptr obj) {

}

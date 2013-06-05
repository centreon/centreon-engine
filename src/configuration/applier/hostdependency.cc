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

#include "com/centreon/engine/configuration/applier/hostdependency.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

static applier::hostdependency* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::hostdependency::apply(state const& config) {
  _diff(::config->hostdependencies(), config.hostdependencies());
}

/**
 *  Get the singleton instance of hostdependency applier.
 *
 *  @return Singleton instance.
 */
applier::hostdependency& applier::hostdependency::instance() {
  return (*_instance);
}

/**
 *  Load hostdependency applier singleton.
 */
void applier::hostdependency::load() {
  if (!_instance)
    _instance = new applier::hostdependency;
}

/**
 *  Unload hostdependency applier singleton.
 */
void applier::hostdependency::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::hostdependency::hostdependency() {

}

/**
 *  Destructor.
 */
applier::hostdependency::~hostdependency() throw () {

}

/**
 *  Add new hostdependency.
 *
 *  @param[in] obj The new hostdependency to add into the monitoring engine.
 */
void applier::hostdependency::_add_object(hostdependency_ptr obj) {

}

/**
 *  Modified hostdependency.
 *
 *  @param[in] obj The new hostdependency to modify into the monitoring engine.
 */
void applier::hostdependency::_modify_object(hostdependency_ptr obj) {

}

/**
 *  Remove old hostdependency.
 *
 *  @param[in] obj The new hostdependency to remove from the monitoring engine.
 */
void applier::hostdependency::_remove_object(hostdependency_ptr obj) {

}

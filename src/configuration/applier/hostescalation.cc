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

#include "com/centreon/engine/configuration/applier/hostescalation.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

static applier::hostescalation* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::hostescalation::apply(configuration::state const& config) {
  _diff(::config->hostescalations(), config.hostescalations());
}

/**
 *  Get the singleton instance of hostescalation applier.
 *
 *  @return Singleton instance.
 */
applier::hostescalation& applier::hostescalation::instance() {
  return (*_instance);
}

/**
 *  Load hostescalation applier singleton.
 */
void applier::hostescalation::load() {
  if (!_instance)
    _instance = new applier::hostescalation;
}

/**
 *  Unload hostescalation applier singleton.
 */
void applier::hostescalation::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::hostescalation::hostescalation() {

}

/**
 *  Destructor.
 */
applier::hostescalation::~hostescalation() throw () {

}

/**
 *  Add new hostescalation.
 *
 *  @param[in] obj The new hostescalation to add into the monitoring engine.
 */
void applier::hostescalation::_add_object(hostescalation_ptr obj) {

}

/**
 *  Modified hostescalation.
 *
 *  @param[in] obj The new hostescalation to modify into the monitoring engine.
 */
void applier::hostescalation::_modify_object(hostescalation_ptr obj) {

}

/**
 *  Remove old hostescalation.
 *
 *  @param[in] obj The new hostescalation to remove from the monitoring engine.
 */
void applier::hostescalation::_remove_object(hostescalation_ptr obj) {

}

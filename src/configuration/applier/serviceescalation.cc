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

#include "com/centreon/engine/configuration/applier/serviceescalation.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

static applier::serviceescalation* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::serviceescalation::apply(state const& config) {
  _diff(::config->serviceescalations(), config.serviceescalations());
}

/**
 *  Get the singleton instance of serviceescalation applier.
 *
 *  @return Singleton instance.
 */
applier::serviceescalation& applier::serviceescalation::instance() {
  return (*_instance);
}

/**
 *  Load serviceescalation applier singleton.
 */
void applier::serviceescalation::load() {
  if (!_instance)
    _instance = new applier::serviceescalation;
}

/**
 *  Unload serviceescalation applier singleton.
 */
void applier::serviceescalation::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::serviceescalation::serviceescalation() {

}

/**
 *  Destructor.
 */
applier::serviceescalation::~serviceescalation() throw () {

}

/**
 *  Add new serviceescalation.
 *
 *  @param[in] obj The new serviceescalation to add into the monitoring engine.
 */
void applier::serviceescalation::_add_object(serviceescalation_ptr obj) {

}

/**
 *  Modified serviceescalation.
 *
 *  @param[in] obj The new serviceescalation to modify into the monitoring engine.
 */
void applier::serviceescalation::_modify_object(serviceescalation_ptr obj) {

}

/**
 *  Remove old serviceescalation.
 *
 *  @param[in] obj The new serviceescalation to remove from the monitoring engine.
 */
void applier::serviceescalation::_remove_object(serviceescalation_ptr obj) {

}

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

#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

static applier::service* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::service::apply(state const& config) {
  _diff(::config->services(), config.services());
}

/**
 *  Get the singleton instance of service applier.
 *
 *  @return Singleton instance.
 */
applier::service& applier::service::instance() {
  return (*_instance);
}

/**
 *  Load service applier singleton.
 */
void applier::service::load() {
  if (!_instance)
    _instance = new applier::service;
}

/**
 *  Unload service applier singleton.
 */
void applier::service::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::service::service() {

}

/**
 *  Destructor.
 */
applier::service::~service() throw () {

}

/**
 *  Add new service.
 *
 *  @param[in] obj The new service to add into the monitoring engine.
 */
void applier::service::_add_object(service_ptr obj) {

}

/**
 *  Modified service.
 *
 *  @param[in] obj The new service to modify into the monitoring engine.
 */
void applier::service::_modify_object(service_ptr obj) {

}

/**
 *  Remove old service.
 *
 *  @param[in] obj The new service to remove from the monitoring engine.
 */
void applier::service::_remove_object(service_ptr obj) {

}

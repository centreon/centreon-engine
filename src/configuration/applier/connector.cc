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

#include "com/centreon/engine/configuration/applier/connector.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

static applier::connector* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::connector::apply(configuration::state const& config) {
  _diff(::config->connectors(), config.connectors());
}

/**
 *  Get the singleton instance of connector applier.
 *
 *  @return Singleton instance.
 */
applier::connector& applier::connector::instance() {
  return (*_instance);
}

/**
 *  Load connector applier singleton.
 */
void applier::connector::load() {
  if (!_instance)
    _instance = new applier::connector;
}

/**
 *  Unload connector applier singleton.
 */
void applier::connector::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::connector::connector() {

}

/**
 *  Destructor.
 */
applier::connector::~connector() throw () {

}

/**
 *  Add new connector.
 *
 *  @param[in] obj The new connector to add into the monitoring engine.
 */
void applier::connector::_add_object(connector_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new connector '" << obj->connector_name() << "'.";

}

/**
 *  Modified connector.
 *
 *  @param[in] obj The new connector to modify into the monitoring engine.
 */
void applier::connector::_modify_object(connector_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying connector '" << obj->connector_name() << "'.";

}

/**
 *  Remove old connector.
 *
 *  @param[in] obj The new connector to remove from the monitoring engine.
 */
void applier::connector::_remove_object(connector_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing connector '" << obj->connector_name() << "'.";

}

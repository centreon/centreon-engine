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

/**
 *  Default constructor.
 */
applier::connector::connector() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::connector::connector(applier::connector const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::connector::~connector() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::connector& applier::connector::operator=(
                      applier::connector const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new connector.
 *
 *  @param[in] obj The new connector to add into the monitoring engine.
 */
void applier::connector::add_object(connector_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new connector '" << obj->connector_name() << "'.";

  // XXX

  return ;
}

/**
 *  Modified connector.
 *
 *  @param[in] obj The new connector to modify into the monitoring engine.
 */
void applier::connector::modify_object(connector_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying connector '" << obj->connector_name() << "'.";

  // XXX

  return ;
}

/**
 *  Remove old connector.
 *
 *  @param[in] obj The new connector to remove from the monitoring engine.
 */
void applier::connector::remove_object(connector_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing connector '" << obj->connector_name() << "'.";

  // XXX

  return ;
}

/**
 *  Resolve a connector.
 *
 *  @param[in] obj Connector object.
 */
void applier::connector::resolve_object(connector_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving connector '" << obj->connector_name() << "'.";

  // XXX

  return ;
}

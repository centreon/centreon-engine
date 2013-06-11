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
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

// XXX : update the event_list_low and event_list_high

/**
 *  Default constructor.
 */
applier::service::service() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::service::service(applier::service const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::service::~service() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::service& applier::service::operator=(
                                      applier::service const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new service.
 *
 *  @param[in] obj The new service to add into the monitoring engine.
 */
void applier::service::add_object(service_ptr obj) {
  // Logging.
  // logger(logging::dbg_config, logging::more)
  //   << "Creating new service '" << obj->service_description()
  //   << "' of host '" << obj->host_name() << "'.";

  // XXX

  return ;
}

/**
 *  Modified service.
 *
 *  @param[in] obj The new service to modify into the monitoring engine.
 */
void applier::service::modify_object(service_ptr obj) {
  // Logging.
  // logger(logging::dbg_config, logging::more)
  //   << "Modifying service '" << obj->service_description()
  //   << "' of host '" << obj->host_name() << "'.";

  // XXX

  return ;
}

/**
 *  Remove old service.
 *
 *  @param[in] obj The new service to remove from the monitoring engine.
 */
void applier::service::remove_object(service_ptr obj) {
  // Logging.
  // logger(logging::dbg_config, logging::more)
  //   << "Removing service '" << obj->service_description()
  //   << "' of host '" << obj->host_name() << "'.";

  // XXX

  return ;
}

/**
 *  Resolve a service.
 *
 *  @param[in] obj Service object.
 */
void applier::service::resolve_object(service_ptr obj) {
  // Logging.
  // logger(logging::dbg_config, logging::more)
  //   << "Resolving service '" << obj->service_description()
  //   << "' of host '" << obj->host_name () << "'.";

  // XXX

  return ;
}

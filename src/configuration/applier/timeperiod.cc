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

#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::timeperiod::timeperiod() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::timeperiod::timeperiod(applier::timeperiod const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::timeperiod::~timeperiod() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 */
applier::timeperiod& applier::timeperiod::operator=(
                       applier::timeperiod const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new timeperiod.
 *
 *  @param[in] obj The new timeperiod to add into the monitoring engine.
 */
void applier::timeperiod::add_object(timeperiod_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new timeperiod '" << obj->timeperiod_name() << "'.";

  // XXX

  return ;
}

/**
 *  Modified timeperiod.
 *
 *  @param[in] obj The new timeperiod to modify into the monitoring engine.
 */
void applier::timeperiod::modify_object(timeperiod_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying timeperiod '" << obj->timeperiod_name() << "'.";

  // XXX

  return ;
}

/**
 *  Remove old timeperiod.
 *
 *  @param[in] obj The new timeperiod to remove from the monitoring engine.
 */
void applier::timeperiod::remove_object(timeperiod_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing timeperiod '" << obj->timeperiod_name() << "'.";

  // Unregister host.
  unregister_object<timeperiod_struct, &timeperiod_struct::name>(
    &timeperiod_list,
    obj->timeperiod_name().c_str());
  applier::state::instance().timeperiods().erase(obj->timeperiod_name());

  return ;
}

/**
 *  Resolve a timeperiod.
 *
 *  @param[in] obj Timeperiod object.
 */
void applier::timeperiod::resolve_object(timeperiod_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving timeperiod '" << obj->timeperiod_name() << "'.";
  return ;
}

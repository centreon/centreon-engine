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

#include "com/centreon/engine/configuration/applier/servicegroup.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::servicegroup::servicegroup() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::servicegroup::servicegroup(
                         applier::servicegroup const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::servicegroup::~servicegroup() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::servicegroup& applier::servicegroup::operator=(
                         applier::servicegroup const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new servicegroup.
 *
 *  @param[in] obj The new servicegroup to add into the monitoring engine.
 */
void applier::servicegroup::add_object(servicegroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new servicegroup '" << obj->servicegroup_name() << "'.";

  // XXX

  return ;
}

/**
 *  Modify servicegroup.
 *
 *  @param[in] obj The new servicegroup to modify into the monitoring engine.
 */
void applier::servicegroup::modify_object(servicegroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying servicegroup '" << obj->servicegroup_name() << "'.";

  // XXX

  return ;
}

/**
 *  Remove old servicegroup.
 *
 *  @param[in] obj The new servicegroup to remove from the monitoring engine.
 */
void applier::servicegroup::remove_object(servicegroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing servicegroup '" << obj->servicegroup_name() << "'.";

  // Unregister host.
  unregister_object<servicegroup_struct, &servicegroup_struct::group_name>(
    &servicegroup_list,
    obj->servicegroup_name().c_str());
  applier::state::instance().servicegroups().erase(obj->servicegroup_name());

  return ;
}

/**
 *  Resolve a servicegroup.
 *
 *  @param[in] obj Servicegroup object.
 */
void applier::servicegroup::resolve_object(servicegroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving servicegroup '" << obj->servicegroup_name() << "'.";

  // XXX

  return ;
}

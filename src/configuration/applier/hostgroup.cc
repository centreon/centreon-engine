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

#include "com/centreon/engine/configuration/applier/hostgroup.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/hostgroup.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::hostgroup::hostgroup() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::hostgroup::hostgroup(applier::hostgroup const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::hostgroup::~hostgroup() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::hostgroup& applier::hostgroup::operator=(
                      applier::hostgroup const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new hostgroup.
 *
 *  @param[in] obj The new hostgroup to add into the monitoring engine.
 */
void applier::hostgroup::add_object(hostgroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new hostgroup '" << obj->hostgroup_name() << "'.";

  // Create hostgroup.
  shared_ptr<hostgroup_struct>
    hg(
      add_hostgroup(
        obj->hostgroup_name().c_str(),
        NULL_IF_EMPTY(obj->alias()),
        NULL_IF_EMPTY(obj->notes()),
        NULL_IF_EMPTY(obj->notes_url()),
        NULL_IF_EMPTY(obj->action_url())),
      &deleter::hostgroup);
  if (!hg.get())
    throw (engine_error() << "Error: Could not register hostgroup '"
           << obj->hostgroup_name() << "'.");

  // XXX

  // Register hostgroup.
  hg->next = hostgroup_list;
  applier::state::instance().hostgroups()[obj->hostgroup_name()] = hg;
  hostgroup_list = hg.get();

  return ;
}

/**
 *  Modified hostgroup.
 *
 *  @param[in] obj The new hostgroup to modify into the monitoring engine.
 */
void applier::hostgroup::modify_object(hostgroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying hostgroup '" << obj->hostgroup_name() << "'.";

  // XXX

  return ;
}

/**
 *  Remove old hostgroup.
 *
 *  @param[in] obj The new hostgroup to remove from the monitoring engine.
 */
void applier::hostgroup::remove_object(hostgroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing hostgroup '" << obj->hostgroup_name() << "'.";

  // Unregister host.
  unregister_object<hostgroup_struct, &hostgroup_struct::group_name>(
    &hostgroup_list,
    obj->hostgroup_name().c_str());
  applier::state::instance().hostgroups().erase(obj->hostgroup_name());

  return ;
}

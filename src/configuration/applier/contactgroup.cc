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

#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/member.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/shared.hh"

using namespace com::centreon::engine::configuration;

static applier::contactgroup* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::contactgroup::apply(configuration::state const& config) {
  _diff(::config->contactgroups(), config.contactgroups());
}

/**
 *  Get the singleton instance of contactgroup applier.
 *
 *  @return Singleton instance.
 */
applier::contactgroup& applier::contactgroup::instance() {
  return (*_instance);
}

/**
 *  Load contactgroup applier singleton.
 */
void applier::contactgroup::load() {
  if (!_instance)
    _instance = new applier::contactgroup;
}

/**
 *  Unload contactgroup applier singleton.
 */
void applier::contactgroup::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::contactgroup::contactgroup() {

}

/**
 *  Destructor.
 */
applier::contactgroup::~contactgroup() throw () {

}

/**
 *  Add new contactgroup.
 *
 *  @param[in] obj The new contactgroup to add into the monitoring engine.
 */
void applier::contactgroup::_add_object(contactgroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new contactgroup '"
    << obj->contactgroup_name() << "'.";

  // Create contactgroup.
  shared_ptr<contactgroup_struct> g(new contactgroup_struct);
  memset(g.get(), 0, sizeof(*g));

  g->group_name = my_strdup(obj->contactgroup_name().c_str());
  g->alias = my_strdup(obj->alias().c_str());
  applier::add_members<contact_struct, contactsmember_struct>(
    applier::state::instance().contacts(),
    obj->members(),
    g->members);
  // XXX: todo fill contactgroup.

  // Register contact.
  g->next = contactgroup_list;
  applier::state::instance().contactgroups()[obj->contactgroup_name()] = g;
  contactgroup_list = g.get();
}

/**
 *  Modified contactgroup.
 *
 *  @param[in] obj The new contactgroup to modify into the monitoring engine.
 */
void applier::contactgroup::_modify_object(contactgroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying contactgroup '" << obj->contactgroup_name() << "'.";

  // // Modify command.
  // shared_ptr<contactgroup_struct>&
  //   g(applier::state::instance().contactgroups()[obj->contactgroup_name()]);
  // modify_if_different(g->alias, obj->alias().c_str());
  // if (applier::members_has_change<contactsmember_struct, &contactsmember_struct::contact_name>(obj->members(), g->members))
  //   applier::update_members(applier::state::instance().contacts(), obj->members(), g->members);
}

/**
 *  Remove old contactgroup.
 *
 *  @param[in] obj The new contactgroup to remove from the monitoring engine.
 */
void applier::contactgroup::_remove_object(contactgroup_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing contactgroup '" << obj->contactgroup_name() << "'.";

  // Unregister contactgroup.
  unregister_object<contactgroup_struct, &contactgroup_struct::group_name>(
    &contactgroup_list,
    obj->contactgroup_name().c_str());
  applier::state::instance().contactgroups().erase(obj->contactgroup_name());
}

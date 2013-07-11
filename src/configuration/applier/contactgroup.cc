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

#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/member.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/shared.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::contactgroup::contactgroup() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::contactgroup::contactgroup(
                         applier::contactgroup const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::contactgroup::~contactgroup() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::contactgroup& applier::contactgroup::operator=(
                         applier::contactgroup const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new contactgroup.
 *
 *  @param[in] obj The new contactgroup to add into the monitoring engine.
 */
void applier::contactgroup::add_object(
                              shared_ptr<configuration::contactgroup> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new contactgroup '"
    << obj->contactgroup_name() << "'.";

  // Add contact group to the global configuration set.
  config->contactgroups().insert(obj);

  // Create contact group.
  contactgroup_struct* cg(add_contactgroup(
                            obj->contactgroup_name().c_str(),
                            NULL_IF_EMPTY(obj->alias())));
  if (!cg)
    throw (engine_error() << "Error: Could not register contact group '"
           << obj->contactgroup_name() << "'.");

  // Apply resolved contacts on contactgroup.
  for (set_string::const_iterator
         it(obj->resolved_members().begin()),
         end(obj->resolved_members().end());
       it != end;
       ++it)
    if (!add_contact_to_contactgroup(cg, it->c_str()))
      throw (engine_error() << "Error: Could not add contact '" << *it
             << "' to contact group '" << obj->contactgroup_name()
             << "'.");

  return ;
}

/**
 *  Expand a contactgroup.
 *
 *  @param[in,out] obj Object to expand.
 *  @param[in]     s   State being applied.
 */
void applier::contactgroup::expand_object(
                              shared_ptr<configuration::contactgroup> obj,
                              configuration::state& s) {
  // Resolve group.
  _resolve_members(obj, s);

  // We do not need to reinsert the object in the set, as
  // _resolve_members() only affects contactgroup::resolved_members()
  // and contactgroup::is_resolved() that are not used for the set
  // ordering.

  return ;
}

/**
 *  Modified contactgroup.
 *
 *  @param[in] obj The new contactgroup to modify into the monitoring engine.
 */
void applier::contactgroup::modify_object(
                              shared_ptr<configuration::contactgroup> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying contactgroup '" << obj->contactgroup_name() << "'.";

  // // Modify contact group.
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
void applier::contactgroup::remove_object(
                              shared_ptr<configuration::contactgroup> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing contactgroup '" << obj->contactgroup_name() << "'.";

  // Find contact group.
  umultimap<std::string, shared_ptr<contactgroup_struct> >::iterator
    it(applier::state::instance().contactgroups_find(obj->key()));
  if (it != applier::state::instance().contactgroups().end()) {
    // Remove contact group from its list.
    unregister_object<contactgroup_struct>(
      &contactgroup_list,
      it->second.get());

    // Remove contact group (this will effectively delete the object).
    applier::state::instance().contactgroups().erase(it);
  }

  // Remove contact group from the global configuration set.
  config->contactgroups().erase(obj);

  return ;
}

/**
 *  Resolve a contact group.
 *
 *  @param[in] obj Contact group object.
 */
void applier::contactgroup::resolve_object(
                              shared_ptr<configuration::contactgroup> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving contact group '" << obj->contactgroup_name() << "'.";

  // Find contact group.
  umap<std::string, shared_ptr<contactgroup_struct> >::iterator
    it(applier::state::instance().contactgroups_find(obj->key()));
  if (applier::state::instance().contactgroups().end() == it)
    throw (engine_error() << "Error: Cannot resolve non-existing "
           << "contact group '" << obj->contactgroup_name() << "'.");

  // Resolve contact group.
  if (!check_contactgroup(it->second.get(), NULL, NULL))
    throw (engine_error() << "Error: Cannot resolve contact group "
           << obj->contactgroup_name() << "'.");

  return ;
}

/**
 *  Resolve members of a contact group.
 *
 *  @param[in,out] obj Contactgroup object.
 *  @param[in]     s   Configuration being applied.
 */
void applier::contactgroup::_resolve_members(
                              shared_ptr<configuration::contactgroup> obj,
                              configuration::state& s) {
  // Only process if contactgroup has not been resolved already.
  if (!obj->is_resolved()) {
    // Logging.
    logger(logging::dbg_config, logging::more)
      << "Resolving members of contact group '"
      << obj->contactgroup_name() << "'.";

    // Mark object as resolved.
    obj->set_resolved(true);

    // Add base members.
    for (list_string::const_iterator
           it(obj->members().begin()),
           end(obj->members().end());
         it != end;
         ++it)
      obj->resolved_members().insert(*it);

    // Add contactgroup members.
    for (list_string::const_iterator
           it(obj->contactgroup_members().begin()),
           end(obj->contactgroup_members().end());
         it != end;
         ++it) {
      // Find contactgroup entry.
      set_contactgroup::iterator
        it2(s.contactgroups().begin()),
        end2(s.contactgroups().end());
      while (it2 != end2) {
        if ((*it2)->contactgroup_name() == *it)
          break ;
        ++it2;
      }
      if (it2 == s.contactgroups().end())
        throw (engine_error()
               << "Error: Could not add non-existing contact group member '"
               << *it << "' to contactgroup '"
               << obj->contactgroup_name() << "'.");

      // Resolve contactgroup member.
      _resolve_members(*it2, s);

      // Add contactgroup member members to members.
      for (set_string::const_iterator
             it3((*it2)->resolved_members().begin()),
             end3((*it2)->resolved_members().end());
           it3 != end3;
           ++it3)
        obj->resolved_members().insert(*it3);
    }
  }

  return ;
}

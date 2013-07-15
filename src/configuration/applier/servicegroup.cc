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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/servicegroup.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/servicesmember.hh"
#include "com/centreon/engine/error.hh"
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
void applier::servicegroup::add_object(
                              shared_ptr<configuration::servicegroup> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new servicegroup '" << obj->servicegroup_name() << "'.";

  // Add service group to the global configuration set.
  config->servicegroups().insert(obj);

  // Create servicegroup.
  servicegroup_struct* sg(add_servicegroup(
                            obj->servicegroup_name().c_str(),
                            NULL_IF_EMPTY(obj->alias()),
                            NULL_IF_EMPTY(obj->notes()),
                            NULL_IF_EMPTY(obj->notes_url()),
                            NULL_IF_EMPTY(obj->action_url())));
  if (!sg)
    throw (engine_error() << "Error: Could not register service group '"
           << obj->servicegroup_name() << "'.");

  // Apply resolved services on servicegroup.
  for (set_pair_string::const_iterator
         it(obj->resolved_members().begin()),
         end(obj->resolved_members().end());
       it != end;
       ++it)
    if (!add_service_to_servicegroup(
           sg,
           it->first.c_str(),
           it->second.c_str()))
      throw (engine_error() << "Error: Could not add service member '"
             << it->second << "' of host '" << it->first
             << "' to service group '" << obj->servicegroup_name()
             << "'.");

  return ;
}

/**
 *  Expand a servicegroup.
 *
 *  @param[in,out] obj Object to expand.
 *  @param[in]     s   State being applied.
 */
void applier::servicegroup::expand_object(
                              shared_ptr<configuration::servicegroup> obj,
                              configuration::state& s) {
  // Resolve group.
  _resolve_members(obj, s);

  // We do not need to reinsert the object in the set, as
  // _resolve_members() only affects servicegroup::resolved_members()
  // and servicegroup::is_resolved() that are not used for the set
  // ordering.

  return ;
}

/**
 *  Modify servicegroup.
 *
 *  @param[in] obj The new servicegroup to modify into the monitoring engine.
 */
void applier::servicegroup::modify_object(
                              shared_ptr<configuration::servicegroup> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying servicegroup '" << obj->servicegroup_name() << "'.";

  // Find old configuration.
  set_servicegroup::iterator
    it_cfg(config->servicegroups_find(obj->key()));
  if (it_cfg == config->servicegroups().end())
    throw (engine_error() << "Error: Could not modify non-existing "
           << "service group '" << obj->servicegroup_name() << "'.");

  // Find service group object.
  umap<std::string, shared_ptr<servicegroup_struct> >::iterator
    it_obj(applier::state::instance().servicegroups_find(obj->key()));
  if (it_obj == applier::state::instance().servicegroups().end())
    throw (engine_error() << "Error: Could not modify non-existing "
           << "service group object '" << obj->servicegroup_name()
           << "'.");
  servicegroup_struct* sg(it_obj->second.get());

  // Update the global configuration set.
  shared_ptr<configuration::servicegroup> old_cfg(*it_cfg);
  config->servicegroups().insert(obj);
  config->servicegroups().erase(it_cfg);

  // Modify properties.
  modify_if_different(
    sg->action_url,
    NULL_IF_EMPTY(obj->action_url()));
  modify_if_different(
    sg->alias,
    NULL_IF_EMPTY(obj->alias()));
  modify_if_different(
    sg->notes,
    NULL_IF_EMPTY(obj->notes()));
  modify_if_different(
    sg->notes_url,
    NULL_IF_EMPTY(obj->notes_url()));

  // Were members modified ?
  if (obj->members() != old_cfg->members()) {
    // Delete all old service group members.
    for (servicesmember* m((*it_obj).second->members); m;) {
      servicesmember* to_delete(m);
      m = m->next;
      deleter::servicesmember(to_delete);
    }
    (*it_obj).second->members = NULL;

    // Create new service group members.
    for (set_pair_string::const_iterator
           it(obj->resolved_members().begin()),
           end(obj->resolved_members().end());
         it != end;
         ++it)
      if (!add_service_to_servicegroup(
             sg,
             it->first.c_str(),
             it->second.c_str()))
        throw (engine_error() << "Error: Could not add service member '"
               << it->second << "' of host '" << it->first
               << "' to service group '" << obj->servicegroup_name()
               << "'.");
  }

  return ;
}

/**
 *  Remove old servicegroup.
 *
 *  @param[in] obj The new servicegroup to remove from the monitoring engine.
 */
void applier::servicegroup::remove_object(
                              shared_ptr<configuration::servicegroup> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing servicegroup '" << obj->servicegroup_name() << "'.";

  // Find service group.
  umap<std::string, shared_ptr<servicegroup_struct> >::iterator
    it(applier::state::instance().servicegroups_find(obj->key()));
  if (it != applier::state::instance().servicegroups().end()) {
    servicegroup_struct* grp(it->second.get());

    // Remove service dependency from its list.
    unregister_object<servicegroup_struct>(&servicegroup_list, grp);

    // XXX: call unregister broker members.

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group(
      NEBTYPE_SERVICEGROUP_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      grp,
      &tv);

    // Erase service group object (will effectively delete the object).
    applier::state::instance().servicegroups().erase(it);
  }

  // Remove service group from the global configuration state.
  config->servicegroups().erase(obj);

  return ;
}

/**
 *  Resolve a servicegroup.
 *
 *  @param[in,out] obj Servicegroup object.
 */
void applier::servicegroup::resolve_object(
                              shared_ptr<configuration::servicegroup> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing service group '" << obj->servicegroup_name() << "'.";

  // Find service group.
  umap<std::string, shared_ptr<servicegroup_struct> >::const_iterator
    it(applier::state::instance().servicegroups_find(obj->key()));
  if (applier::state::instance().servicegroups().end() == it)
    throw (engine_error() << "Error: Cannot resolve non-existing "
           << "service group '" << obj->servicegroup_name() << "'.");

  // Resolve service group.
  if (!check_servicegroup(it->second.get(), NULL, NULL))
    throw (engine_error() << "Error: Cannot resolve service group '"
           << obj->servicegroup_name() << "'.");

  return ;
}

/**
 *  Resolve members of a service group.
 *
 *  @param[in,out] obj Service group object.
 *  @param[in]     s   Configuration being applied.
 */
void applier::servicegroup::_resolve_members(
                              shared_ptr<configuration::servicegroup> obj,
                              configuration::state& s) {
  // Only process if servicegroup has not been resolved already.
  if (!obj->is_resolved()) {
    // Logging.
    logger(logging::dbg_config, logging::more)
      << "Resolving members of service group '"
      << obj->servicegroup_name() << "'.";

    // Mark object as resolved.
    obj->set_resolved(true);

    // Add base members.
    for (list_string::const_iterator
           it(obj->members().begin()),
           end(obj->members().end());
         it != end;
         ++it) {
      list_string::const_iterator it_prev(it++);
      if (it == end)
        throw (engine_error() << "Error: members of service group '"
               << obj->servicegroup_name()
               << "' were not specified as a list of host-service pairs"
               << " (host1,service1,host2,service2,...)");
      obj->resolved_members().insert(std::make_pair(*it_prev, *it));
    }

    // Add servicegroup members.
    for (list_string::const_iterator
           it(obj->servicegroup_members().begin()),
           end(obj->servicegroup_members().end());
         it != end;
         ++it) {
      // Find servicegroup entry.
      set_servicegroup::iterator
        it2(s.servicegroups().begin()),
        end2(s.servicegroups().end());
      while (it2 != end2) {
        if ((*it2)->servicegroup_name() == *it)
          break ;
        ++it2;
      }
      if (it2 == s.servicegroups().end())
        throw (engine_error()
               << "Error: Could not add non-existing servicegroup member '"
               << *it << "' to servicegroup '"
               << obj->servicegroup_name() << "'.");

      // Resolve servicegroup member.
      _resolve_members(*it2, s);

      // Add servicegroup member members to members.
      for (set_pair_string::const_iterator
             it3((*it2)->resolved_members().begin()),
             end3((*it2)->resolved_members().end());
           it3 != end3;
           ++it3)
        obj->resolved_members().insert(*it3);
    }
  }

  return ;
}

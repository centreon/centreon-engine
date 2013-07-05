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

#include "com/centreon/engine/configuration/applier/hostescalation.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::hostescalation::hostescalation() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::hostescalation::hostescalation(
                           applier::hostescalation const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::hostescalation::~hostescalation() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::hostescalation& applier::hostescalation::operator=(
                           applier::hostescalation const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new host escalation.
 *
 *  @param[in] obj The new host escalation to add into the monitoring
 *                 engine.
 */
void applier::hostescalation::add_object(
                                shared_ptr<configuration::hostescalation> obj) {
  // Check host escalation.
  if ((obj->hosts().size() != 1) || !obj->hostgroups().empty())
    throw (engine_error() << "Error: Could not create host escalation "
           << "with multiple hosts / host groups.");

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new escalation for host '"
    << obj->hosts().front() << "'.";

  // Add escalation to the global configuration set.
  config->hostescalations().insert(obj);

  // Create host escalation.
  hostescalation_struct*
    he(add_host_escalation(
         obj->hosts().front().c_str(),
         obj->first_notification(),
         obj->last_notification(),
         obj->notification_interval(),
         NULL_IF_EMPTY(obj->escalation_period()),
         static_cast<bool>(
           obj->escalation_options()
           & configuration::hostescalation::down),
         static_cast<bool>(
           obj->escalation_options()
           & configuration::hostescalation::unreachable),
         static_cast<bool>(
           obj->escalation_options()
           & configuration::hostescalation::recovery)));
  if (!he)
    throw (engine_error() << "Error: Could not create escalation "
           << "on host '" << obj->hosts().front() << "'.");

  // Unique contacts.
  std::set<std::string> contacts;
  for (std::list<std::string>::const_iterator
         it(obj->contacts().begin()),
         end(obj->contacts().end());
       it != end;
       ++it)
    contacts.insert(*it);

  // Add contacts to host escalation.
  for (std::set<std::string>::const_iterator
         it(contacts.begin()),
         end(contacts.end());
       it != end;
       ++it)
    if (!add_contact_to_host_escalation(he, it->c_str()))
      throw (engine_error() << "Error: Could not add contact '"
             << *it << "' on escalation of host '"
             << obj->hosts().front() << "'.");

  // Unique contact groups.
  std::set<std::string> contact_groups;
  for (std::list<std::string>::const_iterator
         it(obj->contactgroups().begin()),
         end(obj->contactgroups().end());
       it != end;
       ++it)
    contact_groups.insert(*it);

  // Add contact groups to host escalation.
  for (std::set<std::string>::const_iterator
         it(contact_groups.begin()),
         end(contact_groups.end());
       it != end;
       ++it)
    if (!add_contactgroup_to_host_escalation(he, it->c_str()))
      throw (engine_error() << "Error: Could not add contact group '"
             << *it << "' on escalation of host '"
             << obj->hosts().front() << "'.");

  return ;
}

/**
 *  Expand a host escalation.
 *
 *  @param[in]     obj Host escalation object.
 *  @param[in,out] s   Configuration being applied.
 */
void applier::hostescalation::expand_object(
                                shared_ptr<configuration::hostescalation> obj,
                                configuration::state& s) {
  // Inherits special vars.
  if ((obj->hosts().size() == 1) && obj->hostgroups().empty())
    _inherits_special_vars(obj, s);
  // Expand host escalation.
  else {
    // Expanded hosts.
    std::set<std::string> expanded_hosts;
    _expand_hosts(
      obj->hosts(),
      obj->hostgroups(),
      s,
      expanded_hosts);

    // Remove current host escalation.
    s.hostescalations().erase(obj);

    // Browse all hosts.
    for (std::set<std::string>::const_iterator
           it(expanded_hosts.begin()),
           end(expanded_hosts.end());
         it != end;
         ++it) {
      shared_ptr<configuration::hostescalation>
        hesc(new configuration::hostescalation(*obj));
      hesc->hostgroups().clear();
      hesc->hosts().clear();
      hesc->hosts().push_back(*it);

      // Insert new host escalation and expand it.
      s.hostescalations().insert(hesc);
      expand_object(hesc, s);
    }
  }

  return ;
}

/**
 *  @brief Modify host escalation.
 *
 *  Host escalations cannot be defined with anything else than their
 *  full content. Therefore no modification can occur.
 *
 *  @param[in] obj Unused.
 */
void applier::hostescalation::modify_object(
                                shared_ptr<configuration::hostescalation> obj) {
  (void)obj;
  throw (engine_error() << "Error: Could not modify a host escalation: "
         << "host escalation objects can only be added or removed, "
         << "this is likely a software bug that you should report to "
         << "Centreon Engine developers");
  return ;
}

/**
 *  Remove old hostescalation.
 *
 *  @param[in] obj The new hostescalation to remove from the monitoring engine.
 */
void applier::hostescalation::remove_object(
                                shared_ptr<configuration::hostescalation> obj) {
  // XXX
}

/**
 *  Resolve a hostescalation.
 *
 *  @param[in] obj Hostescalation object.
 */
void applier::hostescalation::resolve_object(
                                shared_ptr<configuration::hostescalation> obj) {
  // XXX
}

/**
 *  Expand hosts.
 *
 *  @param[in]     hosts      Host list.
 *  @param[in]     hostgroups Host group list.
 *  @param[in,out] s          Configuration being applied.
 *  @param[out]    expanded   Expanded hosts.
 */
void applier::hostescalation::_expand_hosts(
                                std::list<std::string> const& hosts,
                                std::list<std::string> const& hostgroups,
                                configuration::state const& s,
                                std::set<std::string>& expanded) {
  // Copy hosts.
  for (std::list<std::string>::const_iterator
         it(hosts.begin()),
         end(hosts.end());
       it != end;
       ++it)
    expanded.insert(*it);

  // Browse host groups.
  for (std::list<std::string>::const_iterator
         it(hostgroups.begin()),
         end(hostgroups.end());
       it != end;
       ++it) {
    // Find host group.
    set_hostgroup::const_iterator
      it_group(s.hostgroups().begin()),
      end_group(s.hostgroups().end());
    while (it_group != end_group) {
      if ((*it_group)->hostgroup_name() == *it)
        break ;
      ++it_group;
    }
    if (it_group == end_group)
      throw (engine_error()
             << "Error: Could not expand non-existing host group '"
             << *it << "'.");

    // Add host group members.
    for (std::set<std::string>::const_iterator
           it_member((*it_group)->resolved_members().begin()),
           end_member((*it_group)->resolved_members().end());
         it_member != end_member;
         ++it_member)
      expanded.insert(*it_member);
  }

  return ;
}

/**
 *  Inherits special variables from the host.
 *
 *  @param[in,out] obj Host escalation object.
 *  @param[in,out] s   Configuration state.
 */
void applier::hostescalation::_inherits_special_vars(
                                shared_ptr<configuration::hostescalation> obj,
                                configuration::state& s) {
  // Detect if any special variable has not been defined.
  if (!obj->contacts_defined()
      || !obj->contactgroups_defined()
      || !obj->notification_interval_defined()
      || !obj->escalation_period_defined()) {
    // Remove host escalation from state. It will be modified and
    // reinserted at the end of the method.
    s.hostescalations().erase(obj);

    // Find host.
    std::set<shared_ptr<configuration::host> >::const_iterator
      it(s.hosts().begin()),
      end(s.hosts().end());
    while (it != end) {
      if ((*it)->host_name() == obj->hosts().front())
        break ;
      ++it;
    }
    if (it == end)
      throw (engine_error()
             << "Error: Could not inherit special variables from host '"
             << obj->hosts().front() << "': host does not exist.");

    // Inherits variables.
    if (!obj->contacts_defined())
      obj->contacts() = (*it)->contacts();
    if (!obj->contactgroups_defined())
      obj->contactgroups() = (*it)->contactgroups();
    if (!obj->notification_interval_defined())
      obj->notification_interval((*it)->notification_interval());
    if (!obj->escalation_period_defined())
      obj->escalation_period((*it)->notification_period());

    // Reinsert host escalation.
    s.hostescalations().insert(obj);
  }

  return ;
}

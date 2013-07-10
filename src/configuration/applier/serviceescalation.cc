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

#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/serviceescalation.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::serviceescalation::serviceescalation() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::serviceescalation::serviceescalation(
                              applier::serviceescalation const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::serviceescalation::~serviceescalation() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::serviceescalation& applier::serviceescalation::operator=(
                              applier::serviceescalation const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new service escalation.
 *
 *  @param[in] obj The new service escalation to add into the monitoring
 *                 engine.
 */
void applier::serviceescalation::add_object(
                                   shared_ptr<configuration::serviceescalation> obj) {
  // Check service escalation.
  if ((obj->hosts().size() != 1)
      || !obj->hostgroups().empty()
      || (obj->service_description().size() != 1)
      || !obj->servicegroups().empty())
    throw (engine_error() << "Error: Could not create service "
           << "escalation with multiple hosts / host groups / services "
           << "/ service groups.");

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new escalation for service '"
    << obj->service_description().front() << "' of host '"
    << obj->hosts().front() << "'.";

  // Add escalation to the global configuration set.
  config->serviceescalations().insert(obj);

  // Create service escalation.
  serviceescalation_struct*
    se(add_service_escalation(
         obj->hosts().front().c_str(),
         obj->service_description().front().c_str(),
         obj->first_notification(),
         obj->last_notification(),
         obj->notification_interval(),
         NULL_IF_EMPTY(obj->escalation_period()),
         static_cast<bool>(
           obj->escalation_options()
           & configuration::serviceescalation::recovery),
         static_cast<bool>(
           obj->escalation_options()
           & configuration::serviceescalation::warning),
         static_cast<bool>(
           obj->escalation_options()
           & configuration::serviceescalation::unknown),
         static_cast<bool>(
           obj->escalation_options()
           & configuration::serviceescalation::critical)));
  if (!se)
    throw (engine_error() << "Error: Could not create escalation on "
           << "service '" << obj->service_description().front()
           << "' of host '" << obj->hosts().front() << "'.");

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
    if (!add_contact_to_serviceescalation(se, it->c_str()))
      throw (engine_error() << "Error: Could not add contact '"
             << *it << "' to escalation of service '"
             << obj->service_description().front() << "' of host '"
             << obj->hosts().front() << "'.");

  // Unique contact groups.
  std::set<std::string> contact_groups;
  for (std::list<std::string>::const_iterator
         it(obj->contactgroups().begin()),
         end(obj->contactgroups().end());
       it != end;
       ++it)
    contact_groups.insert(*it);

  // Add contact groups to service escalation.
  for (std::set<std::string>::const_iterator
         it(contact_groups.begin()),
         end(contact_groups.end());
       it != end;
       ++it)
    if (!add_contactgroup_to_serviceescalation(se, it->c_str()))
      throw (engine_error() << "Error: Could not add contact group '"
             << *it << "' to escalation of service '"
             << obj->service_description().front() << "' of host '"
             << obj->hosts().front() << "'.");

  return ;
}

/**
 *  Expand a service escalation.
 *
 *  @param[in]     obj Service escalation object.
 *  @param[in,out] s   Configuration being applied.
 */
void applier::serviceescalation::expand_object(
                                   shared_ptr<configuration::serviceescalation> obj,
                                   configuration::state& s) {
  // Inherits special vars.
  if ((obj->hosts().size() == 1)
      && obj->hostgroups().empty()
      && (obj->service_description().size() == 1)
      && obj->servicegroups().empty())
    _inherits_special_vars(obj, s);
  // Expand service escalation.
  else {
    // Expanded services.
    std::set<std::pair<std::string, std::string> > expanded_services;
    _expand_services(
      obj->hosts(),
      obj->hostgroups(),
      obj->service_description(),
      obj->servicegroups(),
      s,
      expanded_services);

    // Remove current service escalation.
    s.serviceescalations().erase(obj);

    // Browse all services.
    for (std::set<std::pair<std::string, std::string> >::const_iterator
           it(expanded_services.begin()),
           end(expanded_services.end());
         it != end;
         ++it) {
      shared_ptr<configuration::serviceescalation>
        sesc(new configuration::serviceescalation(*obj));
      sesc->hostgroups().clear();
      sesc->hosts().clear();
      sesc->hosts().push_back(it->first);
      sesc->servicegroups().clear();
      sesc->service_description().clear();
      sesc->service_description().push_back(it->second);

      // Insert new service escalation and expand it.
      s.serviceescalations().insert(sesc);
      expand_object(sesc, s);
    }
  }

  return ;
}

/**
 *  @brief Modify service escalation.
 *
 *  Service escalations cannot be defined with anything else than their
 *  full content. Therefore no modification can occur.
 *
 *  @param[in] obj Unused.
 */
void applier::serviceescalation::modify_object(
                                   shared_ptr<configuration::serviceescalation> obj) {
  (void)obj;
  throw (engine_error() << "Error: Could not modify a service "
         << "escalation: service escalation objects can only be added "
         << "or removed, this is likely a software bug that you should "
         << "report to Centreon Engine developers");
  return ;
}

/**
 *  Remove old service escalation.
 *
 *  @param[in] obj The service escalation to remove from the monitoring
 *                 engine.
 */
void applier::serviceescalation::remove_object(
                                   shared_ptr<configuration::serviceescalation> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing a service escalation.";

  // Find service escalation.
  umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> >::iterator
    it(applier::state::instance().serviceescalations_find(obj->key()));
  if (it != applier::state::instance().serviceescalations().end()) {
    // Remove service escalation from its list.
    unregister_object<serviceescalation_struct>(
      &serviceescalation_list,
      it->second.get());

    // Erase service escalation (will effectively delete the object).
    applier::state::instance().serviceescalations().erase(it);
  }

  // Remove escalation from the global configuration set.
  config->serviceescalations().erase(obj);

  return ;
}

/**
 *  Resolve a serviceescalation.
 *
 *  @param[in] obj Serviceescalation object.
 */
void applier::serviceescalation::resolve_object(
                                   shared_ptr<configuration::serviceescalation> obj) {
  // XXX
}

/**
 *  Expand services.
 *
 *  @param[in]     hst      Hosts.
 *  @param[in]     hg       Host groups.
 *  @param[in]     svc      Service descriptions.
 *  @param[in]     sg       Service groups.
 *  @param[in,out] s        Configuration state.
 *  @param[out]    expanded Expanded services.
 */
void applier::serviceescalation::_expand_services(
       std::list<std::string> const& hst,
       std::list<std::string> const& hg,
       std::list<std::string> const& svc,
       std::list<std::string> const& sg,
       configuration::state& s,
       std::set<std::pair<std::string, std::string> >& expanded) {
  // Expanded hosts.
  std::set<std::string> all_hosts;

  // Base hosts.
  for (std::list<std::string>::const_iterator
         it(hst.begin()),
         end(hst.end());
       it != end;
       ++it)
    all_hosts.insert(*it);

  // Host groups.
  for (std::list<std::string>::const_iterator
         it(hg.begin()),
         end(hg.end());
       it != end;
       ++it) {
    // Find host group.
    std::set<shared_ptr<configuration::hostgroup> >::iterator
      it_group(s.hostgroups().begin()),
      end_group(s.hostgroups().end());
    while (it_group != end_group) {
      if ((*it_group)->hostgroup_name() == *it)
        break ;
      ++it_group;
    }
    if (it_group == end_group)
      throw (engine_error() << "Error: Could not resolve host group '"
             << *it << "'.");

    // Add host group members.
    for (std::set<std::string>::const_iterator
           it_member((*it_group)->resolved_members().begin()),
           end_member((*it_group)->resolved_members().end());
         it_member != end_member;
         ++it_member)
      all_hosts.insert(*it_member);
  }

  // Hosts * services.
  for (std::set<std::string>::const_iterator
         it_host(all_hosts.begin()),
         end_host(all_hosts.end());
       it_host != end_host;
       ++it_host)
    for (std::list<std::string>::const_iterator
           it_service(svc.begin()),
           end_service(svc.end());
         it_service != end_service;
         ++it_service)
      expanded.insert(std::make_pair(*it_host, *it_service));

  // Service groups.
  for (std::list<std::string>::const_iterator
         it(sg.begin()),
         end(sg.end());
       it != end;
       ++it) {
    // Find service group.
    std::set<shared_ptr<configuration::servicegroup> >::iterator
      it_group(s.servicegroups().begin()),
      end_group(s.servicegroups().end());
    while (it_group != end_group) {
      if ((*it_group)->servicegroup_name() == *it)
        break ;
      ++it_group;
    }
    if (it_group == end_group)
      throw (engine_error()
             << "Error: Could not resolve service group '"
             << *it << "'.");

    // Add service group members.
    for (std::set<std::pair<std::string, std::string> >::const_iterator
           it_member((*it_group)->resolved_members().begin()),
           end_member((*it_group)->resolved_members().end());
         it_member != end_member;
         ++it_member)
      expanded.insert(*it_member);
  }

  return ;
}

/**
 *  Inherits special variables from the service.
 *
 *  @param[in,out] obj Service escalation object.
 *  @param[in,out] s   Configuration state.
 */
void applier::serviceescalation::_inherits_special_vars(
                                   shared_ptr<configuration::serviceescalation> obj,
                                   configuration::state& s) {
  // Detect if any special variables has not been defined.
  if (!obj->contacts_defined()
      || !obj->contactgroups_defined()
      || !obj->notification_interval_defined()
      || !obj->escalation_period_defined()) {
    // Remove service escalation from state. It will be modified and
    // reinserted at the end of the method.
    s.serviceescalations().erase(obj);

    // Find service.
    std::set<shared_ptr<configuration::service> >::const_iterator
      it(s.services().begin()),
      end(s.services().end());
    while (it != end) {
      if (((*it)->hosts().front() == obj->hosts().front())
          && ((*it)->service_description()
              == obj->service_description().front()))
        break ;
      ++it;
    }
    if (it == end)
      throw (engine_error() << "Error: Could not inherit special "
             << "variables from service '"
             << obj->service_description().front() << "' of host '"
             << obj->hosts().front() << "': service does not exist.");

    // Inherits variables.
    if (!obj->contacts_defined())
      obj->contacts() = (*it)->contacts();
    if (!obj->contactgroups_defined())
      obj->contactgroups() = (*it)->contactgroups();
    if (!obj->notification_interval_defined())
      obj->notification_interval((*it)->notification_interval());
    if (!obj->escalation_period_defined())
      obj->escalation_period((*it)->notification_period());

    // Reinsert service escalation.
    s.serviceescalations().insert(obj);
  }

  return ;
}

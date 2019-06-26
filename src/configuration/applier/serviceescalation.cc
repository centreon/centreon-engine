/*
** Copyright 2011-2019 Centreon
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
 *  @param[in] obj  The new service escalation to add into the
 *                  monitoring engine.
 */
void applier::serviceescalation::add_object(
    configuration::serviceescalation const& obj) {
  // Check service escalation.
  if ((obj.hosts().size() != 1) || !obj.hostgroups().empty() ||
      obj.service_description().size() != 1 || !obj.servicegroups().empty())
    throw engine_error()
          << "Could not create service escalation with multiple hosts / host groups / services / service groups";

  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Creating new escalation for service '"
      << obj.service_description().front() << "' of host '"
      << obj.hosts().front() << "'";

  // Add escalation to the global configuration set.
  config->serviceescalations().insert(obj);

  // Create service escalation.
  std::shared_ptr<engine::serviceescalation> se{new engine::serviceescalation(
      obj.hosts().front(), obj.service_description().front(),
      obj.first_notification(), obj.last_notification(),
      obj.notification_interval(), obj.escalation_period(),
      ((obj.escalation_options() & configuration::serviceescalation::warning)
           ? notifier::warning
           : notifier::none) |
          ((obj.escalation_options() &
            configuration::serviceescalation::unknown)
               ? notifier::unknown
               : notifier::none) |
          ((obj.escalation_options() &
            configuration::serviceescalation::critical)
               ? notifier::critical
               : notifier::none) |
          ((obj.escalation_options() &
            configuration::serviceescalation::recovery)
               ? notifier::recovery
               : notifier::none))};

  // Add new items to tail the list.
  engine::serviceescalation::serviceescalations.insert(
      {{se->get_hostname(), se->get_description()}, se});

  // Notify event broker.
  timeval tv(get_broker_timestamp(nullptr));
  broker_adaptive_escalation_data(NEBTYPE_SERVICEESCALATION_ADD, NEBFLAG_NONE,
                                  NEBATTR_NONE, se.get(), &tv);

  // Add contacts to host escalation.
  for (set_string::const_iterator it(obj.contacts().begin()),
       end(obj.contacts().end());
       it != end; ++it)
    se->contacts().insert({*it, nullptr});

  // Add contact groups to service escalation.
  for (set_string::const_iterator it(obj.contactgroups().begin()),
       end(obj.contactgroups().end());
       it != end; ++it)
    se->contact_groups().insert({*it, nullptr});
}

/**
 *  Expand all service escalations.
 *
 *  @param[in,out] s  Configuration being applied.
 */
void applier::serviceescalation::expand_objects(configuration::state& s) {
  // Browse all escalations.
  configuration::set_serviceescalation expanded;
  for (configuration::set_serviceescalation::const_iterator
         it_esc(s.serviceescalations().begin()),
         end_esc(s.serviceescalations().end());
       it_esc != end_esc;
       ++it_esc) {
    // Expanded services.
    std::set<std::pair<std::string, std::string> > expanded_services;
    _expand_services(
      it_esc->hosts(),
      it_esc->hostgroups(),
      it_esc->service_description(),
      it_esc->servicegroups(),
      s,
      expanded_services);

    // Browse all services.
    for (std::set<std::pair<std::string, std::string> >::const_iterator
           it(expanded_services.begin()),
           end(expanded_services.end());
         it != end;
         ++it) {
      configuration::serviceescalation sesc(*it_esc);
      sesc.hostgroups().clear();
      sesc.hosts().clear();
      sesc.hosts().push_back(it->first);
      sesc.servicegroups().clear();
      sesc.service_description().clear();
      sesc.service_description().push_back(it->second);

      // Insert new service escalation and expand it.
      _inherits_special_vars(sesc, s);
      expanded.insert(sesc);
    }
  }

  // Set expanded service escalations in configuration state.
  s.serviceescalations().swap(expanded);
}

/**
 *  @brief Modify service escalation.
 *
 *  Service escalations cannot be defined with anything else than their
 *  full content. Therefore no modification can occur.
 *
 *  @param[in] obj  Unused.
 */
void applier::serviceescalation::modify_object(
       configuration::serviceescalation const& obj) {
  (void)obj;
  throw (engine_error() << "Could not modify a service "
         << "escalation: service escalation objects can only be added "
         << "or removed, this is likely a software bug that you should "
         << "report to Centreon Engine developers");
}

/**
 *  Remove old service escalation.
 *
 *  @param[in] obj  The service escalation to remove from the monitoring
 *                  engine.
 */
void applier::serviceescalation::remove_object(
       configuration::serviceescalation const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing a service escalation.";

  // Find service escalation.
  std::string const& host_name{*obj.hosts().begin()};
  std::string const& description{obj.service_description().front()};
  std::pair<serviceescalation_mmap::iterator, serviceescalation_mmap::iterator>
      range{engine::serviceescalation::serviceescalations.equal_range({host_name, description})};
  service_map::iterator sit{engine::service::services.find({host_name, description})};

  for (serviceescalation_mmap::iterator
       it{range.first}, end{range.second};
       it != end; ++it) {
    std::list<std::shared_ptr<escalation>>& escalations(sit->second->get_escalations());
    for (std::list<std::shared_ptr<engine::escalation>>::iterator
        itt{escalations.begin()},
        next_itt{escalations.begin()},
        end{escalations.end()};
        itt != end; itt = next_itt) {
      ++next_itt;
      /* It's a pity but for now we don't have any possibility or key to verify
       * if the hostescalation is the good one. */
      if ((*itt)->get_first_notification() == obj.first_notification() &&
          (*itt)->get_last_notification() == obj.last_notification() &&
          (*itt)->get_notification_interval() == obj.notification_interval() &&
          (*itt)->get_escalation_period() == obj.escalation_period() &&
          (*itt)->get_escalate_on(notifier::down) ==
              (obj.escalation_options() &
               configuration::hostescalation::down) &&
          (*itt)->get_escalate_on(notifier::unreachable) ==
              (obj.escalation_options() &
               configuration::hostescalation::unreachable) &&
          (*itt)->get_escalate_on(notifier::recovery) ==
              (obj.escalation_options() &
               configuration::hostescalation::recovery)) {
        // We have the hostescalation to remove.
        escalations.erase(itt);
        break;
      }
    }
    // Remove escalation from the global configuration set.
    engine::serviceescalation::serviceescalations.erase(it);
  }

  config->serviceescalations().erase(obj);
}

/**
 *  Resolve a serviceescalation.
 *
 *  @param[in] obj  Serviceescalation object.
 */
void applier::serviceescalation::resolve_object(
       configuration::serviceescalation const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Resolving a service escalation.";

  service_map::iterator it{engine::service::services.find(
    {*obj.hosts().begin(), obj.service_description().front()})};
  for (std::list<std::shared_ptr<engine::escalation>>::const_iterator
      itt{it->second->get_escalations().begin()},
      end{it->second->get_escalations().end()};
      itt != end;
      ++itt) {
    /* It's a pity but for now we don't have any idea or key to verify if
     * the hostescalation is the good one. */
    if ((*itt)->get_first_notification() == obj.first_notification() &&
        (*itt)->get_last_notification() == obj.last_notification() &&
        (*itt)->get_notification_interval() == obj.notification_interval() &&
        (*itt)->get_escalation_period() == obj.escalation_period() &&
        (*itt)->get_escalate_on(notifier::down) == (obj.escalation_options() & configuration::hostescalation::down) &&
        (*itt)->get_escalate_on(notifier::unreachable) == (obj.escalation_options() & configuration::hostescalation::unreachable) &&
        (*itt)->get_escalate_on(notifier::recovery) == (obj.escalation_options() & configuration::hostescalation::recovery)) {
      // Resolve service escalation.
      (*itt)->resolve(config_warnings, config_errors);
    } else
      throw engine_error() << "Cannot resolve non-existing service escalation";
  }
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
  all_hosts.insert(hst.begin(), hst.end());

  // Host groups.
  for (std::list<std::string>::const_iterator
         it(hg.begin()),
         end(hg.end());
       it != end;
       ++it) {
    // Find host group.
    configuration::set_hostgroup::iterator
      it_group(s.hostgroups_find(*it));
    if (it_group == s.hostgroups().end())
      throw (engine_error() << "Could not resolve host group '"
             << *it << "'");

    // Add host group members.
    all_hosts.insert(
                it_group->members().begin(),
                it_group->members().end());
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
    configuration::set_servicegroup::iterator
      it_group(s.servicegroups_find(*it));
    if (it_group == s.servicegroups().end())
      throw (engine_error() << "Could not resolve service group '"
             << *it << "'");

    // Add service group members.
    for (set_pair_string::const_iterator
           it_member(it_group->members().begin()),
           end_member(it_group->members().end());
         it_member != end_member;
         ++it_member)
      expanded.insert(*it_member);
  }
}

/**
 *  Inherits special variables from the service.
 *
 *  @param[in,out] obj Service escalation object.
 *  @param[in]     s   Configuration state.
 */
void applier::serviceescalation::_inherits_special_vars(
       configuration::serviceescalation& obj,
       configuration::state const& s) {
  // Detect if any special variables has not been defined.
  if (!obj.contacts_defined()
      || !obj.contactgroups_defined()
      || !obj.notification_interval_defined()
      || !obj.escalation_period_defined()) {
    // Find service.
    unsigned int host_id(get_host_id(obj.hosts().front().c_str()));
    unsigned int service_id(get_service_id(
                   obj.hosts().front().c_str(),
                   obj.hosts().front().c_str()));
    configuration::set_service::const_iterator
      it(s.services_find(std::make_pair(
                                host_id,
                                service_id)));
    if (it == s.services().end())
      throw (engine_error() << "Could not inherit special "
             << "variables from service '"
             << obj.service_description().front() << "' of host '"
             << obj.hosts().front() << "': service does not exist");

    // Inherits variables.
    if (!obj.contacts_defined())
      obj.contacts() = it->contacts();
    if (!obj.contactgroups_defined())
      obj.contactgroups() = it->contactgroups();
    if (!obj.notification_interval_defined())
      obj.notification_interval(it->notification_interval());
    if (!obj.escalation_period_defined())
      obj.escalation_period(it->notification_period());
  }
}

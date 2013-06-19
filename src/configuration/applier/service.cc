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
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/service.hh"
#include "com/centreon/engine/error.hh"
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
 *  @param[in] s   Configuration being applied.
 */
void applier::service::add_object(
                         service_ptr obj,
                         configuration::state& s) {
  // Get host list.
  std::set<std::string> target_hosts;
  // Hosts members.
  for (list_string::const_iterator
         it(obj->hosts().begin()),
         end(obj->hosts().end());
       it != end;
       ++it)
    target_hosts.insert(*it);
  // Host group members.
  for (list_string::const_iterator
         it(obj->hostgroups().begin()),
         end(obj->hostgroups().end());
       it != end;
       ++it) {
    // Find host group.
    list_hostgroup::iterator
      it2(s.hostgroups().begin()),
      end2(s.hostgroups().end());
    while (it2 != end2) {
      if (*it == (*it2)->hostgroup_name())
        break ;
      ++it2;
    }
    if (it2 == end2)
      throw (engine_error() << "Error: Could not find host group '"
             << *it << "' on which to apply service '"
             << obj->service_description() << "'.");

    // Add host group members.
    for (set_string::const_iterator
           it3((*it2)->resolved_members().begin()),
           end3((*it2)->resolved_members().end());
         it3 != end3;
         ++it3)
      target_hosts.insert(*it3);
  }

  // Get service group list.
  std::list<shared_ptr<servicegroup_struct> > target_groups;
  for (list_string::const_iterator
         it(obj->servicegroups().begin()),
         end(obj->servicegroups().end());
       it != end;
       ++it) {
    umap<std::string,
         std::pair<configuration::servicegroup,
                   shared_ptr<servicegroup_struct> > >::iterator
      it2(applier::state::instance().servicegroups().find(*it));
    if (it2 == applier::state::instance().servicegroups().end())
      throw (engine_error() << "Error: Could not add service '"
             << obj->service_description() << "' to service group '"
             << *it << "'.");
    target_groups.push_back(it2->second.second);
  }

  // Browse all hosts of this service.
  for (std::set<std::string>::const_iterator
         it(target_hosts.begin()),
         end(target_hosts.end());
       it != end;
       ++it) {
    // Logging.
    logger(logging::dbg_config, logging::more)
      << "Creating new service '" << obj->service_description()
      << "' of host '" << *it << "'.";

    // Create service.
    shared_ptr<service_struct>
      s(
        add_service(
          it->c_str(),
          obj->service_description().c_str(),
          NULL_IF_EMPTY(obj->display_name()),
          NULL_IF_EMPTY(obj->check_period()),
          obj->initial_state(),
          obj->max_check_attempts(),
          false, // parallelize
          obj->checks_passive(),
          obj->check_interval(),
          obj->retry_interval(),
          obj->notification_interval(),
          obj->first_notification_delay(),
          NULL_IF_EMPTY(obj->notification_period()),
          static_cast<bool>(obj->notification_options()
                            & configuration::service::ok),
          static_cast<bool>(obj->notification_options()
                            & configuration::service::unknown),
          static_cast<bool>(obj->notification_options()
                            & configuration::service::warning),
          static_cast<bool>(obj->notification_options()
                            & configuration::service::critical),
          static_cast<bool>(obj->notification_options()
                            & configuration::service::flapping),
          static_cast<bool>(obj->notification_options()
                            & configuration::service::downtime),
          obj->notifications_enabled(),
          obj->is_volatile(),
          NULL_IF_EMPTY(obj->event_handler()),
          obj->event_handler_enabled(),
          NULL_IF_EMPTY(obj->check_command()),
          obj->checks_active(),
          obj->flap_detection_enabled(),
          obj->low_flap_threshold(),
          obj->high_flap_threshold(),
          static_cast<bool>(obj->flap_detection_options()
                            & configuration::service::ok),
          static_cast<bool>(obj->flap_detection_options()
                            &configuration::service::warning),
          static_cast<bool>(obj->flap_detection_options()
                            &configuration::service::unknown),
          static_cast<bool>(obj->flap_detection_options()
                            &configuration::service::critical),
          static_cast<bool>(obj->stalking_options()
                            &configuration::service::ok),
          static_cast<bool>(obj->stalking_options()
                            &configuration::service::warning),
          static_cast<bool>(obj->stalking_options()
                            &configuration::service::unknown),
          static_cast<bool>(obj->stalking_options()
                            &configuration::service::critical),
          obj->process_perf_data(),
          false, // failure_prediction_enabled
          NULL, // failure_prediction_options
          obj->check_freshness(),
          obj->freshness_threshold(),
          NULL_IF_EMPTY(obj->notes()),
          NULL_IF_EMPTY(obj->notes_url()),
          NULL_IF_EMPTY(obj->action_url()),
          NULL_IF_EMPTY(obj->icon_image()),
          NULL_IF_EMPTY(obj->icon_image_alt()),
          obj->retain_status_information(),
          obj->retain_nonstatus_information(),
          obj->obsess_over_service()),
        &deleter::service);
    if (!s.get())
      throw (engine_error() << "Error: Could not register service '"
             << obj->service_description()
             << "' of host '" << *it << "'.");

    // Add contacts.
    for (list_string::const_iterator
           it2(obj->contacts().begin()),
           end2(obj->contacts().end());
         it2 != end2;
         ++it2)
      if (!add_contact_to_service(s.get(), it2->c_str()))
        throw (engine_error() << "Error: Could not add contact '"
               << *it2 << "' to service '" << obj->service_description()
               << "' of host '" << *it << "'.");

    // Add contactgroups.
    for (list_string::const_iterator
           it2(obj->contactgroups().begin()),
           end2(obj->contactgroups().end());
         it2 != end2;
         ++it2)
      if (!add_contactgroup_to_service(s.get(), it2->c_str()))
        throw (engine_error() << "Error: Could not add contact group '"
               << *it2 << "' to service '" << obj->service_description()
               << "' of host '" << *it << "'.");

    // Add custom variables.
    for (properties::const_iterator
           it2(obj->customvariables().begin()),
           end2(obj->customvariables().end());
         it2 != end2;
         ++it2)
      if (!add_custom_variable_to_service(
             s.get(),
             it2->first.c_str(),
             it2->second.c_str()))
        throw (engine_error()
               << "Error: Could not add custom variable '"
               << it2->first << "' to service '"
               << obj->service_description() << "' of host '" << *it
               << "'.");

    // Service groups.
    for (std::list<shared_ptr<servicegroup_struct> >::iterator
           it2(target_groups.begin()),
           end2(target_groups.end());
         it2 != end2;
         ++it2)
      if (!add_service_to_servicegroup(
             it2->get(),
             it->c_str(),
             obj->service_description().c_str()))
        throw (engine_error() << "Error: Could not add service '"
               << obj->service_description() << "' of host '" << *it
               << "' to service group '" << (*it2)->group_name
               << "'.");

    // Register service.
    s->next = service_list;
    applier::state::instance().services()[std::make_pair(*it, obj->service_description())] = s;
    service_list = s.get();
  }

  return ;
}

/**
 *  Modified service.
 *
 *  @param[in] obj The new service to modify into the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::service::modify_object(
                         service_ptr obj,
                         configuration::state& s) {
  // Browse all hosts of this service.
  for (list_string::const_iterator
         it(obj->hosts().begin()),
         end(obj->hosts().end());
       it != end;
       ++it) {
    // Logging.
    logger(logging::dbg_config, logging::more)
      << "Modifying service '" << obj->service_description()
      << "' of host '" << *it << "'.";

    // XXX
  }

  return ;
}

/**
 *  Remove old service.
 *
 *  @param[in] obj The new service to remove from the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::service::remove_object(
                         service_ptr obj,
                         configuration::state& s) {
  // Browse all hosts of this service.
  for (list_string::const_iterator
         it(obj->hosts().begin()),
         end(obj->hosts().end());
       it != end;
       ++it) {
    // Logging.
    logger(logging::dbg_config, logging::more)
      << "Removing service '" << obj->service_description()
      << "' of host '" << *it << "'.";

    // Unregister service.
    for (service_struct** s(&service_list); *s; s = &(*s)->next)
      if (!strcmp((*s)->host_name, it->c_str())
          && !strcmp(
                (*s)->description,
                obj->service_description().c_str())) {
        *s = (*s)->next;
        break ;
      }
    applier::state::instance().services().erase(
      std::make_pair(*it, obj->service_description()));
  }

  return ;
}

/**
 *  Resolve a service.
 *
 *  @param[in] obj Service object.
 *  @param[in] s   Configuration being applied.
 */
void applier::service::resolve_object(
                         service_ptr obj,
                         configuration::state& s) {
  // Browse all hosts of this service.
  for (list_string::const_iterator
         it(obj->hosts().begin()),
         end(obj->hosts().end());
       it != end;
       ++it) {
    // Logging.
    logger(logging::dbg_config, logging::more)
      << "Resolving service '" << obj->service_description()
      << "' of host '" << *it << "'.";

    // Find service.
    umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::iterator
      it2(applier::state::instance().services().find(
           std::make_pair(*it, obj->service_description())));
    if (applier::state::instance().services().end() == it2)
      throw (engine_error()
             << "Error: Cannot resolve non-existing service '"
             << obj->service_description() << "' of host '"
             << *it << "'.");

    // Resolve service.
    if (!check_service(it2->second.get(), NULL, NULL))
      throw (engine_error() << "Error: Cannot resolve service '"
             << obj->service_description() << "' of host '"
             << *it << "'.");
  }

  return ;
}

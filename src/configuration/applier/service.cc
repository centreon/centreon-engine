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

#include <algorithm>
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

/**
 *  Check if the service group name matches the configuration object.
 */
class         servicegroup_name_comparator {
public:
              servicegroup_name_comparator(
                std::string const& servicegroup_name) {
    _servicegroup_name = servicegroup_name;
  }

  bool        operator()(shared_ptr<configuration::servicegroup> sg) {
    return (_servicegroup_name == sg->servicegroup_name());
  }

private:
  std::string _servicegroup_name;
};

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
 */
void applier::service::add_object(
                         shared_ptr<configuration::service> obj) {
  // Check service.
  if (obj->hosts().size() != 1)
    throw (engine_error() << "Error: Could not create service '"
           << obj->service_description()
           << "' with multiple hosts defined.");
  else if (!obj->hostgroups().empty())
    throw (engine_error() << "Error: Could not create service '"
           << obj->service_description()
           << "' with multiple host groups defined.");

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new service '" << obj->service_description()
    << "' of host '" << obj->hosts().front() << "'.";

  // Add service to the global configuration set.
  config->services().insert(obj);

  // Create service.
  service_struct* svc(add_service(
    obj->hosts().front().c_str(),
    obj->service_description().c_str(),
    NULL_IF_EMPTY(obj->display_name()),
    NULL_IF_EMPTY(obj->check_period()),
    obj->initial_state(),
    obj->max_check_attempts(),
    true, // parallelize, enabled by default in Nagios
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
    true, // failure_prediction_enabled, enabled by default in Nagios
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
    obj->obsess_over_service()));
  if (!svc)
      throw (engine_error() << "Error: Could not register service '"
             << obj->service_description()
             << "' of host '" << obj->hosts().front() << "'.");

  // Add contacts.
  for (list_string::const_iterator
         it(obj->contacts().begin()),
         end(obj->contacts().end());
       it != end;
       ++it)
    if (!add_contact_to_service(svc, it->c_str()))
      throw (engine_error() << "Error: Could not add contact '"
             << *it << "' to service '" << obj->service_description()
             << "' of host '" << obj->hosts().front() << "'.");

  // Add contactgroups.
  for (list_string::const_iterator
         it(obj->contactgroups().begin()),
         end(obj->contactgroups().end());
       it != end;
       ++it)
    if (!add_contactgroup_to_service(svc, it->c_str()))
      throw (engine_error() << "Error: Could not add contact group '"
             << *it << "' to service '" << obj->service_description()
             << "' of host '" << obj->hosts().front() << "'.");

  // Add custom variables.
  for (properties::const_iterator
         it(obj->customvariables().begin()),
         end(obj->customvariables().end());
       it != end;
       ++it)
    if (!add_custom_variable_to_service(
           svc,
           it->first.c_str(),
           it->second.c_str()))
      throw (engine_error()
             << "Error: Could not add custom variable '"
             << it->first << "' to service '"
             << obj->service_description() << "' of host '"
             << obj->hosts().front() << "'.");

  return ;
}

/**
 *  Expand a service object.
 *
 *  @param[in]  obj      Object to expand.
 *  @param[in]  s        State being applied.
 *  @param[out] expanded Expanded services.
 */
void applier::service::expand_object(
                         shared_ptr<configuration::service> obj,
                         configuration::state& s) {
  // Either expand service instance.
  if ((obj->hosts().size() == 1) && obj->hostgroups().empty()) {
    // Expand memberships.
    _expand_service_memberships(obj, s);

    // Inherits special vars.
    _inherits_special_vars(obj, s);
  }
  // Or expand service to instances.
  else {
    // All hosts members.
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
      set_hostgroup::iterator
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

    // Remove current service.
    s.services().erase(obj);

    // Browse all target hosts.
    for (std::set<std::string>::const_iterator
           it(target_hosts.begin()),
           end(target_hosts.end());
         it != end;
         ++it) {
      // Create service instance.
      shared_ptr<configuration::service>
        svc(new configuration::service(*obj));
      svc->hostgroups().clear();
      svc->hosts().clear();
      svc->hosts().push_back(*it);

      // Insert new service instance and expand it.
      s.services().insert(svc);
      expand_object(svc, s);
    }
  }

  return ;
}

/**
 *  Modified service.
 *
 *  @param[in] obj The new service to modify into the monitoring engine.
 */
void applier::service::modify_object(
                         shared_ptr<configuration::service> obj) {
  // XXX
  return ;
}

/**
 *  Remove old service.
 *
 *  @param[in] obj The new service to remove from the monitoring engine.
 */
void applier::service::remove_object(
                         shared_ptr<configuration::service> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing service '" << obj->service_description()
    << "' of host '" << obj->hosts().front() << "'.";

  // Unregister service.
  for (service_struct** s(&service_list); *s; s = &(*s)->next)
    if (!strcmp((*s)->host_name, obj->hosts().front().c_str())
        && !strcmp(
              (*s)->description,
              obj->service_description().c_str())) {
      *s = (*s)->next;
      break ;
    }

  // Remove service object (will effectively delete the object).
  applier::state::instance().services().erase(obj->key());

  // Remove service from the global configuration set.
  config->services().erase(obj);

  return ;
}

/**
 *  Resolve a service.
 *
 *  @param[in] obj Service object.
 */
void applier::service::resolve_object(
                         shared_ptr<configuration::service> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving service '" << obj->service_description()
    << "' of host '" << obj->hosts().front() << "'.";

  // Find service.
  umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::iterator
    it(applier::state::instance().services().find(obj->key()));
  if (applier::state::instance().services().end() == it)
    throw (engine_error()
           << "Error: Cannot resolve non-existing service '"
           << obj->service_description() << "' of host '"
           << obj->hosts().front() << "'.");

  // Resolve service.
  if (!check_service(it->second.get(), NULL, NULL))
      throw (engine_error() << "Error: Cannot resolve service '"
             << obj->service_description() << "' of host '"
             << obj->hosts().front() << "'.");

  return ;
}

/**
 *  Expand service instance memberships.
 *
 *  @param[in]  obj Target service.
 *  @param[out] s   Configuration state.
 */
void applier::service::_expand_service_memberships(
                         shared_ptr<configuration::service> obj,
                         configuration::state& s) {
  // Browse service groups.
  for (list_string::const_iterator
         it(obj->servicegroups().begin()),
         end(obj->servicegroups().end());
       it != end;
       ++it) {
    // Find service group.
    std::set<shared_ptr<configuration::servicegroup> >::iterator
      it_group(std::find_if(
                      s.servicegroups().begin(),
                      s.servicegroups().end(),
                      servicegroup_name_comparator(*it)));
    if (it_group == s.servicegroups().end())
      throw (engine_error() << "Error: Could not add service '"
             << obj->service_description() << "' of host '"
             << obj->hosts().front()
             << "' to non-existing service group '" << *it << "'.");

    // Remove service group from state.
    shared_ptr<configuration::servicegroup> backup(*it_group);
    s.servicegroups().erase(it_group);

    // Add service to service members.
    backup->members().push_back(obj->hosts().front());
    backup->members().push_back(obj->service_description());

    // Reinsert service group.
    s.servicegroups().insert(backup);
  }

  return ;
}

/**
 *  @brief Inherits special variables from host.
 *
 *  These special variables, if not defined are inherited from host.
 *  They are contact_groups, notification_interval and
 *  notification_period.
 *
 *  @param[in,out] obj Target service.
 *  @param[in,out] s   Configuration state.
 */
void applier::service::_inherits_special_vars(
                         shared_ptr<configuration::service> obj,
                         configuration::state& s) {
  // Detect if any special variable has not been defined.
  if (!obj->contactgroups_defined()
      || !obj->notification_interval_defined()
      || !obj->notification_period_defined()) {
    // Remove service from state (it will be modified
    // and reinserted at the end of the method).
    s.services().erase(obj);

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
             << "Error: Could not inherit special variables for service '"
             << obj->service_description() << "': host '"
             << obj->hosts().front() << "' does not exist.");

    // Inherits variables.
    if (!obj->contactgroups_defined())
      obj->contactgroups() = (*it)->contactgroups();
    if (!obj->notification_interval_defined())
      obj->notification_interval((*it)->notification_interval());
    if (!obj->notification_period_defined())
      obj->notification_period((*it)->notification_period());

    // Reinsert service.
    s.services().insert(obj);
  }

  return ;
}

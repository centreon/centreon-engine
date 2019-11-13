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

#include "com/centreon/engine/configuration/applier/service.hh"
#include <algorithm>
#include <cassert>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/scheduler.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::downtimes;
using namespace com::centreon::engine::configuration;

/**
 *  Check if the service group name matches the configuration object.
 */
class servicegroup_name_comparator {
 public:
  servicegroup_name_comparator(std::string const& servicegroup_name) {
    _servicegroup_name = servicegroup_name;
  }

  bool operator()(std::shared_ptr<configuration::servicegroup> sg) {
    return _servicegroup_name == sg->servicegroup_name();
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
applier::service::~service() {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::service& applier::service::operator=(applier::service const& right) {
  (void)right;
  return *this;
}

/**
 *  Add new service.
 *
 *  @param[in] obj  The new service to add into the monitoring engine.
 */
void applier::service::add_object(configuration::service const& obj) {
  // Check service.
  if (obj.hosts().size() < 1)
    throw engine_error() << "Could not create service '"
                         << obj.service_description()
                         << "' with no host defined";
  else if (obj.hosts().size() > 1)
    throw engine_error() << "Could not create service '"
                         << obj.service_description()
                         << "' with multiple hosts defined";
  else if (!obj.hostgroups().empty())
    throw engine_error() << "Could not create service '"
                         << obj.service_description()
                         << "' with multiple host groups defined";
  else if (!obj.host_id())
    throw engine_error() << "No host_id available for the host '"
                         << *obj.hosts().begin()
                         << "' - unable to create service '"
                         << obj.service_description() << "'";

  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Creating new service '" << obj.service_description() << "' of host '"
      << *obj.hosts().begin() << "'.";

  // Add service to the global configuration set.
  config->services().insert(obj);

  // Create service.
  engine::service* svc{add_service(
      obj.host_id(), obj.service_id(), *obj.hosts().begin(),
      obj.service_description(), obj.display_name(), obj.check_period(),
      static_cast<engine::service::service_state>(obj.initial_state()),
      obj.max_check_attempts(), obj.check_interval(), obj.retry_interval(),
      obj.notification_interval(), obj.first_notification_delay(),
      obj.recovery_notification_delay(), obj.notification_period(),
      static_cast<bool>(obj.notification_options() &
                        configuration::service::ok),
      static_cast<bool>(obj.notification_options() &
                        configuration::service::unknown),
      static_cast<bool>(obj.notification_options() &
                        configuration::service::warning),
      static_cast<bool>(obj.notification_options() &
                        configuration::service::critical),
      static_cast<bool>(obj.notification_options() &
                        configuration::service::flapping),
      static_cast<bool>(obj.notification_options() &
                        configuration::service::downtime),
      obj.notifications_enabled(), obj.is_volatile(), obj.event_handler(),
      obj.event_handler_enabled(), obj.check_command(), obj.checks_active(),
      obj.checks_passive(), obj.flap_detection_enabled(),
      obj.low_flap_threshold(), obj.high_flap_threshold(),
      static_cast<bool>(obj.flap_detection_options() &
                        configuration::service::ok),
      static_cast<bool>(obj.flap_detection_options() &
                        configuration::service::warning),
      static_cast<bool>(obj.flap_detection_options() &
                        configuration::service::unknown),
      static_cast<bool>(obj.flap_detection_options() &
                        configuration::service::critical),
      static_cast<bool>(obj.stalking_options() & configuration::service::ok),
      static_cast<bool>(obj.stalking_options() &
                        configuration::service::warning),
      static_cast<bool>(obj.stalking_options() &
                        configuration::service::unknown),
      static_cast<bool>(obj.stalking_options() &
                        configuration::service::critical),
      obj.process_perf_data(), obj.check_freshness(), obj.freshness_threshold(),
      obj.notes(), obj.notes_url(), obj.action_url(), obj.icon_image(),
      obj.icon_image_alt(), obj.retain_status_information(),
      obj.retain_nonstatus_information(), obj.obsess_over_service(),
      obj.timezone())};
  if (!svc)
    throw engine_error() << "Could not register service '"
                         << obj.service_description() << "' of host '"
                         << *obj.hosts().begin() << "'";
  svc->set_initial_notif_time(0);
  engine::service::services[{*obj.hosts().begin(), obj.service_description()}]
      ->set_host_id(obj.host_id());
  engine::service::services[{*obj.hosts().begin(), obj.service_description()}]
      ->set_service_id(obj.service_id());
  svc->set_acknowledgement_timeout(obj.get_acknowledgement_timeout() *
                                   config->interval_length());
  svc->set_last_acknowledgement(0);

  // Add contacts.
  for (set_string::const_iterator it(obj.contacts().begin()),
       end(obj.contacts().end());
       it != end; ++it)
    svc->get_contacts().insert({*it, nullptr});

  // Add contactgroups.
  for (set_string::const_iterator it(obj.contactgroups().begin()),
       end(obj.contactgroups().end());
       it != end; ++it)
    svc->get_contactgroups().insert({*it, nullptr});

  // Add custom variables.
  for (map_customvar::const_iterator it(obj.customvariables().begin()),
       end(obj.customvariables().end());
       it != end; ++it) {
    svc->custom_variables[it->first] = it->second;

    if (it->second.is_sent()) {
      timeval tv(get_broker_timestamp(nullptr));
      broker_custom_variable(NEBTYPE_SERVICECUSTOMVARIABLE_ADD, NEBFLAG_NONE,
                             NEBATTR_NONE, svc, it->first.c_str(),
                             it->second.get_value().c_str(), &tv);
    }
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_service_data(NEBTYPE_SERVICE_ADD, NEBFLAG_NONE, NEBATTR_NONE,
                               svc, CMD_NONE, MODATTR_ALL, MODATTR_ALL, &tv);
}

/**
 *  Expand a service object.
 *
 *  @param[in,out] s  State being applied.
 */
void applier::service::expand_objects(configuration::state& s) {
  // Browse all services.
  configuration::set_service expanded;
  for (configuration::set_service::iterator it_svc(s.services().begin()),
       end_svc(s.services().end());
       it_svc != end_svc; ++it_svc) {
    // Should custom variables be sent to broker ?
    for (map_customvar::iterator
             it(const_cast<map_customvar&>(it_svc->customvariables()).begin()),
         end(const_cast<map_customvar&>(it_svc->customvariables()).end());
         it != end; ++it) {
      if (!s.enable_macros_filter() ||
          s.macros_filter().find(it->first) != s.macros_filter().end()) {
        it->second.set_sent(true);
      }
    }

    // Expand service to instances.
    std::set<std::string> target_hosts;

    // Hosts members.
    target_hosts = it_svc->hosts();

    // Host group members.
    for (set_string::const_iterator it(it_svc->hostgroups().begin()),
         end(it_svc->hostgroups().end());
         it != end; ++it) {
      // Find host group.
      set_hostgroup::iterator it2(s.hostgroups_find(*it));
      if (it2 == s.hostgroups().end())
        throw(engine_error() << "Could not find host group '" << *it
                             << "' on which to apply service '"
                             << it_svc->service_description() << "'");

      // Check host group and user configuration.
      if (it2->members().empty() && !s.allow_empty_hostgroup_assignment())
        throw(engine_error() << "Could not expand host group '" << *it
                             << "' specified in service '"
                             << it_svc->service_description() << "'");

      // Add host group members.
      target_hosts.insert(it2->members().begin(), it2->members().end());
    }

    // Browse all target hosts.
    for (std::set<std::string>::const_iterator it(target_hosts.begin()),
         end(target_hosts.end());
         it != end; ++it) {
      // Create service instance.
      configuration::service svc(*it_svc);
      svc.hostgroups().clear();
      svc.hosts().clear();
      svc.hosts().insert(*it);

      // Expand memberships.
      _expand_service_memberships(svc, s);

      // Inherits special vars.
      _inherits_special_vars(svc, s);

      // Insert object.
      expanded.insert(svc);
    }
  }

  // Set expanded services in configuration state.
  s.services().swap(expanded);
}

/**
 *  Modified service.
 *
 *  @param[in] obj  The new service to modify into the monitoring
 *                  engine.
 */
void applier::service::modify_object(configuration::service const& obj) {
  std::string const& host_name(*obj.hosts().begin());
  std::string const& service_description(obj.service_description());

  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Modifying new service '" << service_description << "' of host '"
      << host_name << "'.";

  // Find the configuration object.
  set_service::iterator it_cfg(config->services_find(obj.key()));
  if (it_cfg == config->services().end())
    throw engine_error() << "Cannot modify non-existing "
                            "service '"
                         << service_description << "' of host '" << host_name
                         << "'";

  // Find service object.
  service_id_map::iterator it_obj(
      engine::service::services_by_id.find(obj.key()));
  if (it_obj == engine::service::services_by_id.end())
    throw engine_error() << "Could not modify non-existing "
                         << "service object '" << service_description
                         << "' of host '" << host_name << "'";
  std::shared_ptr<engine::service> s{it_obj->second};

  // Update the global configuration set.
  configuration::service obj_old(*it_cfg);
  config->services().erase(it_cfg);
  config->services().insert(obj);

  // Modify properties.
  if (it_obj->second->get_hostname() != *obj.hosts().begin() ||
      it_obj->second->get_description() != obj.service_description()) {
    engine::service::services.erase(
        {it_obj->second->get_hostname(), it_obj->second->get_description()});
    engine::service::services.insert(
        {{*obj.hosts().begin(), obj.service_description()}, it_obj->second});
  }

  s->set_hostname(*obj.hosts().begin());
  s->set_description(obj.service_description());
  s->set_display_name(obj.display_name()),
      s->set_check_command(obj.check_command());
  s->set_event_handler(obj.event_handler());
  s->set_event_handler_enabled(obj.event_handler_enabled());
  s->set_initial_state(
      static_cast<engine::service::service_state>(obj.initial_state()));
  s->set_check_interval(obj.check_interval());
  s->set_retry_interval(obj.retry_interval());
  s->set_max_attempts(obj.max_check_attempts());

  s->set_notify_on(
      (obj.notification_options() & configuration::service::unknown
           ? notifier::unknown
           : notifier::none) |
      (obj.notification_options() & configuration::service::warning
           ? notifier::warning
           : notifier::none) |
      (obj.notification_options() & configuration::service::critical
           ? notifier::critical
           : notifier::none) |
      (obj.notification_options() & configuration::service::ok
           ? notifier::ok
           : notifier::none) |
      (obj.notification_options() & configuration::service::flapping
           ? (notifier::flappingstart | notifier::flappingstop |
              notifier::flappingdisabled)
           : notifier::none) |
      (obj.notification_options() & configuration::service::downtime
           ? notifier::downtime
           : notifier::none));

  s->set_notification_interval(
      static_cast<double>(obj.notification_interval()));
  s->set_first_notification_delay(
      static_cast<double>(obj.first_notification_delay()));

  s->add_stalk_on(obj.notification_options() & configuration::service::ok
                      ? notifier::ok
                      : notifier::none);
  s->add_stalk_on(obj.notification_options() & configuration::service::warning
                      ? notifier::warning
                      : notifier::none);
  s->add_stalk_on(obj.notification_options() & configuration::service::unknown
                      ? notifier::unknown
                      : notifier::none);
  s->add_stalk_on(obj.notification_options() & configuration::service::critical
                      ? notifier::critical
                      : notifier::none);

  s->set_notification_period(obj.notification_period());
  s->set_check_period(obj.check_period());
  s->set_flap_detection_enabled(obj.flap_detection_enabled());
  s->set_low_flap_threshold(obj.low_flap_threshold());
  s->set_high_flap_threshold(obj.high_flap_threshold());

  s->set_flap_detection_on(notifier::none);
  s->add_flap_detection_on(obj.flap_detection_options() &
                                   configuration::service::ok
                               ? notifier::ok
                               : notifier::none);
  s->add_flap_detection_on(obj.flap_detection_options() &
                                   configuration::service::warning
                               ? notifier::warning
                               : notifier::none);
  s->add_flap_detection_on(obj.flap_detection_options() &
                                   configuration::service::unknown
                               ? notifier::unknown
                               : notifier::none);
  s->add_flap_detection_on(obj.flap_detection_options() &
                                   configuration::service::critical
                               ? notifier::critical
                               : notifier::none);

  s->set_process_performance_data(static_cast<int>(obj.process_perf_data()));
  s->set_check_freshness(obj.check_freshness());
  s->set_freshness_threshold(obj.freshness_threshold());
  s->set_accept_passive_checks(obj.checks_passive());
  s->set_event_handler(obj.event_handler());
  s->set_checks_enabled(obj.checks_active());
  s->set_retain_status_information(
      static_cast<bool>(obj.retain_status_information()));
  s->set_retain_nonstatus_information(
      static_cast<bool>(obj.retain_nonstatus_information()));
  s->set_notifications_enabled(obj.notifications_enabled());
  s->set_obsess_over(obj.obsess_over_service());
  s->set_notes(obj.notes());
  s->set_notes_url(obj.notes_url());
  s->set_action_url(obj.action_url());
  s->set_icon_image(obj.icon_image());
  s->set_icon_image_alt(obj.icon_image_alt());
  s->set_is_volatile(obj.is_volatile());
  s->set_timezone(obj.timezone());
  s->set_host_id(obj.host_id());
  s->set_service_id(obj.service_id());
  s->set_acknowledgement_timeout(obj.get_acknowledgement_timeout() *
                                 config->interval_length());
  s->set_recovery_notification_delay(obj.recovery_notification_delay());

  // Contacts.
  if (obj.contacts() != obj_old.contacts()) {
    // Delete old contacts.
    s->get_contacts().clear();

    // Add contacts to host.
    for (set_string::const_iterator it(obj.contacts().begin()),
         end(obj.contacts().end());
         it != end; ++it)
      s->get_contacts().insert({*it, nullptr});
  }

  // Contact groups.
  if (obj.contactgroups() != obj_old.contactgroups()) {
    // Delete old contact groups.
    s->get_contactgroups().clear();

    // Add contact groups to host.
    for (set_string::const_iterator it(obj.contactgroups().begin()),
         end(obj.contactgroups().end());
         it != end; ++it)
      s->get_contactgroups().insert({*it, nullptr});
  }

  // Custom variables.
  if (obj.customvariables() != obj_old.customvariables()) {
    for (auto& c : s->custom_variables) {
      if (c.second.is_sent()) {
        timeval tv(get_broker_timestamp(nullptr));
        broker_custom_variable(
            NEBTYPE_SERVICECUSTOMVARIABLE_DELETE, NEBFLAG_NONE, NEBATTR_NONE,
            s.get(), c.first.c_str(), c.second.get_value().c_str(), &tv);
      }
    }
    s->custom_variables.clear();

    for (auto& c : obj.customvariables()) {
      s->custom_variables[c.first] = c.second;

      if (c.second.is_sent()) {
        timeval tv(get_broker_timestamp(nullptr));
        broker_custom_variable(NEBTYPE_SERVICECUSTOMVARIABLE_ADD, NEBFLAG_NONE,
                               NEBATTR_NONE, s.get(), c.first.c_str(),
                               c.second.get_value().c_str(), &tv);
      }
    }
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_service_data(NEBTYPE_SERVICE_UPDATE, NEBFLAG_NONE,
                               NEBATTR_NONE, s.get(), CMD_NONE, MODATTR_ALL,
                               MODATTR_ALL, &tv);
}

/**
 *  Remove old service.
 *
 *  @param[in] obj  The new service to remove from the monitoring
 *                  engine.
 */
void applier::service::remove_object(configuration::service const& obj) {
  std::string const& host_name(*obj.hosts().begin());
  std::string const& service_description(obj.service_description());

  assert(obj.key().first);
  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Removing service '" << service_description << "' of host '"
      << host_name << "'.";

  // Find service.
  service_id_map::iterator it(engine::service::services_by_id.find(obj.key()));
  if (it != engine::service::services_by_id.end()) {
    std::shared_ptr<engine::service> svc(it->second);

    // Remove service comments.
    comment::delete_service_comments(host_name, service_description);

    // Remove service downtimes.
    downtime_manager::instance()
        .delete_downtime_by_hostname_service_description_start_time_comment(
            host_name, service_description, (time_t)0, "");

    // Remove events related to this service.
    applier::scheduler::instance().remove_service(obj);

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_service_data(NEBTYPE_SERVICE_DELETE, NEBFLAG_NONE,
                                 NEBATTR_NONE, svc.get(), CMD_NONE, MODATTR_ALL,
                                 MODATTR_ALL, &tv);

    // Unregister service.
    engine::service::services.erase({host_name, service_description});
    engine::service::services_by_id.erase(it);
  }

  // Remove service from the global configuration set.
  config->services().erase(obj);
}

/**
 *  Resolve a service.
 *
 *  @param[in] obj  Service object.
 */
void applier::service::resolve_object(configuration::service const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Resolving service '" << obj.service_description() << "' of host '"
      << *obj.hosts().begin() << "'.";

  // Find service.
  service_id_map::iterator it(engine::service::services_by_id.find(obj.key()));
  if (engine::service::services_by_id.end() == it)
    throw engine_error() << "Cannot resolve non-existing service '"
                         << obj.service_description() << "' of host '"
                         << *obj.hosts().begin() << "'";

  // Remove service group links.
  it->second->get_parent_groups().clear();

  // Find host and adjust its counters.
  host_id_map::iterator hst(engine::host::hosts_by_id.find(it->first.first));
  if (hst != engine::host::hosts_by_id.end()) {
    hst->second->set_total_services(hst->second->get_total_services() + 1);
    hst->second->set_total_service_check_interval(
        hst->second->get_total_service_check_interval() +
        static_cast<uint64_t>(it->second->get_check_interval()));
  }

  // Resolve service.
  it->second->resolve(config_warnings, config_errors);
}

/**
 *  Expand service instance memberships.
 *
 *  @param[in]  obj Target service.
 *  @param[out] s   Configuration state.
 */
void applier::service::_expand_service_memberships(configuration::service& obj,
                                                   configuration::state& s) {
  // Browse service groups.
  for (set_string::const_iterator it(obj.servicegroups().begin()),
       end(obj.servicegroups().end());
       it != end; ++it) {
    // Find service group.
    configuration::set_servicegroup::iterator it_group(
        s.servicegroups_find(*it));
    if (it_group == s.servicegroups().end())
      throw(engine_error() << "Could not add service '"
                           << obj.service_description() << "' of host '"
                           << *obj.hosts().begin()
                           << "' to non-existing service group '" << *it
                           << "'");

    // Remove service group from state.
    configuration::servicegroup backup(*it_group);
    s.servicegroups().erase(it_group);

    // Add service to service members.
    backup.members().insert(
        std::make_pair(*obj.hosts().begin(), obj.service_description()));

    // Reinsert service group.
    s.servicegroups().insert(backup);
  }

  return;
}

/**
 *  @brief Inherits special variables from host.
 *
 *  These special variables, if not defined are inherited from host.
 *  They are contact_groups, notification_interval and
 *  notification_period.
 *
 *  @param[in,out] obj Target service.
 *  @param[in]     s   Configuration state.
 */
void applier::service::_inherits_special_vars(configuration::service& obj,
                                              configuration::state const& s) {
  // Detect if any special variable has not been defined.
  if (!obj.host_id() || !obj.contacts_defined() ||
      !obj.contactgroups_defined() || !obj.notification_interval_defined() ||
      !obj.notification_period_defined() || !obj.timezone_defined()) {
    // Find host.
    configuration::set_host::const_iterator it(
        s.hosts_find(obj.hosts().begin()->c_str()));
    if (it == s.hosts().end())
      throw engine_error()
          << "Could not inherit special variables for service '"
          << obj.service_description() << "': host '" << *obj.hosts().begin()
          << "' does not exist";

    // Inherits variables.
    if (!obj.host_id())
      obj.set_host_id(it->host_id());
    if (!obj.contacts_defined() && !obj.contactgroups_defined()) {
      obj.contacts() = it->contacts();
      obj.contactgroups() = it->contactgroups();
    }
    if (!obj.notification_interval_defined())
      obj.notification_interval(it->notification_interval());
    if (!obj.notification_period_defined())
      obj.notification_period(it->notification_period());
    if (!obj.timezone_defined())
      obj.timezone(it->timezone());
  }
}

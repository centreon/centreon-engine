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

#include <algorithm>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/scheduler.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/deleter/servicesmember.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::host::host() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::host::host(applier::host const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::host::~host() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::host& applier::host::operator=(applier::host const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new host.
 *
 *  @param[in] obj  The new host to add into the monitoring engine.
 */
void applier::host::add_object(
                      configuration::host const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new host '" << obj.host_name() << "'.";

  // Add host to the global configuration set.
  config->hosts().insert(obj);

  // Create host.
  std::shared_ptr<com::centreon::engine::host> h =
    std::make_shared<com::centreon::engine::host>(
        obj.host_id(),
        obj.host_name(),
        obj.display_name(),
        obj.alias(),
        obj.address(),
        obj.check_period(),
        obj.initial_state(),
        obj.check_interval(),
        obj.retry_interval(),
        obj.max_check_attempts(),
        static_cast<bool>(obj.notification_options()
                          & configuration::host::up),
        static_cast<bool>(obj.notification_options()
                          & configuration::host::down),
        static_cast<bool>(obj.notification_options()
                          & configuration::host::unreachable),
        static_cast<bool>(obj.notification_options()
                          & configuration::host::flapping),
        static_cast<bool>(obj.notification_options()
                          & configuration::host::downtime),
        obj.notification_interval(),
        obj.first_notification_delay(),
        obj.notification_period(),
        obj.notifications_enabled(),
        obj.check_command(),
        obj.checks_active(),
        obj.checks_passive(),
        obj.event_handler(),
        obj.event_handler_enabled(),
        obj.flap_detection_enabled(),
        obj.low_flap_threshold(),
        obj.high_flap_threshold(),
        static_cast<bool>(obj.flap_detection_options()
                          & configuration::host::up),
        static_cast<bool>(obj.flap_detection_options()
                          & configuration::host::down),
        static_cast<bool>(obj.flap_detection_options()
                          & configuration::host::unreachable),
        static_cast<bool>(obj.stalking_options()
                          & configuration::host::up),
        static_cast<bool>(obj.stalking_options()
                          & configuration::host::down),
        static_cast<bool>(obj.stalking_options()
                          & configuration::host::unreachable),
        obj.process_perf_data(),
        obj.check_freshness(),
        obj.freshness_threshold(),
        obj.notes(),
        obj.notes_url(),
        obj.action_url(),
        obj.icon_image(),
        obj.icon_image_alt(),
        obj.vrml_image(),
        obj.statusmap_image(),
        obj.coords_2d().x(),
        obj.coords_2d().y(),
        obj.have_coords_2d(),
        obj.coords_3d().x(),
        obj.coords_3d().y(),
        obj.coords_3d().z(),
        obj.have_coords_3d(),
        true, // should_be_drawn, enabled by Nagios
        obj.retain_status_information(),
        obj.retain_nonstatus_information(),
        obj.obsess_over_host());


  state::instance().hosts()[obj.host_id()] = h;
  com::centreon::engine::host::hosts.insert({h->get_name(), h});

  host_other_props[obj.host_name()].initial_notif_time = 0;
  host_other_props[obj.host_name()].should_reschedule_current_check = false;
  host_other_props[obj.host_name()].timezone = obj.timezone();
  host_other_props[obj.host_name()].host_id = obj.host_id();
  host_other_props[obj.host_name()].acknowledgement_timeout
    = obj.get_acknowledgement_timeout() * config->interval_length();
  host_other_props[obj.host_name()].last_acknowledgement = 0;
  host_other_props[obj.host_name()].recovery_notification_delay
    = obj.recovery_notification_delay();
  host_other_props[obj.host_name()].recovery_been_sent
    = true;

  // Contacts.
  for (set_string::const_iterator
         it(obj.contacts().begin()),
         end(obj.contacts().end());
       it != end;
       ++it)
    h->contacts.insert({*it, nullptr});

  // Contact groups.
  for (set_string::const_iterator
         it(obj.contactgroups().begin()),
         end(obj.contactgroups().end());
       it != end;
       ++it)
    h->contact_groups.insert({*it, nullptr});

  // Custom variables.
  for (map_customvar::const_iterator
         it(obj.customvariables().begin()),
         end(obj.customvariables().end());
       it != end;
       ++it)
    h->custom_variables.insert({it->first, it->second});

  // Parents.
  for (set_string::const_iterator
         it(obj.parents().begin()),
         end(obj.parents().end());
       it != end;
       ++it)
    h->add_parent_host(*it);

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_host_data(
    NEBTYPE_HOST_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    h.get(),
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);

  return ;
}

/**
 *  @brief Expand a host.
 *
 *  During expansion, the host will be added to its host groups. These
 *  will be modified in the state.
 *
 *  @param[int,out] s   Configuration state.
 */
void applier::host::expand_objects(configuration::state& s) {
  // Browse all hosts.
  for (configuration::set_host::iterator
         it_host(s.hosts().begin()),
         end_host(s.hosts().end());
       it_host != end_host;
       ++it_host) {

    // Should custom variables be sent to broker ?
    for (map_customvar::const_iterator
           it(it_host->customvariables().begin()),
           end(it_host->customvariables().end());
         it != end;
         ++it) {
      if (!s.enable_macros_filter()
          || s.macros_filter().find(it->first) != s.macros_filter().end()) {
        customvariable& cv(const_cast<customvariable&>(it->second));
        cv.set_sent(true);
      }
    }

    // Browse current host's groups.
    for (set_string::const_iterator
           it_group(it_host->hostgroups().begin()),
           end_group(it_host->hostgroups().end());
         it_group != end_group;
         ++it_group) {
      // Find host group.
      configuration::set_hostgroup::iterator
        group(s.hostgroups_find(*it_group));
      if (group == s.hostgroups().end())
        throw (engine_error() << "Could not add host '"
               << it_host->host_name() << "' to non-existing host group '"
               << *it_group << "'");

      // Remove host group from state.
      configuration::hostgroup backup(*group);
      s.hostgroups().erase(group);

      // Add host to group members.
      backup.members().insert(it_host->host_name());

      // Reinsert host group.
      s.hostgroups().insert(backup);
    }
  }

  return ;
}

/**
 *  Modified host.
 *
 *  @param[in] obj  The new host to modify into the monitoring engine.
 */
void applier::host::modify_object(
                      configuration::host const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying host '" << obj.host_name() << "'.";

  // Find the configuration object.
  set_host::iterator it_cfg(config->hosts_find(obj.key()));
  if (it_cfg == config->hosts().end())
    throw (engine_error() << "Cannot modify non-existing host '"
           << obj.host_name() << "'");

  // Find host object.
  umap<unsigned int,
       std::shared_ptr<com::centreon::engine::host>>::iterator
    it_obj(applier::state::instance().hosts_find(obj.key()));
  if (it_obj == applier::state::instance().hosts().end())
    throw (engine_error() << "Could not modify non-existing "
           << "host object '" << obj.host_name() << "'");
  com::centreon::engine::host* h(it_obj->second.get());

  // Update the global configuration set.
  configuration::host obj_old(*it_cfg);
  config->hosts().erase(it_cfg);
  config->hosts().insert(obj);

  // Modify properties.
  h->set_name(obj.host_name());
  h->set_display_name(obj.display_name());
  if (!obj.alias().empty())
    h->set_alias(obj.alias());
  else
    h->set_alias(obj.host_name());
  h->set_address(obj.address());
  if (obj.check_period().empty())
  h->set_check_period(obj.check_period());
  h->set_initial_state(static_cast<int>(obj.initial_state()));
  h->set_check_interval(static_cast<double>(obj.check_interval()));
  h->set_retry_interval(static_cast<double>(obj.retry_interval()));
  h->set_max_attempts(static_cast<int>(obj.max_check_attempts()));
  h->set_notify_on_recovery(static_cast<int>(static_cast<bool>(
    obj.notification_options() & configuration::host::up)));
  h->set_notify_on_down(static_cast<int>(static_cast<bool>(
    obj.notification_options() & configuration::host::down)));
  h->set_notify_on_unreachable(static_cast<int>(static_cast<bool>(
    obj.notification_options() & configuration::host::unreachable)));
  h->set_notify_on_flapping(static_cast<int>(static_cast<bool>(
    obj.notification_options() & configuration::host::flapping)));
  h->set_notify_on_downtime(static_cast<int>(static_cast<bool>(
    obj.notification_options() & configuration::host::downtime)));
  h->set_notification_interval(static_cast<double>(obj.notification_interval()));
  h->set_first_notification_delay(static_cast<double>(obj.first_notification_delay()));
  h->set_notification_period(obj.notification_period());
  h->set_notifications_enabled(static_cast<int>(obj.notifications_enabled()));
  h->set_host_check_command(obj.check_command());
  h->set_checks_enabled(static_cast<int>(obj.checks_active()));
  h->set_accept_passive_host_checks(static_cast<int>(obj.checks_passive()));
  h->set_event_handler(obj.event_handler());
  h->set_event_handler_enabled(static_cast<int>(obj.event_handler_enabled()));
  h->set_flap_detection_enabled(static_cast<int>(obj.flap_detection_enabled()));
  h->set_low_flap_threshold(static_cast<double>(obj.low_flap_threshold()));
  h->set_high_flap_threshold(static_cast<double>(obj.high_flap_threshold()));
  h->set_flap_detection_on_up(static_cast<int>(static_cast<bool>(
    obj.flap_detection_options() & configuration::host::up)));
  h->set_flap_detection_on_down(static_cast<int>(static_cast<bool>(
    obj.flap_detection_options() & configuration::host::down)));
  h->set_flap_detection_on_unreachable(static_cast<int>(static_cast<bool>(
    obj.flap_detection_options() & configuration::host::unreachable)));
  h->set_stalk_on_up(static_cast<int>(static_cast<bool>(
    obj.stalking_options() & configuration::host::up)));
  h->set_stalk_on_down(static_cast<int>(static_cast<bool>(
    obj.stalking_options() & configuration::host::down)));
  h->set_stalk_on_unreachable(static_cast<int>(static_cast<bool>(
    obj.stalking_options() & configuration::host::unreachable)));
  h->set_process_performance_data(static_cast<int>(obj.process_perf_data()));
  h->set_check_freshness(static_cast<int>(obj.check_freshness()));
  h->set_freshness_threshold(static_cast<int>(obj.freshness_threshold()));
  h->set_notes(obj.notes());
  h->set_notes_url(obj.notes_url());
  h->set_action_url(obj.action_url());
  h->set_icon_image(obj.icon_image());
  h->set_icon_image_alt(obj.icon_image_alt());
  h->set_vrml_image(obj.vrml_image());
  h->set_statusmap_image(obj.statusmap_image());
  h->set_x_2d(obj.coords_2d().x());
  h->set_y_2d(obj.coords_2d().y());
  h->set_have_2d_coords(static_cast<int>(obj.have_coords_2d()));
  h->set_x_3d(obj.coords_3d().x());
  h->set_y_3d(obj.coords_3d().y());
  h->set_z_3d(obj.coords_3d().z());
  h->set_have_3d_coords(static_cast<int>(obj.have_coords_3d()));
  h->set_retain_status_information(static_cast<int>(obj.retain_status_information()));
  h->set_retain_nonstatus_information(static_cast<int>(obj.retain_nonstatus_information()));
  h->set_obsess_over_host(static_cast<int>(obj.obsess_over_host()));
  host_other_props[obj.host_name()].timezone = obj.timezone();
  host_other_props[obj.host_name()].host_id = obj.host_id();
  host_other_props[obj.host_name()].acknowledgement_timeout
    = obj.get_acknowledgement_timeout() * config->interval_length();
  host_other_props[obj.host_name()].recovery_notification_delay
    = obj.recovery_notification_delay();

  // Contacts.
  if (obj.contacts() != obj_old.contacts()) {
    // Delete old contacts.
    h->contacts.clear();

    // Add contacts to host.
    for (set_string::const_iterator
           it(obj.contacts().begin()),
           end(obj.contacts().end());
         it != end;
         ++it)
      h->contacts.insert({*it, nullptr});
  }

  // Contact groups.
  if (obj.contactgroups() != obj_old.contactgroups()) {
    // Delete old contact groups.
    h->contact_groups.clear();

    // Add contact groups to host.
    for (set_string::const_iterator
           it(obj.contactgroups().begin()),
           end(obj.contactgroups().end());
         it != end;
         ++it)
      h->contact_groups.insert({*it, nullptr});
  }

  // Custom variables.
  if (obj.customvariables() != obj_old.customvariables())
    h->custom_variables = obj.customvariables();

  // Parents.
  if (obj.parents() != obj_old.parents()) {
    // Delete old parents.
    {
      timeval tv(get_broker_timestamp(NULL));
      for (host_map::iterator
             it(h->parent_hosts.begin()),
             end(h->parent_hosts.end());
           it != end;
           it++)
        broker_relation_data(
          NEBTYPE_PARENT_DELETE,
          NEBFLAG_NONE,
          NEBATTR_NONE,
          it->second.get(),
          NULL,
          h,
          NULL,
          &tv);
    }
    h->parent_hosts.clear();

    // Create parents.
    for (set_string::const_iterator
           it(obj.parents().begin()),
           end(obj.parents().end());
         it != end;
         ++it)
      h->add_parent_host(*it);
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_host_data(
    NEBTYPE_HOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    h,
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);

  return ;
}

/**
 *  Remove old host.
 *
 *  @param[in] obj The new host to remove from the monitoring engine.
 */
void applier::host::remove_object(
                      configuration::host const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing host '" << obj.host_name() << "'.";

  // Find host.
  umap<unsigned int, std::shared_ptr<com::centreon::engine::host> >::iterator
    it(applier::state::instance().hosts_find(obj.key()));
  if (it != applier::state::instance().hosts().end()) {
    com::centreon::engine::host* hst(it->second.get());

    // Remove host comments.
    delete_all_host_comments(obj.host_name().c_str());

    // Remove host downtimes.
    downtimes::downtime_manager::instance().delete_downtime_by_hostname_service_description_start_time_comment(
      obj.host_name(),
      NULL,
      (time_t)0,
      NULL);

    // Remove events related to this host.
    applier::scheduler::instance().remove_host(obj);

    // Remove host from its list.
    com::centreon::engine::host::hosts.erase(hst->get_name());

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_host_data(
      NEBTYPE_HOST_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      hst,
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);

    // Erase host object (will effectively delete the object).
    host_other_props.erase(obj.host_name());
    applier::state::instance().hosts().erase(it);
  }

  // Remove host from the global configuration set.
  config->hosts().erase(obj);

  return ;
}

/**
 *  Resolve a host.
 *
 *  @param[in] obj  Host object.
 */
void applier::host::resolve_object(
                      configuration::host const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving host '" << obj.host_name() << "'.";

  // If it is the very first host to be resolved,
  // remove all the child backlinks of all the hosts.
  // It is necessary to do it only once to prevent the removal
  // of valid child backlinks.
  if (obj == *config->hosts().begin()) {
    for (umap<unsigned int,
              std::shared_ptr<com::centreon::engine::host> >::iterator
         it(applier::state::instance().hosts().begin()),
         end(applier::state::instance().hosts().end()); it != end; ++it)
      it->second->child_hosts.clear();
  }

  // Find host.
  umap<unsigned int, std::shared_ptr<com::centreon::engine::host> >::iterator
    it(applier::state::instance().hosts_find(obj.key()));
  if (applier::state::instance().hosts().end() == it)
    throw (engine_error() << "Cannot resolve non-existing host '"
           << obj.host_name() << "'");

  // Remove service backlinks.
  deleter::listmember(it->second->services, &deleter::servicesmember);

  // Remove host group links.
  deleter::listmember(it->second->hostgroups_ptr, &deleter::objectlist);

  // Reset host counters.
  it->second->set_total_services(0);
  it->second->set_total_service_check_interval(0);

  // Resolve host.
  if (!check_host(it->second.get(), &config_warnings, &config_errors))
    throw (engine_error() << "Cannot resolve host '"
           << obj.host_name() << "'");

  return ;
}

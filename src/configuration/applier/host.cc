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

#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/member.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

// XXX : update the event_list_low and event_list_high

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
 *  @param[in] obj The new host to add into the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::host::add_object(
                      configuration::host const& obj,
                      configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new host '" << obj.host_name() << "'.";

  // Create host.
  host_struct*
    h(add_host(
        obj.host_name().c_str(),
        NULL_IF_EMPTY(obj.display_name()),
        NULL_IF_EMPTY(obj.alias()),
        NULL_IF_EMPTY(obj.address()),
        NULL_IF_EMPTY(obj.check_period()),
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
        NULL_IF_EMPTY(obj.notification_period()),
        obj.notifications_enabled(),
        NULL_IF_EMPTY(obj.check_command()),
        obj.checks_active(),
        obj.checks_passive(),
        NULL_IF_EMPTY(obj.event_handler()),
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
        false, // failure_prediction_enabled
        NULL, // failure_prediction_options
        obj.check_freshness(),
        obj.freshness_threshold(),
        NULL_IF_EMPTY(obj.notes()),
        NULL_IF_EMPTY(obj.notes_url()),
        NULL_IF_EMPTY(obj.action_url()),
        NULL_IF_EMPTY(obj.icon_image()),
        NULL_IF_EMPTY(obj.icon_image_alt()),
        NULL_IF_EMPTY(obj.vrml_image()),
        NULL_IF_EMPTY(obj.statusmap_image()),
        obj.coords_2d().x(),
        obj.coords_2d().y(),
        obj.have_coords_2d(),
        obj.coords_3d().x(),
        obj.coords_3d().y(),
        obj.coords_3d().z(),
        obj.have_coords_3d(),
        false, // should_be_drawn
        obj.retain_status_information(),
        obj.retain_nonstatus_information(),
        obj.obsess_over_host()));
  if (!h)
    throw (engine_error() << "Error: Could not register host '"
           << obj.host_name() << "'.");

  // Contacts.
  for (list_string::const_iterator
         it(obj.contacts().begin()),
         end(obj.contacts().end());
       it != end;
       ++it)
    if (!add_contact_to_host(h, it->c_str()))
      throw (engine_error() << "Error: Could not add contact '"
             << *it << "' to host '" << obj.host_name() << "'.");

  // Contact groups.
  for (list_string::const_iterator
         it(obj.contactgroups().begin()),
         end(obj.contactgroups().end());
       it != end;
       ++it)
    if (!add_contactgroup_to_host(h, it->c_str()))
      throw (engine_error() << "Error: Could not add contact group '"
             << *it << "' to host '" << obj.host_name() << "'.");

  // Custom variables.
  for (properties::const_iterator
         it(obj.customvariables().begin()),
         end(obj.customvariables().end());
       it != end;
       ++it)
    if (!add_custom_variable_to_host(
           h,
           it->first.c_str(),
           it->second.c_str()))
      throw (engine_error() << "Error: Could not add custom variable '"
             << it->first << "' to host '" << obj.host_name() << "'.");

  // Parents.
  for (list_string::const_iterator
         it(obj.parents().begin()),
         end(obj.parents().end());
       it != end;
       ++it)
    if (!add_parent_host_to_host(h, it->c_str()))
      throw (engine_error() << "Error: Could not add parent '"
             << *it << "' to host '" << obj.host_name() << "'.");

  // Host groups.
  for (list_string::const_iterator
         it(obj.hostgroups().begin()),
         end(obj.hostgroups().end());
       it != end;
       ++it) {
    // Find host group object.
    umap<std::string, shared_ptr<hostgroup_struct> >::iterator
      it2(applier::state::instance().hostgroups().find(*it));
    if (it2 == applier::state::instance().hostgroups().end())
      throw (engine_error() << "Error: Could not add host '"
             << obj.host_name() << "' to non-existent host group '"
             << *it << "'.");

    // Add host to host group.
    if (!add_host_to_hostgroup(
           it2->second.get(),
           obj.host_name().c_str()))
      throw (engine_error() << "Error: Could not add host '"
             << obj.host_name() << "' to host group '" << *it << "'.");
  }

  return ;
}

/**
 *  Modified host.
 *
 *  @param[in] obj The new host to modify into the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::host::modify_object(
                      configuration::host const& obj,
                      configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying host '" << obj.host_name() << "'.";

  // Modify command.
  shared_ptr<host_struct>&
    h(applier::state::instance().hosts()[obj.host_name()]);
  modify_if_different(
    h->display_name,
    NULL_IF_EMPTY(obj.display_name()));
  modify_if_different(h->alias, NULL_IF_EMPTY(obj.alias()));
  modify_if_different(h->address, NULL_IF_EMPTY(obj.address()));
  modify_if_different(
    h->check_period,
    NULL_IF_EMPTY(obj.check_period()));
  modify_if_different(
    h->initial_state,
    static_cast<int>(obj.initial_state()));
  modify_if_different(
    h->check_interval,
    static_cast<double>(obj.check_interval()));
  modify_if_different(
    h->retry_interval,
    static_cast<double>(obj.retry_interval()));
  modify_if_different(
    h->max_attempts,
    static_cast<int>(obj.max_check_attempts()));
  modify_if_different(
    h->notify_on_recovery,
    static_cast<int>(static_cast<bool>(
      obj.notification_options() & configuration::host::up)));
  modify_if_different(
    h->notify_on_down,
    static_cast<int>(static_cast<bool>(
      obj.notification_options() & configuration::host::down)));
  modify_if_different(
    h->notify_on_unreachable,
    static_cast<int>(static_cast<bool>(
      obj.notification_options() & configuration::host::unreachable)));
  modify_if_different(
    h->notify_on_flapping,
    static_cast<int>(static_cast<bool>(
      obj.notification_options() & configuration::host::flapping)));
  modify_if_different(
    h->notify_on_downtime,
    static_cast<int>(static_cast<bool>(
      obj.notification_options() & configuration::host::downtime)));
  modify_if_different(
    h->notification_interval,
    static_cast<double>(obj.notification_interval()));
  modify_if_different(
    h->first_notification_delay,
    static_cast<double>(obj.first_notification_delay()));
  modify_if_different(
    h->notification_period,
    NULL_IF_EMPTY(obj.notification_period()));
  modify_if_different(
    h->notifications_enabled,
    static_cast<int>(obj.notifications_enabled()));
  modify_if_different(
    h->host_check_command,
    NULL_IF_EMPTY(obj.check_command()));
  modify_if_different(
    h->checks_enabled,
    static_cast<int>(obj.checks_active()));
  modify_if_different(
    h->accept_passive_host_checks,
    static_cast<int>(obj.checks_passive()));
  modify_if_different(
    h->event_handler,
    NULL_IF_EMPTY(obj.event_handler()));
  modify_if_different(
    h->flap_detection_enabled,
    static_cast<int>(obj.flap_detection_enabled()));
  modify_if_different(
    h->low_flap_threshold,
    static_cast<double>(obj.low_flap_threshold()));
  modify_if_different(
    h->high_flap_threshold,
    static_cast<double>(obj.high_flap_threshold()));
  modify_if_different(
    h->flap_detection_on_up,
    static_cast<int>(static_cast<bool>(
      obj.flap_detection_options() & configuration::host::up)));
  modify_if_different(
    h->flap_detection_on_down,
    static_cast<int>(static_cast<bool>(
      obj.flap_detection_options() & configuration::host::down)));
  modify_if_different(
    h->flap_detection_on_unreachable,
    static_cast<int>(static_cast<bool>(
      obj.flap_detection_options() & configuration::host::unreachable)));
  modify_if_different(
    h->stalk_on_up,
    static_cast<int>(static_cast<bool>(
      obj.stalking_options() & configuration::host::up)));
  modify_if_different(
    h->stalk_on_down,
    static_cast<int>(static_cast<bool>(
      obj.stalking_options() & configuration::host::down)));
  modify_if_different(
    h->stalk_on_unreachable,
    static_cast<int>(static_cast<bool>(
      obj.stalking_options() & configuration::host::unreachable)));
  modify_if_different(
    h->process_performance_data,
    static_cast<int>(obj.process_perf_data()));
  modify_if_different(
    h->check_freshness,
    static_cast<int>(obj.check_freshness()));
  modify_if_different(
    h->freshness_threshold,
    static_cast<int>(obj.freshness_threshold()));
  modify_if_different(h->notes, NULL_IF_EMPTY(obj.notes()));
  modify_if_different(h->notes_url, NULL_IF_EMPTY(obj.notes_url()));
  modify_if_different(h->action_url, NULL_IF_EMPTY(obj.action_url()));
  modify_if_different(h->icon_image, NULL_IF_EMPTY(obj.icon_image()));
  modify_if_different(
    h->icon_image_alt,
    NULL_IF_EMPTY(obj.icon_image_alt()));
  modify_if_different(h->vrml_image, NULL_IF_EMPTY(obj.vrml_image()));
  modify_if_different(
    h->statusmap_image,
    NULL_IF_EMPTY(obj.statusmap_image()));
  modify_if_different(h->x_2d, obj.coords_2d().x());
  modify_if_different(h->y_2d, obj.coords_2d().y());
  modify_if_different(
    h->have_2d_coords,
    static_cast<int>(obj.have_coords_2d()));
  modify_if_different(h->x_3d, obj.coords_3d().x());
  modify_if_different(h->y_3d, obj.coords_3d().y());
  modify_if_different(h->z_3d, obj.coords_3d().z());
  modify_if_different(
    h->have_3d_coords,
    static_cast<int>(obj.have_coords_3d()));
  modify_if_different(
    h->retain_status_information,
    static_cast<int>(obj.retain_status_information()));
  modify_if_different(
    h->retain_nonstatus_information,
    static_cast<int>(obj.retain_nonstatus_information()));
  modify_if_different(
    h->obsess_over_host,
    static_cast<int>(obj.obsess_over_host()));

  // XXX : contacts
  // XXX : contactgroups
  // XXX : customvariables
  // XXX : parents
  // XXX : hostgroups

  return ;
}

/**
 *  Remove old host.
 *
 *  @param[in] obj The new host to remove from the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::host::remove_object(
                      configuration::host const& obj,
                      configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing host '" << obj.host_name() << "'.";

  // Unregister host.
  unregister_object<host_struct, &host_struct::name>(
    &host_list,
    obj.host_name().c_str());
  applier::state::instance().hosts().erase(obj.host_name());

  return ;
}

/**
 *  Resolve a host.
 *
 *  @param[in] obj Host object.
 *  @param[in] s   Configuration being applied.
 */
void applier::host::resolve_object(
                      configuration::host const& obj,
                      configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving host '" << obj.host_name() << "'.";

  // Find host.
  umap<std::string, shared_ptr<host_struct> >::iterator
    it(applier::state::instance().hosts().find(obj.host_name()));
  if (applier::state::instance().hosts().end() == it)
    throw (engine_error() << "Error: Cannot resolve non-existing host '"
           << obj.host_name() << "'.");

  // Resolve host.
  if (!check_host(it->second.get(), NULL, NULL))
    throw (engine_error() << "Error: Cannot resolve host '"
           << obj.host_name() << "'.");

  return ;
}

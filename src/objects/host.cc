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
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/host.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/misc/object.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/objects/commandsmember.hh"
#include "com/centreon/engine/objects/contactgroupsmember.hh"
#include "com/centreon/engine/objects/contactsmember.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"
#include "com/centreon/engine/objects/host.hh"
#include "com/centreon/engine/objects/hostsmember.hh"
#include "com/centreon/engine/objects/servicesmember.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::misc;

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       host const& obj1,
       host const& obj2) throw () {
  return (is_equal(obj1.name, obj2.name)
          && is_equal(obj1.display_name, obj2.display_name)
          && is_equal(obj1.alias, obj2.alias)
          && is_equal(obj1.address, obj2.address)
          && is_equal(obj1.parent_hosts, obj2.parent_hosts)
          && is_equal(obj1.services, obj2.services)
          && is_equal(obj1.host_check_command, obj2.host_check_command)
          && obj1.initial_state == obj2.initial_state
          && obj1.check_interval == obj2.check_interval
          && obj1.retry_interval == obj2.retry_interval
          && obj1.max_attempts == obj2.max_attempts
          && is_equal(obj1.event_handler, obj2.event_handler)
          && is_equal(obj1.contact_groups, obj2.contact_groups)
          && is_equal(obj1.contacts, obj2.contacts)
          && obj1.notification_interval == obj2.notification_interval
          && obj1.first_notification_delay == obj2.first_notification_delay
          && obj1.notify_on_down == obj2.notify_on_down
          && obj1.notify_on_unreachable == obj2.notify_on_unreachable
          && obj1.notify_on_recovery == obj2.notify_on_recovery
          && obj1.notify_on_flapping == obj2.notify_on_flapping
          && obj1.notify_on_downtime == obj2.notify_on_downtime
          && is_equal(obj1.notification_period, obj2.notification_period)
          && is_equal(obj1.check_period, obj2.check_period)
          && obj1.flap_detection_enabled == obj2.flap_detection_enabled
          && obj1.low_flap_threshold == obj2.low_flap_threshold
          && obj1.high_flap_threshold == obj2.high_flap_threshold
          && obj1.flap_detection_on_up == obj2.flap_detection_on_up
          && obj1.flap_detection_on_down == obj2.flap_detection_on_down
          && obj1.flap_detection_on_unreachable == obj2.flap_detection_on_unreachable
          && obj1.stalk_on_up == obj2.stalk_on_up
          && obj1.stalk_on_down == obj2.stalk_on_down
          && obj1.stalk_on_unreachable == obj2.stalk_on_unreachable
          && obj1.check_freshness == obj2.check_freshness
          && obj1.freshness_threshold == obj2.freshness_threshold
          && obj1.process_performance_data == obj2.process_performance_data
          && obj1.checks_enabled == obj2.checks_enabled
          && obj1.accept_passive_host_checks == obj2.accept_passive_host_checks
          && obj1.event_handler_enabled == obj2.event_handler_enabled
          && obj1.retain_status_information == obj2.retain_status_information
          && obj1.retain_nonstatus_information == obj2.retain_nonstatus_information
          && obj1.failure_prediction_enabled == obj2.failure_prediction_enabled
          && is_equal(obj1.failure_prediction_options, obj2.failure_prediction_options)
          && obj1.obsess_over_host == obj2.obsess_over_host
          && is_equal(obj1.notes, obj2.notes)
          && is_equal(obj1.notes_url, obj2.notes_url)
          && is_equal(obj1.action_url, obj2.action_url)
          && is_equal(obj1.icon_image, obj2.icon_image)
          && is_equal(obj1.icon_image_alt, obj2.icon_image_alt)
          && is_equal(obj1.vrml_image, obj2.vrml_image)
          && is_equal(obj1.statusmap_image, obj2.statusmap_image)
          && obj1.have_2d_coords == obj2.have_2d_coords
          && obj1.x_2d == obj2.x_2d
          && obj1.y_2d == obj2.y_2d
          && obj1.have_3d_coords == obj2.have_3d_coords
          && obj1.x_3d == obj2.x_3d
          && obj1.y_3d == obj2.y_3d
          && obj1.z_3d == obj2.z_3d
          && obj1.should_be_drawn == obj2.should_be_drawn
          && is_equal(obj1.custom_variables, obj2.custom_variables)
          && obj1.problem_has_been_acknowledged == obj2.problem_has_been_acknowledged
          && obj1.acknowledgement_type == obj2.acknowledgement_type
          && obj1.check_type == obj2.check_type
          && obj1.current_state == obj2.current_state
          && obj1.last_state == obj2.last_state
          && obj1.last_hard_state == obj2.last_hard_state
          && is_equal(obj1.plugin_output, obj2.plugin_output)
          && is_equal(obj1.long_plugin_output, obj2.long_plugin_output)
          && is_equal(obj1.perf_data, obj2.perf_data)
          && obj1.state_type == obj2.state_type
          && obj1.current_attempt == obj2.current_attempt
          && obj1.current_event_id == obj2.current_event_id
          && obj1.last_event_id == obj2.last_event_id
          && obj1.current_problem_id == obj2.current_problem_id
          && obj1.last_problem_id == obj2.last_problem_id
          && obj1.latency == obj2.latency
          && obj1.execution_time == obj2.execution_time
          && obj1.is_executing == obj2.is_executing
          && obj1.check_options == obj2.check_options
          && obj1.notifications_enabled == obj2.notifications_enabled
          && obj1.last_host_notification == obj2.last_host_notification
          && obj1.next_host_notification == obj2.next_host_notification
          && obj1.next_check == obj2.next_check
          && obj1.should_be_scheduled == obj2.should_be_scheduled
          && obj1.last_check == obj2.last_check
          && obj1.last_state_change == obj2.last_state_change
          && obj1.last_hard_state_change == obj2.last_hard_state_change
          && obj1.last_time_up == obj2.last_time_up
          && obj1.last_time_down == obj2.last_time_down
          && obj1.last_time_unreachable == obj2.last_time_unreachable
          && obj1.has_been_checked == obj2.has_been_checked
          && obj1.is_being_freshened == obj2.is_being_freshened
          && obj1.notified_on_down == obj2.notified_on_down
          && obj1.notified_on_unreachable == obj2.notified_on_unreachable
          && obj1.current_notification_number == obj2.current_notification_number
          && obj1.no_more_notifications == obj2.no_more_notifications
          && obj1.current_notification_id == obj2.current_notification_id
          && obj1.check_flapping_recovery_notification == obj2.check_flapping_recovery_notification
          && obj1.scheduled_downtime_depth == obj2.scheduled_downtime_depth
          && obj1.pending_flex_downtime == obj2.pending_flex_downtime
          && is_equal(obj1.state_history, obj2.state_history, MAX_STATE_HISTORY_ENTRIES)
          && obj1.state_history_index == obj2.state_history_index
          && obj1.last_state_history_update == obj2.last_state_history_update
          && obj1.is_flapping == obj2.is_flapping
          && obj1.flapping_comment_id == obj2.flapping_comment_id
          && obj1.percent_state_change == obj2.percent_state_change
          && obj1.total_services == obj2.total_services
          && obj1.total_service_check_interval == obj2.total_service_check_interval
          && obj1.modified_attributes == obj2.modified_attributes
          && obj1.circular_path_checked == obj2.circular_path_checked
          && obj1.contains_circular_path == obj2.contains_circular_path);
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool operator!=(
       host const& obj1,
       host const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump host content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The host to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, host const& obj) {
  os << "host {\n"
    "  name:                                 " << chkstr(obj.name) << "\n"
    "  display_name:                         " << chkstr(obj.display_name) << "\n"
    "  alias:                                " << chkstr(obj.alias) << "\n"
    "  address:                              " << chkstr(obj.address) << "\n"
    "  parent_hosts:                         " << chkobj(obj.parent_hosts) << "\n"
    "  child_hosts:                          " << chkobj(obj.child_hosts) << "\n"
    "  services:                             " << chkobj(obj.services) << "\n"
    "  host_check_command:                   " << chkstr(obj.host_check_command) << "\n"
    "  initial_state:                        " << obj.initial_state << "\n"
    "  check_interval:                       " << obj.check_interval << "\n"
    "  retry_interval:                       " << obj.retry_interval << "\n"
    "  max_attempts:                         " << obj.max_attempts << "\n"
    "  event_handler:                        " << chkstr(obj.event_handler) << "\n"
    "  contact_groups:                       " << chkobj(obj.contact_groups) << "\n"
    "  contacts:                             " << chkobj(obj.contacts) << "\n"
    "  notification_interval:                " << obj.notification_interval << "\n"
    "  first_notification_delay:             " << obj.first_notification_delay << "\n"
    "  notify_on_down:                       " << obj.notify_on_down << "\n"
    "  notify_on_unreachable:                " << obj.notify_on_unreachable << "\n"
    "  notify_on_recovery:                   " << obj.notify_on_recovery << "\n"
    "  notify_on_flapping:                   " << obj.notify_on_flapping << "\n"
    "  notify_on_downtime:                   " << obj.notify_on_downtime << "\n"
    "  notification_period:                  " << chkstr(obj.notification_period) << "\n"
    "  check_period:                         " << chkstr(obj.check_period) << "\n"
    "  flap_detection_enabled:               " << obj.flap_detection_enabled << "\n"
    "  low_flap_threshold:                   " << obj.low_flap_threshold << "\n"
    "  high_flap_threshold:                  " << obj.high_flap_threshold << "\n"
    "  flap_detection_on_up:                 " << obj.flap_detection_on_up << "\n"
    "  flap_detection_on_down:               " << obj.flap_detection_on_down << "\n"
    "  flap_detection_on_unreachable:        " << obj.flap_detection_on_unreachable << "\n"
    "  stalk_on_up:                          " << obj.stalk_on_up << "\n"
    "  stalk_on_down:                        " << obj.stalk_on_down << "\n"
    "  stalk_on_unreachable:                 " << obj.stalk_on_unreachable << "\n"
    "  check_freshness:                      " << obj.check_freshness << "\n"
    "  freshness_threshold:                  " << obj.freshness_threshold << "\n"
    "  process_performance_data:             " << obj.process_performance_data << "\n"
    "  checks_enabled:                       " << obj.checks_enabled << "\n"
    "  accept_passive_host_checks:           " << obj.accept_passive_host_checks << "\n"
    "  event_handler_enabled:                " << obj.event_handler_enabled << "\n"
    "  retain_status_information:            " << obj.retain_status_information << "\n"
    "  retain_nonstatus_information:         " << obj.retain_nonstatus_information << "\n"
    "  failure_prediction_enabled:           " << obj.failure_prediction_enabled << "\n"
    "  failure_prediction_options:           " << chkstr(obj.failure_prediction_options) << "\n"
    "  obsess_over_host:                     " << obj.obsess_over_host << "\n"
    "  notes:                                " << chkstr(obj.notes) << "\n"
    "  notes_url:                            " << chkstr(obj.notes_url) << "\n"
    "  action_url:                           " << chkstr(obj.action_url) << "\n"
    "  icon_image:                           " << chkstr(obj.icon_image) << "\n"
    "  icon_image_alt:                       " << chkstr(obj.icon_image_alt) << "\n"
    "  vrml_image:                           " << chkstr(obj.vrml_image) << "\n"
    "  statusmap_image:                      " << chkstr(obj.statusmap_image) << "\n"
    "  have_2d_coords:                       " << obj.have_2d_coords << "\n"
    "  x_2d:                                 " << obj.x_2d << "\n"
    "  y_2d:                                 " << obj.y_2d << "\n"
    "  have_3d_coords:                       " << obj.have_3d_coords << "\n"
    "  x_3d:                                 " << obj.x_3d << "\n"
    "  y_3d:                                 " << obj.y_3d << "\n"
    "  z_3d:                                 " << obj.z_3d << "\n"
    "  should_be_drawn:                      " << obj.should_be_drawn << "\n"
    "  custom_variables:                     " << chkobj(obj.custom_variables) << "\n"
    "  problem_has_been_acknowledged:        " << obj.problem_has_been_acknowledged << "\n"
    "  acknowledgement_type:                 " << obj.acknowledgement_type << "\n"
    "  check_type:                           " << obj.check_type << "\n"
    "  current_state:                        " << obj.current_state << "\n"
    "  last_state:                           " << obj.last_state << "\n"
    "  last_hard_state:                      " << obj.last_hard_state << "\n"
    "  plugin_output:                        " << chkstr(obj.plugin_output) << "\n"
    "  long_plugin_output:                   " << chkstr(obj.long_plugin_output) << "\n"
    "  perf_data:                            " << chkstr(obj.perf_data) << "\n"
    "  state_type:                           " << obj.state_type << "\n"
    "  current_attempt:                      " << obj.current_attempt << "\n"
    "  current_event_id:                     " << obj.current_event_id << "\n"
    "  last_event_id:                        " << obj.last_event_id << "\n"
    "  current_problem_id:                   " << obj.current_problem_id << "\n"
    "  last_problem_id:                      " << obj.last_problem_id << "\n"
    "  latency:                              " << obj.latency << "\n"
    "  execution_time:                       " << obj.execution_time << "\n"
    "  is_executing:                         " << obj.is_executing << "\n"
    "  check_options:                        " << obj.check_options << "\n"
    "  notifications_enabled:                " << obj.notifications_enabled << "\n"
    "  last_host_notification:               " << obj.last_host_notification << "\n"
    "  next_host_notification:               " << obj.next_host_notification << "\n"
    "  next_check:                           " << obj.next_check << "\n"
    "  should_be_scheduled:                  " << obj.should_be_scheduled << "\n"
    "  last_check:                           " << obj.last_check << "\n"
    "  last_state_change:                    " << obj.last_state_change << "\n"
    "  last_hard_state_change:               " << obj.last_hard_state_change << "\n"
    "  last_time_up:                         " << obj.last_time_up << "\n"
    "  last_time_down:                       " << obj.last_time_down << "\n"
    "  last_time_unreachable:                " << obj.last_time_unreachable << "\n"
    "  has_been_checked:                     " << obj.has_been_checked << "\n"
    "  is_being_freshened:                   " << obj.is_being_freshened << "\n"
    "  notified_on_down:                     " << obj.notified_on_down << "\n"
    "  notified_on_unreachable:              " << obj.notified_on_unreachable << "\n"
    "  current_notification_number:          " << obj.current_notification_number << "\n"
    "  no_more_notifications:                " << obj.no_more_notifications << "\n"
    "  current_notification_id:              " << obj.current_notification_id << "\n"
    "  check_flapping_recovery_notification: " << obj.check_flapping_recovery_notification << "\n"
    "  scheduled_downtime_depth:             " << obj.scheduled_downtime_depth << "\n"
    "  pending_flex_downtime:                " << obj.pending_flex_downtime << "\n"
    "  state_history:                        " << obj.state_history << "\n"
    "  state_history_index:                  " << obj.state_history_index << "\n"
    "  last_state_history_update:            " << obj.last_state_history_update << "\n"
    "  is_flapping:                          " << obj.is_flapping << "\n"
    "  flapping_comment_id:                  " << obj.flapping_comment_id << "\n"
    "  percent_state_change:                 " << obj.percent_state_change << "\n"
    "  total_services:                       " << obj.total_services << "\n"
    "  total_service_check_interval:         " << obj.total_service_check_interval << "\n"
    "  modified_attributes:                  " << obj.modified_attributes << "\n"
    "  circular_path_checked:                " << obj.circular_path_checked << "\n"
    "  contains_circular_path:               " << obj.contains_circular_path << "\n"
    "  event_handler_ptr:                    " << obj.event_handler_ptr << "\n"
    "  check_command_ptr:                    " << obj.check_command_ptr << "\n"
    "  check_period_ptr:                     " << obj.check_period_ptr << "\n"
    "  notification_period_ptr:              " << obj.notification_period_ptr << "\n"
    "  hostgroups_ptr:                       " << obj.hostgroups_ptr << "\n"
    "}\n";
  return (os);
}

/**
 *  Add a new host definition.
 *
 *  @param[in] name                          Host name.
 *  @param[in] display_name                  Display name.
 *  @param[in] alias                         Host alias.
 *  @param[in] address                       Host address.
 *  @param[in] check_period                  Check period.
 *  @param[in] initial_state                 Initial host state.
 *  @param[in] check_interval                Normal check interval.
 *  @param[in] retry_interval                Retry check interval.
 *  @param[in] max_attempts                  Max check attempts.
 *  @param[in] notify_up                     Does this host notify when
 *                                           up ?
 *  @param[in] notify_down                   Does this host notify when
 *                                           down ?
 *  @param[in] notify_unreachable            Does this host notify when
 *                                           unreachable ?
 *  @param[in] notify_flapping               Does this host notify for
 *                                           flapping ?
 *  @param[in] notify_downtime               Does this host notify for
 *                                           downtimes ?
 *  @param[in] notification_interval         Notification interval.
 *  @param[in] first_notification_delay      First notification delay.
 *  @param[in] notification_period           Notification period.
 *  @param[in] notifications_enabled         Whether notifications are
 *                                           enabled for this host.
 *  @param[in] check_command                 Active check command name.
 *  @param[in] checks_enabled                Are active checks enabled ?
 *  @param[in] accept_passive_checks         Can we submit passive check
 *                                           results ?
 *  @param[in] event_handler                 Event handler command name.
 *  @param[in] event_handler_enabled         Whether event handler is
 *                                           enabled or not.
 *  @param[in] flap_detection_enabled        Whether flap detection is
 *                                           enabled or not.
 *  @param[in] low_flap_threshold            Low flap threshold.
 *  @param[in] high_flap_threshold           High flap threshold.
 *  @param[in] flap_detection_on_up          Is flap detection enabled
 *                                           for up state ?
 *  @param[in] flap_detection_on_down        Is flap detection enabled
 *                                           for down state ?
 *  @param[in] flap_detection_on_unreachable Is flap detection enabled
 *                                           for unreachable state ?
 *  @param[in] stalk_on_up                   Stalk on up ?
 *  @param[in] stalk_on_down                 Stalk on down ?
 *  @param[in] stalk_on_unreachable          Stalk on unreachable ?
 *  @param[in] process_perfdata              Should host perfdata be
 *                                           processed ?
 *  @param[in] failure_prediction_enabled    Whether or not failure
 *                                           prediction is enabled.
 *  @param[in] check_freshness               Whether or not freshness
 *                                           check is enabled.
 *  @param[in] freshness_threshold           Freshness threshold.
 *  @param[in] notes                         Notes.
 *  @param[in] notes_url                     URL.
 *  @param[in] action_url                    Action URL.
 *  @param[in] icon_image                    Icon image.
 *  @param[in] icon_image_alt                Alternative icon image.
 *  @param[in] vrml_image                    VRML image.
 *  @param[in] statusmap_image               Status-map image.
 *  @param[in] x_2d                          2D x-coord.
 *  @param[in] y_2d                          2D y-coord.
 *  @param[in] have_2d_coords                Whether host has 2D coords.
 *  @param[in] x_3d                          3D x-coord.
 *  @param[in] y_3d                          3D y-coord.
 *  @param[in] z_3d                          3D z-coord.
 *  @param[in] have_3d_coords                Whether host has 3D coords.
 *  @param[in] should_be_drawn               Whether this host should be
 *                                           drawn.
 *  @param[in] retain_status_information     Should Engine retain status
 *                                           information of this host ?
 *  @param[in] retain_nonstatus_information  Should Engine retain
 *                                           non-status information of
 *                                           this host ?
 *  @param[in] obsess_over_host              Should we obsess over this
 *                                           host ?
 *
 *  @return New host.
 */
host* add_host(
        char const* name,
        char const* display_name,
        char const* alias,
        char const* address,
        char const* check_period,
        int initial_state,
        double check_interval,
        double retry_interval,
        int max_attempts,
        int notify_up,
        int notify_down,
        int notify_unreachable,
        int notify_flapping,
        int notify_downtime,
        double notification_interval,
        double first_notification_delay,
        char const* notification_period,
        int notifications_enabled,
        char const* check_command,
        int checks_enabled,
        int accept_passive_checks,
        char const* event_handler,
        int event_handler_enabled,
        int flap_detection_enabled,
        double low_flap_threshold,
        double high_flap_threshold,
        int flap_detection_on_up,
        int flap_detection_on_down,
        int flap_detection_on_unreachable,
        int stalk_on_up,
        int stalk_on_down,
        int stalk_on_unreachable,
        int process_perfdata,
        int failure_prediction_enabled,
        char const* failure_prediction_options,
        int check_freshness,
        int freshness_threshold,
        char const* notes,
        char const* notes_url,
        char const* action_url,
        char const* icon_image,
        char const* icon_image_alt,
        char const* vrml_image,
        char const* statusmap_image,
        int x_2d,
        int y_2d,
        int have_2d_coords,
        double x_3d,
        double y_3d,
        double z_3d,
        int have_3d_coords,
        int should_be_drawn,
        int retain_status_information,
        int retain_nonstatus_information,
        int obsess_over_host) {
  // Make sure we have the data we need.
  if (!name || !name[0] || !address || !address[0]) {
    logger(log_config_error, basic)
      << "Error: Host name or address is NULL";
    return (NULL);
  }
  if (max_attempts <= 0) {
    logger(log_config_error, basic)
      << "Error: Invalid max_check_attempts value for host '"
      << name << "'";
    return (NULL);
  }
  if (check_interval < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid check_interval value for host '"
      << name << "'";
    return (NULL);
  }
  if (notification_interval < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid notification_interval value for host '"
      << name << "'";
    return (NULL);
  }
  if (first_notification_delay < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid first_notification_delay value for host '"
      << name << "'";
    return (NULL);
  }
  if (freshness_threshold < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid freshness_threshold value for host '"
      << name << "'";
    return (NULL);
  }

  // Allocate memory for a new host.
  shared_ptr<host> obj(new host, deleter::host);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate string vars.
    obj->name = my_strdup(name);
    obj->address = my_strdup(address);
    obj->alias = my_strdup(alias ? alias : name);
    obj->display_name = my_strdup(display_name ? display_name : name);
    if (action_url)
      obj->action_url = my_strdup(action_url);
    if (check_period)
      obj->check_period = my_strdup(check_period);
    if (event_handler)
      obj->event_handler = my_strdup(event_handler);
    if (failure_prediction_options)
      obj->failure_prediction_options = my_strdup(failure_prediction_options);
    if (check_command)
      obj->host_check_command = my_strdup(check_command);
    if (icon_image)
      obj->icon_image = my_strdup(icon_image);
    if (icon_image_alt)
      obj->icon_image_alt = my_strdup(icon_image_alt);
    if (notes)
      obj->notes = my_strdup(notes);
    if (notes_url)
      obj->notes_url = my_strdup(notes_url);
    if (notification_period)
      obj->notification_period = my_strdup(notification_period);
    if (statusmap_image)
      obj->statusmap_image = my_strdup(statusmap_image);
    if (vrml_image)
      obj->vrml_image = my_strdup(vrml_image);

    // Duplicate non-string vars.
    obj->accept_passive_host_checks = (accept_passive_checks > 0);
    obj->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
    obj->check_freshness = (check_freshness > 0);
    obj->check_interval = check_interval;
    obj->check_options = CHECK_OPTION_NONE;
    obj->check_type = HOST_CHECK_ACTIVE;
    obj->checks_enabled = (checks_enabled > 0);
    obj->current_attempt = (initial_state == HOST_UP) ? 1 : max_attempts;
    obj->current_state = initial_state;
    obj->event_handler_enabled = (event_handler_enabled > 0);
    obj->failure_prediction_enabled = (failure_prediction_enabled > 0);
    obj->first_notification_delay = first_notification_delay;
    obj->flap_detection_enabled = (flap_detection_enabled > 0);
    obj->flap_detection_on_down = (flap_detection_on_down > 0);
    obj->flap_detection_on_unreachable = (flap_detection_on_unreachable > 0);
    obj->flap_detection_on_up = (flap_detection_on_up > 0);
    obj->freshness_threshold = freshness_threshold;
    obj->have_2d_coords = (have_2d_coords > 0);
    obj->have_3d_coords = (have_3d_coords > 0);
    obj->high_flap_threshold = high_flap_threshold;
    obj->last_hard_state = initial_state;
    obj->last_state = initial_state;
    obj->low_flap_threshold = low_flap_threshold;
    obj->max_attempts = max_attempts;
    obj->modified_attributes = MODATTR_NONE;
    obj->notification_interval = notification_interval;
    obj->notifications_enabled = (notifications_enabled > 0);
    obj->notify_on_down = (notify_down > 0);
    obj->notify_on_downtime = (notify_downtime > 0);
    obj->notify_on_flapping = (notify_flapping > 0);
    obj->notify_on_recovery = (notify_up > 0);
    obj->notify_on_unreachable = (notify_unreachable > 0);
    obj->obsess_over_host = (obsess_over_host > 0);
    obj->process_performance_data = (process_perfdata > 0);
    obj->retain_nonstatus_information = (retain_nonstatus_information > 0);
    obj->retain_status_information = (retain_status_information > 0);
    obj->retry_interval = retry_interval;
    obj->should_be_drawn = (should_be_drawn > 0);
    obj->should_be_scheduled = true;
    obj->stalk_on_down = (stalk_on_down > 0);
    obj->stalk_on_unreachable = (stalk_on_unreachable > 0);
    obj->stalk_on_up = (stalk_on_up > 0);
    obj->state_type = HARD_STATE;
    obj->x_2d = x_2d;
    obj->x_3d = x_3d;
    obj->y_2d = y_2d;
    obj->y_3d = y_3d;
    obj->z_3d = z_3d;

    for (unsigned int x(0); x < MAX_STATE_HISTORY_ENTRIES; ++x)
      obj->state_history[x] = STATE_OK;

    // Add new host to the monitoring engine.
    std::string id(name);
    umap<std::string, shared_ptr<host_struct> >::const_iterator
      it(state::instance().hosts().find(id));
    if (it != state::instance().hosts().end()) {
      logger(log_config_error, basic)
        << "Error: Host '" << name << "' has already been defined";
      return (NULL);
    }

    // Add new items to the configuration state.
    state::instance().hosts()[id] = obj;

    // Add new items to the list.
    obj->next = host_list;
    host_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_host_data(
      NEBTYPE_HOST_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Get number of registered hosts.
 *
 *  @return Number of registered hosts.
 */
int get_host_count() {
  return (state::instance().hosts().size());
}

/**
 *  Tests whether a contact is a contact for a particular host.
 *
 *  @param[in] hst   Target host.
 *  @param[in] cntct Target contact.
 *
 *  @return true or false.
 */
int is_contact_for_host(host* hst, contact* cntct) {
  if (!hst || !cntct)
    return (false);

  // Search all individual contacts of this host.
  for (contactsmember* member(hst->contacts);
       member;
       member = member->next)
    if (member->contact_ptr == cntct)
      return (true);

  // Search all contactgroups of this host.
  for (contactgroupsmember* member(hst->contact_groups);
       member;
       member = member->next)
    if (is_contact_member_of_contactgroup(member->group_ptr, cntct))
      return (true);

  return (false);
}

/**
 *  Tests whether or not a contact is an escalated contact for a
 *  particular host.
 *
 *  @param[in] hst   Target host.
 *  @param[in] cntct Target contact.
 *
 *  @return true or false.
 */
int is_escalated_contact_for_host(host* hst, contact* cntct) {
  if (!hst || !cntct)
    return (false);

  std::string id(hst->name);
  umultimap<std::string, shared_ptr<hostescalation> > const&
    escalations(state::instance().hostescalations());

  for (umultimap<std::string, shared_ptr<hostescalation> >::const_iterator
         it(escalations.find(id)), end(escalations.end());
         it != end && it->first == id;
       ++it) {
    hostescalation* hstescalation(&*it->second);
    // Search all contacts of this host escalation.
    for (contactsmember* member(hstescalation->contacts);
         member;
         member = member->next)
      if (member->contact_ptr == cntct)
        return (true);

    // Search all contactgroups of this host escalation.
    for (contactgroupsmember* member(hstescalation->contact_groups);
         member;
         member = member->next)
      if (is_contact_member_of_contactgroup(member->group_ptr, cntct))
        return (true);
  }

  return (false);
}

/**
 *  Determines whether or not a specific host is an immediate child of
 *  another host.
 *
 *  @param[in] parent_host Parent host.
 *  @param[in] child_host  Child host.
 *
 *  @return true or false.
 */
int is_host_immediate_child_of_host(
      host* parent_host,
      host* child_host) {
  // Not enough data.
  if (!child_host)
    return (false);

  // Root/top-level hosts.
  if (!parent_host) {
    if (!child_host->parent_hosts)
      return (true);
  }
  // Mid-level/bottom hosts.
  else {
    for (hostsmember* member(child_host->parent_hosts);
         member;
         member = member->next)
      if (member->host_ptr == parent_host)
        return (true);
  }

  return (false);
}

/**
 *  Determines whether or not a specific host is an immediate parent of
 *  another host.
 *
 *  @param[in] child_host  Child host.
 *  @param[in] parent_host Parent host.
 *
 *  @return true or false.
 */
int is_host_immediate_parent_of_host(
      host* child_host,
      host* parent_host) {
  if (is_host_immediate_child_of_host(parent_host, child_host) == true)
    return (true);
  return (false);
}

/**
 *  Returns a count of the immediate children for a given host.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] hst Target host.
 *
 *  @return Number of immediate child hosts.
 */
int number_of_immediate_child_hosts(host* hst) {
  int children(0);
  for (host* tmp(host_list); tmp; tmp = tmp->next)
    if (is_host_immediate_child_of_host(hst, tmp))
      ++children;
  return (children);
}

/**
 *  Get the number of immediate parent hosts for a given host.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] hst Target host.
 *
 *  @return Number of immediate parent hosts.
 */
int number_of_immediate_parent_hosts(host* hst) {
  int parents(0);
  for (host* tmp(host_list); tmp; tmp = tmp->next)
    if (is_host_immediate_parent_of_host(hst, tmp))
      ++parents;
  return (parents);
}

/**
 *  Returns a count of the total children for a given host.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] hst Target host.
 *
 *  @return Number of total child hosts.
 */
int number_of_total_child_hosts(host* hst) {
  int children(0);
  for (host* tmp(host_list); tmp; tmp = tmp->next)
    if (is_host_immediate_child_of_host(hst, tmp))
      children += number_of_total_child_hosts(tmp) + 1;
  return (children);
}

/**
 *  Get the total number of parent hosts for a given host.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] hst Target host.
 *
 *  @return Number of total parent hosts.
 */
int number_of_total_parent_hosts(host* hst) {
  int parents(0);
  for (host* tmp(host_list); tmp; tmp = tmp->next)
    if (is_host_immediate_parent_of_host(hst, tmp))
      parents += number_of_total_parent_hosts(tmp) + 1;
  return (parents);
}

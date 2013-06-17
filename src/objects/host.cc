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

#include "com/centreon/engine/misc/object.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/objects/commandsmember.hh"
#include "com/centreon/engine/objects/contactgroupsmember.hh"
#include "com/centreon/engine/objects/contactsmember.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"
#include "com/centreon/engine/objects/host.hh"
#include "com/centreon/engine/objects/hostsmember.hh"
#include "com/centreon/engine/objects/servicesmember.hh"

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
  return (false);
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
    "  parent_hosts:                         " << chkobj(os, obj.parent_hosts) << "\n"
    "  child_hosts:                          " << chkobj(os, obj.child_hosts) << "\n"
    "  services:                             " << chkobj(os, obj.services) << "\n"
    "  host_check_command:                   " << chkstr(obj.host_check_command) << "\n"
    "  initial_state:                        " << obj.initial_state << "\n"
    "  check_interval:                       " << obj.check_interval << "\n"
    "  retry_interval:                       " << obj.retry_interval << "\n"
    "  max_attempts:                         " << obj.max_attempts << "\n"
    "  event_handler:                        " << chkstr(obj.event_handler) << "\n"
    "  contact_groups:                       " << chkobj(os, obj.contact_groups) << "\n"
    "  contacts:                             " << chkobj(os, obj.contacts) << "\n"
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
    "  custom_variables:                     " << chkobj(os, obj.custom_variables) << "\n"
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


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
#include "com/centreon/engine/objects/service.hh"

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
       service const& obj1,
       service const& obj2) throw () {
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
       service const& obj1,
       service const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump service content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The service to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, service const& obj) {
  os << "service {\n"
    "  host_name:                            " << chkstr(obj.host_name) << "\n"
    "  description:                          " << chkstr(obj.description) << "\n"
    "  display_name:                         " << chkstr(obj.display_name) << "\n"
    "  service_check_command:                " << chkstr(obj.service_check_command) << "\n"
    "  event_handler:                        " << chkstr(obj.event_handler) << "\n"
    "  initial_state:                        " << obj.initial_state << "\n"
    "  check_interval:                       " << obj.check_interval << "\n"
    "  retry_interval:                       " << obj.retry_interval << "\n"
    "  max_attempts:                         " << obj.max_attempts << "\n"
    "  parallelize:                          " << obj.parallelize << "\n"
    "  contact_groups:                       " << chkobj(os, obj.contact_groups) << "\n"
    "  contacts:                             " << chkobj(os, obj.contacts) << "\n"
    "  notification_interval:                " << obj.notification_interval << "\n"
    "  first_notification_delay:             " << obj.first_notification_delay << "\n"
    "  notify_on_unknown:                    " << obj.notify_on_unknown << "\n"
    "  notify_on_warning:                    " << obj.notify_on_warning << "\n"
    "  notify_on_critical:                   " << obj.notify_on_critical << "\n"
    "  notify_on_recovery:                   " << obj.notify_on_recovery << "\n"
    "  notify_on_flapping:                   " << obj.notify_on_flapping << "\n"
    "  notify_on_downtime:                   " << obj.notify_on_downtime << "\n"
    "  stalk_on_ok:                          " << obj.stalk_on_ok << "\n"
    "  stalk_on_warning:                     " << obj.stalk_on_warning << "\n"
    "  stalk_on_unknown:                     " << obj.stalk_on_unknown << "\n"
    "  stalk_on_critical:                    " << obj.stalk_on_critical << "\n"
    "  is_volatile:                          " << obj.is_volatile << "\n"
    "  notification_period:                  " << chkstr(obj.notification_period) << "\n"
    "  check_period:                         " << chkstr(obj.check_period) << "\n"
    "  flap_detection_enabled:               " << obj.flap_detection_enabled << "\n"
    "  low_flap_threshold:                   " << obj.low_flap_threshold << "\n"
    "  high_flap_threshold:                  " << obj.high_flap_threshold << "\n"
    "  flap_detection_on_ok:                 " << obj.flap_detection_on_ok << "\n"
    "  flap_detection_on_warning:            " << obj.flap_detection_on_warning << "\n"
    "  flap_detection_on_unknown:            " << obj.flap_detection_on_unknown << "\n"
    "  flap_detection_on_critical:           " << obj.flap_detection_on_critical << "\n"
    "  process_performance_data:             " << obj.process_performance_data << "\n"
    "  check_freshness:                      " << obj.check_freshness << "\n"
    "  freshness_threshold:                  " << obj.freshness_threshold << "\n"
    "  accept_passive_service_checks:        " << obj.accept_passive_service_checks << "\n"
    "  event_handler_enabled:                " << obj.event_handler_enabled << "\n"
    "  checks_enabled:                       " << obj.checks_enabled << "\n"
    "  retain_status_information:            " << obj.retain_status_information << "\n"
    "  retain_nonstatus_information:         " << obj.retain_nonstatus_information << "\n"
    "  notifications_enabled:                " << obj.notifications_enabled << "\n"
    "  obsess_over_service:                  " << obj.obsess_over_service << "\n"
    "  failure_prediction_enabled:           " << obj.failure_prediction_enabled << "\n"
    "  failure_prediction_options:           " << chkstr(obj.failure_prediction_options) << "\n"
    "  notes:                                " << chkstr(obj.notes) << "\n"
    "  notes_url:                            " << chkstr(obj.notes_url) << "\n"
    "  action_url:                           " << chkstr(obj.action_url) << "\n"
    "  icon_image:                           " << chkstr(obj.icon_image) << "\n"
    "  icon_image_alt:                       " << chkstr(obj.icon_image_alt) << "\n"
    "  custom_variables:                     " << chkobj(os, obj.custom_variables) << "\n"
    "  problem_has_been_acknowledged:        " << obj.problem_has_been_acknowledged << "\n"
    "  acknowledgement_type:                 " << obj.acknowledgement_type << "\n"
    "  host_problem_at_last_check:           " << obj.host_problem_at_last_check << "\n"
    "  check_type:                           " << obj.check_type << "\n"
    "  current_state:                        " << obj.current_state << "\n"
    "  last_state:                           " << obj.last_state << "\n"
    "  last_hard_state:                      " << obj.last_hard_state << "\n"
    "  plugin_output:                        " << chkstr(obj.plugin_output) << "\n"
    "  long_plugin_output:                   " << chkstr(obj.long_plugin_output) << "\n"
    "  perf_data:                            " << chkstr(obj.perf_data) << "\n"
    "  state_type:                           " << obj.state_type << "\n"
    "  next_check:                           " << obj.next_check << "\n"
    "  should_be_scheduled:                  " << obj.should_be_scheduled << "\n"
    "  last_check:                           " << obj.last_check << "\n"
    "  current_attempt:                      " << obj.current_attempt << "\n"
    "  current_event_id:                     " << obj.current_event_id << "\n"
    "  last_event_id:                        " << obj.last_event_id << "\n"
    "  current_problem_id:                   " << obj.current_problem_id << "\n"
    "  last_problem_id:                      " << obj.last_problem_id << "\n"
    "  last_notification:                    " << obj.last_notification << "\n"
    "  next_notification:                    " << obj.next_notification << "\n"
    "  no_more_notifications:                " << obj.no_more_notifications << "\n"
    "  check_flapping_recovery_notification: " << obj.check_flapping_recovery_notification << "\n"
    "  last_state_change:                    " << obj.last_state_change << "\n"
    "  last_hard_state_change:               " << obj.last_hard_state_change << "\n"
    "  last_time_ok:                         " << obj.last_time_ok << "\n"
    "  last_time_warning:                    " << obj.last_time_warning << "\n"
    "  last_time_unknown:                    " << obj.last_time_unknown << "\n"
    "  last_time_critical:                   " << obj.last_time_critical << "\n"
    "  has_been_checked:                     " << obj.has_been_checked << "\n"
    "  is_being_freshened:                   " << obj.is_being_freshened << "\n"
    "  notified_on_unknown:                  " << obj.notified_on_unknown << "\n"
    "  notified_on_warning:                  " << obj.notified_on_warning << "\n"
    "  notified_on_critical:                 " << obj.notified_on_critical << "\n"
    "  current_notification_number:          " << obj.current_notification_number << "\n"
    "  current_notification_id:              " << obj.current_notification_id << "\n"
    "  latency:                              " << obj.latency << "\n"
    "  execution_time:                       " << obj.execution_time << "\n"
    "  is_executing:                         " << obj.is_executing << "\n"
    "  check_options:                        " << obj.check_options << "\n"
    "  scheduled_downtime_depth:             " << obj.scheduled_downtime_depth << "\n"
    "  pending_flex_downtime:                " << obj.pending_flex_downtime << "\n"
    "  state_history:                        " << obj.state_history << "\n"
    "  state_history_index:                  " << obj.state_history_index << "\n"
    "  is_flapping:                          " << obj.is_flapping << "\n"
    "  flapping_comment_id:                  " << obj.flapping_comment_id << "\n"
    "  percent_state_change:                 " << obj.percent_state_change << "\n"
    "  modified_attributes:                  " << obj.modified_attributes << "\n"
    "  host_ptr:                             " << obj.host_ptr << "\n"
    "  event_handler_ptr:                    " << obj.event_handler_ptr << "\n"
    "  event_handler_args:                   " << chkstr(obj.event_handler_args) << "\n"
    "  check_command_ptr:                    " << obj.check_command_ptr << "\n"
    "  check_command_args:                   " << chkstr(obj.check_command_args) << "\n"
    "  check_period_ptr:                     " << obj.check_period_ptr << "\n"
    "  notification_period_ptr:              " << obj.notification_period_ptr << "\n"
    "  servicegroups_ptr:                    " << obj.servicegroups_ptr << "\n"
    "}\n";
  return (os);
}

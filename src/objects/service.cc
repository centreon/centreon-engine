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
  return (is_equal(obj1.host_name, obj2.host_name)
          && is_equal(obj1.description, obj2.description)
          && is_equal(obj1.display_name, obj2.display_name)
          && is_equal(obj1.service_check_command, obj2.service_check_command)
          && is_equal(obj1.event_handler, obj2.event_handler)
          && obj1.initial_state == obj2.initial_state
          && obj1.check_interval == obj2.check_interval
          && obj1.retry_interval == obj2.retry_interval
          && obj1.max_attempts == obj2.max_attempts
          && obj1.parallelize == obj2.parallelize
          && is_equal(obj1.contact_groups, obj2.contact_groups)
          && is_equal(obj1.contacts, obj2.contacts)
          && obj1.notification_interval == obj2.notification_interval
          && obj1.first_notification_delay == obj2.first_notification_delay
          && obj1.notify_on_unknown == obj2.notify_on_unknown
          && obj1.notify_on_warning == obj2.notify_on_warning
          && obj1.notify_on_critical == obj2.notify_on_critical
          && obj1.notify_on_recovery == obj2.notify_on_recovery
          && obj1.notify_on_flapping == obj2.notify_on_flapping
          && obj1.notify_on_downtime == obj2.notify_on_downtime
          && obj1.stalk_on_ok == obj2.stalk_on_ok
          && obj1.stalk_on_warning == obj2.stalk_on_warning
          && obj1.stalk_on_unknown == obj2.stalk_on_unknown
          && obj1.stalk_on_critical == obj2.stalk_on_critical
          && obj1.is_volatile == obj2.is_volatile
          && is_equal(obj1.notification_period, obj2.notification_period)
          && is_equal(obj1.check_period, obj2.check_period)
          && obj1.flap_detection_enabled == obj2.flap_detection_enabled
          && obj1.low_flap_threshold == obj2.low_flap_threshold
          && obj1.high_flap_threshold == obj2.high_flap_threshold
          && obj1.flap_detection_on_ok == obj2.flap_detection_on_ok
          && obj1.flap_detection_on_warning == obj2.flap_detection_on_warning
          && obj1.flap_detection_on_unknown == obj2.flap_detection_on_unknown
          && obj1.flap_detection_on_critical == obj2.flap_detection_on_critical
          && obj1.process_performance_data == obj2.process_performance_data
          && obj1.check_freshness == obj2.check_freshness
          && obj1.freshness_threshold == obj2.freshness_threshold
          && obj1.accept_passive_service_checks == obj2.accept_passive_service_checks
          && obj1.event_handler_enabled == obj2.event_handler_enabled
          && obj1.checks_enabled == obj2.checks_enabled
          && obj1.retain_status_information == obj2.retain_status_information
          && obj1.retain_nonstatus_information == obj2.retain_nonstatus_information
          && obj1.notifications_enabled == obj2.notifications_enabled
          && obj1.obsess_over_service == obj2.obsess_over_service
          && obj1.failure_prediction_enabled == obj2.failure_prediction_enabled
          && is_equal(obj1.failure_prediction_options, obj2.failure_prediction_options)
          && is_equal(obj1.notes, obj2.notes)
          && is_equal(obj1.notes_url, obj2.notes_url)
          && is_equal(obj1.action_url, obj2.action_url)
          && is_equal(obj1.icon_image, obj2.icon_image)
          && is_equal(obj1.icon_image_alt, obj2.icon_image_alt)
          && is_equal(obj1.custom_variables, obj2.custom_variables)
          && obj1.problem_has_been_acknowledged == obj2.problem_has_been_acknowledged
          && obj1.acknowledgement_type == obj2.acknowledgement_type
          && obj1.host_problem_at_last_check == obj2.host_problem_at_last_check
          && obj1.check_type == obj2.check_type
          && obj1.current_state == obj2.current_state
          && obj1.last_state == obj2.last_state
          && obj1.last_hard_state == obj2.last_hard_state
          && is_equal(obj1.plugin_output, obj2.plugin_output)
          && is_equal(obj1.long_plugin_output, obj2.long_plugin_output)
          && is_equal(obj1.perf_data, obj2.perf_data)
          && obj1.state_type == obj2.state_type
          && obj1.next_check == obj2.next_check
          && obj1.should_be_scheduled == obj2.should_be_scheduled
          && obj1.last_check == obj2.last_check
          && obj1.current_attempt == obj2.current_attempt
          && obj1.current_event_id == obj2.current_event_id
          && obj1.last_event_id == obj2.last_event_id
          && obj1.current_problem_id == obj2.current_problem_id
          && obj1.last_problem_id == obj2.last_problem_id
          && obj1.last_notification == obj2.last_notification
          && obj1.next_notification == obj2.next_notification
          && obj1.no_more_notifications == obj2.no_more_notifications
          && obj1.check_flapping_recovery_notification == obj2.check_flapping_recovery_notification
          && obj1.last_state_change == obj2.last_state_change
          && obj1.last_hard_state_change == obj2.last_hard_state_change
          && obj1.last_time_ok == obj2.last_time_ok
          && obj1.last_time_warning == obj2.last_time_warning
          && obj1.last_time_unknown == obj2.last_time_unknown
          && obj1.last_time_critical == obj2.last_time_critical
          && obj1.has_been_checked == obj2.has_been_checked
          && obj1.is_being_freshened == obj2.is_being_freshened
          && obj1.notified_on_unknown == obj2.notified_on_unknown
          && obj1.notified_on_warning == obj2.notified_on_warning
          && obj1.notified_on_critical == obj2.notified_on_critical
          && obj1.current_notification_number == obj2.current_notification_number
          && obj1.current_notification_id == obj2.current_notification_id
          && obj1.latency == obj2.latency
          && obj1.execution_time == obj2.execution_time
          && obj1.is_executing == obj2.is_executing
          && obj1.check_options == obj2.check_options
          && obj1.scheduled_downtime_depth == obj2.scheduled_downtime_depth
          && obj1.pending_flex_downtime == obj2.pending_flex_downtime
          && is_equal(obj1.state_history, obj2.state_history, MAX_STATE_HISTORY_ENTRIES)
          && obj1.state_history_index == obj2.state_history_index
          && obj1.is_flapping == obj2.is_flapping
          && obj1.flapping_comment_id == obj2.flapping_comment_id
          && obj1.percent_state_change == obj2.percent_state_change
          && obj1.modified_attributes == obj2.modified_attributes
          && is_equal(obj1.event_handler_args, obj2.event_handler_args)
          && is_equal(obj1.check_command_args, obj2.check_command_args));
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




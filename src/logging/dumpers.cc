/*
** Copyright 2011 Merethis
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

#include <QTextStream>
#include "logging/dumpers.hh"

using namespace com::centreon::engine;

static inline char const* checkstr(char const* str) throw() {
  return (str != NULL ? str : "\"NULL\"");
}

QTextStream& logging::operator<<(QTextStream& stream, host const& hst) {
  stream << "dump object\n"                                             \
    "host (" << &hst << ") {\n"                                         \
    "  name:                                 " << checkstr(hst.name) << "\n" \
    "  display_name:                         " << checkstr(hst.display_name) << "\n" \
    "  alias:                                " << checkstr(hst.alias) << "\n" \
    "  address:                              " << checkstr(hst.address) << "\n";

  stream << "  parent_hosts:                         { ";
  for (hostsmember* hs = hst.parent_hosts; hs != NULL; hs = hs->next)
    stream << checkstr(hs->host_name) << (hs->next ? ", " : "");
  stream << (hst.parent_hosts ? " }\n" : "\"NULL\" }\n");

  stream << "  child_hosts:                          { ";
  for (hostsmember* hs = hst.child_hosts; hs != NULL; hs = hs->next)
    stream << checkstr(hs->host_name) << (hs->next ? ", " : "");
  stream << (hst.child_hosts ? " }\n" : "\"NULL\" }\n");

  stream << "  services:                             { ";
  for (servicesmember* sm = hst.services; sm != NULL; sm = sm->next)
    stream << "{ " << checkstr(sm->host_name) << ", "
           << checkstr(sm->service_description) << " }"
           << (sm->next ? ", " : "");
  stream << (hst.services ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  host_check_command:                   " << checkstr(hst.host_check_command) << "\n" \
    "  initial_state:                        " << hst.initial_state << "\n" \
    "  check_interval:                       " << hst.check_interval << "\n" \
    "  retry_interval:                       " << hst.retry_interval << "\n" \
    "  max_attempts:                         " << hst.max_attempts << "\n" \
    "  event_handler:                        " << checkstr(hst.event_handler) << "\n";

  stream << "  contact_groups:                       { ";
  for (contactgroupsmember* cgm = hst.contact_groups; cgm != NULL; cgm = cgm->next)
    stream << checkstr(cgm->group_name) << (cgm->next ? ", " : "");
  stream << (hst.contact_groups ? " }\n" : "\"NULL\" }\n");

  stream << "  contacts:                             { ";
  for (contactsmember* cm = hst.contacts; cm != NULL; cm = cm->next)
    stream << checkstr(cm->contact_name) << (cm->next ? ", " : "");
  stream << (hst.contacts ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  notification_interval:                " << hst.notification_interval << "\n" \
    "  first_notification_delay:             " << hst.first_notification_delay << "\n" \
    "  notify_on_down:                       " << hst.notify_on_down << "\n" \
    "  notify_on_unreachable:                " << hst.notify_on_unreachable << "\n" \
    "  notify_on_recovery:                   " << hst.notify_on_recovery << "\n" \
    "  notify_on_flapping:                   " << hst.notify_on_flapping << "\n" \
    "  notify_on_downtime:                   " << hst.notify_on_downtime << "\n" \
    "  notification_period:                  " << checkstr(hst.notification_period) << "\n" \
    "  check_period:                         " << checkstr(hst.check_period) << "\n" \
    "  flap_detection_enabled:               " << hst.flap_detection_enabled << "\n" \
    "  low_flap_threshold:                   " << hst.low_flap_threshold << "\n" \
    "  high_flap_threshold:                  " << hst.high_flap_threshold << "\n" \
    "  flap_detection_on_up:                 " << hst.flap_detection_on_up << "\n" \
    "  flap_detection_on_down:               " << hst.flap_detection_on_down << "\n" \
    "  flap_detection_on_unreachable:        " << hst.flap_detection_on_unreachable << "\n" \
    "  stalk_on_up:                          " << hst.stalk_on_up << "\n" \
    "  stalk_on_down:                        " << hst.stalk_on_down << "\n" \
    "  stalk_on_unreachable:                 " << hst.stalk_on_unreachable << "\n" \
    "  check_freshness:                      " << hst.check_freshness << "\n" \
    "  freshness_threshold:                  " << hst.freshness_threshold << "\n" \
    "  process_performance_data:             " << hst.process_performance_data << "\n" \
    "  checks_enabled:                       " << hst.checks_enabled << "\n" \
    "  accept_passive_host_checks:           " << hst.accept_passive_host_checks << "\n" \
    "  event_handler_enabled:                " << hst.event_handler_enabled << "\n" \
    "  retain_status_information:            " << hst.retain_status_information << "\n" \
    "  retain_nonstatus_information:         " << hst.retain_nonstatus_information << "\n" \
    "  failure_prediction_enabled:           " << hst.failure_prediction_enabled << "\n" \
    "  failure_prediction_options:           " << checkstr(hst.failure_prediction_options) << "\n" \
    "  obsess_over_host:                     " << hst.obsess_over_host << "\n" \
    "  notes:                                " << checkstr(hst.notes) << "\n" \
    "  notes_url:                            " << checkstr(hst.notes_url) << "\n" \
    "  action_url:                           " << checkstr(hst.action_url) << "\n" \
    "  icon_image:                           " << checkstr(hst.icon_image) << "\n" \
    "  icon_image_alt:                       " << checkstr(hst.icon_image_alt) << "\n" \
    "  vrml_image:                           " << checkstr(hst.vrml_image) << "\n" \
    "  statusmap_image:                      " << checkstr(hst.statusmap_image) << "\n" \
    "  have_2d_coords:                       " << hst.have_2d_coords << "\n" \
    "  x_2d:                                 " << hst.x_2d << "\n" \
    "  y_2d:                                 " << hst.y_2d << "\n" \
    "  have_3d_coords:                       " << hst.have_3d_coords << "\n" \
    "  x_3d:                                 " << hst.x_3d << "\n" \
    "  y_3d:                                 " << hst.y_3d << "\n" \
    "  z_3d:                                 " << hst.z_3d << "\n" \
    "  should_be_drawn:                      " << hst.should_be_drawn << "\n";

  stream << "  custom_variables:                     { ";
  for (customvariablesmember* cvm = hst.custom_variables; cvm != NULL; cvm = cvm->next)
    stream << checkstr(cvm->variable_name) << (cvm->next ? ", " : "");
  stream << (hst.custom_variables ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  problem_has_been_acknowledged:        " << hst.problem_has_been_acknowledged << "\n" \
    "  acknowledgement_type:                 " << hst.acknowledgement_type << "\n" \
    "  check_type:                           " << hst.check_type << "\n" \
    "  current_state:                        " << hst.current_state << "\n" \
    "  last_state:                           " << hst.last_state << "\n" \
    "  last_hard_state:                      " << hst.last_hard_state << "\n" \
    "  plugin_output:                        " << checkstr(hst.plugin_output) << "\n" \
    "  long_plugin_output:                   " << checkstr(hst.long_plugin_output) << "\n" \
    "  perf_data:                            " << checkstr(hst.perf_data) << "\n" \
    "  state_type:                           " << hst.state_type << "\n" \
    "  current_attempt:                      " << hst.current_attempt << "\n" \
    "  current_event_id:                     " << hst.current_event_id << "\n" \
    "  last_event_id:                        " << hst.last_event_id << "\n" \
    "  current_problem_id:                   " << hst.current_problem_id << "\n" \
    "  last_problem_id:                      " << hst.last_problem_id << "\n" \
    "  latency:                              " << hst.latency << "\n" \
    "  execution_time:                       " << hst.execution_time << "\n" \
    "  is_executing:                         " << hst.is_executing << "\n" \
    "  check_options:                        " << hst.check_options << "\n" \
    "  notifications_enabled:                " << hst.notifications_enabled << "\n" \
    "  last_host_notification:               " << hst.last_host_notification << "\n" \
    "  next_host_notification:               " << hst.next_host_notification << "\n" \
    "  next_check:                           " << hst.next_check << "\n" \
    "  should_be_scheduled:                  " << hst.should_be_scheduled << "\n" \
    "  last_check:                           " << hst.last_check << "\n" \
    "  last_state_change:                    " << hst.last_state_change << "\n" \
    "  last_hard_state_change:               " << hst.last_hard_state_change << "\n" \
    "  last_time_up:                         " << hst.last_time_up << "\n" \
    "  last_time_down:                       " << hst.last_time_down << "\n" \
    "  last_time_unreachable:                " << hst.last_time_unreachable << "\n" \
    "  has_been_checked:                     " << hst.has_been_checked << "\n" \
    "  is_being_freshened:                   " << hst.is_being_freshened << "\n" \
    "  notified_on_down:                     " << hst.notified_on_down << "\n" \
    "  notified_on_unreachable:              " << hst.notified_on_unreachable << "\n" \
    "  current_notification_number:          " << hst.current_notification_number << "\n" \
    "  no_more_notifications:                " << hst.no_more_notifications << "\n" \
    "  current_notification_id:              " << hst.current_notification_id << "\n" \
    "  check_flapping_recovery_notification: " << hst.check_flapping_recovery_notification << "\n" \
    "  scheduled_downtime_depth:             " << hst.scheduled_downtime_depth << "\n" \
    "  pending_flex_downtime:                " << hst.pending_flex_downtime << "\n" \
    "  state_history:                        " << hst.state_history << "\n" \
    "  state_history_index:                  " << hst.state_history_index << "\n" \
    "  last_state_history_update:            " << hst.last_state_history_update << "\n" \
    "  is_flapping:                          " << hst.is_flapping << "\n" \
    "  flapping_comment_id:                  " << hst.flapping_comment_id << "\n" \
    "  percent_state_change:                 " << hst.percent_state_change << "\n" \
    "  total_services:                       " << hst.total_services << "\n" \
    "  total_service_check_interval:         " << hst.total_service_check_interval << "\n" \
    "  modified_attributes:                  " << hst.modified_attributes << "\n" \
    "  circular_path_checked:                " << hst.circular_path_checked << "\n" \
    "  contains_circular_path:               " << hst.contains_circular_path << "\n" \
    "  event_handler_ptr:                    " << hst.event_handler_ptr << "\n" \
    "  check_command_ptr:                    " << hst.check_command_ptr << "\n" \
    "  check_period_ptr:                     " << hst.check_period_ptr << "\n" \
    "  notification_period_ptr:              " << hst.notification_period_ptr << "\n" \
    "  hostgroups_ptr:                       " << hst.hostgroups_ptr << "\n" \
    "}\n";
  return (stream);
}

QTextStream& logging::operator<<(QTextStream& stream, service const& svc) {
  stream << "dump object\n"                                             \
    "service (" << &svc << ") {\n"                                      \
    "  host_name:                            " << checkstr(svc.host_name) << "\n" \
    "  description:                          " << checkstr(svc.description) << "\n" \
    "  display_name:                         " << checkstr(svc.display_name) << "\n" \
    "  service_check_command:                " << checkstr(svc.service_check_command) << "\n" \
    "  event_handler:                        " << checkstr(svc.event_handler) << "\n" \
    "  initial_state:                        " << svc.initial_state << "\n" \
    "  check_interval:                       " << svc.check_interval << "\n" \
    "  retry_interval:                       " << svc.retry_interval << "\n" \
    "  max_attempts:                         " << svc.max_attempts << "\n" \
    "  parallelize:                          " << svc.parallelize << "\n";

  stream << "  contact_groups:                       { ";
  for (contactgroupsmember* cgm = svc.contact_groups; cgm != NULL; cgm = cgm->next)
    stream << checkstr(cgm->group_name) << (cgm->next ? ", " : "");
  stream << (svc.contact_groups ? " }\n" : "\"NULL\" }\n");

  stream << "  contacts:                             { ";
  for (contactsmember* cm = svc.contacts; cm != NULL; cm = cm->next)
    stream << checkstr(cm->contact_name) << (cm->next ? ", " : "");
  stream << (svc.contacts ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  notification_interval:                " << svc.notification_interval << "\n" \
    "  first_notification_delay:             " << svc.first_notification_delay << "\n" \
    "  notify_on_unknown:                    " << svc.notify_on_unknown << "\n" \
    "  notify_on_warning:                    " << svc.notify_on_warning << "\n" \
    "  notify_on_critical:                   " << svc.notify_on_critical << "\n" \
    "  notify_on_recovery:                   " << svc.notify_on_recovery << "\n" \
    "  notify_on_flapping:                   " << svc.notify_on_flapping << "\n" \
    "  notify_on_downtime:                   " << svc.notify_on_downtime << "\n" \
    "  stalk_on_ok:                          " << svc.stalk_on_ok << "\n" \
    "  stalk_on_warning:                     " << svc.stalk_on_warning << "\n" \
    "  stalk_on_unknown:                     " << svc.stalk_on_unknown << "\n" \
    "  stalk_on_critical:                    " << svc.stalk_on_critical << "\n" \
    "  is_volatile:                          " << svc.is_volatile << "\n" \
    "  notification_period:                  " << checkstr(svc.notification_period) << "\n" \
    "  check_period:                         " << checkstr(svc.check_period) << "\n" \
    "  flap_detection_enabled:               " << svc.flap_detection_enabled << "\n" \
    "  low_flap_threshold:                   " << svc.low_flap_threshold << "\n" \
    "  high_flap_threshold:                  " << svc.high_flap_threshold << "\n" \
    "  flap_detection_on_ok:                 " << svc.flap_detection_on_ok << "\n" \
    "  flap_detection_on_warning:            " << svc.flap_detection_on_warning << "\n" \
    "  flap_detection_on_unknown:            " << svc.flap_detection_on_unknown << "\n" \
    "  flap_detection_on_critical:           " << svc.flap_detection_on_critical << "\n" \
    "  process_performance_data:             " << svc.process_performance_data << "\n" \
    "  check_freshness:                      " << svc.check_freshness << "\n" \
    "  freshness_threshold:                  " << svc.freshness_threshold << "\n" \
    "  accept_passive_service_checks:        " << svc.accept_passive_service_checks << "\n" \
    "  event_handler_enabled:                " << svc.event_handler_enabled << "\n" \
    "  checks_enabled:                       " << svc.checks_enabled << "\n" \
    "  retain_status_information:            " << svc.retain_status_information << "\n" \
    "  retain_nonstatus_information:         " << svc.retain_nonstatus_information << "\n" \
    "  notifications_enabled:                " << svc.notifications_enabled << "\n" \
    "  obsess_over_service:                  " << svc.obsess_over_service << "\n" \
    "  failure_prediction_enabled:           " << svc.failure_prediction_enabled << "\n" \
    "  failure_prediction_options:           " << checkstr(svc.failure_prediction_options) << "\n" \
    "  notes:                                " << checkstr(svc.notes) << "\n" \
    "  notes_url:                            " << checkstr(svc.notes_url) << "\n" \
    "  action_url:                           " << checkstr(svc.action_url) << "\n" \
    "  icon_image:                           " << checkstr(svc.icon_image) << "\n" \
    "  icon_image_alt:                       " << checkstr(svc.icon_image_alt) << "\n";

  stream << "  custom_variables:                     { ";
  for (customvariablesmember* cvm = svc.custom_variables; cvm != NULL; cvm = cvm->next)
    stream << checkstr(cvm->variable_name) << (cvm->next ? ", " : "");
  stream << (svc.custom_variables ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  problem_has_been_acknowledged:        " << svc.problem_has_been_acknowledged << "\n" \
    "  acknowledgement_type:                 " << svc.acknowledgement_type << "\n" \
    "  host_problem_at_last_check:           " << svc.host_problem_at_last_check << "\n" \
    "  check_type:                           " << svc.check_type << "\n" \
    "  current_state:                        " << svc.current_state << "\n" \
    "  last_state:                           " << svc.last_state << "\n" \
    "  last_hard_state:                      " << svc.last_hard_state << "\n" \
    "  plugin_output:                        " << checkstr(svc.plugin_output) << "\n" \
    "  long_plugin_output:                   " << checkstr(svc.long_plugin_output) << "\n" \
    "  perf_data:                            " << checkstr(svc.perf_data) << "\n" \
    "  state_type:                           " << svc.state_type << "\n" \
    "  next_check:                           " << svc.next_check << "\n" \
    "  should_be_scheduled:                  " << svc.should_be_scheduled << "\n" \
    "  last_check:                           " << svc.last_check << "\n" \
    "  current_attempt:                      " << svc.current_attempt << "\n" \
    "  current_event_id:                     " << svc.current_event_id << "\n" \
    "  last_event_id:                        " << svc.last_event_id << "\n" \
    "  current_problem_id:                   " << svc.current_problem_id << "\n" \
    "  last_problem_id:                      " << svc.last_problem_id << "\n" \
    "  last_notification:                    " << svc.last_notification << "\n" \
    "  next_notification:                    " << svc.next_notification << "\n" \
    "  no_more_notifications:                " << svc.no_more_notifications << "\n" \
    "  check_flapping_recovery_notification: " << svc.check_flapping_recovery_notification << "\n" \
    "  last_state_change:                    " << svc.last_state_change << "\n" \
    "  last_hard_state_change:               " << svc.last_hard_state_change << "\n" \
    "  last_time_ok:                         " << svc.last_time_ok << "\n" \
    "  last_time_warning:                    " << svc.last_time_warning << "\n" \
    "  last_time_unknown:                    " << svc.last_time_unknown << "\n" \
    "  last_time_critical:                   " << svc.last_time_critical << "\n" \
    "  has_been_checked:                     " << svc.has_been_checked << "\n" \
    "  is_being_freshened:                   " << svc.is_being_freshened << "\n" \
    "  notified_on_unknown:                  " << svc.notified_on_unknown << "\n" \
    "  notified_on_warning:                  " << svc.notified_on_warning << "\n" \
    "  notified_on_critical:                 " << svc.notified_on_critical << "\n" \
    "  current_notification_number:          " << svc.current_notification_number << "\n" \
    "  current_notification_id:              " << svc.current_notification_id << "\n" \
    "  latency:                              " << svc.latency << "\n" \
    "  execution_time:                       " << svc.execution_time << "\n" \
    "  is_executing:                         " << svc.is_executing << "\n" \
    "  check_options:                        " << svc.check_options << "\n" \
    "  scheduled_downtime_depth:             " << svc.scheduled_downtime_depth << "\n" \
    "  pending_flex_downtime:                " << svc.pending_flex_downtime << "\n" \
    "  state_history:                        " << svc.state_history << "\n" \
    "  state_history_index:                  " << svc.state_history_index << "\n"     \
    "  is_flapping:                          " << svc.is_flapping << "\n" \
    "  flapping_comment_id:                  " << svc.flapping_comment_id << "\n" \
    "  percent_state_change:                 " << svc.percent_state_change << "\n" \
    "  modified_attributes:                  " << svc.modified_attributes << "\n" \
    "  host_ptr:                             " << svc.host_ptr << "\n"  \
    "  event_handler_ptr:                    " << svc.event_handler_ptr << "\n" \
    "  event_handler_args:                   " << checkstr(svc.event_handler_args) << "\n" \
    "  check_command_ptr:                    " << svc.check_command_ptr << "\n" \
    "  check_command_args:                   " << checkstr(svc.check_command_args) << "\n" \
    "  check_period_ptr:                     " << svc.check_period_ptr << "\n" \
    "  notification_period_ptr:              " << svc.notification_period_ptr << "\n" \
    "  servicegroups_ptr:                    " << svc.servicegroups_ptr << "\n" \
    "}\n";

  return (stream);
}

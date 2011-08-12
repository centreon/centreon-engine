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
#include "globals.hh"
#include "engine.hh"

using namespace com::centreon::engine;

static inline char const* checkstr(char const* str) throw() {
  return (str != NULL ? str : "\"NULL\"");
}

void logging::dump_object_list() {
  logger(dbg_functions, basic) << scheduling_info;

  for (host const* obj = host_list; obj != NULL; obj = obj->next)
    logger(dbg_functions, basic) << *obj;

  for (service const* obj = service_list; obj != NULL; obj = obj->next)
    logger(dbg_functions, basic) << *obj;

  for (contact const* obj = contact_list; obj != NULL; obj = obj->next)
    logger(dbg_functions, basic) << *obj;

  for (command const* obj = command_list; obj != NULL; obj = obj->next)
    logger(dbg_functions, basic) << *obj;

  for (contactgroup const* obj = contactgroup_list; obj != NULL; obj = obj->next)
    logger(dbg_functions, basic) << *obj;

  for (hostgroup const* obj = hostgroup_list; obj != NULL; obj = obj->next)
    logger(dbg_functions, basic) << *obj;

  for (servicegroup const* obj = servicegroup_list; obj != NULL; obj = obj->next)
    logger(dbg_functions, basic) << *obj;

  // for (timeperiod const* obj = timeperiod_list; obj != NULL; obj = obj->next)
  //   logger(dbg_functions, basic) << *obj;

  for (serviceescalation const* obj = serviceescalation_list; obj != NULL; obj = obj->next)
    logger(dbg_functions, basic) << *obj;

  for (servicedependency const* obj = servicedependency_list; obj != NULL; obj = obj->next)
    logger(dbg_functions, basic) << *obj;

  for (hostdependency const* obj = hostdependency_list; obj != NULL; obj = obj->next)
    logger(dbg_functions, basic) << *obj;

  for (hostescalation const* obj = hostescalation_list; obj != NULL; obj = obj->next)
    logger(dbg_functions, basic) << *obj;
}

QTextStream& logging::operator<<(QTextStream& stream, host const& obj) {
  stream << "dump object\n"                                             \
    "host (" << &obj << ") {\n"                                         \
    "  name:                                 " << checkstr(obj.name) << "\n" \
    "  display_name:                         " << checkstr(obj.display_name) << "\n" \
    "  alias:                                " << checkstr(obj.alias) << "\n" \
    "  address:                              " << checkstr(obj.address) << "\n";

  stream << "  parent_hosts:                         { ";
  for (hostsmember const* hm = obj.parent_hosts; hm != NULL; hm = hm->next)
    stream << checkstr(hm->host_name) << (hm->next ? ", " : "");
  stream << (obj.parent_hosts ? " }\n" : "\"NULL\" }\n");

  stream << "  child_hosts:                          { ";
  for (hostsmember const* hm = obj.child_hosts; hm != NULL; hm = hm->next)
    stream << checkstr(hm->host_name) << (hm->next ? ", " : "");
  stream << (obj.child_hosts ? " }\n" : "\"NULL\" }\n");

  stream << "  services:                             { ";
  for (servicesmember const* sm = obj.services; sm != NULL; sm = sm->next)
    stream << "{ " << checkstr(sm->host_name) << ", "
           << checkstr(sm->service_description) << " }"
           << (sm->next ? ", " : "");
  stream << (obj.services ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  host_check_command:                   " << checkstr(obj.host_check_command) << "\n" \
    "  initial_state:                        " << obj.initial_state << "\n" \
    "  check_interval:                       " << obj.check_interval << "\n" \
    "  retry_interval:                       " << obj.retry_interval << "\n" \
    "  max_attempts:                         " << obj.max_attempts << "\n" \
    "  event_handler:                        " << checkstr(obj.event_handler) << "\n";

  stream << "  contact_groups:                       { ";
  for (contactgroupsmember const* cgm = obj.contact_groups; cgm != NULL; cgm = cgm->next)
    stream << checkstr(cgm->group_name) << (cgm->next ? ", " : "");
  stream << (obj.contact_groups ? " }\n" : "\"NULL\" }\n");

  stream << "  contacts:                             { ";
  for (contactsmember const* cm = obj.contacts; cm != NULL; cm = cm->next)
    stream << checkstr(cm->contact_name) << (cm->next ? ", " : "");
  stream << (obj.contacts ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  notification_interval:                " << obj.notification_interval << "\n" \
    "  first_notification_delay:             " << obj.first_notification_delay << "\n" \
    "  notify_on_down:                       " << obj.notify_on_down << "\n" \
    "  notify_on_unreachable:                " << obj.notify_on_unreachable << "\n" \
    "  notify_on_recovery:                   " << obj.notify_on_recovery << "\n" \
    "  notify_on_flapping:                   " << obj.notify_on_flapping << "\n" \
    "  notify_on_downtime:                   " << obj.notify_on_downtime << "\n" \
    "  notification_period:                  " << checkstr(obj.notification_period) << "\n" \
    "  check_period:                         " << checkstr(obj.check_period) << "\n" \
    "  flap_detection_enabled:               " << obj.flap_detection_enabled << "\n" \
    "  low_flap_threshold:                   " << obj.low_flap_threshold << "\n" \
    "  high_flap_threshold:                  " << obj.high_flap_threshold << "\n" \
    "  flap_detection_on_up:                 " << obj.flap_detection_on_up << "\n" \
    "  flap_detection_on_down:               " << obj.flap_detection_on_down << "\n" \
    "  flap_detection_on_unreachable:        " << obj.flap_detection_on_unreachable << "\n" \
    "  stalk_on_up:                          " << obj.stalk_on_up << "\n" \
    "  stalk_on_down:                        " << obj.stalk_on_down << "\n" \
    "  stalk_on_unreachable:                 " << obj.stalk_on_unreachable << "\n" \
    "  check_freshness:                      " << obj.check_freshness << "\n" \
    "  freshness_threshold:                  " << obj.freshness_threshold << "\n" \
    "  process_performance_data:             " << obj.process_performance_data << "\n" \
    "  checks_enabled:                       " << obj.checks_enabled << "\n" \
    "  accept_passive_host_checks:           " << obj.accept_passive_host_checks << "\n" \
    "  event_handler_enabled:                " << obj.event_handler_enabled << "\n" \
    "  retain_status_information:            " << obj.retain_status_information << "\n" \
    "  retain_nonstatus_information:         " << obj.retain_nonstatus_information << "\n" \
    "  failure_prediction_enabled:           " << obj.failure_prediction_enabled << "\n" \
    "  failure_prediction_options:           " << checkstr(obj.failure_prediction_options) << "\n" \
    "  obsess_over_host:                     " << obj.obsess_over_host << "\n" \
    "  notes:                                " << checkstr(obj.notes) << "\n" \
    "  notes_url:                            " << checkstr(obj.notes_url) << "\n" \
    "  action_url:                           " << checkstr(obj.action_url) << "\n" \
    "  icon_image:                           " << checkstr(obj.icon_image) << "\n" \
    "  icon_image_alt:                       " << checkstr(obj.icon_image_alt) << "\n" \
    "  vrml_image:                           " << checkstr(obj.vrml_image) << "\n" \
    "  statusmap_image:                      " << checkstr(obj.statusmap_image) << "\n" \
    "  have_2d_coords:                       " << obj.have_2d_coords << "\n" \
    "  x_2d:                                 " << obj.x_2d << "\n" \
    "  y_2d:                                 " << obj.y_2d << "\n" \
    "  have_3d_coords:                       " << obj.have_3d_coords << "\n" \
    "  x_3d:                                 " << obj.x_3d << "\n" \
    "  y_3d:                                 " << obj.y_3d << "\n" \
    "  z_3d:                                 " << obj.z_3d << "\n" \
    "  should_be_drawn:                      " << obj.should_be_drawn << "\n";

  stream << "  custom_variables:                     { ";
  for (customvariablesmember const* cvm = obj.custom_variables; cvm != NULL; cvm = cvm->next)
    stream << checkstr(cvm->variable_name) << (cvm->next ? ", " : "");
  stream << (obj.custom_variables ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  problem_has_been_acknowledged:        " << obj.problem_has_been_acknowledged << "\n" \
    "  acknowledgement_type:                 " << obj.acknowledgement_type << "\n" \
    "  check_type:                           " << obj.check_type << "\n" \
    "  current_state:                        " << obj.current_state << "\n" \
    "  last_state:                           " << obj.last_state << "\n" \
    "  last_hard_state:                      " << obj.last_hard_state << "\n" \
    "  plugin_output:                        " << checkstr(obj.plugin_output) << "\n" \
    "  long_plugin_output:                   " << checkstr(obj.long_plugin_output) << "\n" \
    "  perf_data:                            " << checkstr(obj.perf_data) << "\n" \
    "  state_type:                           " << obj.state_type << "\n" \
    "  current_attempt:                      " << obj.current_attempt << "\n" \
    "  current_event_id:                     " << obj.current_event_id << "\n" \
    "  last_event_id:                        " << obj.last_event_id << "\n" \
    "  current_problem_id:                   " << obj.current_problem_id << "\n" \
    "  last_problem_id:                      " << obj.last_problem_id << "\n" \
    "  latency:                              " << obj.latency << "\n" \
    "  execution_time:                       " << obj.execution_time << "\n" \
    "  is_executing:                         " << obj.is_executing << "\n" \
    "  check_options:                        " << obj.check_options << "\n" \
    "  notifications_enabled:                " << obj.notifications_enabled << "\n" \
    "  last_host_notification:               " << obj.last_host_notification << "\n" \
    "  next_host_notification:               " << obj.next_host_notification << "\n" \
    "  next_check:                           " << obj.next_check << "\n" \
    "  should_be_scheduled:                  " << obj.should_be_scheduled << "\n" \
    "  last_check:                           " << obj.last_check << "\n" \
    "  last_state_change:                    " << obj.last_state_change << "\n" \
    "  last_hard_state_change:               " << obj.last_hard_state_change << "\n" \
    "  last_time_up:                         " << obj.last_time_up << "\n" \
    "  last_time_down:                       " << obj.last_time_down << "\n" \
    "  last_time_unreachable:                " << obj.last_time_unreachable << "\n" \
    "  has_been_checked:                     " << obj.has_been_checked << "\n" \
    "  is_being_freshened:                   " << obj.is_being_freshened << "\n" \
    "  notified_on_down:                     " << obj.notified_on_down << "\n" \
    "  notified_on_unreachable:              " << obj.notified_on_unreachable << "\n" \
    "  current_notification_number:          " << obj.current_notification_number << "\n" \
    "  no_more_notifications:                " << obj.no_more_notifications << "\n" \
    "  current_notification_id:              " << obj.current_notification_id << "\n" \
    "  check_flapping_recovery_notification: " << obj.check_flapping_recovery_notification << "\n" \
    "  scheduled_downtime_depth:             " << obj.scheduled_downtime_depth << "\n" \
    "  pending_flex_downtime:                " << obj.pending_flex_downtime << "\n" \
    "  state_history:                        " << obj.state_history << "\n" \
    "  state_history_index:                  " << obj.state_history_index << "\n" \
    "  last_state_history_update:            " << obj.last_state_history_update << "\n" \
    "  is_flapping:                          " << obj.is_flapping << "\n" \
    "  flapping_comment_id:                  " << obj.flapping_comment_id << "\n" \
    "  percent_state_change:                 " << obj.percent_state_change << "\n" \
    "  total_services:                       " << obj.total_services << "\n" \
    "  total_service_check_interval:         " << obj.total_service_check_interval << "\n" \
    "  modified_attributes:                  " << obj.modified_attributes << "\n" \
    "  circular_path_checked:                " << obj.circular_path_checked << "\n" \
    "  contains_circular_path:               " << obj.contains_circular_path << "\n" \
    "  event_handler_ptr:                    " << obj.event_handler_ptr << "\n" \
    "  check_command_ptr:                    " << obj.check_command_ptr << "\n" \
    "  check_period_ptr:                     " << obj.check_period_ptr << "\n" \
    "  notification_period_ptr:              " << obj.notification_period_ptr << "\n" \
    "  hostgroups_ptr:                       " << obj.hostgroups_ptr << "\n" \
    "}\n";
  return (stream);
}

QTextStream& logging::operator<<(QTextStream& stream, service const& obj) {
  stream << "dump object\n"                                             \
    "service (" << &obj << ") {\n"                                      \
    "  host_name:                            " << checkstr(obj.host_name) << "\n" \
    "  description:                          " << checkstr(obj.description) << "\n" \
    "  display_name:                         " << checkstr(obj.display_name) << "\n" \
    "  service_check_command:                " << checkstr(obj.service_check_command) << "\n" \
    "  event_handler:                        " << checkstr(obj.event_handler) << "\n" \
    "  initial_state:                        " << obj.initial_state << "\n" \
    "  check_interval:                       " << obj.check_interval << "\n" \
    "  retry_interval:                       " << obj.retry_interval << "\n" \
    "  max_attempts:                         " << obj.max_attempts << "\n" \
    "  parallelize:                          " << obj.parallelize << "\n";

  stream << "  contact_groups:                       { ";
  for (contactgroupsmember const* cgm = obj.contact_groups; cgm != NULL; cgm = cgm->next)
    stream << checkstr(cgm->group_name) << (cgm->next ? ", " : "");
  stream << (obj.contact_groups ? " }\n" : "\"NULL\" }\n");

  stream << "  contacts:                             { ";
  for (contactsmember const* cm = obj.contacts; cm != NULL; cm = cm->next)
    stream << checkstr(cm->contact_name) << (cm->next ? ", " : "");
  stream << (obj.contacts ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  notification_interval:                " << obj.notification_interval << "\n" \
    "  first_notification_delay:             " << obj.first_notification_delay << "\n" \
    "  notify_on_unknown:                    " << obj.notify_on_unknown << "\n" \
    "  notify_on_warning:                    " << obj.notify_on_warning << "\n" \
    "  notify_on_critical:                   " << obj.notify_on_critical << "\n" \
    "  notify_on_recovery:                   " << obj.notify_on_recovery << "\n" \
    "  notify_on_flapping:                   " << obj.notify_on_flapping << "\n" \
    "  notify_on_downtime:                   " << obj.notify_on_downtime << "\n" \
    "  stalk_on_ok:                          " << obj.stalk_on_ok << "\n" \
    "  stalk_on_warning:                     " << obj.stalk_on_warning << "\n" \
    "  stalk_on_unknown:                     " << obj.stalk_on_unknown << "\n" \
    "  stalk_on_critical:                    " << obj.stalk_on_critical << "\n" \
    "  is_volatile:                          " << obj.is_volatile << "\n" \
    "  notification_period:                  " << checkstr(obj.notification_period) << "\n" \
    "  check_period:                         " << checkstr(obj.check_period) << "\n" \
    "  flap_detection_enabled:               " << obj.flap_detection_enabled << "\n" \
    "  low_flap_threshold:                   " << obj.low_flap_threshold << "\n" \
    "  high_flap_threshold:                  " << obj.high_flap_threshold << "\n" \
    "  flap_detection_on_ok:                 " << obj.flap_detection_on_ok << "\n" \
    "  flap_detection_on_warning:            " << obj.flap_detection_on_warning << "\n" \
    "  flap_detection_on_unknown:            " << obj.flap_detection_on_unknown << "\n" \
    "  flap_detection_on_critical:           " << obj.flap_detection_on_critical << "\n" \
    "  process_performance_data:             " << obj.process_performance_data << "\n" \
    "  check_freshness:                      " << obj.check_freshness << "\n" \
    "  freshness_threshold:                  " << obj.freshness_threshold << "\n" \
    "  accept_passive_service_checks:        " << obj.accept_passive_service_checks << "\n" \
    "  event_handler_enabled:                " << obj.event_handler_enabled << "\n" \
    "  checks_enabled:                       " << obj.checks_enabled << "\n" \
    "  retain_status_information:            " << obj.retain_status_information << "\n" \
    "  retain_nonstatus_information:         " << obj.retain_nonstatus_information << "\n" \
    "  notifications_enabled:                " << obj.notifications_enabled << "\n" \
    "  obsess_over_service:                  " << obj.obsess_over_service << "\n" \
    "  failure_prediction_enabled:           " << obj.failure_prediction_enabled << "\n" \
    "  failure_prediction_options:           " << checkstr(obj.failure_prediction_options) << "\n" \
    "  notes:                                " << checkstr(obj.notes) << "\n" \
    "  notes_url:                            " << checkstr(obj.notes_url) << "\n" \
    "  action_url:                           " << checkstr(obj.action_url) << "\n" \
    "  icon_image:                           " << checkstr(obj.icon_image) << "\n" \
    "  icon_image_alt:                       " << checkstr(obj.icon_image_alt) << "\n";

  stream << "  custom_variables:                     { ";
  for (customvariablesmember const* cvm = obj.custom_variables; cvm != NULL; cvm = cvm->next)
    stream << checkstr(cvm->variable_name) << (cvm->next ? ", " : "");
  stream << (obj.custom_variables ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  problem_has_been_acknowledged:        " << obj.problem_has_been_acknowledged << "\n" \
    "  acknowledgement_type:                 " << obj.acknowledgement_type << "\n" \
    "  host_problem_at_last_check:           " << obj.host_problem_at_last_check << "\n" \
    "  check_type:                           " << obj.check_type << "\n" \
    "  current_state:                        " << obj.current_state << "\n" \
    "  last_state:                           " << obj.last_state << "\n" \
    "  last_hard_state:                      " << obj.last_hard_state << "\n" \
    "  plugin_output:                        " << checkstr(obj.plugin_output) << "\n" \
    "  long_plugin_output:                   " << checkstr(obj.long_plugin_output) << "\n" \
    "  perf_data:                            " << checkstr(obj.perf_data) << "\n" \
    "  state_type:                           " << obj.state_type << "\n" \
    "  next_check:                           " << obj.next_check << "\n" \
    "  should_be_scheduled:                  " << obj.should_be_scheduled << "\n" \
    "  last_check:                           " << obj.last_check << "\n" \
    "  current_attempt:                      " << obj.current_attempt << "\n" \
    "  current_event_id:                     " << obj.current_event_id << "\n" \
    "  last_event_id:                        " << obj.last_event_id << "\n" \
    "  current_problem_id:                   " << obj.current_problem_id << "\n" \
    "  last_problem_id:                      " << obj.last_problem_id << "\n" \
    "  last_notification:                    " << obj.last_notification << "\n" \
    "  next_notification:                    " << obj.next_notification << "\n" \
    "  no_more_notifications:                " << obj.no_more_notifications << "\n" \
    "  check_flapping_recovery_notification: " << obj.check_flapping_recovery_notification << "\n" \
    "  last_state_change:                    " << obj.last_state_change << "\n" \
    "  last_hard_state_change:               " << obj.last_hard_state_change << "\n" \
    "  last_time_ok:                         " << obj.last_time_ok << "\n" \
    "  last_time_warning:                    " << obj.last_time_warning << "\n" \
    "  last_time_unknown:                    " << obj.last_time_unknown << "\n" \
    "  last_time_critical:                   " << obj.last_time_critical << "\n" \
    "  has_been_checked:                     " << obj.has_been_checked << "\n" \
    "  is_being_freshened:                   " << obj.is_being_freshened << "\n" \
    "  notified_on_unknown:                  " << obj.notified_on_unknown << "\n" \
    "  notified_on_warning:                  " << obj.notified_on_warning << "\n" \
    "  notified_on_critical:                 " << obj.notified_on_critical << "\n" \
    "  current_notification_number:          " << obj.current_notification_number << "\n" \
    "  current_notification_id:              " << obj.current_notification_id << "\n" \
    "  latency:                              " << obj.latency << "\n" \
    "  execution_time:                       " << obj.execution_time << "\n" \
    "  is_executing:                         " << obj.is_executing << "\n" \
    "  check_options:                        " << obj.check_options << "\n" \
    "  scheduled_downtime_depth:             " << obj.scheduled_downtime_depth << "\n" \
    "  pending_flex_downtime:                " << obj.pending_flex_downtime << "\n" \
    "  state_history:                        " << obj.state_history << "\n" \
    "  state_history_index:                  " << obj.state_history_index << "\n"     \
    "  is_flapping:                          " << obj.is_flapping << "\n" \
    "  flapping_comment_id:                  " << obj.flapping_comment_id << "\n" \
    "  percent_state_change:                 " << obj.percent_state_change << "\n" \
    "  modified_attributes:                  " << obj.modified_attributes << "\n" \
    "  host_ptr:                             " << obj.host_ptr << "\n"  \
    "  event_handler_ptr:                    " << obj.event_handler_ptr << "\n" \
    "  event_handler_args:                   " << checkstr(obj.event_handler_args) << "\n" \
    "  check_command_ptr:                    " << obj.check_command_ptr << "\n" \
    "  check_command_args:                   " << checkstr(obj.check_command_args) << "\n" \
    "  check_period_ptr:                     " << obj.check_period_ptr << "\n" \
    "  notification_period_ptr:              " << obj.notification_period_ptr << "\n" \
    "  servicegroups_ptr:                    " << obj.servicegroups_ptr << "\n" \
    "}\n";

  return (stream);
}

QTextStream& logging::operator<<(QTextStream& stream, sched_info const& obj) {
  stream << "dump object\n"                                             \
    "sched_info (" << &obj << ") {\n"                                     \
    "  total_services:                      " << obj.total_services << "\n" \
    "  total_scheduled_services:            " << obj.total_scheduled_services << "\n" \
    "  total_hosts:                         " << obj.total_hosts << "\n" \
    "  total_scheduled_hosts:               " << obj.total_scheduled_hosts << "\n" \
    "  average_services_per_host:           " << obj.average_services_per_host << "\n" \
    "  average_scheduled_services_per_host: " << obj.average_scheduled_services_per_host << "\n" \
    "  service_check_interval_total:        " << obj.service_check_interval_total << "\n" \
    "  host_check_interval_total:           " << obj.host_check_interval_total << "\n" \
    "  average_service_execution_time:      " << obj.average_service_execution_time << "\n" \
    "  average_service_check_interval:      " << obj.average_service_check_interval << "\n" \
    "  average_host_check_interval:         " << obj.average_host_check_interval << "\n" \
    "  average_service_inter_check_delay:   " << obj.average_service_inter_check_delay << "\n" \
    "  average_host_inter_check_delay:      " << obj.average_host_inter_check_delay << "\n" \
    "  service_inter_check_delay:           " << obj.service_inter_check_delay << "\n" \
    "  host_inter_check_delay:              " << obj.host_inter_check_delay << "\n" \
    "  service_interleave_factor:           " << obj.service_interleave_factor << "\n" \
    "  max_service_check_spread:            " << obj.max_service_check_spread << "\n" \
    "  max_host_check_spread:               " << obj.max_host_check_spread << "\n"  \
    "  first_service_check:                 " << obj.first_service_check << "\n" \
    "  last_service_check:                  " << obj.last_service_check << "\n" \
    "  first_host_check:                    " << obj.first_host_check << "\n" \
    "  last_host_check:                     " << obj.last_host_check << "\n" \
    "}\n";

  return (stream);
}

QTextStream& logging::operator<<(QTextStream& stream, command const& obj) {
  stream << "dump object\n"                                  \
    "command (" << &obj << ") {\n"                           \
    "  name:         " << checkstr(obj.name) << "\n"         \
    "  command_line: " << checkstr(obj.command_line) << "\n" \
    "}\n";
  return (stream);
}

QTextStream& logging::operator<<(QTextStream& stream, contact const& obj) {
  stream << "dump object\n"                                             \
    "contact (" << &obj << ") {\n"                                      \
    "  name:                            " << checkstr(obj.name) << "\n" \
    "  alias:                           " << checkstr(obj.alias) << "\n" \
    "  email:                           " << checkstr(obj.email) << "\n" \
    "  pager:                           " << checkstr(obj.pager) << "\n";

  stream << "  address:                         { ";
  for (unsigned int i = 0; i < MAX_CONTACT_ADDRESSES && obj.address[i] != NULL; ++i)
    stream << checkstr(obj.address[i])
           << (i < MAX_CONTACT_ADDRESSES && obj.address[i] != NULL ? ", " : "");
  stream << (obj.address[0] != NULL ? " }\n" : "\"NULL\" }\n");

  stream << "  host_notification_commands:      { ";
  for (commandsmember const* cm = obj.host_notification_commands; cm != NULL; cm = cm->next)
    stream << checkstr(cm->cmd) << (cm->next ? ", " : "");
  stream << (obj.host_notification_commands ? " }\n" : "\"NULL\" }\n");

  stream << "  service_notification_commands:   { ";
  for (commandsmember const* cm = obj.service_notification_commands; cm != NULL; cm = cm->next)
    stream << checkstr(cm->cmd) << (cm->next ? ", " : "");
  stream << (obj.service_notification_commands ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  notify_on_service_unknown:       " << obj.notify_on_service_unknown << "\n" \
    "  notify_on_service_warning:       " << obj.notify_on_service_warning << "\n" \
    "  notify_on_service_critical:      " << obj.notify_on_service_critical << "\n" \
    "  notify_on_service_recovery:      " << obj.notify_on_service_recovery << "\n" \
    "  notify_on_service_flapping:      " << obj.notify_on_service_flapping << "\n" \
    "  notify_on_service_downtime:      " << obj.notify_on_service_downtime << "\n" \
    "  notify_on_host_down:             " << obj.notify_on_host_down << "\n" \
    "  notify_on_host_unreachable:      " << obj.notify_on_host_unreachable << "\n" \
    "  notify_on_host_recovery:         " << obj.notify_on_host_recovery << "\n" \
    "  notify_on_host_flapping:         " << obj.notify_on_host_flapping << "\n" \
    "  notify_on_host_downtime:         " << obj.notify_on_host_downtime << "\n" \
    "  host_notification_period:        " << checkstr(obj.host_notification_period) << "\n" \
    "  service_notification_period:     " << checkstr(obj.service_notification_period) << "\n" \
    "  host_notifications_enabled:      " << obj.host_notifications_enabled << "\n" \
    "  service_notifications_enabled:   " << obj.service_notifications_enabled << "\n" \
    "  can_submit_commands:             " << obj.can_submit_commands << "\n" \
    "  retain_status_information:       " << obj.retain_status_information << "\n" \
    "  retain_nonstatus_information:    " << obj.retain_nonstatus_information << "\n";

  stream << "  custom_variables:                { ";
  for (customvariablesmember const* cvm = obj.custom_variables; cvm != NULL; cvm = cvm->next)
    stream << checkstr(cvm->variable_name) << (cvm->next ? ", " : "");
  stream << (obj.custom_variables ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  last_host_notification:          " << obj.last_host_notification << "\n" \
    "  last_service_notification:       " << obj.last_service_notification << "\n" \
    "  modified_attributes:             " << obj.modified_attributes << "\n" \
    "  modified_host_attributes:        " << obj.modified_host_attributes << "\n" \
    "  modified_service_attributes:     " << obj.modified_service_attributes << "\n" \
    "  host_notification_period_ptr:    " << obj.host_notification_period_ptr << "\n" \
    "  service_notification_period_ptr: " << obj.service_notification_period_ptr << "\n" \
    "  contactgroups_ptr:               " << obj.contactgroups_ptr << "\n" \
    "}\n";
  return (stream);
}

QTextStream& logging::operator<<(QTextStream& stream, hostgroup const& obj) {
  stream << "dump object\n"                                             \
    "hostgroup (" << &obj << ") {\n"                                    \
    "  group_name: " << checkstr(obj.group_name) << "\n"                \
    "  alias:      " << checkstr(obj.alias) << "\n";

  stream << "  members:    { ";
  for (hostsmember const* hm = obj.members; hm != NULL; hm = hm->next)
    stream << checkstr(hm->host_name) << (hm->next ? ", " : "");
  stream << (obj.members ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  notes:      " << checkstr(obj.notes) << "\n"                     \
    "  notes_url:  " << checkstr(obj.notes_url) << "\n"                 \
    "  action_url: " << checkstr(obj.action_url) << "\n"                \
    "}\n";
  return (stream);
}

QTextStream& logging::operator<<(QTextStream& stream, servicegroup const& obj) {
  stream << "dump object\n"                                             \
    "servicegroup (" << &obj << ") {\n"                                 \
    "  group_name: " << checkstr(obj.group_name) << "\n"                \
    "  alias:      " << checkstr(obj.alias) << "\n";

  stream << "  members:    { ";
  for (servicesmember const* sm = obj.members; sm != NULL; sm = sm->next)
    stream << checkstr(sm->host_name) << (sm->next ? ", " : "");
  stream << (obj.members ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  notes:      " << checkstr(obj.notes) << "\n"                     \
    "  notes_url:  " << checkstr(obj.notes_url) << "\n"                 \
    "  action_url: " << checkstr(obj.action_url) << "\n"                \
    "}\n";
  return (stream);
}

QTextStream& logging::operator<<(QTextStream& stream, contactgroup const& obj) {
  stream << "dump object\n"                                             \
    "contactgroup (" << &obj << ") {\n"                                 \
    "  group_name: " << checkstr(obj.group_name) << "\n"                \
    "  alias:      " << checkstr(obj.alias) << "\n";

  stream << "  members:    { ";
  for (contactsmember const* cm = obj.members; cm != NULL; cm = cm->next)
    stream << checkstr(cm->contact_name) << (cm->next ? ", " : "");
  stream << (obj.members ? " }\n" : "\"NULL\" }\n");

  stream << "}\n";
  return (stream);
}

QTextStream& logging::operator<<(QTextStream& stream, serviceescalation const& obj) {
  stream << "dump object\n"                                             \
    "serviceescalation (" << &obj << ") {\n"                            \
    "  host_name:             " << checkstr(obj.host_name) << "\n"      \
    "  description:           " << checkstr(obj.description) << "\n"    \
    "  first_notification:    " << obj.first_notification << "\n"       \
    "  last_notification:     " << obj.last_notification << "\n"        \
    "  notification_interval: " << obj.notification_interval << "\n"    \
    "  escalation_period:     " << checkstr(obj.escalation_period) << "\n" \
    "  escalate_on_recovery:  " << obj.escalate_on_recovery << "\n"     \
    "  escalate_on_warning:   " << obj.escalate_on_warning << "\n"      \
    "  escalate_on_unknown:   " << obj.escalate_on_unknown << "\n"      \
    "  escalate_on_critical:  " << obj.escalate_on_critical << "\n";

  stream << "  contact_groups:        { ";
  for (contactgroupsmember const* cgm = obj.contact_groups; cgm != NULL; cgm = cgm->next)
    stream << checkstr(cgm->group_name) << (cgm->next ? ", " : "");
  stream << (obj.contact_groups ? " }\n" : "\"NULL\" }\n");

  stream << "  contacts:              { ";
  for (contactsmember const* cm = obj.contacts; cm != NULL; cm = cm->next)
    stream << checkstr(cm->contact_name) << (cm->next ? ", " : "");
  stream << (obj.contacts ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  service_ptr:           " << obj.service_ptr << "\n"           \
    "  escalation_period_ptr: " << obj.escalation_period_ptr << "\n" \
    "}\n";
  return (stream);
}

QTextStream& logging::operator<<(QTextStream& stream, servicedependency const& obj) {
  stream << "dump object\n"                                             \
    "servicedependency (" << &obj << ") {\n"                            \
    "  dependency_type:               " << obj.dependency_type << "\n"  \
    "  dependent_host_name:           " << checkstr(obj.dependent_host_name) << "\n" \
    "  dependent_service_description: " << checkstr(obj.dependent_service_description) << "\n" \
    "  host_name:                     " << checkstr(obj.host_name) << "\n" \
    "  service_description:           " << checkstr(obj.service_description) << "\n" \
    "  dependency_period:             " << checkstr(obj.dependency_period) << "\n" \
    "  inherits_parent:               " << obj.inherits_parent << "\n"  \
    "  fail_on_ok:                    " << obj.fail_on_ok << "\n"       \
    "  fail_on_warning:               " << obj.fail_on_warning << "\n"  \
    "  fail_on_unknown:               " << obj.fail_on_unknown << "\n"  \
    "  fail_on_critical:              " << obj.fail_on_critical << "\n" \
    "  fail_on_pending:               " << obj.fail_on_pending << "\n"  \
    "  circular_path_checked:         " << obj.circular_path_checked << "\n" \
    "  contains_circular_path:        " << obj.contains_circular_path << "\n" \
    "  master_service_ptr:            " << obj.master_service_ptr << "\n" \
    "  dependent_service_ptr:         " << obj.dependent_service_ptr << "\n" \
    "  dependency_period_ptr:         " << obj.dependency_period_ptr << "\n" \
    "}\n";
  return (stream);
}

QTextStream& logging::operator<<(QTextStream& stream, hostescalation const& obj) {
  stream << "dump object\n"                     \
    "hostescalation (" << &obj << ") {\n"       \
    "  host_name:               " << checkstr(obj.host_name) << "\n"      \
    "  first_notification:      " << obj.first_notification << "\n"       \
    "  last_notification:       " << obj.last_notification << "\n"        \
    "  notification_interval:   " << obj.notification_interval << "\n"    \
    "  escalation_period:       " << checkstr(obj.escalation_period) << "\n" \
    "  escalate_on_recovery:    " << obj.escalate_on_recovery << "\n"     \
    "  escalate_on_down:        " << obj.escalate_on_down << "\n"      \
    "  escalate_on_unreachable: " << obj.escalate_on_unreachable << "\n";

  stream << "  contact_groups:          { ";
  for (contactgroupsmember const* cgm = obj.contact_groups; cgm != NULL; cgm = cgm->next)
    stream << checkstr(cgm->group_name) << (cgm->next ? ", " : "");
  stream << (obj.contact_groups ? " }\n" : "\"NULL\" }\n");

  stream << "  contacts:                { ";
  for (contactsmember const* cm = obj.contacts; cm != NULL; cm = cm->next)
    stream << checkstr(cm->contact_name) << (cm->next ? ", " : "");
  stream << (obj.contacts ? " }\n" : "\"NULL\" }\n");

  stream <<
    "  host_ptr:                " << obj.host_ptr << "\n"              \
    "  escalation_period_ptr:   " << obj.escalation_period_ptr << "\n" \
    "}\n";
  return (stream);
}

QTextStream& logging::operator<<(QTextStream& stream, hostdependency const& obj) {
  stream << "dump object\n"                                             \
    "hostdependency (" << &obj << ") {\n"                               \
    "  dependency_type:        " << obj.dependency_type << "\n"         \
    "  dependent_host_name:    " << checkstr(obj.dependent_host_name) << "\n" \
    "  host_name:              " << checkstr(obj.host_name) << "\n"     \
    "  dependency_period:      " << checkstr(obj.dependency_period) << "\n" \
    "  inherits_parent:        " << obj.inherits_parent << "\n"         \
    "  fail_on_up:             " << obj.fail_on_up << "\n"              \
    "  fail_on_down:           " << obj.fail_on_down << "\n"            \
    "  fail_on_unreachable:    " << obj.fail_on_unreachable << "\n"     \
    "  fail_on_pending:        " << obj.fail_on_pending << "\n"         \
    "  circular_path_checked:  " << obj.circular_path_checked << "\n"   \
    "  contains_circular_path: " << obj.contains_circular_path << "\n"  \
    "  master_host_ptr:        " << obj.master_host_ptr << "\n"         \
    "  dependent_host_ptr:     " << obj.dependent_host_ptr << "\n"      \
    "  dependency_period_ptr:  " << obj.dependency_period_ptr << "\n"   \
    "}\n";
  return (stream);
}

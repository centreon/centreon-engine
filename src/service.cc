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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/objectlist.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

service2::~service2() {
  this->contact_groups.clear();
  deleter::listmember(this->servicegroups_ptr, &deleter::objectlist);

  delete[] this->display_name;
  this->display_name = NULL;
  delete[] this->service_check_command;
  this->service_check_command = NULL;
  delete[] this->event_handler;
  this->event_handler = NULL;
  delete[] this->notification_period;
  this->notification_period = NULL;
  delete[] this->check_period;
  this->check_period = NULL;
  delete[] this->failure_prediction_options;
  this->failure_prediction_options = NULL;
  delete[] this->notes;
  this->notes = NULL;
  delete[] this->notes_url;
  this->notes_url = NULL;
  delete[] this->action_url;
  this->action_url = NULL;
  delete[] this->icon_image;
  this->icon_image = NULL;
  delete[] this->icon_image_alt;
  this->icon_image_alt = NULL;
  delete[] this->plugin_output;
  this->plugin_output = NULL;
  delete[] this->long_plugin_output;
  this->long_plugin_output = NULL;
  delete[] this->perf_data;
  this->perf_data = NULL;
  delete[] this->event_handler_args;
  this->event_handler_args = NULL;
  delete[] this->check_command_args;
  this->check_command_args = NULL;
}

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       com::centreon::engine::service2 const& obj1,
       com::centreon::engine::service2 const& obj2) throw () {
  return obj1.get_hostname() == obj2.get_hostname()
          && obj1.get_description() == obj2.get_description()
          && is_equal(obj1.display_name, obj2.display_name)
          && is_equal(obj1.service_check_command, obj2.service_check_command)
          && is_equal(obj1.event_handler, obj2.event_handler)
          && obj1.initial_state == obj2.initial_state
          && obj1.check_interval == obj2.check_interval
          && obj1.retry_interval == obj2.retry_interval
          && obj1.max_attempts == obj2.max_attempts
          && obj1.parallelize == obj2.parallelize
          && ((obj1.contact_groups.size() == obj2.contact_groups.size()) &&
               std::equal(obj1.contact_groups.begin(),
                          obj1.contact_groups.end(),
                          obj2.contact_groups.begin()))
          && ((obj1.contacts.size() == obj2.contacts.size()) &&
               std::equal(obj1.contacts.begin(),
                          obj1.contacts.end(),
                          obj2.contacts.begin()))
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
          && is_equal(obj1.notes, obj2.notes)
          && is_equal(obj1.notes_url, obj2.notes_url)
          && is_equal(obj1.action_url, obj2.action_url)
          && is_equal(obj1.icon_image, obj2.icon_image)
          && is_equal(obj1.icon_image_alt, obj2.icon_image_alt)
          && obj1.custom_variables == obj2.custom_variables
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
          && is_equal(obj1.check_command_args, obj2.check_command_args);
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
       com::centreon::engine::service2 const& obj1,
       com::centreon::engine::service2 const& obj2) throw () {
  return !operator==(obj1, obj2);
}

/**
 *  Dump service content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The service to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, com::centreon::engine::service2 const& obj) {
  char const* evt_str(nullptr);
  if (obj.event_handler_ptr)
    evt_str = obj.event_handler_ptr->get_name().c_str();
  char const* cmd_str(nullptr);
  if (obj.check_command_ptr)
    cmd_str = obj.check_command_ptr->get_name().c_str();
  char const* chk_period_str(nullptr);
  if (obj.check_period_ptr)
    chk_period_str = chkstr(obj.check_period_ptr->name);
  char const* notif_period_str(nullptr);
  if (obj.notification_period_ptr)
    notif_period_str = chkstr(obj.notification_period_ptr->name);
  char const* svcgrp_str(nullptr);
  if (obj.servicegroups_ptr)
    svcgrp_str = chkstr(static_cast<servicegroup const*>(obj.servicegroups_ptr->object_ptr)->group_name);

  std::string cg_oss;
  std::string c_oss;

  if (obj.contact_groups.empty())
    cg_oss = "\"nullptr\"";
  else {
    std::ostringstream oss;
    oss << obj.contact_groups;
    cg_oss = oss.str();
  }
  if (obj.contacts.empty())
    c_oss = "\"nullptr\"";
  else {
    std::ostringstream oss;
    oss << obj.contacts;
    c_oss = oss.str();
  }

  os << "service {\n"
    "  host_name:                            " << obj.get_hostname() << "\n"
    "  description:                          " << obj.get_description() << "\n"
    "  display_name:                         " << chkstr(obj.display_name) << "\n"
    "  service_check_command:                " << chkstr(obj.service_check_command) << "\n"
    "  event_handler:                        " << chkstr(obj.event_handler) << "\n"
    "  initial_state:                        " << obj.initial_state << "\n"
    "  check_interval:                       " << obj.check_interval << "\n"
    "  retry_interval:                       " << obj.retry_interval << "\n"
    "  max_attempts:                         " << obj.max_attempts << "\n"
    "  parallelize:                          " << obj.parallelize << "\n"
    "  contact_groups:                       " << cg_oss << "\n"
    "  contacts:                             " << c_oss << "\n"
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
    "  notes:                                " << chkstr(obj.notes) << "\n"
    "  notes_url:                            " << chkstr(obj.notes_url) << "\n"
    "  action_url:                           " << chkstr(obj.action_url) << "\n"
    "  icon_image:                           " << chkstr(obj.icon_image) << "\n"
    "  icon_image_alt:                       " << chkstr(obj.icon_image_alt) << "\n"
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
    "  next_check:                           " << string::ctime(obj.next_check) << "\n"
    "  should_be_scheduled:                  " << obj.should_be_scheduled << "\n"
    "  last_check:                           " << string::ctime(obj.last_check) << "\n"
    "  current_attempt:                      " << obj.current_attempt << "\n"
    "  current_event_id:                     " << obj.current_event_id << "\n"
    "  last_event_id:                        " << obj.last_event_id << "\n"
    "  current_problem_id:                   " << obj.current_problem_id << "\n"
    "  last_problem_id:                      " << obj.last_problem_id << "\n"
    "  last_notification:                    " << string::ctime(obj.last_notification) << "\n"
    "  next_notification:                    " << string::ctime(obj.next_notification) << "\n"
    "  no_more_notifications:                " << obj.no_more_notifications << "\n"
    "  check_flapping_recovery_notification: " << obj.check_flapping_recovery_notification << "\n"
    "  last_state_change:                    " << string::ctime(obj.last_state_change) << "\n"
    "  last_hard_state_change:               " << string::ctime(obj.last_hard_state_change) << "\n"
    "  last_time_ok:                         " << string::ctime(obj.last_time_ok) << "\n"
    "  last_time_warning:                    " << string::ctime(obj.last_time_warning) << "\n"
    "  last_time_unknown:                    " << string::ctime(obj.last_time_unknown) << "\n"
    "  last_time_critical:                   " << string::ctime(obj.last_time_critical) << "\n"
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
    "  pending_flex_downtime:                " << obj.pending_flex_downtime << "\n";

  os << "  state_history:                        ";
  for (unsigned int i(0), end(sizeof(obj.state_history) / sizeof(obj.state_history[0]));
       i < end;
       ++i)
    os << obj.state_history[i] << (i + 1 < end ? ", " : "\n");

  os <<
    "  state_history_index:                  " << obj.state_history_index << "\n"
    "  is_flapping:                          " << obj.is_flapping << "\n"
    "  flapping_comment_id:                  " << obj.flapping_comment_id << "\n"
    "  percent_state_change:                 " << obj.percent_state_change << "\n"
    "  modified_attributes:                  " << obj.modified_attributes << "\n"
    "  host_ptr:                             " << (obj.host_ptr ? obj.host_ptr->get_name() : "\"nullptr\"") << "\n"
    "  event_handler_ptr:                    " << chkstr(evt_str) << "\n"
    "  event_handler_args:                   " << chkstr(obj.event_handler_args) << "\n"
    "  check_command_ptr:                    " << chkstr(cmd_str) << "\n"
    "  check_command_args:                   " << chkstr(obj.check_command_args) << "\n"
    "  check_period_ptr:                     " << chkstr(chk_period_str) << "\n"
    "  notification_period_ptr:              " << chkstr(notif_period_str) << "\n"
    "  servicegroups_ptr:                    " << chkstr(svcgrp_str) << "\n";

  for (std::pair<std::string, customvariable> const& cv : obj.custom_variables)
    os << cv.first << " ; ";

  os << "\n}\n";
  return os;
}

/**
 *  Add a new service to the list in memory.
 *
 *  @param[in] host_name                    Name of the host this
 *                                          service is running on.
 *  @param[in] description                  Service description.
 *  @param[in] display_name                 Display name.
 *  @param[in] check_period                 Check timeperiod name.
 *  @param[in] initial_state                Initial service state.
 *  @param[in] max_attempts                 Max check attempts.
 *  @param[in] parallelize                  Can active checks be
 *                                          parallelized ?
 *  @param[in] accept_passive_checks        Does this service accept
 *                                          check result submission ?
 *  @param[in] check_interval               Normal check interval.
 *  @param[in] retry_interval               Retry check interval.
 *  @param[in] notification_interval        Notification interval.
 *  @param[in] first_notification_delay     First notification delay.
 *  @param[in] notification_period          Notification timeperiod
 *                                          name.
 *  @param[in] notify_recovery              Does this service notify
 *                                          when recovering ?
 *  @param[in] notify_unknown               Does this service notify in
 *                                          unknown state ?
 *  @param[in] notify_warning               Does this service notify in
 *                                          warning state ?
 *  @param[in] notify_critical              Does this service notify in
 *                                          critical state ?
 *  @param[in] notify_flapping              Does this service notify
 *                                          when flapping ?
 *  @param[in] notify_downtime              Does this service notify on
 *                                          downtime ?
 *  @param[in] notifications_enabled        Are notifications enabled
 *                                          for this service ?
 *  @param[in] is_volatile                  Is this service volatile ?
 *  @param[in] event_handler                Event handler command name.
 *  @param[in] event_handler_enabled        Whether or not event handler
 *                                          is enabled.
 *  @param[in] check_command                Active check command name.
 *  @param[in] checks_enabled               Are active checks enabled ?
 *  @param[in] flap_detection_enabled       Whether or not flap
 *                                          detection is enabled.
 *  @param[in] low_flap_threshold           Low flap threshold.
 *  @param[in] high_flap_threshold          High flap threshold.
 *  @param[in] flap_detection_on_ok         Is flap detection enabled
 *                                          for ok state ?
 *  @param[in] flap_detection_on_warning    Is flap detection enabled
 *                                          for warning state ?
 *  @param[in] flap_detection_on_unknown    Is flap detection enabled
 *                                          for unknown state ?
 *  @param[in] flap_detection_on_critical   Is flap detection enabled
 *                                          for critical state ?
 *  @param[in] stalk_on_ok                  Stalk on ok state ?
 *  @param[in] stalk_on_warning             Stalk on warning state ?
 *  @param[in] stalk_on_unknown             Stalk on unknown state ?
 *  @param[in] stalk_on_critical            Stalk on critical state ?
 *  @param[in] process_perfdata             Whether or not service
 *                                          performance data should be
 *                                          processed.
 *  @param[in] failure_prediction_enabled   Deprecated.
 *  @param[in] failure_prediction_options   Deprecated.
 *  @param[in] check_freshness              Enable freshness check ?
 *  @param[in] freshness_threshold          Freshness threshold.
 *  @param[in] notes                        Notes.
 *  @param[in] notes_url                    URL.
 *  @param[in] action_url                   Action URL.
 *  @param[in] icon_image                   Icon image.
 *  @param[in] icon_image_alt               Alternative icon image.
 *  @param[in] retain_status_information    Should Engine retain service
 *                                          status information ?
 *  @param[in] retain_nonstatus_information Should Engine retain service
 *                                          non-status information ?
 *  @param[in] obsess_over_service          Should we obsess over
 *                                          service ?
 *
 *  @return New service.
 */
com::centreon::engine::service2* add_service(
           uint64_t host_id,
           uint64_t service_id,
           std::string const& host_name,
           std::string const& description,
           char const* display_name,
           char const* check_period,
           int initial_state,
           int max_attempts,
           int parallelize,
           int accept_passive_checks,
           double check_interval,
           double retry_interval,
           double notification_interval,
           double first_notification_delay,
           char const* notification_period,
           int notify_recovery,
           int notify_unknown,
           int notify_warning,
           int notify_critical,
           int notify_flapping,
           int notify_downtime,
           int notifications_enabled,
           int is_volatile,
           char const* event_handler,
           int event_handler_enabled,
           char const* check_command,
           int checks_enabled,
           int flap_detection_enabled,
           double low_flap_threshold,
           double high_flap_threshold,
           int flap_detection_on_ok,
           int flap_detection_on_warning,
           int flap_detection_on_unknown,
           int flap_detection_on_critical,
           int stalk_on_ok,
           int stalk_on_warning,
           int stalk_on_unknown,
           int stalk_on_critical,
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
           int retain_status_information,
           int retain_nonstatus_information,
           int obsess_over_service) {
  (void)failure_prediction_enabled;
  (void)failure_prediction_options;

  // Make sure we have everything we need.
  if (!service_id) {
    logger(log_config_error, basic)
      << "Error: Service comes from a database, therefore its service id "
      << "must not be null";
    return nullptr;
  }
  else if (description.empty()) {
    logger(log_config_error, basic)
      << "Error: Service description is not set";
    return nullptr;
  }
  else if (host_name.empty()) {
    logger(log_config_error, basic)
      << "Error: Host name of service '"
      << description << "' is not set";
    return nullptr;
  }
  else if (!check_command || !check_command[0]) {
    logger(log_config_error, basic)
      << "Error: Check command of service '" << description
      << "' on host '" << host_name << "' is not set";
    return nullptr;
  }

  host_id = get_host_id(host_name);
  if (!host_id) {
    logger(log_config_error, basic)
      << "Error: The service '" << description << "' cannot be created because"
      << " host '" << host_name << "' does not exist (host_id is null)";
    return nullptr;
  }

  // Check values.
  if ((max_attempts <= 0)
      || (check_interval < 0)
      || (retry_interval <= 0)
      || (notification_interval < 0)) {
    logger(log_config_error, basic)
      << "Error: Invalid max_attempts, check_interval, retry_interval"
         ", or notification_interval value for service '"
      << description << "' on host '" << host_name << "'";
    return nullptr;
  }
  if (first_notification_delay < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid first_notification_delay value for service '"
      << description << "' on host '" << host_name << "'";
    return nullptr;
  }

  // Check if the service is already exist.
  std::pair<uint64_t, uint64_t>
    id(std::make_pair(host_id, service_id));
  if (is_service_exist(id)) {
    logger(log_config_error, basic)
      << "Error: Service '" << description << "' on host '"
      << host_name << "' has already been defined";
    return nullptr;
  }

  // Allocate memory.
  std::shared_ptr<service2> obj{new service2};

  try {
    // Duplicate vars.
    obj->set_hostname(host_name);
    obj->set_description(description);
    obj->display_name = string::dup(display_name ? display_name : description);
    obj->service_check_command = string::dup(check_command);
    if (event_handler)
      obj->event_handler = string::dup(event_handler);
    if (notification_period)
      obj->notification_period = string::dup(notification_period);
    if (check_period)
      obj->check_period = string::dup(check_period);
    if (notes)
      obj->notes = string::dup(notes);
    if (notes_url)
      obj->notes_url = string::dup(notes_url);
    if (action_url)
      obj->action_url = string::dup(action_url);
    if (icon_image)
      obj->icon_image = string::dup(icon_image);
    if (icon_image_alt)
      obj->icon_image_alt = string::dup(icon_image_alt);

    obj->accept_passive_service_checks = (accept_passive_checks > 0);
    obj->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
    obj->check_freshness = (check_freshness > 0);
    obj->check_interval = check_interval;
    obj->check_options = CHECK_OPTION_NONE;
    obj->check_type = SERVICE_CHECK_ACTIVE;
    obj->checks_enabled = (checks_enabled > 0);
    obj->current_attempt = (initial_state == STATE_OK) ? 1 : max_attempts;
    obj->current_state = initial_state;
    obj->event_handler_enabled = (event_handler_enabled > 0);
    obj->first_notification_delay = first_notification_delay;
    obj->flap_detection_enabled = (flap_detection_enabled > 0);
    obj->flap_detection_on_critical = (flap_detection_on_critical > 0);
    obj->flap_detection_on_ok = (flap_detection_on_ok > 0);
    obj->flap_detection_on_unknown = (flap_detection_on_unknown > 0);
    obj->flap_detection_on_warning = (flap_detection_on_warning > 0);
    obj->freshness_threshold = freshness_threshold;
    obj->high_flap_threshold = high_flap_threshold;
    obj->initial_state = initial_state;
    obj->is_volatile = (is_volatile > 0);
    obj->last_hard_state = initial_state;
    obj->last_state = initial_state;
    obj->low_flap_threshold = low_flap_threshold;
    obj->max_attempts = max_attempts;
    obj->modified_attributes = MODATTR_NONE;
    obj->notification_interval = notification_interval;
    obj->notifications_enabled = (notifications_enabled > 0);
    obj->notify_on_critical = (notify_critical > 0);
    obj->notify_on_downtime = (notify_downtime > 0);
    obj->notify_on_flapping = (notify_flapping > 0);
    obj->notify_on_recovery = (notify_recovery > 0);
    obj->notify_on_unknown = (notify_unknown > 0);
    obj->notify_on_warning = (notify_warning > 0);
    obj->obsess_over_service = (obsess_over_service > 0);
    obj->parallelize = (parallelize > 0);
    obj->process_performance_data = (process_perfdata > 0);
    obj->retain_nonstatus_information = (retain_nonstatus_information > 0);
    obj->retain_status_information = (retain_status_information > 0);
    obj->retry_interval = retry_interval;
    obj->should_be_scheduled = true;
    obj->stalk_on_critical = (stalk_on_critical > 0);
    obj->stalk_on_ok = (stalk_on_ok > 0);
    obj->stalk_on_unknown = (stalk_on_unknown > 0);
    obj->stalk_on_warning = (stalk_on_warning > 0);
    obj->state_type = HARD_STATE;

    // STATE_OK = 0, so we don't need to set state_history (memset
    // is used before).
    // for (unsigned int x(0); x < MAX_STATE_HISTORY_ENTRIES; ++x)
    //   obj->state_history[x] = STATE_OK;

    // Add new items to the configuration state.
    state::instance().services()[id] = obj;

    // Add new items to the list.
    obj->next = service_list;
    service_list = obj.get();
  }
  catch (...) {
    obj.reset();
  }

  return obj.get();
}

/**
 *  Get number of registered services.
 *
 *  @return Number of registered services.
 */
int get_service_count() {
  return state::instance().services().size();
}

/**
 *  Tests whether a contact is a contact for a particular service.
 *
 *  @param[in] svc   Target service.
 *  @param[in] cntct Target contact.
 *
 *  @return true or false.
 */
int is_contact_for_service(com::centreon::engine::service2* svc, contact* cntct) {
  if (!svc || !cntct)
    return false;

  // Search all individual contacts of this service.
  for (contact_map::iterator
         it(svc->contacts.begin()),
         end(svc->contacts.end());
       it != end;
       ++it)
    if (it->second.get() == cntct)
      return (true);

  // Search all contactgroups of this service.
  for (contactgroup_map::iterator
         it(svc->contact_groups.begin()),
         end(svc->contact_groups.end());
       it != end;
       ++it)
    if (it->second->get_members().find(cntct->get_name()) ==
        it->second->get_members().end())
      return true;

  return false;
}

/**
 *  Tests whether or not a contact is an escalated contact for a
 *  particular service.
 *
 *  @param[in] svc   Target service.
 *  @param[in] cntct Target contact.
 *
 *  @return true or false.
 */
int is_escalated_contact_for_service(com::centreon::engine::service2* svc, contact* cntct) {
  if (!svc || !cntct)
    return false;

  std::pair<std::string, std::string>
    id(std::make_pair(svc->get_hostname(), svc->get_description()));
  umultimap<std::pair<std::string, std::string>,
            std::shared_ptr<serviceescalation> > const&
    escalations(state::instance().serviceescalations());

  for (umultimap<std::pair<std::string, std::string>,
                 std::shared_ptr<serviceescalation> >::const_iterator
         it(escalations.find(id)), end(escalations.end());
       it != end && it->first == id;
       ++it) {
    serviceescalation* svcescalation(&*it->second);
    // Search all contacts of this service escalation.
    for (contact_map::iterator
           it(svcescalation->contacts.begin()),
           end(svcescalation->contacts.end());
         it != end;
         ++it)
      if (it->second.get() == cntct)
        return (true);

    // Search all contactgroups of this service escalation.
    for (contactgroup_map::iterator
           it(svcescalation->contact_groups.begin()),
           end(svcescalation->contact_groups.end());
         it != end;
         ++it)
      if (it->second->get_members().find(cntct->get_name()) ==
          it->second->get_members().end())
        return true;
  }

  return false;
}

/**
 *  Check if acknowledgement on service expired.
 *
 *  @param[in] s  Target service.
 */
void engine::check_for_expired_acknowledgement(com::centreon::engine::service2* s) {
  if (s->problem_has_been_acknowledged) {
    int acknowledgement_timeout(
          service_other_props[std::make_pair(
                                     s->host_ptr->get_name().c_str(),
                                     s->get_description())].acknowledgement_timeout);
    if (acknowledgement_timeout > 0) {
      time_t last_ack(
               service_other_props[std::make_pair(
                                          s->host_ptr->get_name().c_str(),
                                          s->get_description())].last_acknowledgement);
      time_t now(time(nullptr));
      if (last_ack + acknowledgement_timeout >= now) {
        logger(log_info_message, basic)
          << "Acknowledgement of service '" << s->get_description()
          << "' on host '" << s->host_ptr->get_name() << "' just expired";
        s->problem_has_been_acknowledged = false;
        s->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
        update_service_status(s, false);
      }
    }
  }
  return ;
}

/**
 *  Get service by host name and service description.
 *
 *  @param[in] host_name           The host name.
 *  @param[in] service_description The service_description.
 *
 *  @return The struct service or throw exception if the
 *          service is not found.
 */
com::centreon::engine::service2& engine::find_service(
           uint64_t host_id,
           uint64_t service_id) {
  std::pair<uint64_t, uint64_t>
    id(std::make_pair(host_id, service_id));
  umap<std::pair<uint64_t, uint64_t>,
       std::shared_ptr<com::centreon::engine::service2> >::const_iterator
    it(state::instance().services().find(id));
  if (it == state::instance().services().end())
    throw (engine_error() << "Service '" << service_id
           << "' on host '" << host_id << "' was not found");
  return *it->second;
}

/**
 *  Get service timezone.
 *
 *  @param[in] hst  Host name.
 *  @param[in] svc  Service description.
 *
 *  @return Service timezone.
 */
char const* engine::get_service_timezone(
                      std::string const& hst,
                      std::string const& svc) {
  std::string const& timezone(service_other_props[{hst, svc}].timezone);
  return timezone.empty() ? nullptr : timezone.c_str();
}

/**
 *  Get if service exist.
 *
 *  @param[in] id The service id.
 *
 *  @return True if the service is found, otherwise false.
 */
bool engine::is_service_exist(
       std::pair<uint64_t, uint64_t> const& id) {
  umap<std::pair<uint64_t, uint64_t>,
       std::shared_ptr<com::centreon::engine::service2> >::const_iterator
    it(state::instance().services().find(id));
  return it != state::instance().services().end();
}

/**
 * Get the host and service IDs of a service.
 *
 *  @param[in] host  The host name.
 *  @param[in] svc   The service description.
 *
 *  @return  Pair of ID if found, pair of 0 otherwise.
 */
std::pair<uint64_t, uint64_t> engine::get_host_and_service_id(
                                                std::string const& host,
                                                std::string const& svc) {
  std::map<std::pair<std::string, std::string>, service_other_properties>::const_iterator
    found = service_other_props.find({host, std::string(svc)});
  return found != service_other_props.end()
          ? std::pair<uint64_t, uint64_t>{found->second.host_id, found->second.service_id}
          : std::pair<uint64_t, uint64_t>{0u, 0u};
}

/**
 *  Get a service' ID.
 *
 *  @param[in] host  The host name.
 *  @param[in] svc   The service description.
 *
 *  @return The service ID if found, 0 otherwise.
 */
uint64_t engine::get_service_id(std::string const& host, std::string const& svc) {
  return get_host_and_service_id(host, svc).second;
}

/**
 *  Schedule acknowledgement expiration check.
 *
 *  @param[in] s  Target service.
 */
void engine::schedule_acknowledgement_expiration(com::centreon::engine::service2* s) {
  std::pair<std::string, std::string>
    hs(std::make_pair(s->host_ptr->get_name(), s->get_description()));
  int ack_timeout(service_other_props[hs].acknowledgement_timeout);
  time_t last_ack(service_other_props[hs].last_acknowledgement);
  if ((ack_timeout > 0) && (last_ack != (time_t)0)) {
    schedule_new_event(
      EVENT_EXPIRE_SERVICE_ACK,
      false,
      last_ack + ack_timeout,
      false,
      0,
      nullptr,
      true,
      s,
      nullptr,
      0);
  }
  return ;
}

void service2::set_hostname(std::string const& name) {
  _hostname = name;
}

/**
 * @brief Get the hostname of the host associated with this downtime.
 *
 * @return A string reference to the host name.
 */
std::string const& service2::get_hostname() const {
  return _hostname;
}

void service2::set_description(std::string const& desc) {
  _description = desc;
}

/**
 * @brief Get the description of the service.
 *
 * @return A string reference to the description.
 */
std::string const& service2::get_description() const {
  return _description;
}

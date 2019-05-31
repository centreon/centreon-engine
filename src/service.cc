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

#include <iomanip>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/checks/viability_failure.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/hash_timed_event.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/macros/grab_host.hh"
#include "com/centreon/engine/macros/grab_service.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/objects/objectlist.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/sehandlers.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/timezone_locker.hh"
#include "compatibility/xpddefault.h"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::events;
using namespace com::centreon::engine::downtimes;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

std::array<std::pair<uint32_t, std::string>, 3> const
    service::tab_service_states{{{NSLOG_SERVICE_OK, "OK"},
                                 {NSLOG_SERVICE_WARNING, "WARNING"},
                                 {NSLOG_SERVICE_CRITICAL, "CRITICAL"}}};

service::service(std::string const& hostname,
                 std::string const& description,
                 std::string const& display_name,
                 std::string const& check_command,
                 int initial_state,
                 double check_interval,
                 double retry_interval,
                 int max_attempts,
                 std::string const& notification_period,
                 std::string const& check_period,
                 std::string const& event_handler,
                 std::string const& notes,
                 std::string const& notes_url,
                 std::string const& action_url,
                 std::string const& icon_image,
                 std::string const& icon_image_alt)
    : notifier{SERVICE_NOTIFICATION,
               display_name,
               check_command,
               initial_state,
               check_interval,
               retry_interval,
               max_attempts,
               notification_period,
               check_period,
               event_handler,
               notes,
               notes_url,
               action_url,
               icon_image,
               icon_image_alt},
      _hostname{hostname},
      _description{description} {}

service::~service() {
  this->contact_groups.clear();
  deleter::listmember(this->servicegroups_ptr, &deleter::objectlist);

  delete[] this->event_handler_args;
  this->event_handler_args = nullptr;
  delete[] this->check_command_args;
  this->check_command_args = nullptr;
}

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(com::centreon::engine::service const& obj1,
                com::centreon::engine::service const& obj2) throw() {
  return obj1.get_hostname() == obj2.get_hostname() &&
         obj1.get_description() == obj2.get_description() &&
         obj1.get_display_name() == obj2.get_display_name() &&
         obj1.get_check_command() == obj2.get_check_command() &&
         obj1.get_event_handler() == obj2.get_event_handler() &&
         obj1.get_initial_state() == obj2.get_initial_state() &&
         obj1.get_check_interval() == obj2.get_check_interval() &&
         obj1.get_retry_interval() == obj2.get_retry_interval() &&
         obj1.get_max_attempts() == obj2.get_max_attempts() &&
         obj1.parallelize == obj2.parallelize &&
         ((obj1.contact_groups.size() == obj2.contact_groups.size()) &&
          std::equal(obj1.contact_groups.begin(), obj1.contact_groups.end(),
                     obj2.contact_groups.begin())) &&
         ((obj1.contacts.size() == obj2.contacts.size()) &&
          std::equal(obj1.contacts.begin(), obj1.contacts.end(),
                     obj2.contacts.begin())) &&
         obj1.notification_interval == obj2.notification_interval &&
         obj1.first_notification_delay == obj2.first_notification_delay &&
         obj1.notify_on_unknown == obj2.notify_on_unknown &&
         obj1.notify_on_warning == obj2.notify_on_warning &&
         obj1.notify_on_critical == obj2.notify_on_critical &&
         obj1.notify_on_recovery == obj2.notify_on_recovery &&
         obj1.notify_on_flapping == obj2.notify_on_flapping &&
         obj1.notify_on_downtime == obj2.notify_on_downtime &&
         obj1.stalk_on_ok == obj2.stalk_on_ok &&
         obj1.stalk_on_warning == obj2.stalk_on_warning &&
         obj1.stalk_on_unknown == obj2.stalk_on_unknown &&
         obj1.stalk_on_critical == obj2.stalk_on_critical &&
         obj1.is_volatile == obj2.is_volatile &&
         obj1.get_notification_period() == obj2.get_notification_period() &&
         obj1.get_check_period() == obj2.get_check_period() &&
         obj1.flap_detection_enabled == obj2.flap_detection_enabled &&
         obj1.low_flap_threshold == obj2.low_flap_threshold &&
         obj1.high_flap_threshold == obj2.high_flap_threshold &&
         obj1.flap_detection_on_ok == obj2.flap_detection_on_ok &&
         obj1.flap_detection_on_warning == obj2.flap_detection_on_warning &&
         obj1.flap_detection_on_unknown == obj2.flap_detection_on_unknown &&
         obj1.flap_detection_on_critical == obj2.flap_detection_on_critical &&
         obj1.process_performance_data == obj2.process_performance_data &&
         obj1.check_freshness == obj2.check_freshness &&
         obj1.freshness_threshold == obj2.freshness_threshold &&
         obj1.accept_passive_service_checks ==
             obj2.accept_passive_service_checks &&
         obj1.event_handler_enabled == obj2.event_handler_enabled &&
         obj1.checks_enabled == obj2.checks_enabled &&
         obj1.retain_status_information == obj2.retain_status_information &&
         obj1.retain_nonstatus_information ==
             obj2.retain_nonstatus_information &&
         obj1.notifications_enabled == obj2.notifications_enabled &&
         obj1.obsess_over_service == obj2.obsess_over_service &&
         obj1.get_notes() == obj2.get_notes() &&
         obj1.get_notes_url() == obj2.get_notes_url() &&
         obj1.get_action_url() == obj2.get_action_url() &&
         obj1.get_icon_image() == obj2.get_icon_image() &&
         obj1.get_icon_image_alt() == obj2.get_icon_image_alt() &&
         obj1.custom_variables == obj2.custom_variables &&
         obj1.problem_has_been_acknowledged ==
             obj2.problem_has_been_acknowledged &&
         obj1.acknowledgement_type == obj2.acknowledgement_type &&
         obj1.host_problem_at_last_check == obj2.host_problem_at_last_check &&
         obj1.check_type == obj2.check_type &&
         obj1.current_state == obj2.current_state &&
         obj1.last_state == obj2.last_state &&
         obj1.last_hard_state == obj2.last_hard_state &&
         obj1.get_plugin_output() == obj2.get_plugin_output() &&
         obj1.get_long_plugin_output() == obj2.get_long_plugin_output() &&
         obj1.get_perf_data() == obj2.get_perf_data() &&
         obj1.state_type == obj2.state_type &&
         obj1.next_check == obj2.next_check &&
         obj1.should_be_scheduled == obj2.should_be_scheduled &&
         obj1.last_check == obj2.last_check &&
         obj1.current_attempt == obj2.current_attempt &&
         obj1.current_event_id == obj2.current_event_id &&
         obj1.last_event_id == obj2.last_event_id &&
         obj1.current_problem_id == obj2.current_problem_id &&
         obj1.last_problem_id == obj2.last_problem_id &&
         obj1.get_last_notification() == obj2.get_last_notification() &&
         obj1.get_next_notification() == obj2.get_next_notification() &&
         obj1.no_more_notifications == obj2.no_more_notifications &&
         obj1.check_flapping_recovery_notification ==
             obj2.check_flapping_recovery_notification &&
         obj1.last_state_change == obj2.last_state_change &&
         obj1.last_hard_state_change == obj2.last_hard_state_change &&
         obj1.last_time_ok == obj2.last_time_ok &&
         obj1.last_time_warning == obj2.last_time_warning &&
         obj1.last_time_unknown == obj2.last_time_unknown &&
         obj1.last_time_critical == obj2.last_time_critical &&
         obj1.has_been_checked == obj2.has_been_checked &&
         obj1.is_being_freshened == obj2.is_being_freshened &&
         obj1.notified_on_unknown == obj2.notified_on_unknown &&
         obj1.notified_on_warning == obj2.notified_on_warning &&
         obj1.notified_on_critical == obj2.notified_on_critical &&
         obj1.current_notification_number == obj2.current_notification_number &&
         obj1.current_notification_id == obj2.current_notification_id &&
         obj1.latency == obj2.latency &&
         obj1.execution_time == obj2.execution_time &&
         obj1.is_executing == obj2.is_executing &&
         obj1.check_options == obj2.check_options &&
         obj1.scheduled_downtime_depth == obj2.scheduled_downtime_depth &&
         obj1.pending_flex_downtime == obj2.pending_flex_downtime &&
         is_equal(obj1.state_history, obj2.state_history,
                  MAX_STATE_HISTORY_ENTRIES) &&
         obj1.state_history_index == obj2.state_history_index &&
         obj1.is_flapping == obj2.is_flapping &&
         obj1.flapping_comment_id == obj2.flapping_comment_id &&
         obj1.percent_state_change == obj2.percent_state_change &&
         obj1.modified_attributes == obj2.modified_attributes &&
         is_equal(obj1.event_handler_args, obj2.event_handler_args) &&
         is_equal(obj1.check_command_args, obj2.check_command_args);
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool operator!=(com::centreon::engine::service const& obj1,
                com::centreon::engine::service const& obj2) throw() {
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
std::ostream& operator<<(std::ostream& os,
                         com::centreon::engine::service const& obj) {
  std::string evt_str;
  if (obj.event_handler_ptr)
    evt_str = obj.event_handler_ptr->get_name();
  std::string cmd_str;
  if (obj.check_command_ptr)
    cmd_str = obj.check_command_ptr->get_name();
  std::string chk_period_str;
  if (obj.check_period_ptr)
    chk_period_str = obj.check_period_ptr->get_name();
  std::string notif_period_str;
  if (obj.notification_period_ptr)
    notif_period_str = obj.notification_period_ptr->get_name();
  char const* svcgrp_str(nullptr);
  if (obj.servicegroups_ptr)
    svcgrp_str = chkstr(
        static_cast<servicegroup const*>(obj.servicegroups_ptr->object_ptr)
            ->group_name);

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
        "  host_name:                            "
     << obj.get_hostname()
     << "\n"
        "  description:                          "
     << obj.get_description()
     << "\n"
        "  display_name:                         "
     << obj.get_display_name()
     << "\n"
        "  service_check_command:                "
     << obj.get_check_command()
     << "\n"
        "  event_handler:                        "
     << obj.get_event_handler()
     << "\n"
        "  initial_state:                        "
     << obj.get_initial_state()
     << "\n"
        "  check_interval:                       "
     << obj.get_check_interval()
     << "\n"
        "  retry_interval:                       "
     << obj.get_retry_interval()
     << "\n"
        "  max_attempts:                         "
     << obj.get_max_attempts()
     << "\n"
        "  parallelize:                          "
     << obj.parallelize
     << "\n"
        "  contact_groups:                       "
     << cg_oss
     << "\n"
        "  contacts:                             "
     << c_oss
     << "\n"
        "  notification_interval:                "
     << obj.notification_interval
     << "\n"
        "  first_notification_delay:             "
     << obj.first_notification_delay
     << "\n"
        "  notify_on_unknown:                    "
     << obj.notify_on_unknown
     << "\n"
        "  notify_on_warning:                    "
     << obj.notify_on_warning
     << "\n"
        "  notify_on_critical:                   "
     << obj.notify_on_critical
     << "\n"
        "  notify_on_recovery:                   "
     << obj.notify_on_recovery
     << "\n"
        "  notify_on_flapping:                   "
     << obj.notify_on_flapping
     << "\n"
        "  notify_on_downtime:                   "
     << obj.notify_on_downtime
     << "\n"
        "  stalk_on_ok:                          "
     << obj.stalk_on_ok
     << "\n"
        "  stalk_on_warning:                     "
     << obj.stalk_on_warning
     << "\n"
        "  stalk_on_unknown:                     "
     << obj.stalk_on_unknown
     << "\n"
        "  stalk_on_critical:                    "
     << obj.stalk_on_critical
     << "\n"
        "  is_volatile:                          "
     << obj.is_volatile
     << "\n"
        "  notification_period:                  "
     << obj.get_notification_period()
     << "\n"
        "  check_period:                         "
     << obj.get_check_period()
     << "\n"
        "  flap_detection_enabled:               "
     << obj.flap_detection_enabled
     << "\n"
        "  low_flap_threshold:                   "
     << obj.low_flap_threshold
     << "\n"
        "  high_flap_threshold:                  "
     << obj.high_flap_threshold
     << "\n"
        "  flap_detection_on_ok:                 "
     << obj.flap_detection_on_ok
     << "\n"
        "  flap_detection_on_warning:            "
     << obj.flap_detection_on_warning
     << "\n"
        "  flap_detection_on_unknown:            "
     << obj.flap_detection_on_unknown
     << "\n"
        "  flap_detection_on_critical:           "
     << obj.flap_detection_on_critical
     << "\n"
        "  process_performance_data:             "
     << obj.process_performance_data
     << "\n"
        "  check_freshness:                      "
     << obj.check_freshness
     << "\n"
        "  freshness_threshold:                  "
     << obj.freshness_threshold
     << "\n"
        "  accept_passive_service_checks:        "
     << obj.accept_passive_service_checks
     << "\n"
        "  event_handler_enabled:                "
     << obj.event_handler_enabled
     << "\n"
        "  checks_enabled:                       "
     << obj.checks_enabled
     << "\n"
        "  retain_status_information:            "
     << obj.retain_status_information
     << "\n"
        "  retain_nonstatus_information:         "
     << obj.retain_nonstatus_information
     << "\n"
        "  notifications_enabled:                "
     << obj.notifications_enabled
     << "\n"
        "  obsess_over_service:                  "
     << obj.obsess_over_service
     << "\n"
        "  notes:                                "
     << obj.get_notes()
     << "\n"
        "  notes_url:                            "
     << obj.get_notes_url()
     << "\n"
        "  action_url:                           "
     << obj.get_action_url()
     << "\n"
        "  icon_image:                           "
     << obj.get_icon_image()
     << "\n"
        "  icon_image_alt:                       "
     << obj.get_icon_image_alt()
     << "\n"
        "  problem_has_been_acknowledged:        "
     << obj.problem_has_been_acknowledged
     << "\n"
        "  acknowledgement_type:                 "
     << obj.acknowledgement_type
     << "\n"
        "  host_problem_at_last_check:           "
     << obj.host_problem_at_last_check
     << "\n"
        "  check_type:                           "
     << obj.check_type
     << "\n"
        "  current_state:                        "
     << obj.current_state
     << "\n"
        "  last_state:                           "
     << obj.last_state
     << "\n"
        "  last_hard_state:                      "
     << obj.last_hard_state
     << "\n"
        "  plugin_output:                        "
     << obj.get_plugin_output()
     << "\n"
        "  long_plugin_output:                   "
     << obj.get_long_plugin_output()
     << "\n"
        "  perf_data:                            "
     << obj.get_perf_data()
     << "\n"
        "  state_type:                           "
     << obj.state_type
     << "\n"
        "  next_check:                           "
     << string::ctime(obj.next_check)
     << "\n"
        "  should_be_scheduled:                  "
     << obj.should_be_scheduled
     << "\n"
        "  last_check:                           "
     << string::ctime(obj.last_check)
     << "\n"
        "  current_attempt:                      "
     << obj.current_attempt
     << "\n"
        "  current_event_id:                     "
     << obj.current_event_id
     << "\n"
        "  last_event_id:                        "
     << obj.last_event_id
     << "\n"
        "  current_problem_id:                   "
     << obj.current_problem_id
     << "\n"
        "  last_problem_id:                      "
     << obj.last_problem_id
     << "\n"
        "  last_notification:                    "
     << string::ctime(obj.get_last_notification())
     << "\n"
        "  next_notification:                    "
     << string::ctime(obj.get_next_notification())
     << "\n"
        "  no_more_notifications:                "
     << obj.no_more_notifications
     << "\n"
        "  check_flapping_recovery_notification: "
     << obj.check_flapping_recovery_notification
     << "\n"
        "  last_state_change:                    "
     << string::ctime(obj.last_state_change)
     << "\n"
        "  last_hard_state_change:               "
     << string::ctime(obj.last_hard_state_change)
     << "\n"
        "  last_time_ok:                         "
     << string::ctime(obj.last_time_ok)
     << "\n"
        "  last_time_warning:                    "
     << string::ctime(obj.last_time_warning)
     << "\n"
        "  last_time_unknown:                    "
     << string::ctime(obj.last_time_unknown)
     << "\n"
        "  last_time_critical:                   "
     << string::ctime(obj.last_time_critical)
     << "\n"
        "  has_been_checked:                     "
     << obj.has_been_checked
     << "\n"
        "  is_being_freshened:                   "
     << obj.is_being_freshened
     << "\n"
        "  notified_on_unknown:                  "
     << obj.notified_on_unknown
     << "\n"
        "  notified_on_warning:                  "
     << obj.notified_on_warning
     << "\n"
        "  notified_on_critical:                 "
     << obj.notified_on_critical
     << "\n"
        "  current_notification_number:          "
     << obj.current_notification_number
     << "\n"
        "  current_notification_id:              "
     << obj.current_notification_id
     << "\n"
        "  latency:                              "
     << obj.latency
     << "\n"
        "  execution_time:                       "
     << obj.execution_time
     << "\n"
        "  is_executing:                         "
     << obj.is_executing
     << "\n"
        "  check_options:                        "
     << obj.check_options
     << "\n"
        "  scheduled_downtime_depth:             "
     << obj.scheduled_downtime_depth
     << "\n"
        "  pending_flex_downtime:                "
     << obj.pending_flex_downtime << "\n";

  os << "  state_history:                        ";
  for (unsigned int i(0),
       end(sizeof(obj.state_history) / sizeof(obj.state_history[0]));
       i < end; ++i)
    os << obj.state_history[i] << (i + 1 < end ? ", " : "\n");

  os << "  state_history_index:                  " << obj.state_history_index
     << "\n"
        "  is_flapping:                          "
     << obj.is_flapping
     << "\n"
        "  flapping_comment_id:                  "
     << obj.flapping_comment_id
     << "\n"
        "  percent_state_change:                 "
     << obj.percent_state_change
     << "\n"
        "  modified_attributes:                  "
     << obj.modified_attributes
     << "\n"
        "  host_ptr:                             "
     << (obj.host_ptr ? obj.host_ptr->get_name() : "\"nullptr\"")
     << "\n"
        "  event_handler_ptr:                    "
     << evt_str
     << "\n"
        "  event_handler_args:                   "
     << chkstr(obj.event_handler_args)
     << "\n"
        "  check_command_ptr:                    "
     << cmd_str
     << "\n"
        "  check_command_args:                   "
     << chkstr(obj.check_command_args)
     << "\n"
        "  check_period_ptr:                     "
     << chk_period_str
     << "\n"
        "  notification_period_ptr:              "
     << notif_period_str
     << "\n"
        "  servicegroups_ptr:                    "
     << chkstr(svcgrp_str) << "\n";

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
com::centreon::engine::service* add_service(
    uint64_t host_id,
    uint64_t service_id,
    std::string const& host_name,
    std::string const& description,
    std::string const& display_name,
    std::string const& check_period,
    int initial_state,
    int max_attempts,
    bool parallelize,
    int accept_passive_checks,
    double check_interval,
    double retry_interval,
    double notification_interval,
    double first_notification_delay,
    std::string const& notification_period,
    bool notify_recovery,
    bool notify_unknown,
    bool notify_warning,
    bool notify_critical,
    bool notify_flapping,
    bool notify_downtime,
    bool notifications_enabled,
    bool is_volatile,
    std::string const& event_handler,
    bool event_handler_enabled,
    std::string const& check_command,
    bool checks_enabled,
    bool flap_detection_enabled,
    double low_flap_threshold,
    double high_flap_threshold,
    bool flap_detection_on_ok,
    bool flap_detection_on_warning,
    bool flap_detection_on_unknown,
    bool flap_detection_on_critical,
    bool stalk_on_ok,
    bool stalk_on_warning,
    bool stalk_on_unknown,
    bool stalk_on_critical,
    int process_perfdata,
    int check_freshness,
    int freshness_threshold,
    std::string const& notes,
    std::string const& notes_url,
    std::string const& action_url,
    std::string const& icon_image,
    std::string const& icon_image_alt,
    int retain_status_information,
    int retain_nonstatus_information,
    int obsess_over_service) {

  // Make sure we have everything we need.
  if (!service_id) {
    logger(log_config_error, basic)
        << "Error: Service comes from a database, therefore its service id "
        << "must not be null";
    return nullptr;
  } else if (description.empty()) {
    logger(log_config_error, basic) << "Error: Service description is not set";
    return nullptr;
  } else if (host_name.empty()) {
    logger(log_config_error, basic)
        << "Error: Host name of service '" << description << "' is not set";
    return nullptr;
  } else if (check_command.empty()) {
    logger(log_config_error, basic)
        << "Error: Check command of service '" << description << "' on host '"
        << host_name << "' is not set";
    return nullptr;
  }

  host_id = get_host_id(host_name);
  if (!host_id) {
    logger(log_config_error, basic)
        << "Error: The service '" << description
        << "' cannot be created because"
        << " host '" << host_name << "' does not exist (host_id is null)";
    return nullptr;
  }

  // Check values.
  if ((max_attempts <= 0) || (check_interval < 0) || (retry_interval <= 0) ||
      (notification_interval < 0)) {
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
  std::pair<uint64_t, uint64_t> id(std::make_pair(host_id, service_id));
  if (is_service_exist(id)) {
    logger(log_config_error, basic)
        << "Error: Service '" << description << "' on host '" << host_name
        << "' has already been defined";
    return nullptr;
  }

  // Allocate memory.
  std::shared_ptr<service> obj{new service(
      host_name, description, display_name.empty() ? description : display_name,
      check_command, initial_state, check_interval, retry_interval,
      max_attempts, notification_period, check_period, event_handler,
      notes, notes_url, action_url, icon_image, icon_image_alt)};

  try {
    obj->accept_passive_service_checks = (accept_passive_checks > 0);
    obj->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
    obj->check_freshness = (check_freshness > 0);
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
    obj->is_volatile = (is_volatile > 0);
    obj->last_hard_state = initial_state;
    obj->last_state = initial_state;
    obj->low_flap_threshold = low_flap_threshold;
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
  } catch (...) {
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
int is_contact_for_service(com::centreon::engine::service* svc,
                           contact* cntct) {
  if (!svc || !cntct)
    return false;

  // Search all individual contacts of this service.
  for (contact_map::iterator it(svc->contacts.begin()),
       end(svc->contacts.end());
       it != end; ++it)
    if (it->second.get() == cntct)
      return true;

  // Search all contactgroups of this service.
  for (contactgroup_map::iterator it(svc->contact_groups.begin()),
       end(svc->contact_groups.end());
       it != end; ++it)
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
int is_escalated_contact_for_service(com::centreon::engine::service* svc,
                                     contact* cntct) {
  if (!svc || !cntct)
    return false;

  std::pair<std::string, std::string> id(
      std::make_pair(svc->get_hostname(), svc->get_description()));
  umultimap<std::pair<std::string, std::string>,
            std::shared_ptr<serviceescalation> > const&
      escalations(state::instance().serviceescalations());

  for (umultimap<std::pair<std::string, std::string>,
                 std::shared_ptr<serviceescalation> >::const_iterator
           it(escalations.find(id)),
       end(escalations.end());
       it != end && it->first == id; ++it) {
    serviceescalation* svcescalation(&*it->second);
    // Search all contacts of this service escalation.
    for (contact_map::iterator it(svcescalation->contacts.begin()),
         end(svcescalation->contacts.end());
         it != end; ++it)
      if (it->second.get() == cntct)
        return true;

    // Search all contactgroups of this service escalation.
    for (contactgroup_map::iterator it(svcescalation->contact_groups.begin()),
         end(svcescalation->contact_groups.end());
         it != end; ++it)
      if (it->second->get_members().find(cntct->get_name()) ==
          it->second->get_members().end())
        return true;
  }

  return false;
}

/**
 *  Check if acknowledgement on service expired.
 *
 */
void service::check_for_expired_acknowledgement() {
  if (this->problem_has_been_acknowledged) {
    if (_acknowledgement_timeout > 0) {
      time_t now(time(nullptr));
      if (_last_acknowledgement + _acknowledgement_timeout >= now) {
        logger(log_info_message, basic)
            << "Acknowledgement of service '" << get_description()
            << "' on host '" << this->host_ptr->get_name() << "' just expired";
        this->problem_has_been_acknowledged = false;
        this->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
        update_status(false);
      }
    }
  }
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
com::centreon::engine::service& engine::find_service(uint64_t host_id,
                                                     uint64_t service_id) {
  std::pair<uint64_t, uint64_t> id(std::make_pair(host_id, service_id));
  umap<std::pair<uint64_t, uint64_t>,
       std::shared_ptr<com::centreon::engine::service> >::const_iterator
      it(state::instance().services().find(id));
  if (it == state::instance().services().end())
    throw(engine_error() << "Service '" << service_id << "' on host '"
                         << host_id << "' was not found");
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
char const* engine::get_service_timezone(std::string const& hst,
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
bool engine::is_service_exist(std::pair<uint64_t, uint64_t> const& id) {
  umap<std::pair<uint64_t, uint64_t>,
       std::shared_ptr<com::centreon::engine::service> >::const_iterator
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
  std::map<std::pair<std::string, std::string>,
           service_other_properties>::const_iterator found =
      service_other_props.find({host, std::string(svc)});
  return found != service_other_props.end()
             ? std::pair<uint64_t, uint64_t>{found->second.host_id,
                                             found->second.service_id}
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
uint64_t engine::get_service_id(std::string const& host,
                                std::string const& svc) {
  return get_host_and_service_id(host, svc).second;
}

/**
 *  Schedule acknowledgement expiration check.
 *
 */
void service::schedule_acknowledgement_expiration() {
  if (_acknowledgement_timeout > 0 && _last_acknowledgement != (time_t)0)
    schedule_new_event(EVENT_EXPIRE_SERVICE_ACK, false,
                       _last_acknowledgement + _acknowledgement_timeout, false,
                       0, nullptr, true, this, nullptr, 0);
}

void service::set_hostname(std::string const& name) {
  _hostname = name;
}

/**
 * @brief Get the hostname of the host associated with this downtime.
 *
 * @return A string reference to the host name.
 */
std::string const& service::get_hostname() const {
  return _hostname;
}

void service::set_description(std::string const& desc) {
  _description = desc;
}

/**
 * @brief Get the description of the service.
 *
 * @return A string reference to the description.
 */
std::string const& service::get_description() const {
  return _description;
}

int service::handle_async_check_result(check_result* queued_check_result) {
  com::centreon::engine::host* temp_host = nullptr;
  time_t next_service_check = 0L;
  time_t preferred_time = 0L;
  time_t next_valid_time = 0L;
  int reschedule_check = false;
  int state_change = false;
  int hard_state_change = false;
  int first_host_check_initiated = false;
  int route_result = HOST_UP;
  time_t current_time = 0L;
  int state_was_logged = false;
  std::string old_plugin_output;
  char* temp_ptr = nullptr;
  objectlist* check_servicelist = nullptr;
  objectlist* servicelist_item = nullptr;
  com::centreon::engine::service* master_service = nullptr;
  int run_async_check = true;
  /* TODO - 09/23/07 move this to a global variable */
  int state_changes_use_cached_state = true;
  int flapping_check_done = false;

  logger(dbg_functions, basic) << "handle_async_service_check_result()";

  /* make sure we have what we need */
  if (queued_check_result == nullptr)
    return ERROR;

  /* get the current time */
  time(&current_time);

  logger(dbg_checks, basic)
      << "** Handling check result for service '" << this->get_description()
      << "' on host '" << this->get_hostname() << "'...";
  logger(dbg_checks, more)
      << "HOST: " << this->get_hostname()
      << ", SERVICE: " << this->get_description() << ", CHECK TYPE: "
      << (queued_check_result->check_type == SERVICE_CHECK_ACTIVE ? "Active"
                                                                  : "Passive")
      << ", OPTIONS: " << queued_check_result->check_options << ", SCHEDULED: "
      << (queued_check_result->scheduled_check ? "Yes" : "No")
      << ", RESCHEDULE: "
      << (queued_check_result->reschedule_check ? "Yes" : "No")
      << ", EXITED OK: " << (queued_check_result->exited_ok ? "Yes" : "No")
      << ", return CODE: " << queued_check_result->return_code
      << ", OUTPUT: " << queued_check_result->output;

  /* decrement the number of service checks still out there... */
  if (queued_check_result->check_type == SERVICE_CHECK_ACTIVE &&
      currently_running_service_checks > 0)
    currently_running_service_checks--;

  /*
   * skip this service check results if its passive and we aren't accepting
   * passive check results
   */
  if (queued_check_result->check_type == SERVICE_CHECK_PASSIVE) {
    if (!config->accept_passive_service_checks()) {
      logger(dbg_checks, basic)
          << "Discarding passive service check result because passive "
             "service checks are disabled globally.";
      return ERROR;
    }
    if (!this->accept_passive_service_checks) {
      logger(dbg_checks, basic)
          << "Discarding passive service check result because passive "
             "checks are disabled for this service.";
      return ERROR;
    }
  }

  /*
   * clear the freshening flag (it would have been set if this service was
   * determined to be stale)
   */
  if (queued_check_result->check_options & CHECK_OPTION_FRESHNESS_CHECK)
    this->is_being_freshened = false;

  /* clear the execution flag if this was an active check */
  if (queued_check_result->check_type == SERVICE_CHECK_ACTIVE)
    this->is_executing = false;

  /* DISCARD INVALID FRESHNESS CHECK RESULTS */
  /* If a services goes stale, Engine will initiate a forced check in
  ** order to freshen it.  There is a race condition whereby a passive
  ** check could arrive between the 1) initiation of the forced check
  ** and 2) the time when the forced check result is processed here.
  ** This would make the service fresh again, so we do a quick check to
  ** make sure the service is still stale before we accept the check
  ** result.
  */
  if ((queued_check_result->check_options & CHECK_OPTION_FRESHNESS_CHECK) &&
      is_service_result_fresh(this, current_time, false)) {
    logger(dbg_checks, basic)
        << "Discarding service freshness check result because the service "
           "is currently fresh (race condition avoided).";
    return OK;
  }

  /* check latency is passed to us */
  this->latency = queued_check_result->latency;

  /* update the execution time for this check (millisecond resolution) */
  this->execution_time =
      (double)((double)(queued_check_result->finish_time.tv_sec -
                        queued_check_result->start_time.tv_sec) +
               (double)((queued_check_result->finish_time.tv_usec -
                         queued_check_result->start_time.tv_usec) /
                        1000.0) /
                   1000.0);
  if (this->execution_time < 0.0)
    this->execution_time = 0.0;

  /* get the last check time */
  this->last_check = queued_check_result->start_time.tv_sec;

  /* was this check passive or active? */
  this->check_type = (queued_check_result->check_type == SERVICE_CHECK_ACTIVE)
                         ? SERVICE_CHECK_ACTIVE
                         : SERVICE_CHECK_PASSIVE;

  /* update check statistics for passive checks */
  if (queued_check_result->check_type == SERVICE_CHECK_PASSIVE)
    update_check_stats(PASSIVE_SERVICE_CHECK_STATS,
                       queued_check_result->start_time.tv_sec);

  /*
   * should we reschedule the next service check? NOTE: This may be overridden
   * later...
   */
  reschedule_check = queued_check_result->reschedule_check;

  /* save the old service status info */
  this->last_state = this->current_state;

  /* save old plugin output */
  old_plugin_output = get_plugin_output();

  /*
   * if there was some error running the command, just skip it (this
   * shouldn't be happening)
   */
  if (!queued_check_result->exited_ok) {
    logger(log_runtime_warning, basic)
        << "Warning:  Check of service '" << this->get_description()
        << "' on host '" << this->get_hostname() << "' did not exit properly!";

    set_plugin_output("(Service check did not exit properly)");
    this->current_state = STATE_UNKNOWN;
  }

  /* make sure the return code is within bounds */
  else if (queued_check_result->return_code < 0 ||
           queued_check_result->return_code > 3) {
    logger(log_runtime_warning, basic)
        << "Warning: return (code of " << queued_check_result->return_code
        << " for check of service '" << this->get_description() << "' on host '"
        << this->get_hostname() << "' was out of bounds."
        << (queued_check_result->return_code == 126
                ? "Make sure the plugin you're trying to run is executable."
                : (queued_check_result->return_code == 127
                       ? " Make sure the plugin you're trying to run actually "
                         "exists."
                       : ""));

    std::ostringstream oss;
    oss << "(Return code of " << queued_check_result->return_code
        << " is out of bounds"
        << (queued_check_result->return_code == 126
                ? " - plugin may not be executable"
                : (queued_check_result->return_code == 127
                       ? " - plugin may be missing"
                       : ""))
        << ')';

    set_plugin_output(oss.str());
    this->current_state = STATE_UNKNOWN;
  }

  /* else the return code is okay... */
  else {
    /*
     * parse check output to get: (1) short output, (2) long output,
     * (3) perf data
     */
    std::string output{queued_check_result->output};
    std::string plugin_output;
    std::string long_plugin_output;
    std::string perf_data;
    parse_check_output(output, plugin_output,
                       long_plugin_output, perf_data, true, true);

    set_long_plugin_output(long_plugin_output);
    set_perf_data(perf_data);
    /* make sure the plugin output isn't null */
    if (plugin_output.empty())
      set_plugin_output("(No output returned from plugin)");
    else {
      std::replace(plugin_output.begin(), plugin_output.end(), ';', ':');

      /*
       * replace semicolons in plugin output (but not performance data) with
       * colons
       */
      set_plugin_output(plugin_output);
    }

    logger(dbg_checks, most)
        << "Parsing check output...\n"
        << "Short Output:\n"
        << (get_plugin_output().empty() ? "NULL" : this->get_plugin_output()) << "\n"
        << "Long Output:\n"
        << (get_long_plugin_output().empty() ? "NULL" : get_long_plugin_output())
        << "\n"
        << "Perf Data:\n"
        << (get_perf_data().empty() ? "NULL" : get_perf_data());

    /* grab the return code */
    this->current_state = queued_check_result->return_code;
  }

  /* record the last state time */
  switch (this->current_state) {
    case STATE_OK:
      this->last_time_ok = this->last_check;
      break;

    case STATE_WARNING:
      this->last_time_warning = this->last_check;
      break;

    case STATE_UNKNOWN:
      this->last_time_unknown = this->last_check;
      break;

    case STATE_CRITICAL:
      this->last_time_critical = this->last_check;
      break;

    default:
      break;
  }

  /*
   * log passive checks - we need to do this here, as some my bypass external
   * commands by getting dropped in checkresults dir
   */
  if (this->check_type == SERVICE_CHECK_PASSIVE) {
    if (config->log_passive_checks())
      logger(log_passive_check, basic)
          << "PASSIVE SERVICE CHECK: " << this->get_hostname() << ";"
          << this->get_description() << ";" << this->current_state << ";"
          << get_plugin_output();
  }

  /* get the host that this service runs on */
  temp_host = (host*)this->host_ptr;

  /* if the service check was okay... */
  if (this->current_state == STATE_OK) {
    /* if the host has never been checked before, verify its status
     * only do this if 1) the initial state was set to non-UP or 2) the host
     * is not scheduled to be checked soon (next 5 minutes)
     */
    if (!temp_host->get_has_been_checked() &&
        (temp_host->get_initial_state() != HOST_UP ||
         (unsigned long)temp_host->get_next_check() == 0L ||
         (unsigned long)(temp_host->get_next_check() - current_time) > 300)) {
      /* set a flag to remember that we launched a check */
      first_host_check_initiated = true;

      /* 08/04/07 EG launch an async (parallel) host check unless
       * aggressive host checking is enabled
       * previous logic was to simply run a sync (serial) host check
       * do NOT allow cached check results to happen here - we need the host
       * to be checked for real...
       * */
      if (config->use_aggressive_host_checking())
        perform_on_demand_host_check(temp_host, nullptr, CHECK_OPTION_NONE, false,
                                     0L);
      else
        temp_host->run_async_check(CHECK_OPTION_NONE, 0.0, false, false, nullptr,
                                   nullptr);
    }
  }

  /*
   **** NOTE - THIS WAS MOVED UP FROM LINE 1049 BELOW TO FIX PROBLEMS ****
   **** WHERE CURRENT ATTEMPT VALUE WAS ACTUALLY "LEADING" REAL VALUE ****
   * increment the current attempt number if this is a soft state
   * (service was rechecked)
   */
  if (this->state_type == SOFT_STATE &&
      (this->current_attempt < get_max_attempts()))
    this->current_attempt = this->current_attempt + 1;

  logger(dbg_checks, most) << "ST: "
                           << (this->state_type == SOFT_STATE ? "SOFT" : "HARD")
                           << "  CA: " << this->current_attempt
                           << "  MA: " << get_max_attempts()
                           << "  CS: " << this->current_state
                           << "  LS: " << this->last_state
                           << "  LHS: " << this->last_hard_state;

  /* check for a state change (either soft or hard) */
  if (this->current_state != this->last_state) {
    logger(dbg_checks, most) << "Service has changed state since last check!";
    state_change = true;
  }

  /*
   * checks for a hard state change where host was down at last service
   * check this occurs in the case where host goes down and service current
   * attempt gets reset to 1 if this check is not made, the service recovery
   * looks like a soft recovery instead of a hard one
   */
  if (this->host_problem_at_last_check && this->current_state == STATE_OK) {
    logger(dbg_checks, most) << "Service had a HARD STATE CHANGE!!";
    hard_state_change = true;
  }

  /*
   * check for a "normal" hard state change where max check attempts is
   * reached
   */
  if (this->current_attempt >= get_max_attempts() &&
      this->current_state != this->last_hard_state) {
    logger(dbg_checks, most) << "Service had a HARD STATE CHANGE!!";
    hard_state_change = true;
  }

  /* a state change occurred...
   * reset last and next notification times and acknowledgement flag if
   * necessary, misc other stuff
   */
  if (state_change || hard_state_change) {
    /* reschedule the service check */
    reschedule_check = true;

    /* reset notification times */
    set_last_notification(static_cast<time_t>(0));
    set_next_notification(static_cast<time_t>(0));

    /* reset notification suppression option */
    this->no_more_notifications = false;

    if (ACKNOWLEDGEMENT_NORMAL == this->acknowledgement_type &&
        (state_change || !hard_state_change)) {
      this->problem_has_been_acknowledged = false;
      this->acknowledgement_type = ACKNOWLEDGEMENT_NONE;

      /* remove any non-persistant comments associated with the ack */
      comment::delete_service_acknowledgement_comments(this);
    } else if (this->acknowledgement_type == ACKNOWLEDGEMENT_STICKY &&
               this->current_state == STATE_OK) {
      this->problem_has_been_acknowledged = false;
      this->acknowledgement_type = ACKNOWLEDGEMENT_NONE;

      /* remove any non-persistant comments associated with the ack */
      comment::delete_service_acknowledgement_comments(this);
    }

    /*
     * do NOT reset current notification number!!!
     * hard changes between non-OK states should continue to be escalated,
     * so don't reset current notification number
     */
    /*this->current_notification_number=0; */
  }

  /* initialize the last host and service state change times if necessary */
  if (this->last_state_change == (time_t)0)
    this->last_state_change = this->last_check;
  if (this->last_hard_state_change == (time_t)0)
    this->last_hard_state_change = this->last_check;
  if (temp_host->get_last_state_change() == (time_t)0)
    temp_host->set_last_state_change(this->last_check);
  if (temp_host->get_last_hard_state_change() == (time_t)0)
    temp_host->set_last_hard_state_change(this->last_check);

  /* update last service state change times */
  if (state_change)
    this->last_state_change = this->last_check;
  if (hard_state_change)
    this->last_hard_state_change = this->last_check;

  /* update the event and problem ids */
  if (state_change) {
    /* always update the event id on a state change */
    this->last_event_id = this->current_event_id;
    this->current_event_id = next_event_id;
    next_event_id++;

    /* update the problem id when transitioning to a problem state */
    if (this->last_state == STATE_OK) {
      /* don't reset last problem id, or it will be zero the next time a problem
       * is encountered */
      /* this->last_problem_id=this->current_problem_id; */
      this->current_problem_id = next_problem_id;
      next_problem_id++;
    }

    /* clear the problem id when transitioning from a problem state to an OK
     * state */
    if (this->current_state == STATE_OK) {
      this->last_problem_id = this->current_problem_id;
      this->current_problem_id = 0L;
    }
  }

  /**************************************/
  /******* SERVICE CHECK OK LOGIC *******/
  /**************************************/

  /* if the service is up and running OK... */
  if (this->current_state == STATE_OK) {
    logger(dbg_checks, more) << "Service is OK.";

    /* reset the acknowledgement flag (this should already have been done, but
     * just in case...) */
    this->problem_has_been_acknowledged = false;
    this->acknowledgement_type = ACKNOWLEDGEMENT_NONE;

    /* verify the route to the host and send out host recovery notifications */
    if (temp_host->get_current_state() != HOST_UP) {
      logger(dbg_checks, more)
          << "Host is NOT UP, so we'll check it to see if it recovered...";

      /* 08/04/07 EG launch an async (parallel) host check (possibly cached)
       * unless aggressive host checking is enabled */
      /* previous logic was to simply run a sync (serial) host check */
      if (config->use_aggressive_host_checking())
        perform_on_demand_host_check(temp_host, nullptr, CHECK_OPTION_NONE, true,
                                     config->cached_host_check_horizon());
      /* 09/23/07 EG don't launch a new host check if we already did so earlier
       */
      else if (first_host_check_initiated)
        logger(dbg_checks, more)
            << "First host check was already initiated, so we'll skip a "
               "new host check.";
      else {
        /* can we use the last cached host state? */
        /* usually only use cached host state if no service state change has
         * occurred */
        if ((!state_change || state_changes_use_cached_state) &&
            temp_host->get_has_been_checked() &&
            (static_cast<unsigned long>(current_time -
                                        temp_host->get_last_check()) <=
             config->cached_host_check_horizon())) {
          logger(dbg_checks, more) << "* Using cached host state: "
                                   << temp_host->get_current_state();
          update_check_stats(ACTIVE_ONDEMAND_HOST_CHECK_STATS, current_time);
          update_check_stats(ACTIVE_CACHED_HOST_CHECK_STATS, current_time);
        }

        /* else launch an async (parallel) check of the host */
        else
          temp_host->run_async_check(CHECK_OPTION_NONE, 0.0, false, false, nullptr,
                                     nullptr);
      }
    }

    /* if a hard service recovery has occurred... */
    if (hard_state_change) {
      logger(dbg_checks, more) << "Service experienced a HARD RECOVERY.";

      /* set the state type macro */
      this->state_type = HARD_STATE;

      /* log the service recovery */
      log_event();
      state_was_logged = true;

      /* Set the recovery been sent parameter. */
      _recovery_been_sent = false;
      _initial_notif_time = 0;

      /* 10/04/07 check to see if the service and/or associate host is flapping
       */
      /* this should be done before a notification is sent out to ensure the
       * host didn't just start flapping */
      check_for_flapping(true, true);
      temp_host->check_for_flapping(true, false, true);
      flapping_check_done = true;

      /* notify contacts about the service recovery */
      notify(NOTIFICATION_NORMAL, nullptr, nullptr,
                           NOTIFICATION_OPTION_NONE);

      /* run the service event handler to handle the hard state change */
      handle_service_event();
    }

    /* else if a soft service recovery has occurred... */
    else if (state_change) {
      logger(dbg_checks, more) << "Service experienced a SOFT RECOVERY.";

      /* this is a soft recovery */
      this->state_type = SOFT_STATE;

      /* log the soft recovery */
      log_event();
      state_was_logged = true;

      /* run the service event handler to handle the soft state change */
      handle_service_event();
    }

    /* else no service state change has occurred... */
    else
      logger(dbg_checks, more) << "Service did not change state.";

    /* Check if we need to send a recovery notification */
    if (!_recovery_been_sent && !hard_state_change)
      notify(NOTIFICATION_NORMAL, nullptr, nullptr, NOTIFICATION_OPTION_NONE);

    /* should we obsessive over service checks? */
    if (config->obsess_over_services())
      obsessive_compulsive_service_check_processor();

    /* reset all service variables because its okay now... */
    this->host_problem_at_last_check = false;
    this->current_attempt = 1;
    this->state_type = HARD_STATE;
    this->last_hard_state = STATE_OK;
    _last_notification = static_cast<time_t>(0);
    _next_notification = static_cast<time_t>(0);
    if (_recovery_been_sent) {
      this->current_notification_number = 0;
      this->notified_on_unknown = false;
      this->notified_on_warning = false;
      this->notified_on_critical = false;
      _initial_notif_time = static_cast<time_t>(0);
    }
    this->problem_has_been_acknowledged = false;
    this->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
    this->no_more_notifications = false;

    if (reschedule_check)
      next_service_check =
          (time_t)(this->last_check +
                   (this->get_check_interval() * config->interval_length()));
  }

  /*******************************************/
  /******* SERVICE CHECK PROBLEM LOGIC *******/
  /*******************************************/

  /* hey, something's not working quite like it should... */
  else {
    logger(dbg_checks, more) << "Service is in a non-OK state!";

    /* check the route to the host if its up right now... */
    if (temp_host->get_current_state() == HOST_UP) {
      logger(dbg_checks, more)
          << "Host is currently UP, so we'll recheck its state to "
             "make sure...";

      /* 08/04/07 EG launch an async (parallel) host check (possibly cached)
       * unless aggressive host checking is enabled */
      /* previous logic was to simply run a sync (serial) host check */
      if (config->use_aggressive_host_checking())
        perform_on_demand_host_check(temp_host, &route_result,
                                     CHECK_OPTION_NONE, true,
                                     config->cached_host_check_horizon());
      else {
        /* can we use the last cached host state? */
        /* only use cached host state if no service state change has occurred */
        if ((!state_change || state_changes_use_cached_state) &&
            temp_host->get_has_been_checked() &&
            (static_cast<unsigned long>(current_time -
                                        temp_host->get_last_check()) <=
             config->cached_host_check_horizon())) {
          /* use current host state as route result */
          route_result = temp_host->get_current_state();
          logger(dbg_checks, more) << "* Using cached host state: "
                                   << temp_host->get_current_state();
          update_check_stats(ACTIVE_ONDEMAND_HOST_CHECK_STATS, current_time);
          update_check_stats(ACTIVE_CACHED_HOST_CHECK_STATS, current_time);
        }

        /* else launch an async (parallel) check of the host */
        /* CHANGED 02/15/08 only if service changed state since service was last
           checked */
        else if (state_change) {
          /* use current host state as route result */
          route_result = temp_host->get_current_state();
          temp_host->run_async_check(CHECK_OPTION_NONE, 0.0, false, false, nullptr,
                                     nullptr);
        }

        /* ADDED 02/15/08 */
        /* else assume same host state */
        else {
          route_result = temp_host->get_current_state();
          logger(dbg_checks, more) << "* Using last known host state: "
                                   << temp_host->get_current_state();
          update_check_stats(ACTIVE_ONDEMAND_HOST_CHECK_STATS, current_time);
          update_check_stats(ACTIVE_CACHED_HOST_CHECK_STATS, current_time);
        }
      }
    }

    /* else the host is either down or unreachable, so recheck it if necessary
     */
    else {
      logger(dbg_checks, more) << "Host is currently DOWN/UNREACHABLE.";

      /* we're using aggressive host checking, so really do recheck the host...
       */
      if (config->use_aggressive_host_checking()) {
        logger(dbg_checks, more)
            << "Aggressive host checking is enabled, so we'll recheck the "
               "host state...";
        perform_on_demand_host_check(temp_host, &route_result,
                                     CHECK_OPTION_NONE, true,
                                     config->cached_host_check_horizon());
      }

      /* the service wobbled between non-OK states, so check the host... */
      else if ((state_change && !state_changes_use_cached_state) &&
               this->last_hard_state != STATE_OK) {
        logger(dbg_checks, more)
            << "Service wobbled between non-OK states, so we'll recheck"
               " the host state...";
        /* 08/04/07 EG launch an async (parallel) host check unless aggressive
         * host checking is enabled */
        /* previous logic was to simply run a sync (serial) host check */
        /* use current host state as route result */
        route_result = temp_host->get_current_state();
        temp_host->run_async_check(CHECK_OPTION_NONE, 0.0, false, false, nullptr,
                                   nullptr);
        /*perform_on_demand_host_check(temp_host,&route_result,CHECK_OPTION_NONE,true,config->cached_host_check_horizon());
         */
      }

      /* else fake the host check, but (possibly) resend host notifications to
         contacts... */
      else {
        logger(dbg_checks, more)
            << "Assuming host is in same state as before...";

        /* if the host has never been checked before, set the checked flag and
         * last check time */
        /* 03/11/06 EG Note: This probably never evaluates to false, present for
         * historical reasons only, can probably be removed in the future */
        if (!temp_host->get_has_been_checked()) {
          temp_host->set_has_been_checked(true);
          temp_host->set_last_check(this->last_check);
        }

        /* fake the route check result */
        route_result = temp_host->get_current_state();

        /* possibly re-send host notifications... */
        temp_host->notify(NOTIFICATION_NORMAL, nullptr, nullptr,
                          NOTIFICATION_OPTION_NONE);
      }
    }

    /* if the host is down or unreachable ... */
    /* 05/29/2007 NOTE: The host might be in a SOFT problem state due to host
     * check retries/caching.  Not sure if we should take that into account and
     * do something different or not... */
    if (route_result != HOST_UP) {
      logger(dbg_checks, most)
          << "Host is not UP, so we mark state changes if appropriate";

      /* "fake" a hard state change for the service - well, its not really fake,
       * but it didn't get caught earlier... */
      if (this->last_hard_state != this->current_state)
        hard_state_change = true;

      /* update last state change times */
      if (state_change || hard_state_change)
        this->last_state_change = this->last_check;
      if (hard_state_change) {
        this->last_hard_state_change = this->last_check;
        this->state_type = HARD_STATE;
        this->last_hard_state = this->current_state;
      }

      /* put service into a hard state without attempting check retries and
       * don't send out notifications about it */
      this->host_problem_at_last_check = true;
    }

    /* the host is up - it recovered since the last time the service was
       checked... */
    else if (this->host_problem_at_last_check) {
      /* next time the service is checked we shouldn't get into this same
       * case... */
      this->host_problem_at_last_check = false;

      /* reset the current check counter, so we give the service a chance */
      /* this helps prevent the case where service has N max check attempts, N-1
       * of which have already occurred. */
      /* if we didn't do this, the next check might fail and result in a hard
       * problem - we should really give it more time */
      /* ADDED IF STATEMENT 01-17-05 EG */
      /* 01-17-05: Services in hard problem states before hosts went down would
       * sometimes come back as soft problem states after */
      /* the hosts recovered.  This caused problems, so hopefully this will fix
       * it */
      if (this->state_type == SOFT_STATE)
        this->current_attempt = 1;
    }

    logger(dbg_checks, more)
        << "Current/Max Attempt(s): " << this->current_attempt << '/'
        << get_max_attempts();

    /* if we should retry the service check, do so (except it the host is down
     * or unreachable!) */
    if (this->current_attempt < get_max_attempts()) {
      /* the host is down or unreachable, so don't attempt to retry the service
       * check */
      if (route_result != HOST_UP) {
        logger(dbg_checks, more)
            << "Host isn't UP, so we won't retry the service check...";

        /* the host is not up, so reschedule the next service check at regular
         * interval */
        if (reschedule_check)
          next_service_check =
              (time_t)(this->last_check + (this->get_check_interval() *
                                           config->interval_length()));

        /* log the problem as a hard state if the host just went down */
        if (hard_state_change) {
          log_event();
          state_was_logged = true;

          /* run the service event handler to handle the hard state */
          handle_service_event();
        }
      }

      /* the host is up, so continue to retry the service check */
      else {
        logger(dbg_checks, more)
            << "Host is UP, so we'll retry the service check...";

        /* this is a soft state */
        this->state_type = SOFT_STATE;

        /* log the service check retry */
        log_event();
        state_was_logged = true;

        /* run the service event handler to handle the soft state */
        handle_service_event();

        if (reschedule_check)
          next_service_check =
              (time_t)(this->last_check +
                       this->get_retry_interval() * config->interval_length());
      }

      /* perform dependency checks on the second to last check of the service */
      if (config->enable_predictive_service_dependency_checks() &&
          this->current_attempt == (get_max_attempts() - 1)) {
        logger(dbg_checks, more)
            << "Looking for services to check for predictive "
               "dependency checks...";

        /* check services that THIS ONE depends on for notification AND
         * execution */
        /* we do this because we might be sending out a notification soon and we
         * want the dependency logic to be accurate */
        std::pair<std::string, std::string> id(
            std::make_pair(this->get_hostname(), this->get_description()));
        umultimap<std::pair<std::string, std::string>,
                  std::shared_ptr<servicedependency> > const&
            dependencies(state::instance().servicedependencies());
        for (umultimap<std::pair<std::string, std::string>,
                       std::shared_ptr<servicedependency> >::const_iterator
                 it(dependencies.find(id)),
             end(dependencies.end());
             it != end && it->first == id; ++it) {
          servicedependency* temp_dependency(&*it->second);

          if (temp_dependency->dependent_service_ptr == this &&
              temp_dependency->master_service_ptr != nullptr) {
            master_service = (com::centreon::engine::service*)
                                 temp_dependency->master_service_ptr;
            logger(dbg_checks, most)
                << "Predictive check of service '"
                << master_service->get_description() << "' on host '"
                << master_service->get_hostname() << "' queued.";
            add_object_to_objectlist(&check_servicelist, (void*)master_service);
          }
        }
      }
    }

    /* we've reached the maximum number of service rechecks, so handle the error
     */
    else {
      logger(dbg_checks, more)
          << "Service has reached max number of rechecks, so we'll "
             "handle the error...";

      /* this is a hard state */
      this->state_type = HARD_STATE;

      /* if we've hard a hard state change... */
      if (hard_state_change) {
        /* log the service problem (even if host is not up, which is new in
         * 0.0.5) */
        log_event();
        state_was_logged = true;
      }

      /* else log the problem (again) if this service is flagged as being
         volatile */
      else if (this->is_volatile) {
        log_event();
        state_was_logged = true;
      }

      /* check for start of flexible (non-fixed) scheduled downtime if we just
       * had a hard error */
      /* we need to check for both, state_change (SOFT) and hard_state_change
       * (HARD) values */
      if (((true == hard_state_change) || (true == state_change)) &&
          (this->pending_flex_downtime > 0))
        downtime_manager::instance().check_pending_flex_service_downtime(this);

      /* 10/04/07 check to see if the service and/or associate host is flapping
       */
      /* this should be done before a notification is sent out to ensure the
       * host didn't just start flapping */
      check_for_flapping(true, true);
      temp_host->check_for_flapping(true, false, true);
      flapping_check_done = true;

      /* (re)send notifications out about this service problem if the host is up
       * (and was at last check also) and the dependencies were okay... */
      notify(NOTIFICATION_NORMAL, nullptr, nullptr,
                           NOTIFICATION_OPTION_NONE);

      /* run the service event handler if we changed state from the last hard
       * state or if this service is flagged as being volatile */
      if (hard_state_change || this->is_volatile)
        handle_service_event();

      /* save the last hard state */
      this->last_hard_state = this->current_state;

      /* reschedule the next check at the regular interval */
      if (reschedule_check)
        next_service_check =
            (time_t)(this->last_check +
                     (this->get_check_interval() * config->interval_length()));
    }

    /* should we obsessive over service checks? */
    if (config->obsess_over_services())
      obsessive_compulsive_service_check_processor();
  }

  /* reschedule the next service check ONLY for active, scheduled checks */
  if (reschedule_check) {
    logger(dbg_checks, more) << "Rescheduling next check of service at "
                             << my_ctime(&next_service_check);

    /* default is to reschedule service check unless a test below fails... */
    this->should_be_scheduled = true;

    /* next check time was calculated above */
    this->next_check = next_service_check;

    /* make sure we don't get ourselves into too much trouble... */
    if (current_time > this->next_check)
      this->next_check = current_time;

    // Make sure we rescheduled the next service check at a valid time.
    {
      timezone_locker lock(
          get_service_timezone(this->get_hostname(), this->get_description()));
      preferred_time = this->next_check;
      get_next_valid_time(preferred_time, &next_valid_time,
                          this->check_period_ptr);
      this->next_check = next_valid_time;
    }

    /* services with non-recurring intervals do not get rescheduled */
    if (this->get_check_interval() == 0)
      this->should_be_scheduled = false;

    /* services with active checks disabled do not get rescheduled */
    if (!this->checks_enabled)
      this->should_be_scheduled = false;

    /* schedule a non-forced check if we can */
    if (this->should_be_scheduled)
      schedule_check(this->next_check, CHECK_OPTION_NONE);
  }

  /* if we're stalking this state type and state was not already logged AND the
   * plugin output changed since last check, log it now.. */
  if (this->state_type == HARD_STATE && !state_change && !state_was_logged &&
      old_plugin_output == get_plugin_output()) {
    if ((this->current_state == STATE_OK && this->stalk_on_ok))
      log_event();

    else if ((this->current_state == STATE_WARNING && this->stalk_on_warning))
      log_event();

    else if ((this->current_state == STATE_UNKNOWN && this->stalk_on_unknown))
      log_event();

    else if ((this->current_state == STATE_CRITICAL && this->stalk_on_critical))
      log_event();
  }

  /* send data to event broker */
  broker_service_check(NEBTYPE_SERVICECHECK_PROCESSED, NEBFLAG_NONE,
                       NEBATTR_NONE, this, this->check_type,
                       queued_check_result->start_time,
                       queued_check_result->finish_time, nullptr, this->latency,
                       this->execution_time, config->service_check_timeout(),
                       queued_check_result->early_timeout,
                       queued_check_result->return_code, nullptr, nullptr);

  if (!(reschedule_check && this->should_be_scheduled &&
        this->has_been_checked) ||
      !this->checks_enabled) {
    /* set the checked flag */
    this->has_been_checked = true;
    /* update the current service status log */
    this->update_status(false);
  }

  /* check to see if the service and/or associate host is flapping */
  if (!flapping_check_done) {
    check_for_flapping(true, true);
    temp_host->check_for_flapping(true, false, true);
  }

  /* update service performance info */
  update_service_performance_data();

  /* run async checks of all services we added above */
  /* don't run a check if one is already executing or we can get by with a
   * cached state */
  for (servicelist_item = check_servicelist; servicelist_item != nullptr;
       servicelist_item = servicelist_item->next) {
    run_async_check = true;
    service* svc = static_cast<service*>(servicelist_item->object_ptr);

    /* we can get by with a cached state, so don't check the service */
    if (static_cast<unsigned long>(current_time - svc->last_check) <=
        config->cached_service_check_horizon()) {
      run_async_check = false;

      /* update check statistics */
      update_check_stats(ACTIVE_CACHED_SERVICE_CHECK_STATS, current_time);
    }

    if (svc->is_executing)
      run_async_check = false;

    if (run_async_check)
      svc->run_async_check(CHECK_OPTION_NONE, 0.0, false, false, nullptr, nullptr);
  }
  free_objectlist(&check_servicelist);
  return OK;
}

/**
 *  Log service event information.
 *  This function has been DEPRECATED.
 *
 *  @param[in] svc The service to log.
 *
 *  @return Return true on success.
 */
int service::log_event() {
  if (this->state_type == SOFT_STATE && !config->log_service_retries())
    return OK;

  if (!this->host_ptr)
    return ERROR;

  uint32_t log_options{NSLOG_SERVICE_UNKNOWN};
  char const* state{"UNKNOWN"};
  if (this->current_state >= 0 &&
      (unsigned int)this->current_state < tab_service_states.size()) {
    log_options = tab_service_states[this->current_state].first;
    state = tab_service_states[this->current_state].second.c_str();
  }
  std::string const& state_type{tab_state_type[this->state_type]};

  logger(log_options, basic)
      << "SERVICE ALERT: " << get_hostname() << ";"
      << get_description() << ";" << state << ";" << state_type << ";"
      << this->current_attempt << ";" << get_plugin_output();
  return OK;
}

// int service::get_check_viability(...)  << check_service_check_viability()
/* detects service flapping */
void service::check_for_flapping(int update,
                                         int allow_flapstart_notification) {
  int update_history = true;
  int is_flapping = false;
  unsigned int x = 0;
  unsigned int y = 0;
  int last_state_history_value = STATE_OK;
  double curved_changes = 0.0;
  double curved_percent_change = 0.0;
  double low_threshold = 0.0;
  double high_threshold = 0.0;
  double low_curve_value = 0.75;
  double high_curve_value = 1.25;

  /* large install tweaks skips all flap detection logic - including state
   * change calculation */

  logger(dbg_functions, basic) << "check_for_flapping()";

  logger(dbg_flapping, more)
      << "Checking service '" << get_description() << "' on host '"
      << get_hostname() << "' for flapping...";

  /* if this is a soft service state and not a soft recovery, don't record this
   * in the history */
  /* only hard states and soft recoveries get recorded for flap detection */
  if (this->state_type == SOFT_STATE && this->current_state != STATE_OK)
    return;

  /* what threshold values should we use (global or service-specific)? */
  low_threshold = (this->low_flap_threshold <= 0.0)
                      ? config->low_service_flap_threshold()
                      : this->low_flap_threshold;
  high_threshold = (this->high_flap_threshold <= 0.0)
                       ? config->high_service_flap_threshold()
                       : this->high_flap_threshold;

  update_history = update;

  /* should we update state history for this state? */
  if (update_history) {
    if (this->current_state == STATE_OK && !this->flap_detection_on_ok)
      update_history = false;
    if (this->current_state == STATE_WARNING &&
        !this->flap_detection_on_warning)
      update_history = false;
    if (this->current_state == STATE_UNKNOWN &&
        !this->flap_detection_on_unknown)
      update_history = false;
    if (this->current_state == STATE_CRITICAL &&
        !this->flap_detection_on_critical)
      update_history = false;
  }

  /* record current service state */
  if (update_history) {
    /* record the current state in the state history */
    this->state_history[this->state_history_index] = this->current_state;

    /* increment state history index to next available slot */
    this->state_history_index++;
    if (this->state_history_index >= MAX_STATE_HISTORY_ENTRIES)
      this->state_history_index = 0;
  }

  /* calculate overall and curved percent state changes */
  for (x = 0, y = this->state_history_index; x < MAX_STATE_HISTORY_ENTRIES;
       x++) {
    if (x == 0) {
      last_state_history_value = this->state_history[y];
      y++;
      if (y >= MAX_STATE_HISTORY_ENTRIES)
        y = 0;
      continue;
    }

    if (last_state_history_value != this->state_history[y])
      curved_changes +=
          (((double)(x - 1) * (high_curve_value - low_curve_value)) /
           ((double)(MAX_STATE_HISTORY_ENTRIES - 2))) +
          low_curve_value;

    last_state_history_value = this->state_history[y];

    y++;
    if (y >= MAX_STATE_HISTORY_ENTRIES)
      y = 0;
  }

  /* calculate overall percent change in state */
  curved_percent_change = (double)(((double)curved_changes * 100.0) /
                                   (double)(MAX_STATE_HISTORY_ENTRIES - 1));

  this->percent_state_change = curved_percent_change;

  logger(dbg_flapping, most)
      << com::centreon::logging::setprecision(2) << "LFT=" << low_threshold
      << ", HFT=" << high_threshold << ", CPC=" << curved_percent_change
      << ", PSC=" << curved_percent_change << "%";

  /* don't do anything if we don't have flap detection enabled on a program-wide
   * basis */
  if (!config->enable_flap_detection())
    return;

  /* don't do anything if we don't have flap detection enabled for this service
   */
  if (!this->flap_detection_enabled)
    return;

  /* are we flapping, undecided, or what?... */

  /* we're undecided, so don't change the current flap state */
  if (curved_percent_change > low_threshold &&
      curved_percent_change < high_threshold)
    return;
  /* we're below the lower bound, so we're not flapping */
  else if (curved_percent_change <= low_threshold)
    is_flapping = false;
  /* else we're above the upper bound, so we are flapping */
  else if (curved_percent_change >= high_threshold)
    is_flapping = true;

  logger(dbg_flapping, more)
      << com::centreon::logging::setprecision(2) << "Service "
      << (is_flapping ? "is" : "is not") << " flapping ("
      << curved_percent_change << "% state change).";

  /* did the service just start flapping? */
  if (is_flapping && !this->is_flapping)
    set_flap(curved_percent_change, high_threshold, low_threshold,
             allow_flapstart_notification);

  /* did the service just stop flapping? */
  else if (!is_flapping && this->is_flapping)
    clear_flap(curved_percent_change, high_threshold, low_threshold);
}

/* handles changes in the state of a service */
int service::handle_service_event() {
  com::centreon::engine::host* temp_host = nullptr;
  nagios_macros mac;

  logger(dbg_functions, basic) << "handle_service_event()";

  /* send event data to broker */
  broker_statechange_data(NEBTYPE_STATECHANGE_END, NEBFLAG_NONE, NEBATTR_NONE,
                          SERVICE_STATECHANGE, (void*)this, this->current_state,
                          this->state_type, this->current_attempt,
                          get_max_attempts(), nullptr);

  /* bail out if we shouldn't be running event handlers */
  if (config->enable_event_handlers() == false)
    return OK;
  if (this->event_handler_enabled == false)
    return OK;

  /* find the host */
  if ((temp_host = (com::centreon::engine::host*)this->host_ptr) == nullptr)
    return ERROR;

  /* update service macros */
  memset(&mac, 0, sizeof(mac));
  grab_host_macros_r(&mac, temp_host);
  grab_service_macros_r(&mac, this);

  /* run the global service event handler */
  run_global_service_event_handler(&mac, this);

  /* run the event handler command if there is one */
  if (!get_event_handler().empty())
    run_service_event_handler(&mac, this);
  clear_volatile_macros_r(&mac);

  /* send data to event broker */
  broker_external_command(NEBTYPE_EXTERNALCOMMAND_CHECK, NEBFLAG_NONE,
                          NEBATTR_NONE, CMD_NONE, time(nullptr), nullptr, nullptr, nullptr);

  return OK;
}

/* handles service check results in an obsessive compulsive manner... */
int service::obsessive_compulsive_service_check_processor() {
  char* raw_command = nullptr;
  char* processed_command = nullptr;
  com::centreon::engine::host* temp_host = nullptr;
  int early_timeout = false;
  double exectime = 0.0;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;
  nagios_macros mac;

  logger(dbg_functions, basic)
      << "obsessive_compulsive_service_check_processor()";

  /* bail out if we shouldn't be obsessing */
  if (config->obsess_over_services() == false)
    return OK;
  if (this->obsess_over_service == false)
    return OK;

  /* if there is no valid command, exit */
  if (config->ocsp_command().empty())
    return ERROR;

  /* find the associated host */
  if ((temp_host = (com::centreon::engine::host*)this->host_ptr) == nullptr)
    return ERROR;

  /* update service macros */
  memset(&mac, 0, sizeof(mac));
  grab_host_macros_r(&mac, temp_host);
  grab_service_macros_r(&mac, this);

  /* get the raw command line */
  get_raw_command_line_r(&mac, ocsp_command_ptr, config->ocsp_command().c_str(),
                         &raw_command, macro_options);
  if (raw_command == nullptr) {
    clear_volatile_macros_r(&mac);
    return ERROR;
  }

  logger(dbg_checks, most) << "Raw obsessive compulsive service processor "
                              "command line: "
                           << raw_command;

  /* process any macros in the raw command line */
  process_macros_r(&mac, raw_command, &processed_command, macro_options);
  if (processed_command == nullptr) {
    clear_volatile_macros_r(&mac);
    return ERROR;
  }

  logger(dbg_checks, most) << "Processed obsessive compulsive service "
                              "processor command line: "
                           << processed_command;

  /* run the command */
  try {
    my_system_r(&mac, processed_command, config->ocsp_timeout(), &early_timeout,
                &exectime, nullptr, 0);
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
        << "Error: can't execute compulsive service processor command line '"
        << processed_command << "' : " << e.what();
  }

  clear_volatile_macros_r(&mac);

  /* check to see if the command timed out */
  if (early_timeout == true)
    logger(log_runtime_warning, basic)
        << "Warning: OCSP command '" << processed_command << "' for service '"
        << this->get_description() << "' on host '" << this->get_hostname()
        << "' timed out after " << config->ocsp_timeout() << " seconds";

  /* free memory */
  delete[] raw_command;
  delete[] processed_command;

  return OK;
}

/* updates service performance data */
int service::update_service_performance_data() {
  /* should we be processing performance data for anything? */
  if (!config->process_performance_data())
    return OK;

  /* should we process performance data for this service? */
  if (!this->process_performance_data)
    return OK;

  /* process the performance data! */
  xpddefault_update_service_performance_data(this);
  return OK;
}

/* executes a scheduled service check */
int service::run_scheduled_check(int check_options, double latency) {
  int result = OK;
  time_t current_time = 0L;
  time_t preferred_time = 0L;
  time_t next_valid_time = 0L;
  int time_is_valid = true;

  logger(dbg_functions, basic) << "run_scheduled_service_check()";
  logger(dbg_checks, basic)
      << "Attempting to run scheduled check of service '"
      << this->get_description() << "' on host '" << this->get_hostname()
      << "': check options=" << check_options << ", latency=" << latency;

  /* attempt to run the check */
  result = run_async_check(check_options, latency, true, true, &time_is_valid,
                           &preferred_time);

  /* an error occurred, so reschedule the check */
  if (result == ERROR) {
    logger(dbg_checks, more)
        << "Unable to run scheduled service check at this time";

    /* only attempt to (re)schedule checks that should get checked... */
    if (this->should_be_scheduled) {
      /* get current time */
      time(&current_time);

      /*
       * determine next time we should check the service if needed
       * if service has no check interval, schedule it again for 5
       * minutes from now
       * */
      if (current_time >= preferred_time)
        preferred_time =
            current_time +
            static_cast<time_t>(this->get_check_interval() <= 0
                                    ? 300
                                    : this->get_check_interval() *
                                          config->interval_length());

      // Make sure we rescheduled the next service check at a valid time.
      {
        timezone_locker lock(get_service_timezone(this->get_hostname(),
                                                  this->get_description()));
        get_next_valid_time(preferred_time, &next_valid_time,
                            this->check_period_ptr);

        // The service could not be rescheduled properly.
        // Set the next check time for next week.
        if (!time_is_valid &&
            check_time_against_period(next_valid_time,
                                      this->check_period_ptr) == ERROR) {
          this->next_check = (time_t)(next_valid_time + (60 * 60 * 24 * 7));
          logger(log_runtime_warning, basic)
              << "Warning: Check of service '" << this->get_description()
              << "' on host '" << this->get_hostname()
              << "' could not be "
                 "rescheduled properly. Scheduling check for next week...";
          logger(dbg_checks, more)
              << "Unable to find any valid times to reschedule the next "
                 "service check!";
        }
        // This service could be rescheduled...
        else {
          this->next_check = next_valid_time;
          this->should_be_scheduled = true;
          logger(dbg_checks, more) << "Rescheduled next service check for "
                                   << my_ctime(&next_valid_time);
        }
      }
    }

    /*
     * reschedule the next service check - unless we couldn't find a valid
     * next check time
     * 10/19/07 EG - keep original check options
     */
    if (this->should_be_scheduled)
      schedule_check(this->next_check, check_options);

    /* update the status log */
    this->update_status(false);
    return ERROR;
  }
  return OK;
}

/*
 * forks a child process to run a service check, but does not wait for the
 * service check result
 */
int service::run_async_check(int check_options,
                             double latency,
                             int scheduled_check,
                             int reschedule_check,
                             int* time_is_valid,
                             time_t* preferred_time) {
  try {
    checks::checker::instance().run(this, check_options, latency,
                                    scheduled_check, reschedule_check,
                                    time_is_valid, preferred_time);
  } catch (checks::viability_failure const& e) {
    // Do not log viability failures.
    (void)e;
    return ERROR;
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic) << "Error: " << e.what();
    return ERROR;
  }
  return OK;
}

/**
 *  Schedules an immediate or delayed service check.
 *
 *  @param[in] svc         Target service.
 *  @param[in] check_time  Desired check time.
 *  @param[in] options     Check options (FORCED, FRESHNESS, ...).
 */
void service::schedule_check(time_t check_time, int options) {
  logger(dbg_functions, basic) << "schedule_service_check()";

  logger(dbg_checks, basic)
      << "Scheduling a "
      << (options & CHECK_OPTION_FORCE_EXECUTION ? "forced" : "non-forced")
      << ", active check of service '" << this->get_description()
      << "' on host '" << this->get_hostname() << "' @ "
      << my_ctime(&check_time);

  // Don't schedule a check if active checks
  // of this service are disabled.
  if (!this->checks_enabled && !(options & CHECK_OPTION_FORCE_EXECUTION)) {
    logger(dbg_checks, basic) << "Active checks of this service are disabled.";
    return;
  }

  // Default is to use the new event.
  bool use_original_event(false);
  timed_event* temp_event = quick_timed_event.find(
      hash_timed_event::low, hash_timed_event::service_check, this);

  // We found another service check event for this service in
  // the queue - what should we do?
  if (temp_event) {
    logger(dbg_checks, most)
        << "Found another service check event for this service @ "
        << my_ctime(&temp_event->run_time);

    // Use the originally scheduled check unless we decide otherwise.
    use_original_event = true;

    // The original event is a forced check...
    if ((temp_event->event_options & CHECK_OPTION_FORCE_EXECUTION)) {
      // The new event is also forced and its execution time is earlier
      // than the original, so use it instead.
      if ((options & CHECK_OPTION_FORCE_EXECUTION) &&
          (check_time < temp_event->run_time)) {
        use_original_event = false;
        logger(dbg_checks, most)
            << "New service check event is forced and occurs before the "
               "existing event, so the new event will be used instead.";
      }
    }
    // The original event is not a forced check...
    else {
      // The new event is a forced check, so use it instead.
      if ((options & CHECK_OPTION_FORCE_EXECUTION)) {
        use_original_event = false;
        logger(dbg_checks, most)
            << "New service check event is forced, so it will be used "
               "instead of the existing event.";
      }
      // The new event is not forced either and its execution time is
      // earlier than the original, so use it instead.
      else if (check_time < temp_event->run_time) {
        use_original_event = false;
        logger(dbg_checks, most)
            << "New service check event occurs before the existing "
               "(older) event, so it will be used instead.";
      }
      // The new event is older, so override the existing one.
      else {
        logger(dbg_checks, most)
            << "New service check event occurs after the existing event, "
               "so we'll ignore it.";
      }
    }
  }

  // Save check options for retention purposes.
  this->check_options = options;

  // Schedule a new event.
  if (!use_original_event) {
    // We're using the new event, so remove the old one.
    if (temp_event) {
      remove_event(temp_event, &event_list_low, &event_list_low_tail);
      delete temp_event;
      temp_event = nullptr;
    }

    logger(dbg_checks, most) << "Scheduling new service check event.";

    // Allocate memory for a new event item.
    try {
      timed_event* new_event(new timed_event);

      // Set the next service check time.
      this->next_check = check_time;

      // Place the new event in the event queue.
      new_event->event_type = EVENT_SERVICE_CHECK;
      new_event->event_data = (void*)this;
      new_event->event_args = (void*)nullptr;
      new_event->event_options = options;
      new_event->run_time = this->next_check;
      new_event->recurring = false;
      new_event->event_interval = 0L;
      new_event->timing_func = nullptr;
      new_event->compensate_for_time_change = true;
      reschedule_event(new_event, &event_list_low, &event_list_low_tail);
    } catch (...) {
      // Update the status log.
      this->update_status(false);
      throw;
    }
  } else {
    // Reset the next check time (it may be out of sync).
    if (temp_event)
      this->next_check = temp_event->run_time;

    logger(dbg_checks, most)
        << "Keeping original service check event (ignoring the new one).";
  }

  // Update the status log.
  this->update_status(false);
}

void service::set_flap(double percent_change,
                       double high_threshold,
                       double low_threshold,
                       int allow_flapstart_notification) {
  logger(dbg_functions, basic) << "set_service_flap()";

  logger(dbg_flapping, more)
      << "Service '" << this->get_description() << "' on host '"
      << this->get_hostname() << "' started flapping!";

  /* log a notice - this one is parsed by the history CGI */
  logger(log_runtime_warning, basic)
      << com::centreon::logging::setprecision(1)
      << "SERVICE FLAPPING ALERT: " << this->get_hostname() << ";"
      << this->get_description()
      << ";STARTED; Service appears to have started flapping ("
      << percent_change << "% change >= " << high_threshold << "% threshold)";

  /* add a non-persistent comment to the service */
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(1)
      << "Notifications for this service are being suppressed because "
         "it was detected as "
      << "having been flapping between different "
         "states ("
      << percent_change << "% change >= " << high_threshold
      << "% threshold).  When the service state stabilizes and the "
         "flapping "
      << "stops, notifications will be re-enabled.";

  std::shared_ptr<comment> com{new comment(
      comment::service, comment::flapping, this->get_hostname(),
      this->get_description(), time(nullptr), "(Centreon Engine Process)",
      oss.str(), false, comment::internal, false, (time_t)0)};

  comment::comments.insert({com->get_comment_id(), com});

  this->flapping_comment_id = com->get_comment_id();

  /* set the flapping indicator */
  this->is_flapping = true;

  /* send data to event broker */
  broker_flapping_data(NEBTYPE_FLAPPING_START, NEBFLAG_NONE, NEBATTR_NONE,
                       SERVICE_FLAPPING, this, percent_change, high_threshold,
                       low_threshold, nullptr);

  /* see if we should check to send a recovery notification out when flapping
   * stops */
  if (this->current_state != STATE_OK && this->current_notification_number > 0)
    this->check_flapping_recovery_notification = true;
  else
    this->check_flapping_recovery_notification = false;

  /* send a notification */
  if (allow_flapstart_notification)
    notify(NOTIFICATION_FLAPPINGSTART, nullptr, nullptr,
                         NOTIFICATION_OPTION_NONE);
}

/* handles a service that has stopped flapping */
void service::clear_flap(double percent_change,
                         double high_threshold,
                         double low_threshold) {
  logger(dbg_functions, basic) << "clear_service_flap()";

  logger(dbg_flapping, more)
      << "Service '" << this->get_description() << "' on host '"
      << this->get_hostname() << "' stopped flapping.";

  /* log a notice - this one is parsed by the history CGI */
  logger(log_info_message, basic)
      << com::centreon::logging::setprecision(1)
      << "SERVICE FLAPPING ALERT: " << this->get_hostname() << ";"
      << this->get_description()
      << ";STOPPED; Service appears to have stopped flapping ("
      << percent_change << "% change < " << low_threshold << "% threshold)";

  /* delete the comment we added earlier */
  if (this->flapping_comment_id != 0)
    comment::delete_comment(this->flapping_comment_id);
  this->flapping_comment_id = 0;

  /* clear the flapping indicator */
  this->is_flapping = false;

  /* send data to event broker */
  broker_flapping_data(NEBTYPE_FLAPPING_STOP, NEBFLAG_NONE,
                       NEBATTR_FLAPPING_STOP_NORMAL, SERVICE_FLAPPING, this,
                       percent_change, high_threshold, low_threshold, nullptr);

  /* send a notification */
  notify(NOTIFICATION_FLAPPINGSTOP, nullptr, nullptr,
                       NOTIFICATION_OPTION_NONE);

  /* should we send a recovery notification? */
  if (this->check_flapping_recovery_notification &&
      this->current_state == STATE_OK)
    notify(NOTIFICATION_NORMAL, nullptr, nullptr,
                         NOTIFICATION_OPTION_NONE);

  /* clear the recovery notification flag */
  this->check_flapping_recovery_notification = false;
}

/* enables flap detection for a specific service */
void service::enable_flap_detection() {
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic)
    << "enable_service_flap_detection()";

  logger(dbg_flapping, more)
    << "Enabling flap detection for service '" << this->get_description()
    << "' on host '" << this->get_hostname() << "'.";

  /* nothing to do... */
  if (this->flap_detection_enabled)
    return;

  /* set the attribute modified flag */
  this->modified_attributes |= attr;

  /* set the flap detection enabled flag */
  this->flap_detection_enabled = true;

  /* send data to event broker */
  broker_adaptive_service_data(
    NEBTYPE_ADAPTIVESERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    CMD_NONE,
    attr,
    this->modified_attributes,
    nullptr);

  /* check for flapping */
  check_for_flapping(false, true);

  /* update service status */
  this->update_status(false);
}

/* disables flap detection for a specific service */
void service::disable_flap_detection() {
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic)
    << "disable_service_flap_detection()";

  logger(dbg_flapping, more)
    << "Disabling flap detection for service '" << get_description()
    << "' on host '" << get_hostname() << "'.";

  /* nothing to do... */
  if (!this->flap_detection_enabled)
    return;

  /* set the attribute modified flag */
  this->modified_attributes |= attr;

  /* set the flap detection enabled flag */
  this->flap_detection_enabled = false;

  /* send data to event broker */
  broker_adaptive_service_data(
    NEBTYPE_ADAPTIVESERVICE_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this,
    CMD_NONE,
    attr,
    this->modified_attributes,
    nullptr);

  /* handle the details... */
  handle_service_flap_detection_disabled(this);
}

/* updates service status info */
void service::update_status(bool aggregated_dump) {
  /* send data to event broker (non-aggregated dumps only) */
  if (!aggregated_dump)
    broker_service_status(
      NEBTYPE_SERVICESTATUS_UPDATE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      this,
      nullptr);
}

/* sets the current notification number for a specific service */
void service::set_notification_number(int num) {
  /* set the notification number */
  current_notification_number = num;

  /* update the status log with the service info */
  update_status(false);
}

/* checks the viability of sending out a service alert (top level filters) */
int service::check_notification_viability(unsigned int type, int options) {
  host* temp_host;
  timeperiod* temp_period;
  time_t current_time;
  time_t timeperiod_start;

  logger(dbg_functions, basic) << "check_service_notification_viability()";

  /* forced notifications bust through everything */
  if (options & NOTIFICATION_OPTION_FORCED) {
    logger(dbg_notifications, more)
        << "This is a forced service notification, so we'll send it out.";
    return OK;
  }

  /* get current time */
  time(&current_time);

  /* are notifications enabled? */
  if (config->enable_notifications() == false) {
    logger(dbg_notifications, more)
        << "Notifications are disabled, so service notifications will "
           "not be sent out.";
    return ERROR;
  }

  /* find the host this service is associated with */
  if ((temp_host = (host*)this->host_ptr) == nullptr) {
    logger(dbg_notifications, more)
        << "Couldn't find the host associated with this service, "
           "so we won't send a notification.";
    return ERROR;
  }

  /* if the service has no notification period, inherit one from the host */
  temp_period = this->notification_period_ptr;
  if (temp_period == nullptr)
    temp_period = this->host_ptr->notification_period_ptr;

  // See if the service can have notifications sent out at this time.
  {
    timezone_locker lock(
        get_service_timezone(this->get_hostname(), this->get_description()));
    if (check_time_against_period(current_time, temp_period) == ERROR) {
      logger(dbg_notifications, more)
          << "This service shouldn't have notifications sent out "
             "at this time.";

      // Calculate the next acceptable notification time,
      // once the next valid time range arrives...
      if (type == NOTIFICATION_NORMAL) {
        get_next_valid_time(current_time, &timeperiod_start,
                            this->notification_period_ptr);

        // Looks like there are no valid notification times defined, so
        // schedule the next one far into the future (one year)...
        if (timeperiod_start == (time_t)0)
          _next_notification = static_cast<time_t>(current_time + 60 * 60 * 24 * 365);
        // Else use the next valid notification time.
        else
          _next_notification = timeperiod_start;
        logger(dbg_notifications, more) << "Next possible notification time: "
                                        << my_ctime(&_next_notification);
      }

      return ERROR;
    }
  }

  /* are notifications temporarily disabled for this service? */
  if (this->notifications_enabled == false) {
    logger(dbg_notifications, more)
        << "Notifications are temporarily disabled for "
           "this service, so we won't send one out.";
    return ERROR;
  }

  /*********************************************/
  /*** SPECIAL CASE FOR CUSTOM NOTIFICATIONS ***/
  /*********************************************/

  /* custom notifications are good to go at this point... */
  if (type == NOTIFICATION_CUSTOM) {
    if (this->scheduled_downtime_depth > 0 ||
        temp_host->get_scheduled_downtime_depth() > 0) {
      logger(dbg_notifications, more)
          << "We shouldn't send custom notification during "
             "scheduled downtime.";
      return ERROR;
    }
    return OK;
  }

  /****************************************/
  /*** SPECIAL CASE FOR ACKNOWLEGEMENTS ***/
  /****************************************/

  /*
   * acknowledgements only have to pass three general filters, although they
   * have another test of their own...
   */
  if (type == NOTIFICATION_ACKNOWLEDGEMENT) {
    /* don't send an acknowledgement if there isn't a problem... */
    if (this->current_state == STATE_OK) {
      logger(dbg_notifications, more)
          << "The service is currently OK, so we won't send an "
             "acknowledgement.";
      return ERROR;
    }

    /*
     * acknowledgement viability test passed, so the notification can be sent
     * out
     */
    return OK;
  }

  /****************************************/
  /*** SPECIAL CASE FOR FLAPPING ALERTS ***/
  /****************************************/

  /* flapping notifications only have to pass three general filters */
  if (type == NOTIFICATION_FLAPPINGSTART || type == NOTIFICATION_FLAPPINGSTOP ||
      type == NOTIFICATION_FLAPPINGDISABLED) {
    /* don't send a notification if we're not supposed to... */
    if (this->notify_on_flapping == false) {
      logger(dbg_notifications, more)
          << "We shouldn't notify about FLAPPING events for this "
             "service.";
      return ERROR;
    }

    /* don't send notifications during scheduled downtime */
    if (this->scheduled_downtime_depth > 0 ||
        temp_host->get_scheduled_downtime_depth() > 0) {
      logger(dbg_notifications, more)
          << "We shouldn't notify about FLAPPING events during "
             "scheduled downtime.";
      return ERROR;
    }

    /* flapping viability test passed, so the notification can be sent out */
    return OK;
  }

  /****************************************/
  /*** SPECIAL CASE FOR DOWNTIME ALERTS ***/
  /****************************************/

  /* downtime notifications only have to pass three general filters */
  if (type == NOTIFICATION_DOWNTIMESTART || type == NOTIFICATION_DOWNTIMEEND ||
      type == NOTIFICATION_DOWNTIMECANCELLED) {
    /* don't send a notification if we're not supposed to... */
    if (this->notify_on_downtime == false) {
      logger(dbg_notifications, more)
          << "We shouldn't notify about DOWNTIME events for "
             "this service.";
      return ERROR;
    }

    /*
     * don't send notifications during scheduled downtime (for service only,
     * not host)
     */
    if (this->scheduled_downtime_depth > 0) {
      logger(dbg_notifications, more)
          << "We shouldn't notify about DOWNTIME events during "
             "scheduled downtime.";
      return ERROR;
    }

    /* downtime viability test passed, so the notification can be sent out */
    return OK;
  }

  /****************************************/
  /*** NORMAL NOTIFICATIONS ***************/
  /****************************************/

  /* is this a hard problem/recovery? */
  if (this->state_type == SOFT_STATE) {
    logger(dbg_notifications, more)
        << "This service is in a soft state, so we won't send a "
           "notification out.";
    return ERROR;
  }

  /* has this problem already been acknowledged? */
  if (this->problem_has_been_acknowledged == true) {
    logger(dbg_notifications, more)
        << "This service problem has already been acknowledged, "
           "so we won't send a notification out.";
    return ERROR;
  }

  /* check service notification dependencies */
  if (check_service_dependencies(this, hostdependency::notification) ==
      DEPENDENCIES_FAILED) {
    logger(dbg_notifications, more)
        << "Service notification dependencies for this service "
           "have failed, so we won't sent a notification out.";
    return ERROR;
  }

  /* check host notification dependencies */
  if (check_host_dependencies(temp_host, hostdependency::notification) ==
      DEPENDENCIES_FAILED) {
    logger(dbg_notifications, more)
        << "Host notification dependencies for this service have failed, "
           "so we won't sent a notification out.";
    return ERROR;
  }

  /* see if we should notify about problems with this service */
  if (this->current_state == STATE_UNKNOWN &&
      this->notify_on_unknown == false) {
    logger(dbg_notifications, more)
        << "We shouldn't notify about UNKNOWN states for this service.";
    return ERROR;
  }
  if (this->current_state == STATE_WARNING &&
      this->notify_on_warning == false) {
    logger(dbg_notifications, more)
        << "We shouldn't notify about WARNING states for this service.";
    return ERROR;
  }
  if (this->current_state == STATE_CRITICAL &&
      this->notify_on_critical == false) {
    logger(dbg_notifications, more)
        << "We shouldn't notify about CRITICAL states for this service.";
    return ERROR;
  }
  if (this->current_state == STATE_OK) {
    if (this->notify_on_recovery == false) {
      logger(dbg_notifications, more)
          << "We shouldn't notify about RECOVERY states for this service.";
      return ERROR;
    }
    if (!(this->notified_on_unknown == true ||
          this->notified_on_warning == true ||
          this->notified_on_critical == true)) {
      logger(dbg_notifications, more)
          << "We shouldn't notify about this recovery.";
      return ERROR;
    }
  }

  /* see if enough time has elapsed for first notification */
  if (type == NOTIFICATION_NORMAL &&
      (this->current_notification_number == 0 ||
       (this->current_state == STATE_OK && !_recovery_been_sent))) {
    /* get the time at which a notification should have been sent */
    time_t& initial_notif_time(_initial_notif_time);

    /* if not set, set it to now */
    if (!initial_notif_time)
      initial_notif_time = time(nullptr);

    double notification_delay =
        (this->current_state != STATE_OK ? this->first_notification_delay
                                         : _recovery_notification_delay) *
        config->interval_length();

    if (current_time <
        (time_t)(initial_notif_time + (time_t)(notification_delay))) {
      if (this->current_state == STATE_OK)
        logger(dbg_notifications, more)
            << "Not enough time has elapsed since the service changed to a "
               "OK state, so we should not notify about this problem yet";
      else
        logger(dbg_notifications, more)
            << "Not enough time has elapsed since the service changed to a "
               "non-OK state, so we should not notify about this problem yet";
      return ERROR;
    }
  }

  /* if this service is currently flapping, don't send the notification */
  if (this->is_flapping == true) {
    logger(dbg_notifications, more)
        << "This service is currently flapping, so we won't send "
           "notifications.";
    return ERROR;
  }

  /*
   * if this service is currently in a scheduled downtime period, don't send
   * the notification
   */
  if (this->scheduled_downtime_depth > 0) {
    logger(dbg_notifications, more)
        << "This service is currently in a scheduled downtime, so "
           "we won't send notifications.";
    return ERROR;
  }

  /*
   * if this host is currently in a scheduled downtime period, don't send the
   * notification
   * */
  if (temp_host->get_scheduled_downtime_depth() > 0) {
    logger(dbg_notifications, more)
        << "The host this service is associated with is currently in "
           "a scheduled downtime, so we won't send notifications.";
    return ERROR;
  }

  /*
   ***** RECOVERY NOTIFICATIONS ARE GOOD TO GO AT THIS POINT IF ANY OTHER *****
   ***** NOTIFICATION WAS SENT                                            *****
   */
  if (this->current_state == STATE_OK)
    return (this->current_notification_number > 0) ? OK : ERROR;

  /*
   * don't notify contacts about this service problem again if the notification
   * interval is set to 0
   */
  if (this->no_more_notifications == true) {
    logger(dbg_notifications, more)
        << "We shouldn't re-notify contacts about this service problem.";
    return ERROR;
  }

  /*
   * if the host is down or unreachable, don't notify contacts about service
   * failures
   */
  if (temp_host->get_current_state() != HOST_UP) {
    logger(dbg_notifications, more)
        << "The host is either down or unreachable, so we won't "
           "notify contacts about this service.";
    return ERROR;
  }

  /*
   * don't notify if we haven't waited long enough since the last time (and
   * the service is not marked as being volatile)
   */
  if (current_time < _next_notification && !this->is_volatile) {
    logger(dbg_notifications, more)
        << "We haven't waited long enough to re-notify contacts "
           "about this service.";
    logger(dbg_notifications, more) << "Next valid notification time: "
                                    << my_ctime(&_next_notification);
    return ERROR;
  }

  return OK;
}

/* checks viability of performing a service check */
int service::verify_check_viability(
      int check_options,
      int* time_is_valid,
      time_t* new_time) {
  int perform_check = true;
  time_t current_time = 0L;
  time_t preferred_time = 0L;
  int check_interval = 0;

  logger(dbg_functions, basic)
    << "check_service_check_viability()";

  /* get the check interval to use if we need to reschedule the check */
  if (this->state_type == SOFT_STATE && this->current_state != STATE_OK)
    check_interval = static_cast<int>(this->get_retry_interval() * config->interval_length());
  else
    check_interval = static_cast<int>(this->get_check_interval() * config->interval_length());

  /* get the current time */
  time(&current_time);

  /* initialize the next preferred check time */
  preferred_time = current_time;

  /* can we check the host right now? */
  if (!(check_options & CHECK_OPTION_FORCE_EXECUTION)) {

    /* if checks of the service are currently disabled... */
    if (!this->checks_enabled) {
      preferred_time = current_time + check_interval;
      perform_check = false;

      logger(dbg_checks, most)
        << "Active checks of the service are currently disabled.";
    }

    // Make sure this is a valid time to check the service.
    {
      timezone_locker lock(get_service_timezone(
                             this->get_hostname(),
                             this->get_description()));
      if (check_time_against_period(
            (unsigned long)current_time,
            this->check_period_ptr) == ERROR) {
        preferred_time = current_time;
        if (time_is_valid)
          *time_is_valid = false;
        perform_check = false;
        logger(dbg_checks, most)
          << "This is not a valid time for this service to be actively "
             "checked.";
      }
    }

    /* check service dependencies for execution */
    if (check_service_dependencies(
          this,
          hostdependency::execution) == DEPENDENCIES_FAILED) {
      preferred_time = current_time + check_interval;
      perform_check = false;

      logger(dbg_checks, most)
        << "Execution dependencies for this service failed, so it will "
        "not be actively checked.";
    }
  }

  /* pass back the next viable check time */
  if (new_time)
    *new_time = preferred_time;

  return (perform_check) ? OK : ERROR;
}

/*
 * given a service, create a list of contacts to be notified, removing
 * duplicates
 */
int service::create_notification_list(
      nagios_macros* mac,
      int options,
      bool* escalated) {
  int escalate_notification = false;

  logger(dbg_functions, basic)
    << "create_notification_list_from_service()";

  /* see if this notification should be escalated */
  escalate_notification = should_service_notification_be_escalated(this);

  /* set the escalation flag */
  *escalated = escalate_notification;

  /* make sure there aren't any leftover contacts */
  free_notification_list();

  /* set the escalation macro */
  string::setstr(mac->x[MACRO_NOTIFICATIONISESCALATED], escalate_notification);

  if (options & NOTIFICATION_OPTION_BROADCAST)
    logger(dbg_notifications, more)
      << "This notification will be BROADCAST to all "
      "(escalated and normal) contacts...";

  /* use escalated contacts for this notification */
  if (escalate_notification == true
      || (options & NOTIFICATION_OPTION_BROADCAST)) {

    logger(dbg_notifications, more)
      << "Adding contacts from service escalation(s) to "
      "notification list.";

    std::pair<std::string, std::string>
      id(std::make_pair(this->get_hostname(), this->get_description()));
    umultimap<std::pair<std::string, std::string>,
              std::shared_ptr<serviceescalation> > const&
      escalations(state::instance().serviceescalations());
    for (umultimap<std::pair<std::string, std::string>,
                   std::shared_ptr<serviceescalation> >::const_iterator
           it(escalations.find(id)), end(escalations.end());
         it != end && it->first == id;
         ++it) {
      serviceescalation* temp_se(&*it->second);

      /* skip this entry if it isn't appropriate */
      if (is_valid_escalation_for_service_notification(
            this,
            temp_se,
            options) == false)
        continue;

      logger(dbg_notifications, most)
        << "Adding individual contacts from service escalation(s) "
        "to notification list.";

      /* add all individual contacts for this escalation entry */
      for (contact_map::iterator
             it(temp_se->contacts.begin()),
             end(temp_se->contacts.end());
           it != end;
           ++it)
        add_notification(mac, it->second.get());

      logger(dbg_notifications, most)
        << "Adding members of contact groups from service escalation(s) "
        "to notification list.";

      /* add all contacts that belong to contactgroups for this escalation */
      for (contactgroup_map::iterator
             it(temp_se->contact_groups.begin()),
             end(temp_se->contact_groups.end());
           it != end;
           ++it) {
        logger(dbg_notifications, most)
          << "Adding members of contact group '"
          << it->first
          << "' for service escalation to notification list.";

        if (it->second == nullptr)
          continue;
        for (std::unordered_map<std::string, contact *>::const_iterator
               itm(it->second->get_members().begin()),
               mend(it->second->get_members().end());
              itm != mend;
              ++itm) {
          if (itm->second == nullptr)
            continue;
          add_notification(mac, itm->second);
        }
      }
    }
  }

  /* else use normal, non-escalated contacts */
  if (escalate_notification == false
      || (options & NOTIFICATION_OPTION_BROADCAST)) {

    logger(dbg_notifications, more)
      << "Adding normal contacts for service to notification list.";

    /* add all individual contacts for this service */
    for (contact_map::iterator
           it(this->contacts.begin()),
           end(this->contacts.end());
         it != end;
         ++it) {
      add_notification(mac, it->second.get());
    }

    /* add all contacts that belong to contactgroups for this service */
    for (contactgroup_map::iterator
           it(this->contact_groups.begin()),
           end(this->contact_groups.end());
         it != end;
         ++it) {
      logger(dbg_notifications, most)
        << "Adding members of contact group '"
        << it->first
        << "' for service to notification list.";

      if (it->second == nullptr)
        continue;
      for (std::unordered_map<std::string, contact *>::const_iterator
             itm(it->second->get_members().begin()),
             mend(it->second->get_members().end());
            itm != mend;
            ++itm) {
        if (itm->second == nullptr)
          continue;
        add_notification(mac, itm->second);
      }
    }
  }

  return OK;
}

void service::grab_macros_r(nagios_macros* mac) {
  grab_host_macros_r(mac, this->host_ptr);
  grab_service_macros_r(mac, this);
}

/* notify a specific contact about a service problem or recovery */
int service::notify_contact(
      nagios_macros* mac,
      contact* cntct,
      int type,
      char const* not_author,
      char const* not_data,
      int options,
      int escalated) {
  char* command_name_ptr(nullptr);
  char* raw_command = nullptr;
  char* processed_command = nullptr;
  int early_timeout = false;
  double exectime;
  struct timeval start_time, end_time;
  struct timeval method_start_time, method_end_time;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;
  int neb_result;

  logger(dbg_functions, basic)
    << "notify_contact_of_service()";
  logger(dbg_notifications, most)
    << "** Attempting to notifying contact '" << cntct->get_name() << "'...";

  /*
   * check viability of notifying this user
   * acknowledgements are no longer excluded from this test -
   * added 8/19/02 Tom Bertelson
   */
  if (check_contact_service_notification_viability(
        cntct,
        this,
        type,
        options) == ERROR)
    return ERROR;

  logger(dbg_notifications, most)
    << "** Notifying contact '" << cntct->get_name() << "'";

  /* get start time */
  gettimeofday(&start_time, nullptr);

  /* send data to event broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_contact_notification_data(
                 NEBTYPE_CONTACTNOTIFICATION_START,
                 NEBFLAG_NONE,
                 NEBATTR_NONE,
                 SERVICE_NOTIFICATION,
                 type,
                 start_time,
                 end_time,
                 (void*)this,
                 cntct,
                 not_author,
                 not_data,
                 escalated,
                 nullptr);
  if (NEBERROR_CALLBACKCANCEL == neb_result)
    return ERROR;
  else if (NEBERROR_CALLBACKOVERRIDE == neb_result)
    return OK;

  /* process all the notification commands this user has */
  for (std::shared_ptr<commands::command> const& cmd :
        cntct->get_service_notification_commands()) {
    /* get start time */
    gettimeofday(&method_start_time, nullptr);

    /* send data to event broker */
    method_end_time.tv_sec = 0L;
    method_end_time.tv_usec = 0L;
    neb_result = broker_contact_notification_method_data(
                   NEBTYPE_CONTACTNOTIFICATIONMETHOD_START,
                   NEBFLAG_NONE,
                   NEBATTR_NONE,
                   SERVICE_NOTIFICATION,
                   type,
                   method_start_time,
                   method_end_time,
                   (void*)this,
                   cntct,
                   cmd->get_command_line().c_str(),
                   not_author,
                   not_data,
                   escalated,
                   nullptr);
    if (NEBERROR_CALLBACKCANCEL == neb_result)
      break;
    else if (NEBERROR_CALLBACKOVERRIDE == neb_result)
      continue;

    /* get the raw command line */
    get_raw_command_line_r(
      mac,
      cmd.get(),
      cmd->get_command_line().c_str(),
      &raw_command,
      macro_options);
    if (raw_command == nullptr)
      continue;

    logger(dbg_notifications, most)
      << "Raw notification command: " << raw_command;

    /* process any macros contained in the argument */
    process_macros_r(
      mac,
      raw_command,
      &processed_command,
      macro_options);
    if (processed_command == nullptr)
      continue;

    /* run the notification command... */

    logger(dbg_notifications, most)
      << "Processed notification command: " << processed_command;

    /* log the notification to program log file */
    if (config->log_notifications() == true) {
      char const* service_state_str("UNKNOWN");
      if ((unsigned int)this->current_state < tab_service_states.size())
        service_state_str = tab_service_states[this->current_state].second.c_str();

      char const* notification_str("");
      if ((unsigned int)type < tab_notification_str.size())
        notification_str = tab_notification_str[type].c_str();

      std::string info;
      switch (type) {
      case NOTIFICATION_CUSTOM:
        notification_str = "CUSTOM";

      case NOTIFICATION_ACKNOWLEDGEMENT:
        info
          .append(";").append(not_author ? not_author : "")
          .append(";").append(not_data ? not_data : "");
        break;
      }

      std::string service_notification_state;
      if (strcmp(notification_str, "NORMAL") == 0)
        service_notification_state.append(service_state_str);
      else
        service_notification_state
          .append(notification_str)
          .append(" (")
          .append(service_state_str)
          .append(")");

      logger(log_service_notification, basic)
        << "SERVICE NOTIFICATION: " << cntct->get_name() << ';'
        << get_hostname() << ';' << get_description() << ';'
        << service_notification_state << ";"
        << cmd->get_name() << ';'
        << get_plugin_output()
        << info;
    }

    /* run the notification command */
    try {
      my_system_r(
        mac,
        processed_command,
        config->notification_timeout(),
        &early_timeout,
        &exectime,
        nullptr,
        0);
    } catch (std::exception const& e) {
      logger(log_runtime_error, basic)
        << "Error: can't execute service notification '"
        << cntct->get_name() << "' : " << e.what();
    }

    /* check to see if the notification command timed out */
    if (early_timeout == true) {
      logger(log_service_notification | log_runtime_warning, basic)
        << "Warning: Contact '" << cntct->get_name()
        << "' service notification command '" << processed_command
        << "' timed out after " << config->notification_timeout()
        << " seconds";
    }

    /* free memory */
    delete[] raw_command;
    delete[] processed_command;

    /* get end time */
    gettimeofday(&method_end_time, nullptr);

    /* send data to event broker */
    broker_contact_notification_method_data(
      NEBTYPE_CONTACTNOTIFICATIONMETHOD_END,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      SERVICE_NOTIFICATION,
      type,
      method_start_time,
      method_end_time,
      (void*)this,
      cntct,
      cmd->get_command_line().c_str(),
      not_author,
      not_data,
      escalated,
      nullptr);
  }

  /* get end time */
  gettimeofday(&end_time, nullptr);

  /* update the contact's last service notification time */
  cntct->set_last_service_notification(start_time.tv_sec);

  /* send data to event broker */
  broker_contact_notification_data(
    NEBTYPE_CONTACTNOTIFICATION_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    SERVICE_NOTIFICATION,
    type,
    start_time,
    end_time,
    (void*)this,
    cntct,
    not_author,
    not_data,
    escalated,
    nullptr);
  return OK;
}

void service::update_notification_flags() {
  /* update notifications flags */
  if (this->current_state == STATE_UNKNOWN)
    this->notified_on_unknown = true;
  else if (this->current_state == STATE_WARNING)
    this->notified_on_warning = true;
  else if (this->current_state == STATE_CRITICAL)
    this->notified_on_critical = true;
}

/* calculates next acceptable re-notification time for a service */
time_t service::get_next_notification_time(time_t offset) {
  time_t next_notification{0L};
  double interval_to_use{0.0};
  serviceescalation* temp_se{nullptr};
  bool have_escalated_interval{false};

  logger(dbg_functions, basic)
    << "service::get_next_notification_time()";
  logger(dbg_notifications, most)
    << "Calculating next valid notification time...";

  /* default notification interval */
  interval_to_use = get_notification_interval();

  logger(dbg_notifications, most)
    << "Default interval: " << interval_to_use;

  /*
   * search all the escalation entries for valid matches for this service (at
   * its current notification number)
   */
  for (temp_se = serviceescalation_list;
       temp_se != nullptr;
       temp_se = temp_se->next) {

    /* interval < 0 means to use non-escalated interval */
    if (temp_se->notification_interval < 0.0)
      continue;

    /* skip this entry if it isn't appropriate */
    if (is_valid_escalation_for_service_notification(
          this,
          temp_se,
          NOTIFICATION_OPTION_NONE) == false)
      continue;

    logger(dbg_notifications, most)
      << "Found a valid escalation w/ interval of "
      << temp_se->notification_interval;

    /*
     * if we haven't used a notification interval from an escalation yet,
     * use this one
     */
    if (have_escalated_interval == false) {
      have_escalated_interval = true;
      interval_to_use = temp_se->notification_interval;
    }

    /* else use the shortest of all valid escalation intervals */
    else if (temp_se->notification_interval < interval_to_use)
      interval_to_use = temp_se->notification_interval;

    logger(dbg_notifications, most)
      << "New interval: " << interval_to_use;
  }

  /*
   * if notification interval is 0, we shouldn't send any more problem
   * notifications (unless service is volatile)
   */
  if (interval_to_use == 0.0 && this->is_volatile == false)
    this->no_more_notifications = true;
  else
    this->no_more_notifications = false;

  logger(dbg_notifications, most)
    << "Interval used for calculating next valid "
    "notification time: " << interval_to_use;

  /* calculate next notification time */
  next_notification = offset + static_cast<time_t>(interval_to_use *
    config->interval_length());
  return next_notification;
}

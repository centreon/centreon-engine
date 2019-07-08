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

#include <cassert>
#include <iomanip>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/checks/viability_failure.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/hash_timed_event.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/hostdependency.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/macros/grab_host.hh"
#include "com/centreon/engine/macros/grab_service.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/objects.hh"
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

std::array<std::pair<uint32_t, std::string>, 4> const
    service::tab_service_states{{{NSLOG_SERVICE_OK, "OK"},
                                 {NSLOG_SERVICE_WARNING, "WARNING"},
                                 {NSLOG_SERVICE_CRITICAL, "CRITICAL"},
                                 {NSLOG_SERVICE_CRITICAL, "UNKNOWN"}}};

service_map service::services;
service_id_map service::services_by_id;

service::service(std::string const& hostname,
                 std::string const& description,
                 std::string const& display_name,
                 std::string const& check_command,
                 bool checks_enabled,
                 bool accept_passive_checks,
                 enum service::service_state initial_state,
                 double check_interval,
                 double retry_interval,
                 double notification_interval,
                 int max_attempts,
                 uint32_t first_notification_delay,
                 uint32_t recovery_notification_delay,
                 std::string const& notification_period,
                 bool notifications_enabled,
                 bool is_volatile,
                 std::string const& check_period,
                 std::string const& event_handler,
                 bool event_handler_enabled,
                 std::string const& notes,
                 std::string const& notes_url,
                 std::string const& action_url,
                 std::string const& icon_image,
                 std::string const& icon_image_alt,
                 bool flap_detection_enabled,
                 double low_flap_threshold,
                 double high_flap_threshold,
                 bool check_freshness,
                 int freshness_threshold,
                 bool obsess_over,
                 std::string const& timezone)
    : notifier{service_notification,
               display_name,
               check_command,
               checks_enabled,
               accept_passive_checks,
               check_interval,
               retry_interval,
               notification_interval,
               max_attempts,
               first_notification_delay,
               recovery_notification_delay,
               notification_period,
               notifications_enabled,
               check_period,
               event_handler,
               event_handler_enabled,
               notes,
               notes_url,
               action_url,
               icon_image,
               icon_image_alt,
               flap_detection_enabled,
               low_flap_threshold,
               high_flap_threshold,
               check_freshness,
               freshness_threshold,
               obsess_over,
               timezone},
      _hostname{hostname},
      _description{description},
      _is_volatile{is_volatile},
      _initial_state{initial_state},
      _current_state{initial_state},
      _last_hard_state{initial_state},
      _last_state{initial_state},
      _host_ptr{nullptr},
      _host_problem_at_last_check{false} {
  set_current_attempt(initial_state == service::state_ok ? 1 : max_attempts);
}

service::~service() {}

time_t service::get_last_time_ok() const {
  return _last_time_ok;
}

void service::set_last_time_ok(time_t last_time) {
  _last_time_ok = last_time;
}

time_t service::get_last_time_warning() const {
  return _last_time_warning;
}

void service::set_last_time_warning(time_t last_time) {
  _last_time_warning = last_time;
}

time_t service::get_last_time_unknown() const {
  return _last_time_unknown;
}

void service::set_last_time_unknown(time_t last_time) {
  _last_time_unknown = last_time;
}

time_t service::get_last_time_critical() const {
  return _last_time_critical;
}

void service::set_last_time_critical(time_t last_time) {
  _last_time_critical = last_time;
}

enum service::service_state service::get_current_state() const {
  return _current_state;
}

void service::set_current_state(enum service::service_state current_state) {
  _current_state = current_state;
}

enum service::service_state service::get_last_state() const {
  return _last_state;
}

void service::set_last_state(enum service::service_state last_state) {
  _last_state = last_state;
}

enum service::service_state service::get_last_hard_state() const {
  return _last_hard_state;
}

void service::set_last_hard_state(enum service::service_state last_hard_state) {
  _last_hard_state = last_hard_state;
}

enum service::service_state service::get_initial_state() const {
  return _initial_state;
}

void service::set_initial_state(enum service::service_state current_state) {
  _initial_state = current_state;
}

bool service::recovered() const {
  return _current_state == service::state_ok;
}

int service::get_current_state_int() const {
  return static_cast<int>(_current_state);
}

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool service::operator==(service const& other) {
  return get_hostname() == other.get_hostname() &&
         get_description() == other.get_description() &&
         get_display_name() == other.get_display_name() &&
         get_check_command() == other.get_check_command() &&
         get_event_handler() == other.get_event_handler() &&
         get_initial_state() == other.get_initial_state() &&
         get_check_interval() == other.get_check_interval() &&
         get_retry_interval() == other.get_retry_interval() &&
         get_max_attempts() == other.get_max_attempts() &&
         (get_contactgroups().size() == other.get_contactgroups().size() &&
          std::equal(get_contactgroups().begin(), get_contactgroups().end(),
                     other.get_contactgroups().begin())) &&
         (get_contacts().size() == other.get_contacts().size() &&
          std::equal(get_contacts().begin(), get_contacts().end(),
                     other.get_contacts().begin())) &&
         this->get_notification_interval() == other.get_notification_interval() &&
         get_first_notification_delay() == get_first_notification_delay() &&
         get_recovery_notification_delay() == get_recovery_notification_delay() &&
         get_notify_on() == other.get_notify_on() &&
         get_stalk_on() == other.get_stalk_on() &&
         get_is_volatile() == other.get_is_volatile() &&
         get_notification_period() == other.get_notification_period() &&
         get_check_period() == other.get_check_period() &&
         get_flap_detection_enabled() ==
             other.get_flap_detection_enabled() &&
         get_low_flap_threshold() == other.get_low_flap_threshold() &&
         get_high_flap_threshold() == other.get_high_flap_threshold() &&
         _flap_type == other.get_flap_detection_on() &&
         this->process_performance_data == other.process_performance_data &&
         get_check_freshness() == other.get_check_freshness() &&
         get_freshness_threshold() == other.get_freshness_threshold() &&
         get_accept_passive_checks() ==
             other.get_accept_passive_checks() &&
         get_event_handler_enabled() == other.get_event_handler_enabled() &&
         get_checks_enabled() == other.get_checks_enabled() &&
         this->retain_status_information == other.retain_status_information &&
         this->retain_nonstatus_information ==
             other.retain_nonstatus_information &&
         get_notifications_enabled() == other.get_notifications_enabled() &&
         get_obsess_over() == other.get_obsess_over() &&
         get_notes() == other.get_notes() &&
         get_notes_url() == other.get_notes_url() &&
         get_action_url() == other.get_action_url() &&
         get_icon_image() == other.get_icon_image() &&
         get_icon_image_alt() == other.get_icon_image_alt() &&
         this->custom_variables == other.custom_variables &&
         get_problem_has_been_acknowledged() ==
             other.get_problem_has_been_acknowledged() &&
         this->acknowledgement_type == other.acknowledgement_type &&
         _host_problem_at_last_check == other._host_problem_at_last_check &&
         get_check_type() == other.get_check_type() &&
         _current_state == other.get_current_state() &&
         _last_state == other.get_last_state() &&
         _last_hard_state == other.get_last_hard_state() &&
         get_plugin_output() == other.get_plugin_output() &&
         get_long_plugin_output() == other.get_long_plugin_output() &&
         get_perf_data() == other.get_perf_data() &&
         get_state_type() == other.get_state_type() &&
         get_next_check() == other.get_next_check() &&
         get_should_be_scheduled() == other.get_should_be_scheduled() &&
         get_last_check() == other.get_last_check() &&
         get_current_attempt() == other.get_current_attempt() &&
         _current_event_id == other.get_current_event_id() &&
         _last_event_id == other.get_last_event_id() &&
         _current_problem_id == other.get_current_problem_id() &&
         _last_problem_id == other.get_last_problem_id() &&
         get_last_notification() == other.get_last_notification() &&
         get_next_notification() == other.get_next_notification() &&
         get_no_more_notifications() == other.get_no_more_notifications() &&
         get_last_state_change() == other.get_last_state_change() &&
         get_last_hard_state_change() == other.get_last_hard_state_change() &&
         _last_time_ok == other.get_last_time_ok() &&
         _last_time_warning == other.get_last_time_warning() &&
         _last_time_unknown == other.get_last_time_unknown() &&
         _last_time_critical == other.get_last_time_critical() &&
         get_has_been_checked() == other.get_has_been_checked() &&
         this->is_being_freshened == other.is_being_freshened &&
         get_notified_on() == other.get_notified_on() &&
         get_notification_number() == other.get_notification_number() &&
         this->current_notification_id == other.current_notification_id &&
         get_latency() == other.get_latency() &&
         get_execution_time() == other.get_execution_time() &&
         get_is_executing() == other.get_is_executing() &&
         this->check_options == other.check_options &&
         get_scheduled_downtime_depth() == other.get_scheduled_downtime_depth() &&
         get_pending_flex_downtime() == other.get_pending_flex_downtime() &&
         std::equal(get_state_history().begin(), get_state_history().end(), other.get_state_history().begin()) &&
         get_state_history_index() == other.get_state_history_index() &&
         get_is_flapping() == other.get_is_flapping() &&
         this->flapping_comment_id == other.flapping_comment_id &&
         get_percent_state_change() == other.get_percent_state_change() &&
         _modified_attributes == other._modified_attributes &&
         _event_handler_args == other.get_event_handler_args() &&
         _check_command_args == other.get_check_command_args();
}

/**
 *  Not equal operator.
 *
 *  @param[in] other The first object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool service::operator!=(service const& other) throw() {
  return !operator==(other);
}

/**
 *  Dump a service_map content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The service_map to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, service_map_unsafe const& obj) {
  for (service_map_unsafe::const_iterator it(obj.begin()), end(obj.end());
       it != end; ++it)
    os << "(" << it->first.first << ", " << it->first.second
       << (std::next(it) != obj.end() ? "), " : ")");
  return os;
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
  if (obj.get_event_handler_ptr())
    evt_str = obj.get_event_handler_ptr()->get_name();
  std::string cmd_str;
  if (obj.get_check_command_ptr())
    cmd_str = obj.get_check_command_ptr()->get_name();
  std::string chk_period_str;
  if (obj.check_period_ptr)
    chk_period_str = obj.check_period_ptr->get_name();
  std::string notif_period_str;
  if (obj.notification_period_ptr)
    notif_period_str = obj.notification_period_ptr->get_name();
  std::string svcgrp_str;
  if (!obj.get_parent_groups().empty())
    svcgrp_str = obj.get_parent_groups().front()->get_group_name();

  std::string cg_oss;
  std::string c_oss;

  if (obj.get_contactgroups().empty())
    cg_oss = "\"nullptr\"";
  else {
    std::ostringstream oss;
    oss << obj.get_contactgroups();
    cg_oss = oss.str();
  }
  if (obj.get_contacts().empty())
    c_oss = "\"nullptr\"";
  else {
    std::ostringstream oss;
    oss << obj.get_contacts();
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
        "  contact_groups:                       "
     << cg_oss
     << "\n"
        "  contacts:                             "
     << c_oss
     << "\n"
        "  notification_interval:                "
     << obj.get_notification_interval()
     << "\n"
        "  first_notification_delay:             "
     << obj.get_first_notification_delay()
     << "\n"
        "  recovery_notification_delay:          "
     << obj.get_recovery_notification_delay()
     << "\n"
        "  notify_on_unknown:                    "
     << obj.get_notify_on(notifier::unknown)
     << "\n"
        "  notify_on_warning:                    "
     << obj.get_notify_on(notifier::warning)
     << "\n"
        "  notify_on_critical:                   "
     << obj.get_notify_on(notifier::critical)
     << "\n"
        "  notify_on_recovery:                   "
     << obj.get_notify_on(notifier::recovery)
     << "\n"
        "  notify_on_flappingstart:              "
     << obj.get_notify_on(notifier::flappingstart)
     << "\n"
        "  notify_on_flappingstop:               "
     << obj.get_notify_on(notifier::flappingstop)
     << "\n"
        "  notify_on_flappingdisabled:           "
     << obj.get_notify_on(notifier::flappingdisabled)
     << "\n"
        "  notify_on_downtime:                   "
     << obj.get_notify_on(notifier::downtime)
     << "\n"
        "  stalk_on_ok:                          "
     << obj.get_stalk_on(notifier::ok)
     << "\n"
        "  stalk_on_warning:                     "
     << obj.get_stalk_on(notifier::warning)
     << "\n"
        "  stalk_on_unknown:                     "
     << obj.get_stalk_on(notifier::unknown)
     << "\n"
        "  stalk_on_critical:                    "
     << obj.get_stalk_on(notifier::critical)
     << "\n"
        "  is_volatile:                          "
     << obj.get_is_volatile()
     << "\n"
        "  notification_period:                  "
     << obj.get_notification_period()
     << "\n"
        "  check_period:                         "
     << obj.get_check_period()
     << "\n"
        "  flap_detection_enabled:               "
     << obj.get_flap_detection_enabled()
     << "\n"
        "  low_flap_threshold:                   "
     << obj.get_low_flap_threshold()
     << "\n"
        "  high_flap_threshold:                  "
     << obj.get_high_flap_threshold()
     << "\n"
        "  flap_detection_on_ok:                 "
     << obj.get_flap_detection_on(notifier::ok)
     << "\n"
        "  flap_detection_on_warning:            "
     << obj.get_flap_detection_on(notifier::warning)
     << "\n"
        "  flap_detection_on_unknown:            "
     << obj.get_flap_detection_on(notifier::unknown)
     << "\n"
        "  flap_detection_on_critical:           "
     << obj.get_flap_detection_on(notifier::critical)
     << "\n"
        "  process_performance_data:             "
     << obj.process_performance_data
     << "\n"
        "  check_freshness:                      "
     << obj.get_check_freshness()
     << "\n"
        "  freshness_threshold:                  "
     << obj.get_freshness_threshold()
     << "\n"
        "  accept_passive_service_checks:        "
     << obj.get_accept_passive_checks()
     << "\n  event_handler_enabled:                "
     << obj.get_event_handler_enabled()
     << "\n  checks_enabled:                       " << obj.get_checks_enabled()
     << "\n  retain_status_information:            "
     << obj.retain_status_information
     << "\n  retain_nonstatus_information:         "
     << obj.retain_nonstatus_information
     << "\n  notifications_enabled:                "
     << obj.get_notifications_enabled()
     << "\n  obsess_over_service:                  " << obj.get_obsess_over()
     << "\n  notes:                                " << obj.get_notes()
     << "\n  notes_url:                            " << obj.get_notes_url()
     << "\n  action_url:                           " << obj.get_action_url()
     << "\n  icon_image:                           " << obj.get_icon_image()
     << "\n  icon_image_alt:                       " << obj.get_icon_image_alt()
     << "\n  problem_has_been_acknowledged:        "
     << obj.get_problem_has_been_acknowledged()
     << "\n  acknowledgement_type:                 " << obj.acknowledgement_type
     << "\n  host_problem_at_last_check:           "
     << obj.get_host_problem_at_last_check()
     << "\n  check_type:                           " << obj.get_check_type()
     << "\n  current_state:                        " << obj.get_current_state()
     << "\n  last_state:                           " << obj.get_last_state()
     << "\n  last_hard_state:                      " << obj.get_last_hard_state()
     << "\n  plugin_output:                        " << obj.get_plugin_output()
     << "\n  long_plugin_output:                   "
     << obj.get_long_plugin_output()
     << "\n  perf_data:                            " << obj.get_perf_data()
     << "\n  state_type:                           " << obj.get_state_type()
     << "\n  next_check:                           "
     << string::ctime(obj.get_next_check())
     << "\n  should_be_scheduled:                  " << obj.get_should_be_scheduled()
     << "\n  last_check:                           "
     << string::ctime(obj.get_last_check())
     << "\n  current_attempt:                      "
     << obj.get_current_attempt()
     << "\n  current_event_id:                     " << obj.get_current_event_id()
     << "\n  last_event_id:                        " << obj.get_last_event_id()
     << "\n  current_problem_id:                   " << obj.get_current_problem_id()
     << "\n  last_problem_id:                      " << obj.get_last_problem_id()
     << "\n  last_notification:                    "
     << string::ctime(obj.get_last_notification())
     << "\n  next_notification:                    "
     << string::ctime(obj.get_next_notification())
     << "\n  no_more_notifications:                "
     << obj.get_no_more_notifications()
     << "\n  last_state_change:                    "
     << string::ctime(obj.get_last_state_change())
     << "\n  last_hard_state_change:               "
     << string::ctime(obj.get_last_hard_state_change())
     << "\n  last_time_ok:                         "
     << string::ctime(obj.get_last_time_ok())
     << "\n  last_time_warning:                    "
     << string::ctime(obj.get_last_time_warning())
     << "\n  last_time_unknown:                    "
     << string::ctime(obj.get_last_time_unknown())
     << "\n  last_time_critical:                   "
     << string::ctime(obj.get_last_time_critical())
     << "\n  has_been_checked:                     " << obj.get_has_been_checked()
     << "\n  is_being_freshened:                   " << obj.is_being_freshened
     << "\n  notified_on_unknown:                  "
     << obj.get_notified_on(notifier::unknown)
     << "\n  notified_on_warning:                  "
     << obj.get_notified_on(notifier::warning)
     << "\n  notified_on_critical:                 "
     << obj.get_notified_on(notifier::critical)
     << "\n  current_notification_number:          "
     << obj.get_notification_number()
     << "\n  current_notification_id:              "
     << obj.current_notification_id
     << "\n  latency:                              " << obj.get_latency()
     << "\n  execution_time:                       " << obj.get_execution_time()
     << "\n  is_executing:                         " << obj.get_is_executing()
     << "\n  check_options:                        " << obj.check_options
     << "\n  scheduled_downtime_depth:             "
     << obj.get_scheduled_downtime_depth()
     << "\n  pending_flex_downtime:                "
     << obj.get_pending_flex_downtime() << "\n";

  os << "  state_history:                        ";
  for (size_t i{0}, end{obj.get_state_history().size()}; i < end; ++i)
    os << obj.get_state_history()[i] << (i + 1 < end ? ", " : "\n");

  os << "  state_history_index:                  " << obj.get_state_history_index()
     << "\n  is_flapping:                          " << obj.get_is_flapping()
     << "\n  flapping_comment_id:                  " << obj.flapping_comment_id
     << "\n  percent_state_change:                 " << obj.get_percent_state_change()
     << "\n  modified_attributes:                  "
     << obj.get_modified_attributes()
     << "\n  host_ptr:                             "
     << (obj.get_host_ptr() ? obj.get_host_ptr()->get_name() : "\"nullptr\"")
     << "\n  event_handler_ptr:                    " << evt_str
     << "\n  event_handler_args:                   "
     << obj.get_event_handler_args()
     << "\n  check_command_ptr:                    " << cmd_str
     << "\n  check_command_args:                   "
     << obj.get_check_command_args()
     << "\n  check_period_ptr:                     " << chk_period_str
     << "\n  notification_period_ptr:              " << notif_period_str
     << "\n  servicegroups_ptr:                    " << svcgrp_str << "\n";

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
 *  @param[in] accept_passive_checks        Does this service accept
 *                                          check result submission ?
 *  @param[in] check_interval               Normal check interval.
 *  @param[in] retry_interval               Retry check interval.
 *  @param[in] notification_interval        Notification interval.
 *  @param[in] first_notification_delay     First notification delay.
 *  @param[in] recovery_notification_delay  Recovery notification delay.
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
    com::centreon::engine::service::service_state initial_state,
    int max_attempts,
    double check_interval,
    double retry_interval,
    double notification_interval,
    uint32_t first_notification_delay,
    uint32_t recovery_notification_delay,
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
    bool accept_passive_checks,
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
    bool check_freshness,
    int freshness_threshold,
    std::string const& notes,
    std::string const& notes_url,
    std::string const& action_url,
    std::string const& icon_image,
    std::string const& icon_image_alt,
    int retain_status_information,
    int retain_nonstatus_information,
    bool obsess_over_service,
    std::string const& timezone) {
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
      check_command, checks_enabled, accept_passive_checks, initial_state, check_interval,
      retry_interval, notification_interval, max_attempts, first_notification_delay, recovery_notification_delay,
      notification_period, notifications_enabled, is_volatile, check_period, event_handler, event_handler_enabled,
      notes, notes_url, action_url, icon_image, icon_image_alt,
      flap_detection_enabled, low_flap_threshold, high_flap_threshold,
      check_freshness, freshness_threshold, obsess_over_service, timezone)};
  try {
    obj->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
    obj->check_options = CHECK_OPTION_NONE;
    uint32_t flap_detection_on;
    flap_detection_on = none;
    flap_detection_on |= (flap_detection_on_critical > 0 ? notifier::critical : 0);
    flap_detection_on |= (flap_detection_on_ok > 0 ? notifier::ok : 0);
    flap_detection_on |= (flap_detection_on_unknown > 0 ? notifier::unknown : 0);
    flap_detection_on |= (flap_detection_on_warning > 0 ? notifier::warning : 0);
    obj->set_flap_detection_on(flap_detection_on);
    obj->set_modified_attributes(MODATTR_NONE);
    uint32_t notify_on;
    notify_on = none;
    notify_on |= (notify_critical > 0 ? notifier::critical : 0);
    notify_on |= (notify_downtime > 0 ? notifier::downtime : 0);
    notify_on |= (notify_flapping > 0
                      ? (notifier::flappingstart | notifier::flappingstop |
                         notifier::flappingdisabled)
                      : 0);
    notify_on |= (notify_recovery > 0 ? notifier::recovery : 0);
    notify_on |= (notify_unknown > 0 ? notifier::unknown : 0);
    notify_on |= (notify_warning > 0 ? notifier::warning : 0);
    obj->set_notify_on(notify_on);
    obj->process_performance_data = (process_perfdata > 0);
    obj->retain_nonstatus_information = (retain_nonstatus_information > 0);
    obj->retain_status_information = (retain_status_information > 0);
    obj->set_should_be_scheduled(true);
    uint32_t stalk_on;
    stalk_on = none;
    stalk_on |= (stalk_on_critical > 0 ? notifier::critical : 0);
    stalk_on |= (stalk_on_ok > 0 ? notifier::ok : 0);
    stalk_on |= (stalk_on_unknown > 0 ? notifier::warning: 0);
    stalk_on |= (stalk_on_warning > 0 ? notifier::unknown : 0);
    obj->set_stalk_on(stalk_on);
    obj->set_state_type(notifier::hard);

    // state_ok = 0, so we don't need to set state_history (memset
    // is used before).
    // for (unsigned int x(0); x < MAX_STATE_HISTORY_ENTRIES; ++x)
    //   obj->state_history[x] = state_ok;

    // Add new items to the list.
    service::services[{obj->get_hostname(), obj->get_description()}] = obj;
    service::services_by_id[{host_id, service_id}] = obj;
  } catch (...) {
    obj.reset();
  }

  return obj.get();
}

/**
 *  Check if acknowledgement on service expired.
 *
 */
void service::check_for_expired_acknowledgement() {
  if (get_problem_has_been_acknowledged()) {
    if (_acknowledgement_timeout > 0) {
      time_t now(time(nullptr));
      if (_last_acknowledgement + _acknowledgement_timeout >= now) {
        logger(log_info_message, basic)
            << "Acknowledgement of service '" << get_description()
            << "' on host '" << this->get_host_ptr()->get_name() << "' just expired";
        set_problem_has_been_acknowledged(false);
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
  service_id_map::const_iterator it(service::services_by_id.find(
    {host_id, service_id}));
  if (it == service::services_by_id.end())
    throw(engine_error() << "Service '" << service_id << "' on host '"
                         << host_id << "' was not found");
  return *it->second;
}

/**
 *  Get if service exist.
 *
 *  @param[in] id The service id.
 *
 *  @return True if the service is found, otherwise false.
 */
bool engine::is_service_exist(std::pair<uint64_t, uint64_t> const& id) {
  service_id_map::const_iterator it(service::services_by_id.find(id));
  return it != service::services_by_id.end();
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
  service_map::const_iterator found = service::services.find({host, svc});
  return found != service::services.end()
             ? std::pair<uint64_t, uint64_t>{found->second->get_host_id(),
                                             found->second->get_service_id()}
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

void service::set_host_id(uint64_t host_id) {
  _host_id = host_id;
}

uint64_t service::get_host_id() const {
  return _host_id;
}

void service::set_service_id(uint64_t service_id) {
  _service_id = service_id;
}

uint64_t service::get_service_id() const {
  return _service_id;
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

/**
 * @brief set the event handler arguments
 *
 *  @param[in] event_hdl_args the event handler arguments
 */
void service::set_event_handler_args(std::string const& event_hdl_args) {
  _event_handler_args = event_hdl_args;
}

/**
 * @brief Get the event handler arguments of the service.
 *
 * @return A string reference to the event handler arguments.
 */
std::string const& service::get_event_handler_args() const {
  return _event_handler_args;
}

/**
 * @brief set the command arguments
 *
 *  @param[in] cmd_args the command arguments
 */
void service::set_check_command_args(std::string const& cmd_args) {
  _check_command_args = cmd_args;
}

/**
 * @brief Get the event command arguments of the service.
 *
 * @return A string reference to the command arguments.
 */
std::string const& service::get_check_command_args() const {
  return _check_command_args;
}

int service::handle_async_check_result(check_result* queued_check_result) {
  time_t next_service_check = 0L;
  time_t preferred_time = 0L;
  time_t next_valid_time = 0L;
  int reschedule_check = false;
  int state_change = false;
  int hard_state_change = false;
  int first_host_check_initiated = false;
  host::host_state route_result = host::state_up;
  time_t current_time = 0L;
  int state_was_logged = false;
  std::string old_plugin_output;
  std::list<service*> check_servicelist;
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
      << "** Handling check result for service '" << _description
      << "' on host '" << _hostname << "'...";
  logger(dbg_checks, more)
      << "HOST: " << _hostname
      << ", SERVICE: " << _description << ", CHECK TYPE: "
      << (queued_check_result->get_check_type() == check_active ? "Active"
                                                          : "Passive")
      << ", OPTIONS: " << queued_check_result->get_check_options()
      << ", RESCHEDULE: "
      << (queued_check_result->get_reschedule_check() ? "Yes" : "No")
      << ", EXITED OK: " << (queued_check_result->get_exited_ok() ? "Yes" : "No")
      << ", return CODE: " << queued_check_result->get_return_code()
      << ", OUTPUT: " << queued_check_result->get_output();

  /* decrement the number of service checks still out there... */
  if (queued_check_result->get_check_type() == check_active &&
      currently_running_service_checks > 0)
    currently_running_service_checks--;

  /*
   * skip this service check results if its passive and we aren't accepting
   * passive check results
   */
  if (queued_check_result->get_check_type() == check_passive) {
    if (!config->accept_passive_service_checks()) {
      logger(dbg_checks, basic)
          << "Discarding passive service check result because passive "
             "service checks are disabled globally.";
      return ERROR;
    }
    if (!get_accept_passive_checks()) {
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
  if (queued_check_result->get_check_options() & CHECK_OPTION_FRESHNESS_CHECK)
    this->is_being_freshened = false;

  /* clear the execution flag if this was an active check */
  if (queued_check_result->get_check_type() == check_active)
    set_is_executing(false);

  /* DISCARD INVALID FRESHNESS CHECK RESULTS */
  /* If a services goes stale, Engine will initiate a forced check in
  ** order to freshen it.  There is a race condition whereby a passive
  ** check could arrive between the 1) initiation of the forced check
  ** and 2) the time when the forced check result is processed here.
  ** This would make the service fresh again, so we do a quick check to
  ** make sure the service is still stale before we accept the check
  ** result.
  */
  if ((queued_check_result->get_check_options() & CHECK_OPTION_FRESHNESS_CHECK) &&
      is_result_fresh(current_time, false)) {
    logger(dbg_checks, basic)
        << "Discarding service freshness check result because the service "
           "is currently fresh (race condition avoided).";
    return OK;
  }

  /* check latency is passed to us */
  set_latency(queued_check_result->get_latency());

  /* update the execution time for this check (millisecond resolution) */
  set_execution_time(
      (double)((double)(queued_check_result->get_finish_time().tv_sec -
                        queued_check_result->get_start_time().tv_sec) +
               (double)((queued_check_result->get_finish_time().tv_usec -
                         queued_check_result->get_start_time().tv_usec) /
                        1000.0) /
                   1000.0));
  if (get_execution_time() < 0.0)
    set_execution_time(0.0);

  /* get the last check time */
  set_last_check(queued_check_result->get_start_time().tv_sec);

  /* was this check passive or active? */
  set_check_type((queued_check_result->get_check_type() == check_active)
                     ? check_active
                     : check_passive);

  /* update check statistics for passive checks */
  if (queued_check_result->get_check_type() == check_passive)
    update_check_stats(PASSIVE_SERVICE_CHECK_STATS,
                       queued_check_result->get_start_time().tv_sec);

  /*
   * should we reschedule the next service check? NOTE: This may be overridden
   * later...
   */
  reschedule_check = queued_check_result->get_reschedule_check();

  /* save the old service status info */
  _last_state = _current_state;

  /* save old plugin output */
  old_plugin_output = get_plugin_output();

  /*
   * if there was some error running the command, just skip it (this
   * shouldn't be happening)
   */
  if (!queued_check_result->get_exited_ok()) {
    logger(log_runtime_warning, basic)
        << "Warning:  Check of service '" << _description
        << "' on host '" << _hostname << "' did not exit properly!";

    set_plugin_output("(Service check did not exit properly)");
    _current_state = service::state_unknown;
  }

  /* make sure the return code is within bounds */
  else if (queued_check_result->get_return_code() < 0 ||
           queued_check_result->get_return_code() > 3) {
    logger(log_runtime_warning, basic)
        << "Warning: return (code of " << queued_check_result->get_return_code()
        << " for check of service '" << _description << "' on host '"
        << _hostname << "' was out of bounds."
        << (queued_check_result->get_return_code() == 126
                ? "Make sure the plugin you're trying to run is executable."
                : (queued_check_result->get_return_code() == 127
                       ? " Make sure the plugin you're trying to run actually "
                         "exists."
                       : ""));

    std::ostringstream oss;
    oss << "(Return code of " << queued_check_result->get_return_code()
        << " is out of bounds"
        << (queued_check_result->get_return_code() == 126
                ? " - plugin may not be executable"
                : (queued_check_result->get_return_code() == 127
                       ? " - plugin may be missing"
                       : ""))
        << ')';

    set_plugin_output(oss.str());
    _current_state = service::state_unknown;
  }

  /* else the return code is okay... */
  else {
    /*
     * parse check output to get: (1) short output, (2) long output,
     * (3) perf data
     */
    std::string output{queued_check_result->get_output()};
    std::string plugin_output;
    std::string long_plugin_output;
    std::string perf_data;
    parse_check_output(output, plugin_output, long_plugin_output, perf_data,
                       true, true);

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
        << (get_plugin_output().empty() ? "NULL" : get_plugin_output())
        << "\n"
        << "Long Output:\n"
        << (get_long_plugin_output().empty() ? "NULL"
                                             : get_long_plugin_output())
        << "\n"
        << "Perf Data:\n"
        << (get_perf_data().empty() ? "NULL" : get_perf_data());

    /* grab the return code */
    _current_state = static_cast<service::service_state>(queued_check_result->get_return_code());
  }

  /* record the last state time */
  switch (_current_state) {
    case service::state_ok:
      set_last_time_ok(get_last_check());
      break;

    case service::state_warning:
      set_last_time_warning(get_last_check());
      break;

    case service::state_unknown:
      set_last_time_unknown(get_last_check());
      break;

    case service::state_critical:
      set_last_time_critical(get_last_check());
      break;

    default:
      break;
  }

  /*
   * log passive checks - we need to do this here, as some my bypass external
   * commands by getting dropped in checkresults dir
   */
  if (get_check_type() == check_passive) {
    if (config->log_passive_checks())
      logger(log_passive_check, basic)
          << "PASSIVE SERVICE CHECK: " << _hostname << ";"
          << _description << ";" << _current_state << ";"
          << get_plugin_output();
  }

  host* hst{get_host_ptr()};
  /* if the service check was okay... */
  if (_current_state == service::state_ok) {
    /* if the host has never been checked before, verify its status
     * only do this if 1) the initial state was set to non-UP or 2) the host
     * is not scheduled to be checked soon (next 5 minutes)
     */
    if (!hst->get_has_been_checked() &&
        (hst->get_initial_state() !=  host::state_up ||
         (unsigned long)hst->get_next_check() == 0L ||
         (unsigned long)(hst->get_next_check() - current_time) > 300)) {
      /* set a flag to remember that we launched a check */
      first_host_check_initiated = true;

      /* 08/04/07 EG launch an async (parallel) host check unless
       * aggressive host checking is enabled
       * previous logic was to simply run a sync (serial) host check
       * do NOT allow cached check results to happen here - we need the host
       * to be checked for real...
       * */
      if (config->use_aggressive_host_checking())
        hst->perform_on_demand_check(nullptr, CHECK_OPTION_NONE,
                                     false, 0L);
      else
        hst->run_async_check(CHECK_OPTION_NONE, 0.0, false, false,
                                   nullptr, nullptr);
    }
  }

  /*
   **** NOTE - THIS WAS MOVED UP FROM LINE 1049 BELOW TO FIX PROBLEMS ****
   **** WHERE CURRENT ATTEMPT VALUE WAS ACTUALLY "LEADING" REAL VALUE ****
   * increment the current attempt number if this is a soft state
   * (service was rechecked)
   */
  if (get_state_type() == soft &&
      get_current_attempt() < get_max_attempts())
    add_current_attempt(1);

  logger(dbg_checks, most) << "ST: "
                           << (get_state_type() == soft ? "SOFT" : "HARD")
                           << "  CA: " << get_current_attempt()
                           << "  MA: " << get_max_attempts()
                           << "  CS: " << _current_state
                           << "  LS: " << _last_state
                           << "  LHS: " << _last_hard_state;

  /* check for a state change (either soft or hard) */
  if (_current_state != _last_state) {
    logger(dbg_checks, most) << "Service has changed state since last check!";
    state_change = true;
  }

  /*
   * checks for a hard state change where host was down at last service
   * check this occurs in the case where host goes down and service current
   * attempt gets reset to 1 if this check is not made, the service recovery
   * looks like a soft recovery instead of a hard one
   */
  if (_host_problem_at_last_check && _current_state == service::state_ok) {
    logger(dbg_checks, most) << "Service had a HARD STATE CHANGE!!";
    hard_state_change = true;
  }

  /*
   * check for a "normal" hard state change where max check attempts is
   * reached
   */
  if (get_current_attempt() >= get_max_attempts() &&
      _current_state != _last_hard_state) {
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
    set_no_more_notifications(false);

    if (ACKNOWLEDGEMENT_NORMAL == this->acknowledgement_type &&
        (state_change || !hard_state_change)) {
      set_problem_has_been_acknowledged(false);
      this->acknowledgement_type = ACKNOWLEDGEMENT_NONE;

      /* remove any non-persistant comments associated with the ack */
      comment::delete_service_acknowledgement_comments(this);
    } else if (this->acknowledgement_type == ACKNOWLEDGEMENT_STICKY &&
               _current_state == service::state_ok) {
      set_problem_has_been_acknowledged(false);
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
  if (get_last_state_change() == (time_t)0)
    set_last_state_change(get_last_check());
  if (get_last_hard_state_change() == (time_t)0)
    set_last_hard_state_change(get_last_check());
  if (hst->get_last_state_change() == (time_t)0)
    hst->set_last_state_change(get_last_check());
  if (hst->get_last_hard_state_change() == (time_t)0)
    hst->set_last_hard_state_change(get_last_check());

  /* update last service state change times */
  if (state_change)
    set_last_state_change(get_last_check());
  if (hard_state_change)
    set_last_hard_state_change(get_last_check());

  /* update the event and problem ids */
  if (state_change) {
    /* always update the event id on a state change */
    _last_event_id = _current_event_id;
    _current_event_id = next_event_id;
    next_event_id++;

    /* update the problem id when transitioning to a problem state */
    if (_last_state == service::state_ok) {
      /* don't reset last problem id, or it will be zero the next time a problem
       * is encountered */
      /* this->last_problem_id=this->current_problem_id; */
      set_current_problem_id(next_problem_id);
      next_problem_id++;
    }

    /* clear the problem id when transitioning from a problem state to an OK
     * state */
    if (_current_state == service::state_ok) {
      _last_problem_id = _current_problem_id;
      _current_problem_id = 0L;
    }
  }

  /**************************************/
  /******* SERVICE CHECK OK LOGIC *******/
  /**************************************/

  /* if the service is up and running OK... */
  if (_current_state == service::state_ok) {
    logger(dbg_checks, more) << "Service is OK.";

    /* reset the acknowledgement flag (this should already have been done, but
     * just in case...) */
    set_problem_has_been_acknowledged(false);
    this->acknowledgement_type = ACKNOWLEDGEMENT_NONE;

    /* verify the route to the host and send out host recovery notifications */
    if (hst->get_current_state() !=  host::state_up) {
      logger(dbg_checks, more)
          << "Host is NOT UP, so we'll check it to see if it recovered...";

      /* 08/04/07 EG launch an async (parallel) host check (possibly cached)
       * unless aggressive host checking is enabled */
      /* previous logic was to simply run a sync (serial) host check */
      if (config->use_aggressive_host_checking())
        hst->perform_on_demand_check(nullptr, CHECK_OPTION_NONE,
                                     true, config->cached_host_check_horizon());
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
            hst->get_has_been_checked() &&
            (static_cast<unsigned long>(current_time -
                                        hst->get_last_check()) <=
             config->cached_host_check_horizon())) {
          logger(dbg_checks, more) << "* Using cached host state: "
                                   << hst->get_current_state();
          update_check_stats(ACTIVE_ONDEMAND_HOST_CHECK_STATS, current_time);
          update_check_stats(ACTIVE_CACHED_HOST_CHECK_STATS, current_time);
        }

        /* else launch an async (parallel) check of the host */
        else
          hst->run_async_check(CHECK_OPTION_NONE, 0.0, false, false,
                                     nullptr, nullptr);
      }
    }

    /* if a hard service recovery has occurred... */
    if (hard_state_change) {
      logger(dbg_checks, more) << "Service experienced a HARD RECOVERY.";

      /* set the state type macro */
      set_state_type(hard);

      /* log the service recovery */
      log_event();
      state_was_logged = true;

      /* 10/04/07 check to see if the service and/or associate host is flapping
       */
      /* this should be done before a notification is sent out to ensure the
       * host didn't just start flapping */
      check_for_flapping(true, true);
      hst->check_for_flapping(true, false, true);
      flapping_check_done = true;

      /* notify contacts about the service recovery */
      notify(reason_normal, "", "", notification_option_none);

      /* run the service event handler to handle the hard state change */
      handle_service_event();
    }

    /* else if a soft service recovery has occurred... */
    else if (state_change) {
      logger(dbg_checks, more) << "Service experienced a SOFT RECOVERY.";

      /* this is a soft recovery */
      set_state_type(soft);

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
    notify(reason_recovery, "", "", notification_option_none);

    /* should we obsessive over service checks? */
    if (config->obsess_over_services())
      obsessive_compulsive_service_check_processor();

    /* reset all service variables because its okay now... */
    _host_problem_at_last_check = false;
    set_current_attempt(1);
    set_state_type(hard);
    _last_hard_state = service::state_ok;
    _last_notification = static_cast<time_t>(0);
    _next_notification = static_cast<time_t>(0);
    set_problem_has_been_acknowledged(false);
    this->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
    set_no_more_notifications(false);

    if (reschedule_check)
      next_service_check =
          (time_t)(get_last_check() +
                   get_check_interval() * config->interval_length());
  }

  /*******************************************/
  /******* SERVICE CHECK PROBLEM LOGIC *******/
  /*******************************************/

  /* hey, something's not working quite like it should... */
  else {
    logger(dbg_checks, more) << "Service is in a non-OK state!";

    /* check the route to the host if its up right now... */
    if (hst->get_current_state() ==  host::state_up) {
      logger(dbg_checks, more)
          << "Host is currently UP, so we'll recheck its state to "
             "make sure...";

      /* 08/04/07 EG launch an async (parallel) host check (possibly cached)
       * unless aggressive host checking is enabled */
      /* previous logic was to simply run a sync (serial) host check */
      if (config->use_aggressive_host_checking())
        hst->perform_on_demand_check(&route_result,
                                     CHECK_OPTION_NONE, true,
                                     config->cached_host_check_horizon());
      else {
        /* can we use the last cached host state? */
        /* only use cached host state if no service state change has occurred */
        if ((!state_change || state_changes_use_cached_state) &&
            hst->get_has_been_checked() &&
            (static_cast<unsigned long>(current_time -
                                        hst->get_last_check()) <=
             config->cached_host_check_horizon())) {
          /* use current host state as route result */
          route_result = hst->get_current_state();
          logger(dbg_checks, more) << "* Using cached host state: "
                                   << hst->get_current_state();
          update_check_stats(ACTIVE_ONDEMAND_HOST_CHECK_STATS, current_time);
          update_check_stats(ACTIVE_CACHED_HOST_CHECK_STATS, current_time);
        }

        /* else launch an async (parallel) check of the host */
        /* CHANGED 02/15/08 only if service changed state since service was last
           checked */
        else if (state_change) {
          /* use current host state as route result */
          route_result = hst->get_current_state();
          hst->run_async_check(CHECK_OPTION_NONE, 0.0, false, false,
                                     nullptr, nullptr);
        }

        /* ADDED 02/15/08 */
        /* else assume same host state */
        else {
          route_result = hst->get_current_state();
          logger(dbg_checks, more) << "* Using last known host state: "
                                   << hst->get_current_state();
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
        hst->perform_on_demand_check(&route_result,
                                     CHECK_OPTION_NONE, true,
                                     config->cached_host_check_horizon());
      }

      /* the service wobbled between non-OK states, so check the host... */
      else if ((state_change && !state_changes_use_cached_state) &&
               _last_hard_state != service::state_ok) {
        logger(dbg_checks, more)
            << "Service wobbled between non-OK states, so we'll recheck"
               " the host state...";
        /* 08/04/07 EG launch an async (parallel) host check unless aggressive
         * host checking is enabled */
        /* previous logic was to simply run a sync (serial) host check */
        /* use current host state as route result */
        route_result = hst->get_current_state();
        hst->run_async_check(CHECK_OPTION_NONE, 0.0, false, false,
                                   nullptr, nullptr);
        /*perform_on_demand_host_check(hst,&route_result,CHECK_OPTION_NONE,true,config->cached_host_check_horizon());
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
        if (!hst->get_has_been_checked()) {
          hst->set_has_been_checked(true);
          hst->set_last_check(get_last_check());
        }

        /* fake the route check result */
        route_result = hst->get_current_state();

        /* possibly re-send host notifications... */
        hst->notify(reason_normal, "", "", notification_option_none);
      }
    }

    /* if the host is down or unreachable ... */
    /* 05/29/2007 NOTE: The host might be in a SOFT problem state due to host
     * check retries/caching.  Not sure if we should take that into account and
     * do something different or not... */
    if (route_result !=  host::state_up) {
      logger(dbg_checks, most)
          << "Host is not UP, so we mark state changes if appropriate";

      /* "fake" a hard state change for the service - well, its not really fake,
       * but it didn't get caught earlier... */
      if (_last_hard_state != _current_state)
        hard_state_change = true;

      /* update last state change times */
      if (state_change || hard_state_change)
        set_last_state_change(get_last_check());
      if (hard_state_change) {
        set_last_hard_state_change(get_last_check());
        set_state_type(hard );
        _last_hard_state = _current_state;
      }

      /* put service into a hard state without attempting check retries and
       * don't send out notifications about it */
      _host_problem_at_last_check = true;
    }

    /* the host is up - it recovered since the last time the service was
       checked... */
    else if (_host_problem_at_last_check) {
      /* next time the service is checked we shouldn't get into this same
       * case... */
      _host_problem_at_last_check = false;

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
      if (get_state_type() == soft)
        set_current_attempt(1);
    }

    logger(dbg_checks, more)
        << "Current/Max Attempt(s): " << get_current_attempt() << '/'
        << get_max_attempts();

    /* if we should retry the service check, do so (except it the host is down
     * or unreachable!) */
    if (get_current_attempt() < get_max_attempts()) {
      /* the host is down or unreachable, so don't attempt to retry the service
       * check */
      if (route_result !=  host::state_up) {
        logger(dbg_checks, more)
            << "Host isn't UP, so we won't retry the service check...";

        /* the host is not up, so reschedule the next service check at regular
         * interval */
        if (reschedule_check)
          next_service_check =
              (time_t)(get_last_check() + get_check_interval() *
                                           config->interval_length());

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
        set_state_type(soft);

        /* log the service check retry */
        log_event();
        state_was_logged = true;

        /* run the service event handler to handle the soft state */
        handle_service_event();

        if (reschedule_check)
          next_service_check =
              (time_t)(get_last_check() +
                       get_retry_interval() * config->interval_length());
      }

      /* perform dependency checks on the second to last check of the service */
      if (config->enable_predictive_service_dependency_checks() &&
          get_current_attempt() == (get_max_attempts() - 1)) {
        logger(dbg_checks, more)
            << "Looking for services to check for predictive "
               "dependency checks...";

        /* check services that THIS ONE depends on for notification AND
         * execution */
        /* we do this because we might be sending out a notification soon and we
         * want the dependency logic to be accurate */
        std::pair<std::string, std::string> id({_hostname, _description});
        servicedependency_mmap const&
            dependencies(state::instance().servicedependencies());
        for (servicedependency_mmap::const_iterator
               it{dependencies.find(id)},
               end{dependencies.end()};
             it != end && it->first == id; ++it) {
          servicedependency* temp_dependency(&*it->second.get());

          if (temp_dependency->dependent_service_ptr == this &&
              temp_dependency->master_service_ptr != nullptr) {
            master_service = (com::centreon::engine::service*)
                                 temp_dependency->master_service_ptr;
            logger(dbg_checks, most)
                << "Predictive check of service '"
                << master_service->get_description() << "' on host '"
                << master_service->get_hostname() << "' queued.";
            check_servicelist.push_back(master_service);
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
      set_state_type(hard);

      /* if we've hard a hard state change... */
      if (hard_state_change) {
        /* log the service problem (even if host is not up, which is new in
         * 0.0.5) */
        log_event();
        state_was_logged = true;
      }

      /* else log the problem (again) if this service is flagged as being
         volatile */
      else if (get_is_volatile()) {
        log_event();
        state_was_logged = true;
      }

      /* check for start of flexible (non-fixed) scheduled downtime if we just
       * had a hard error */
      /* we need to check for both, state_change (SOFT) and hard_state_change
       * (HARD) values */
      if ((hard_state_change || state_change) &&
          get_pending_flex_downtime() > 0)
        downtime_manager::instance().check_pending_flex_service_downtime(this);

      /* 10/04/07 check to see if the service and/or associate host is flapping
       */
      /* this should be done before a notification is sent out to ensure the
       * host didn't just start flapping */
      check_for_flapping(true, true);
      hst->check_for_flapping(true, false, true);
      flapping_check_done = true;

      /* (re)send notifications out about this service problem if the host is up
       * (and was at last check also) and the dependencies were okay... */
      notify(reason_normal, "", "", notification_option_none);

      /* run the service event handler if we changed state from the last hard
       * state or if this service is flagged as being volatile */
      if (hard_state_change || get_is_volatile())
        handle_service_event();

      /* save the last hard state */
      _last_hard_state = _current_state;

      /* reschedule the next check at the regular interval */
      if (reschedule_check)
        next_service_check =
            (time_t)(get_last_check() +
                     get_check_interval() * config->interval_length());
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
    set_should_be_scheduled(true);

    /* next check time was calculated above */
    set_next_check(next_service_check);

    /* make sure we don't get ourselves into too much trouble... */
    if (current_time > get_next_check())
      set_next_check(current_time);

    // Make sure we rescheduled the next service check at a valid time.
    {
      timezone_locker lock(get_timezone());
      preferred_time = get_next_check();
      get_next_valid_time(preferred_time, &next_valid_time,
                          this->check_period_ptr);
      set_next_check(next_valid_time);
    }

    /* services with non-recurring intervals do not get rescheduled */
    if (get_check_interval() == 0)
      set_should_be_scheduled(false);

    /* services with active checks disabled do not get rescheduled */
    if (!get_checks_enabled())
      set_should_be_scheduled(false);

    /* schedule a non-forced check if we can */
    if (get_should_be_scheduled())
      schedule_check(get_next_check(), CHECK_OPTION_NONE);
  }

  /* if we're stalking this state type and state was not already logged AND the
   * plugin output changed since last check, log it now.. */
  if (get_state_type() == hard && !state_change && !state_was_logged &&
      old_plugin_output == get_plugin_output()) {
    if ((_current_state == service::state_ok && get_stalk_on(ok)))
      log_event();

    else if ((_current_state == service::state_warning && get_stalk_on(warning)))
      log_event();

    else if ((_current_state == service::state_unknown && get_stalk_on(unknown)))
      log_event();

    else if ((_current_state == service::state_critical && get_stalk_on(critical)))
      log_event();
  }

  /* send data to event broker */
  broker_service_check(NEBTYPE_SERVICECHECK_PROCESSED, NEBFLAG_NONE,
                       NEBATTR_NONE, this, get_check_type(),
                       queued_check_result->get_start_time(),
                       queued_check_result->get_finish_time(), nullptr, get_latency(),
                       get_execution_time(), config->service_check_timeout(),
                       queued_check_result->get_early_timeout(),
                       queued_check_result->get_return_code(), nullptr, nullptr);

  if (!(reschedule_check && get_should_be_scheduled() &&
        get_has_been_checked()) ||
      !get_checks_enabled()) {
    /* set the checked flag */
    set_has_been_checked(true);
    /* update the current service status log */
    this->update_status(false);
  }

  /* check to see if the service and/or associate host is flapping */
  if (!flapping_check_done) {
    check_for_flapping(true, true);
    hst->check_for_flapping(true, false, true);
  }

  /* update service performance info */
  update_service_performance_data();

  /* run async checks of all services we added above */
  /* don't run a check if one is already executing or we can get by with a
   * cached state */
  for (std::list<service*>::iterator
         it{check_servicelist.begin()},
         end{check_servicelist.end()};
       it != end;
       ++it) {
    run_async_check = true;
    service* svc{*it};

    /* we can get by with a cached state, so don't check the service */
    if (static_cast<unsigned long>(current_time - svc->get_last_check()) <=
        config->cached_service_check_horizon()) {
      run_async_check = false;

      /* update check statistics */
      update_check_stats(ACTIVE_CACHED_SERVICE_CHECK_STATS, current_time);
    }

    if (svc->get_is_executing())
      run_async_check = false;

    if (run_async_check)
      svc->run_async_check(CHECK_OPTION_NONE, 0.0, false, false, nullptr,
                           nullptr);
  }
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
  if (get_state_type() == soft && !config->log_service_retries())
    return OK;

  if (!_host_ptr)
    return ERROR;

  uint32_t log_options{NSLOG_SERVICE_UNKNOWN};
  char const* state{"UNKNOWN"};
  if (_current_state >= 0 &&
      (unsigned int)_current_state < tab_service_states.size()) {
    log_options = tab_service_states[_current_state].first;
    state = tab_service_states[_current_state].second.c_str();
  }
  std::string const& state_type{tab_state_type[get_state_type()]};

  logger(log_options, basic)
      << "SERVICE ALERT: " << _hostname << ";" << _description << ";"
      << state << ";" << state_type << ";" << get_current_attempt() << ";"
      << get_plugin_output();
  return OK;
}

// int service::get_check_viability(...)  << check_service_check_viability()
/* detects service flapping */
void service::check_for_flapping(bool update,
                                 bool allow_flapstart_notification) {
  bool update_history;
  bool is_flapping = false;
  unsigned int x = 0;
  unsigned int y = 0;
  int last_state_history_value = service::state_ok;
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
      << "Checking service '" << _description << "' on host '"
      << _hostname << "' for flapping...";

  /* if this is a soft service state and not a soft recovery, don't record this
   * in the history */
  /* only hard states and soft recoveries get recorded for flap detection */
  if (get_state_type() == soft && _current_state != service::state_ok)
    return;

  /* what threshold values should we use (global or service-specific)? */
  low_threshold = (get_low_flap_threshold() <= 0.0)
                      ? config->low_service_flap_threshold()
                      : get_low_flap_threshold();
  high_threshold = (get_high_flap_threshold() <= 0.0)
                       ? config->high_service_flap_threshold()
                       : get_high_flap_threshold();

  update_history = update;

  /* should we update state history for this state? */
  if (update_history) {
    if (_current_state == service::state_ok && !get_flap_detection_on(ok))
      update_history = false;
    if (_current_state == service::state_warning &&
        !get_flap_detection_on(warning))
      update_history = false;
    if (_current_state == service::state_unknown &&
        !get_flap_detection_on(unknown))
      update_history = false;
    if (_current_state == service::state_critical &&
        !get_flap_detection_on(critical))
      update_history = false;
  }

  /* record current service state */
  if (update_history) {
    /* record the current state in the state history */
    get_state_history()[get_state_history_index()] = _current_state;

    /* increment state history index to next available slot */
    set_state_history_index(get_state_history_index() + 1);
    if (get_state_history_index() >= MAX_STATE_HISTORY_ENTRIES)
      set_state_history_index(0);
  }

  /* calculate overall and curved percent state changes */
  for (x = 0, y = get_state_history_index(); x < MAX_STATE_HISTORY_ENTRIES;
       x++) {
    if (x == 0) {
      last_state_history_value = get_state_history()[y];
      y++;
      if (y >= MAX_STATE_HISTORY_ENTRIES)
        y = 0;
      continue;
    }

    if (last_state_history_value != get_state_history()[y])
      curved_changes +=
          (((double)(x - 1) * (high_curve_value - low_curve_value)) /
           ((double)(MAX_STATE_HISTORY_ENTRIES - 2))) +
          low_curve_value;

    last_state_history_value = get_state_history()[y];

    y++;
    if (y >= MAX_STATE_HISTORY_ENTRIES)
      y = 0;
  }

  /* calculate overall percent change in state */
  curved_percent_change = (double)(((double)curved_changes * 100.0) /
                                   (double)(MAX_STATE_HISTORY_ENTRIES - 1));

  set_percent_state_change(curved_percent_change);

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
  if (get_flap_detection_enabled())
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
  if (is_flapping && !get_is_flapping())
    set_flap(curved_percent_change, high_threshold, low_threshold,
             allow_flapstart_notification);

  /* did the service just stop flapping? */
  else if (!is_flapping && get_is_flapping())
    clear_flap(curved_percent_change, high_threshold, low_threshold);
}

/* handles changes in the state of a service */
int service::handle_service_event() {
  nagios_macros mac;

  logger(dbg_functions, basic) << "handle_service_event()";

  /* send event data to broker */
  broker_statechange_data(NEBTYPE_STATECHANGE_END, NEBFLAG_NONE, NEBATTR_NONE,
                          SERVICE_STATECHANGE, (void*)this, _current_state,
                          get_state_type(), get_current_attempt(),
                          get_max_attempts(), nullptr);

  /* bail out if we shouldn't be running event handlers */
  if (config->enable_event_handlers() == false)
    return OK;
  if (!get_event_handler_enabled())
    return OK;

  /* find the host */
  if (!get_host_ptr())
    return ERROR;

  /* update service macros */
  grab_host_macros_r(&mac, get_host_ptr());
  grab_service_macros_r(&mac, this);

  /* run the global service event handler */
  run_global_service_event_handler(&mac, this);

  /* run the event handler command if there is one */
  if (!get_event_handler().empty())
    run_service_event_handler(&mac, this);
  clear_volatile_macros_r(&mac);

  /* send data to event broker */
  broker_external_command(NEBTYPE_EXTERNALCOMMAND_CHECK, NEBFLAG_NONE,
                          NEBATTR_NONE, CMD_NONE, time(nullptr), nullptr,
                          nullptr, nullptr);

  return OK;
}

/* handles service check results in an obsessive compulsive manner... */
int service::obsessive_compulsive_service_check_processor() {
  std::string raw_command;
  std::string processed_command;
  host* temp_host{get_host_ptr()};
  int early_timeout = false;
  double exectime = 0.0;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;
  nagios_macros mac;

  logger(dbg_functions, basic)
      << "obsessive_compulsive_service_check_processor()";

  /* bail out if we shouldn't be obsessing */
  if (config->obsess_over_services() == false)
    return OK;
  if (!get_obsess_over())
    return OK;

  /* if there is no valid command, exit */
  if (config->ocsp_command().empty())
    return ERROR;

  /* find the associated host */
  if (temp_host == nullptr)
    return ERROR;

  /* update service macros */
  grab_host_macros_r(&mac, temp_host);
  grab_service_macros_r(&mac, this);

  /* get the raw command line */
  get_raw_command_line_r(&mac, ocsp_command_ptr, config->ocsp_command().c_str(),
                         raw_command, macro_options);
  if (raw_command.empty()) {
    clear_volatile_macros_r(&mac);
    return ERROR;
  }

  logger(dbg_checks, most) << "Raw obsessive compulsive service processor "
                              "command line: "
                           << raw_command;

  /* process any macros in the raw command line */
  process_macros_r(&mac, raw_command, processed_command, macro_options);
  if (processed_command.empty()) {
    clear_volatile_macros_r(&mac);
    return ERROR;
  }

  logger(dbg_checks, most) << "Processed obsessive compulsive service "
                              "processor command line: "
                           << processed_command;

  /* run the command */
  try {
    std::string tmp;
    my_system_r(&mac, processed_command, config->ocsp_timeout(), &early_timeout,
                &exectime, tmp, 0);
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
        << _description << "' on host '" << _hostname
        << "' timed out after " << config->ocsp_timeout() << " seconds";

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
      << _description << "' on host '" << _hostname
      << "': check options=" << check_options << ", latency=" << latency;

  /* attempt to run the check */
  result = run_async_check(check_options, latency, true, true, &time_is_valid,
                           &preferred_time);

  /* an error occurred, so reschedule the check */
  if (result == ERROR) {
    logger(dbg_checks, more)
        << "Unable to run scheduled service check at this time";

    /* only attempt to (re)schedule checks that should get checked... */
    if (get_should_be_scheduled()) {
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
            static_cast<time_t>(get_check_interval() <= 0
                                    ? 300
                                    : get_check_interval() *
                                          config->interval_length());

      // Make sure we rescheduled the next service check at a valid time.
      {
        timezone_locker lock(get_timezone());
        get_next_valid_time(preferred_time, &next_valid_time,
                            this->check_period_ptr);

        // The service could not be rescheduled properly.
        // Set the next check time for next week.
        if (!time_is_valid &&
            !check_time_against_period(next_valid_time,
                                      this->check_period_ptr)) {
          set_next_check((time_t)(next_valid_time + 60 * 60 * 24 * 7));
          logger(log_runtime_warning, basic)
              << "Warning: Check of service '" << _description
              << "' on host '" << _hostname
              << "' could not be "
                 "rescheduled properly. Scheduling check for next week...";
          logger(dbg_checks, more)
              << "Unable to find any valid times to reschedule the next "
                 "service check!";
        }
        // This service could be rescheduled...
        else {
          set_next_check(next_valid_time);
          set_should_be_scheduled(true);
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
    if (get_should_be_scheduled())
      schedule_check(get_next_check(), check_options);

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
      << ", active check of service '" << _description
      << "' on host '" << _hostname << "' @ "
      << my_ctime(&check_time);

  // Don't schedule a check if active checks
  // of this service are disabled.
  if (!get_checks_enabled() && !(options & CHECK_OPTION_FORCE_EXECUTION)) {
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
      set_next_check(check_time);

      // Place the new event in the event queue.
      new_event->event_type = EVENT_SERVICE_CHECK;
      new_event->event_data = (void*)this;
      new_event->event_args = (void*)nullptr;
      new_event->event_options = options;
      new_event->run_time = get_next_check();
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
      set_next_check(temp_event->run_time);

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
      << "Service '" << _description << "' on host '"
      << _hostname << "' started flapping!";

  /* log a notice - this one is parsed by the history CGI */
  logger(log_runtime_warning, basic)
      << com::centreon::logging::setprecision(1)
      << "SERVICE FLAPPING ALERT: " << _hostname << ";"
      << _description
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
      comment::service, comment::flapping, _hostname,
      _description, time(nullptr), "(Centreon Engine Process)",
      oss.str(), false, comment::internal, false, (time_t)0)};

  comment::comments.insert({com->get_comment_id(), com});

  this->flapping_comment_id = com->get_comment_id();

  /* set the flapping indicator */
  set_is_flapping(true);

  /* send data to event broker */
  broker_flapping_data(NEBTYPE_FLAPPING_START, NEBFLAG_NONE, NEBATTR_NONE,
                       SERVICE_FLAPPING, this, percent_change, high_threshold,
                       low_threshold, nullptr);

  /* send a notification */
  if (allow_flapstart_notification)
    notify(reason_flappingstart, "", "",
           notification_option_none);
}

/* handles a service that has stopped flapping */
void service::clear_flap(double percent_change,
                         double high_threshold,
                         double low_threshold) {
  logger(dbg_functions, basic) << "clear_service_flap()";

  logger(dbg_flapping, more)
      << "Service '" << _description << "' on host '"
      << _hostname << "' stopped flapping.";

  /* log a notice - this one is parsed by the history CGI */
  logger(log_info_message, basic)
      << com::centreon::logging::setprecision(1)
      << "SERVICE FLAPPING ALERT: " << _hostname << ";"
      << _description
      << ";STOPPED; Service appears to have stopped flapping ("
      << percent_change << "% change < " << low_threshold << "% threshold)";

  /* delete the comment we added earlier */
  if (this->flapping_comment_id != 0)
    comment::delete_comment(this->flapping_comment_id);
  this->flapping_comment_id = 0;

  /* clear the flapping indicator */
  set_is_flapping(false);

  /* send data to event broker */
  broker_flapping_data(NEBTYPE_FLAPPING_STOP, NEBFLAG_NONE,
                       NEBATTR_FLAPPING_STOP_NORMAL, SERVICE_FLAPPING, this,
                       percent_change, high_threshold, low_threshold, nullptr);

  /* send a notification */
  notify(reason_flappingstop, "", "", notification_option_none);

  /* should we send a recovery notification? */
  notify(reason_recovery, "", "", notification_option_none);
}

/* enables flap detection for a specific service */
void service::enable_flap_detection() {
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic) << "service::enable_flap_detection()";

  logger(dbg_flapping, more)
      << "Enabling flap detection for service '" << _description
      << "' on host '" << _hostname << "'.";

  /* nothing to do... */
  if (get_flap_detection_enabled())
    return;

  /* set the attribute modified flag */
  _modified_attributes |= attr;

  /* set the flap detection enabled flag */
  set_flap_detection_enabled(true);

  /* send data to event broker */
  broker_adaptive_service_data(NEBTYPE_ADAPTIVESERVICE_UPDATE, NEBFLAG_NONE,
                               NEBATTR_NONE, this, CMD_NONE, attr,
                               get_modified_attributes(), nullptr);

  /* check for flapping */
  check_for_flapping(false, true);

  /* update service status */
  update_status(false);
}

/* disables flap detection for a specific service */
void service::disable_flap_detection() {
  unsigned long attr = MODATTR_FLAP_DETECTION_ENABLED;

  logger(dbg_functions, basic) << "disable_service_flap_detection()";

  logger(dbg_flapping, more)
      << "Disabling flap detection for service '" << _description
      << "' on host '" << _hostname << "'.";

  /* nothing to do... */
  if (!get_flap_detection_enabled())
    return;

  /* set the attribute modified flag */
  _modified_attributes |= attr;

  /* set the flap detection enabled flag */
  set_flap_detection_enabled(false);

  /* send data to event broker */
  broker_adaptive_service_data(NEBTYPE_ADAPTIVESERVICE_UPDATE, NEBFLAG_NONE,
                               NEBATTR_NONE, this, CMD_NONE, attr,
                               _modified_attributes, nullptr);

  /* handle the details... */
  handle_flap_detection_disabled();
}

/* updates service status info */
void service::update_status(bool aggregated_dump) {
  /* send data to event broker (non-aggregated dumps only) */
  if (!aggregated_dump)
    broker_service_status(NEBTYPE_SERVICESTATUS_UPDATE, NEBFLAG_NONE,
                          NEBATTR_NONE, this, nullptr);
}

/**
 * @brief Checks the viability of sending out a service alert (top level
 * filters)
 *
 * @param type A reason type notification
 * @param options Options for the notification.
 *
 * @return true if the check is good, false otherwise.
 */
//bool service::check_notification_viability(reason_type type, int options) {
//  time_t current_time;
//  /* get current time */
//  time(&current_time);
//
//  logger(dbg_functions, basic) << "service::check_notification_viability()";
//
//  timeperiod* temp_period{get_notification_timeperiod()};
//
//  // See if the service can have notifications sent out at this time.
//  {
//    timezone_locker lock(get_timezone());
//    if (check_time_against_period(current_time,
//                                  temp_period) == ERROR) {
//      logger(dbg_notifications, more)
//          << "This service shouldn't have notifications sent out "
//             "at this time.";
//
//      // Calculate the next acceptable notification time,
//      // once the next valid time range arrives...
//      if (type == notification_normal) {
//        time_t timeperiod_start;
//        get_next_valid_time(current_time, &timeperiod_start,
//                            temp_period);
//
//        // Looks like there are no valid notification times defined, so
//        // schedule the next one far into the future (one year)...
//        if (timeperiod_start == (time_t)0)
//          _next_notification =
//              static_cast<time_t>(current_time + 60 * 60 * 24 * 365);
//        // Else use the next valid notification time.
//        else
//          _next_notification = timeperiod_start;
//        logger(dbg_notifications, more) << "Next possible notification time: "
//                                        << my_ctime(&_next_notification);
//      }
//
//      return false;
//    }
//  }
//
//  /*********************************************/
//  /*** SPECIAL CASE FOR CUSTOM NOTIFICATIONS ***/
//  /*********************************************/
//
//  /* custom notifications are good to go at this point... */
//  if (type == notification_custom) {
//    if (get_scheduled_downtime_depth() > 0 ||
//        host_ptr->get_scheduled_downtime_depth() > 0) {
//      logger(dbg_notifications, more)
//          << "We shouldn't send custom notification during "
//             "scheduled downtime.";
//      return false;
//    }
//    return true;
//  }
//
//  /****************************************/
//  /*** SPECIAL CASE FOR ACKNOWLEGEMENTS ***/
//  /****************************************/
//
//  /*
//   * acknowledgements only have to pass three general filters, although they
//   * have another test of their own...
//   */
//  if (type == notification_acknowledgement) {
//    /* don't send an acknowledgement if there isn't a problem... */
//    if (_current_state == service::state_ok) {
//      logger(dbg_notifications, more)
//          << "The service is currently true, so we won't send an "
//             "acknowledgement.";
//      return false;
//    }
//
//    /*
//     * acknowledgement viability test passed, so the notification can be sent
//     * out
//     */
//    return true;
//  }
//
//  /****************************************/
//  /*** SPECIAL CASE FOR FLAPPING ALERTS ***/
//  /****************************************/
//
//  /* flapping notifications only have to pass three general filters */
//  if (type == notification_flappingstart || type == notification_flappingstop ||
//      type == notification_flappingdisabled) {
//    /* don't send a notification if we're not supposed to... */
//    if (!get_notify_on(flapping)) {
//      logger(dbg_notifications, more)
//          << "We shouldn't notify about FLAPPING events for this "
//             "service.";
//      return false;
//    }
//
//    /* don't send notifications during scheduled downtime */
//    if (get_scheduled_downtime_depth() > 0 ||
//        host_ptr->get_scheduled_downtime_depth() > 0) {
//      logger(dbg_notifications, more)
//          << "We shouldn't notify about FLAPPING events during "
//             "scheduled downtime.";
//      return false;
//    }
//
//    /* flapping viability test passed, so the notification can be sent out */
//    return true;
//  }
//
//  /****************************************/
//  /*** SPECIAL CASE FOR DOWNTIME ALERTS ***/
//  /****************************************/
//
//  /* downtime notifications only have to pass three general filters */
//  if (type == notification_downtimestart || type == notification_downtimeend ||
//      type == notification_downtimecancelled) {
//    /* don't send a notification if we're not supposed to... */
//    if (!get_notify_on(downtime)) {
//      logger(dbg_notifications, more)
//          << "We shouldn't notify about DOWNTIME events for "
//             "this service.";
//      return false;
//    }
//
//    /*
//     * don't send notifications during scheduled downtime (for service only,
//     * not host)
//     */
//    if (get_scheduled_downtime_depth() > 0) {
//      logger(dbg_notifications, more)
//          << "We shouldn't notify about DOWNTIME events during "
//             "scheduled downtime.";
//      return false;
//    }
//
//    /* downtime viability test passed, so the notification can be sent out */
//    return true;
//  }
//
//  /****************************************/
//  /*** NORMAL NOTIFICATIONS ***************/
//  /****************************************/
//
//  /* is this a hard problem/recovery? */
//  if (get_state_type() == soft) {
//    logger(dbg_notifications, more)
//        << "This service is in a soft state, so we won't send a "
//           "notification out.";
//    return false;
//  }
//
//  /* has this problem already been acknowledged? */
//  if (get_problem_has_been_acknowledged()) {
//    logger(dbg_notifications, more)
//        << "This service problem has already been acknowledged, "
//           "so we won't send a notification out.";
//    return false;
//  }
//
//  /* check service notification dependencies */
//  if (check_dependencies(hostdependency::notification) ==
//      DEPENDENCIES_FAILED) {
//    logger(dbg_notifications, more)
//        << "Service notification dependencies for this service "
//           "have failed, so we won't sent a notification out.";
//    return false;
//  }
//
//  /* check host notification dependencies */
//  if (host_ptr->check_dependencies(hostdependency::notification) ==
//      DEPENDENCIES_FAILED) {
//    logger(dbg_notifications, more)
//        << "Host notification dependencies for this service have failed, "
//           "so we won't sent a notification out.";
//    return false;
//  }
//
//  /* see if we should notify about problems with this service */
//  if (_current_state == service::state_unknown &&
//    !get_notify_on(unknown)) {
//    logger(dbg_notifications, more)
//        << "We shouldn't notify about UNKNOWN states for this service.";
//    return false;
//  }
//  if (_current_state == service::state_warning &&
//    !get_notify_on(warning)) {
//    logger(dbg_notifications, more)
//        << "We shouldn't notify about WARNING states for this service.";
//    return false;
//  }
//  if (_current_state == service::state_critical &&
//    !get_notify_on(critical)) {
//    logger(dbg_notifications, more)
//        << "We shouldn't notify about CRITICAL states for this service.";
//    return false;
//  }
//  if (_current_state == service::state_ok) {
//    if (!get_notify_on(recovery)) {
//      logger(dbg_notifications, more)
//          << "We shouldn't notify about RECOVERY states for this service.";
//      return false;
//    }
//    /* No notification in input */
//    if (get_notified_on() == 0) {
//      logger(dbg_notifications, more)
//          << "We shouldn't notify about this recovery.";
//      return false;
//    }
//  }
//
//  /* see if enough time has elapsed for first notification */
//  if (type == notification_normal &&
//      (this->current_notification_number == 0 ||
//       (_current_state == service::state_ok && !_recovery_been_sent))) {
//    /* get the time at which a notification should have been sent */
//    time_t& initial_notif_time(_initial_notif_time);
//
//    /* if not set, set it to now */
//    if (!initial_notif_time)
//      initial_notif_time = time(nullptr);
//
//    double notification_delay =
//        (_current_state != service::state_ok ? get_first_notification_delay()
//                                         : _recovery_notification_delay) *
//        config->interval_length();
//
//    if (current_time <
//        (time_t)(initial_notif_time + (time_t)(notification_delay))) {
//      if (_current_state == service::state_ok)
//        logger(dbg_notifications, more)
//            << "Not enough time has elapsed since the service changed to a "
//               "true state, so we should not notify about this problem yet";
//      else
//        logger(dbg_notifications, more)
//            << "Not enough time has elapsed since the service changed to a "
//               "non-true state, so we should not notify about this problem yet";
//      return false;
//    }
//  }
//
//  /* if this service is currently flapping, don't send the notification */
//  if (get_is_flapping()) {
//    logger(dbg_notifications, more)
//        << "This service is currently flapping, so we won't send "
//           "notifications.";
//    return false;
//  }
//
//  /*
//   * if this service is currently in a scheduled downtime period, don't send
//   * the notification
//   */
//  if (get_scheduled_downtime_depth() > 0) {
//    logger(dbg_notifications, more)
//        << "This service is currently in a scheduled downtime, so "
//           "we won't send notifications.";
//    return false;
//  }
//
//  /*
//   * if this host is currently in a scheduled downtime period, don't send the
//   * notification
//   * */
//  if (host_ptr->get_scheduled_downtime_depth() > 0) {
//    logger(dbg_notifications, more)
//        << "The host this service is associated with is currently in "
//           "a scheduled downtime, so we won't send notifications.";
//    return false;
//  }
//
//  /*
//   ***** RECOVERY NOTIFICATIONS ARE GOOD TO GO AT THIS POINT IF ANY OTHER *****
//   ***** NOTIFICATION WAS SENT                                            *****
//   */
//  if (_current_state == service::state_ok)
//    return (this->current_notification_number > 0) ? true : false;
//
//  /*
//   * don't notify contacts about this service problem again if the notification
//   * interval is set to 0
//   */
//  if (get_no_more_notifications()) {
//    logger(dbg_notifications, more)
//        << "We shouldn't re-notify contacts about this service problem.";
//    return false;
//  }
//
//  /*
//   * if the host is down or unreachable, don't notify contacts about service
//   * failures
//   */
//  if (host_ptr->get_current_state() !=  host::state_up) {
//    logger(dbg_notifications, more)
//        << "The host is either down or unreachable, so we won't "
//           "notify contacts about this service.";
//    return false;
//  }
//
//  /*
//   * don't notify if we haven't waited long enough since the last time (and
//   * the service is not marked as being volatile)
//   */
//  if (current_time < _next_notification && !get_is_volatile()) {
//    logger(dbg_notifications, more)
//        << "We haven't waited long enough to re-notify contacts "
//           "about this service.";
//    logger(dbg_notifications, more)
//        << "Next valid notification time: " << my_ctime(&_next_notification);
//    return false;
//  }
//
//  return true;
//}

/* checks viability of performing a service check */
int service::verify_check_viability(int check_options,
                                    int* time_is_valid,
                                    time_t* new_time) {
  int perform_check = true;
  time_t current_time = 0L;
  time_t preferred_time = 0L;
  int check_interval = 0;

  logger(dbg_functions, basic) << "check_service_check_viability()";

  /* get the check interval to use if we need to reschedule the check */
  if (get_state_type() == soft && _current_state != service::state_ok)
    check_interval = static_cast<int>(get_retry_interval() *
                                      config->interval_length());
  else
    check_interval = static_cast<int>(get_check_interval() *
                                      config->interval_length());

  /* get the current time */
  time(&current_time);

  /* initialize the next preferred check time */
  preferred_time = current_time;

  /* can we check the host right now? */
  if (!(check_options & CHECK_OPTION_FORCE_EXECUTION)) {
    /* if checks of the service are currently disabled... */
    if (!get_checks_enabled()) {
      preferred_time = current_time + check_interval;
      perform_check = false;

      logger(dbg_checks, most)
          << "Active checks of the service are currently disabled.";
    }

    // Make sure this is a valid time to check the service.
    {
      timezone_locker lock(get_timezone());
      if (!check_time_against_period((unsigned long)current_time,
                                    this->check_period_ptr)) {
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
    if (check_dependencies(hostdependency::execution) ==
        DEPENDENCIES_FAILED) {
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

void service::grab_macros_r(nagios_macros* mac) {
  grab_host_macros_r(mac, _host_ptr);
  grab_service_macros_r(mac, this);
}

/* notify a specific contact about a service problem or recovery */
int service::notify_contact(nagios_macros* mac,
                            contact* cntct,
                            reason_type type,
                            std::string const& not_author,
                            std::string const& not_data,
                            int options __attribute__((unused)),
                            int escalated) {
  std::string raw_command;
  std::string processed_command;
  int early_timeout = false;
  double exectime;
  struct timeval start_time, end_time;
  struct timeval method_start_time, method_end_time;
  int macro_options = STRIP_ILLEGAL_MACRO_CHARS | ESCAPE_MACRO_CHARS;
  int neb_result;

  logger(dbg_functions, basic) << "notify_contact_of_service()";
  logger(dbg_notifications, most)
      << "** Attempting to notifying contact '" << cntct->get_name() << "'...";

  /*
   * check viability of notifying this user
   * acknowledgements are no longer excluded from this test -
   * added 8/19/02 Tom Bertelson
   */
  notification_category cat{get_category(type)};
  if (!cntct->should_be_notified(cat, type, *this))
    return ERROR;
//  if (cntct->check_service_notification_viability(this, type,
//                                                   options) == ERROR)
//    return ERROR;

  logger(dbg_notifications, most)
      << "** Notifying contact '" << cntct->get_name() << "'";

  /* get start time */
  gettimeofday(&start_time, nullptr);

  /* send data to event broker */
  end_time.tv_sec = 0L;
  end_time.tv_usec = 0L;
  neb_result = broker_contact_notification_data(
      NEBTYPE_CONTACTNOTIFICATION_START, NEBFLAG_NONE, NEBATTR_NONE,
      service_notification, type, start_time, end_time, (void*)this, cntct,
      not_author.c_str(), not_data.c_str(), escalated, nullptr);
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
        NEBTYPE_CONTACTNOTIFICATIONMETHOD_START, NEBFLAG_NONE, NEBATTR_NONE,
        service_notification, type, method_start_time, method_end_time,
        (void*)this, cntct, cmd->get_command_line().c_str(), not_author.c_str(),
        not_data.c_str(), escalated, nullptr);
    if (NEBERROR_CALLBACKCANCEL == neb_result)
      break;
    else if (NEBERROR_CALLBACKOVERRIDE == neb_result)
      continue;

    /* get the raw command line */
    get_raw_command_line_r(mac, cmd.get(), cmd->get_command_line().c_str(),
                           raw_command, macro_options);
    if (raw_command.empty())
      continue;

    logger(dbg_notifications, most)
        << "Raw notification command: " << raw_command;

    /* process any macros contained in the argument */
    process_macros_r(mac, raw_command, processed_command, macro_options);
    if (processed_command.empty())
      continue;

    /* run the notification command... */

    logger(dbg_notifications, most)
        << "Processed notification command: " << processed_command;

    /* log the notification to program log file */
    if (config->log_notifications()) {
      char const* service_state_str("UNKNOWN");
      if ((unsigned int)_current_state < tab_service_states.size())
        service_state_str =
            tab_service_states[_current_state].second.c_str();

      char const* notification_str("");
      if ((unsigned int)type < tab_notification_str.size())
        notification_str = tab_notification_str[type].c_str();

      std::string info;
      if (type == reason_custom)
        notification_str = "CUSTOM";
      else if (type == reason_acknowledgement)
        info.append(";").append(not_author).append(";").append(not_data);

      std::string service_notification_state;
      if (strcmp(notification_str, "NORMAL") == 0)
        service_notification_state.append(service_state_str);
      else
        service_notification_state.append(notification_str)
            .append(" (")
            .append(service_state_str)
            .append(")");

      logger(log_service_notification, basic)
          << "SERVICE NOTIFICATION: " << cntct->get_name() << ';'
          << get_hostname() << ';' << get_description() << ';'
          << service_notification_state << ";" << cmd->get_name() << ';'
          << get_plugin_output() << info;
    }

    /* run the notification command */
    try {
      std::string tmp;
      my_system_r(mac, processed_command, config->notification_timeout(),
                  &early_timeout, &exectime, tmp, 0);
    } catch (std::exception const& e) {
      logger(log_runtime_error, basic)
          << "Error: can't execute service notification '" << cntct->get_name()
          << "' : " << e.what();
    }

    /* check to see if the notification command timed out */
    if (early_timeout) {
      logger(log_service_notification | log_runtime_warning, basic)
          << "Warning: Contact '" << cntct->get_name()
          << "' service notification command '" << processed_command
          << "' timed out after " << config->notification_timeout()
          << " seconds";
    }

    /* get end time */
    gettimeofday(&method_end_time, nullptr);

    /* send data to event broker */
    broker_contact_notification_method_data(
        NEBTYPE_CONTACTNOTIFICATIONMETHOD_END, NEBFLAG_NONE, NEBATTR_NONE,
        service_notification, type, method_start_time, method_end_time,
        (void*)this, cntct, cmd->get_command_line().c_str(), not_author.c_str(),
        not_data.c_str(), escalated, nullptr);
  }

  /* get end time */
  gettimeofday(&end_time, nullptr);

  /* update the contact's last service notification time */
  cntct->set_last_service_notification(start_time.tv_sec);

  /* send data to event broker */
  broker_contact_notification_data(
      NEBTYPE_CONTACTNOTIFICATION_END, NEBFLAG_NONE, NEBATTR_NONE,
      service_notification, type, start_time, end_time, (void*)this, cntct,
      not_author.c_str(), not_data.c_str(), escalated, nullptr);
  return OK;
}

void service::update_notification_flags() {
  /* update notifications flags */
  if (_current_state == service::state_unknown)
    add_notified_on(unknown);
  else if (_current_state == service::state_warning)
    add_notified_on(warning);
  else if (_current_state == service::state_critical)
    add_notified_on(critical);
}

/* calculates next acceptable re-notification time for a service */
time_t service::get_next_notification_time(time_t offset) {
  bool have_escalated_interval{false};

  logger(dbg_functions, basic) << "service::get_next_notification_time()";
  logger(dbg_notifications, most)
      << "Calculating next valid notification time...";

  /* default notification interval */
  double interval_to_use{_notification_interval};

  logger(dbg_notifications, most) << "Default interval: " << interval_to_use;

  /*
   * search all the escalation entries for valid matches for this service (at
   * its current notification number)
   */
  for (std::shared_ptr<escalation>& e : get_escalations()) {
    /* interval < 0 means to use non-escalated interval */
    if (e->get_notification_interval() < 0.0)
      continue;

    /* skip this entry if it isn't appropriate */
    if (!is_valid_escalation_for_notification(e, notification_option_none))
      continue;

    logger(dbg_notifications, most)
        << "Found a valid escalation w/ interval of "
        << e->get_notification_interval();

    /*
     * if we haven't used a notification interval from an escalation yet,
     * use this one
     */
    if (!have_escalated_interval) {
      have_escalated_interval = true;
      interval_to_use = e->get_notification_interval();
    }

    /* else use the shortest of all valid escalation intervals */
    else if (e->get_notification_interval() < interval_to_use)
      interval_to_use = e->get_notification_interval();

    logger(dbg_notifications, most) << "New interval: " << interval_to_use;
  }

  /*
   * if notification interval is 0, we shouldn't send any more problem
   * notifications (unless service is volatile)
   */
  if (interval_to_use == 0.0 && !get_is_volatile())
    set_no_more_notifications(true);
  else
    set_no_more_notifications(false);

  logger(dbg_notifications, most) << "Interval used for calculating next valid "
                                     "notification time: "
                                  << interval_to_use;

  /* calculate next notification time */
  time_t next_notification{
      offset +
      static_cast<time_t>(interval_to_use * config->interval_length())};

  return next_notification;
}

/*
 * checks to see if a service escalation entry is a match for the current
 * service notification
 */
bool service::is_valid_escalation_for_notification(
    std::shared_ptr<escalation> e,
    int options) const {
  uint32_t notification_number;
  time_t current_time;

  logger(dbg_functions, basic)
      << "service::is_valid_escalation_for_notification()";

  /* get the current time */
  time(&current_time);

  /*
   * if this is a recovery, really we check for who got notified about a
   * previous problem
   */
  if (_current_state == service::state_ok)
    notification_number = get_notification_number() - 1;
  else
    notification_number = get_notification_number();

  /* find the service this escalation entry is associated with */
  if (e->notifier_ptr != this)
    return false;

  /*** EXCEPTION ***/
  /* broadcast options go to everyone, so this escalation is valid */
  if (options & notification_option_broadcast)
    return true;

  /* skip this escalation if it happens later */
  if (e->get_first_notification() > notification_number)
    return false;

  /* skip this escalation if it has already passed */
  if (e->get_last_notification() != 0 &&
      e->get_last_notification() < notification_number)
    return false;

  /*
   * skip this escalation if it has a timeperiod and the current time isn't
   * valid
   */
  if (!e->get_escalation_period().empty() &&
      !check_time_against_period(current_time, e->escalation_period_ptr))
    return false;

  /* skip this escalation if the state options don't match */
  if (_current_state == service::state_ok &&
      !e->get_escalate_on(recovery))
    return false;
  else if (_current_state == service::state_warning &&
           !e->get_escalate_on(warning))
    return false;
  else if (_current_state == service::state_unknown &&
           !e->get_escalate_on(unknown))
    return false;
  else if (_current_state == service::state_critical &&
           !e->get_escalate_on(critical))
    return false;

  return true;
}

/* tests whether or not a service's check results are fresh */
bool service::is_result_fresh(
      time_t current_time,
      int log_this) {
  int freshness_threshold;
  time_t expiration_time = 0L;
  int days = 0;
  int hours = 0;
  int minutes = 0;
  int seconds = 0;
  int tdays = 0;
  int thours = 0;
  int tminutes = 0;
  int tseconds = 0;

  logger(dbg_checks, most)
    << "Checking freshness of service '" << this->get_description()
    << "' on host '" << this->get_hostname() << "'...";

  /* use user-supplied freshness threshold or auto-calculate a freshness threshold to use? */
  if (get_freshness_threshold() == 0) {
    if (get_state_type() == hard ||
        this->_current_state == service::state_ok)
      freshness_threshold = static_cast<int>(
          (get_check_interval() * config->interval_length()) +
          get_latency() + config->additional_freshness_latency());
    else
      freshness_threshold = static_cast<int>(
          this->get_retry_interval() * config->interval_length() +
          get_latency() + config->additional_freshness_latency());
  } else
    freshness_threshold = this->get_freshness_threshold();

  logger(dbg_checks, most)
    << "Freshness thresholds: service="
    << this->get_freshness_threshold()
    << ", use=" << freshness_threshold;

  /* calculate expiration time */
  /* CHANGED 11/10/05 EG - program start is only used in expiration time calculation if > last check AND active checks are enabled, so active checks can become stale immediately upon program startup */
  /* CHANGED 02/25/06 SG - passive checks also become stale, so remove dependence on active check logic */
  if (!this->get_has_been_checked())
    expiration_time = (time_t)(event_start + freshness_threshold);
  /* CHANGED 06/19/07 EG - Per Ton's suggestion (and user requests), only use program start time over last check if no specific threshold has been set by user.  Otheriwse use it.  Problems can occur if Engine is restarted more frequently that freshness threshold intervals (services never go stale). */
  /* CHANGED 10/07/07 EG - Only match next condition for services that have active checks enabled... */
  /* CHANGED 10/07/07 EG - Added max_service_check_spread to expiration time as suggested by Altinity */
  else if (this->get_checks_enabled()
           && event_start > get_last_check()
           && this->get_freshness_threshold() == 0)
    expiration_time
      = (time_t)(event_start + freshness_threshold
                 + (config->max_service_check_spread()
                    * config->interval_length()));
  else
    expiration_time
      = (time_t)(get_last_check() + freshness_threshold);

  logger(dbg_checks, most)
    << "HBC: " << this->get_has_been_checked()
    << ", PS: " << program_start
    << ", ES: " << event_start
    << ", LC: " << get_last_check()
    << ", CT: " << current_time
    << ", ET: " << expiration_time;

  /* the results for the last check of this service are stale */
  if (expiration_time < current_time) {

    get_time_breakdown(
      (current_time - expiration_time),
      &days,
      &hours,
      &minutes,
      &seconds);
    get_time_breakdown(
      freshness_threshold,
      &tdays,
      &thours,
      &tminutes,
      &tseconds);

    /* log a warning */
    if (log_this)
      logger(log_runtime_warning, basic)
        << "Warning: The results of service '" << this->get_description()
        << "' on host '" << this->get_hostname() << "' are stale by "
        << days << "d " << hours << "h " << minutes << "m " << seconds
        << "s (threshold=" << tdays << "d " << thours << "h " << tminutes
        << "m " << tseconds << "s).  I'm forcing an immediate check "
        "of the service.";

    logger(dbg_checks, more)
      << "Check results for service '" << this->get_description()
      << "' on host '" << this->get_hostname() << "' are stale by "
      << days << "d " << hours << "h " << minutes << "m " << seconds
      << "s (threshold=" << tdays << "d " << thours << "h " << tminutes
      << "m " << tseconds << "s).  Forcing an immediate check of "
      "the service...";

    return false;
  }

  logger(dbg_checks, more)
    << "Check results for service '" << this->get_description()
    << "' on host '" << this->get_hostname() << "' are fresh.";

  return true;
}

/* handles the details for a service when flap detection is disabled (globally or per-service) */
void service::handle_flap_detection_disabled() {
  logger(dbg_functions, basic)
    << "handle_service_flap_detection_disabled()";

  /* if the service was flapping, remove the flapping indicator */
  if (get_is_flapping()) {
    set_is_flapping(false);

    /* delete the original comment we added earlier */
    if (this->flapping_comment_id != 0)
      comment::delete_comment(this->flapping_comment_id);
    this->flapping_comment_id = 0;

    /* log a notice - this one is parsed by the history CGI */
    logger(log_info_message, basic)
      << "SERVICE FLAPPING ALERT: " << this->get_hostname()
      << ";" << this->get_description()
      << ";DISABLED; Flap detection has been disabled";

    /* send data to event broker */
    broker_flapping_data(
      NEBTYPE_FLAPPING_STOP,
      NEBFLAG_NONE,
      NEBATTR_FLAPPING_STOP_DISABLED,
      SERVICE_FLAPPING,
      this,
      get_percent_state_change(),
      0.0,
      0.0,
      nullptr);

    /* send a notification */
    this->notify(
      reason_flappingdisabled,
      "",
      "",
      notification_option_none);

    /* should we send a recovery notification? */
    notify(reason_recovery, "", "", notification_option_none);
  }

  /* update service status */
  update_status(false);
}

bool service::get_is_volatile() const {
  return _is_volatile;
}

void service::set_is_volatile(bool vol) {
  _is_volatile = vol;
}

std::list<servicegroup*> const& service::get_parent_groups() const {
  return _servicegroups;
}

std::list<servicegroup*>& service::get_parent_groups() {
  return _servicegroups;
}

timeperiod* service::get_notification_timeperiod() const {
  /* if the service has no notification period, inherit one from the host */
  return notification_period_ptr ? notification_period_ptr
                                 : _host_ptr->notification_period_ptr;
}

/* handles asynchronous service check results */
/* checks service dependencies */
uint64_t service::check_dependencies(
  int dependency_type) {
  service* temp_service = nullptr;
  int state =  service::state_ok;
  time_t current_time = 0L;

  logger(dbg_functions, basic)
    << "check_service_dependencies()";

  std::pair<std::string, std::string>
    id(_hostname, _description);
  servicedependency_mmap const& dependencies(
    state::instance().servicedependencies());
  for (servicedependency_mmap::const_iterator
         it(dependencies.find(id)), end(dependencies.end());
       it != end && it->first == id;
       ++it) {
    servicedependency* temp_dependency(&*it->second);

    /* only check dependencies of the desired type (notification or execution) */
    if (temp_dependency->get_dependency_type() != dependency_type)
      continue;

    /* find the service we depend on... */
    if ((temp_service = temp_dependency->master_service_ptr) == nullptr)
      continue;

    /* skip this dependency if it has a timeperiod and the current time isn't valid */
    time(&current_time);
    if (!check_time_against_period(
      current_time,
      temp_dependency->dependency_period_ptr))
      return DEPENDENCIES_OK;

    /* get the status to use (use last hard state if its currently in a soft state) */
    if (temp_service->get_state_type() == notifier::soft
      && !config->soft_state_dependencies())
      state = temp_service->get_last_hard_state();
    else
      state = temp_service->get_current_state();

    /* is the service we depend on in state that fails the dependency tests? */
    if (state ==  service::state_ok && temp_dependency->get_fail_on_ok())
      return DEPENDENCIES_FAILED;
    if (state ==  service::state_warning
      && temp_dependency->get_fail_on_warning())
      return DEPENDENCIES_FAILED;
    if (state ==  service::state_unknown
      && temp_dependency->get_fail_on_unknown())
      return DEPENDENCIES_FAILED;
    if (state ==  service::state_critical
      && temp_dependency->get_fail_on_critical())
      return DEPENDENCIES_FAILED;
    if ((state ==  service::state_ok && !temp_service->get_has_been_checked())
      && temp_dependency->get_fail_on_pending())
      return DEPENDENCIES_FAILED;

    /* immediate dependencies ok at this point - check parent dependencies if necessary */
    if (temp_dependency->get_inherits_parent()) {
      if (temp_service->check_dependencies(
        dependency_type) != DEPENDENCIES_OK)
        return DEPENDENCIES_FAILED;
    }
  }
  return DEPENDENCIES_OK;
}

/* check for services that never returned from a check... */
void service::check_for_orphaned() {
  time_t current_time{0L};
  time_t expected_time{0L};

  logger(dbg_functions, basic)
    << "check_for_orphaned_services()";

  /* get the current time */
  time(&current_time);

  /* check all services... */
  for (service_map::iterator
         it(service::services.begin()),
         end(service::services.end());
       it != end;
       ++it) {

    /* skip services that are not currently executing */
    if (!it->second->get_is_executing())
      continue;

    /* determine the time at which the check results should have come in (allow 10 minutes slack time) */
    expected_time
      = (time_t)(it->second->get_next_check() + it->second->get_latency()
      + config->service_check_timeout()
      + config->check_reaper_interval() + 600);

    /* this service was supposed to have executed a while ago, but for some reason the results haven't come back in... */
    if (expected_time < current_time) {

      /* log a warning */
      logger(log_runtime_warning, basic)
        << "Warning: The check of service '"
        << it->first.second << "' on host '"
        << it->first.first << "' looks like it was orphaned "
                              "(results never came back).  I'm scheduling an immediate check "
                              "of the service...";

      logger(dbg_checks, more)
        << "Service '" << it->first.second
        << "' on host '" << it->first.first
        << "' was orphaned, so we're scheduling an immediate check...";

      /* decrement the number of running service checks */
      if (currently_running_service_checks > 0)
        currently_running_service_checks--;

      /* disable the executing flag */
      it->second->set_is_executing(false);

      /* schedule an immediate check of the service */
      it->second->schedule_check(current_time, CHECK_OPTION_ORPHAN_CHECK);
    }
  }
}

/* check freshness of service results */
void service::check_result_freshness() {
  time_t current_time{0L};

  logger(dbg_functions, basic)
    << "check_service_result_freshness()";
  logger(dbg_checks, more)
    << "Checking the freshness of service check results...";

  /* bail out if we're not supposed to be checking freshness */
  if (!config->check_service_freshness()) {
    logger(dbg_checks, more)
      << "Service freshness checking is disabled.";
    return;
  }
  /* get the current time */
  time(&current_time);

  /* check all services... */
  for (service_map::iterator
         it(service::services.begin()),
         end(service::services.end());
       it != end;
       ++it) {

    /* skip services we shouldn't be checking for freshness */
    if (!it->second->get_check_freshness())
      continue;

    /* skip services that are currently executing (problems here will be caught by orphaned service check) */
    if (it->second->get_is_executing())
      continue;

    /* skip services that have both active and passive checks disabled */
    if (!it->second->get_checks_enabled()
      && !it->second->get_accept_passive_checks())
      continue;

    /* skip services that are already being freshened */
    if (it->second->is_being_freshened)
      continue;

    // See if the time is right...
    {
      timezone_locker lock(it->second->get_timezone());
      if (!check_time_against_period(
        current_time,
        it->second->check_period_ptr))
        continue ;
    }

    /* EXCEPTION */
    /* don't check freshness of services without regular check intervals if we're using auto-freshness threshold */
    if (it->second->get_check_interval() == 0 &&
      it->second->get_freshness_threshold() == 0)
      continue;

    /* the results for the last check of this service are stale! */
    if (!it->second->is_result_fresh(current_time, true)) {

      /* set the freshen flag */
      it->second->is_being_freshened = true;

      /* schedule an immediate forced check of the service */
      it->second->schedule_check(
        current_time,
        CHECK_OPTION_FORCE_EXECUTION | CHECK_OPTION_FRESHNESS_CHECK);
    }
  }
  return;
}

std::string const& service::get_current_state_as_string() const {
  return tab_service_states[get_current_state()].second;
}

bool service::get_notify_on_current_state() const {
  notification_flag type[]{ok, warning, critical, unknown};
  return get_notify_on(type[get_current_state()]);
}

bool service::is_in_downtime() const {
  return get_scheduled_downtime_depth() > 0 || _host_ptr->get_scheduled_downtime_depth() > 0;
}

void service::set_host_ptr(host* h) {
  _host_ptr = h;
}

host const* service::get_host_ptr() const {
  return _host_ptr;
}

host* service::get_host_ptr() {
  return _host_ptr;
}

void service::resolve(int& w, int& e) {
  int warnings{0}, errors{0};

  try {
    notifier::resolve(warnings, errors);
  }
  catch (std::exception const& e) {
    logger(log_verification_error, basic)
      << "Error: Service description '" << _description
      << "' of host '" << _hostname
      << "' has problem in its notifier part: " << e.what();
  }

  {
    /* check for a valid host */
    host_map::const_iterator it{host::hosts.find(_hostname)};

    /* we couldn't find an associated host! */

    if (it == host::hosts.end() || !it->second) {
      logger(log_verification_error, basic)
          << "Error: Host '" << _hostname
          << "' specified in service "
             "'"
          << _description << "' not defined anywhere!";
      errors++;
      set_host_ptr(nullptr);
    } else {
      /* save the host pointer for later */
      set_host_ptr(it->second.get());

      /* add a reverse link from the host to the service for faster lookups
       * later
       */
      it->second->services.insert({{_hostname, _description}, this});

      // Notify event broker.
      timeval tv(get_broker_timestamp(NULL));
      broker_relation_data(NEBTYPE_PARENT_ADD, NEBFLAG_NONE, NEBATTR_NONE,
                           get_host_ptr(), NULL, NULL, this, &tv);
    }
  }

  // Check for sane recovery options.
  if (get_notifications_enabled()
      && get_notify_on(notifier::recovery)
      && !get_notify_on(notifier::warning)
      && !get_notify_on(notifier::critical)) {
    logger(log_verification_error, basic)
      << "Warning: Recovery notification option in service '" << _description
      << "' for host '" << _hostname
      << "' doesn't make any sense - specify warning and /or critical "
         "options as well";
    warnings++;
  }

  // See if the notification interval is less than the check interval.
  if (get_notifications_enabled()
      && get_notification_interval()
      && get_notification_interval() < get_check_interval()) {
    logger(log_verification_error, basic)
      << "Warning: Service '" << _description << "' on host '"
      << _hostname << "'  has a notification interval less than "
         "its check interval!  Notifications are only re-sent after "
         "checks are made, so the effective notification interval will "
         "be that of the check interval.";
    warnings++;
  }

  /* check for illegal characters in service description */
  if (contains_illegal_object_chars(_description.c_str())) {
    logger(log_verification_error, basic)
      << "Error: The description string for service '"
      << _description << "' on host '" << _hostname
      << "' contains one or more illegal characters.";
    errors++;
  }

  w += warnings;
  e += errors;

  if (errors)
    throw engine_error() << "Cannot resolve service '" << _description
                         << "' of host '" << _hostname << "'";
}

bool service::get_host_problem_at_last_check() const {
  return _host_problem_at_last_check;
}

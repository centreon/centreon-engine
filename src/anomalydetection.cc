/*
** Copyright 2020 Centreon
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

#include "com/centreon/engine/anomalydetection.hh"
#include <cmath>
#include <cstring>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/macros/grab_host.hh"
#include "com/centreon/engine/macros/grab_service.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/exceptions/interruption.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/**
 *  Anomaly detection constructor
 *
 *  @param[in] host_name                    Name of the host this
 *                                          service is running on.
 *  @param[in] description                  Service description.
 *  @param[in] display_name                 Display name.
 *  @param[in] dependent_service            Dependent service
 *  @param[in] metric_name                  Metric to consider.
 *  @param[in] thresholds_file              Full path of the file containing
 *                                          metric thresholds.
 *  @param[in] status_change                Should we follow the thresholds file
 *                                          to determine status.
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
anomalydetection::anomalydetection(std::string const& hostname,
                                   std::string const& description,
                                   std::string const& display_name,
                                   service* dependent_service,
                                   std::string const& metric_name,
                                   std::string const& thresholds_file,
                                   bool status_change,
                                   bool checks_enabled,
                                   bool accept_passive_checks,
                                   enum service::service_state initial_state,
                                   uint32_t check_interval,
                                   uint32_t retry_interval,
                                   uint32_t notification_interval,
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
    : service{hostname,
              description,
              display_name,
              "",
              checks_enabled,
              accept_passive_checks,
              initial_state,
              check_interval,
              retry_interval,
              notification_interval,
              max_attempts,
              first_notification_delay,
              recovery_notification_delay,
              notification_period,
              notifications_enabled,
              is_volatile,
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
      _dependent_service{dependent_service},
      _metric_name{metric_name},
      _thresholds_file{thresholds_file},
      _status_change{status_change} {}

/**
 *  Add a new anomalydetection to the list in memory.
 *
 *  @param[in] host_name                    Name of the host this
 *                                          service is running on.
 *  @param[in] description                  Service description.
 *  @param[in] display_name                 Display name.
 *  @param[in] dependent_service_id         Dependent service id.
 *  @param[in] metric_name                  Metric to consider.
 *  @param[in] thresholds_file,             fullname to the thresholds file.
 *  @param[in] status_change,               should we follow the thresholds file
 *                                          to determine status.
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
com::centreon::engine::anomalydetection* add_anomalydetection(
    uint64_t host_id,
    uint64_t service_id,
    std::string const& host_name,
    std::string const& description,
    std::string const& display_name,
    uint64_t dependent_service_id,
    std::string const& metric_name,
    std::string const& thresholds_file,
    bool status_change,
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
  } else if (!host_name.empty()) {
    uint64_t hid = get_host_id(host_name);
    if (hid != host_id) {
      logger(log_config_error, basic)
          << "Error: host id (" << host_id << ") of host ('" << host_name
          << "') of anomaly detection service '" << description
          << "' has a conflict between config does not match with the config "
             "id ("
          << hid << ")";
      return nullptr;
    }
  }

  auto it = service::services_by_id.find({host_id, dependent_service_id});
  if (it == service::services_by_id.end()) {
    logger(log_config_error, basic) << "Error: Dependent service does not exist";
    return nullptr;
  }
  service* dependent_service = it->second.get();

  if (metric_name.empty()) {
    logger(log_config_error, basic)
        << "Error: metric name must be provided for an anomaly detection "
           "service (host_id:"
        << host_id << ", service_id:" << service_id << ")";
    return nullptr;
  }

  if (thresholds_file.empty()) {
    logger(log_config_error, basic)
        << "Error: thresholds file must be provided for an anomaly detection "
           "service (host_id:"
        << host_id << ", service_id:" << service_id << ")";
    return nullptr;
  }

  // Check values.
  if (max_attempts <= 0 || check_interval < 0 || retry_interval <= 0 ||
      notification_interval < 0) {
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
  std::shared_ptr<anomalydetection> obj{std::make_shared<anomalydetection>(
      host_name, description, display_name.empty() ? description : display_name,
      dependent_service, metric_name, thresholds_file, status_change,
      checks_enabled, accept_passive_checks, initial_state, check_interval,
      retry_interval, notification_interval, max_attempts,
      first_notification_delay, recovery_notification_delay,
      notification_period, notifications_enabled, is_volatile, check_period,
      event_handler, event_handler_enabled, notes, notes_url, action_url,
      icon_image, icon_image_alt, flap_detection_enabled, low_flap_threshold,
      high_flap_threshold, check_freshness, freshness_threshold,
      obsess_over_service, timezone)};
  try {
    obj->set_acknowledgement_type(ACKNOWLEDGEMENT_NONE);
    obj->set_check_options(CHECK_OPTION_NONE);
    uint32_t flap_detection_on;
    flap_detection_on = none;
    flap_detection_on |=
        (flap_detection_on_critical > 0 ? notifier::critical : 0);
    flap_detection_on |= (flap_detection_on_ok > 0 ? notifier::ok : 0);
    flap_detection_on |=
        (flap_detection_on_unknown > 0 ? notifier::unknown : 0);
    flap_detection_on |=
        (flap_detection_on_warning > 0 ? notifier::warning : 0);
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
    notify_on |= (notify_recovery > 0 ? notifier::ok : 0);
    notify_on |= (notify_unknown > 0 ? notifier::unknown : 0);
    notify_on |= (notify_warning > 0 ? notifier::warning : 0);
    obj->set_notify_on(notify_on);
    obj->set_process_performance_data(process_perfdata > 0);
    obj->set_retain_nonstatus_information(retain_nonstatus_information > 0);
    obj->set_retain_status_information(retain_status_information > 0);
    obj->set_should_be_scheduled(true);
    uint32_t stalk_on = (stalk_on_critical ? notifier::critical : 0) |
                        (stalk_on_ok ? notifier::ok : 0) |
                        (stalk_on_unknown ? notifier::unknown : 0) |
                        (stalk_on_warning ? notifier::warning : 0);
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

service* anomalydetection::get_dependent_service() const {
  return _dependent_service;
}

void anomalydetection::set_dependent_service(service* svc) {
  _dependent_service = svc;
}

void anomalydetection::set_metric_name(std::string const& name) {
  _metric_name = name;
}

void anomalydetection::set_thresholds_file(std::string const& file) {
  _thresholds_file = file;
}

/*
 * forks a child process to run a service check, but does not wait for the
 * service check result
 */
int anomalydetection::run_async_check(int check_options,
                                      double latency,
                                      bool scheduled_check,
                                      bool reschedule_check,
                                      bool* time_is_valid,
                                      time_t* preferred_time) noexcept {
  logger(dbg_functions, basic)
      << "anomalydetection::run_async_check, check_options=" << check_options
      << ", latency=" << latency << ", scheduled_check=" << scheduled_check
      << ", reschedule_check=" << reschedule_check;

  logger(dbg_checks, basic)
      << "** Running async check of service '" << get_description()
      << "' on host '" << get_hostname() << "'...";

  // Check if the service is viable now.
  if (verify_check_viability(check_options, time_is_valid, preferred_time) ==
      ERROR)
    return ERROR;

  // Send broker event.
  timeval start_time;
  timeval end_time;
  memset(&start_time, 0, sizeof(start_time));
  memset(&end_time, 0, sizeof(end_time));
  int res =
      broker_service_check(NEBTYPE_SERVICECHECK_ASYNC_PRECHECK, NEBFLAG_NONE,
                           NEBATTR_NONE, this, checkable::check_active,
                           start_time, end_time, get_check_command().c_str(),
                           get_latency(), 0.0, 0, false, 0, nullptr, nullptr);

  // Service check was cancel by NEB module. reschedule check later.
  if (NEBERROR_CALLBACKCANCEL == res) {
    if (preferred_time != nullptr)
      *preferred_time += static_cast<time_t>(get_check_interval() *
                                             config->interval_length());
    logger(log_runtime_error, basic)
        << "Error: Some broker module cancelled check of service '"
        << get_description() << "' on host '" << get_hostname();
    return ERROR;
  }
  // Service check was override by NEB module.
  else if (NEBERROR_CALLBACKOVERRIDE == res) {
    logger(dbg_functions, basic)
        << "Some broker module overrode check of service '" << get_description()
        << "' on host '" << get_hostname() << "' so we'll bail out";
    return OK;
  }

  // Checking starts.
  logger(dbg_checks, basic) << "Checking service '" << get_description()
                            << "' on host '" << get_hostname() << "'...";

  // Clear check options.
  if (scheduled_check)
    set_check_options(CHECK_OPTION_NONE);

  // Update latency for event broker and macros.
  double old_latency(get_latency());
  set_latency(latency);

  // Get current host and service macros.
  nagios_macros macros;
  grab_host_macros_r(&macros, get_host_ptr());
  grab_service_macros_r(&macros, this);
  std::string tmp;
  get_raw_command_line_r(&macros, get_check_command_ptr(),
                         get_check_command().c_str(), tmp, 0);

  // Time to start command.
  gettimeofday(&start_time, nullptr);

  // Update the number of running service checks.
  ++currently_running_service_checks;

  // Set the execution flag.
  set_is_executing(true);

  // Init check result info.
  check_result check_result_info(
      service_check, get_host_id(), get_service_id(),
      checkable::check_active, check_options, reschedule_check, latency,
      start_time, start_time, false, true, service::state_ok, "");

  std::ostringstream oss;
  oss << "Anomaly detection on metric '" << _metric_name << "', from service '" << _dependent_service->get_description() << "' on host '" << get_hostname() << "'";
  // Send event broker.
  res = broker_service_check(
      NEBTYPE_SERVICECHECK_INITIATE, NEBFLAG_NONE, NEBATTR_NONE, this,
      checkable::check_active, start_time, end_time,
      get_check_command().c_str(), get_latency(), 0.0,
      config->service_check_timeout(), false, 0, oss.str().c_str(), nullptr);

  // Restore latency.
  set_latency(old_latency);

  // Service check was override by neb_module.
  if (NEBERROR_CALLBACKOVERRIDE == res) {
    clear_volatile_macros_r(&macros);
    return OK;
  }

  // Update statistics.
  update_check_stats(scheduled_check
                         ? ACTIVE_SCHEDULED_SERVICE_CHECK_STATS
                         : ACTIVE_ONDEMAND_SERVICE_CHECK_STATS,
                     start_time.tv_sec);

  std::string perfdata = string::extract_perfdata(
      _dependent_service->get_perf_data(), _metric_name);
  std::tuple<service::service_state, double, double, double> pd = parse_perfdata(perfdata);

  oss.str("");
  check_result_info.set_early_timeout(false);
  if (std::get<0>(pd) == service::state_ok)
    check_result_info.set_exited_ok(true);

  check_result_info.set_return_code(std::get<0>(pd));
  oss << perfdata << ' ' << _metric_name << "_lower_thresholds=" << std::get<2>(pd)
    << ' ' << _metric_name << "_upper_thresholds=" << std::get<3>(pd);
  check_result_info.set_output(oss.str());

  timestamp now(timestamp::now());

  // Update check result.
  timeval tv;
  tv.tv_sec = now.to_seconds();
  tv.tv_usec = now.to_useconds() - tv.tv_sec * 1000000ull;
  check_result_info.set_finish_time(tv);

  handle_async_check_result(&check_result_info);

  // Cleanup.
  clear_volatile_macros_r(&macros);
  return OK;
}

commands::command* anomalydetection::get_check_command_ptr() const {
  return _dependent_service->get_check_command_ptr();
}

std::tuple<service::service_state, double, double, double>
anomalydetection::parse_perfdata(std::string const& perfdata) {
  size_t pos = perfdata.find_last_of("=");
  if (pos == std::string::npos)
    return std::tuple<service::service_state, double, double, double>(service::state_unknown, NAN, NAN, NAN);
  pos++;
  double value = std::strtod(perfdata.c_str(), nullptr);
  service::service_state status;

  if (!_status_change)
    status = service::state_ok;
  else {
    //FIXME DBR We have to read thresholds...
    status = service::state_ok;
  }
  return std::make_tuple(status, value, NAN, NAN);
}

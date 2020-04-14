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

#include <cmath>
#include <cstring>
#include <limits>
#include "com/centreon/engine/anomalydetection.hh"
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
anomalydetection::anomalydetection(uint64_t host_id,
                                   uint64_t service_id,
                                   std::string const& hostname,
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
    : service{hostname,                    description,
              display_name,                "",
              checks_enabled,              accept_passive_checks,
              initial_state,               check_interval,
              retry_interval,              notification_interval,
              max_attempts,                first_notification_delay,
              recovery_notification_delay, notification_period,
              notifications_enabled,       is_volatile,
              dependent_service->get_check_period(), event_handler,
              event_handler_enabled,       notes,
              notes_url,                   action_url,
              icon_image,                  icon_image_alt,
              flap_detection_enabled,      low_flap_threshold,
              high_flap_threshold,         check_freshness,
              freshness_threshold,         obsess_over,
              timezone},
      _dependent_service{dependent_service},
      _metric_name{metric_name},
      _thresholds_file{thresholds_file},
      _status_change{status_change},
      _thresholds_file_viable{false} {
  set_host_id(host_id);
  set_service_id(service_id);
  init_thresholds();
}

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
             "id (" << hid << ")";
      return nullptr;
    }
  }

  auto it = service::services_by_id.find({host_id, dependent_service_id});
  if (it == service::services_by_id.end()) {
    logger(log_config_error, basic)
        << "Error: Dependent service " << dependent_service_id
        << " does not exist (anomaly detection " << service_id << ")";
    return nullptr;
  }
  service* dependent_service = it->second.get();

  if (metric_name.empty()) {
    logger(log_config_error, basic)
        << "Error: metric name must be provided for an anomaly detection "
           "service (host_id:" << host_id << ", service_id:" << service_id
        << ")";
    return nullptr;
  }

  if (thresholds_file.empty()) {
    logger(log_config_error, basic)
        << "Error: thresholds file must be provided for an anomaly detection "
           "service (host_id:" << host_id << ", service_id:" << service_id
        << ")";
    return nullptr;
  }

  // Check values.
  if (max_attempts <= 0 || check_interval < 0 || retry_interval <= 0 ||
      notification_interval < 0) {
    logger(log_config_error, basic)
        << "Error: Invalid max_attempts, check_interval, retry_interval"
           ", or notification_interval value for service '" << description
        << "' on host '" << host_name << "'";
    return nullptr;
  }
  // Check if the service is already exist.
  std::pair<uint64_t, uint64_t> id(std::make_pair(host_id, service_id));
  if (is_service_exist(id)) {
    logger(log_config_error, basic) << "Error: Service '" << description
                                    << "' on host '" << host_name
                                    << "' has already been defined";
    return nullptr;
  }

  // Allocate memory.
  std::shared_ptr<anomalydetection> obj{std::make_shared<anomalydetection>(
      host_id,
      service_id,
      host_name,
      description,
      display_name.empty() ? description : display_name,
      dependent_service,
      metric_name,
      thresholds_file,
      status_change,
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
      obsess_over_service,
      timezone)};
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
  }
  catch (...) {
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
  std::lock_guard<std::mutex> lock(_thresholds_m);
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

  logger(dbg_checks, basic) << "** Running async check of anomalydetection '"
                            << get_description() << "' on host '"
                            << get_hostname() << "'...";

  // Check if the service is viable now.
  if (!verify_check_viability(check_options, time_is_valid, preferred_time))
    return ERROR;

  // Send broker event.
  timeval start_time = {0, 0};
  timeval end_time = {0, 0};
  int res = broker_service_check(NEBTYPE_SERVICECHECK_ASYNC_PRECHECK,
                                 NEBFLAG_NONE,
                                 NEBATTR_NONE,
                                 this,
                                 checkable::check_active,
                                 start_time,
                                 end_time,
                                 get_check_command().c_str(),
                                 get_latency(),
                                 0.0,
                                 0,
                                 false,
                                 0,
                                 nullptr,
                                 nullptr);

  // Anomalydetection check was cancelled by NEB module. reschedule check later.
  if (NEBERROR_CALLBACKCANCEL == res) {
    if (preferred_time != nullptr)
      *preferred_time +=
          static_cast<time_t>(get_check_interval() * config->interval_length());
    logger(log_runtime_error, basic)
        << "Error: Some broker module cancelled check of anomalydetection '"
        << get_description() << "' on host '" << get_hostname();
    return ERROR;
  }
  // Anomalydetection check was override by NEB module.
  else if (NEBERROR_CALLBACKOVERRIDE == res) {
    logger(dbg_functions, basic)
        << "Some broker module overrode check of anomalydetection '"
        << get_description() << "' on host '" << get_hostname()
        << "' so we'll bail out";
    return OK;
  }

  // Checking starts.
  logger(dbg_checks, basic) << "Checking anomalydetection '"
                            << get_description() << "' on host '"
                            << get_hostname() << "'...";

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
  get_raw_command_line_r(
      &macros, get_check_command_ptr(), get_check_command().c_str(), tmp, 0);

  // Time to start command.
  gettimeofday(&start_time, nullptr);

  // Update the number of running service checks.
  ++currently_running_service_checks;
  logger(dbg_checks, basic) << "Current running service checks: "
                            << currently_running_service_checks;

  // Set the execution flag.
  set_is_executing(true);

  std::ostringstream oss;
  oss << "Anomaly detection on metric '" << _metric_name << "', from service '"
      << _dependent_service->get_description() << "' on host '"
      << get_hostname() << "'";
  // Send event broker.
  res = broker_service_check(NEBTYPE_SERVICECHECK_INITIATE,
                             NEBFLAG_NONE,
                             NEBATTR_NONE,
                             this,
                             checkable::check_active,
                             start_time,
                             end_time,
                             get_check_command().c_str(),
                             get_latency(),
                             0.0,
                             config->service_check_timeout(),
                             false,
                             0,
                             oss.str().c_str(),
                             nullptr);

  // Restore latency.
  set_latency(old_latency);

  // Service check was override by neb_module.
  if (NEBERROR_CALLBACKOVERRIDE == res) {
    clear_volatile_macros_r(&macros);
    return OK;
  }

  // Update statistics.
  update_check_stats(scheduled_check ? ACTIVE_SCHEDULED_SERVICE_CHECK_STATS
                                     : ACTIVE_ONDEMAND_SERVICE_CHECK_STATS,
                     start_time.tv_sec);

  std::string perfdata = string::extract_perfdata(
      _dependent_service->get_perf_data(), _metric_name);

  std::string without_thresholds(string::remove_thresholds(perfdata));
  std::tuple<service::service_state, double, std::string, double, double> pd =
      parse_perfdata(without_thresholds, start_time.tv_sec);
  size_t pos = without_thresholds.find(';');
  if (pos != std::string::npos)
    without_thresholds = without_thresholds.substr(pos);
  else
    without_thresholds = "";

  // Init check result info.
  std::unique_ptr<check_result> check_result_info(
      new check_result(service_check,
                       this,
                       checkable::check_active,
                       check_options,
                       reschedule_check,
                       latency,
                       start_time,
                       start_time,
                       false,
                       true,
                       service::state_ok,
                       ""));

  oss.str("");
  oss.setf(std::ios_base::fixed, std::ios_base::floatfield);
  oss.precision(2);
  check_result_info->set_early_timeout(false);
  check_result_info->set_exited_ok(true);
  if (std::get<0>(pd) == service::state_ok)
    oss << "OK: Regular activity, " << _metric_name << '=' << std::get<1>(pd)
        << std::get<2>(pd) << " |";
  else if (std::get<0>(pd) == service::state_unknown &&
           std::isnan(std::get<1>(pd)))
    oss << "UNKNOWN: Unknown activity, " << _metric_name
        << " did not return any values| ";
  else {
    oss << "NON-OK: Unusual activity, the actual value of " << _metric_name
        << " is " << std::get<1>(pd) << std::get<2>(pd);
    if (!std::isnan(std::get<3>(pd)) && !std::isnan(std::get<4>(pd)))
      oss << " which is outside the forecasting range [" << std::get<3>(pd)
          << " ; " << std::get<4>(pd) << "] |";
    else
      oss << " and the forecasting range is unknown |";
  }

  check_result_info->set_return_code(std::get<0>(pd));
  oss << perfdata;
  if (!std::isnan(std::get<3>(pd))) {
    oss << ' ' << _metric_name << "_lower_thresholds=" << std::get<3>(pd) << std::get<2>(pd) << without_thresholds;
  }
  if (!std::isnan(std::get<4>(pd))) {
    oss << ' ' << _metric_name << "_upper_thresholds=" << std::get<4>(pd) << std::get<2>(pd) << without_thresholds;
  }
  check_result_info->set_output(oss.str());

  timestamp now(timestamp::now());

  // Update check result.
  timeval tv;
  gettimeofday(&tv, nullptr);
  check_result_info->set_finish_time(tv);

  // Queue check result.
  //handle_async_check_result(check_result_info.get());
  checks::checker::instance().add_check_result_to_reap(
      check_result_info.release());

  // Cleanup.
  clear_volatile_macros_r(&macros);

  return OK;
}

commands::command* anomalydetection::get_check_command_ptr() const {
  return _dependent_service->get_check_command_ptr();
}

/**
 * @brief Parse the given perfdata. The only metric parsed is the one whose name
 * is _metric_name.
 *
 * @param perfdata A string containing perfdata.
 *
 * @return A tuple containing the status, the value, its unit, the lower bound
 * and the upper bound
 */
std::tuple<service::service_state, double, std::string, double, double>
anomalydetection::parse_perfdata(std::string const& perfdata,
                                 time_t check_time) {
  std::lock_guard<std::mutex> lock(_thresholds_m);
  size_t pos = perfdata.find_last_of("=");
  /* If the perfdata is wrong. */
  if (pos == std::string::npos) {
    logger(log_runtime_error, basic) << "Error: Unable to parse perfdata '"
                                     << perfdata << "'";
    return std::make_tuple(service::state_unknown, NAN, "", NAN, NAN);
  }

  /* If the perfdata is good. */
  pos++;
  char* unit;

  double value = std::strtod(perfdata.c_str() + pos, &unit);
  char const* end = perfdata.c_str() + perfdata.size() - 1;
  size_t l = 0;
  /* If there is a unit, it starts at unit char* */
  while (unit + l <= end && unit[l] != ' ' && unit[l] != ';')
    ++l;
  std::string uom = std::string(unit, l);

  service::service_state status;

  if (!_thresholds_file_viable) {
    status = service::state_ok;
    if (_status_change) {
      logger(log_info_message, basic)
          << "The thresholds file is not viable (not available or not readable).";
    }
    return std::make_tuple(status, value, unit, NAN, NAN);
  }

  /* The check time is probably between two timestamps stored in _thresholds.
   *
   *   |                    d2 +
   *   |            dc+
   *   |   d1 +
   *   |
   *   +------+-------+--------+-->
   *         t1       tc       t2
   *
   * For both lower bound and upper bound, we get values d1 and d2
   * respectively corresponding to timestamp t1 and t2.
   * We have a check_time tc between them, and we would like a value dc
   * corresponding to this timestamp.
   *
   * The linear approximation gives the formula:
   *                       dc = (d2-d1) * (tc-t1) / (t2-t1) + d1
   */
  auto it2 = _thresholds.upper_bound(check_time);
  auto it1 = it2;
  if (it2 == _thresholds.end()) {
    logger(log_runtime_error, basic) << "Error: the thresholds file is too old "
                                        "compared to the check timestamp "
                                     << check_time;
    return std::make_tuple(service::state_unknown, value, uom, NAN, NAN);
  }
  if (it1 != _thresholds.begin())
    --it1;
  else {
    logger(log_runtime_error, basic)
        << "Error: timestamp " << check_time
        << " too old compared with the thresholds file";
    return std::make_tuple(service::state_unknown, value, uom, NAN, NAN);
  }

  /* Now it1.first <= check_time < it2.first */
  double upper = (it2->second.second - it1->second.second) *
                     (check_time - it1->first) / (it2->first - it1->first) +
                 it1->second.second;
  double lower = (it2->second.first - it1->second.first) *
                     (check_time - it1->first) / (it2->first - it1->first) +
                 it1->second.first;

  if (!_status_change)
    status = service::state_ok;
  else {
    if (std::isnan(value))
      status = service::state_unknown;
    else if (value >= lower && value <= upper)
      status = service::state_ok;
    else
      status = service::state_critical;
  }

  return std::make_tuple(status, value, uom, lower, upper);
}

void anomalydetection::init_thresholds() {
  std::lock_guard<std::mutex> lock(_thresholds_m);

  logger(log_info_message, basic) << "Trying to read thresholds file '"
                                 << _thresholds_file << "'";
  std::ifstream t(_thresholds_file);
  if (!t)
    return;

  std::stringstream buffer;
  buffer << t.rdbuf();
  std::string err;
  auto json = json11::Json::parse(buffer.str(), err);
  if (!err.empty()) {
    logger(log_config_error, basic) << "Error: the file '" << _thresholds_file
                                    << "' contains errors: " << err;
    return;
  }
  if (!json.is_array()) {
    logger(log_config_error, basic)
        << "Error: the file '" << _thresholds_file
        << "' is not a thresholds file. Its global structure is not an array.";
    return;
  }

  int count = 0;
  for (auto& item : json.array_items()) {
    uint64_t host_id, service_id;
    try {
      host_id = stoull(item["host_id"].string_value());
      service_id = stoull(item["service_id"].string_value());
    }
    catch (std::exception const& e) {
      logger(log_config_error, basic)
          << "Error: host_id and service_id must be strings containing integers: "
          << e.what();
      return;
    }
    if (host_id == get_host_id() && service_id == get_service_id() &&
        item["metric_name"].string_value() == _metric_name) {
      logger(log_info_message, basic)
          << "Filling thresholds in anomaly detection (host_id: "
          << get_host_id() << ", service_id: " << get_service_id()
          << ", metric: " << _metric_name << ")";
      auto predict = item["predict"];
      _thresholds.clear();
      for (auto& i : predict.array_items()) {
        time_t timestamp = static_cast<time_t>(i["timestamp"].number_value());
        double upper = i["upper"].number_value();
        double lower = i["lower"].number_value();
        _thresholds.emplace_hint(
            _thresholds.end(),
            std::make_pair(timestamp, std::make_pair(lower, upper)));
        count++;
      }
      break;
    }
  }
  if (count > 1) {
    logger(log_info_message, most) << "Number of rows in memory: " << count;
    _thresholds_file_viable = true;
  } else
    logger(log_info_message, most) << "Nothing in memory";
}

/**
 * @brief Update all the anomaly detection services concerned by one thresholds
 *        file. The file has already been parsed and is translated into json.
 *
 * @param filename The fullname of the file to parse.
 */
int anomalydetection::update_thresholds(const std::string& filename) {
  logger(log_info_message, most) << "Reading thresholds file '" << filename
                                 << "'.";
  std::ifstream t(filename);
  if (!t) {
    logger(log_config_error, basic)
        << "Error: Unable to read the thresholds file '" << filename << "'.";
    return -1;
  }

  std::stringstream buffer;
  buffer << t.rdbuf();
  std::string err;
  auto json = json11::Json::parse(buffer.str(), err);
  if (!err.empty()) {
    logger(log_config_error, basic) << "Error: The thresholds file '"
                                    << filename << "' should be a json file.";
    return -2;
  }

  if (!json.is_array()) {
    logger(log_config_error, basic)
        << "Error: the file '" << filename
        << "' is not a thresholds file. Its global structure is not an array.";
    return -3;
  }

  for (auto& item : json.array_items()) {
    uint64_t host_id, svc_id;
    try {
      host_id = stoull(item["host_id"].string_value());
      svc_id = stoull(item["service_id"].string_value());
    }
    catch (std::exception const& e) {
      logger(log_config_error, basic)
          << "Error: host_id and service_id must be strings containing integers: "
          << e.what();
      continue;
    }
    auto found = service::services_by_id.find({host_id, svc_id});
    if (found == service::services_by_id.end()) {
      logger(log_config_error, basic)
          << "Error: The thresholds file contains thresholds for the anomaly "
             "detection service (host_id: " << host_id
          << ", service_id: " << svc_id << ") that does not exist";
      continue;
    }
    std::shared_ptr<anomalydetection> ad =
        std::static_pointer_cast<anomalydetection>(found->second);
    const std::string& metric_name(item["metric_name"].string_value());
    if (ad->get_metric_name() != metric_name) {
      logger(log_config_error, basic)
          << "Error: The thresholds file contains thresholds for the anomaly "
             "detection service (host_id: " << ad->get_host_id()
          << ", service_id: " << ad->get_service_id() << ") with metric_name='"
          << metric_name << "' whereas the configured metric name is '"
          << ad->get_metric_name() << "'";
      continue;
    }
    logger(log_info_message, basic)
        << "Filling thresholds in anomaly detection (host_id: "
        << ad->get_host_id() << ", service_id: " << ad->get_service_id()
        << ", metric: " << ad->get_metric_name() << ")";

    auto predict = item["predict"];
    std::map<time_t, std::pair<double, double> > thresholds;
    for (auto& i : predict.array_items()) {
      time_t timestamp = static_cast<time_t>(i["timestamp"].number_value());
      double upper = i["upper"].number_value();
      double lower = i["lower"].number_value();
      thresholds.emplace_hint(
          thresholds.end(),
          std::make_pair(timestamp, std::make_pair(lower, upper)));
    }
    ad->set_thresholds(filename, std::move(thresholds));
  }
  return 0;
}

void anomalydetection::set_thresholds(
    const std::string& filename,
    std::map<time_t, std::pair<double, double> >&& thresholds) noexcept {
  std::lock_guard<std::mutex> _lock(_thresholds_m);
  _thresholds_file = filename, _thresholds = thresholds;
  _thresholds_file_viable = _thresholds.size() > 0;
}

void anomalydetection::set_status_change(bool status_change) {
  _status_change = status_change;
}

const std::string& anomalydetection::get_metric_name() const {
  return _metric_name;
}

const std::string& anomalydetection::get_thresholds_file() const {
  return _thresholds_file;
}

void anomalydetection::resolve(int& w, int& e) {
  set_check_period(_dependent_service->get_check_period());
  service::resolve(w, e);
}

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

#ifndef CCE_ANOMALYDETECTION_HH
#define CCE_ANOMALYDETECTION_HH

#include <nlohmann/json.hpp>
#include <map>
#include <mutex>
#include <tuple>

#include "com/centreon/engine/service.hh"

CCE_BEGIN()

class anomalydetection : public service {
  service* _dependent_service;
  std::string _metric_name;
  std::string _thresholds_file;
  bool _status_change;
  bool _thresholds_file_viable;
  std::map<time_t, std::pair<double, double> > _thresholds;
  std::mutex _thresholds_m;

 public:
  anomalydetection(uint64_t host_id,
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
                   std::string const& timezone);
  ~anomalydetection() = default;
  service* get_dependent_service() const;
  void set_dependent_service(service* svc);
  void set_metric_name(std::string const& name);
  void set_thresholds_file(std::string const& file);
  void set_thresholds(
      const std::string& filename,
      std::map<time_t, std::pair<double, double> >&& thresholds) noexcept;
  static int update_thresholds(const std::string& filename);
  virtual int run_async_check(int check_options,
                              double latency,
                              bool scheduled_check,
                              bool reschedule_check,
                              bool* time_is_valid,
                              time_t* preferred_time) noexcept override;
  commands::command* get_check_command_ptr() const;
  std::tuple<service::service_state, double, std::string, double, double>
  parse_perfdata(std::string const& perfdata, time_t check_time);
  void init_thresholds();
  void set_status_change(bool status_change);
  const std::string& get_metric_name() const;
  const std::string& get_thresholds_file() const;
  void resolve(int& w, int& e);
};
CCE_END()

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
    enum com::centreon::engine::service::service_state initial_state,
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
    bool obsess_over,
    std::string const& timezone);

#endif  // !CCE_ANOMALYDETECTION_HH

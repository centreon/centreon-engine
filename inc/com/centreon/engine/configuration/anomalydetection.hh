/*
** Copyright 2011-2013,2015-2017 Centreon
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

#ifndef CCE_CONFIGURATION_ANOMALYDETECTION_HH
#define CCE_CONFIGURATION_ANOMALYDETECTION_HH

#include <list>
#include <memory>
#include <set>
#include <utility>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/configuration/group.hh"
#include "com/centreon/engine/configuration/object.hh"
#include "com/centreon/engine/customvariable.hh"
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/opt.hh"

CCE_BEGIN()

namespace configuration {
class serviceextinfo;

class anomalydetection : public object {
 public:
  enum action_on {
    none = 0,
    ok = (1 << 0),
    warning = (1 << 1),
    unknown = (1 << 2),
    critical = (1 << 3),
    flapping = (1 << 4),
    downtime = (1 << 5)
  };
  typedef std::pair<uint64_t, uint64_t> key_type;

  anomalydetection();
  anomalydetection(anomalydetection const& other);
  ~anomalydetection() noexcept override;
  anomalydetection& operator=(anomalydetection const& other);
  bool operator==(anomalydetection const& other) const noexcept;
  bool operator!=(anomalydetection const& other) const noexcept;
  bool operator<(anomalydetection const& other) const noexcept;
  void check_validity() const override;
  key_type key() const;
  void merge(configuration::serviceextinfo const& obj);
  void merge(object const& obj) override;
  bool parse(char const* key, char const* value) override;

  std::string const& action_url() const noexcept;
  bool status_change() const noexcept;
  bool checks_active() const noexcept;
  bool checks_passive() const noexcept;
  std::string const& metric_name() const noexcept;
  std::string const& thresholds_file() const noexcept;
  bool check_freshness() const noexcept;
  unsigned int check_interval() const noexcept;
  set_string& contactgroups() noexcept;
  set_string const& contactgroups() const noexcept;
  bool contactgroups_defined() const noexcept;
  set_string& contacts() noexcept;
  set_string const& contacts() const noexcept;
  bool contacts_defined() const noexcept;
  map_customvar const& customvariables() const noexcept;
  map_customvar& customvariables() noexcept;
  std::string const& display_name() const noexcept;
  std::string const& event_handler() const noexcept;
  bool event_handler_enabled() const noexcept;
  unsigned int first_notification_delay() const noexcept;
  bool flap_detection_enabled() const noexcept;
  unsigned short flap_detection_options() const noexcept;
  unsigned int freshness_threshold() const noexcept;
  unsigned int high_flap_threshold() const noexcept;
  std::string& host_name() noexcept;
  const std::string& host_name() const noexcept;
  uint64_t host_id() const noexcept;
  bool set_host_id(uint64_t id);
  std::string const& icon_image() const noexcept;
  std::string const& icon_image_alt() const noexcept;
  unsigned int initial_state() const noexcept;
  bool is_volatile() const noexcept;
  unsigned int low_flap_threshold() const noexcept;
  unsigned int max_check_attempts() const noexcept;
  std::string const& notes() const noexcept;
  std::string const& notes_url() const noexcept;
  bool notifications_enabled() const noexcept;
  void notification_interval(unsigned int interval) noexcept;
  unsigned int notification_interval() const noexcept;
  bool notification_interval_defined() const noexcept;
  unsigned short notification_options() const noexcept;
  void notification_period(std::string const& period);
  std::string const& notification_period() const noexcept;
  bool notification_period_defined() const noexcept;
  bool obsess_over_service() const noexcept;
  bool process_perf_data() const noexcept;
  bool retain_nonstatus_information() const noexcept;
  bool retain_status_information() const noexcept;
  unsigned int retry_interval() const noexcept;
  unsigned int recovery_notification_delay() const noexcept;
  set_string& servicegroups() noexcept;
  set_string const& servicegroups() const noexcept;
  std::string& service_description() noexcept;
  std::string const& service_description() const noexcept;
  uint64_t service_id() const noexcept;
  bool set_service_id(uint64_t value);
  uint64_t dependent_service_id() const noexcept;
  bool set_dependent_service_id(uint64_t value);
  unsigned short stalking_options() const noexcept;
  void timezone(std::string const& time_zone);
  std::string const& timezone() const noexcept;
  bool timezone_defined() const noexcept;
  int get_acknowledgement_timeout() const noexcept;
  bool set_acknowledgement_timeout(int value);

 private:
  typedef bool (*setter_func)(anomalydetection&, char const*);

  bool _set_action_url(std::string const& value);
  bool _set_metric_name(std::string const& value);
  bool _set_thresholds_file(std::string const& value);
  bool _set_status_change(bool value);
  bool _set_checks_active(bool value);
  bool _set_checks_passive(bool value);
  bool _set_check_freshness(bool value);
  bool _set_check_interval(unsigned int value);
  bool _set_contactgroups(std::string const& value);
  bool _set_contacts(std::string const& value);
  bool _set_display_name(std::string const& value);
  bool _set_event_handler(std::string const& value);
  bool _set_event_handler_enabled(bool value);
  bool _set_failure_prediction_enabled(bool value);
  bool _set_failure_prediction_options(std::string const& value);
  bool _set_first_notification_delay(unsigned int value);
  bool _set_flap_detection_enabled(bool value);
  bool _set_flap_detection_options(std::string const& value);
  bool _set_freshness_threshold(unsigned int value);
  bool _set_high_flap_threshold(unsigned int value);
  bool _set_host_name(std::string const& value);
  bool _set_icon_image(std::string const& value);
  bool _set_icon_image_alt(std::string const& value);
  bool _set_initial_state(std::string const& value);
  bool _set_is_volatile(bool value);
  bool _set_low_flap_threshold(unsigned int value);
  bool _set_max_check_attempts(unsigned int value);
  bool _set_notes(std::string const& value);
  bool _set_notes_url(std::string const& value);
  bool _set_notifications_enabled(bool value);
  bool _set_notification_options(std::string const& value);
  bool _set_notification_interval(unsigned int value);
  bool _set_notification_period(std::string const& value);
  bool _set_obsess_over_service(bool value);
  bool _set_parallelize_check(bool value);
  bool _set_process_perf_data(bool value);
  bool _set_retain_nonstatus_information(bool value);
  bool _set_retain_status_information(bool value);
  bool _set_retry_interval(unsigned int value);
  bool _set_recovery_notification_delay(unsigned int value);
  bool _set_servicegroups(std::string const& value);
  bool _set_service_description(std::string const& value);
  bool _set_stalking_options(std::string const& value);
  bool _set_timezone(std::string const& value);

  opt<int> _acknowledgement_timeout;
  std::string _action_url;
  opt<bool> _status_change;
  opt<bool> _checks_active;
  opt<bool> _checks_passive;
  std::string _metric_name;
  std::string _thresholds_file;
  opt<bool> _check_freshness;
  opt<unsigned int> _check_interval;
  group<set_string> _contactgroups;
  group<set_string> _contacts;
  map_customvar _customvariables;
  std::string _display_name;
  std::string _event_handler;
  opt<bool> _event_handler_enabled;
  opt<unsigned int> _first_notification_delay;
  opt<bool> _flap_detection_enabled;
  opt<unsigned short> _flap_detection_options;
  opt<unsigned int> _freshness_threshold;
  opt<unsigned int> _high_flap_threshold;
  std::string _host_name;
  std::string _icon_image;
  std::string _icon_image_alt;
  opt<unsigned int> _initial_state;
  opt<bool> _is_volatile;
  opt<unsigned int> _low_flap_threshold;
  opt<unsigned int> _max_check_attempts;
  std::string _notes;
  std::string _notes_url;
  opt<bool> _notifications_enabled;
  opt<unsigned int> _notification_interval;
  opt<unsigned short> _notification_options;
  opt<std::string> _notification_period;
  opt<bool> _obsess_over_service;
  opt<bool> _process_perf_data;
  opt<bool> _retain_nonstatus_information;
  opt<bool> _retain_status_information;
  opt<unsigned int> _retry_interval;
  opt<unsigned int> _recovery_notification_delay;
  group<set_string> _servicegroups;
  std::string _service_description;
  uint64_t _host_id;
  uint64_t _service_id;
  uint64_t _dependent_service_id;
  static std::unordered_map<std::string, setter_func> const _setters;
  opt<unsigned short> _stalking_options;
  opt<std::string> _timezone;
};

typedef std::shared_ptr<anomalydetection> anomalydetection_ptr;
typedef std::list<anomalydetection_ptr> list_anomalydetection;
typedef std::set<anomalydetection> set_anomalydetection;
typedef std::unordered_map<std::pair<std::string, std::string>, anomalydetection_ptr>
    map_anomalydetection;
}  // namespace configuration

CCE_END()

#endif  // !CCE_CONFIGURATION_ANOMALYDETECTION_HH

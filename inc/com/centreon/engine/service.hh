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

#ifndef CCE_SERVICE_HH
#define CCE_SERVICE_HH

#include <time.h>

#include <array>
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <utility>

#include "com/centreon/engine/check_result.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/customvariable.hh"
#include "com/centreon/engine/hash.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/notifier.hh"

/* Forward declaration. */
CCE_BEGIN()
class contact;
namespace commands {
class command;
}
class host;
class service;
class servicegroup;
class serviceescalation;
CCE_END()

typedef std::unordered_map<std::pair<std::string, std::string>,
                           std::shared_ptr<com::centreon::engine::service>,
                           pair_hash>
    service_map;
typedef std::unordered_map<std::pair<std::string, std::string>,
                           com::centreon::engine::service*,
                           pair_hash>
    service_map_unsafe;
typedef std::unordered_map<std::pair<uint64_t, uint64_t>,
                           std::shared_ptr<com::centreon::engine::service>,
                           pair_hash>
    service_id_map;

CCE_BEGIN()

class service : public notifier {
 public:
  static std::array<std::pair<uint32_t, std::string>, 4> const
      tab_service_states;

  enum service_state { state_ok, state_warning, state_critical, state_unknown };

  service(std::string const& hostname,
          std::string const& description,
          std::string const& display_name,
          std::string const& check_command,
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
          std::string const& timezone);
  ~service() = default;
  void set_host_id(uint64_t host_id);
  uint64_t get_host_id() const;
  void set_service_id(uint64_t service_id);
  uint64_t get_service_id() const;
  void set_hostname(std::string const& name);
  std::string const& get_hostname() const;
  void set_description(std::string const& desc);
  std::string const& get_description() const;
  void set_event_handler_args(std::string const& event_hdl_args);
  std::string const& get_event_handler_args() const;
  void set_check_command_args(std::string const& cmd_args);
  std::string const& get_check_command_args() const;
  time_t get_last_time_ok() const;
  void set_last_time_ok(time_t last_time);
  time_t get_last_time_warning() const;
  void set_last_time_warning(time_t last_time);
  time_t get_last_time_unknown() const;
  void set_last_time_unknown(time_t last_time);
  time_t get_last_time_critical() const;
  void set_last_time_critical(time_t last_time);
  enum service_state get_current_state() const;
  void set_current_state(enum service_state current_state);
  enum service_state get_last_state() const;
  void set_last_state(enum service_state last_state);
  enum service_state get_last_hard_state() const;
  void set_last_hard_state(enum service_state last_hard_state);
  enum service_state get_initial_state() const;
  void set_initial_state(enum service_state current_state);
  int get_process_performance_data(void) const;
  void set_process_performance_data(int perf_data);
  bool get_check_flapping_recovery_notification(void) const;
  void set_check_flapping_recovery_notification(bool check);
  bool recovered() const override;
  int get_current_state_int() const override;
  std::string const& get_current_state_as_string() const override;

  int handle_async_check_result(check_result* queued_check_result);
  int log_event();
  void check_for_flapping(bool update, bool allow_flapstart_notification);
  int handle_service_event();
  int obsessive_compulsive_service_check_processor();
  int update_service_performance_data();
  int run_scheduled_check(int check_options, double latency);
  virtual int run_async_check(int check_options,
                              double latency,
                              bool scheduled_check,
                              bool reschedule_check,
                              bool* time_is_valid,
                              time_t* preferred_time) noexcept;
  bool schedule_check(time_t check_time, int options) override;
  void set_flap(double percent_change,
                double high_threshold,
                double low_threshold,
                int allow_flapstart_notification);
  // handles a service that has stopped flapping
  void clear_flap(double percent_change,
                  double high_threshold,
                  double low_threshold);
  void enable_flap_detection();
  void disable_flap_detection();
  void update_status() override;
  bool verify_check_viability(int check_options,
                              bool* time_is_valid,
                              time_t* new_time);
  void grab_macros_r(nagios_macros* mac) override;
  int notify_contact(nagios_macros* mac,
                     contact* cntct,
                     reason_type type,
                     std::string const& not_author,
                     std::string const& not_data,
                     int options,
                     int escalated) override;
  void update_notification_flags() override;
  void check_for_expired_acknowledgement();
  void schedule_acknowledgement_expiration();
  bool operator==(service const& other) = delete;
  bool operator!=(service const& other) = delete;  // throw();
  bool is_valid_escalation_for_notification(escalation const* e,
                                            int options) const override;
  bool is_result_fresh(time_t current_time, int log_this);
  void handle_flap_detection_disabled();
  timeperiod* get_notification_timeperiod() const override;
  bool get_notify_on_current_state() const override;

  bool authorized_by_dependencies(
      dependency::types dependency_type) const override;
  static void check_for_orphaned();
  static void check_result_freshness();
  bool is_in_downtime() const override;
  void resolve(int& w, int& e);

  std::list<servicegroup*> const& get_parent_groups() const;
  std::list<servicegroup*>& get_parent_groups();
  void set_host_ptr(host* h);
  host const* get_host_ptr() const;
  host* get_host_ptr();
  bool get_host_problem_at_last_check() const;

  static service_map services;
  static service_id_map services_by_id;

 private:
  uint64_t _host_id;
  uint64_t _service_id;
  std::string _hostname;
  std::string _description;
  std::string _event_handler_args;
  std::string _check_command_args;

  int _process_performance_data;
  bool _check_flapping_recovery_notification;

  time_t _last_time_ok;
  time_t _last_time_warning;
  time_t _last_time_unknown;
  time_t _last_time_critical;
  enum service_state _initial_state;
  enum service_state _current_state;
  enum service_state _last_hard_state;
  enum service_state _last_state;
  std::list<servicegroup*> _servicegroups;
  host* _host_ptr;
  bool _host_problem_at_last_check;
};
CCE_END()

com::centreon::engine::service* add_service(
    uint64_t host_id,
    uint64_t service_id,
    std::string const& host_name,
    std::string const& description,
    std::string const& display_name,
    std::string const& check_period,
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
    bool obsess_over,
    std::string const& timezone);

std::ostream& operator<<(std::ostream& os,
                         com::centreon::engine::service const& obj);
std::ostream& operator<<(std::ostream& os, service_map_unsafe const& obj);

CCE_BEGIN()

com::centreon::engine::service& find_service(uint64_t host_id,
                                             uint64_t service_id);
bool is_service_exist(std::pair<uint64_t, uint64_t> const& id);
std::pair<uint64_t, uint64_t> get_host_and_service_id(std::string const& host,
                                                      std::string const& svc);
uint64_t get_service_id(std::string const& host, std::string const& svc);

CCE_END()

#endif  // !CCE_SERVICE_HH

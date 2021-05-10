/*
** Copyright 2011-2013,2015-2016 Centreon
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

#ifndef CCE_RETENTION_SERVICE_HH
#define CCE_RETENTION_SERVICE_HH

#include <list>
#include <string>
#include <vector>
#include "com/centreon/engine/customvariable.hh"
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/opt.hh"
#include "com/centreon/engine/retention/object.hh"

CCE_BEGIN()

namespace retention {
class service : public object {
 public:
  service();
  service(service const& right);
  ~service() throw() override;
  service& operator=(service const& right);
  bool operator==(service const& right) const throw();
  bool operator!=(service const& right) const throw();
  bool set(char const* key, char const* value) override;

  opt<int> const& acknowledgement_type() const throw();
  opt<bool> const& active_checks_enabled() const throw();
  opt<std::string> const& check_command() const throw();
  opt<double> const& check_execution_time() const throw();
  opt<int> const& check_flapping_recovery_notification() const throw();
  opt<double> const& check_latency() const throw();
  opt<int> const& check_options() const throw();
  opt<std::string> const& check_period() const throw();
  opt<int> const& check_type() const throw();
  opt<int> const& current_attempt() const throw();
  opt<uint64_t> const& current_event_id() const throw();
  opt<uint64_t> const& current_notification_id() const throw();
  opt<int> const& current_notification_number() const throw();
  opt<uint64_t> const& current_problem_id() const throw();
  opt<int> const& current_state() const throw();
  map_customvar const& customvariables() const throw();
  opt<std::string> const& event_handler() const throw();
  opt<bool> const& event_handler_enabled() const throw();
  opt<bool> const& flap_detection_enabled() const throw();
  opt<bool> const& has_been_checked() const noexcept;
  uint64_t host_id() const noexcept;
  std::string const& host_name() const throw();
  opt<bool> const& is_flapping() const throw();
  opt<time_t> const& last_acknowledgement() const throw();
  opt<time_t> const& last_check() const throw();
  opt<uint64_t> const& last_event_id() const throw();
  opt<time_t> const& last_hard_state() const throw();
  opt<time_t> const& last_hard_state_change() const throw();
  opt<time_t> const& last_notification() const throw();
  opt<uint64_t> const& last_problem_id() const throw();
  opt<time_t> const& last_state() const throw();
  opt<time_t> const& last_state_change() const throw();
  opt<time_t> const& last_time_critical() const throw();
  opt<time_t> const& last_time_ok() const throw();
  opt<time_t> const& last_time_unknown() const throw();
  opt<time_t> const& last_time_warning() const throw();
  opt<std::string> const& long_plugin_output() const throw();
  opt<unsigned int> const& max_attempts() const throw();
  opt<unsigned long> const& modified_attributes() const throw();
  opt<time_t> const& next_check() const throw();
  opt<unsigned int> const& normal_check_interval() const throw();
  opt<std::string> const& notification_period() const throw();
  opt<bool> const& notifications_enabled() const throw();
  opt<bool> const& notified_on_critical() const throw();
  opt<bool> const& notified_on_unknown() const throw();
  opt<bool> const& notified_on_warning() const throw();
  opt<int> const& obsess_over_service() const throw();
  opt<bool> const& passive_checks_enabled() const throw();
  opt<double> const& percent_state_change() const throw();
  opt<std::string> const& performance_data() const throw();
  opt<std::string> const& plugin_output() const throw();
  opt<bool> const& problem_has_been_acknowledged() const throw();
  opt<int> const& process_performance_data() const throw();
  opt<unsigned int> const& retry_check_interval() const throw();
  uint64_t service_id() const throw();
  std::string const& service_description() const throw();
  opt<std::vector<int> > const& state_history() const throw();
  opt<int> const& state_type() const throw();
  bool has_notifications() const;
  std::array<std::string, 6> notifications() const noexcept;

 private:
  struct setters {
    char const* name;
    bool (*func)(service&, char const*);
  };

  bool _set_acknowledgement_type(int value);
  bool _set_active_checks_enabled(bool value);
  bool _set_check_command(std::string const& value);
  bool _set_check_execution_time(double value);
  bool _set_check_flapping_recovery_notification(int value);
  bool _set_check_latency(double value);
  bool _set_check_options(int value);
  bool _set_check_period(std::string const& value);
  bool _set_check_type(int value);
  bool _set_current_attempt(int value);
  bool _set_current_event_id(uint64_t value);
  bool _set_current_notification_id(uint64_t value);
  bool _set_current_notification_number(int value);
  bool _set_current_problem_id(uint64_t value);
  bool _set_current_state(int value);
  bool _set_event_handler(std::string const& value);
  bool _set_event_handler_enabled(bool value);
  bool _set_failure_prediction_enabled(bool value);
  bool _set_flap_detection_enabled(bool value);
  bool _set_has_been_checked(bool value);
  bool _set_host_id(uint64_t value);
  bool _set_host_name(std::string const& value);
  bool _set_is_flapping(bool value);
  bool _set_last_acknowledgement(time_t value);
  bool _set_last_check(time_t value);
  bool _set_last_event_id(uint64_t value);
  bool _set_last_hard_state(time_t value);
  bool _set_last_hard_state_change(time_t value);
  bool _set_last_notification(time_t value);
  bool _set_last_problem_id(uint64_t value);
  bool _set_last_state(time_t value);
  bool _set_last_state_change(time_t value);
  bool _set_last_time_critical(time_t value);
  bool _set_last_time_ok(time_t value);
  bool _set_last_time_unknown(time_t value);
  bool _set_last_time_warning(time_t value);
  bool _set_long_plugin_output(std::string const& value);
  bool _set_max_attempts(unsigned int value);
  bool _set_modified_attributes(unsigned long value);
  bool _set_next_check(time_t value);
  bool _set_normal_check_interval(unsigned int value);

  template <int N>
  bool _set_notification(std::string const& value) {
    if (N < 6 && N >= 0) {
      _notification[N] = value;
      return true;
    } else
      return false;
  }

  bool _set_notification_period(std::string const& value);
  bool _set_notifications_enabled(bool value);
  bool _set_notified_on_critical(bool value);
  bool _set_notified_on_unknown(bool value);
  bool _set_notified_on_warning(bool value);
  bool _set_obsess_over_service(int value);
  bool _set_passive_checks_enabled(bool value);
  bool _set_percent_state_change(double value);
  bool _set_performance_data(std::string const& value);
  bool _set_plugin_output(std::string const& value);
  bool _set_problem_has_been_acknowledged(bool value);
  bool _set_process_performance_data(int value);
  bool _set_retry_check_interval(unsigned int value);
  bool _set_service_id(uint64_t value);
  bool _set_service_description(std::string const& value);
  bool _set_state_history(std::string const& value);
  bool _set_state_type(int value);

  opt<int> _acknowledgement_type;
  opt<bool> _active_checks_enabled;
  opt<std::string> _check_command;
  opt<double> _check_execution_time;
  opt<int> _check_flapping_recovery_notification;
  opt<double> _check_latency;
  opt<int> _check_options;
  opt<std::string> _check_period;
  opt<int> _check_type;
  opt<int> _current_attempt;
  opt<uint64_t> _current_event_id;
  opt<uint64_t> _current_notification_id;
  opt<int> _current_notification_number;
  opt<uint64_t> _current_problem_id;
  opt<int> _current_state;
  map_customvar _customvariables;
  opt<std::string> _event_handler;
  opt<bool> _event_handler_enabled;
  opt<bool> _flap_detection_enabled;
  opt<bool> _has_been_checked;
  uint64_t _host_id;
  std::string _host_name;
  opt<bool> _is_flapping;
  opt<time_t> _last_acknowledgement;
  opt<time_t> _last_check;
  opt<uint64_t> _last_event_id;
  opt<time_t> _last_hard_state;
  opt<time_t> _last_hard_state_change;
  opt<time_t> _last_notification;
  opt<uint64_t> _last_problem_id;
  opt<time_t> _last_state;
  opt<time_t> _last_state_change;
  opt<time_t> _last_time_critical;
  opt<time_t> _last_time_ok;
  opt<time_t> _last_time_unknown;
  opt<time_t> _last_time_warning;
  opt<std::string> _long_plugin_output;
  opt<unsigned int> _max_attempts;
  opt<unsigned long> _modified_attributes;
  opt<time_t> _next_check;
  setters const* _next_setter;
  opt<unsigned int> _normal_check_interval;
  opt<std::string> _notification_period;
  opt<bool> _notifications_enabled;
  opt<bool> _notified_on_critical;
  opt<bool> _notified_on_unknown;
  opt<bool> _notified_on_warning;
  opt<int> _obsess_over_service;
  opt<bool> _passive_checks_enabled;
  opt<double> _percent_state_change;
  opt<std::string> _performance_data;
  opt<std::string> _plugin_output;
  opt<bool> _problem_has_been_acknowledged;
  opt<int> _process_performance_data;
  opt<unsigned int> _retry_check_interval;
  uint64_t _service_id;
  std::string _service_description;
  static setters const _setters[];
  opt<std::vector<int> > _state_history;
  opt<int> _state_type;
  std::array<std::string, 6> _notification;
};

typedef std::shared_ptr<service> service_ptr;
typedef std::list<service_ptr> list_service;
}  // namespace retention

CCE_END()

#endif  // !CCE_RETENTION_SERVICE_HH

/*
** Copyright 2011-2013,2016 Centreon
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

#include "com/centreon/engine/retention/host.hh"
#include <array>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::retention;

#define SETTER(type, method) &object::setter<host, type, &host::method>::generic

host::setters const host::_setters[] = {
    {"acknowledgement_type", SETTER(int, _set_acknowledgement_type)},
    {"active_checks_enabled", SETTER(bool, _set_active_checks_enabled)},
    {"check_command", SETTER(std::string const&, _set_check_command)},
    {"check_execution_time", SETTER(double, _set_check_execution_time)},
    {"check_flapping_recovery_notification",
     SETTER(int, _set_check_flapping_recovery_notification)},
    {"check_latency", SETTER(double, _set_check_latency)},
    {"check_options", SETTER(int, _set_check_options)},
    {"check_period", SETTER(std::string const&, _set_check_period)},
    {"check_type", SETTER(int, _set_check_type)},
    {"current_attempt", SETTER(int, _set_current_attempt)},
    {"current_event_id", SETTER(uint64_t, _set_current_event_id)},
    {"current_notification_id", SETTER(uint64_t, _set_current_notification_id)},
    {"current_notification_number",
     SETTER(int, _set_current_notification_number)},
    {"current_problem_id", SETTER(uint64_t, _set_current_problem_id)},
    {"current_state", SETTER(int, _set_current_state)},
    {"event_handler", SETTER(std::string const&, _set_event_handler)},
    {"event_handler_enabled", SETTER(bool, _set_event_handler_enabled)},
    {"failure_prediction_enabled",
     SETTER(bool, _set_failure_prediction_enabled)},
    {"flap_detection_enabled", SETTER(bool, _set_flap_detection_enabled)},
    {"has_been_checked", SETTER(bool, _set_has_been_checked)},
    {"host_id", SETTER(uint64_t, _set_host_id)},
    {"host_name", SETTER(std::string const&, _set_host_name)},
    {"is_flapping", SETTER(bool, _set_is_flapping)},
    {"last_acknowledgement", SETTER(time_t, _set_last_acknowledgement)},
    {"last_check", SETTER(time_t, _set_last_check)},
    {"last_event_id", SETTER(uint64_t, _set_last_event_id)},
    {"last_hard_state", SETTER(time_t, _set_last_hard_state)},
    {"last_hard_state_change", SETTER(time_t, _set_last_hard_state_change)},
    {"last_notification", SETTER(time_t, _set_last_notification)},
    {"last_problem_id", SETTER(uint64_t, _set_last_problem_id)},
    {"last_state", SETTER(time_t, _set_last_state)},
    {"last_state_change", SETTER(time_t, _set_last_state_change)},
    {"last_time_down", SETTER(time_t, _set_last_time_down)},
    {"last_time_unreachable", SETTER(time_t, _set_last_time_unreachable)},
    {"last_time_up", SETTER(time_t, _set_last_time_up)},
    {"long_plugin_output", SETTER(std::string const&, _set_long_plugin_output)},
    {"max_attempts", SETTER(unsigned int, _set_max_attempts)},
    {"modified_attributes", SETTER(unsigned long, _set_modified_attributes)},
    {"next_check", SETTER(time_t, _set_next_check)},
    {"normal_check_interval", SETTER(unsigned int, _set_normal_check_interval)},
    {"notification_0", SETTER(std::string const&, _set_notification<0>)},
    {"notification_1", SETTER(std::string const&, _set_notification<1>)},
    {"notification_2", SETTER(std::string const&, _set_notification<2>)},
    {"notification_3", SETTER(std::string const&, _set_notification<3>)},
    {"notification_4", SETTER(std::string const&, _set_notification<4>)},
    {"notification_5", SETTER(std::string const&, _set_notification<5>)},
    {"notification_period",
     SETTER(std::string const&, _set_notification_period)},
    {"notifications_enabled", SETTER(bool, _set_notifications_enabled)},
    {"notified_on_down", SETTER(bool, _set_notified_on_down)},
    {"notified_on_unreachable", SETTER(bool, _set_notified_on_unreachable)},
    {"obsess_over_host", SETTER(int, _set_obsess_over_host)},
    {"passive_checks_enabled", SETTER(bool, _set_passive_checks_enabled)},
    {"percent_state_change", SETTER(double, _set_percent_state_change)},
    {"performance_data", SETTER(std::string const&, _set_performance_data)},
    {"plugin_output", SETTER(std::string const&, _set_plugin_output)},
    {"problem_has_been_acknowledged",
     SETTER(bool, _set_problem_has_been_acknowledged)},
    {"process_performance_data", SETTER(int, _set_process_performance_data)},
    {"retry_check_interval", SETTER(unsigned int, _set_retry_check_interval)},
    {"state_history", SETTER(std::string const&, _set_state_history)},
    {"state_type", SETTER(int, _set_state_type)}};

/**
 *  Constructor.
 */
host::host() : object(object::host) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
host::host(host const& right) : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
host::~host() throw() {}

/**
 *  Copy operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
host& host::operator=(host const& right) {
  if (this != &right) {
    object::operator=(right);
    _acknowledgement_type = right._acknowledgement_type;
    _active_checks_enabled = right._active_checks_enabled;
    _check_command = right._check_command;
    _check_execution_time = right._check_execution_time;
    _check_flapping_recovery_notification =
        right._check_flapping_recovery_notification;
    _check_latency = right._check_latency;
    _check_options = right._check_options;
    _check_period = right._check_period;
    _check_type = right._check_type;
    _current_attempt = right._current_attempt;
    _current_event_id = right._current_event_id;
    _current_notification_id = right._current_notification_id;
    _current_notification_number = right._current_notification_number;
    _current_problem_id = right._current_problem_id;
    _current_state = right._current_state;
    _customvariables = right._customvariables;
    _event_handler = right._event_handler;
    _event_handler_enabled = right._event_handler_enabled;
    _flap_detection_enabled = right._flap_detection_enabled;
    _has_been_checked = right._has_been_checked;
    _host_id = right._host_id;
    _host_name = right._host_name;
    _is_flapping = right._is_flapping;
    _last_acknowledgement = right._last_acknowledgement;
    _last_check = right._last_check;
    _last_event_id = right._last_event_id;
    _last_hard_state = right._last_hard_state;
    _last_hard_state_change = right._last_hard_state_change;
    _last_notification = right._last_notification;
    _last_problem_id = right._last_problem_id;
    _last_state = right._last_state;
    _last_state_change = right._last_state_change;
    _last_time_down = right._last_time_down;
    _last_time_unreachable = right._last_time_unreachable;
    _last_time_up = right._last_time_up;
    _long_plugin_output = right._long_plugin_output;
    _max_attempts = right._max_attempts;
    _modified_attributes = right._modified_attributes;
    _next_check = right._next_check;
    _normal_check_interval = right._normal_check_interval;
    _notification_period = right._notification_period;
    _notifications_enabled = right._notifications_enabled;
    _notified_on_down = right._notified_on_down;
    _notified_on_unreachable = right._notified_on_unreachable;
    _obsess_over_host = right._obsess_over_host;
    _passive_checks_enabled = right._passive_checks_enabled;
    _percent_state_change = right._percent_state_change;
    _performance_data = right._performance_data;
    _plugin_output = right._plugin_output;
    _problem_has_been_acknowledged = right._problem_has_been_acknowledged;
    _process_performance_data = right._process_performance_data;
    _retry_check_interval = right._retry_check_interval;
    _state_history = right._state_history;
    _state_type = right._state_type;
    _notification = right._notification;
  }
  return *this;
}

/**
 *  Equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool host::operator==(host const& right) const throw() {
  return (
      object::operator==(right) &&
      _acknowledgement_type == right._acknowledgement_type &&
      _active_checks_enabled == right._active_checks_enabled &&
      _check_command == right._check_command &&
      _check_execution_time == right._check_execution_time &&
      _check_flapping_recovery_notification ==
          right._check_flapping_recovery_notification &&
      _check_latency == right._check_latency &&
      _check_options == right._check_options &&
      _check_period == right._check_period &&
      _check_type == right._check_type &&
      _current_attempt == right._current_attempt &&
      _current_event_id == right._current_event_id &&
      _current_notification_id == right._current_notification_id &&
      _current_notification_number == right._current_notification_number &&
      _current_problem_id == right._current_problem_id &&
      _current_state == right._current_state &&
      std::operator==(_customvariables, right._customvariables) &&
      _event_handler == right._event_handler &&
      _event_handler_enabled == right._event_handler_enabled &&
      _flap_detection_enabled == right._flap_detection_enabled &&
      _has_been_checked == right._has_been_checked &&
      _host_id == right._host_id && _host_name == right._host_name &&
      _is_flapping == right._is_flapping &&
      _last_acknowledgement == right._last_acknowledgement &&
      _last_check == right._last_check &&
      _last_event_id == right._last_event_id &&
      _last_hard_state == right._last_hard_state &&
      _last_hard_state_change == right._last_hard_state_change &&
      _last_notification == right._last_notification &&
      _last_problem_id == right._last_problem_id &&
      _last_state == right._last_state &&
      _last_state_change == right._last_state_change &&
      _last_time_down == right._last_time_down &&
      _last_time_unreachable == right._last_time_unreachable &&
      _last_time_up == right._last_time_up &&
      _long_plugin_output == right._long_plugin_output &&
      _max_attempts == right._max_attempts &&
      _modified_attributes == right._modified_attributes &&
      _next_check == right._next_check &&
      _normal_check_interval == right._normal_check_interval &&
      _notification_period == right._notification_period &&
      _notifications_enabled == right._notifications_enabled &&
      _notified_on_down == right._notified_on_down &&
      _notified_on_unreachable == right._notified_on_unreachable &&
      _obsess_over_host == right._obsess_over_host &&
      _passive_checks_enabled == right._passive_checks_enabled &&
      _percent_state_change == right._percent_state_change &&
      _performance_data == right._performance_data &&
      _plugin_output == right._plugin_output &&
      _problem_has_been_acknowledged == right._problem_has_been_acknowledged &&
      _process_performance_data == right._process_performance_data &&
      _retry_check_interval == right._retry_check_interval &&
      _state_history == right._state_history &&
      _state_type == right._state_type && _notification == right._notification);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool host::operator!=(host const& right) const throw() {
  return !operator==(right);
}

/**
 *  Set new value on specific property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool host::set(char const* key, char const* value) {
  for (unsigned int i(0); i < sizeof(_setters) / sizeof(_setters[0]); ++i)
    if (!strcmp(_setters[i].name, key))
      return (_setters[i].func)(*this, value);
  if ((key[0] == '_') && (strlen(value) > 3)) {
    _customvariables[key + 1] = customvariable(value + 2);
    return true;
  }
  return false;
}

/**
 *  Get acknowledgement_type.
 *
 *  @return The acknowledgement_type.
 */
opt<int> const& host::acknowledgement_type() const throw() {
  return _acknowledgement_type;
}

/**
 *  Get active_checks_enabled.
 *
 *  @return The active_checks_enabled.
 */
opt<bool> const& host::active_checks_enabled() const throw() {
  return _active_checks_enabled;
}

/**
 *  Get check_command.
 *
 *  @return The check_command.
 */
opt<std::string> const& host::check_command() const throw() {
  return _check_command;
}

/**
 *  Get check_execution_time.
 *
 *  @return The check_execution_time.
 */
opt<double> const& host::check_execution_time() const throw() {
  return _check_execution_time;
}

/**
 *  Get check_flapping_recovery_notification.
 *
 *  @return The check_flapping_recovery_notification.
 */
opt<int> const& host::check_flapping_recovery_notification() const throw() {
  return _check_flapping_recovery_notification;
}

/**
 *  Get check_latency.
 *
 *  @return The check_latency.
 */
opt<double> const& host::check_latency() const throw() {
  return _check_latency;
}

/**
 *  Get check_options.
 *
 *  @return The check_options.
 */
opt<int> const& host::check_options() const throw() {
  return _check_options;
}

/**
 *  Get check_period.
 *
 *  @return The check_period.
 */
opt<std::string> const& host::check_period() const throw() {
  return _check_period;
}

/**
 *  Get check_type.
 *
 *  @return The check_type.
 */
opt<int> const& host::check_type() const throw() {
  return _check_type;
}

/**
 *  Get current_attempt.
 *
 *  @return The current_attempt.
 */
opt<int> const& host::current_attempt() const throw() {
  return _current_attempt;
}

/**
 *  Get current_event_id.
 *
 *  @return The current_event_id.
 */
opt<uint64_t> const& host::current_event_id() const throw() {
  return _current_event_id;
}

/**
 *  Get current_notification_id.
 *
 *  @return The current_notification_id.
 */
opt<uint64_t> const& host::current_notification_id() const throw() {
  return _current_notification_id;
}

/**
 *  Get current_notification_number.
 *
 *  @return The current_notification_number.
 */
opt<int> const& host::current_notification_number() const throw() {
  return _current_notification_number;
}

/**
 *  Get current_problem_id.
 *
 *  @return The current_problem_id.
 */
opt<uint64_t> const& host::current_problem_id() const throw() {
  return _current_problem_id;
}

/**
 *  Get current_state.
 *
 *  @return The current_state.
 */
opt<int> const& host::current_state() const throw() {
  return _current_state;
}

/**
 *  Get customvariables.
 *
 *  @return The customvariables.
 */
map_customvar const& host::customvariables() const throw() {
  return _customvariables;
}

/**
 *  Get event_handler.
 *
 *  @return The event_handler.
 */
opt<std::string> const& host::event_handler() const throw() {
  return _event_handler;
}

/**
 *  Get event_handler_enabled.
 *
 *  @return The event_handler_enabled.
 */
opt<bool> const& host::event_handler_enabled() const throw() {
  return _event_handler_enabled;
}

/**
 *  Get flap_detection_enabled.
 *
 *  @return The flap_detection_enabled.
 */
opt<bool> const& host::flap_detection_enabled() const throw() {
  return _flap_detection_enabled;
}

/**
 *  Get has_been_checked.
 *
 *  @return The has_been_checked.
 */
opt<bool> const& host::has_been_checked() const throw() {
  return _has_been_checked;
}

/**
 *  Get host_id.
 *
 *  @return The host_id.
 */
uint64_t host::host_id() const throw() {
  return _host_id;
}

/**
 *  Get host_name.
 *
 *  @return The host_name.
 */
std::string const& host::host_name() const throw() {
  return _host_name;
}

/**
 *  Get is_flapping.
 *
 *  @return The is_flapping.
 */
opt<bool> const& host::is_flapping() const throw() {
  return _is_flapping;
}

/**
 *  Get last_acknowledgement.
 *
 *  @return The last acknowledgement.
 */
opt<time_t> const& host::last_acknowledgement() const throw() {
  return _last_acknowledgement;
}

/**
 *  Get last_check.
 *
 *  @return The last_check.
 */
opt<time_t> const& host::last_check() const throw() {
  return _last_check;
}

/**
 *  Get last_event_id.
 *
 *  @return The last_event_id.
 */
opt<uint64_t> const& host::last_event_id() const throw() {
  return _last_event_id;
}

/**
 *  Get last_hard_state.
 *
 *  @return The last_hard_state.
 */
opt<time_t> const& host::last_hard_state() const throw() {
  return _last_hard_state;
}

/**
 *  Get last_hard_state_change.
 *
 *  @return The last_hard_state_change.
 */
opt<time_t> const& host::last_hard_state_change() const throw() {
  return _last_hard_state_change;
}

/**
 *  Get last_notification.
 *
 *  @return The last_notification.
 */
opt<time_t> const& host::last_notification() const throw() {
  return _last_notification;
}

/**
 *  Get last_problem_id.
 *
 *  @return The last_problem_id.
 */
opt<uint64_t> const& host::last_problem_id() const throw() {
  return _last_problem_id;
}

/**
 *  Get last_state.
 *
 *  @return The last_state.
 */
opt<time_t> const& host::last_state() const throw() {
  return _last_state;
}

/**
 *  Get last_state_change.
 *
 *  @return The last_state_change.
 */
opt<time_t> const& host::last_state_change() const throw() {
  return _last_state_change;
}

/**
 *  Get last_time_down.
 *
 *  @return The last_time_down.
 */
opt<time_t> const& host::last_time_down() const throw() {
  return _last_time_down;
}

/**
 *  Get last_time_unreachable.
 *
 *  @return The last_time_unreachable.
 */
opt<time_t> const& host::last_time_unreachable() const throw() {
  return _last_time_unreachable;
}

/**
 *  Get last_time_up.
 *
 *  @return The last_time_up.
 */
opt<time_t> const& host::last_time_up() const throw() {
  return _last_time_up;
}

/**
 *  Get long_plugin_output.
 *
 *  @return The long_plugin_output.
 */
opt<std::string> const& host::long_plugin_output() const throw() {
  return _long_plugin_output;
}

/**
 *  Get max_attempts.
 *
 *  @return The max_attempts.
 */
opt<unsigned int> const& host::max_attempts() const throw() {
  return _max_attempts;
}

/**
 *  Get modified_attributes.
 *
 *  @return The modified_attributes.
 */
opt<unsigned long> const& host::modified_attributes() const throw() {
  return _modified_attributes;
}

/**
 *  Get next_check.
 *
 *  @return The next_check.
 */
opt<time_t> const& host::next_check() const throw() {
  return _next_check;
}

/**
 *  Get normal_check_interval.
 *
 *  @return The normal_check_interval.
 */
opt<unsigned int> const& host::normal_check_interval() const throw() {
  return _normal_check_interval;
}

/**
 *  Get notification_period.
 *
 *  @return The notification_period.
 */
opt<std::string> const& host::notification_period() const throw() {
  return _notification_period;
}

/**
 *  Get notifications_enabled.
 *
 *  @return The notifications_enabled.
 */
opt<bool> const& host::notifications_enabled() const throw() {
  return _notifications_enabled;
}

/**
 *  Get notified_on_down.
 *
 *  @return The notified_on_down.
 */
opt<bool> const& host::notified_on_down() const throw() {
  return _notified_on_down;
}

/**
 *  Get notified_on_unreachable.
 *
 *  @return The notified_on_unreachable.
 */
opt<bool> const& host::notified_on_unreachable() const throw() {
  return _notified_on_unreachable;
}

/**
 *  Get obsess_over_host.
 *
 *  @return The obsess_over_host.
 */
opt<int> const& host::obsess_over_host() const throw() {
  return _obsess_over_host;
}

/**
 *  Get passive_checks_enabled.
 *
 *  @return The passive_checks_enabled.
 */
opt<bool> const& host::passive_checks_enabled() const throw() {
  return _passive_checks_enabled;
}

/**
 *  Get percent_state_change.
 *
 *  @return The percent_state_change.
 */
opt<double> const& host::percent_state_change() const throw() {
  return _percent_state_change;
}

/**
 *  Get performance_data.
 *
 *  @return The performance_data.
 */
opt<std::string> const& host::performance_data() const throw() {
  return _performance_data;
}

/**
 *  Get plugin_output.
 *
 *  @return The plugin_output.
 */
opt<std::string> const& host::plugin_output() const throw() {
  return _plugin_output;
}

/**
 *  Get problem_has_been_acknowledged.
 *
 *  @return The problem_has_been_acknowledged.
 */
opt<bool> const& host::problem_has_been_acknowledged() const throw() {
  return _problem_has_been_acknowledged;
}

/**
 *  Get process_performance_data.
 *
 *  @return The process_performance_data.
 */
opt<int> const& host::process_performance_data() const throw() {
  return _process_performance_data;
}

/**
 *  Get retry_check_interval.
 *
 *  @return The retry_check_interval.
 */
opt<unsigned int> const& host::retry_check_interval() const throw() {
  return _retry_check_interval;
}

/**
 *  Get state_history.
 *
 *  @return The state_history.
 */
opt<std::vector<int> > const& host::state_history() const throw() {
  return _state_history;
}

/**
 *  Get state_type.
 *
 *  @return The state_type.
 */
opt<int> const& host::state_type() const throw() {
  return _state_type;
}

/**
 *  Set acknowledgement_type.
 *
 *  @param[in] value The new acknowledgement_type.
 */
bool host::_set_acknowledgement_type(int value) {
  _acknowledgement_type = value;
  return true;
}

/**
 *  Set active_checks_enabled.
 *
 *  @param[in] value The new active_checks_enabled.
 */
bool host::_set_active_checks_enabled(bool value) {
  _active_checks_enabled = value;
  return true;
}

/**
 *  Set check_command.
 *
 *  @param[in] value The new check_command.
 */
bool host::_set_check_command(std::string const& value) {
  _check_command = value;
  return true;
}

/**
 *  Set check_execution_time.
 *
 *  @param[in] value The new check_execution_time.
 */
bool host::_set_check_execution_time(double value) {
  _check_execution_time = value;
  return true;
}

/**
 *  Set check_flapping_recovery_notification.
 *
 *  @param[in] value The new check_flapping_recovery_notification.
 */
bool host::_set_check_flapping_recovery_notification(int value) {
  _check_flapping_recovery_notification = value;
  return true;
}

/**
 *  Set check_latency.
 *
 *  @param[in] value The new check_latency.
 */
bool host::_set_check_latency(double value) {
  _check_latency = value;
  return true;
}

/**
 *  Set check_options.
 *
 *  @param[in] value The new check_options.
 */
bool host::_set_check_options(int value) {
  _check_options = value;
  return true;
}

/**
 *  Set check_period.
 *
 *  @param[in] value The new check_period.
 */
bool host::_set_check_period(std::string const& value) {
  _check_period = value;
  return true;
}

/**
 *  Set check_type.
 *
 *  @param[in] value The new check_type.
 */
bool host::_set_check_type(int value) {
  _check_type = value;
  return true;
}

/**
 *  Set current_attempt.
 *
 *  @param[in] value The new current_attempt.
 */
bool host::_set_current_attempt(int value) {
  _current_attempt = value;
  return true;
}

/**
 *  Set current_event_id.
 *
 *  @param[in] value The new current_event_id.
 */
bool host::_set_current_event_id(uint64_t value) {
  _current_event_id = value;
  return true;
}

/**
 *  Set current_notification_id.
 *
 *  @param[in] value The new current_notification_id.
 */
bool host::_set_current_notification_id(uint64_t value) {
  _current_notification_id = value;
  return true;
}

/**
 *  Set current_notification_number.
 *
 *  @param[in] value The new current_notification_number.
 */
bool host::_set_current_notification_number(int value) {
  _current_notification_number = value;
  return true;
}

/**
 *  Set current_problem_id.
 *
 *  @param[in] value The new current_problem_id.
 */
bool host::_set_current_problem_id(uint64_t value) {
  _current_problem_id = value;
  return true;
}

/**
 *  Set current_state.
 *
 *  @param[in] value The new current_state.
 */
bool host::_set_current_state(int value) {
  _current_state = value;
  return true;
}

/**
 *  Set event_handler.
 *
 *  @param[in] value The new event_handler.
 */
bool host::_set_event_handler(std::string const& value) {
  _event_handler = value;
  return true;
}

/**
 *  Set event_handler_enabled.
 *
 *  @param[in] value The new event_handler_enabled.
 */
bool host::_set_event_handler_enabled(bool value) {
  _event_handler_enabled = value;
  return true;
}

/**
 *  Deprecated.
 *
 *  @param[in] value  Unused.
 */
bool host::_set_failure_prediction_enabled(bool value) {
  (void)value;
  return true;
}

/**
 *  Set flap_detection_enabled.
 *
 *  @param[in] value The new flap_detection_enabled.
 */
bool host::_set_flap_detection_enabled(bool value) {
  _flap_detection_enabled = value;
  return true;
}

/**
 *  Set has_been_checked.
 *
 *  @param[in] value The new has_been_checked.
 */
bool host::_set_has_been_checked(bool value) {
  _has_been_checked = value;
  return true;
}

/**
 *  Set host_id.
 *
 *  @param[in] value The new host_id.
 */
bool host::_set_host_id(uint64_t value) {
  _host_id = value;
  return true;
}

/**
 *  Set host_name.
 *
 *  @param[in] value The new host_name.
 */
bool host::_set_host_name(std::string const& value) {
  _host_name = value;
  return true;
}

/**
 *  Set is_flapping.
 *
 *  @param[in] value The new is_flapping.
 */
bool host::_set_is_flapping(bool value) {
  _is_flapping = value;
  return true;
}

/**
 *  Set last_acknowledgement.
 *
 *  @param[in] value  The new last_acknowledgement.
 */
bool host::_set_last_acknowledgement(time_t value) {
  _last_acknowledgement = value;
  return true;
}

/**
 *  Set last_check.
 *
 *  @param[in] value The new last_check.
 */
bool host::_set_last_check(time_t value) {
  _last_check = value;
  return true;
}

/**
 *  Set last_event_id.
 *
 *  @param[in] value The new last_event_id.
 */
bool host::_set_last_event_id(uint64_t value) {
  _last_event_id = value;
  return true;
}

/**
 *  Set last_hard_state.
 *
 *  @param[in] value The new last_hard_state.
 */
bool host::_set_last_hard_state(time_t value) {
  _last_hard_state = value;
  return true;
}

/**
 *  Set last_hard_state_change.
 *
 *  @param[in] value The new last_hard_state_change.
 */
bool host::_set_last_hard_state_change(time_t value) {
  _last_hard_state_change = value;
  return true;
}

/**
 *  Set last_notification.
 *
 *  @param[in] value The new last_notification.
 */
bool host::_set_last_notification(time_t value) {
  _last_notification = value;
  return true;
}

/**
 *  Set last_problem_id.
 *
 *  @param[in] value The new last_problem_id.
 */
bool host::_set_last_problem_id(uint64_t value) {
  _last_problem_id = value;
  return true;
}

/**
 *  Set last_state.
 *
 *  @param[in] value The new last_state.
 */
bool host::_set_last_state(time_t value) {
  _last_state = value;
  return true;
}

/**
 *  Set last_state_change.
 *
 *  @param[in] value The new last_state_change.
 */
bool host::_set_last_state_change(time_t value) {
  _last_state_change = value;
  return true;
}

/**
 *  Set last_time_down.
 *
 *  @param[in] value The new last_time_down.
 */
bool host::_set_last_time_down(time_t value) {
  _last_time_down = value;
  return true;
}

/**
 *  Set last_time_unreachable.
 *
 *  @param[in] value The new last_time_unreachable.
 */
bool host::_set_last_time_unreachable(time_t value) {
  _last_time_unreachable = value;
  return true;
}

/**
 *  Set last_time_up.
 *
 *  @param[in] value The new last_time_up.
 */
bool host::_set_last_time_up(time_t value) {
  _last_time_up = value;
  return true;
}

/**
 *  Set long_plugin_output.
 *
 *  @param[in] value The new long_plugin_output.
 */
bool host::_set_long_plugin_output(std::string const& value) {
  _long_plugin_output = value;
  return true;
}

/**
 *  Set max_attempts.
 *
 *  @param[in] value The new max_attempts.
 */
bool host::_set_max_attempts(unsigned int value) {
  if (value) {
    _max_attempts = value;
    return true;
  }
  return false;
}

/**
 *  Set modified_attributes.
 *
 *  @param[in] value The new modified_attributes.
 */
bool host::_set_modified_attributes(unsigned long value) {
  _modified_attributes = value;
  return true;
}

/**
 *  Set next_check.
 *
 *  @param[in] value The new next_check.
 */
bool host::_set_next_check(time_t value) {
  _next_check = value;
  return true;
}

/**
 *  Set normal_check_interval.
 *
 *  @param[in] value The new normal_check_interval.
 */
bool host::_set_normal_check_interval(unsigned int value) {
  _normal_check_interval = value;
  return true;
}

/**
 *  Set notification_period.
 *
 *  @param[in] value The new notification_period.
 */
bool host::_set_notification_period(std::string const& value) {
  _notification_period = value;
  return true;
}

/**
 *  Set notifications_enabled.
 *
 *  @param[in] value The new notifications_enabled.
 */
bool host::_set_notifications_enabled(bool value) {
  _notifications_enabled = value;
  return true;
}

bool host::has_notifications() const {
  for (auto const& n : _notification)
    if (!n.empty())
      return true;
  return false;
}

std::array<std::string, 6> host::notifications() const noexcept {
  return _notification;
}

/**
 *  Set notified_on_down.
 *
 *  @param[in] value The new notified_on_down.
 */
bool host::_set_notified_on_down(bool value) {
  _notified_on_down = value;
  return true;
}

/**
 *  Set notified_on_unreachable.
 *
 *  @param[in] value The new notified_on_unreachable.
 */
bool host::_set_notified_on_unreachable(bool value) {
  _notified_on_unreachable = value;
  return true;
}

/**
 *  Set obsess_over_host.
 *
 *  @param[in] value The new obsess_over_host.
 */
bool host::_set_obsess_over_host(int value) {
  _obsess_over_host = value;
  return true;
}

/**
 *  Set passive_checks_enabled.
 *
 *  @param[in] value The new passive_checks_enabled.
 */
bool host::_set_passive_checks_enabled(bool value) {
  _passive_checks_enabled = value;
  return true;
}

/**
 *  Set percent_state_change.
 *
 *  @param[in] value The new percent_state_change.
 */
bool host::_set_percent_state_change(double value) {
  _percent_state_change = value;
  return true;
}

/**
 *  Set performance_data.
 *
 *  @param[in] value The new performance_data.
 */
bool host::_set_performance_data(std::string const& value) {
  _performance_data = value;
  return true;
}

/**
 *  Set plugin_output.
 *
 *  @param[in] value The new plugin_output.
 */
bool host::_set_plugin_output(std::string const& value) {
  _plugin_output = value;
  return true;
}

/**
 *  Set problem_has_been_acknowledged.
 *
 *  @param[in] value The new problem_has_been_acknowledged.
 */
bool host::_set_problem_has_been_acknowledged(bool value) {
  _problem_has_been_acknowledged = value;
  return true;
}

/**
 *  Set process_performance_data.
 *
 *  @param[in] value The new process_performance_data.
 */
bool host::_set_process_performance_data(int value) {
  _process_performance_data = value;
  return true;
}

/**
 *  Set retry_check_interval.
 *
 *  @param[in] value The new retry_check_interval.
 */
bool host::_set_retry_check_interval(unsigned int value) {
  _retry_check_interval = value;
  return true;
}

/**
 *  Set state_history.
 *
 *  @param[in] value The new state_history.
 */
bool host::_set_state_history(std::string const& value) {
  unsigned int x(0);
  std::list<std::string> lst_history;
  string::split(value, lst_history, ',');
  std::vector<int>& state_history(*_state_history);
  for (std::list<std::string>::const_iterator it(lst_history.begin()),
       end(lst_history.end());
       it != end && x < MAX_STATE_HISTORY_ENTRIES; ++it) {
    int state(0);
    if (!string::to(it->c_str(), state)) {
      _state_history.reset();
      return false;
    }
    state_history.push_back(state);
  }
  _state_history.set(state_history);
  return true;
}

/**
 *  Set state_type.
 *
 *  @param[in] value The new state_type.
 */
bool host::_set_state_type(int value) {
  _state_type = value;
  return true;
}

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

#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/retention/service.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::retention;

#define SETTER(type, method) \
  &object::setter<service, type, &service::method>::generic

service::setters const service::_setters[] = {
  { "acknowledgement_type",                 SETTER(int, _set_acknowledgement_type) },
  { "active_checks_enabled",                SETTER(bool, _set_active_checks_enabled) },
  { "check_command",                        SETTER(std::string const&, _set_check_command) },
  { "check_execution_time",                 SETTER(double, _set_check_execution_time) },
  { "check_flapping_recovery_notification", SETTER(int, _set_check_flapping_recovery_notification) },
  { "check_latency",                        SETTER(double, _set_check_latency) },
  { "check_options",                        SETTER(int, _set_check_options) },
  { "check_period",                         SETTER(std::string const&, _set_check_period) },
  { "check_type",                           SETTER(int, _set_check_type) },
  { "current_attempt",                      SETTER(int, _set_current_attempt) },
  { "current_event_id",                     SETTER(uint64_t, _set_current_event_id) },
  { "current_notification_id",              SETTER(uint64_t, _set_current_notification_id) },
  { "current_notification_number",          SETTER(int, _set_current_notification_number) },
  { "current_problem_id",                   SETTER(uint64_t, _set_current_problem_id) },
  { "current_state",                        SETTER(int, _set_current_state) },
  { "event_handler",                        SETTER(std::string const&, _set_event_handler) },
  { "event_handler_enabled",                SETTER(bool, _set_event_handler_enabled) },
  { "failure_prediction_enabled",           SETTER(bool, _set_failure_prediction_enabled) },
  { "flap_detection_enabled",               SETTER(bool, _set_flap_detection_enabled) },
  { "has_been_checked",                     SETTER(bool, _set_has_been_checked) },
  { "host_name",                            SETTER(std::string const&, _set_host_name) },
  { "is_flapping",                          SETTER(bool, _set_is_flapping) },
  { "last_acknowledgement",                 SETTER(time_t, _set_last_acknowledgement) },
  { "last_check",                           SETTER(time_t, _set_last_check) },
  { "last_event_id",                        SETTER(uint64_t, _set_last_event_id) },
  { "last_hard_state",                      SETTER(time_t, _set_last_hard_state) },
  { "last_hard_state_change",               SETTER(time_t, _set_last_hard_state_change) },
  { "last_notification",                    SETTER(time_t, _set_last_notification) },
  { "last_problem_id",                      SETTER(uint64_t, _set_last_problem_id) },
  { "last_state",                           SETTER(time_t, _set_last_state) },
  { "last_state_change",                    SETTER(time_t, _set_last_state_change) },
  { "last_time_critical",                   SETTER(time_t, _set_last_time_critical) },
  { "last_time_ok",                         SETTER(time_t, _set_last_time_ok) },
  { "last_time_unknown",                    SETTER(time_t, _set_last_time_unknown) },
  { "last_time_warning",                    SETTER(time_t, _set_last_time_warning) },
  { "long_plugin_output",                   SETTER(std::string const&, _set_long_plugin_output) },
  { "max_attempts",                         SETTER(unsigned int, _set_max_attempts) },
  { "modified_attributes",                  SETTER(unsigned long, _set_modified_attributes) },
  { "next_check",                           SETTER(time_t, _set_next_check) },
  { "normal_check_interval",                SETTER(unsigned int, _set_normal_check_interval) },
  { "notification_period",                  SETTER(std::string const&, _set_notification_period) },
  { "notifications_enabled",                SETTER(bool, _set_notifications_enabled) },
  { "notified_on_critical",                 SETTER(bool, _set_notified_on_critical) },
  { "notified_on_unknown",                  SETTER(bool, _set_notified_on_unknown) },
  { "notified_on_warning",                  SETTER(bool, _set_notified_on_warning) },
  { "obsess_over_service",                  SETTER(int, _set_obsess_over_service) },
  { "passive_checks_enabled",               SETTER(bool, _set_passive_checks_enabled) },
  { "percent_state_change",                 SETTER(double, _set_percent_state_change) },
  { "performance_data",                     SETTER(std::string const&, _set_performance_data) },
  { "plugin_output",                        SETTER(std::string const&, _set_plugin_output) },
  { "problem_has_been_acknowledged",        SETTER(bool, _set_problem_has_been_acknowledged) },
  { "process_performance_data",             SETTER(int, _set_process_performance_data) },
  { "recovery_been_sent",                   SETTER(bool, _set_recovery_been_sent)},
  { "retry_check_interval",                 SETTER(unsigned int, _set_retry_check_interval) },
  { "service_id",                           SETTER(uint64_t, _set_service_id) },
  { "service_description",                  SETTER(std::string const&, _set_service_description) },
  { "state_history",                        SETTER(std::string const&, _set_state_history) },
  { "state_type",                           SETTER(int, _set_state_type) }
};

/**
 *  Constructor.
 */
service::service() : object(object::service), _next_setter(_setters) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
service::service(service const& right) : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
service::~service() throw () {}

/**
 *  Copy operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
service& service::operator=(service const& right) {
  if (this != &right) {
    object::operator=(right);
    _acknowledgement_type = right._acknowledgement_type;
    _active_checks_enabled = right._active_checks_enabled;
    _check_command = right._check_command;
    _check_execution_time = right._check_execution_time;
    _check_flapping_recovery_notification = right._check_flapping_recovery_notification;
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
    _last_time_critical = right._last_time_critical;
    _last_time_ok = right._last_time_ok;
    _last_time_unknown = right._last_time_unknown;
    _last_time_warning = right._last_time_warning;
    _long_plugin_output = right._long_plugin_output;
    _max_attempts = right._max_attempts;
    _modified_attributes = right._modified_attributes;
    _next_check = right._next_check;
    _next_setter = right._next_setter;
    _normal_check_interval = right._normal_check_interval;
    _notification_period = right._notification_period;
    _notifications_enabled = right._notifications_enabled;
    _notified_on_critical = right._notified_on_critical;
    _notified_on_unknown = right._notified_on_unknown;
    _notified_on_warning = right._notified_on_warning;
    _obsess_over_service = right._obsess_over_service;
    _passive_checks_enabled = right._passive_checks_enabled;
    _percent_state_change = right._percent_state_change;
    _performance_data = right._performance_data;
    _plugin_output = right._plugin_output;
    _problem_has_been_acknowledged = right._problem_has_been_acknowledged;
    _process_performance_data = right._process_performance_data;
    _recovery_been_sent = right._recovery_been_sent;
    _retry_check_interval = right._retry_check_interval;
    _service_id = right._service_id;
    _service_description = right._service_description;
    _state_history = right._state_history;
    _state_type = right._state_type;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool service::operator==(service const& right) const throw () {
  return (object::operator==(right)
          && _acknowledgement_type == right._acknowledgement_type
          && _active_checks_enabled == right._active_checks_enabled
          && _check_command == right._check_command
          && _check_execution_time == right._check_execution_time
          && _check_flapping_recovery_notification == right._check_flapping_recovery_notification
          && _check_latency == right._check_latency
          && _check_options == right._check_options
          && _check_period == right._check_period
          && _check_type == right._check_type
          && _current_attempt == right._current_attempt
          && _current_event_id == right._current_event_id
          && _current_notification_id == right._current_notification_id
          && _current_notification_number == right._current_notification_number
          && _current_problem_id == right._current_problem_id
          && _current_state == right._current_state
          && std::operator==(_customvariables, right._customvariables)
          && _event_handler == right._event_handler
          && _event_handler_enabled == right._event_handler_enabled
          && _flap_detection_enabled == right._flap_detection_enabled
          && _has_been_checked == right._has_been_checked
          && _host_name == right._host_name
          && _is_flapping == right._is_flapping
          && _last_acknowledgement == right._last_acknowledgement
          && _last_check == right._last_check
          && _last_event_id == right._last_event_id
          && _last_hard_state == right._last_hard_state
          && _last_hard_state_change == right._last_hard_state_change
          && _last_notification == right._last_notification
          && _last_problem_id == right._last_problem_id
          && _last_state == right._last_state
          && _last_state_change == right._last_state_change
          && _last_time_critical == right._last_time_critical
          && _last_time_ok == right._last_time_ok
          && _last_time_unknown == right._last_time_unknown
          && _last_time_warning == right._last_time_warning
          && _long_plugin_output == right._long_plugin_output
          && _max_attempts == right._max_attempts
          && _modified_attributes == right._modified_attributes
          && _next_check == right._next_check
          && _normal_check_interval == right._normal_check_interval
          && _notification_period == right._notification_period
          && _notifications_enabled == right._notifications_enabled
          && _notified_on_critical == right._notified_on_critical
          && _notified_on_unknown == right._notified_on_unknown
          && _notified_on_warning == right._notified_on_warning
          && _obsess_over_service == right._obsess_over_service
          && _passive_checks_enabled == right._passive_checks_enabled
          && _percent_state_change == right._percent_state_change
          && _performance_data == right._performance_data
          && _plugin_output == right._plugin_output
          && _problem_has_been_acknowledged == right._problem_has_been_acknowledged
          && _process_performance_data == right._process_performance_data
          && _recovery_been_sent == right._recovery_been_sent
          && _retry_check_interval == right._retry_check_interval
          && _service_id == right._service_id
          && _service_description == right._service_description
          && _state_history == right._state_history
          && _state_type == right._state_type);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool service::operator!=(service const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Set new value on specific property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @warning This method is not thread safe.
 *
 *  @return True on success, otherwise false.
 */
bool service::set(char const* key, char const* value) {
  // The strategy in service retention loading is to keep the position
  // at which the last entry was processed and to start from this
  // position at the next entry. Therefore most of the time entry lookup
  // will be the matter of one iteration, but the keep compatibility
  // with modified retention files.
  static setters const* const
    end(_setters + sizeof(_setters) / sizeof(*_setters));

  // Custom variables.
  if ((key[0] == '_') && value[0] && value[1] && value[2]) {
    _customvariables.insert(std::make_pair(key + 1, customvariable(key + 1, value + 2)));
    return (true);
  }

  // Normal properties.
  setters const* it(_next_setter);
  do {
    if (!strcmp(it->name, key)) {
      _next_setter = it;
      ++_next_setter;
      if (_next_setter == end)
        _next_setter = _setters;
      return ((it->func)(*this, value));
    }
    ++it;
    if (it == end)
      it = _setters;
  } while (it != _next_setter);

  return (false);
}

/**
 *  Get acknowledgement_type.
 *
 *  @return The acknowledgement_type.
 */
opt<int> const& service::acknowledgement_type() const throw () {
  return (_acknowledgement_type);
}

/**
 *  Get active_checks_enabled.
 *
 *  @return The active_checks_enabled.
 */
opt<bool> const& service::active_checks_enabled() const throw () {
  return (_active_checks_enabled);
}

/**
 *  Get check_command.
 *
 *  @return The check_command.
 */
opt<std::string> const& service::check_command() const throw () {
  return (_check_command);
}

/**
 *  Get check_execution_time.
 *
 *  @return The check_execution_time.
 */
opt<double> const& service::check_execution_time() const throw () {
  return (_check_execution_time);
}

/**
 *  Get check_flapping_recovery_notification.
 *
 *  @return The check_flapping_recovery_notification.
 */
opt<int> const& service::check_flapping_recovery_notification() const throw () {
  return (_check_flapping_recovery_notification);
}

/**
 *  Get check_latency.
 *
 *  @return The check_latency.
 */
opt<double> const& service::check_latency() const throw () {
  return (_check_latency);
}

/**
 *  Get check_options.
 *
 *  @return The check_options.
 */
opt<int> const& service::check_options() const throw () {
  return (_check_options);
}

/**
 *  Get check_period.
 *
 *  @return The check_period.
 */
opt<std::string> const& service::check_period() const throw () {
  return (_check_period);
}

/**
 *  Get check_type.
 *
 *  @return The check_type.
 */
opt<int> const& service::check_type() const throw () {
  return (_check_type);
}

/**
 *  Get current_attempt.
 *
 *  @return The current_attempt.
 */
opt<int> const& service::current_attempt() const throw () {
  return (_current_attempt);
}

/**
 *  Get current_event_id.
 *
 *  @return The current_event_id.
 */
opt<uint64_t> const& service::current_event_id() const throw () {
  return (_current_event_id);
}

/**
 *  Get current_notification_id.
 *
 *  @return The current_notification_id.
 */
opt<uint64_t> const& service::current_notification_id() const throw () {
  return (_current_notification_id);
}

/**
 *  Get current_notification_number.
 *
 *  @return The current_notification_number.
 */
opt<int> const& service::current_notification_number() const throw () {
  return (_current_notification_number);
}

/**
 *  Get current_problem_id.
 *
 *  @return The current_problem_id.
 */
opt<uint64_t> const& service::current_problem_id() const throw () {
  return (_current_problem_id);
}

/**
 *  Get current_state.
 *
 *  @return The current_state.
 */
opt<int> const& service::current_state() const throw () {
  return (_current_state);
}

/**
 *  Get event_handler.
 *
 *  @return The event_handler.
 */
opt<std::string> const& service::event_handler() const throw () {
  return (_event_handler);
}

/**
 *  Get customvariables.
 *
 *  @return The customvariables.
 */
map_customvar const& service::customvariables() const throw () {
  return (_customvariables);
}

/**
 *  Get event_handler_enabled.
 *
 *  @return The event_handler_enabled.
 */
opt<bool> const& service::event_handler_enabled() const throw () {
  return (_event_handler_enabled);
}

/**
 *  Get flap_detection_enabled.
 *
 *  @return The flap_detection_enabled.
 */
opt<bool> const& service::flap_detection_enabled() const throw () {
  return (_flap_detection_enabled);
}

/**
 *  Get has_been_checked.
 *
 *  @return The has_been_checked.
 */
opt<bool> const& service::has_been_checked() const throw () {
  return (_has_been_checked);
}

/**
 *  Get host_name.
 *
 *  @return The host_name.
 */
std::string const& service::host_name() const throw () {
  return (_host_name);
}

/**
 *  Get is_flapping.
 *
 *  @return The is_flapping.
 */
opt<bool> const& service::is_flapping() const throw () {
  return (_is_flapping);
}

/**
 *  Get last_acknowledgement.
 *
 *  @return The last acknowledgement.
 */
opt<time_t> const& service::last_acknowledgement() const throw () {
  return (_last_acknowledgement);
}

/**
 *  Get last_check.
 *
 *  @return The last_check.
 */
opt<time_t> const& service::last_check() const throw () {
  return (_last_check);
}

/**
 *  Get last_event_id.
 *
 *  @return The last_event_id.
 */
opt<uint64_t> const& service::last_event_id() const throw () {
  return (_last_event_id);
}

/**
 *  Get last_hard_state.
 *
 *  @return The last_hard_state.
 */
opt<time_t> const& service::last_hard_state() const throw () {
  return (_last_hard_state);
}

/**
 *  Get last_hard_state_change.
 *
 *  @return The last_hard_state_change.
 */
opt<time_t> const& service::last_hard_state_change() const throw () {
  return (_last_hard_state_change);
}

/**
 *  Get last_notification.
 *
 *  @return The last_notification.
 */
opt<time_t> const& service::last_notification() const throw () {
  return (_last_notification);
}

/**
 *  Get last_problem_id.
 *
 *  @return The last_problem_id.
 */
opt<uint64_t> const& service::last_problem_id() const throw () {
  return (_last_problem_id);
}

/**
 *  Get last_state.
 *
 *  @return The last_state.
 */
opt<time_t> const& service::last_state() const throw () {
  return (_last_state);
}

/**
 *  Get last_state_change.
 *
 *  @return The last_state_change.
 */
opt<time_t> const& service::last_state_change() const throw () {
  return (_last_state_change);
}

/**
 *  Get last_time_critical.
 *
 *  @return The last_time_critical.
 */
opt<time_t> const& service::last_time_critical() const throw () {
  return (_last_time_critical);
}

/**
 *  Get last_time_ok.
 *
 *  @return The last_time_ok.
 */
opt<time_t> const& service::last_time_ok() const throw () {
  return (_last_time_ok);
}

/**
 *  Get last_time_unknown.
 *
 *  @return The last_time_unknown.
 */
opt<time_t> const& service::last_time_unknown() const throw () {
  return (_last_time_unknown);
}

/**
 *  Get last_time_warning.
 *
 *  @return The last_time_warning.
 */
opt<time_t> const& service::last_time_warning() const throw () {
  return (_last_time_warning);
}

/**
 *  Get long_plugin_output.
 *
 *  @return The long_plugin_output.
 */
opt<std::string> const& service::long_plugin_output() const throw () {
  return (_long_plugin_output);
}

/**
 *  Get max_attempts.
 *
 *  @return The max_attempts.
 */
opt<unsigned int> const& service::max_attempts() const throw () {
  return (_max_attempts);
}

/**
 *  Get modified_attributes.
 *
 *  @return The modified_attributes.
 */
opt<unsigned long> const& service::modified_attributes() const throw () {
  return (_modified_attributes);
}

/**
 *  Get next_check.
 *
 *  @return The next_check.
 */
opt<time_t> const& service::next_check() const throw () {
  return (_next_check);
}

/**
 *  Get normal_check_interval.
 *
 *  @return The normal_check_interval.
 */
opt<unsigned int> const& service::normal_check_interval() const throw () {
  return (_normal_check_interval);
}

/**
 *  Get notification_period.
 *
 *  @return The notification_period.
 */
opt<std::string> const& service::notification_period() const throw () {
  return (_notification_period);
}

/**
 *  Get notifications_enabled.
 *
 *  @return The notifications_enabled.
 */
opt<bool> const& service::notifications_enabled() const throw () {
  return (_notifications_enabled);
}

/**
 *  Get notified_on_critical.
 *
 *  @return The notified_on_critical.
 */
opt<bool> const& service::notified_on_critical() const throw () {
  return (_notified_on_critical);
}

/**
 *  Get notified_on_unknown.
 *
 *  @return The notified_on_unknown.
 */
opt<bool> const& service::notified_on_unknown() const throw () {
  return (_notified_on_unknown);
}

/**
 *  Get notified_on_warning.
 *
 *  @return The notified_on_warning.
 */
opt<bool> const& service::notified_on_warning() const throw () {
  return (_notified_on_warning);
}

/**
 *  Get obsess_over_service.
 *
 *  @return The obsess_over_service.
 */
opt<int> const& service::obsess_over_service() const throw () {
  return (_obsess_over_service);
}

/**
 *  Get passive_checks_enabled.
 *
 *  @return The passive_checks_enabled.
 */
opt<bool> const& service::passive_checks_enabled() const throw () {
  return (_passive_checks_enabled);
}

/**
 *  Get percent_state_change.
 *
 *  @return The percent_state_change.
 */
opt<double> const& service::percent_state_change() const throw () {
  return (_percent_state_change);
}

/**
 *  Get performance_data.
 *
 *  @return The performance_data.
 */
opt<std::string> const& service::performance_data() const throw () {
  return (_performance_data);
}

/**
 *  Get plugin_output.
 *
 *  @return The plugin_output.
 */
opt<std::string> const& service::plugin_output() const throw () {
  return (_plugin_output);
}

/**
 *  Get problem_has_been_acknowledged.
 *
 *  @return The problem_has_been_acknowledged.
 */
opt<bool> const& service::problem_has_been_acknowledged() const throw () {
  return (_problem_has_been_acknowledged);
}

/**
 *  Get process_performance_data.
 *
 *  @return The process_performance_data.
 */
opt<int> const& service::process_performance_data() const throw () {
  return (_process_performance_data);
}

/**
 *  Get recovery_been_sent.
 *
 *  @return The recovery_been_sent.
 */
opt<bool> const& service::recovery_been_sent() const throw () {
  return (_recovery_been_sent);
}

/**
 *  Get retry_check_interval.
 *
 *  @return The retry_check_interval.
 */
opt<unsigned int> const& service::retry_check_interval() const throw () {
  return (_retry_check_interval);
}

/**
 *  Get service_id.
 *
 *  @return The service_id.
 */
uint64_t service::service_id() const throw () {
  return (_service_id);
}

/**
 *  Get service_description.
 *
 *  @return The service_description.
 */
std::string const& service::service_description() const throw () {
  return (_service_description);
}

/**
 *  Get state_history.
 *
 *  @return The state_history.
 */
opt<std::vector<int> > const& service::state_history() const throw () {
  return (_state_history);
}

/**
 *  Get state_type.
 *
 *  @return The state_type.
 */
opt<int> const& service::state_type() const throw () {
  return (_state_type);
}

/**
 *  Set acknowledgement_type.
 *
 *  @param[in] value The new acknowledgement_type.
 */
bool service::_set_acknowledgement_type(int value) {
  _acknowledgement_type = value;
  return (true);
}

/**
 *  Set active_checks_enabled.
 *
 *  @param[in] value The new active_checks_enabled.
 */
bool service::_set_active_checks_enabled(bool value) {
  _active_checks_enabled = value;
  return (true);
}

/**
 *  Set check_command.
 *
 *  @param[in] value The new check_command.
 */
bool service::_set_check_command(std::string const& value) {
  _check_command = value;
  return (true);
}

/**
 *  Set check_execution_time.
 *
 *  @param[in] value The new check_execution_time.
 */
bool service::_set_check_execution_time(double value) {
  _check_execution_time = value;
  return (true);
}

/**
 *  Set check_flapping_recovery_notification.
 *
 *  @param[in] value The new check_flapping_recovery_notification.
 */
bool service::_set_check_flapping_recovery_notification(int value) {
  _check_flapping_recovery_notification = value;
  return (true);
}

/**
 *  Set check_latency.
 *
 *  @param[in] value The new check_latency.
 */
bool service::_set_check_latency(double value) {
  _check_latency = value;
  return (true);
}

/**
 *  Set check_options.
 *
 *  @param[in] value The new check_options.
 */
bool service::_set_check_options(int value) {
  _check_options = value;
  return (true);
}

/**
 *  Set check_period.
 *
 *  @param[in] value The new check_period.
 */
bool service::_set_check_period(std::string const& value) {
  _check_period = value;
  return (true);
}

/**
 *  Set check_type.
 *
 *  @param[in] value The new check_type.
 */
bool service::_set_check_type(int value) {
  _check_type = value;
  return (true);
}

/**
 *  Set current_attempt.
 *
 *  @param[in] value The new current_attempt.
 */
bool service::_set_current_attempt(int value) {
  _current_attempt = value;
  return (true);
}

/**
 *  Set current_event_id.
 *
 *  @param[in] value The new current_event_id.
 */
bool service::_set_current_event_id(uint64_t value) {
  _current_event_id = value;
  return (true);
}

/**
 *  Set current_notification_id.
 *
 *  @param[in] value The new current_notification_id.
 */
bool service::_set_current_notification_id(uint64_t value) {
  _current_notification_id = value;
  return (true);
}

/**
 *  Set current_notification_number.
 *
 *  @param[in] value The new current_notification_number.
 */
bool service::_set_current_notification_number(int value) {
  _current_notification_number = value;
  return (true);
}

/**
 *  Set current_problem_id.
 *
 *  @param[in] value The new current_problem_id.
 */
bool service::_set_current_problem_id(uint64_t value) {
  _current_problem_id = value;
  return (true);
}

/**
 *  Set current_state.
 *
 *  @param[in] value The new current_state.
 */
bool service::_set_current_state(int value) {
  _current_state = value;
  return (true);
}

/**
 *  Set event_handler.
 *
 *  @param[in] value The new event_handler.
 */
bool service::_set_event_handler(std::string const& value) {
  _event_handler = value;
  return (true);
}

/**
 *  Set event_handler_enabled.
 *
 *  @param[in] value The new event_handler_enabled.
 */
bool service::_set_event_handler_enabled(bool value) {
  _event_handler_enabled = value;
  return (true);
}

/**
 *  Deprecated.
 *
 *  @param[in] value  Unused.
 */
bool service::_set_failure_prediction_enabled(bool value) {
  (void)value;
  return (true);
}

/**
 *  Set flap_detection_enabled.
 *
 *  @param[in] value The new flap_detection_enabled.
 */
bool service::_set_flap_detection_enabled(bool value) {
  _flap_detection_enabled = value;
  return (true);
}

/**
 *  Set has_been_checked.
 *
 *  @param[in] value The new has_been_checked.
 */
bool service::_set_has_been_checked(bool value) {
  _has_been_checked = value;
  return (true);
}

/**
 *  Set host_name.
 *
 *  @param[in] value The new host_name.
 */
bool service::_set_host_name(std::string const& value) {
  _host_name = value;
  return (true);
}

/**
 *  Set is_flapping.
 *
 *  @param[in] value The new is_flapping.
 */
bool service::_set_is_flapping(bool value) {
  _is_flapping = value;
  return (true);
}

/**
 *  Set last_acknowledgement.
 *
 *  @param[in] value  The new last acknowledgement.
 */
bool service::_set_last_acknowledgement(time_t value) {
  _last_acknowledgement = value;
  return (true);
}

/**
 *  Set last_check.
 *
 *  @param[in] value The new last_check.
 */
bool service::_set_last_check(time_t value) {
  _last_check = value;
  return (true);
}

/**
 *  Set last_event_id.
 *
 *  @param[in] value The new last_event_id.
 */
bool service::_set_last_event_id(uint64_t value) {
  _last_event_id = value;
  return (true);
}

/**
 *  Set last_hard_state.
 *
 *  @param[in] value The new last_hard_state.
 */
bool service::_set_last_hard_state(time_t value) {
  _last_hard_state = value;
  return (true);
}

/**
 *  Set last_hard_state_change.
 *
 *  @param[in] value The new last_hard_state_change.
 */
bool service::_set_last_hard_state_change(time_t value) {
  _last_hard_state_change = value;
  return (true);
}

/**
 *  Set last_notification.
 *
 *  @param[in] value The new last_notification.
 */
bool service::_set_last_notification(time_t value) {
  _last_notification = value;
  return (true);
}

/**
 *  Set last_problem_id.
 *
 *  @param[in] value The new last_problem_id.
 */
bool service::_set_last_problem_id(uint64_t value) {
  _last_problem_id = value;
  return (true);
}

/**
 *  Set last_state.
 *
 *  @param[in] value The new last_state.
 */
bool service::_set_last_state(time_t value) {
  _last_state = value;
  return (true);
}

/**
 *  Set last_state_change.
 *
 *  @param[in] value The new last_state_change.
 */
bool service::_set_last_state_change(time_t value) {
  _last_state_change = value;
  return (true);
}

/**
 *  Set last_time_critical.
 *
 *  @param[in] value The new last_time_critical.
 */
bool service::_set_last_time_critical(time_t value) {
  _last_time_critical = value;
  return (true);
}

/**
 *  Set last_time_ok.
 *
 *  @param[in] value The new last_time_ok.
 */
bool service::_set_last_time_ok(time_t value) {
  _last_time_ok = value;
  return (true);
}

/**
 *  Set last_time_unknown.
 *
 *  @param[in] value The new last_time_unknown.
 */
bool service::_set_last_time_unknown(time_t value) {
  _last_time_unknown = value;
  return (true);
}

/**
 *  Set last_time_warning.
 *
 *  @param[in] value The new last_time_warning.
 */
bool service::_set_last_time_warning(time_t value) {
  _last_time_warning = value;
  return (true);
}

/**
 *  Set long_plugin_output.
 *
 *  @param[in] value The new long_plugin_output.
 */
bool service::_set_long_plugin_output(std::string const& value) {
  _long_plugin_output = value;
  return (true);
}

/**
 *  Set max_attempts.
 *
 *  @param[in] value The new max_attempts.
 */
bool service::_set_max_attempts(unsigned int value) {
  if (value) {
    _max_attempts = value;
    return (true);
  }
  return (false);
}

/**
 *  Set modified_attributes.
 *
 *  @param[in] value The new modified_attributes.
 */
bool service::_set_modified_attributes(unsigned long value) {
  _modified_attributes = value;
  return (true);
}

/**
 *  Set next_check.
 *
 *  @param[in] value The new next_check.
 */
bool service::_set_next_check(time_t value) {
  _next_check = value;
  return (true);
}

/**
 *  Set normal_check_interval.
 *
 *  @param[in] value The new normal_check_interval.
 */
bool service::_set_normal_check_interval(unsigned int value) {
  _normal_check_interval = value;
  return (true);
}

/**
 *  Set notification_period.
 *
 *  @param[in] value The new notification_period.
 */
bool service::_set_notification_period(std::string const& value) {
  _notification_period = value;
  return (true);
}

/**
 *  Set notifications_enabled.
 *
 *  @param[in] value The new notifications_enabled.
 */
bool service::_set_notifications_enabled(bool value) {
  _notifications_enabled = value;
  return (true);
}

/**
 *  Set notified_on_critical.
 *
 *  @param[in] value The new notified_on_critical.
 */
bool service::_set_notified_on_critical(bool value) {
  _notified_on_critical = value;
  return (true);
}

/**
 *  Set notified_on_unknown.
 *
 *  @param[in] value The new notified_on_unknown.
 */
bool service::_set_notified_on_unknown(bool value) {
  _notified_on_unknown = value;
  return (true);
}

/**
 *  Set notified_on_warning.
 *
 *  @param[in] value The new notified_on_warning.
 */
bool service::_set_notified_on_warning(bool value) {
  _notified_on_warning = value;
  return (true);
}

/**
 *  Set obsess_over_service.
 *
 *  @param[in] value The new obsess_over_service.
 */
bool service::_set_obsess_over_service(int value) {
  _obsess_over_service = value;
  return (true);
}

/**
 *  Set passive_checks_enabled.
 *
 *  @param[in] value The new passive_checks_enabled.
 */
bool service::_set_passive_checks_enabled(bool value) {
  _passive_checks_enabled = value;
  return (true);
}

/**
 *  Set percent_state_change.
 *
 *  @param[in] value The new percent_state_change.
 */
bool service::_set_percent_state_change(double value) {
  _percent_state_change = value;
  return (true);
}

/**
 *  Set performance_data.
 *
 *  @param[in] value The new performance_data.
 */
bool service::_set_performance_data(std::string const& value) {
  _performance_data = value;
  return (true);
}

/**
 *  Set plugin_output.
 *
 *  @param[in] value The new plugin_output.
 */
bool service::_set_plugin_output(std::string const& value) {
  _plugin_output = value;
  return (true);
}

/**
 *  Set problem_has_been_acknowledged.
 *
 *  @param[in] value The new problem_has_been_acknowledged.
 */
bool service::_set_problem_has_been_acknowledged(bool value) {
  _problem_has_been_acknowledged = value;
  return (true);
}

/**
 *  Set process_performance_data.
 *
 *  @param[in] value The new process_performance_data.
 */
bool service::_set_process_performance_data(int value) {
  _process_performance_data = value;
  return (true);
}

/**
 *  Set _set_recovery_been_sent.
 *
 *  @param[in] value The new _set_recovery_been_sent.
 */
bool service::_set_recovery_been_sent(bool value) {
  _recovery_been_sent = value;
  return (true);
}

/**
 *  Set retry_check_interval.
 *
 *  @param[in] value The new retry_check_interval.
 */
bool service::_set_retry_check_interval(unsigned int value) {
  _retry_check_interval = value;
  return (true);
}

/**
 *  Set service_id.
 *
 *  @param[in] value The new service_id.
 */
bool service::_set_service_id(uint64_t value) {
  _service_id = value;
  return (true);
}

/**
 *  Set service_description.
 *
 *  @param[in] value The new service_description.
 */
bool service::_set_service_description(std::string const& value) {
  _service_description = value;
  return (true);
}

/**
 *  Set state_history.
 *
 *  @param[in] value The new state_history.
 */
bool service::_set_state_history(std::string const& value) {
  std::vector<int>& state_history(*_state_history);
  char const* ptr(value.c_str());
  while (*ptr) {
    char* endptr;
    int state;
    state = strtol(ptr, &endptr, 10);
    if (*endptr && (*endptr != ',')) {
      _state_history.reset();
      return (false);
    }
    state_history.push_back(state);
    ptr = endptr;
    if (*ptr)
      ++ptr;
  }
  _state_history.set(state_history);
  return (true);
}

/**
 *  Set state_type.
 *
 *  @param[in] value The new state_type.
 */
bool service::_set_state_type(int value) {
  _state_type = value;
  return (true);
}

/*
** Copyright 2011-2013,2015 Merethis
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

#include "com/centreon/engine/retention/program.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::retention;

#define SETTER(type, method) \
  &object::setter<program, type, &program::method>::generic

program::setters const program::_setters[] = {
    {"active_host_checks_enabled",
     SETTER(bool, _set_active_host_checks_enabled)},
    {"active_service_checks_enabled",
     SETTER(bool, _set_active_service_checks_enabled)},
    {"check_host_freshness", SETTER(bool, _set_check_host_freshness)},
    {"check_service_freshness", SETTER(bool, _set_check_service_freshness)},
    {"enable_event_handlers", SETTER(bool, _set_enable_event_handlers)},
    {"enable_failure_prediction", SETTER(bool, _set_enable_failure_prediction)},
    {"enable_flap_detection", SETTER(bool, _set_enable_flap_detection)},
    {"enable_notifications", SETTER(bool, _set_enable_notifications)},
    {"global_host_event_handler",
     SETTER(std::string const&, _set_global_host_event_handler)},
    {"global_service_event_handler",
     SETTER(std::string const&, _set_global_service_event_handler)},
    {"modified_host_attributes",
     SETTER(unsigned long, _set_modified_host_attributes)},
    {"modified_service_attributes",
     SETTER(unsigned long, _set_modified_service_attributes)},
    {"next_comment_id", SETTER(unsigned long, _set_next_comment_id)},
    {"next_downtime_id", SETTER(unsigned long, _set_next_downtime_id)},
    {"next_event_id", SETTER(unsigned long, _set_next_event_id)},
    {"next_notification_id", SETTER(unsigned long, _set_next_notification_id)},
    {"next_problem_id", SETTER(unsigned long, _set_next_problem_id)},
    {"obsess_over_hosts", SETTER(bool, _set_obsess_over_hosts)},
    {"obsess_over_services", SETTER(bool, _set_obsess_over_services)},
    {"passive_host_checks_enabled",
     SETTER(bool, _set_passive_host_checks_enabled)},
    {"passive_service_checks_enabled",
     SETTER(bool, _set_passive_service_checks_enabled)},
    {"process_performance_data", SETTER(bool, _set_process_performance_data)}};

/**
 *  Constructor.
 */
program::program() : object(object::program) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 */
program::program(program const& right) : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
program::~program() throw() {}

/**
 *  Copy operator.
 *
 *  @param[in] right The object to copy.
 *
 *  @return This object.
 */
program& program::operator=(program const& right) {
  if (this != &right) {
    object::operator=(right);
    _active_host_checks_enabled = right._active_host_checks_enabled;
    _active_service_checks_enabled = right._active_service_checks_enabled;
    _check_host_freshness = right._check_host_freshness;
    _check_service_freshness = right._check_service_freshness;
    _enable_event_handlers = right._enable_event_handlers;
    _enable_flap_detection = right._enable_flap_detection;
    _enable_notifications = right._enable_notifications;
    _global_host_event_handler = right._global_host_event_handler;
    _global_service_event_handler = right._global_service_event_handler;
    _modified_host_attributes = right._modified_host_attributes;
    _modified_service_attributes = right._modified_service_attributes;
    _next_comment_id = right._next_comment_id;
    _next_downtime_id = right._next_downtime_id;
    _next_event_id = right._next_event_id;
    _next_notification_id = right._next_notification_id;
    _next_problem_id = right._next_problem_id;
    _obsess_over_hosts = right._obsess_over_hosts;
    _obsess_over_services = right._obsess_over_services;
    _passive_host_checks_enabled = right._passive_host_checks_enabled;
    _passive_service_checks_enabled = right._passive_service_checks_enabled;
    _process_performance_data = right._process_performance_data;
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
bool program::operator==(program const& right) const throw() {
  return (
      object::operator==(right) &&
      _active_host_checks_enabled == right._active_host_checks_enabled &&
      _active_service_checks_enabled == right._active_service_checks_enabled &&
      _check_host_freshness == right._check_host_freshness &&
      _check_service_freshness == right._check_service_freshness &&
      _enable_event_handlers == right._enable_event_handlers &&
      _enable_flap_detection == right._enable_flap_detection &&
      _enable_notifications == right._enable_notifications &&
      _global_host_event_handler == right._global_host_event_handler &&
      _global_service_event_handler == right._global_service_event_handler &&
      _modified_host_attributes == right._modified_host_attributes &&
      _modified_service_attributes == right._modified_service_attributes &&
      _next_comment_id == right._next_comment_id &&
      _next_downtime_id == right._next_downtime_id &&
      _next_event_id == right._next_event_id &&
      _next_notification_id == right._next_notification_id &&
      _next_problem_id == right._next_problem_id &&
      _obsess_over_hosts == right._obsess_over_hosts &&
      _obsess_over_services == right._obsess_over_services &&
      _passive_host_checks_enabled == right._passive_host_checks_enabled &&
      _passive_service_checks_enabled ==
          right._passive_service_checks_enabled &&
      _process_performance_data == right._process_performance_data);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool program::operator!=(program const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Set new value on specific property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool program::set(char const* key, char const* value) {
  for (unsigned int i(0); i < sizeof(_setters) / sizeof(_setters[0]); ++i)
    if (!strcmp(_setters[i].name, key))
      return ((_setters[i].func)(*this, value));
  return (false);
}

/**
 *  Get active_host_checks_enabled.
 *
 *  @return The active_host_checks_enabled.
 */
opt<bool> const& program::active_host_checks_enabled() const throw() {
  return (_active_host_checks_enabled);
}

/**
 *  Get active_service_checks_enabled.
 *
 *  @return The active_service_checks_enabled.
 */
opt<bool> const& program::active_service_checks_enabled() const throw() {
  return (_active_service_checks_enabled);
}

/**
 *  Get check_host_freshness.
 *
 *  @return The check_host_freshness.
 */
opt<bool> const& program::check_host_freshness() const throw() {
  return (_check_host_freshness);
}

/**
 *  Get check_service_freshness.
 *
 *  @return The check_service_freshness.
 */
opt<bool> const& program::check_service_freshness() const throw() {
  return (_check_service_freshness);
}

/**
 *  Get enable_event_handlers.
 *
 *  @return The enable_event_handlers.
 */
opt<bool> const& program::enable_event_handlers() const throw() {
  return (_enable_event_handlers);
}

/**
 *  Get enable_flap_detection.
 *
 *  @return The enable_flap_detection.
 */
opt<bool> const& program::enable_flap_detection() const throw() {
  return (_enable_flap_detection);
}

/**
 *  Get enable_notifications.
 *
 *  @return The enable_notifications.
 */
opt<bool> const& program::enable_notifications() const throw() {
  return (_enable_notifications);
}

/**
 *  Get global_host_event_handler.
 *
 *  @return The global_host_event_handler.
 */
opt<std::string> const& program::global_host_event_handler() const throw() {
  return (_global_host_event_handler);
}

/**
 *  Get global_service_event_handler.
 *
 *  @return The global_service_event_handler.
 */
opt<std::string> const& program::global_service_event_handler() const throw() {
  return (_global_service_event_handler);
}

/**
 *  Get modified_host_attributes.
 *
 *  @return The modified_host_attributes.
 */
opt<unsigned long> const& program::modified_host_attributes() const throw() {
  return (_modified_host_attributes);
}

/**
 *  Get modified_service_attributes.
 *
 *  @return The modified_service_attributes.
 */
opt<unsigned long> const& program::modified_service_attributes() const throw() {
  return (_modified_service_attributes);
}

/**
 *  Get next_comment_id.
 *
 *  @return The next_comment_id.
 */
opt<unsigned long> const& program::next_comment_id() const throw() {
  return (_next_comment_id);
}

/**
 *  Get next_downtime_id.
 *
 *  @return The next_downtime_id.
 */
opt<unsigned long> const& program::next_downtime_id() const throw() {
  return (_next_downtime_id);
}

/**
 *  Get next_event_id.
 *
 *  @return The next_event_id.
 */
opt<unsigned long> const& program::next_event_id() const throw() {
  return (_next_event_id);
}

/**
 *  Get next_notification_id.
 *
 *  @return The next_notification_id.
 */
opt<unsigned long> const& program::next_notification_id() const throw() {
  return (_next_notification_id);
}

/**
 *  Get next_problem_id.
 *
 *  @return The next_problem_id.
 */
opt<unsigned long> const& program::next_problem_id() const throw() {
  return (_next_problem_id);
}

/**
 *  Get obsess_over_hosts.
 *
 *  @return The obsess_over_hosts.
 */
opt<bool> const& program::obsess_over_hosts() const throw() {
  return (_obsess_over_hosts);
}

/**
 *  Get obsess_over_services.
 *
 *  @return The obsess_over_services.
 */
opt<bool> const& program::obsess_over_services() const throw() {
  return (_obsess_over_services);
}

/**
 *  Get passive_host_checks_enabled.
 *
 *  @return The passive_host_checks_enabled.
 */
opt<bool> const& program::passive_host_checks_enabled() const throw() {
  return (_passive_host_checks_enabled);
}

/**
 *  Get passive_service_checks_enabled.
 *
 *  @return The passive_service_checks_enabled.
 */
opt<bool> const& program::passive_service_checks_enabled() const throw() {
  return (_passive_service_checks_enabled);
}

/**
 *  Get process_performance_data.
 *
 *  @return The process_performance_data.
 */
opt<bool> const& program::process_performance_data() const throw() {
  return (_process_performance_data);
}

/**
 *  Set active_host_checks_enabled.
 *
 *  @param[in] value The new active_host_checks_enabled.
 */
bool program::_set_active_host_checks_enabled(bool value) {
  _active_host_checks_enabled = value;
  return (true);
}

/**
 *  Set active_service_checks_enabled.
 *
 *  @param[in] value The new active_service_checks_enabled.
 */
bool program::_set_active_service_checks_enabled(bool value) {
  _active_service_checks_enabled = value;
  return (true);
}

/**
 *  Set check_host_freshness.
 *
 *  @param[in] value The new check_host_freshness.
 */
bool program::_set_check_host_freshness(bool value) {
  _check_host_freshness = value;
  return (true);
}

/**
 *  Set check_service_freshness.
 *
 *  @param[in] value The new check_service_freshness.
 */
bool program::_set_check_service_freshness(bool value) {
  _check_service_freshness = value;
  return (true);
}

/**
 *  Set enable_event_handlers.
 *
 *  @param[in] value The new enable_event_handlers.
 */
bool program::_set_enable_event_handlers(bool value) {
  _enable_event_handlers = value;
  return (true);
}

/**
 *  Deprecated.
 *
 *  @param[in] value  Unused.
 */
bool program::_set_enable_failure_prediction(bool value) {
  (void)value;
  return (true);
}

/**
 *  Set enable_flap_detection.
 *
 *  @param[in] value The new enable_flap_detection.
 */
bool program::_set_enable_flap_detection(bool value) {
  _enable_flap_detection = value;
  return (true);
}

/**
 *  Set enable_notifications.
 *
 *  @param[in] value The new enable_notifications.
 */
bool program::_set_enable_notifications(bool value) {
  _enable_notifications = value;
  return (true);
}

/**
 *  Set global_host_event_handler.
 *
 *  @param[in] value The new global_host_event_handler.
 */
bool program::_set_global_host_event_handler(std::string const& value) {
  _global_host_event_handler = value;
  return (true);
}

/**
 *  Set global_service_event_handler.
 *
 *  @param[in] value The new global_service_event_handler.
 */
bool program::_set_global_service_event_handler(std::string const& value) {
  _global_service_event_handler = value;
  return (true);
}

/**
 *  Set modified_host_attributes.
 *
 *  @param[in] value The new modified_host_attributes.
 */
bool program::_set_modified_host_attributes(unsigned long value) {
  _modified_host_attributes = value;
  return (true);
}

/**
 *  Set modified_service_attributes.
 *
 *  @param[in] value The new modified_service_attributes.
 */
bool program::_set_modified_service_attributes(unsigned long value) {
  _modified_service_attributes = value;
  return (true);
}

/**
 *  Set next_comment_id.
 *
 *  @param[in] value The new next_comment_id.
 */
bool program::_set_next_comment_id(unsigned long value) {
  _next_comment_id = value;
  return (true);
}

/**
 *  Set next_downtime_id.
 *
 *  @param[in] value The new next_downtime_id.
 */
bool program::_set_next_downtime_id(unsigned long value) {
  _next_downtime_id = value;
  return (true);
}

/**
 *  Set next_event_id.
 *
 *  @param[in] value The new next_event_id.
 */
bool program::_set_next_event_id(unsigned long value) {
  _next_event_id = value;
  return (true);
}

/**
 *  Set next_notification_id.
 *
 *  @param[in] value The new next_notification_id.
 */
bool program::_set_next_notification_id(unsigned long value) {
  _next_notification_id = value;
  return (true);
}

/**
 *  Set next_problem_id.
 *
 *  @param[in] value The new next_problem_id.
 */
bool program::_set_next_problem_id(unsigned long value) {
  _next_problem_id = value;
  return (true);
}

/**
 *  Set obsess_over_hosts.
 *
 *  @param[in] value The new obsess_over_hosts.
 */
bool program::_set_obsess_over_hosts(bool value) {
  _obsess_over_hosts = value;
  return (true);
}

/**
 *  Set obsess_over_services.
 *
 *  @param[in] value The new obsess_over_services.
 */
bool program::_set_obsess_over_services(bool value) {
  _obsess_over_services = value;
  return (true);
}

/**
 *  Set passive_host_checks_enabled.
 *
 *  @param[in] value The new passive_host_checks_enabled.
 */
bool program::_set_passive_host_checks_enabled(bool value) {
  _passive_host_checks_enabled = value;
  return (true);
}

/**
 *  Set passive_service_checks_enabled.
 *
 *  @param[in] value The new passive_service_checks_enabled.
 */
bool program::_set_passive_service_checks_enabled(bool value) {
  _passive_service_checks_enabled = value;
  return (true);
}

/**
 *  Set process_performance_data.
 *
 *  @param[in] value The new process_performance_data.
 */
bool program::_set_process_performance_data(bool value) {
  _process_performance_data = value;
  return (true);
}

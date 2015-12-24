/*
** Copyright 2011-2015 Merethis
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

#include "com/centreon/engine/configuration/deprecated.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/string.hh"

extern int config_warnings;
extern int config_errors;

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::logging;

#define SETTER(type, method) \
  &object::setter<service, type, &service::method>::generic

service::setters const service::_setters[] = {
  { "host",                         SETTER(std::string const&, _set_hosts) },
  { "hosts",                        SETTER(std::string const&, _set_hosts) },
  { "host_name",                    SETTER(std::string const&, _set_hosts) },
  { "service_description",          SETTER(std::string const&, _set_service_description) },
  { "service_id",                   SETTER(unsigned int, _set_service_id) },
  { "_SERVICE_ID",                  SETTER(unsigned int, _set_service_id) },
  { "description",                  SETTER(std::string const&, _set_service_description) },
  { "check_command",                SETTER(std::string const&, _set_check_command) },
  { "check_period",                 SETTER(std::string const&, _set_check_period) },
  { "check_timeout",                SETTER(std::string const&, _set_check_timeout) },
  { "event_handler",                SETTER(std::string const&, _set_event_handler) },
  { "initial_state",                SETTER(std::string const&, _set_initial_state) },
  { "max_check_attempts",           SETTER(unsigned int, _set_max_check_attempts) },
  { "check_interval",               SETTER(std::string const&, _set_check_interval) },
  { "normal_check_interval",        SETTER(std::string const&, _set_check_interval) },
  { "retry_interval",               SETTER(std::string const&, _set_retry_interval) },
  { "retry_check_interval",         SETTER(std::string const&, _set_retry_interval) },
  { "active_checks_enabled",        SETTER(bool, _set_checks_active) },
  { "is_volatile",                  SETTER(bool, _set_is_volatile) },
  { "obsess_over_service",          SETTER(bool, _set_obsess_over_service) },
  { "event_handler_enabled",        SETTER(bool, _set_event_handler_enabled) },
  { "check_freshness",              SETTER(bool, _set_check_freshness) },
  { "freshness_threshold",          SETTER(unsigned int, _set_freshness_threshold) },
  { "low_flap_threshold",           SETTER(unsigned int, _set_low_flap_threshold) },
  { "high_flap_threshold",          SETTER(unsigned int, _set_high_flap_threshold) },
  { "flap_detection_enabled",       SETTER(bool, _set_flap_detection_enabled) },
  { "flap_detection_options",       SETTER(std::string const&, _set_flap_detection_options) },
  { "timezone",                     SETTER(std::string const&, _set_timezone) }
};

char const* const service::_deprecated[][2] = {
  { "action_url",                   deprecated::default_msg },
  { "contact_groups",               deprecated::groups_msg },
  { "contacts",                     deprecated::notification_msg },
  { "display_name",                 deprecated::default_msg },
  { "failure_prediction_enabled",   deprecated::default_msg },
  { "failure_prediction_options",   deprecated::default_msg },
  { "first_notification_delay",     deprecated::notification_msg },
  { "hostgroup",                    deprecated::groups_msg },
  { "hostgroups",                   deprecated::groups_msg },
  { "hostgroup_name",               deprecated::groups_msg },
  { "service_groups",               deprecated::groups_msg },
  { "servicegroups",                deprecated::groups_msg },
  { "icon_image",                   deprecated::default_msg },
  { "icon_image_alt",               deprecated::default_msg },
  { "notes",                        deprecated::default_msg },
  { "notes_url",                    deprecated::default_msg },
  { "notifications_enabled",        deprecated::notification_msg },
  { "notification_interval",        deprecated::notification_msg },
  { "notification_options",         deprecated::notification_msg },
  { "notification_period",          deprecated::notification_msg },
  { "parallelize_check",            deprecated::engine_performance_msg },
  { "passive_checks_enabled",       deprecated::check_acceptance_msg },
  { "process_perf_data",            deprecated::perfdata_msg },
  { "retain_status_information",    deprecated::retention_usage_msg },
  { "retain_nonstatus_information", deprecated::retention_usage_msg },
  { "stalking_options",             deprecated::default_msg }
};

// Default values.
static bool const           default_checks_active(true);
static bool const           default_check_freshness(0);
static unsigned int const   default_check_interval(300);
static bool const           default_event_handler_enabled(true);
static bool const           default_flap_detection_enabled(true);
static unsigned short const default_flap_detection_options(
                              service::ok
                              | service::warning
                              | service::unknown
                              | service::critical);
static unsigned int const   default_freshness_threshold(0);
static unsigned int const   default_high_flap_threshold(0);
static unsigned int const   default_initial_state(STATE_OK);
static bool const           default_is_volatile(false);
static unsigned int const   default_low_flap_threshold(0);
static unsigned int const   default_max_check_attempts(3);
static bool const           default_obsess_over_service(true);
static unsigned int const   default_retry_interval(60);
static unsigned int const   default_check_timeout(0);

/**
 *  Default constructor.
 */
service::service()
  : object(object::service),
    _checks_active(default_checks_active),
    _check_freshness(default_check_freshness),
    _check_interval(default_check_interval),
    _check_timeout(default_check_timeout),
    _event_handler_enabled(default_event_handler_enabled),
    _flap_detection_enabled(default_flap_detection_enabled),
    _flap_detection_options(default_flap_detection_options),
    _freshness_threshold(default_freshness_threshold),
    _high_flap_threshold(default_high_flap_threshold),
    _initial_state(default_initial_state),
    _is_volatile(default_is_volatile),
    _low_flap_threshold(default_low_flap_threshold),
    _max_check_attempts(default_max_check_attempts),
    _obsess_over_service(default_obsess_over_service),
    _retry_interval(default_retry_interval),
    _service_id(0) {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  The service to copy.
 */
service::service(service const& other) : object(other) {
  operator=(other);
}

/**
 *  Destructor.
 */
service::~service() throw () {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  The service to copy.
 *
 *  @return This service.
 */
service& service::operator=(service const& other) {
  if (this != &other) {
    object::operator=(other);
    _checks_active = other._checks_active;
    _check_command = other._check_command;
    _check_freshness = other._check_freshness;
    _check_interval = other._check_interval;
    _check_period = other._check_period;
    _check_timeout = other._check_timeout;
    _customvariables = other._customvariables;
    _event_handler = other._event_handler;
    _event_handler_enabled = other._event_handler_enabled;
    _flap_detection_enabled = other._flap_detection_enabled;
    _flap_detection_options = other._flap_detection_options;
    _freshness_threshold = other._freshness_threshold;
    _high_flap_threshold = other._high_flap_threshold;
    _hosts = other._hosts;
    _initial_state = other._initial_state;
    _is_volatile = other._is_volatile;
    _low_flap_threshold = other._low_flap_threshold;
    _max_check_attempts = other._max_check_attempts;
    _obsess_over_service = other._obsess_over_service;
    _retry_interval = other._retry_interval;
    _service_description = other._service_description;
    _service_id = other._service_id;
    _timezone = other._timezone;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] other  The service to compare.
 *
 *  @return True if is the same service, otherwise false.
 */
bool service::operator==(service const& other) const throw () {
  return (object::operator==(other)
          && _checks_active == other._checks_active
          && _check_command == other._check_command
          && _check_freshness == other._check_freshness
          && _check_interval == other._check_interval
          && _check_period == other._check_period
          && _check_timeout == other._check_timeout
          && std::operator==(_customvariables, other._customvariables)
          && _event_handler == other._event_handler
          && _event_handler_enabled == other._event_handler_enabled
          && _flap_detection_enabled == other._flap_detection_enabled
          && _flap_detection_options == other._flap_detection_options
          && _freshness_threshold == other._freshness_threshold
          && _high_flap_threshold == other._high_flap_threshold
          && _hosts == other._hosts
          && _initial_state == other._initial_state
          && _is_volatile == other._is_volatile
          && _low_flap_threshold == other._low_flap_threshold
          && _max_check_attempts == other._max_check_attempts
          && _obsess_over_service == other._obsess_over_service
          && _retry_interval == other._retry_interval
          && _service_description == other._service_description
          && _service_id == other._service_id
          && _timezone == other._timezone);
}

/**
 *  Inequal operator.
 *
 *  @param[in] other  The service to compare.
 *
 *  @return True if is not the same service, otherwise false.
 */
bool service::operator!=(service const& other) const throw () {
  return (!operator==(other));
}

/**
 *  Less-than operator.
 *
 *  @param[in] other  Object to compare to.
 *
 *  @return True if this object is less than right.
 */
bool service::operator<(service const& other) const throw () {
  if (_hosts != other._hosts)
    return (_hosts < other._hosts);
  else if (_service_id != other._service_id)
    return (_service_id < other._service_id);
  else if (_service_description != other._service_description)
    return (_service_description < other._service_description);
  else if (_checks_active != other._checks_active)
    return (_checks_active < other._checks_active);
  else if (_check_command != other._check_command)
    return (_check_command < other._check_command);
  else if (_check_freshness != other._check_freshness)
    return (_check_freshness < other._check_freshness);
  else if (_check_interval != other._check_interval)
    return (_check_interval < other._check_interval);
  else if (_check_period != other._check_period)
    return (_check_period < other._check_period);
  else if (_check_timeout != other._check_timeout)
    return (_check_timeout < other._check_timeout);
  else if (_customvariables != other._customvariables)
    return (_customvariables < other._customvariables);
  else if (_event_handler != other._event_handler)
    return (_event_handler < other._event_handler);
  else if (_event_handler_enabled != other._event_handler_enabled)
    return (_event_handler_enabled < other._event_handler_enabled);
  else if (_flap_detection_enabled != other._flap_detection_enabled)
    return (_flap_detection_enabled < other._flap_detection_enabled);
  else if (_flap_detection_options != other._flap_detection_options)
    return (_flap_detection_options < other._flap_detection_options);
  else if (_freshness_threshold != other._freshness_threshold)
    return (_freshness_threshold < other._freshness_threshold);
  else if (_high_flap_threshold != other._high_flap_threshold)
    return (_high_flap_threshold < other._high_flap_threshold);
  else if (_initial_state != other._initial_state)
    return (_initial_state < other._initial_state);
  else if (_is_volatile != other._is_volatile)
    return (_is_volatile < other._is_volatile);
  else if (_low_flap_threshold != other._low_flap_threshold)
    return (_low_flap_threshold < other._low_flap_threshold);
  else if (_max_check_attempts != other._max_check_attempts)
    return (_max_check_attempts < other._max_check_attempts);
  else if (_obsess_over_service != other._obsess_over_service)
    return (_obsess_over_service < other._obsess_over_service);
  else if (_retry_interval != other._retry_interval)
    return (_retry_interval < other._retry_interval);
  return (_timezone < other._timezone);
}

/**
 *  Check if the object is valid.
 *
 *  @return True if is a valid object, otherwise false.
 */
void service::check_validity() const {
  if (_service_id == 0)
    throw (engine_error() << "Service has no id (property 'service_id')");
  if (_service_description.empty())
    throw (engine_error() << "Service has no description (property "
           << "'service_description')");
  else if (_hosts->empty())
    throw (engine_error() << "Service '" << _service_description
           << "' is not attached to any host (property 'host_name')");
  else if (_check_command.empty()) {
    if (_checks_active)
      throw (engine_error() << "Service '" << _service_description
             << "' has active checks enabled but does not have its "
             << "check command defined");
    else if (_check_freshness)
      throw (engine_error() << "Service '" << _service_description
             << "' has freshness checks enabled but does not have its "
             << "check command defined");
  }
  return ;
}

/**
 *  Get the service key.
 *
 *  @return A pair with host name and service description.
 */
service::key_type service::key() const {
  key_type k;
  if (!_hosts->empty())
    k.first = _hosts->front();
  k.second = _service_description;
  return (k);
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void service::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "Cannot merge service with '"
           << obj.type() << "'");
  service const& tmpl(static_cast<service const&>(obj));

  MRG_OPTION(_checks_active);
  MRG_DEFAULT(_check_command);
  MRG_OPTION(_check_freshness);
  MRG_OPTION(_check_interval);
  MRG_OPTION(_check_timeout);
  MRG_DEFAULT(_check_period);
  MRG_MAP(_customvariables);
  MRG_DEFAULT(_event_handler);
  MRG_OPTION(_event_handler_enabled);
  MRG_OPTION(_flap_detection_enabled);
  MRG_OPTION(_flap_detection_options);
  MRG_OPTION(_freshness_threshold);
  MRG_OPTION(_high_flap_threshold);
  MRG_INHERIT(_hosts);
  MRG_OPTION(_initial_state);
  MRG_OPTION(_is_volatile);
  MRG_OPTION(_low_flap_threshold);
  MRG_OPTION(_max_check_attempts);
  MRG_OPTION(_obsess_over_service);
  MRG_OPTION(_retry_interval);
  MRG_DEFAULT(_service_description);
  MRG_OPTION(_timezone);
}

/**
 *  Parse and set the service property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool service::parse(char const* key, char const* value) {
  for (unsigned int i(0);
       i < sizeof(_setters) / sizeof(*_setters);
       ++i)
    if (!strcmp(_setters[i].name, key))
      return ((_setters[i].func)(*this, value));
  if (key[0] == '_' && ::strcmp(key, "_SERVICE_ID") != 0) {
    _customvariables[key + 1] = value;
    return (true);
  }
  for (unsigned int i(0);
       i < sizeof(_deprecated) / sizeof(*_deprecated);
       ++i)
    if (!strcmp(_deprecated[i][0], key)) {
      logger(log_config_warning, basic)
        << "Warning: service variable '" << key
        << "' is no longer supported.\n"
        << _deprecated[i][1];
      ++config_warnings;
      return (true);
    }
  return (false);
}

/**
 *  Get checks_active.
 *
 *  @return The checks_active.
 */
bool service::checks_active() const throw () {
  return (_checks_active);
}

/**
 *  Get check_command.
 *
 *  @return The check_command.
 */
std::string const& service::check_command() const throw () {
  return (_check_command);
}

/**
 *  Get check_freshness.
 *
 *  @return The check_freshness.
 */
bool service::check_freshness() const throw () {
  return (_check_freshness);
}

/**
 *  Get check_interval.
 *
 *  @return The check_interval.
 */
duration const& service::check_interval() const throw () {
  return (_check_interval);
}

/**
 *  Get check_period.
 *
 *  @return The check_period.
 */
std::string const& service::check_period() const throw () {
  return (_check_period);
}

/**
 *  Get check_timeout.
 *
 *  @return The check timeout.
 */
duration const& service::check_timeout() const throw() {
  return (_check_timeout);
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
 *  Get event_handler.
 *
 *  @return The event_handler.
 */
std::string const& service::event_handler() const throw () {
  return (_event_handler);
}

/**
 *  Get event_handler_enabled.
 *
 *  @return The event_handler_enabled.
 */
bool service::event_handler_enabled() const throw () {
  return (_event_handler_enabled);
}

/**
 *  Get flap_detection_enabled.
 *
 *  @return The flap_detection_enabled.
 */
bool service::flap_detection_enabled() const throw () {
  return (_flap_detection_enabled);
}

/**
 *  Get flap_detection_options.
 *
 *  @return The flap_detection_options.
 */
unsigned short service::flap_detection_options() const throw () {
  return (_flap_detection_options);
}

/**
 *  Get freshness_threshold.
 *
 *  @return The freshness_threshold.
 */
unsigned int service::freshness_threshold() const throw () {
  return (_freshness_threshold);
}

/**
 *  Get high_flap_threshold.
 *
 *  @return The high_flap_threshold.
 */
unsigned int service::high_flap_threshold() const throw () {
  return (_high_flap_threshold);
}

/**
 *  Get hosts.
 *
 *  @return The hosts.
 */
list_string& service::hosts() throw () {
  return (*_hosts);
}

/**
 *  Get hosts.
 *
 *  @return The hosts.
 */
list_string const& service::hosts() const throw () {
  return (*_hosts);
}

/**
 *  Get initial_state.
 *
 *  @return The initial_state.
 */
unsigned int service::initial_state() const throw () {
  return (_initial_state);
}

/**
 *  Get is_volatile.
 *
 *  @return The is_volatile.
 */
bool service::is_volatile() const throw () {
  return (_is_volatile);
}

/**
 *  Get low_flap_threshold.
 *
 *  @return The low_flap_threshold.
 */
unsigned int service::low_flap_threshold() const throw () {
  return (_low_flap_threshold);
}

/**
 *  Get max_check_attempts.
 *
 *  @return The max_check_attempts.
 */
unsigned int service::max_check_attempts() const throw () {
  return (_max_check_attempts);
}

/**
 *  Get obsess_over_service.
 *
 *  @return The obsess_over_service.
 */
bool service::obsess_over_service() const throw () {
  return (_obsess_over_service);
}

/**
 *  Get retry_interval.
 *
 *  @return The retry_interval.
 */
duration const& service::retry_interval() const throw () {
  return (_retry_interval);
}

/**
 *  Get service_description.
 *
 *  @return The service_description.
 */
std::string& service::service_description() throw () {
  return (_service_description);
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
 *  Get service_id.
 *
 *  @return  The id the service.
 */
unsigned int service::service_id() const throw() {
  return (_service_id);
}

/**
 *  Set timezone.
 *
 *  @param[in] tz  New service timezone.
 */
void service::timezone(std::string const& tz) {
  _timezone = tz;
  return ;
}

/**
 *  Get the service timezone.
 *
 *  @return Service timezone.
 */
std::string const& service::timezone() const throw () {
  return (_timezone);
}

/**
 *  Check if timezone was defined.
 *
 *  @return True if property was defined, false otherwise.
 */
bool service::timezone_defined() const throw () {
  return (_timezone.is_set());
}

/**
 *  Set check_command value.
 *
 *  @param[in] value The new check_command value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_check_command(std::string const& value) {
  _check_command = value;
  return (true);
}

/**
 *  Set checks_active value.
 *
 *  @param[in] value The new checks_active value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_checks_active(bool value) {
  _checks_active = value;
  return (true);
}

/**
 *  Set check_freshness value.
 *
 *  @param[in] value The new check_freshness value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_check_freshness(bool value) {
  _check_freshness = value;
  return (true);
}

/**
 *  Set check_interval value.
 *
 *  @param[in] value  The new check_interval value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_check_interval(std::string const& value) {
  _check_interval = value;
  return (true);
}

/**
 *  Set check_period value.
 *
 *  @param[in] value The new check_period value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_check_period(std::string const& value) {
  _check_period = value;
  return (true);
}

/**
 *  Set check_timeout value.
 *
 *  @param[in] value  The new check_timeout value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_check_timeout(std::string const& value) {
  duration d(value);
  if (d < 0)
    throw (engine_error()
           << "service check timeout cannot be equal to or less than 0");
  _check_timeout = d;
  return (true);
}

/**
 *  Set event_handler value.
 *
 *  @param[in] value The new event_handler value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_event_handler(std::string const& value) {
  _event_handler = value;
  return (true);
}

/**
 *  Set event_handler_enabled value.
 *
 *  @param[in] value The new event_handler_enabled value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_event_handler_enabled(bool value) {
  _event_handler_enabled = value;
  return (true);
}

/**
 *  Set flap_detection_enabled value.
 *
 *  @param[in] value The new flap_detection_enabled value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_flap_detection_enabled(bool value) {
  _flap_detection_enabled = value;
  return (true);
}

/**
 *  Set flap_detection_options value.
 *
 *  @param[in] value The new flap_detection_options value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_flap_detection_options(std::string const& value) {
  unsigned short options(none);
  std::list<std::string> values;
  string::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    string::trim(*it);
    if (*it == "o" || *it == "ok")
      options |= ok;
    else if (*it == "w" || *it == "warning")
      options |= warning;
    else if (*it == "u" || *it == "unknown")
      options |= unknown;
    else if (*it == "c" || *it == "critical")
      options |= critical;
    else if (*it == "n" || *it == "none")
      options = none;
    else if (*it == "a" || *it == "all")
      options = ok | warning | unknown | critical;
    else
      return (false);
  }
  _flap_detection_options = options;
  return (true);
}

/**
 *  Set freshness_threshold value.
 *
 *  @param[in] value The new freshness_threshold value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_freshness_threshold(unsigned int value) {
  _freshness_threshold = value;
  return (true);
}

/**
 *  Set high_flap_threshold value.
 *
 *  @param[in] value The new high_flap_threshold value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_high_flap_threshold(unsigned int value) {
  _high_flap_threshold = value;
  return (true);
}

/**
 *  Set hosts value.
 *
 *  @param[in] value The new hosts value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_hosts(std::string const& value) {
  _hosts = value;
  return (true);
}

/**
 *  Set initial_state value.
 *
 *  @param[in] value The new initial_state value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_initial_state(std::string const& value) {
  std::string data(value);
  string::trim(data);
  if (data == "o" || data == "ok")
    _initial_state = STATE_OK;
  else if (data == "w" || data == "warning")
    _initial_state = STATE_WARNING;
  else if (data == "u" || data == "unknown")
    _initial_state = STATE_UNKNOWN;
  else if (data == "c" || data == "critical")
    _initial_state = STATE_CRITICAL;
  else
    return (false);
  return (true);
}

/**
 *  Set is_volatile value.
 *
 *  @param[in] value The new is_volatile value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_is_volatile(bool value) {
  _is_volatile = value;
  return (true);
}

/**
 *  Set low_flap_threshold value.
 *
 *  @param[in] value The new low_flap_threshold value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_low_flap_threshold(unsigned int value) {
  _low_flap_threshold = value;
  return (true);
}

/**
 *  Set max_check_attempts value.
 *
 *  @param[in] value The new max_check_attempts value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_max_check_attempts(unsigned int value) {
  if (value <= 0)
    return (false);
  _max_check_attempts = value;
  return (true);
}

/**
 *  Set obsess_over_service value.
 *
 *  @param[in] value The new obsess_over_service value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_obsess_over_service(bool value) {
  _obsess_over_service = value;
  return (true);
}

/**
 *  Set retry_interval value.
 *
 *  @param[in] value The new retry_interval value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_retry_interval(std::string const& value) {
  _retry_interval = value;
  return (true);
}

/**
 *  Set service_description value.
 *
 *  @param[in] value The new service_description value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_service_description(std::string const& value) {
  _service_description = value;
  return (true);
}

/**
 *  Set service_id value.
 *
 *  @param[in] value The new service_id value.
 *
 *  @return True on success, otherwise false.
 */
bool service::_set_service_id(unsigned int value) {
  _service_id = value;
  return (true);
}

/**
 *  Set timezone.
 *
 *  @param[in] value  Timezone.
 *
 *  @return True on success, false otherwise.
 */
bool service::_set_timezone(std::string const& value) {
  _timezone = value;
  return (true);
}

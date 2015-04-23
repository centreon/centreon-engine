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
#include "com/centreon/engine/configuration/host.hh"
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
  &object::setter<host, type, &host::method>::generic

host::setters const host::_setters[] = {
  { "host_name",                    SETTER(std::string const&, _set_host_name) },
  { "alias",                        SETTER(std::string const&, _set_alias) },
  { "address",                      SETTER(std::string const&, _set_address) },
  { "parents",                      SETTER(std::string const&, _set_parents) },
  { "check_timeout",                SETTER(std::string const&, _set_check_timeout) },
  { "check_command",                SETTER(std::string const&, _set_check_command) },
  { "check_period",                 SETTER(std::string const&, _set_check_period) },
  { "event_handler",                SETTER(std::string const&, _set_event_handler) },
  { "timezone",                     SETTER(std::string const&, _set_timezone) },
  { "initial_state",                SETTER(std::string const&, _set_initial_state) },
  { "check_interval",               SETTER(std::string const&, _set_check_interval) },
  { "normal_check_interval",        SETTER(std::string const&, _set_check_interval) },
  { "retry_interval",               SETTER(std::string const&, _set_retry_interval) },
  { "retry_check_interval",         SETTER(std::string const&, _set_retry_interval) },
  { "max_check_attempts",           SETTER(unsigned int, _set_max_check_attempts) },
  { "checks_enabled",               SETTER(bool, _set_checks_active) },
  { "active_checks_enabled",        SETTER(bool, _set_checks_active) },
  { "event_handler_enabled",        SETTER(bool, _set_event_handler_enabled) },
  { "check_freshness",              SETTER(bool, _set_check_freshness) },
  { "freshness_threshold",          SETTER(unsigned int, _set_freshness_threshold) },
  { "low_flap_threshold",           SETTER(unsigned int, _set_low_flap_threshold) },
  { "high_flap_threshold",          SETTER(unsigned int, _set_high_flap_threshold) },
  { "flap_detection_enabled",       SETTER(bool, _set_flap_detection_enabled) },
  { "flap_detection_options",       SETTER(std::string const&, _set_flap_detection_options) },
  { "obsess_over_host",             SETTER(bool, _set_obsess_over_host) }
};

char const* const host::_deprecated[][2] = {
  { "2d_coords",                    deprecated::default_msg },
  { "3d_coords",                    deprecated::default_msg },
  { "action_url",                   deprecated::default_msg },
  { "contact_groups",               deprecated::notification_msg },
  { "contacts",                     deprecated::notification_msg },
  { "display_name",                 deprecated::default_msg },
  { "failure_prediction_enabled",   deprecated::default_msg },
  { "failure_prediction_options",   deprecated::default_msg },
  { "first_notification_delay",     deprecated::notification_msg },
  { "gd2_image",                    deprecated::default_msg },
  { "host_groups",                  deprecated::groups_msg },
  { "hostgroups",                   deprecated::groups_msg },
  { "icon_image",                   deprecated::default_msg },
  { "icon_image_alt",               deprecated::default_msg },
  { "notes",                        deprecated::default_msg },
  { "notes_url",                    deprecated::default_msg },
  { "notifications_enabled",        deprecated::notification_msg },
  { "notification_interval",        deprecated::notification_msg },
  { "notification_options",         deprecated::notification_msg },
  { "notification_period",          deprecated::notification_msg },
  { "passive_checks_enabled",       deprecated::check_acceptance_msg },
  { "process_perf_data",            deprecated::perfdata_msg },
  { "retain_status_information",    deprecated::retention_usage_msg },
  { "retain_nonstatus_information", deprecated::retention_usage_msg },
  { "stalking_options",             deprecated::default_msg },
  { "statusmap_image",              deprecated::default_msg },
  { "vrml_image",                   deprecated::default_msg }
};

// Default values.
static bool const           default_checks_active(true);
static bool const           default_check_freshness(false);
static long const           default_check_interval(300);
static bool const           default_event_handler_enabled(true);
static bool const           default_flap_detection_enabled(true);
static unsigned short const default_flap_detection_options(host::up | host::down | host::unreachable);
static unsigned int const   default_freshness_threshold(0);
static unsigned int const   default_high_flap_threshold(0);
static unsigned short const default_initial_state(HOST_UP);
static unsigned int const   default_low_flap_threshold(0);
static unsigned int const   default_max_check_attempts(3);
static bool const           default_obsess_over_host(true);
static long const           default_retry_interval(60);
static long const           default_check_timeout(0);

/**
 *  Constructor.
 *
 *  @param[in] key The object key.
 */
host::host(key_type const& key)
  : object(object::host),
    _checks_active(default_checks_active),
    _check_freshness(default_check_freshness),
    _check_interval(default_check_interval),
    _check_timeout(default_check_timeout),
    _event_handler_enabled(default_event_handler_enabled),
    _flap_detection_enabled(default_flap_detection_enabled),
    _flap_detection_options(default_flap_detection_options),
    _freshness_threshold(default_freshness_threshold),
    _high_flap_threshold(default_high_flap_threshold),
    _host_name(key),
    _initial_state(default_initial_state),
    _low_flap_threshold(default_low_flap_threshold),
    _max_check_attempts(default_max_check_attempts),
    _obsess_over_host(default_obsess_over_host),
    _retry_interval(default_retry_interval) {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  The host to copy.
 */
host::host(host const& other) : object(other) {
  operator=(other);
}

/**
 *  Destructor.
 */
host::~host() throw () {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  The host to copy.
 *
 *  @return This host.
 */
host& host::operator=(host const& other) {
  if (this != &other) {
    object::operator=(other);
    _address = other._address;
    _alias = other._alias;
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
    _host_name = other._host_name;
    _initial_state = other._initial_state;
    _low_flap_threshold = other._low_flap_threshold;
    _max_check_attempts = other._max_check_attempts;
    _obsess_over_host = other._obsess_over_host;
    _parents = other._parents;
    _retry_interval = other._retry_interval;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] other  The host to compare.
 *
 *  @return True if is the same host, otherwise false.
 */
bool host::operator==(host const& other) const throw () {
  return (object::operator==(other)
          && _address == other._address
          && _alias == other._alias
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
          && _host_name == other._host_name
          && _initial_state == other._initial_state
          && _low_flap_threshold == other._low_flap_threshold
          && _max_check_attempts == other._max_check_attempts
          && _obsess_over_host == other._obsess_over_host
          && _parents == other._parents
          && _retry_interval == other._retry_interval
          && _timezone == other._timezone);
}

/**
 *  Equal operator.
 *
 *  @param[in] other  The host to compare.
 *
 *  @return True if is not the same host, otherwise false.
 */
bool host::operator!=(host const& other) const throw () {
  return (!operator==(other));
}

/**
 *  Less-than operator.
 *
 *  @param[in] other  Object to compare to.
 *
 *  @return True if this object is less than right.
 */
bool host::operator<(host const& other) const throw () {
  if (_host_name != other._host_name)
    return (_host_name < other._host_name);
  else if (_address != other._address)
    return (_address < other._address);
  else if (_alias != other._alias)
    return (_alias < other._alias);
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
  else if (_low_flap_threshold != other._low_flap_threshold)
    return (_low_flap_threshold < other._low_flap_threshold);
  else if (_max_check_attempts != other._max_check_attempts)
    return (_max_check_attempts < other._max_check_attempts);
  else if (_obsess_over_host != other._obsess_over_host)
    return (_obsess_over_host < other._obsess_over_host);
  else if (_parents != other._parents)
    return (_parents < other._parents);
  else if (_retry_interval != other._retry_interval)
    return (_retry_interval < other._retry_interval);
  return (_timezone < other._timezone);
}

/**
 *  @brief Check if the object is valid.
 *
 *  If the object is not valid, an exception is thrown.
 */
void host::check_validity() const {
  if (_host_name.empty())
    throw (engine_error() << "Host has no name (property 'host_name')");
  else if (_address.empty())
    throw (engine_error() << "Host '" << _host_name
           << "' has no address (property 'address')");
  else if (_check_command.empty()) {
    if (_checks_active)
      throw (engine_error() << "Host '" << _host_name
             << "' has active checks enabled but does not have its "
             << "check command defined");
    else if (_check_freshness)
      throw (engine_error() << "Host '" << _host_name
             << "' has frehness checks enabled but does not have its "
             << "check command defined");
  }
  return ;
}

/**
 *  Get host key.
 *
 *  @return Host name.
 */
host::key_type const& host::key() const throw () {
  return (_host_name);
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void host::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "Cannot merge host with '"
           << obj.type() << "'");
  host const& tmpl(static_cast<host const&>(obj));

  MRG_DEFAULT(_address);
  MRG_DEFAULT(_alias);
  MRG_OPTION(_checks_active);
  MRG_DEFAULT(_check_command);
  MRG_OPTION(_check_freshness);
  MRG_OPTION(_check_interval);
  MRG_DEFAULT(_check_period);
  MRG_OPTION(_check_timeout);
  MRG_MAP(_customvariables);
  MRG_DEFAULT(_event_handler);
  MRG_OPTION(_event_handler_enabled);
  MRG_OPTION(_flap_detection_enabled);
  MRG_OPTION(_flap_detection_options);
  MRG_OPTION(_freshness_threshold);
  MRG_OPTION(_high_flap_threshold);
  MRG_DEFAULT(_host_name);
  MRG_OPTION(_initial_state);
  MRG_OPTION(_low_flap_threshold);
  MRG_OPTION(_max_check_attempts);
  MRG_OPTION(_obsess_over_host);
  MRG_INHERIT(_parents);
  MRG_OPTION(_retry_interval);
  MRG_OPTION(_timezone);
}

/**
 *  Parse and set the host property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool host::parse(char const* key, char const* value) {
  for (unsigned int i(0);
       i < sizeof(_setters) / sizeof(*_setters);
       ++i)
    if (!strcmp(_setters[i].name, key))
      return ((_setters[i].func)(*this, value));
  if (key[0] == '_') {
    _customvariables[key + 1] = value;
    return (true);
  }
  for (unsigned int i(0);
       i < sizeof(_deprecated) / sizeof(*_deprecated);
       ++i)
    if (!strcmp(_deprecated[i][0], key)) {
      logger(log_config_warning, basic)
        << "Warning: host variable '" << key
        << "' is no longer supported.\n"
        << _deprecated[i][1];
      ++config_warnings;
      return (true);
    }
  return (false);
}

/**
 *  Get address.
 *
 *  @return The address.
 */
std::string const& host::address() const throw () {
  return (_address);
}

/**
 *  Get alias.
 *
 *  @return The alias.
 */
std::string const& host::alias() const throw () {
  return (_alias);
}

/**
 *  Get checks_active.
 *
 *  @return The checks_active.
 */
bool host::checks_active() const throw () {
  return (_checks_active);
}

/**
 *  Get check_command.
 *
 *  @return The check_command.
 */
std::string const& host::check_command() const throw () {
  return (_check_command);
}

/**
 *  Get check_freshness.
 *
 *  @return The check_freshness.
 */
bool host::check_freshness() const throw () {
  return (_check_freshness);
}

/**
 *  Get check_interval.
 *
 *  @return The check_interval.
 */
duration const& host::check_interval() const throw () {
  return (_check_interval);
}

/**
 *  Get check_period.
 *
 *  @return The check_period.
 */
std::string const& host::check_period() const throw () {
  return (_check_period);
}

/**
 *  Get check_timeout.
 *
 *  @return The check_timeout.
 */
duration const& host::check_timeout() const throw() {
  return (_check_timeout);
}

/**
 *  Get customvariables.
 *
 *  @return The customvariables.
 */
map_customvar const& host::customvariables() const throw () {
  return (_customvariables);
}

/**
 *  Get event_handler.
 *
 *  @return The event_handler.
 */
std::string const& host::event_handler() const throw () {
  return (_event_handler);
}

/**
 *  Get event_handler_enabled.
 *
 *  @return The event_handler_enabled.
 */
bool host::event_handler_enabled() const throw () {
  return (_event_handler_enabled);
}

/**
 *  Get flap_detection_enabled.
 *
 *  @return The flap_detection_enabled.
 */
bool host::flap_detection_enabled() const throw () {
  return (_flap_detection_enabled);
}

/**
 *  Get flap_detection_options.
 *
 *  @return The flap_detection_options.
 */
unsigned int host::flap_detection_options() const throw () {
  return (_flap_detection_options);
}

/**
 *  Get freshness_threshold.
 *
 *  @return The freshness_threshold.
 */
unsigned int host::freshness_threshold() const throw () {
  return (_freshness_threshold);
}

/**
 *  Get high_flap_threshold.
 *
 *  @return The high_flap_threshold.
 */
unsigned int host::high_flap_threshold() const throw () {
  return (_high_flap_threshold);
}

/**
 *  Get host_name.
 *
 *  @return The host_name.
 */
std::string const& host::host_name() const throw () {
  return (_host_name);
}

/**
 *  Get initial_state.
 *
 *  @return The initial_state.
 */
unsigned int host::initial_state() const throw () {
  return (_initial_state);
}

/**
 *  Get low_flap_threshold.
 *
 *  @return The low_flap_threshold.
 */
unsigned int host::low_flap_threshold() const throw () {
  return (_low_flap_threshold);
}

/**
 *  Get max_check_attempts.
 *
 *  @return The max_check_attempts.
 */
unsigned int host::max_check_attempts() const throw () {
  return (_max_check_attempts);
}

/**
 *  Get obsess_over_host.
 *
 *  @return The obsess_over_host.
 */
bool host::obsess_over_host() const throw () {
  return (_obsess_over_host);
}

/**
 *  Get parents.
 *
 *  @return The parents.
 */
list_string& host::parents() throw () {
  return (*_parents);
}

/**
 *  Get parents.
 *
 *  @return The parents.
 */
list_string const& host::parents() const throw () {
  return (*_parents);
}

/**
 *  Get retry_interval.
 *
 *  @return The retry_interval.
 */
duration const& host::retry_interval() const throw () {
  return (_retry_interval);
}

/**
 *  Get host timezone.
 *
 *  @return Host timezone.
 */
std::string const& host::timezone() const throw () {
  return (_timezone);
}

/**
 *  Set address value.
 *
 *  @param[in] value The new address value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_address(std::string const& value) {
  _address = value;
  return (true);
}

/**
 *  Set alias value.
 *
 *  @param[in] value The new alias value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_alias(std::string const& value) {
  _alias = value;
  return (true);
}

/**
 *  Set checks_active value.
 *
 *  @param[in] value The new checks_active value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_checks_active(bool value) {
  _checks_active = value;
  return (true);
}

/**
 *  Set check_command value.
 *
 *  @param[in] value The new check_command value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_check_command(std::string const& value) {
  _check_command = value;
  return (true);
}

/**
 *  Set check_freshness value.
 *
 *  @param[in] value The new check_freshness value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_check_freshness(bool value) {
  _check_freshness = value;
  return (true);
}

/**
 *  Set check_interval value.
 *
 *  @param[in] value The new check_interval value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_check_interval(std::string const& value) {
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
bool host::_set_check_period(std::string const& value) {
  _check_period = value;
  return (true);
}

/**
 *  Set check_timeout value.
 *
 *  @param[in] value The new check_timeout value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_check_timeout(std::string const& value) {
  duration d(value);
  if (d < 0)
    throw (engine_error()
           << "host check timeout cannot be equal to or less than 0");
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
bool host::_set_event_handler(std::string const& value) {
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
bool host::_set_event_handler_enabled(bool value) {
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
bool host::_set_flap_detection_enabled(bool value) {
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
bool host::_set_flap_detection_options(std::string const& value) {
  unsigned short options(none);
  std::list<std::string> values;
  string::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    string::trim(*it);
    if (*it == "o" || *it == "up")
      options |= up;
    else if (*it == "d" || *it == "down")
      options |= down;
    else if (*it == "u" || *it == "unreachable")
      options |= unreachable;
    else if (*it == "n" || *it == "none")
      options = none;
    else if (*it == "a" || *it == "all")
      options = up | down | unreachable;
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
bool host::_set_freshness_threshold(unsigned int value) {
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
bool host::_set_high_flap_threshold(unsigned int value) {
  _high_flap_threshold = value;
  return (true);
}

/**
 *  Set host_name value.
 *
 *  @param[in] value The new host_name value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_host_name(std::string const& value) {
  _host_name = value;
  return (true);
}

/**
 *  Set initial_state value.
 *
 *  @param[in] value The new initial_state value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_initial_state(std::string const& value) {
  std::string data(value);
  string::trim(data);
  if (data == "o" || data == "up")
    _initial_state = HOST_UP;
  else if (data == "d" || data == "down")
    _initial_state = HOST_DOWN;
  else if (data == "u" || data == "unreachable")
    _initial_state = HOST_UNREACHABLE;
  else
    return (false);
  return (true);
}

/**
 *  Set low_flap_threshold value.
 *
 *  @param[in] value The new low_flap_threshold value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_low_flap_threshold(unsigned int value) {
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
bool host::_set_max_check_attempts(unsigned int value) {
  if (!value)
    return (false);
  _max_check_attempts = value;
  return (true);
}

/**
 *  Set obsess_over_host value.
 *
 *  @param[in] value The new obsess_over_host value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_obsess_over_host(bool value) {
  _obsess_over_host = value;
  return (true);
}

/**
 *  Set parents value.
 *
 *  @param[in] value The new parents value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_parents(std::string const& value) {
  _parents = value;
  return (true);
}

/**
 *  Set retry_interval value.
 *
 *  @param[in] value The new retry_interval value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_retry_interval(std::string const& value) {
  _retry_interval = value;
  return (true);
}

/**
 *  Set timezone.
 *
 *  @param[in] timezone  The new host timezone.
 *
 *  @return True on success, false otherwise.
 */
bool host::_set_timezone(std::string const& value) {
  _timezone = value;
  return (true);
}

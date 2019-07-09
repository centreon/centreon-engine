/*
** Copyright 2011-2013,2015-2017,2019 Centreon
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

#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/hostextinfo.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/string.hh"

extern int config_warnings;
extern int config_errors;

using namespace com::centreon;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::logging;

#define SETTER(type, method) \
  &object::setter<host, type, &host::method>::generic

std::unordered_map<std::string, host::setter_func> const host::_setters{
  { "host_name",                    SETTER(std::string const&, _set_host_name) },
  { "host_id",                      SETTER(uint64_t, _set_host_id)},
  { "_HOST_ID",                     SETTER(uint64_t, _set_host_id)},
  { "display_name",                 SETTER(std::string const&, _set_display_name) },
  { "alias",                        SETTER(std::string const&, _set_alias) },
  { "acknowledgement_timeout",      SETTER(int, set_acknowledgement_timeout) },
  { "address",                      SETTER(std::string const&, _set_address) },
  { "parents",                      SETTER(std::string const&, _set_parents) },
  { "host_groups",                  SETTER(std::string const&, _set_hostgroups) },
  { "hostgroups",                   SETTER(std::string const&, _set_hostgroups) },
  { "contact_groups",               SETTER(std::string const&, _set_contactgroups) },
  { "contacts",                     SETTER(std::string const&, _set_contacts) },
  { "notification_period",          SETTER(std::string const&, _set_notification_period) },
  { "check_command",                SETTER(std::string const&, _set_check_command) },
  { "check_period",                 SETTER(std::string const&, _set_check_period) },
  { "event_handler",                SETTER(std::string const&, _set_event_handler) },
  { "failure_prediction_options",   SETTER(std::string const&, _set_failure_prediction_options) },
  { "notes",                        SETTER(std::string const&, _set_notes) },
  { "notes_url",                    SETTER(std::string const&, _set_notes_url) },
  { "action_url",                   SETTER(std::string const&, _set_action_url) },
  { "icon_image",                   SETTER(std::string const&, _set_icon_image) },
  { "icon_image_alt",               SETTER(std::string const&, _set_icon_image_alt) },
  { "vrml_image",                   SETTER(std::string const&, _set_vrml_image) },
  { "gd2_image",                    SETTER(std::string const&, _set_statusmap_image) },
  { "statusmap_image",              SETTER(std::string const&, _set_statusmap_image) },
  { "initial_state",                SETTER(std::string const&, _set_initial_state) },
  { "check_interval",               SETTER(unsigned int, _set_check_interval) },
  { "normal_check_interval",        SETTER(unsigned int, _set_check_interval) },
  { "retry_interval",               SETTER(unsigned int, _set_retry_interval) },
  { "retry_check_interval",         SETTER(unsigned int, _set_retry_interval) },
  { "recovery_notification_delay",  SETTER(unsigned int, _set_recovery_notification_delay) },
  { "max_check_attempts",           SETTER(unsigned int, _set_max_check_attempts) },
  { "checks_enabled",               SETTER(bool, _set_checks_active) },
  { "active_checks_enabled",        SETTER(bool, _set_checks_active) },
  { "passive_checks_enabled",       SETTER(bool, _set_checks_passive) },
  { "event_handler_enabled",        SETTER(bool, _set_event_handler_enabled) },
  { "check_freshness",              SETTER(bool, _set_check_freshness) },
  { "freshness_threshold",          SETTER(unsigned int, _set_freshness_threshold) },
  { "low_flap_threshold",           SETTER(unsigned int, _set_low_flap_threshold) },
  { "high_flap_threshold",          SETTER(unsigned int, _set_high_flap_threshold) },
  { "flap_detection_enabled",       SETTER(bool, _set_flap_detection_enabled) },
  { "flap_detection_options",       SETTER(std::string const&, _set_flap_detection_options) },
  { "notification_options",         SETTER(std::string const&, _set_notification_options) },
  { "notifications_enabled",        SETTER(bool, _set_notifications_enabled) },
  { "notification_interval",        SETTER(unsigned int, _set_notification_interval) },
  { "first_notification_delay",     SETTER(unsigned int, _set_first_notification_delay) },
  { "stalking_options",             SETTER(std::string const&, _set_stalking_options) },
  { "process_perf_data",            SETTER(bool, _set_process_perf_data) },
  { "failure_prediction_enabled",   SETTER(bool, _set_failure_prediction_enabled) },
  { "2d_coords",                    SETTER(std::string const&, _set_coords_2d) },
  { "3d_coords",                    SETTER(std::string const&, _set_coords_3d) },
  { "obsess_over_host",             SETTER(bool, _set_obsess_over_host) },
  { "retain_status_information",    SETTER(bool, _set_retain_status_information) },
  { "retain_nonstatus_information", SETTER(bool, _set_retain_nonstatus_information) },
  { "timezone",                     SETTER(std::string const&, _set_timezone) }
};

// Default values.
static bool const           default_checks_active(true);
static bool const           default_checks_passive(true);
static bool const           default_check_freshness(false);
static unsigned int const   default_check_interval(5);
static point_2d const       default_coords_2d;
static point_3d const       default_coords_3d;
static bool const           default_event_handler_enabled(true);
static unsigned int const   default_first_notification_delay(0);
static bool const           default_flap_detection_enabled(true);
static unsigned short const default_flap_detection_options(host::up | host::down | host::unreachable);
static unsigned int const   default_freshness_threshold(0);
static unsigned int const   default_high_flap_threshold(0);
static unsigned short const default_initial_state(engine::host::state_up);
static unsigned int const   default_low_flap_threshold(0);
static unsigned int const   default_max_check_attempts(3);
static bool const           default_notifications_enabled(true);
static unsigned int const   default_notification_interval(30);
static unsigned short const default_notification_options(host::up | host::down | host::unreachable | host::flapping | host::downtime);
static bool const           default_obsess_over_host(true);
static bool const           default_process_perf_data(true);
static bool const           default_retain_nonstatus_information(true);
static bool const           default_retain_status_information(true);
static unsigned int const   default_retry_interval(1);
static unsigned short const default_stalking_options(host::none);

/**
 *  Constructor.
 *
 *  @param[in] key The object key.
 */
host::host(host::key_type const& key)
  : object(object::host),
    _acknowledgement_timeout(0),
    _checks_active(default_checks_active),
    _checks_passive(default_checks_passive),
    _check_freshness(default_check_freshness),
    _check_interval(default_check_interval),
    _event_handler_enabled(default_event_handler_enabled),
    _first_notification_delay(default_first_notification_delay),
    _flap_detection_enabled(default_flap_detection_enabled),
    _flap_detection_options(default_flap_detection_options),
    _freshness_threshold(default_freshness_threshold),
    _high_flap_threshold(default_high_flap_threshold),
    _host_id(key),
    _host_name(""),
    _initial_state(default_initial_state),
    _low_flap_threshold(default_low_flap_threshold),
    _max_check_attempts(default_max_check_attempts),
    _notifications_enabled(default_notifications_enabled),
    _notification_interval(default_notification_interval),
    _notification_options(default_notification_options),
    _obsess_over_host(default_obsess_over_host),
    _process_perf_data(default_process_perf_data),
    _retain_nonstatus_information(default_retain_nonstatus_information),
    _retain_status_information(default_retain_status_information),
    _retry_interval(default_retry_interval),
    _recovery_notification_delay(0),
    _stalking_options(default_stalking_options) {}

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
 *  Assignment operator.
 *
 *  @param[in] other  The host to copy.
 *
 *  @return This host.
 */
host& host::operator=(host const& other) {
  if (this != &other) {
    object::operator=(other);
    _acknowledgement_timeout = other._acknowledgement_timeout;
    _action_url = other._action_url;
    _address = other._address;
    _alias = other._alias;
    _checks_active = other._checks_active;
    _checks_passive = other._checks_passive;
    _check_command = other._check_command;
    _check_freshness = other._check_freshness;
    _check_interval = other._check_interval;
    _check_period = other._check_period;
    _contactgroups = other._contactgroups;
    _contacts = other._contacts;
    _coords_2d = other._coords_2d;
    _coords_3d = other._coords_3d;
    _customvariables = other._customvariables;
    _display_name = other._display_name;
    _event_handler = other._event_handler;
    _event_handler_enabled = other._event_handler_enabled;
    _first_notification_delay = other._first_notification_delay;
    _flap_detection_enabled = other._flap_detection_enabled;
    _flap_detection_options = other._flap_detection_options;
    _freshness_threshold = other._freshness_threshold;
    _high_flap_threshold = other._high_flap_threshold;
    _hostgroups = other._hostgroups;
    _host_id = other._host_id;
    _host_name = other._host_name;
    _icon_image = other._icon_image;
    _icon_image_alt = other._icon_image_alt;
    _initial_state = other._initial_state;
    _low_flap_threshold = other._low_flap_threshold;
    _max_check_attempts = other._max_check_attempts;
    _notes = other._notes;
    _notes_url = other._notes_url;
    _notifications_enabled = other._notifications_enabled;
    _notification_interval = other._notification_interval;
    _notification_options = other._notification_options;
    _notification_period = other._notification_period;
    _obsess_over_host = other._obsess_over_host;
    _parents = other._parents;
    _process_perf_data = other._process_perf_data;
    _retain_nonstatus_information = other._retain_nonstatus_information;
    _retain_status_information = other._retain_status_information;
    _retry_interval = other._retry_interval;
    _recovery_notification_delay = other._recovery_notification_delay;
    _stalking_options = other._stalking_options;
    _statusmap_image = other._statusmap_image;
    _timezone = other._timezone;
    _vrml_image = other._vrml_image;
  }
  return *this;
}

/**
 *  Equality operator.
 *
 *  @param[in] other  The host to compare.
 *
 *  @return True if this object and other object are equal.
 */
bool host::operator==(host const& other) const throw () {
  return (object::operator==(other)
          && _acknowledgement_timeout == other._acknowledgement_timeout
          && _action_url == other._action_url
          && _address == other._address
          && _alias == other._alias
          && _checks_active == other._checks_active
          && _checks_passive == other._checks_passive
          && _check_command == other._check_command
          && _check_freshness == other._check_freshness
          && _check_interval == other._check_interval
          && _check_period == other._check_period
          && _contactgroups == other._contactgroups
          && _contacts == other._contacts
          && _coords_2d == other._coords_2d
          && _coords_3d == other._coords_3d
          && std::operator==(_customvariables, other._customvariables)
          && _display_name == other._display_name
          && _event_handler == other._event_handler
          && _event_handler_enabled == other._event_handler_enabled
          && _first_notification_delay == other._first_notification_delay
          && _flap_detection_enabled == other._flap_detection_enabled
          && _flap_detection_options == other._flap_detection_options
          && _freshness_threshold == other._freshness_threshold
          && _high_flap_threshold == other._high_flap_threshold
          && _hostgroups == other._hostgroups
          && _host_id == other._host_id
          && _host_name == other._host_name
          && _icon_image == other._icon_image
          && _icon_image_alt == other._icon_image_alt
          && _initial_state == other._initial_state
          && _low_flap_threshold == other._low_flap_threshold
          && _max_check_attempts == other._max_check_attempts
          && _notes == other._notes
          && _notes_url == other._notes_url
          && _notifications_enabled == other._notifications_enabled
          && _notification_interval == other._notification_interval
          && _notification_options == other._notification_options
          && _notification_period == other._notification_period
          && _obsess_over_host == other._obsess_over_host
          && _parents == other._parents
          && _process_perf_data == other._process_perf_data
          && _retain_nonstatus_information == other._retain_nonstatus_information
          && _retain_status_information == other._retain_status_information
          && _retry_interval == other._retry_interval
          && _recovery_notification_delay == other._recovery_notification_delay
          && _stalking_options == other._stalking_options
          && _statusmap_image == other._statusmap_image
          && _timezone == other._timezone
          && _vrml_image == other._vrml_image);
}

/**
 *  Inequality operator.
 *
 *  @param[in] other  The host to compare.
 *
 *  @return True if is not the same host, otherwise false.
 */
bool host::operator!=(host const& other) const throw () {
  return !operator==(other);
}

/**
 *  Less-than operator.
 *
 *  @param[in] other  Object to compare to.
 *
 *  @return True if this object is less than right.
 */
bool host::operator<(host const& other) const throw () {
  // host_id has to be first in this operator.
  // The configuration diff mechanism relies on this.
  if (_host_id != other._host_id)
    return _host_id < other._host_id;
  else if (_host_name != other._host_name)
    return _host_name < other._host_name;
  else if (_acknowledgement_timeout != other._acknowledgement_timeout)
    return _acknowledgement_timeout < other._acknowledgement_timeout;
  else if (_action_url != other._action_url)
    return _action_url < other._action_url;
  else if (_address != other._address)
    return _address < other._address;
  else if (_alias != other._alias)
    return _alias < other._alias;
  else if (_checks_active != other._checks_active)
    return _checks_active < other._checks_active;
  else if (_checks_passive != other._checks_passive)
    return _checks_passive < other._checks_passive;
  else if (_check_command != other._check_command)
    return _check_command < other._check_command;
  else if (_check_freshness != other._check_freshness)
    return _check_freshness < other._check_freshness;
  else if (_check_interval != other._check_interval)
    return _check_interval < other._check_interval;
  else if (_check_period != other._check_period)
    return _check_period < other._check_period;
  else if (_contactgroups != other._contactgroups)
    return _contactgroups < other._contactgroups;
  else if (_contacts != other._contacts)
    return _contacts < other._contacts;
  else if (_coords_2d != other._coords_2d)
    return _coords_2d < other._coords_2d;
  else if (_coords_3d != other._coords_3d)
    return _coords_3d < other._coords_3d;
  else if (_customvariables != other._customvariables)
    return _customvariables.size() < other._customvariables.size();
  else if (_display_name != other._display_name)
    return _display_name < other._display_name;
  else if (_event_handler != other._event_handler)
    return _event_handler < other._event_handler;
  else if (_event_handler_enabled != other._event_handler_enabled)
    return _event_handler_enabled < other._event_handler_enabled;
  else if (_first_notification_delay
           != other._first_notification_delay)
    return (_first_notification_delay
            < other._first_notification_delay);
  else if (_flap_detection_enabled != other._flap_detection_enabled)
    return _flap_detection_enabled < other._flap_detection_enabled;
  else if (_flap_detection_options != other._flap_detection_options)
    return _flap_detection_options < other._flap_detection_options;
  else if (_freshness_threshold != other._freshness_threshold)
    return _freshness_threshold < other._freshness_threshold;
  else if (_high_flap_threshold != other._high_flap_threshold)
    return _high_flap_threshold < other._high_flap_threshold;
  else if (_hostgroups != other._hostgroups)
    return _hostgroups < other._hostgroups;
  else if (_icon_image != other._icon_image)
    return _icon_image < other._icon_image;
  else if (_icon_image_alt != other._icon_image_alt)
    return _icon_image_alt < other._icon_image_alt;
  else if (_initial_state != other._initial_state)
    return _initial_state < other._initial_state;
  else if (_low_flap_threshold != other._low_flap_threshold)
    return _low_flap_threshold < other._low_flap_threshold;
  else if (_max_check_attempts != other._max_check_attempts)
    return _max_check_attempts < other._max_check_attempts;
  else if (_notes != other._notes)
    return _notes < other._notes;
  else if (_notes_url != other._notes_url)
    return _notes_url < other._notes_url;
  else if (_notifications_enabled != other._notifications_enabled)
    return _notifications_enabled < other._notifications_enabled;
  else if (_notification_interval != other._notification_interval)
    return _notification_interval < other._notification_interval;
  else if (_notification_options != other._notification_options)
    return _notification_options < other._notification_options;
  else if (_notification_period != other._notification_period)
    return _notification_period < other._notification_period;
  else if (_obsess_over_host != other._obsess_over_host)
    return _obsess_over_host < other._obsess_over_host;
  else if (_parents != other._parents)
    return _parents < other._parents;
  else if (_process_perf_data != other._process_perf_data)
    return _process_perf_data < other._process_perf_data;
  else if (_retain_nonstatus_information
           != other._retain_nonstatus_information)
    return (_retain_nonstatus_information
            < other._retain_nonstatus_information);
  else if (_retain_status_information
           != other._retain_status_information)
    return (_retain_status_information
            < other._retain_status_information);
  else if (_retry_interval != other._retry_interval)
    return _retry_interval < other._retry_interval;
  else if (_recovery_notification_delay
           != other._recovery_notification_delay)
    return (_recovery_notification_delay
            < other._recovery_notification_delay);
  else if (_stalking_options != other._stalking_options)
    return _stalking_options < other._stalking_options;
  else if (_statusmap_image != other._statusmap_image)
    return _statusmap_image < other._statusmap_image;
  else if (_timezone != other._timezone)
    return _timezone < other._timezone;
  return _vrml_image < other._vrml_image;
}

/**
 *  @brief Check if the object is valid.
 *
 *  If the object is not valid, an exception is thrown.
 */
void host::check_validity() const {
  if (_host_name.empty())
    throw (engine_error() << "Host has no name (property 'host_name')");
  if (_address.empty())
    throw (engine_error() << "Host '" << _host_name
           << "' has no address (property 'address')");
  return ;
}

/**
 *  Get host key.
 *
 *  @return Host name.
 */
host::key_type host::key() const throw () {
  return _host_id;
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void host::merge(configuration::hostextinfo const& tmpl) {
  MRG_DEFAULT(_action_url);
  MRG_OPTION(_coords_2d);
  MRG_OPTION(_coords_3d);
  MRG_DEFAULT(_icon_image);
  MRG_DEFAULT(_icon_image_alt);
  MRG_DEFAULT(_notes);
  MRG_DEFAULT(_notes_url);
  MRG_DEFAULT(_statusmap_image);
  MRG_DEFAULT(_vrml_image);
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

  MRG_OPTION(_acknowledgement_timeout);
  MRG_DEFAULT(_action_url);
  MRG_DEFAULT(_address);
  MRG_DEFAULT(_alias);
  MRG_OPTION(_checks_active);
  MRG_OPTION(_checks_passive);
  MRG_DEFAULT(_check_command);
  MRG_OPTION(_check_freshness);
  MRG_OPTION(_check_interval);
  MRG_DEFAULT(_check_period);
  MRG_INHERIT(_contactgroups);
  MRG_INHERIT(_contacts);
  MRG_OPTION(_coords_2d);
  MRG_OPTION(_coords_3d);
  MRG_MAP(_customvariables);
  MRG_DEFAULT(_display_name);
  MRG_DEFAULT(_event_handler);
  MRG_OPTION(_event_handler_enabled);
  MRG_OPTION(_first_notification_delay);
  MRG_OPTION(_flap_detection_enabled);
  MRG_OPTION(_flap_detection_options);
  MRG_OPTION(_freshness_threshold);
  MRG_OPTION(_high_flap_threshold);
  MRG_INHERIT(_hostgroups);
  MRG_DEFAULT(_host_name);
  MRG_DEFAULT(_icon_image);
  MRG_DEFAULT(_icon_image_alt);
  MRG_OPTION(_initial_state);
  MRG_OPTION(_low_flap_threshold);
  MRG_OPTION(_max_check_attempts);
  MRG_DEFAULT(_notes);
  MRG_DEFAULT(_notes_url);
  MRG_OPTION(_notifications_enabled);
  MRG_OPTION(_notification_interval);
  MRG_OPTION(_notification_options);
  MRG_DEFAULT(_notification_period);
  MRG_OPTION(_obsess_over_host);
  MRG_INHERIT(_parents);
  MRG_OPTION(_process_perf_data);
  MRG_OPTION(_recovery_notification_delay);
  MRG_OPTION(_retain_nonstatus_information);
  MRG_OPTION(_retain_status_information);
  MRG_OPTION(_retry_interval);
  MRG_OPTION(_stalking_options);
  MRG_DEFAULT(_statusmap_image);
  MRG_OPTION(_timezone);
  MRG_DEFAULT(_vrml_image);
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
  std::unordered_map<std::string, host::setter_func>::const_iterator
    it{_setters.find(key)};
  if (it != _setters.end())
    return (it->second)(*this, value);
  if (key[0] == '_') {
    map_customvar::iterator it(_customvariables.find(key + 1));
    if (it == _customvariables.end())
      _customvariables.insert({key + 1, std::shared_ptr<customvariable>{new customvariable(value)}});
    else
      it->second->set_value(value);

    return true;
  }
  return false;
}

/**
 *  Get action_url.
 *
 *  @return The action_url.
 */
std::string const& host::action_url() const throw () {
  return _action_url;
}

/**
 *  Get address.
 *
 *  @return The address.
 */
std::string const& host::address() const throw () {
  return _address;
}

/**
 *  Get alias.
 *
 *  @return The alias.
 */
std::string const& host::alias() const throw () {
  return _alias;
}

/**
 *  Get checks_active.
 *
 *  @return The checks_active.
 */
bool host::checks_active() const throw () {
  return _checks_active;
}

/**
 *  Get checks_passive.
 *
 *  @return The checks_passive.
 */
bool host::checks_passive() const throw () {
  return _checks_passive;
}

/**
 *  Get check_command.
 *
 *  @return The check_command.
 */
std::string const& host::check_command() const throw () {
  return _check_command;
}

/**
 *  Get check_freshness.
 *
 *  @return The check_freshness.
 */
bool host::check_freshness() const throw () {
  return _check_freshness;
}

/**
 *  Get check_interval.
 *
 *  @return The check_interval.
 */
unsigned int host::check_interval() const throw () {
  return _check_interval;
}

/**
 *  Get check_period.
 *
 *  @return The check_period.
 */
std::string const& host::check_period() const throw () {
  return _check_period;
}

/**
 *  Get contactgroups.
 *
 *  @return The contactgroups.
 */
set_string const& host::contactgroups() const throw () {
  return *_contactgroups;
}

/**
 *  Get contacts.
 *
 *  @return The contacts.
 */
set_string const& host::contacts() const throw () {
  return *_contacts;
}

/**
 *  Get coords_2d.
 *
 *  @return The coords_2d.
 */
point_2d const& host::coords_2d() const throw () {
  return _coords_2d.get();
}

/**
 *  Get coords_3d.
 *
 *  @return The coords_3d.
 */
point_3d const& host::coords_3d() const throw () {
  return _coords_3d.get();
}

/**
 *  Get customvariables.
 *
 *  @return The customvariables.
 */
engine::map_customvar const& host::customvariables() const throw () {
  return _customvariables;
}

/**
 *  Get display_name.
 *
 *  @return The display_name.
 */
std::string const& host::display_name() const throw () {
  return _display_name;
}

/**
 *  Get event_handler.
 *
 *  @return The event_handler.
 */
std::string const& host::event_handler() const throw () {
  return _event_handler;
}

/**
 *  Get event_handler_enabled.
 *
 *  @return The event_handler_enabled.
 */
bool host::event_handler_enabled() const throw () {
  return _event_handler_enabled;
}

/**
 *  Get first_notification_delay.
 *
 *  @return The first_notification_delay.
 */
unsigned int host::first_notification_delay() const throw () {
  return _first_notification_delay;
}

/**
 *  Get flap_detection_enabled.
 *
 *  @return The flap_detection_enabled.
 */
bool host::flap_detection_enabled() const throw () {
  return _flap_detection_enabled;
}

/**
 *  Get flap_detection_options.
 *
 *  @return The flap_detection_options.
 */
unsigned int host::flap_detection_options() const throw () {
  return _flap_detection_options;
}

/**
 *  Get freshness_threshold.
 *
 *  @return The freshness_threshold.
 */
unsigned int host::freshness_threshold() const throw () {
  return _freshness_threshold;
}

/**
 *  Get if host has coords 2d.
 *
 *  @return True if coords 2d exist, otherwise false.
 */
bool host::have_coords_2d() const throw () {
  return _coords_2d.is_set();
}

/**
 *  Get if host has coords 3d.
 *
 *  @return True if coords 3d exist, otherwise false.
 */
bool host::have_coords_3d() const throw () {
  return _coords_3d.is_set();
}

/**
 *  Get high_flap_threshold.
 *
 *  @return The high_flap_threshold.
 */
unsigned int host::high_flap_threshold() const throw () {
  return _high_flap_threshold;
}

/**
 *  Get host groups.
 *
 *  @return The host groups.
 */
set_string& host::hostgroups() throw () {
  return *_hostgroups;
}

/**
 *  Get hostgroups.
 *
 *  @return The hostgroups.
 */
set_string const& host::hostgroups() const throw () {
  return *_hostgroups;
}

/**
 *  Get host id.
 *
 *  @return  The host id.
 */
uint64_t host::host_id() const throw() {
  return _host_id;
}

/**
 *  Get host_name.
 *
 *  @return The host_name.
 */
std::string const& host::host_name() const throw () {
  return _host_name;
}

/**
 *  Get icon_image.
 *
 *  @return The icon_image.
 */
std::string const& host::icon_image() const throw () {
  return _icon_image;
}

/**
 *  Get icon_image_alt.
 *
 *  @return The icon_image_alt.
 */
std::string const& host::icon_image_alt() const throw () {
  return _icon_image_alt;
}

/**
 *  Get initial_state.
 *
 *  @return The initial_state.
 */
unsigned int host::initial_state() const throw () {
  return _initial_state;
}

/**
 *  Get low_flap_threshold.
 *
 *  @return The low_flap_threshold.
 */
unsigned int host::low_flap_threshold() const throw () {
  return _low_flap_threshold;
}

/**
 *  Get max_check_attempts.
 *
 *  @return The max_check_attempts.
 */
unsigned int host::max_check_attempts() const throw () {
  return _max_check_attempts;
}

/**
 *  Get notes.
 *
 *  @return The notes.
 */
std::string const& host::notes() const throw () {
  return _notes;
}

/**
 *  Get notes_url.
 *
 *  @return The notes_url.
 */
std::string const& host::notes_url() const throw () {
  return _notes_url;
}

/**
 *  Get notifications_enabled.
 *
 *  @return The notifications_enabled.
 */
bool host::notifications_enabled() const throw () {
  return _notifications_enabled;
}

/**
 *  Get notification_interval.
 *
 *  @return The notification_interval.
 */
unsigned int host::notification_interval() const throw () {
  return _notification_interval;
}

/**
 *  Get notification_options.
 *
 *  @return The notification_options.
 */
unsigned int host::notification_options() const throw () {
  return _notification_options;
}

/**
 *  Get notification_period.
 *
 *  @return The notification_period.
 */
std::string const& host::notification_period() const throw () {
  return _notification_period;
}

/**
 *  Get obsess_over_host.
 *
 *  @return The obsess_over_host.
 */
bool host::obsess_over_host() const throw () {
  return _obsess_over_host;
}

/**
 *  Get parents.
 *
 *  @return The parents.
 */
set_string& host::parents() throw () {
  return *_parents;
}

/**
 *  Get parents.
 *
 *  @return The parents.
 */
set_string const& host::parents() const throw () {
  return *_parents;
}

/**
 *  Get process_perf_data.
 *
 *  @return The process_perf_data.
 */
bool host::process_perf_data() const throw () {
  return _process_perf_data;
}

/**
 *  Get retain_nonstatus_information.
 *
 *  @return The retain_nonstatus_information.
 */
bool host::retain_nonstatus_information() const throw () {
  return _retain_nonstatus_information;
}

/**
 *  Get retain_status_information.
 *
 *  @return The retain_status_information.
 */
bool host::retain_status_information() const throw () {
  return _retain_status_information;
}

/**
 *  Get retry_interval.
 *
 *  @return The retry_interval.
 */
unsigned int host::retry_interval() const throw () {
  return _retry_interval;
}

/**
 *  Get recovery_notification_delay.
 *
 *  @return The recovery_notification_delay.
 */
unsigned int host::recovery_notification_delay() const throw() {
  return _recovery_notification_delay;
}

/**
 *  Get stalking_options.
 *
 *  @return The stalking_options.
 */
unsigned int host::stalking_options() const throw () {
  return _stalking_options;
}

/**
 *  Get statusmap_image.
 *
 *  @return The statusmap_image.
 */
std::string const& host::statusmap_image() const throw () {
  return _statusmap_image;
}

/**
 *  Get timezone.
 *
 *  @return The timezone.
 */
std::string const& host::timezone() const throw () {
  return _timezone;
}

/**
 *  Get vrml_image.
 *
 *  @return The vrml_image.
 */
std::string const& host::vrml_image() const throw () {
  return _vrml_image;
}

/**
 *  Get acknowledgement timeout.
 *
 *  @return Acknowledgement timeout.
 */
int host::get_acknowledgement_timeout() const throw () {
  return _acknowledgement_timeout;
}

/**
 *  Set acknowledgement timeout.
 *
 *  @param[in] value  The new acknowledgement timeout value.
 *
 *  @return True on success, false otherwise.
 */
bool host::set_acknowledgement_timeout(int value) {
  bool value_positive(value >= 0);
  if (value_positive)
    _acknowledgement_timeout = value;
  return value_positive;
}

/**
 *  Set action_url value.
 *
 *  @param[in] value The new action_url value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_action_url(std::string const& value) {
  _action_url = value;
  return true;
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
  return true;
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
  return true;
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
  return true;
}

/**
 *  Set checks_passive value.
 *
 *  @param[in] value The new checks_passive value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_checks_passive(bool value) {
  _checks_passive = value;
  return true;
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
  return true;
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
  return true;
}

/**
 *  Set check_interval value.
 *
 *  @param[in] value The new check_interval value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_check_interval(unsigned int value) {
  _check_interval = value;
  return true;
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
  return true;
}

/**
 *  Set contactgroups value.
 *
 *  @param[in] value The new contactgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_contactgroups(std::string const& value) {
  _contactgroups = value;
  return true;
}

/**
 *  Set contacts value.
 *
 *  @param[in] value The new contacts value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_contacts(std::string const& value) {
  _contacts = value;
  return true;
}

/**
 *  Set coords_2d value.
 *
 *  @param[in] value The new coords_2d value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_coords_2d(std::string const& value) {
  std::list<std::string> coords;
  string::split(value, coords, ',');
  if (coords.size() != 2)
    return false;

  int x;
  if (!string::to(string::trim(coords.front()).c_str(), x))
    return false;
  coords.pop_front();

  int y;
  if (!string::to(string::trim(coords.front()).c_str(), y))
    return false;

  _coords_2d = point_2d(x, y);
  return true;
}

/**
 *  Set coords_3d value.
 *
 *  @param[in] value The new coords_3d value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_coords_3d(std::string const& value) {
  std::list<std::string> coords;
  string::split(value, coords, ',');
  if (coords.size() != 3)
    return false;

  double x;
  if (!string::to(string::trim(coords.front()).c_str(), x))
    return false;
  coords.pop_front();

  double y;
  if (!string::to(string::trim(coords.front()).c_str(), y))
    return false;
  coords.pop_front();

  double z;
  if (!string::to(string::trim(coords.front()).c_str(), z))
    return false;

  _coords_3d = point_3d(x, y, z);
  return true;
}

/**
 *  Set display_name value.
 *
 *  @param[in] value The new display_name value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_display_name(std::string const& value) {
  _display_name = value;
  return true;
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
  return true;
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
  return true;
}

/**
 *  Set failure_prediction_enabled value.
 *
 *  @param[in] value The new failure_prediction_enabled value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_failure_prediction_enabled(bool value) {
  (void)value;
  logger(log_verification_error, basic)
    << "Warning: host failure_prediction_enabled is deprecated"
    << " This option will not be supported in 20.04.";
  ++config_warnings;
  return true;
}

/**
 *  Set failure_prediction_options value.
 *
 *  @param[in] value The new failure_prediction_options value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_failure_prediction_options(
       std::string const& value) {
  (void)value;
  logger(log_verification_error, basic)
    << "Warning: service failure_prediction_options is deprecated"
    << " This option will not be supported in 20.04.";
  ++config_warnings;
  return (true);
}

/**
 *  Set first_notification_delay value.
 *
 *  @param[in] value The new first_notification_delay value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_first_notification_delay(
       unsigned int value) {
  _first_notification_delay = value;
  return true;
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
  return true;
}

/**
 *  Set flap_detection_options value.
 *
 *  @param[in] value The new flap_detection_options value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_flap_detection_options(
       std::string const& value) {
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
      return false;
  }
  _flap_detection_options = options;
  return true;
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
  return true;
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
  return true;
}

/**
 *  Set host_id value.
 *
 *  @param[in] value The new host_id value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_host_id(uint64_t value) {
  _host_id = value;
  return true;
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
  return true;
}

/**
 *  Set hostgroups value.
 *
 *  @param[in] value The new hostgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_hostgroups(std::string const& value) {
  _hostgroups = value;
  return true;
}

/**
 *  Set icon_image value.
 *
 *  @param[in] value The new icon_image value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_icon_image(std::string const& value) {
  _icon_image = value;
  return true;
}

/**
 *  Set icon_image_alt value.
 *
 *  @param[in] value The new icon_image_alt value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_icon_image_alt(std::string const& value) {
  _icon_image_alt = value;
  return true;
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
    _initial_state =  engine::host::state_up;
  else if (data == "d" || data == "down")
    _initial_state =  engine::host::state_down;
  else if (data == "u" || data == "unreachable")
    _initial_state =  engine::host::state_unreachable;
  else
    return false;
  return true;
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
  return true;
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
    return false;
  _max_check_attempts = value;
  return true;
}

/**
 *  Set notes value.
 *
 *  @param[in] value The new notes value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_notes(std::string const& value) {
  _notes = value;
  return true;
}

/**
 *  Set notes_url value.
 *
 *  @param[in] value The new notes_url value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_notes_url(std::string const& value) {
  _notes_url = value;
  return true;
}

/**
 *  Set notifications_enabled value.
 *
 *  @param[in] value The new notifications_enabled value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_notifications_enabled(bool value) {
  _notifications_enabled = value;
  return true;
}

/**
 *  Set notification_interval value.
 *
 *  @param[in] value The new notification_interval value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_notification_interval(
       unsigned int value) {
  _notification_interval = value;
  return true;
}

/**
 *  Set notification_options value.
 *
 *  @param[in] value The new notification_options value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_notification_options(
       std::string const& value) {
  unsigned short options(none);
  std::list<std::string> values;
  string::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    string::trim(*it);
    if (*it == "d" || *it == "down")
      options |= down;
    else if (*it == "u" || *it == "unreachable")
      options |= unreachable;
    else if (*it == "r" || *it == "recovery")
      options |= up;
    else if (*it == "f" || *it == "flapping")
      options |= flapping;
    else if (*it == "s" || *it == "downtime")
      options |= downtime;
    else if (*it == "n" || *it == "none")
      options = none;
    else if (*it == "a" || *it == "all")
      options = down | unreachable | up | flapping | downtime;
    else
      return false;
  }
  _notification_options = options;
  return true;
}

/**
 *  Set notification_period value.
 *
 *  @param[in] value The new notification_period value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_notification_period(
       std::string const& value) {
  _notification_period = value;
  return true;
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
  return true;
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
  return true;
}

/**
 *  Set process_perf_data value.
 *
 *  @param[in] value The new process_perf_data value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_process_perf_data(bool value) {
  _process_perf_data = value;
  return true;
}

/**
 *  Set retain_nonstatus_information value.
 *
 *  @param[in] value The new retain_nonstatus_information value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_retain_nonstatus_information(
       bool value) {
  _retain_nonstatus_information = value;
  return true;
}

/**
 *  Set retain_status_information value.
 *
 *  @param[in] value The new retain_status_information value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_retain_status_information(bool value) {
  _retain_status_information = value;
  return true;
}

/**
 *  Set retry_interval value.
 *
 *  @param[in] value The new retry_interval value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_retry_interval(unsigned int value) {
  _retry_interval = value;
  return true;
}

/**
 *  Set recovery_notification_delay value.
 *
 *  @param[in] value  The new recovery_notification_delay value.
 *
 *  @return  True on success, otherwhise false.
 */
bool host::_set_recovery_notification_delay(unsigned int value) {
  _recovery_notification_delay = value;
  return true;
}

/**
 *  Set stalking_options value.
 *
 *  @param[in] value The new stalking_options value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_stalking_options(
       std::string const& value) {
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
      return false;
  }
  _stalking_options = options;
  return true;
}

/**
 *  Set statusmap_image value.
 *
 *  @param[in] value The new statusmap_image value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_statusmap_image(
       std::string const& value) {
  _statusmap_image = value;
  return true;
}

/**
 *  Set timezone value.
 *
 *  @param[in] value  The new timezone.
 *
 *  @return True on success, false otherwise.
 */
bool host::_set_timezone(std::string const& value) {
  _timezone = value;
  return true;
}

/**
 *  Set vrml_image value.
 *
 *  @param[in] value The new vrml_image value.
 *
 *  @return True on success, otherwise false.
 */
bool host::_set_vrml_image(std::string const& value) {
  _vrml_image = value;
  return true;
}

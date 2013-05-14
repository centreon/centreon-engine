/*
** Copyright 2011-2013 Merethis
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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<host, type, &host::method>::generic

static struct {
  std::string const name;
  bool (*func)(host&, std::string const&);
} gl_setters[] = {
  { "host_name",                    SETTER(std::string const&, _set_host_name) },
  { "display_name",                 SETTER(std::string const&, _set_display_name) },
  { "alias",                        SETTER(std::string const&, _set_alias) },
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
  { "gd2_image",                    SETTER(std::string const&, _set_gd2_image) },
  { "statusmap_image",              SETTER(std::string const&, _set_statusmap_image) },
  { "initial_state",                SETTER(std::string const&, _set_initial_state) },
  { "check_interval",               SETTER(unsigned int, _set_check_interval) },
  { "normal_check_interval",        SETTER(unsigned int, _set_check_interval) },
  { "retry_interval",               SETTER(unsigned int, _set_retry_interval) },
  { "retry_check_interval",         SETTER(unsigned int, _set_retry_interval) },
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
  { "2d_coords",                    SETTER(std::string const&, _set_2d_coords) },
  { "3d_coords",                    SETTER(std::string const&, _set_3d_coords) },
  { "obsess_over_host",             SETTER(bool, _set_obsess_over_host) },
  { "retain_status_information",    SETTER(bool, _set_retain_status_information) },
  { "retain_nonstatus_information", SETTER(bool, _set_retain_nonstatus_information) }
};

// Default values.
static point_2d const     default_2d_coords(-1, -1);
static point_3d const     default_3d_coords(0.0, 0.0, 0.0);
static bool const         default_checks_active(true);
static bool const         default_checks_passive(true);
static bool const         default_check_freshness(false);
static unsigned int const default_check_interval(5);
static bool const         default_event_handler_enabled(true);
static unsigned int const default_first_notification_delay(0);
static bool const         default_flap_detection_enabled(true);
static unsigned int const default_flap_detection_options(host::up | host::down | host::unreachable);
static unsigned int const default_freshness_threshold(0);
static unsigned int const default_high_flap_threshold(0);
static unsigned int const default_initial_state(HOST_UP);
static unsigned int const default_low_flap_threshold(0);
static unsigned int const default_max_check_attempts(0);
static bool const         default_notifications_enabled(true);
static unsigned int const default_notification_interval(30);
static unsigned int const default_notification_options(host::none);
static bool const         default_obsess_over_host(true);
static bool const         default_process_perf_data(true);
static bool const         default_retain_nonstatus_information(true);
static bool const         default_retain_status_information(true);
static unsigned int const default_retry_interval(1);
static unsigned int const default_stalking_options(host::none);

/**
 *  Default constructor.
 */
host::host()
  : object("host"),
    _2d_coords(default_2d_coords),
    _3d_coords(default_3d_coords),
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
    _stalking_options(default_stalking_options) {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The host to copy.
 */
host::host(host const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
host::~host() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The host to copy.
 *
 *  @return This host.
 */
host& host::operator=(host const& right) {
  if (this != &right) {
    object::operator=(right);
    _2d_coords = right._2d_coords;
    _3d_coords = right._3d_coords;
    _action_url = right._action_url;
    _address = right._address;
    _alias = right._alias;
    _checks_active = right._checks_active;
    _checks_passive = right._checks_passive;
    _check_command = right._check_command;
    _check_freshness = right._check_freshness;
    _check_interval = right._check_interval;
    _check_period = right._check_period;
    _contactgroups = right._contactgroups;
    _contacts = right._contacts;
    _customvariables = right._customvariables;
    _display_name = right._display_name;
    _event_handler = right._event_handler;
    _event_handler_enabled = right._event_handler_enabled;
    _first_notification_delay = right._first_notification_delay;
    _flap_detection_enabled = right._flap_detection_enabled;
    _flap_detection_options = right._flap_detection_options;
    _freshness_threshold = right._freshness_threshold;
    _gd2_image = right._gd2_image;
    _high_flap_threshold = right._high_flap_threshold;
    _hostgroups = right._hostgroups;
    _host_name = right._host_name;
    _icon_image = right._icon_image;
    _icon_image_alt = right._icon_image_alt;
    _initial_state = right._initial_state;
    _low_flap_threshold = right._low_flap_threshold;
    _max_check_attempts = right._max_check_attempts;
    _notes = right._notes;
    _notes_url = right._notes_url;
    _notifications_enabled = right._notifications_enabled;
    _notification_interval = right._notification_interval;
    _notification_options = right._notification_options;
    _notification_period = right._notification_period;
    _obsess_over_host = right._obsess_over_host;
    _parents = right._parents;
    _process_perf_data = right._process_perf_data;
    _retain_nonstatus_information = right._retain_nonstatus_information;
    _retain_status_information = right._retain_status_information;
    _retry_interval = right._retry_interval;
    _stalking_options = right._stalking_options;
    _statusmap_image = right._statusmap_image;
    _vrml_image = right._vrml_image;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The host to compare.
 *
 *  @return True if is the same host, otherwise false.
 */
bool host::operator==(host const& right) const throw () {
  return (object::operator==(right)
          && _2d_coords == right._2d_coords
          && _3d_coords == right._3d_coords
          && _action_url == right._action_url
          && _address == right._address
          && _alias == right._alias
          && _checks_active == right._checks_active
          && _checks_passive == right._checks_passive
          && _check_command == right._check_command
          && _check_freshness == right._check_freshness
          && _check_interval == right._check_interval
          && _check_period == right._check_period
          && _contactgroups == right._contactgroups
          && _contacts == right._contacts
          // XXX: && _customvariables == right._customvariables
          && _display_name == right._display_name
          && _event_handler == right._event_handler
          && _event_handler_enabled == right._event_handler_enabled
          && _first_notification_delay == right._first_notification_delay
          && _flap_detection_enabled == right._flap_detection_enabled
          && _flap_detection_options == right._flap_detection_options
          && _freshness_threshold == right._freshness_threshold
          && _gd2_image == right._gd2_image
          && _high_flap_threshold == right._high_flap_threshold
          && _hostgroups == right._hostgroups
          && _host_name == right._host_name
          && _icon_image == right._icon_image
          && _icon_image_alt == right._icon_image_alt
          && _initial_state == right._initial_state
          && _low_flap_threshold == right._low_flap_threshold
          && _max_check_attempts == right._max_check_attempts
          && _notes == right._notes
          && _notes_url == right._notes_url
          && _notifications_enabled == right._notifications_enabled
          && _notification_interval == right._notification_interval
          && _notification_options == right._notification_options
          && _notification_period == right._notification_period
          && _obsess_over_host == right._obsess_over_host
          && _parents == right._parents
          && _process_perf_data == right._process_perf_data
          && _retain_nonstatus_information == right._retain_nonstatus_information
          && _retain_status_information == right._retain_status_information
          && _retry_interval == right._retry_interval
          && _stalking_options == right._stalking_options
          && _statusmap_image == right._statusmap_image
          && _vrml_image == right._vrml_image);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The host to compare.
 *
 *  @return True if is not the same host, otherwise false.
 */
bool host::operator!=(host const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void host::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "XXX: todo");
  host const& tmpl(static_cast<host const&>(obj));

  MRG_DEFAULT(_2d_coords);
  MRG_DEFAULT(_3d_coords);
  MRG_STRING(_action_url);
  MRG_STRING(_address);
  MRG_STRING(_alias);
  MRG_DEFAULT(_checks_active);
  MRG_DEFAULT(_checks_passive);
  MRG_STRING(_check_command);
  MRG_DEFAULT(_check_freshness);
  MRG_DEFAULT(_check_interval);
  MRG_STRING(_check_period);
  MRG_INHERIT(_contactgroups);
  MRG_INHERIT(_contacts);
  MRG_MAP(_customvariables);
  MRG_STRING(_display_name);
  MRG_STRING(_event_handler);
  MRG_DEFAULT(_event_handler_enabled);
  MRG_DEFAULT(_first_notification_delay);
  MRG_DEFAULT(_flap_detection_enabled);
  MRG_DEFAULT(_flap_detection_options);
  MRG_DEFAULT(_freshness_threshold);
  MRG_STRING(_gd2_image);
  MRG_DEFAULT(_high_flap_threshold);
  MRG_INHERIT(_hostgroups);
  MRG_STRING(_host_name);
  MRG_STRING(_icon_image);
  MRG_STRING(_icon_image_alt);
  MRG_DEFAULT(_initial_state);
  MRG_DEFAULT(_low_flap_threshold);
  MRG_DEFAULT(_max_check_attempts);
  MRG_STRING(_notes);
  MRG_STRING(_notes_url);
  MRG_DEFAULT(_notifications_enabled);
  MRG_DEFAULT(_notification_interval);
  MRG_DEFAULT(_notification_options);
  MRG_STRING(_notification_period);
  MRG_DEFAULT(_obsess_over_host);
  MRG_INHERIT(_parents);
  MRG_DEFAULT(_process_perf_data);
  MRG_DEFAULT(_retain_nonstatus_information);
  MRG_DEFAULT(_retain_status_information);
  MRG_DEFAULT(_retry_interval);
  MRG_DEFAULT(_stalking_options);
  MRG_STRING(_statusmap_image);
  MRG_STRING(_vrml_image);
}

/**
 *  Parse and set the host property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool host::parse(std::string const& key, std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));

  return (false);
}

void host::_set_2d_coords(std::string const& value) {
  // XXX:
}

void host::_set_3d_coords(std::string const& value) {
  // XXX:
}

void host::_set_action_url(std::string const& value) {
  _action_url = value;
}

void host::_set_address(std::string const& value) {
  _address = value;
}

void host::_set_alias(std::string const& value) {
  _alias = value;
}

void host::_set_checks_active(bool value) {
  _checks_active = value; // XXX:
}

void host::_set_checks_passive(bool value) {
  _checks_passive = value; // XXX:
}

void host::_set_check_command(std::string const& value) {
  _check_command = value;
}

void host::_set_check_freshness(bool value) {
  _check_freshness = value;
}

void host::_set_check_interval(unsigned int value) {
  _check_interval = value;
}

void host::_set_check_period(std::string const& value) {
  _check_period = value;
}

void host::_set_contactgroups(std::string const& value) {
  _contactgroups.clear();
  misc::split(value, _contactgroups.get(), ',');
}

void host::_set_contacts(std::string const& value) {
  _contacts.clear();
  misc::split(value, _contacts.get(), ',');
}

void host::_set_display_name(std::string const& value) {
  _display_name = value;
}

void host::_set_event_handler(std::string const& value) {
  _event_handler = value;
}

void host::_set_event_handler_enabled(bool value) {
  _event_handler_enabled = value;
}

void host::_set_failure_prediction_enabled(bool value) {
  // XXX:
}

void host::_set_failure_prediction_options(std::string const& value) {
  // XXX:
}

void host::_set_first_notification_delay(unsigned int value) {
  _first_notification_delay = value;
}

void host::_set_flap_detection_enabled(bool value) {
  _flap_detection_enabled = value;
}

void host::_set_flap_detection_options(std::string const& value) {
  _flap_detection_options = 0; // XXX:
}

void host::_set_freshness_threshold(unsigned int value) {
  _freshness_threshold = value;
}

void host::_set_gd2_image(std::string const& value) {
  _gd2_image = value;
}

void host::_set_high_flap_threshold(unsigned int value) {
  _high_flap_threshold = value;
}

void host::_set_host_name(std::string const& value) {
  _host_name = value;
}

void host::_set_hostgroups(std::string const& value) {
  _hostgroups.clear();
  misc::split(value, _hostgroups.get(), ',');
}

void host::_set_icon_image(std::string const& value) {
  _icon_image = value;
}

void host::_set_icon_image_alt(std::string const& value) {
  _icon_image_alt = value;
}

void host::_set_initial_state(std::string const& value) {
  _initial_state = 0; // XXX:
}

void host::_set_low_flap_threshold(unsigned int value) {
  _low_flap_threshold = value;
}

void host::_set_max_check_attempts(unsigned int value) {
  _max_check_attempts = value;
}

void host::_set_notes(std::string const& value) {
  _notes = value;
}

void host::_set_notes_url(std::string const& value) {
  _notes_url = value;
}

void host::_set_notifications_enabled(bool value) {
  _notifications_enabled = value;
}

void host::_set_notification_interval(unsigned int value) {
  _notification_interval = value;
}

void host::_set_notification_options(std::string const& value) {
  _notification_options = 0; // XXX:
}

void host::_set_notification_period(std::string const& value) {
  _notification_period = value;
}

void host::_set_obsess_over_host(bool value) {
  _obsess_over_host = value;
}

void host::_set_parents(std::string const& value) {
  _parents.clear();
  misc::split(value, _parents.get(), ',');
}

void host::_set_process_perf_data(bool value) {
  _process_perf_data = value;
}

void host::_set_retain_nonstatus_information(bool value) {
  _retain_nonstatus_information = value;
}

void host::_set_retain_status_information(bool value) {
  _retain_status_information = value;
}

void host::_set_retry_interval(unsigned int value) {
  _retry_interval = value;
}

void host::_set_stalking_options(std::string const& value) {
  _stalking_options = 0; // XXX:
}

void host::_set_statusmap_image(std::string const& value) {
  _statusmap_image = value;
}

void host::_set_vrml_image(std::string const& value) {
  _vrml_image = value;
}

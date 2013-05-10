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

#define setter(type, method) \
  &object::setter<host, type, &host::method>::generic

static struct {
  std::string const name;
  bool (*func)(host&, std::string const&);
} gl_setters[] = {
  { "host_name",                    setter(std::string const&, _set_hosts) },
  { "display_name",                 setter(std::string const&, _set_display_name) },
  { "alias",                        setter(std::string const&, _set_alias) },
  { "address",                      setter(std::string const&, _set_address) },
  { "parents",                      setter(std::string const&, _set_parents) },
  { "host_groups",                  setter(std::string const&, _set_hostgroups) },
  { "hostgroups",                   setter(std::string const&, _set_hostgroups) },
  { "contact_groups",               setter(std::string const&, _set_contactgroups) },
  { "contacts",                     setter(std::string const&, _set_contacts) },
  { "notification_period",          setter(std::string const&, _set_notification_period) },
  { "check_command",                setter(std::string const&, _set_check_command) },
  { "check_period",                 setter(std::string const&, _set_check_period) },
  { "event_handler",                setter(std::string const&, _set_event_handler) },
  { "failure_prediction_options",   setter(std::string const&, _set_failure_prediction_options) },
  { "notes",                        setter(std::string const&, _set_notes) },
  { "notes_url",                    setter(std::string const&, _set_notes_url) },
  { "action_url",                   setter(std::string const&, _set_action_url) },
  { "icon_image",                   setter(std::string const&, _set_icon_image) },
  { "icon_image_alt",               setter(std::string const&, _set_icon_image_alt) },
  { "vrml_image",                   setter(std::string const&, _set_vrml_image) },
  { "gd2_image",                    setter(std::string const&, _set_gd2_image) },
  { "statusmap_image",              setter(std::string const&, _set_statusmap_image) },
  { "initial_state",                setter(std::string const&, _set_initial_state) },
  { "check_interval",               setter(unsigned int, _set_check_interval) },
  { "normal_check_interval",        setter(unsigned int, _set_check_interval) },
  { "retry_interval",               setter(unsigned int, _set_retry_interval) },
  { "retry_check_interval",         setter(unsigned int, _set_retry_interval) },
  { "max_check_attempts",           setter(unsigned int, _set_max_check_attempts) },
  { "checks_enabled",               setter(bool, _set_checks_enabled) },
  { "active_checks_enabled",        setter(bool, _set_checks_active) },
  { "passive_checks_enabled",       setter(bool, _set_checks_passive) },
  { "event_handler_enabled",        setter(bool, _set_event_handler_enabled) },
  { "check_freshness",              setter(bool, _set_check_freshness) },
  { "freshness_threshold",          setter(unsigned int, _set_freshness_threshold) },
  { "low_flap_threshold",           setter(unsigned int, _set_low_flap_threshold) },
  { "high_flap_threshold",          setter(unsigned int, _set_high_flap_threshold) },
  { "flap_detection_enabled",       setter(bool, _set_flap_detection_enabled) },
  { "flap_detection_options",       setter(std::string const&, _set_flap_detection_options) },
  { "notification_options",         setter(std::string const&, _set_notification_options) },
  { "notifications_enabled",        setter(bool, _set_notifications_enabled) },
  { "notification_interval",        setter(unsigned int, _set_notification_interval) },
  { "first_notification_delay",     setter(unsigned int, _set_first_notification_delay) },
  { "stalking_options",             setter(std::string const&, _set_stalking_options) },
  { "process_perf_data",            setter(bool, _set_process_perf_data) },
  { "failure_prediction_enabled",   setter(bool, _set_failure_prediction_enabled) },
  { "2d_coords",                    setter(int, _set_2d_coords) },
  { "3d_coords",                    setter(int, _set_3d_coords) },
  { "obsess_over_host",             setter(bool, _set_obsess_over_host) },
  { "retain_status_information",    setter(bool, _set_retain_status_information) },
  { "retain_nonstatus_information", setter(bool, _set_retain_nonstatus_information) }
};

/**
 *  Default constructor.
 */
host::host()
  : object("host") {

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
  return (object::operator==(right));
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
  return (object::parse(key, value));
}

void host::_set_2d_coords(int value) {
  _2d_coords = value;
}

void host::_set_3d_coords(int value) {
  _3d_coords = value;
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

void host::_set_checks_enabled(bool value) {
  _checks_enabled = value;
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
  misc::split(value, _contactgroups, ',');
}

void host::_set_contacts(std::string const& value) {
  _contacts.clear();
  misc::split(value, _contacts, ',');
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
  _failure_prediction_enabled = value;
}

void host::_set_failure_prediction_options(std::string const& value) {
  _failure_prediction_options = 0; // XXX:
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

void host::_set_hosts(std::string const& value) {
  _hosts.clear();
  misc::split(value, _hosts, ',');
}

void host::_set_hostgroups(std::string const& value) {
  _hostgroups.clear();
  misc::split(value, _hostgroups, ',');
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
  _parents = value;
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

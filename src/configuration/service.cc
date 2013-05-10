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

#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine::configuration;

#define setter(type, method) \
  &object::setter<service, type, &service::method>::generic

static struct {
  std::string const name;
  bool (*func)(service&, std::string const&);
} gl_setters[] = {
  { "host",                         setter(std::string const&, _set_hosts) },
  { "hosts",                        setter(std::string const&, _set_hosts) },
  { "host_name",                    setter(std::string const&, _set_hosts) },
  { "service_description",          setter(std::string const&, _set_service_description) },
  { "description",                  setter(std::string const&, _set_service_description) },
  { "display_name",                 setter(std::string const&, _set_display_name) },
  { "hostgroup",                    setter(std::string const&, _set_hostgroups) },
  { "hostgroups",                   setter(std::string const&, _set_hostgroups) },
  { "hostgroup_name",               setter(std::string const&, _set_hostgroups) },
  { "service_groups",               setter(std::string const&, _set_servicegroups) },
  { "servicegroups",                setter(std::string const&, _set_servicegroups) },
  { "check_command",                setter(std::string const&, _set_check_command) },
  { "check_period",                 setter(std::string const&, _set_check_period) },
  { "event_handler",                setter(std::string const&, _set_event_handler) },
  { "notification_period",          setter(std::string const&, _set_notification_period) },
  { "contact_groups",               setter(std::string const&, _set_contactgroups) },
  { "contacts",                     setter(std::string const&, _set_contacts) },
  { "failure_prediction_options",   setter(std::string const&, _set_failure_prediction_options) },
  { "notes",                        setter(std::string const&, _set_notes) },
  { "notes_url",                    setter(std::string const&, _set_notes_url) },
  { "action_url",                   setter(std::string const&, _set_action_url) },
  { "icon_image",                   setter(std::string const&, _set_icon_image) },
  { "icon_image_alt",               setter(std::string const&, _set_icon_image_alt) },
  { "initial_state",                setter(std::string const&, _set_initial_state) },
  { "max_check_attempts",           setter(unsigned int, _set_max_check_attempts) },
  { "check_interval",               setter(unsigned int, _set_check_interval) },
  { "normal_check_interval",        setter(unsigned int, _set_check_interval) },
  { "retry_interval",               setter(unsigned int, _set_retry_interval) },
  { "retry_check_interval",         setter(unsigned int, _set_retry_interval) },
  { "active_checks_enabled",        setter(bool, _set_checks_active) },
  { "passive_checks_enabled",       setter(bool, _set_checks_passive) },
  { "parallelize_check",            setter(bool, _set_parallelize_check) },
  { "is_volatile",                  setter(bool, _set_is_volatile) },
  { "obsess_over_service",          setter(bool, _set_obsess_over_service) },
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
  { "retain_status_information",    setter(bool, _set_retain_status_information) },
  { "retain_nonstatus_information", setter(bool, _set_retain_nonstatus_information) }
};

/**
 *  Default constructor.
 */
service::service()
  : object("service") {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The service to copy.
 */
service::service(service const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
service::~service() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The service to copy.
 *
 *  @return This service.
 */
service& service::operator=(service const& right) {
  if (this != &right) {
    object::operator=(right);
    _hosts = right._hosts;
    _service_description = right._service_description;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The service to compare.
 *
 *  @return True if is the same service, otherwise false.
 */
bool service::operator==(service const& right) const throw () {
  return (object::operator==(right)
          && _hosts == right._hosts
          && _service_description == right._service_description);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The service to compare.
 *
 *  @return True if is not the same service, otherwise false.
 */
bool service::operator!=(service const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Parse and set the service property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool service::parse(std::string const& key, std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  return (object::parse(key, value));
}

void service::_set_action_url(std::string const& value) {
  _action_url = value;
}

void service::_set_check_command(std::string const& value) {
  _check_command = value;
}

void service::_set_checks_active(bool value) {
  _checks_active = value; // XXX:
}

void service::_set_checks_passive(bool value) {
  _checks_passive = value; // XXX:
}

void service::_set_check_freshness(bool value) {
  _check_freshness = value;
}

void service::_set_check_interval(unsigned int value) {
  _check_interval = value;
}

void service::_set_check_period(std::string const& value) {
  _check_period = value;
}

void service::_set_contactgroups(std::string const& value) {
  _contactgroups.clear();
  misc::split(value, _contactgroups, ',');
}

void service::_set_contacts(std::string const& value) {
  _contacts.clear();
  misc::split(value, _contacts, ',');
}

void service::_set_display_name(std::string const& value) {
  _display_name = value;
}

void service::_set_event_handler(std::string const& value) {
  _event_handler = value;
}

void service::_set_event_handler_enabled(bool value) {
  _event_handler_enabled = value;
}

void service::_set_failure_prediction_enabled(bool value) {
  _failure_prediction_enabled = value;
}

void service::_set_failure_prediction_options(std::string const& value) {
  _failure_prediction_options = 0; // XXX:
}

void service::_set_first_notification_delay(unsigned int value) {
  _first_notification_delay = value;
}

void service::_set_flap_detection_enabled(bool value) {
  _flap_detection_enabled = value;
}

void service::_set_flap_detection_options(std::string const& value) {
  _flap_detection_options = 0; // XXX:
}

void service::_set_freshness_threshold(unsigned int value) {
  _freshness_threshold = value;
}

void service::_set_high_flap_threshold(unsigned int value) {
  _high_flap_threshold = value;
}

void service::_set_hostgroups(std::string const& value) {
  _hostgroups.clear();
  misc::split(value, _hostgroups, ',');
}

void service::_set_hosts(std::string const& value) {
  _hosts.clear();
  misc::split(value, _hosts, ',');
}

void service::_set_icon_image(std::string const& value) {
  _icon_image = value;
}

void service::_set_icon_image_alt(std::string const& value) {
  _icon_image_alt = value;
}

void service::_set_initial_state(std::string const& value) {
  _initial_state = 0; // XXX:
}

void service::_set_is_volatile(bool value) {
  _is_volatile = value;
}

void service::_set_low_flap_threshold(unsigned int value) {
  _low_flap_threshold = value;
}

void service::_set_max_check_attempts(unsigned int value) {
  _max_check_attempts = value;
}

void service::_set_notes(std::string const& value) {
  _notes = value;
}

void service::_set_notes_url(std::string const& value) {
  _notes_url = value;
}

void service::_set_notifications_enabled(bool value) {
  _notifications_enabled = value;
}

void service::_set_notification_options(std::string const& value) {
  _notification_options = 0; // XXX:
}

void service::_set_notification_interval(unsigned int value) {
  _notification_interval = value;
}

void service::_set_notification_period(std::string const& value) {
  _notification_period = value;
}

void service::_set_obsess_over_service(bool value) {
  _obsess_over_service = value;
}

void service::_set_parallelize_check(bool value) {
  // XXX:
}

void service::_set_process_perf_data(bool value) {
  _process_perf_data = value;
}

void service::_set_retain_nonstatus_information(bool value) {
  _retain_nonstatus_information = value;
}

void service::_set_retain_status_information(bool value) {
  _retain_status_information = value;
}

void service::_set_retry_interval(unsigned int value) {
  _retry_interval = value;
}

void service::_set_servicegroups(std::string const& value) {
  _servicegroups.clear();
  misc::split(value, _servicegroups, ',');
}

void service::_set_service_description(std::string const& value) {
  _service_description = value;
}

void service::_set_stalking_options(std::string const& value) {
  _stalking_options = 0; // XXX:
}

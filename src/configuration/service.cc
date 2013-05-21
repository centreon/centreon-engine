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

using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<service, type, &service::method>::generic

static struct {
  std::string const name;
  bool (*func)(service&, std::string const&);
} gl_setters[] = {
  { "host",                         SETTER(std::string const&, _set_hosts) },
  { "hosts",                        SETTER(std::string const&, _set_hosts) },
  { "host_name",                    SETTER(std::string const&, _set_hosts) },
  { "service_description",          SETTER(std::string const&, _set_service_description) },
  { "description",                  SETTER(std::string const&, _set_service_description) },
  { "display_name",                 SETTER(std::string const&, _set_display_name) },
  { "hostgroup",                    SETTER(std::string const&, _set_hostgroups) },
  { "hostgroups",                   SETTER(std::string const&, _set_hostgroups) },
  { "hostgroup_name",               SETTER(std::string const&, _set_hostgroups) },
  { "service_groups",               SETTER(std::string const&, _set_servicegroups) },
  { "servicegroups",                SETTER(std::string const&, _set_servicegroups) },
  { "check_command",                SETTER(std::string const&, _set_check_command) },
  { "check_period",                 SETTER(std::string const&, _set_check_period) },
  { "event_handler",                SETTER(std::string const&, _set_event_handler) },
  { "notification_period",          SETTER(std::string const&, _set_notification_period) },
  { "contact_groups",               SETTER(std::string const&, _set_contactgroups) },
  { "contacts",                     SETTER(std::string const&, _set_contacts) },
  { "failure_prediction_options",   SETTER(std::string const&, _set_failure_prediction_options) },
  { "notes",                        SETTER(std::string const&, _set_notes) },
  { "notes_url",                    SETTER(std::string const&, _set_notes_url) },
  { "action_url",                   SETTER(std::string const&, _set_action_url) },
  { "icon_image",                   SETTER(std::string const&, _set_icon_image) },
  { "icon_image_alt",               SETTER(std::string const&, _set_icon_image_alt) },
  { "initial_state",                SETTER(std::string const&, _set_initial_state) },
  { "max_check_attempts",           SETTER(unsigned int, _set_max_check_attempts) },
  { "check_interval",               SETTER(unsigned int, _set_check_interval) },
  { "normal_check_interval",        SETTER(unsigned int, _set_check_interval) },
  { "retry_interval",               SETTER(unsigned int, _set_retry_interval) },
  { "retry_check_interval",         SETTER(unsigned int, _set_retry_interval) },
  { "active_checks_enabled",        SETTER(bool, _set_checks_active) },
  { "passive_checks_enabled",       SETTER(bool, _set_checks_passive) },
  { "parallelize_check",            SETTER(bool, _set_parallelize_check) },
  { "is_volatile",                  SETTER(bool, _set_is_volatile) },
  { "obsess_over_service",          SETTER(bool, _set_obsess_over_service) },
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
  { "retain_status_information",    SETTER(bool, _set_retain_status_information) },
  { "retain_nonstatus_information", SETTER(bool, _set_retain_nonstatus_information) }
};

// Default values.
static bool const         default_checks_active(true);
static bool const         default_checks_passive(true);
static bool const         default_check_freshness(0);
static unsigned int const default_check_interval(5);
static bool const         default_event_handler_enabled(true);
static unsigned int const default_first_notification_delay(0);
static bool const         default_flap_detection_enabled(true);
static unsigned int const default_flap_detection_options(service::ok | service::warning | service::unknown | service::critical);
static unsigned int const default_freshness_threshold(0);
static unsigned int const default_high_flap_threshold(0);
static unsigned int const default_initial_state(STATE_OK);
static bool const         default_is_volatile(false);
static unsigned int const default_low_flap_threshold(0);
static unsigned int const default_max_check_attempts(0);
static bool const         default_notifications_enabled(true);
static unsigned int const default_notification_interval(30);
static unsigned int const default_notification_options(service::none);
static bool const         default_obsess_over_service(true);
static bool const         default_process_perf_data(true);
static bool const         default_retain_nonstatus_information(true);
static bool const         default_retain_status_information(true);
static unsigned int const default_retry_interval(1);
static unsigned int const default_stalking_options(service::none);

/**
 *  Default constructor.
 */
service::service()
  : object("service"),
    _checks_active(default_checks_active),
    _checks_passive(default_checks_passive),
    _check_command_is_important(false),
    _check_freshness(default_check_freshness),
    _check_interval(default_check_interval),
    _event_handler_enabled(default_event_handler_enabled),
    _first_notification_delay(default_first_notification_delay),
    _flap_detection_enabled(default_flap_detection_enabled),
    _flap_detection_options(default_flap_detection_options),
    _freshness_threshold(default_freshness_threshold),
    _high_flap_threshold(default_high_flap_threshold),
    _initial_state(default_initial_state),
    _is_volatile(default_is_volatile),
    _low_flap_threshold(default_low_flap_threshold),
    _max_check_attempts(default_max_check_attempts),
    _notifications_enabled(default_notifications_enabled),
    _notification_interval(default_notification_interval),
    _notification_options(default_notification_options),
    _obsess_over_service(default_obsess_over_service),
    _process_perf_data(default_process_perf_data),
    _retain_nonstatus_information(default_retain_nonstatus_information),
    _retain_status_information(default_retain_status_information),
    _retry_interval(default_retry_interval),
    _stalking_options(default_stalking_options) {

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
    _action_url = right._action_url;
    _checks_active = right._checks_active;
    _checks_passive = right._checks_passive;
    _check_command = right._check_command;
    _check_command_is_important = right._check_command_is_important;
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
    _high_flap_threshold = right._high_flap_threshold;
    _hostgroups = right._hostgroups;
    _hosts = right._hosts;
    _icon_image = right._icon_image;
    _icon_image_alt = right._icon_image_alt;
    _initial_state = right._initial_state;
    _is_volatile = right._is_volatile;
    _low_flap_threshold = right._low_flap_threshold;
    _max_check_attempts = right._max_check_attempts;
    _notes = right._notes;
    _notes_url = right._notes_url;
    _notifications_enabled = right._notifications_enabled;
    _notification_interval = right._notification_interval;
    _notification_options = right._notification_options;
    _notification_period = right._notification_period;
    _obsess_over_service = right._obsess_over_service;
    _process_perf_data = right._process_perf_data;
    _retain_nonstatus_information = right._retain_nonstatus_information;
    _retain_status_information = right._retain_status_information;
    _retry_interval = right._retry_interval;
    _servicegroups = right._servicegroups;
    _service_description = right._service_description;
    _stalking_options = right._stalking_options;
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
          && _action_url == right._action_url
          && _checks_active == right._checks_active
          && _checks_passive == right._checks_passive
          && _check_command == right._check_command
          && _check_command_is_important == right._check_command_is_important
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
          && _high_flap_threshold == right._high_flap_threshold
          && _hostgroups == right._hostgroups
          && _hosts == right._hosts
          && _icon_image == right._icon_image
          && _icon_image_alt == right._icon_image_alt
          && _initial_state == right._initial_state
          && _is_volatile == right._is_volatile
          && _low_flap_threshold == right._low_flap_threshold
          && _max_check_attempts == right._max_check_attempts
          && _notes == right._notes
          && _notes_url == right._notes_url
          && _notifications_enabled == right._notifications_enabled
          && _notification_interval == right._notification_interval
          && _notification_options == right._notification_options
          && _notification_period == right._notification_period
          && _obsess_over_service == right._obsess_over_service
          && _process_perf_data == right._process_perf_data
          && _retain_nonstatus_information == right._retain_nonstatus_information
          && _retain_status_information == right._retain_status_information
          && _retry_interval == right._retry_interval
          && _servicegroups == right._servicegroups
          && _service_description == right._service_description
          && _stalking_options == right._stalking_options);
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
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void service::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
  service const& tmpl(static_cast<service const&>(obj));

  MRG_STRING(_action_url);
  // XXX: is important -> MRG_STRING(_check_command);
  MRG_DEFAULT(_checks_active);
  MRG_DEFAULT(_checks_passive);
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
  MRG_DEFAULT(_high_flap_threshold);
  MRG_INHERIT(_hostgroups);
  MRG_INHERIT(_hosts);
  MRG_STRING(_icon_image);
  MRG_STRING(_icon_image_alt);
  MRG_DEFAULT(_initial_state);
  MRG_DEFAULT(_is_volatile);
  MRG_DEFAULT(_low_flap_threshold);
  MRG_DEFAULT(_max_check_attempts);
  MRG_STRING(_notes);
  MRG_STRING(_notes_url);
  MRG_DEFAULT(_notifications_enabled);
  MRG_DEFAULT(_notification_interval);
  MRG_DEFAULT(_notification_options);
  MRG_STRING(_notification_period);
  MRG_DEFAULT(_obsess_over_service);
  MRG_DEFAULT(_process_perf_data);
  MRG_DEFAULT(_retain_nonstatus_information);
  MRG_DEFAULT(_retain_status_information);
  MRG_DEFAULT(_retry_interval);
  MRG_INHERIT(_servicegroups);
  MRG_STRING(_service_description);
  MRG_DEFAULT(_stalking_options);
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

  return (false);
}

void service::_set_action_url(std::string const& value) {
  _action_url = value;
}

void service::_set_check_command(std::string const& value) {
  if (!value.empty() && value[0] == '!') {
    _check_command_is_important = true;
    _check_command = value.substr(1);
  }
  else
    _check_command = value;
}

void service::_set_checks_active(bool value) {
  _checks_active = value;
}

void service::_set_checks_passive(bool value) {
  _checks_passive = value;
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
  _contactgroups.set(value);
}

void service::_set_contacts(std::string const& value) {
  _contacts.set(value);
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
  // XXX: service::_set_failure_prediction_enabled
}

void service::_set_failure_prediction_options(std::string const& value) {
  // XXX: service::_set_failure_prediction_options
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
  _hostgroups.set(value);
}

void service::_set_hosts(std::string const& value) {
  _hosts.set(value);
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
  // XXX: service::_set_parallelize_check
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
  _servicegroups.set(value);
}

void service::_set_service_description(std::string const& value) {
  _service_description = value;
}

void service::_set_stalking_options(std::string const& value) {
  _stalking_options = 0; // XXX:
}

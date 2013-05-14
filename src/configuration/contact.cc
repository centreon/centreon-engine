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

#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<contact, type, &contact::method>::generic

static struct {
  std::string const name;
  bool (*func)(contact&, std::string const&);
} gl_setters[] = {
  { "contact_name",                  SETTER(std::string const&, _set_contact_name) },
  { "alias",                         SETTER(std::string const&, _set_alias) },
  { "contact_groups",                SETTER(std::string const&, _set_contactgroups) },
  { "contactgroups",                 SETTER(std::string const&, _set_contactgroups) },
  { "email",                         SETTER(std::string const&, _set_email) },
  { "pager",                         SETTER(std::string const&, _set_email) },
  { "host_notification_period",      SETTER(std::string const&, _set_host_notification_period) },
  { "host_notification_commands",    SETTER(std::string const&, _set_host_notification_commands) },
  { "service_notification_period",   SETTER(std::string const&, _set_service_notification_period) },
  { "service_notification_commands", SETTER(std::string const&, _set_service_notification_commands) },
  { "host_notification_options",     SETTER(std::string const&, _set_host_notification_options) },
  { "service_notification_options",  SETTER(std::string const&, _set_service_notification_options) },
  { "host_notifications_enabled",    SETTER(bool, _set_host_notifications_enabled) },
  { "service_notifications_enabled", SETTER(bool, _set_service_notifications_enabled) },
  { "can_submit_commands",           SETTER(bool, _set_can_submit_commands) },
  { "retain_status_information",     SETTER(bool, _set_retain_status_information) },
  { "retain_nonstatus_information",  SETTER(bool, _set_retain_nonstatus_information) }
};

// Default values.
static bool const         default_can_submit_commands(true);
static bool const         default_host_notifications_enabled(true);
static unsigned int const default_host_notification_options(host::none);
static bool const         default_retain_nonstatus_information(true);
static bool const         default_retain_status_information(true);
static unsigned int const default_service_notification_options(service::none);
static bool const         default_service_notifications_enabled(true);

static unsigned int const MAX_ADDRESSES(6);

/**
 *  Default constructor.
 */
contact::contact()
  : object("contact"),
    _can_submit_commands(default_can_submit_commands),
    _host_notifications_enabled(default_host_notifications_enabled),
    _host_notification_options(default_host_notification_options),
    _retain_nonstatus_information(default_retain_nonstatus_information),
    _retain_status_information(default_retain_status_information),
    _service_notification_options(default_service_notification_options),
    _service_notifications_enabled(default_service_notifications_enabled) {
  _address.reserve(MAX_ADDRESSES);
}

/**
 *  Copy constructor.
 *
 *  @param[in] right The contact to copy.
 */
contact::contact(contact const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
contact::~contact() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The contact to copy.
 *
 *  @return This contact.
 */
contact& contact::operator=(contact const& right) {
  if (this != &right) {
    object::operator=(right);
    _address = right._address;
    _alias = right._alias;
    _can_submit_commands = right._can_submit_commands;
    _contactgroups = right._contactgroups;
    _contact_name = right._contact_name;
    _customvariables = right._customvariables;
    _email = right._email;
    _host_notifications_enabled = right._host_notifications_enabled;
    _host_notification_commands = right._host_notification_commands;
    _host_notification_options = right._host_notification_options;
    _host_notification_period = right._host_notification_period;
    _retain_nonstatus_information = right._retain_nonstatus_information;
    _retain_status_information = right._retain_status_information;
    _pager = right._pager;
    _service_notification_commands = right._service_notification_commands;
    _service_notification_options = right._service_notification_options;
    _service_notification_period = right._service_notification_period;
    _service_notifications_enabled = right._service_notifications_enabled;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The contact to compare.
 *
 *  @return True if is the same contact, otherwise false.
 */
bool contact::operator==(contact const& right) const throw () {
  return (object::operator==(right)
          && _address == right._address
          && _alias == right._alias
          && _can_submit_commands == right._can_submit_commands
          && _contactgroups == right._contactgroups
          && _contact_name == right._contact_name
          // XXX: && _customvariables == right._customvariables
          && _email == right._email
          && _host_notifications_enabled == right._host_notifications_enabled
          && _host_notification_commands == right._host_notification_commands
          && _host_notification_options == right._host_notification_options
          && _host_notification_period == right._host_notification_period
          && _retain_nonstatus_information == right._retain_nonstatus_information
          && _retain_status_information == right._retain_status_information
          && _pager == right._pager
          && _service_notification_commands == right._service_notification_commands
          && _service_notification_options == right._service_notification_options
          && _service_notification_period == right._service_notification_period
          && _service_notifications_enabled == right._service_notifications_enabled);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The contact to compare.
 *
 *  @return True if is not the same contact, otherwise false.
 */
bool contact::operator!=(contact const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void contact::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "XXX: todo");
  contact const& tmpl(static_cast<contact const&>(obj));

  // MERGE_TAB(_address);
  MRG_STRING(_alias);
  MRG_DEFAULT(_can_submit_commands);
  MRG_INHERIT(_contactgroups);
  MRG_STRING(_contact_name);
  MRG_MAP(_customvariables);
  MRG_STRING(_email);
  MRG_DEFAULT(_host_notifications_enabled);
  MRG_INHERIT(_host_notification_commands);
  MRG_DEFAULT(_host_notification_options);
  MRG_STRING(_host_notification_period);
  MRG_DEFAULT(_retain_nonstatus_information);
  MRG_DEFAULT(_retain_status_information);
  MRG_STRING(_pager);
  MRG_INHERIT(_service_notification_commands);
  MRG_DEFAULT(_service_notification_options);
  MRG_STRING(_service_notification_period);
  MRG_DEFAULT(_service_notifications_enabled);
}

/**
 *  Parse and set the contact property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::parse(std::string const& key, std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  if (key.find("address") == 0) {
    _set_address(key, value);
    return (true);
  }
  return (false);
}

void contact::_set_address(
       std::string const& key,
       std::string const& value) {
  // XXX:
}

void contact::_set_alias(std::string const& value) {
  _alias = value;
}

void contact::_set_can_submit_commands(bool value) {
  _can_submit_commands = value;
}

void contact::_set_contactgroups(std::string const& value) {
  _contactgroups.clear();
  misc::split(value, _contactgroups.get(), ',');
}

void contact::_set_contact_name(std::string const& value) {
  _contact_name = value;
}

void contact::_set_email(std::string const& value) {
  _email = value;
}

void contact::_set_host_notifications_enabled(bool value) {
  _host_notifications_enabled = value;
}

void contact::_set_host_notification_commands(std::string const& value) {
  _host_notification_commands.clear();
  misc::split(value, _host_notification_commands.get(), ',');
}

void contact::_set_host_notification_options(std::string const& value) {
  _host_notification_options = 0; // XXX:
}

void contact::_set_host_notification_period(std::string const& value) {
  _host_notification_period = value;
}

void contact::_set_retain_nonstatus_information(bool value) {
  _retain_nonstatus_information = value;
}

void contact::_set_retain_status_information(bool value) {
  _retain_status_information = value;
}

void contact::_set_pager(std::string const& value) {
  _pager = value;
}

void contact::_set_service_notification_commands(std::string const& value) {
  _service_notification_commands.clear();
  misc::split(value, _service_notification_commands.get(), ',');
}

void contact::_set_service_notification_options(std::string const& value) {
  _service_notification_options = 0; // XXX:
}

void contact::_set_service_notification_period(std::string const& value) {
  _service_notification_period = value;
}

void contact::_set_service_notifications_enabled(bool value) {
  _service_notifications_enabled = value;
}

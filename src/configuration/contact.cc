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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine::configuration;

#define setter(type, method) \
  &object::setter<contact, type, &contact::method>::generic

static struct {
  std::string const name;
  bool (*func)(contact&, std::string const&);
} gl_setters[] = {
  { "contact_name",                  setter(std::string const&, _set_contact_name) },
  { "alias",                         setter(std::string const&, _set_alias) },
  { "contact_groups",                setter(std::string const&, _set_contactgroups) },
  { "contactgroups",                 setter(std::string const&, _set_contactgroups) },
  { "email",                         setter(std::string const&, _set_email) },
  { "pager",                         setter(std::string const&, _set_email) },
  { "host_notification_period",      setter(std::string const&, _set_host_notification_period) },
  { "host_notification_commands",    setter(std::string const&, _set_host_notification_commands) },
  { "service_notification_period",   setter(std::string const&, _set_service_notification_period) },
  { "service_notification_commands", setter(std::string const&, _set_service_notification_commands) },
  { "host_notification_options",     setter(std::string const&, _set_host_notification_options) },
  { "service_notification_options",  setter(std::string const&, _set_service_notification_options) },
  { "host_notifications_enabled",    setter(bool, _set_host_notifications_enabled) },
  { "service_notifications_enabled", setter(bool, _set_service_notifications_enabled) },
  { "can_submit_commands",           setter(bool, _set_can_submit_commands) },
  { "retain_status_information",     setter(bool, _set_retain_status_information) },
  { "retain_nonstatus_information",  setter(bool, _set_retain_nonstatus_information) }
};

/**
 *  Default constructor.
 */
contact::contact()
  : object("contact") {

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
  return (object::operator==(right));
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
  return (object::parse(key, value));
}

void contact::_set_alias(std::string const& value) {
  _alias = value;
}

void contact::_set_can_submit_commands(bool value) {
  _can_submit_commands = value;
}

void contact::_set_contactgroups(std::string const& value) {
  _contactgroups.clear();
  misc::split(value, _contactgroups, ',');
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
  _host_notification_commands = value;
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
  _service_notification_commands = value;
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

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

#include "com/centreon/engine/configuration/serviceescalation.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine::configuration;

#define setter(type, method) \
  &object::setter<serviceescalation, type, &serviceescalation::method>::generic

static struct {
  std::string const name;
  bool (*func)(serviceescalation&, std::string const&);
} gl_setters[] = {
  { "host",                  setter(std::string const&, _set_hosts) },
  { "host_name",             setter(std::string const&, _set_hosts) },
  { "description",           setter(std::string const&, _set_service_description) },
  { "service_description",   setter(std::string const&, _set_service_description) },
  { "servicegroup",          setter(std::string const&, _set_servicegroups) },
  { "servicegroups",         setter(std::string const&, _set_servicegroups) },
  { "servicegroup_name",     setter(std::string const&, _set_servicegroups) },
  { "hostgroup",             setter(std::string const&, _set_hostgroups) },
  { "hostgroups",            setter(std::string const&, _set_hostgroups) },
  { "hostgroup_name",        setter(std::string const&, _set_hostgroups) },
  { "contact_groups",        setter(std::string const&, _set_contactgroups) },
  { "contacts",              setter(std::string const&, _set_contacts) },
  { "escalation_period",     setter(std::string const&, _set_escalation_period) },
  { "first_notification",    setter(unsigned int, _set_first_notification) },
  { "last_notification",     setter(unsigned int, _set_last_notification) },
  { "notification_interval", setter(unsigned int, _set_notification_interval) },
  { "escalation_options",    setter(std::string const&, _set_escalation_options) }
};

/**
 *  Default constructor.
 */
serviceescalation::serviceescalation()
  : object("serviceescalation") {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The serviceescalation to copy.
 */
serviceescalation::serviceescalation(serviceescalation const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
serviceescalation::~serviceescalation() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The serviceescalation to copy.
 *
 *  @return This serviceescalation.
 */
serviceescalation& serviceescalation::operator=(serviceescalation const& right) {
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
 *  @param[in] right The serviceescalation to compare.
 *
 *  @return True if is the same serviceescalation, otherwise false.
 */
bool serviceescalation::operator==(serviceescalation const& right) const throw () {
  return (object::operator==(right)
          && _hosts == right._hosts
          && _service_description == right._service_description);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The serviceescalation to compare.
 *
 *  @return True if is not the same serviceescalation, otherwise false.
 */
bool serviceescalation::operator!=(serviceescalation const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Parse and set the serviceescalation property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::parse(
       std::string const& key,
       std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  return (object::parse(key, value));
}

void serviceescalation::_set_contactgroups(std::string const& value) {
  _contactgroups.clear();
  misc::split(value, _contactgroups, ',');
}

void serviceescalation::_set_contacts(std::string const& value) {
  _contacts.clear();
  misc::split(value, _contacts, ',');
}

void serviceescalation::_set_escalation_options(std::string const& value) {
  _escalation_options = 0; // XXX:
}

void serviceescalation::_set_escalation_period(std::string const& value) {
  _escalation_period = value;
}

void serviceescalation::_set_first_notification(unsigned int value) {
  _first_notification = value;
}

void serviceescalation::_set_hostgroups(std::string const& value) {
  _hostgroups.clear();
  misc::split(value, _hostgroups, ',');
}

void serviceescalation::_set_hosts(std::string const& value) {
  _hosts.clear();
  misc::split(value, _hosts, ',');
}

void serviceescalation::_set_last_notification(unsigned int value) {
  _last_notification = value;
}

void serviceescalation::_set_notification_interval(unsigned int value) {
  _notification_interval = value;
}

void serviceescalation::_set_servicegroups(std::string const& value) {
  _servicegroups.clear();
  misc::split(value, _servicegroups, ',');
}

void serviceescalation::_set_service_description(std::string const& value) {
  _service_description = value;
}

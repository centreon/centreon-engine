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

#include "com/centreon/engine/configuration/hostescalation.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"

using namespace com::centreon::engine::configuration;

#define setter(type, method) \
  &object::setter<hostescalation, type, &hostescalation::method>::generic

static struct {
  std::string const name;
  bool (*func)(hostescalation&, std::string const&);
} gl_setters[] = {
  { "hostgroup",             setter(std::string const&, _set_hostgroups) },
  { "hostgroups",            setter(std::string const&, _set_hostgroups) },
  { "hostgroup_name",        setter(std::string const&, _set_hostgroups) },
  { "host",                  setter(std::string const&, _set_hosts) },
  { "host_name",             setter(std::string const&, _set_hosts) },
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
hostescalation::hostescalation()
  : object("hostescalation") {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The hostescalation to copy.
 */
hostescalation::hostescalation(hostescalation const& right)
  : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
hostescalation::~hostescalation() throw () {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right The hostescalation to copy.
 *
 *  @return This hostescalation.
 */
hostescalation& hostescalation::operator=(hostescalation const& right) {
  if (this != &right) {
    object::operator=(right);
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The hostescalation to compare.
 *
 *  @return True if is the same hostescalation, otherwise false.
 */
bool hostescalation::operator==(hostescalation const& right) const throw () {
  return (object::operator==(right));
}

/**
 *  Equal operator.
 *
 *  @param[in] right The hostescalation to compare.
 *
 *  @return True if is not the same hostescalation, otherwise false.
 */
bool hostescalation::operator!=(hostescalation const& right) const throw () {
  return (!operator==(right));
}

/**
 *  Parse and set the hostescalation property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool hostescalation::parse(
       std::string const& key,
       std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  return (object::parse(key, value));
}

void hostescalation::_set_contactgroups(std::string const& value) {
  _contactgroups.clear();
  misc::split(value, _contactgroups, ',');
}

void hostescalation::_set_contacts(std::string const& value) {
  _contacts.clear();
  misc::split(value, _contacts, ',');
}

void hostescalation::_set_escalation_options(std::string const& value) {
  _escalation_options = value; // XXX:
}

void hostescalation::_set_escalation_period(std::string const& value) {
  _escalation_period = value;
}

void hostescalation::_set_first_notification(unsigned int value) {
  _first_notification = value;
}

void hostescalation::_set_hostgroups(std::string const& value) {
  _hostgroups.clear();
  misc::split(value, _hostgroups, ',');
}

void hostescalation::_set_hosts(std::string const& value) {
  _hosts.clear();
  misc::split(value, _hosts, ',');
}

void hostescalation::_set_last_notification(unsigned int value) {
  _last_notification = value;
}

void hostescalation::_set_notification_interval(unsigned int value) {
  _notification_interval = value;
}

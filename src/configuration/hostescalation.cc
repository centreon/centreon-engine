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

using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<hostescalation, type, &hostescalation::method>::generic

static struct {
  std::string const name;
  bool (*func)(hostescalation&, std::string const&);
} gl_setters[] = {
  { "hostgroup",             SETTER(std::string const&, _set_hostgroups) },
  { "hostgroups",            SETTER(std::string const&, _set_hostgroups) },
  { "hostgroup_name",        SETTER(std::string const&, _set_hostgroups) },
  { "host",                  SETTER(std::string const&, _set_hosts) },
  { "host_name",             SETTER(std::string const&, _set_hosts) },
  { "contact_groups",        SETTER(std::string const&, _set_contactgroups) },
  { "contacts",              SETTER(std::string const&, _set_contacts) },
  { "escalation_period",     SETTER(std::string const&, _set_escalation_period) },
  { "first_notification",    SETTER(unsigned int, _set_first_notification) },
  { "last_notification",     SETTER(unsigned int, _set_last_notification) },
  { "notification_interval", SETTER(unsigned int, _set_notification_interval) },
  { "escalation_options",    SETTER(std::string const&, _set_escalation_options) }
};

// Default values.
static unsigned int const default_first_notification(-2);
static unsigned int const default_last_notification(-2);
static unsigned int const default_notification_interval(0);

/**
 *  Default constructor.
 */
hostescalation::hostescalation()
  : object("hostescalation"),
    _first_notification(default_first_notification),
    _last_notification(default_last_notification),
    _notification_interval(default_notification_interval) {

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
    _contactgroups = right._contactgroups;
    _contacts = right._contacts;
    _escalation_options = right._escalation_options;
    _escalation_period = right._escalation_period;
    _first_notification = right._first_notification;
    _hostgroups = right._hostgroups;
    _hosts = right._hosts;
    _last_notification = right._last_notification;
    _notification_interval = right._notification_interval;
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
  return (object::operator==(right)
          && _contactgroups == right._contactgroups
          && _contacts == right._contacts
          && _escalation_options == right._escalation_options
          && _escalation_period == right._escalation_period
          && _first_notification == right._first_notification
          && _hostgroups == right._hostgroups
          && _hosts == right._hosts
          && _last_notification == right._last_notification
          && _notification_interval == right._notification_interval);
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
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void hostescalation::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
  hostescalation const& tmpl(static_cast<hostescalation const&>(obj));

  MRG_INHERIT(_contactgroups);
  MRG_INHERIT(_contacts);
  MRG_STRING(_escalation_options);
  MRG_STRING(_escalation_period);
  MRG_DEFAULT(_first_notification);
  MRG_INHERIT(_hostgroups);
  MRG_INHERIT(_hosts);
  MRG_DEFAULT(_last_notification);
  MRG_DEFAULT(_notification_interval);
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
  return (false);
}

void hostescalation::_set_contactgroups(std::string const& value) {
  _contactgroups.set(value);
}

void hostescalation::_set_contacts(std::string const& value) {
  _contacts.set(value);
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
  _hostgroups.set(value);
}

void hostescalation::_set_hosts(std::string const& value) {
  _hosts.set(value);
}

void hostescalation::_set_last_notification(unsigned int value) {
  _last_notification = value;
}

void hostescalation::_set_notification_interval(unsigned int value) {
  _notification_interval = value;
}

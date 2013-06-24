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
#include "com/centreon/hash.hh"

using namespace com::centreon;
using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<serviceescalation, type, &serviceescalation::method>::generic

static struct {
  std::string const name;
  bool (*func)(serviceescalation&, std::string const&);
} gl_setters[] = {
  { "host",                  SETTER(std::string const&, _set_hosts) },
  { "host_name",             SETTER(std::string const&, _set_hosts) },
  { "description",           SETTER(std::string const&, _set_service_description) },
  { "service_description",   SETTER(std::string const&, _set_service_description) },
  { "servicegroup",          SETTER(std::string const&, _set_servicegroups) },
  { "servicegroups",         SETTER(std::string const&, _set_servicegroups) },
  { "servicegroup_name",     SETTER(std::string const&, _set_servicegroups) },
  { "hostgroup",             SETTER(std::string const&, _set_hostgroups) },
  { "hostgroups",            SETTER(std::string const&, _set_hostgroups) },
  { "hostgroup_name",        SETTER(std::string const&, _set_hostgroups) },
  { "contact_groups",        SETTER(std::string const&, _set_contactgroups) },
  { "contacts",              SETTER(std::string const&, _set_contacts) },
  { "escalation_options",    SETTER(std::string const&, _set_escalation_options) },
  { "escalation_period",     SETTER(std::string const&, _set_escalation_period) },
  { "first_notification",    SETTER(unsigned int, _set_first_notification) },
  { "last_notification",     SETTER(unsigned int, _set_last_notification) },
  { "notification_interval", SETTER(unsigned int, _set_notification_interval) }
};

// Default values.
static unsigned short const default_escalation_options(serviceescalation::none);
static unsigned int const   default_first_notification(-2);
static unsigned int const   default_last_notification(-2);
static unsigned int const   default_notification_interval(0);

/**
 *  Default constructor.
 */
serviceescalation::serviceescalation()
  : object(object::serviceescalation),
    _escalation_options(default_escalation_options),
    _first_notification(default_first_notification),
    _last_notification(default_last_notification),
    _notification_interval(default_notification_interval) {

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
    _contactgroups = right._contactgroups;
    _contacts = right._contacts;
    _escalation_options = right._escalation_options;
    _escalation_period = right._escalation_period;
    _first_notification = right._first_notification;
    _hostgroups = right._hostgroups;
    _hosts = right._hosts;
    _last_notification = right._last_notification;
    _notification_interval = right._notification_interval;
    _servicegroups = right._servicegroups;
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
          && _contactgroups == right._contactgroups
          && _contacts == right._contacts
          && _escalation_options == right._escalation_options
          && _escalation_period == right._escalation_period
          && _first_notification == right._first_notification
          && _hostgroups == right._hostgroups
          && _hosts == right._hosts
          && _last_notification == right._last_notification
          && _notification_interval == right._notification_interval
          && _servicegroups == right._servicegroups
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
 *  Get the unique object id.
 *
 *  @return The object id.
 */
std::size_t serviceescalation::id() const throw () {
  if (!_id) {
    hash_combine(_id, _hosts.get().begin(), _hosts.get().end());
    hash_combine(
      _id,
      _hostgroups.get().begin(),
      _hostgroups.get().end());
    hash_combine(
      _id,
      _service_description.get().begin(),
      _service_description.get().end());
    hash_combine(
      _id,
      _servicegroups.get().begin(),
      _servicegroups.get().end());
  }
  return (_id);
}

/**
 *  Check if the object is valid.
 *
 *  @return True if is a valid object, otherwise false.
 */
void serviceescalation::check_validity() const {
  if (_service_description.empty() && _servicegroups.empty())
    throw (engine_error() << "configuration: invalid serviceescalation "
           "property service_description or servicegroup is missing");
  if (_hosts.empty() && _hostgroups.empty())
    throw (engine_error() << "configuration: invalid serviceescalation "
           "property host or hostgroup is missing");
  if (_contacts.empty() && _contactgroups.empty())
    throw (engine_error() << "configuration: invalid serviceescalation "
           "property contact or contactgroup is missing");
  if (_escalation_period.empty())
    throw (engine_error() << "configuration: invalid serviceescalation "
           "property escalation_period is missing");
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void serviceescalation::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
  serviceescalation const& tmpl(static_cast<serviceescalation const&>(obj));

  MRG_INHERIT(_contactgroups);
  MRG_INHERIT(_contacts);
  MRG_OPTION(_escalation_options);
  MRG_DEFAULT(_escalation_period);
  MRG_OPTION(_first_notification);
  MRG_INHERIT(_hostgroups);
  MRG_INHERIT(_hosts);
  MRG_OPTION(_last_notification);
  MRG_OPTION(_notification_interval);
  MRG_INHERIT(_servicegroups);
  MRG_INHERIT(_service_description);
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
  return (false);
}

/**
 *  Get contactgroups.
 *
 *  @return The contactgroups.
 */
list_string const& serviceescalation::contactgroups() const throw () {
  return (_contactgroups.get());
}

/**
 *  Get contacts.
 *
 *  @return The contacts.
 */
list_string const& serviceescalation::contacts() const throw () {
  return (_contacts.get());
}

/**
 *  Get escalation_options.
 *
 *  @return The escalation_options.
 */
unsigned short serviceescalation::escalation_options() const throw () {
  return (_escalation_options);
}

/**
 *  Get escalation_period.
 *
 *  @return The escalation_period.
 */
std::string const& serviceescalation::escalation_period() const throw () {
  return (_escalation_period);
}

/**
 *  Get first_notification.
 *
 *  @return The first_notification.
 */
unsigned int serviceescalation::first_notification() const throw () {
  return (_first_notification);
}

/**
 *  Get hostgroups.
 *
 *  @return The hostgroups.
 */
list_string const& serviceescalation::hostgroups() const throw () {
  return (_hostgroups.get());
}

/**
 *  Get hosts.
 *
 *  @return The hosts.
 */
list_string const& serviceescalation::hosts() const throw () {
  return (_hosts.get());
}

/**
 *  Get last_notification.
 *
 *  @return The last_notification.
 */
unsigned int serviceescalation::last_notification() const throw () {
  return (_last_notification);
}

/**
 *  Get notification_interval.
 *
 *  @return The notification_interval.
 */
unsigned int serviceescalation::notification_interval() const throw () {
  return (_notification_interval);
}

/**
 *  Get servicegroups.
 *
 *  @return The servicegroups.
 */
list_string const& serviceescalation::servicegroups() const throw () {
  return (_servicegroups.get());
}

/**
 *  Get service_description.
 *
 *  @return The service_description.
 */
list_string const& serviceescalation::service_description() const throw () {
  return (_service_description.get());
}


/**
 *  Set contactgroups value.
 *
 *  @param[in] value The new contactgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_contactgroups(std::string const& value) {
  _contactgroups.set(value);
  return (true);
}

/**
 *  Set contacts value.
 *
 *  @param[in] value The new contacts value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_contacts(std::string const& value) {
  _contacts.set(value);
  return (true);
}

/**
 *  Set escalation_options value.
 *
 *  @param[in] value The new escalation_options value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_escalation_options(std::string const& value) {
  unsigned short options(none);
  std::list<std::string> values;
  misc::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    misc::trim(*it);
    if (*it == "w" || *it == "warning")
      options |= warning;
    else if (*it == "u" || *it == "unknown")
      options |= unknown;
    else if (*it == "c" || *it == "critical")
      options |= critical;
    else if (*it == "r" || *it == "recovery")
      options |= recovery;
    else if (*it == "n" || *it == "none")
      options = none;
    else if (*it == "a" || *it == "all")
      options = warning | unknown | critical | recovery;
    else
      return (false);
  }
  _escalation_options = options;
  return (true);
}

/**
 *  Set escalation_period value.
 *
 *  @param[in] value The new escalation_period value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_escalation_period(std::string const& value) {
  _escalation_period = value;
  return (true);
}

/**
 *  Set first_notification value.
 *
 *  @param[in] value The new first_notification value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_first_notification(unsigned int value) {
  _first_notification = value;
  return (true);
}

/**
 *  Set hostgroups value.
 *
 *  @param[in] value The new hostgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_hostgroups(std::string const& value) {
  _hostgroups.set(value);
  _id = 0;
  return (true);
}

/**
 *  Set hosts value.
 *
 *  @param[in] value The new hosts value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_hosts(std::string const& value) {
  _hosts.set(value);
  _id = 0;
  return (true);
}

/**
 *  Set last_notification value.
 *
 *  @param[in] value The new last_notification value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_last_notification(unsigned int value) {
  _last_notification = value;
  return (true);
}

/**
 *  Set notification_interval value.
 *
 *  @param[in] value The new notification_interval value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_notification_interval(unsigned int value) {
  _notification_interval = value;
  return (true);
}

/**
 *  Set servicegroups value.
 *
 *  @param[in] value The new servicegroups value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_servicegroups(std::string const& value) {
  _servicegroups.set(value);
  _id = 0;
  return (true);
}

/**
 *  Set service_description value.
 *
 *  @param[in] value The new service_description value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_service_description(std::string const& value) {
  _service_description.set(value);
  _id = 0;
  return (true);
}

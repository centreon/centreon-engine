/*
** Copyright 2011-2015,2017 Centreon
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
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<serviceescalation, type, &serviceescalation::method>::generic

std::unordered_map<std::string, serviceescalation::setter_func> const serviceescalation::_setters{
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
    _notification_interval(default_notification_interval) {}

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
serviceescalation::~serviceescalation() throw () {}

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
  return *this;
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
  return !operator==(right);
}

/**
 *  Less-than operator.
 *
 *  @param[in] right Object to compare to.
 *
 *  @return True if this object is less than right.
 */
bool serviceescalation::operator<(serviceescalation const& right) const {
  if (_hosts != right._hosts)
    return _hosts < right._hosts;
  else if (_hostgroups != right. _hostgroups)
    return _hostgroups < right._hostgroups;
  else if (_service_description != right._service_description)
    return _service_description < right._service_description;
  else if (_servicegroups != right._servicegroups)
    return _servicegroups < right._servicegroups;
  else if (_contacts != right._contacts)
    return _contacts < right._contacts;
  else if (_contactgroups != right._contactgroups)
    return _contactgroups < right._contactgroups;
  else if (_escalation_options != right._escalation_options)
    return _escalation_options < right._escalation_options;
  else if (_escalation_period != right._escalation_period)
    return _escalation_period < right._escalation_period;
  else if (_first_notification != right._first_notification)
    return _first_notification < right._first_notification;
  else if (_last_notification != right._last_notification)
    return _last_notification < right._last_notification;
  return _notification_interval < right._notification_interval;
}

/**
 *  @brief Check if the object is valid.
 *
 *  If the object is not valid, an exception is thrown.
 */
void serviceescalation::check_validity() const {
  if (_servicegroups->empty()) {
    if (_service_description->empty())
      throw (engine_error() << "Service escalation is not attached to "
             << "any service or service group (properties "
             << "'service_description' and 'servicegroup_name', "
             << "respectively)");
    else if (_hosts->empty() && _hostgroups->empty())
      throw (engine_error() << "Service escalation is not attached to "
             << "any host or host group (properties 'host_name' or "
             << "'hostgroup_name', respectively)");
  }
  return ;
}

/**
 *  Get the service escalation key.
 *
 *  @return This object.
 */
serviceescalation::key_type const& serviceescalation::key() const throw () {
  return *this;
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void serviceescalation::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "Cannot merge service escalation with '"
           << obj.type() << "'");
  serviceescalation const& tmpl(static_cast<serviceescalation const&>(obj));

  MRG_INHERIT(_contactgroups);
  MRG_INHERIT(_contacts);
  MRG_OPTION(_escalation_options);
  MRG_OPTION(_escalation_period);
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
bool serviceescalation::parse(char const* key, char const* value) {
  std::unordered_map<std::string, serviceescalation::setter_func>::const_iterator
    it{_setters.find(key)};
  if (it != _setters.end())
    return (it->second)(*this, value);
  return false;
}

/**
 *  Get contact groups.
 *
 *  @return The contact groups.
 */
set_string& serviceescalation::contactgroups() throw () {
  return *_contactgroups;
}

/**
 *  Get contactgroups.
 *
 *  @return The contactgroups.
 */
set_string const& serviceescalation::contactgroups() const throw () {
  return *_contactgroups;
}

/**
 *  Check if contact groups were defined.
 *
 *  @return True if contact groups were defined.
 */
bool serviceescalation::contactgroups_defined() const throw () {
  return _contactgroups.is_set();
}

/**
 *  Get contacts.
 *
 *  @return The contacts;
 */
set_string& serviceescalation::contacts() throw () {
  return *_contacts;
}

/**
 *  Get contacts.
 *
 *  @return The contacts.
 */
set_string const& serviceescalation::contacts() const throw () {
  return *_contacts;
}

/**
 *  Check if contacts were defined.
 *
 *  @return True if contacts were defined.
 */
bool serviceescalation::contacts_defined() const throw () {
  return _contacts.is_set();
}

/**
 *  Set escalation options.
 *
 *  @param[in] options New escalation options.
 */
void serviceescalation::escalation_options(
                          unsigned int options) throw () {
  _escalation_options = options;
  return ;
}

/**
 *  Get escalation_options.
 *
 *  @return The escalation_options.
 */
unsigned short serviceescalation::escalation_options() const throw () {
  return _escalation_options;
}

/**
 *  Set the escalation period.
 *
 *  @param[in] period New escalation period.
 */
void serviceescalation::escalation_period(std::string const& period) {
  _escalation_period = period;
  return ;
}

/**
 *  Get escalation_period.
 *
 *  @return The escalation_period.
 */
std::string const& serviceescalation::escalation_period() const throw () {
  return _escalation_period;
}

/**
 *  Check if escalation period was defined.
 *
 *  @return True if the escalation period was defined.
 */
bool serviceescalation::escalation_period_defined() const throw () {
  return _escalation_period.is_set();
}

/**
 *  Set the first notification number.
 *
 *  @param[in] n First notification number.
 */
void serviceescalation::first_notification(unsigned int n) throw () {
  _first_notification = n;
  return ;
}

/**
 *  Get first_notification.
 *
 *  @return The first_notification.
 */
unsigned int serviceescalation::first_notification() const throw () {
  return _first_notification;
}

/**
 *  Get host groups.
 *
 *  @return Host groups.
 */
list_string& serviceescalation::hostgroups() throw () {
  return *_hostgroups;
}

/**
 *  Get hostgroups.
 *
 *  @return The hostgroups.
 */
list_string const& serviceescalation::hostgroups() const throw () {
  return *_hostgroups;
}

/**
 *  Get hosts.
 *
 *  @return The hosts.
 */
list_string& serviceescalation::hosts() throw () {
  return *_hosts;
}

/**
 *  Get hosts.
 *
 *  @return The hosts.
 */
list_string const& serviceescalation::hosts() const throw () {
  return *_hosts;
}

/**
 *  Set the last notification number.
 *
 *  @param[in] n Last notification number.
 */
void serviceescalation::last_notification(unsigned int n) throw () {
  _last_notification = n;
  return ;
}

/**
 *  Get last_notification.
 *
 *  @return The last_notification.
 */
unsigned int serviceescalation::last_notification() const throw () {
  return _last_notification;
}

/**
 *  Set the notification interval.
 *
 *  @param[in] interval New notification interval.
 */
void serviceescalation::notification_interval(
                          unsigned int interval) throw () {
  _notification_interval = interval;
  return ;
}

/**
 *  Get notification_interval.
 *
 *  @return The notification_interval.
 */
unsigned int serviceescalation::notification_interval() const throw () {
  return _notification_interval;
}

/**
 *  Check if notification interval was set.
 *
 *  @return True if the notification interval was set.
 */
bool serviceescalation::notification_interval_defined() const throw () {
  return _notification_interval.is_set();
}

/**
 *  Get service groups.
 *
 *  @return The service groups.
 */
list_string& serviceescalation::servicegroups() throw () {
  return *_servicegroups;
}

/**
 *  Get servicegroups.
 *
 *  @return The servicegroups.
 */
list_string const& serviceescalation::servicegroups() const throw () {
  return *_servicegroups;
}

/**
 *  Get service description.
 *
 *  @return Service description.
 */
list_string& serviceescalation::service_description() throw () {
  return *_service_description;
}

/**
 *  Get service_description.
 *
 *  @return The service_description.
 */
list_string const& serviceescalation::service_description() const throw () {
  return *_service_description;
}


/**
 *  Set contactgroups value.
 *
 *  @param[in] value The new contactgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_contactgroups(std::string const& value) {
  _contactgroups = value;
  return true;
}

/**
 *  Set contacts value.
 *
 *  @param[in] value The new contacts value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_contacts(std::string const& value) {
  _contacts = value;
  return true;
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
  string::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    string::trim(*it);
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
      return false;
  }
  _escalation_options = options;
  return true;
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
  return true;
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
  return true;
}

/**
 *  Set hostgroups value.
 *
 *  @param[in] value The new hostgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_hostgroups(std::string const& value) {
  _hostgroups = value;
  return true;
}

/**
 *  Set hosts value.
 *
 *  @param[in] value The new hosts value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_hosts(std::string const& value) {
  _hosts = value;
  return true;
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
  return true;
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
  return true;
}

/**
 *  Set servicegroups value.
 *
 *  @param[in] value The new servicegroups value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_servicegroups(std::string const& value) {
  _servicegroups = value;
  return true;
}

/**
 *  Set service_description value.
 *
 *  @param[in] value The new service_description value.
 *
 *  @return True on success, otherwise false.
 */
bool serviceescalation::_set_service_description(std::string const& value) {
  _service_description = value;
  return true;
}

/*
** Copyright 2011-2015 Merethis
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
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

#define SETTER(type, method) \
  &object::setter<contact, type, &contact::method>::generic
#define ADDRESS_PROPERTY "address"

std::unordered_map<std::string, contact::setter_func> const contact::_setters{
  { "contact_name",                  SETTER(std::string const&, _set_contact_name) },
  { "alias",                         SETTER(std::string const&, _set_alias) },
  { "contact_groups",                SETTER(std::string const&, _set_contactgroups) },
  { "contactgroups",                 SETTER(std::string const&, _set_contactgroups) },
  { "email",                         SETTER(std::string const&, _set_email) },
  { "pager",                         SETTER(std::string const&, _set_pager) },
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
  { "retain_nonstatus_information",  SETTER(bool, _set_retain_nonstatus_information) },
  { "timezone",                      SETTER(std::string const&, _set_timezone) }
};

// Default values.
static bool const           default_can_submit_commands(true);
static bool const           default_host_notifications_enabled(true);
static unsigned short const default_host_notification_options(host::none);
static bool const           default_retain_nonstatus_information(true);
static bool const           default_retain_status_information(true);
static unsigned short const default_service_notification_options(service::none);
static bool const           default_service_notifications_enabled(true);

static unsigned int const   MAX_ADDRESSES(6);

/**
 *  Constructor.
 *
 *  @param[in] key The object key.
 */
contact::contact(key_type const& key)
  : object(object::contact),
    _can_submit_commands(default_can_submit_commands),
    _contact_name(key),
    _host_notifications_enabled(default_host_notifications_enabled),
    _host_notification_options(default_host_notification_options),
    _retain_nonstatus_information(default_retain_nonstatus_information),
    _retain_status_information(default_retain_status_information),
    _service_notification_options(default_service_notification_options),
    _service_notifications_enabled(default_service_notifications_enabled) {
  _address.resize(MAX_ADDRESSES);
}

/**
 *  Copy constructor.
 *
 *  @param[in] other  The contact to copy.
 */
contact::contact(contact const& other) : object(other) {
  operator=(other);
}

/**
 *  Destructor.
 */
contact::~contact() throw () {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  The contact to copy.
 *
 *  @return This contact.
 */
contact& contact::operator=(contact const& other) {
  if (this != &other) {
    object::operator=(other);
    _address = other._address;
    _alias = other._alias;
    _can_submit_commands = other._can_submit_commands;
    _contactgroups = other._contactgroups;
    _contact_name = other._contact_name;
    _customvariables = other._customvariables;
    _email = other._email;
    _host_notifications_enabled = other._host_notifications_enabled;
    _host_notification_commands = other._host_notification_commands;
    _host_notification_options = other._host_notification_options;
    _host_notification_period = other._host_notification_period;
    _retain_nonstatus_information = other._retain_nonstatus_information;
    _retain_status_information = other._retain_status_information;
    _pager = other._pager;
    _service_notification_commands = other._service_notification_commands;
    _service_notification_options = other._service_notification_options;
    _service_notification_period = other._service_notification_period;
    _service_notifications_enabled = other._service_notifications_enabled;
    _timezone = other._timezone;
  }
  return *this;
}

/**
 *  Equality operator.
 *
 *  @param[in] other  The contact to compare.
 *
 *  @return True if is the same contact, otherwise false.
 */
bool contact::operator==(contact const& other) const throw () {
  return (object::operator==(other)
          && _address == other._address
          && _alias == other._alias
          && _can_submit_commands == other._can_submit_commands
          && _contactgroups == other._contactgroups
          && _contact_name == other._contact_name
          && std::operator==(_customvariables, other._customvariables)
          && _email == other._email
          && _host_notifications_enabled == other._host_notifications_enabled
          && _host_notification_commands == other._host_notification_commands
          && _host_notification_options == other._host_notification_options
          && _host_notification_period == other._host_notification_period
          && _retain_nonstatus_information == other._retain_nonstatus_information
          && _retain_status_information == other._retain_status_information
          && _pager == other._pager
          && _service_notification_commands == other._service_notification_commands
          && _service_notification_options == other._service_notification_options
          && _service_notification_period == other._service_notification_period
          && _service_notifications_enabled == other._service_notifications_enabled
          && _timezone == other._timezone);
}

/**
 *  Ineqality operator.
 *
 *  @param[in] other  The contact to compare.
 *
 *  @return True if is not the same contact, otherwise false.
 */
bool contact::operator!=(contact const& other) const throw () {
  return !operator==(other);
}

/**
 *  Less-than operator.
 *
 *  @param[in] other  Object to compare to.
 *
 *  @return True if this object is less than other.
 */
bool contact::operator<(contact const& other) const throw () {
  if (_contact_name != other._contact_name)
    return _contact_name < other._contact_name;
  else if (_address != other._address)
    return _address < other._address;
  else if (_alias != other._alias)
    return _alias < other._alias;
  else if (_can_submit_commands != other._can_submit_commands)
    return _can_submit_commands < other._can_submit_commands;
  else if (_contactgroups != other._contactgroups)
    return _contactgroups < other._contactgroups;
  else if (_customvariables != other._customvariables)
    return _customvariables.size() < other._customvariables.size();
  else if (_email != other._email)
    return _email < other._email;
  else if (_host_notifications_enabled
           != other._host_notifications_enabled)
    return (_host_notifications_enabled
            < other._host_notifications_enabled);
  else if (_host_notification_commands
           != other._host_notification_commands)
    return (_host_notification_commands
            < other._host_notification_commands);
  else if (_host_notification_options
           != other._host_notification_options)
    return (_host_notification_options
            < other._host_notification_options);
  else if (_host_notification_period
           != other._host_notification_period)
    return (_host_notification_period
            < other._host_notification_period);
  else if (_retain_nonstatus_information
           != other._retain_nonstatus_information)
    return (_retain_nonstatus_information
            < other._retain_nonstatus_information);
  else if (_retain_status_information
           != other._retain_status_information)
    return (_retain_status_information
            < other._retain_status_information);
  else if (_pager != other._pager)
    return _pager < other._pager;
  else if (_service_notification_commands
           != other._service_notification_commands)
    return (_service_notification_commands
            < other._service_notification_commands);
  else if (_service_notification_options
           != other._service_notification_options)
    return (_service_notification_options
            < other._service_notification_options);
  else if (_service_notification_period
           != other._service_notification_period)
    return (_service_notification_period
            < other._service_notification_period);
  else if (_service_notifications_enabled
           != other._service_notifications_enabled)
    return (_service_notifications_enabled
            < other._service_notifications_enabled);
  return _timezone < other._timezone;
}

/**
 *  @brief Check if the object is valid.
 *
 *  If the object is not valid, an exception is thrown.
 */
void contact::check_validity() const {
  if (_contact_name.empty())
    throw (engine_error()
           << "Contact has no name (property 'contact_name')");
  return ;
}

/**
 *  Get contact key.
 *
 *  @return Contact name.
 */
contact::key_type const& contact::key() const throw () {
  return _contact_name;
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void contact::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "Cannot merge contact with '"
           << obj.type() << "'");
  contact const& tmpl(static_cast<contact const&>(obj));

  MRG_TAB(_address);
  MRG_DEFAULT(_alias);
  MRG_OPTION(_can_submit_commands);
  MRG_INHERIT(_contactgroups);
  MRG_DEFAULT(_contact_name);
  MRG_MAP(_customvariables);
  MRG_DEFAULT(_email);
  MRG_OPTION(_host_notifications_enabled);
  MRG_INHERIT(_host_notification_commands);
  MRG_OPTION(_host_notification_options);
  MRG_DEFAULT(_host_notification_period);
  MRG_OPTION(_retain_nonstatus_information);
  MRG_OPTION(_retain_status_information);
  MRG_DEFAULT(_pager);
  MRG_INHERIT(_service_notification_commands);
  MRG_OPTION(_service_notification_options);
  MRG_DEFAULT(_service_notification_period);
  MRG_OPTION(_service_notifications_enabled);
  MRG_OPTION(_timezone);
}

/**
 *  Parse and set the contact property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::parse(char const* key, char const* value) {
  std::unordered_map<std::string, contact::setter_func>::const_iterator
    it{_setters.find(key)};
  if (it != _setters.end())
    return (it->second)(*this, value);
  if (!strncmp(key, ADDRESS_PROPERTY, sizeof(ADDRESS_PROPERTY) - 1))
    return _set_address(key + sizeof(ADDRESS_PROPERTY) - 1, value);
  else if (key[0] == '_') {
    std::unordered_map<std::string, customvariable>::iterator it(_customvariables.find(key + 1));
    if (it == _customvariables.end())
      _customvariables.insert({key + 1, customvariable(value)});
    else
      it->second.set_value(value);

    return true;
  }
  return false;
}

/**
 *  Get address.
 *
 *  @return The address.
 */
tab_string const& contact::address() const throw () {
  return _address;
}

/**
 *  Get alias.
 *
 *  @return The alias.
 */
std::string const& contact::alias() const throw () {
  return _alias;
}

/**
 *  Get can_submit_commands.
 *
 *  @return The can_submit_commands.
 */
bool contact::can_submit_commands() const throw () {
  return _can_submit_commands;
}

/**
 *  Get contact groups.
 *
 *  @return The contact groups.
 */
set_string& contact::contactgroups() throw () {
  return *_contactgroups;
}

/**
 *  Get contactgroups.
 *
 *  @return The contactgroups.
 */
set_string const& contact::contactgroups() const throw () {
  return *_contactgroups;
}

/**
 *  Get contact_name.
 *
 *  @return The contact_name.
 */
std::string const& contact::contact_name() const throw () {
  return _contact_name;
}

/**
 *  Get customvariables.
 *
 *  @return The customvariables.
 */
map_customvar const& contact::customvariables() const throw () {
  return _customvariables;
}

/**
 *  Get email.
 *
 *  @return The email.
 */
std::string const& contact::email() const throw () {
  return _email;
}

/**
 *  Get host_notifications_enabled.
 *
 *  @return The host_notifications_enabled.
 */
bool contact::host_notifications_enabled() const throw () {
  return _host_notifications_enabled;
}

/**
 *  Get host_notification_commands.
 *
 *  @return The host_notification_commands.
 */
list_string const& contact::host_notification_commands() const throw () {
  return *_host_notification_commands;
}

/**
 *  Get host_notification_options.
 *
 *  @return The host_notification_options.
 */
unsigned int contact::host_notification_options() const throw () {
  return _host_notification_options;
}

/**
 *  Get host_notification_period.
 *
 *  @return The host_notification_period.
 */
std::string const& contact::host_notification_period() const throw () {
  return _host_notification_period;
}

/**
 *  Get retain_nonstatus_information.
 *
 *  @return The retain_nonstatus_information.
 */
bool contact::retain_nonstatus_information() const throw () {
  return _retain_nonstatus_information;
}

/**
 *  Get retain_status_information.
 *
 *  @return The retain_status_information.
 */
bool contact::retain_status_information() const throw () {
  return _retain_status_information;
}

/**
 *  Get pager.
 *
 *  @return The pager.
 */
std::string const& contact::pager() const throw () {
  return _pager;
}

/**
 *  Get service_notification_commands.
 *
 *  @return The service_notification_commands.
 */
list_string const& contact::service_notification_commands() const throw () {
  return *_service_notification_commands;
}

/**
 *  Get service_notification_options.
 *
 *  @return The service_notification_options.
 */
unsigned int contact::service_notification_options() const throw () {
  return _service_notification_options;
}

/**
 *  Get service_notification_period.
 *
 *  @return The service_notification_period.
 */
std::string const& contact::service_notification_period() const throw () {
  return _service_notification_period;
}

/**
 *  Get service_notifications_enabled.
 *
 *  @return The service_notifications_enabled.
 */
bool contact::service_notifications_enabled() const throw () {
  return _service_notifications_enabled;
}

/**
 *  Get contact timezone.
 *
 *  @return This contact timezone.
 */
std::string const& contact::timezone() const throw () {
  return _timezone;
}

/**
 *  Set new contact address.
 *
 *  @param[in] key   The address key.
 *  @param[in] value The address value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_address(
       std::string const& key,
       std::string const& value) {
  unsigned int id;
  if (!string::to(key.c_str(), id) || (id < 1) || (id > MAX_ADDRESSES))
    return false;
  _address[id - 1] = value;
  return true;
}

/**
 *  Set alias value.
 *
 *  @param[in] value The new alias value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_alias(std::string const& value) {
  _alias = value;
  return true;
}

/**
 *  Set can_submit_commands value.
 *
 *  @param[in] value The new can_submit_commands value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_can_submit_commands(bool value) {
  _can_submit_commands = value;
  return true;
}

/**
 *  Set contactgroups value.
 *
 *  @param[in] value The new contactgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_contactgroups(std::string const& value) {
  _contactgroups = value;
  return true;
}

/**
 *  Set contact_name value.
 *
 *  @param[in] value The new contact_name value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_contact_name(std::string const& value) {
  _contact_name = value;
  return true;
}

/**
 *  Set email value.
 *
 *  @param[in] value The new email value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_email(std::string const& value) {
  _email = value;
  return true;
}

/**
 *  Set host_notifications_enabled value.
 *
 *  @param[in] value The new host_notifications_enabled value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_host_notifications_enabled(bool value) {
  _host_notifications_enabled = value;
  return true;
}

/**
 *  Set host_notification_commands value.
 *
 *  @param[in] value The new host_notification_commands value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_host_notification_commands(std::string const& value) {
  _host_notification_commands = value;
  return true;
}

/**
 *  Set host_notification_options value.
 *
 *  @param[in] value The new host_notification_options value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_host_notification_options(std::string const& value) {
  unsigned short options(host::none);
  std::list<std::string> values;
  string::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    string::trim(*it);
    if (*it == "d" || *it == "down")
      options |= host::down;
    else if (*it == "u" || *it == "unreachable")
      options |= host::unreachable;
    else if (*it == "r" || *it == "recovery")
      options |= host::up;
    else if (*it == "f" || *it == "flapping")
      options |= host::flapping;
    else if (*it == "s" || *it == "downtime")
      options |= host::downtime;
    else if (*it == "n" || *it == "none")
      options = host::none;
    else if (*it == "a" || *it == "all")
      options = host::down
        | host::unreachable
        | host::up
        | host::flapping
        | host::downtime;
    else
      return false;
  }
  _host_notification_options = options;
  return true;
}

/**
 *  Set host_notification_period value.
 *
 *  @param[in] value The new host_notification_period value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_host_notification_period(std::string const& value) {
  _host_notification_period = value;
  return true;
}

/**
 *  Set retain_nonstatus_information value.
 *
 *  @param[in] value The new retain_nonstatus_information value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_retain_nonstatus_information(bool value) {
  _retain_nonstatus_information = value;
  return true;
}

/**
 *  Set retain_status_information value.
 *
 *  @param[in] value The new retain_status_information value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_retain_status_information(bool value) {
  _retain_status_information = value;
  return true;
}

/**
 *  Set pager value.
 *
 *  @param[in] value The new pager value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_pager(std::string const& value) {
  _pager = value;
  return true;
}

/**
 *  Set service_notification_commands value.
 *
 *  @param[in] value The new service_notification_commands value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_service_notification_commands(std::string const& value) {
  _service_notification_commands = value;
  return true;
}

/**
 *  Set service_notification_options value.
 *
 *  @param[in] value The new service_notification_options value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_service_notification_options(std::string const& value) {
  unsigned short options(service::none);
  std::list<std::string> values;
  string::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    string::trim(*it);
    if (*it == "u" || *it == "unknown")
      options |= service::unknown;
    else if (*it == "w" || *it == "warning")
      options |= service::warning;
    else if (*it == "c" || *it == "critical")
      options |= service::critical;
    else if (*it == "r" || *it == "recovery")
      options |= service::ok;
    else if (*it == "f" || *it == "flapping")
      options |= service::flapping;
    else if (*it == "s" || *it == "downtime")
      options |= service::downtime;
    else if (*it == "n" || *it == "none")
      options = service::none;
    else if (*it == "a" || *it == "all")
      options = service::unknown
        | service::warning
        | service::critical
        | service::ok
        | service::flapping
        | service::downtime;
    else
      return false;
  }
  _service_notification_options = options;
  return true;
}

/**
 *  Set service_notification_period value.
 *
 *  @param[in] value The new service_notification_period value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_service_notification_period(std::string const& value) {
  _service_notification_period = value;
  return true;
}

/**
 *  Set service_notifications_enabled value.
 *
 *  @param[in] value The new service_notifications_enabled value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_service_notifications_enabled(bool value) {
  _service_notifications_enabled = value;
  return true;
}

/**
 *  Set contact timezone.
 *
 *  @param[in] value  New contact timezone.
 *
 *  @return True.
 */
bool contact::_set_timezone(std::string const& value) {
  _timezone = value;
  return true;
}

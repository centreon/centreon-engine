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
static bool const           default_can_submit_commands(true);
static bool const           default_host_notifications_enabled(true);
static unsigned short const default_host_notification_options(host::none);
static bool const           default_retain_nonstatus_information(true);
static bool const           default_retain_status_information(true);
static unsigned short const default_service_notification_options(service::none);
static bool const           default_service_notifications_enabled(true);

static unsigned int const   MAX_ADDRESSES(6);

/**
 *  Default constructor.
 */
contact::contact()
  : object(object::contact, "contact") {
  _address.resize(MAX_ADDRESSES);
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
          && std::operator==(_customvariables, right._customvariables)
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
 *  Get the unique object id.
 *
 *  @return The object id.
 */
std::size_t contact::id() const throw () {
  return (_id);
}

/**
 *  Check if the object is valid.
 *
 *  @return True if is a valid object, otherwise false.
 */
void contact::check_validity() const {
  if (_contact_name.empty())
    throw (engine_error() << "configuration: invalid contact property "
           "contact_name is missing");
}

/**
 *  Merge object.
 *
 *  @param[in] obj The object to merge.
 */
void contact::merge(object const& obj) {
  if (obj.type() != _type)
    throw (engine_error() << "merge failed: invalid object type");
  contact const& tmpl(static_cast<contact const&>(obj));

  MRG_ADDRESS(_address);
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
}

/**
 *  Parse and set the contact property.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The property value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::parse(
       std::string const& key,
       std::string const& value) {
  for (unsigned int i(0);
       i < sizeof(gl_setters) / sizeof(gl_setters[0]);
       ++i)
    if (gl_setters[i].name == key)
      return ((gl_setters[i].func)(*this, value));
  if (key.find("address") == 0)
    return (_set_address(key, value));
  return (false);
}

/**
 *  Get address.
 *
 *  @return The address.
 */
tab_string const& contact::address() const throw () {
  return (_address);
}

/**
 *  Get alias.
 *
 *  @return The alias.
 */
std::string const& contact::alias() const throw () {
  return (_alias);
}

/**
 *  Get can_submit_commands.
 *
 *  @return The can_submit_commands.
 */
bool contact::can_submit_commands() const throw () {
  return (_can_submit_commands);
}

/**
 *  Get contactgroups.
 *
 *  @return The contactgroups.
 */
list_string const& contact::contactgroups() const throw () {
  return (_contactgroups.get());
}

/**
 *  Get contact_name.
 *
 *  @return The contact_name.
 */
std::string const& contact::contact_name() const throw () {
  return (_contact_name);
}

/**
 *  Get customvariables.
 *
 *  @return The customvariables.
 */
properties const& contact::customvariables() const throw () {
  return (_customvariables);
}

/**
 *  Get email.
 *
 *  @return The email.
 */
std::string const& contact::email() const throw () {
  return (_email);
}

/**
 *  Get host_notifications_enabled.
 *
 *  @return The host_notifications_enabled.
 */
bool contact::host_notifications_enabled() const throw () {
  return (_host_notifications_enabled);
}

/**
 *  Get host_notification_commands.
 *
 *  @return The host_notification_commands.
 */
list_string const& contact::host_notification_commands() const throw () {
  return (_host_notification_commands.get());
}

/**
 *  Get host_notification_options.
 *
 *  @return The host_notification_options.
 */
unsigned int contact::host_notification_options() const throw () {
  return (_host_notification_options);
}

/**
 *  Get host_notification_period.
 *
 *  @return The host_notification_period.
 */
std::string const& contact::host_notification_period() const throw () {
  return (_host_notification_period);
}

/**
 *  Get retain_nonstatus_information.
 *
 *  @return The retain_nonstatus_information.
 */
bool contact::retain_nonstatus_information() const throw () {
  return (_retain_nonstatus_information);
}

/**
 *  Get retain_status_information.
 *
 *  @return The retain_status_information.
 */
bool contact::retain_status_information() const throw () {
  return (_retain_status_information);
}

/**
 *  Get pager.
 *
 *  @return The pager.
 */
std::string const& contact::pager() const throw () {
  return (_pager);
}

/**
 *  Get service_notification_commands.
 *
 *  @return The service_notification_commands.
 */
list_string const& contact::service_notification_commands() const throw () {
  return (_service_notification_commands.get());
}

/**
 *  Get service_notification_options.
 *
 *  @return The service_notification_options.
 */
unsigned int contact::service_notification_options() const throw () {
  return (_service_notification_options);
}

/**
 *  Get service_notification_period.
 *
 *  @return The service_notification_period.
 */
std::string const& contact::service_notification_period() const throw () {
  return (_service_notification_period);
}

/**
 *  Get service_notifications_enabled.
 *
 *  @return The service_notifications_enabled.
 */
bool contact::service_notifications_enabled() const throw () {
  return (_service_notifications_enabled);
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
  if (!misc::to(key, id) || id < 1 || id > MAX_ADDRESSES)
    return (false);
  _address[id] = value;
  return (true);
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
  return (true);
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
  return (true);
}

/**
 *  Set contactgroups value.
 *
 *  @param[in] value The new contactgroups value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_contactgroups(std::string const& value) {
  _contactgroups.set(value);
  return (true);
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
  _id = _hash(value);
  return (true);
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
  return (true);
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
  return (true);
}

/**
 *  Set host_notification_commands value.
 *
 *  @param[in] value The new host_notification_commands value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_host_notification_commands(std::string const& value) {
  _host_notification_commands.set(value);
  return (true);
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
  misc::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    misc::trim(*it);
    if (*it == "d" || *it == "down")
      options |= host::down;
    else if (*it == "u" || *it == "unreachable")
      options |= host::unreachable;
    else if (*it == "r" || *it == "recovery")
      options |= host::recovery;
    else if (*it == "f" || *it == "flapping")
      options |= host::flapping;
    else if (*it == "s" || *it == "downtime")
      options |= host::downtime;
    else if (*it == "n" || *it == "none")
      options = host::none;
    else if (*it == "a" || *it == "all")
      options = host::down
        | host::unreachable
        | host::recovery
        | host::flapping
        | host::downtime;
    else
      return (false);
  }
  _host_notification_options = options;
  return (true);
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
  return (true);
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
  return (true);
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
  return (true);
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
  return (true);
}

/**
 *  Set service_notification_commands value.
 *
 *  @param[in] value The new service_notification_commands value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::_set_service_notification_commands(std::string const& value) {
  _service_notification_commands.set(value);
  return (true);
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
  misc::split(value, values, ',');
  for (std::list<std::string>::iterator
         it(values.begin()), end(values.end());
       it != end;
       ++it) {
    misc::trim(*it);
    if (*it == "u" || *it == "unknown")
      options |= service::unknown;
    else if (*it == "w" || *it == "warning")
      options |= service::warning;
    else if (*it == "c" || *it == "critical")
      options |= service::critical;
    else if (*it == "r" || *it == "recovery")
      options |= service::recovery;
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
        | service::recovery
        | service::flapping
        | service::downtime;
    else
      return (false);
  }
  _service_notification_options = options;
  return (true);
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
  return (true);
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
  return (true);
}

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

#include "com/centreon/engine/retention/contact.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::retention;

#define SETTER(type, method) \
  &object::setter<contact, type, &contact::method>::generic

contact::setters const contact::_setters[] = {
    {"contact_name", SETTER(std::string const&, _set_contact_name)},
    {"host_notification_period",
     SETTER(std::string const&, _set_host_notification_period)},
    {"host_notifications_enabled",
     SETTER(bool, _set_host_notifications_enabled)},
    {"last_host_notification", SETTER(time_t, _set_last_host_notification)},
    {"last_service_notification",
     SETTER(time_t, _set_last_service_notification)},
    {"modified_attributes", SETTER(unsigned long, _set_modified_attributes)},
    {"modified_host_attributes",
     SETTER(unsigned long, _set_modified_host_attributes)},
    {"modified_service_attributes",
     SETTER(unsigned long, _set_modified_service_attributes)},
    {"service_notification_period",
     SETTER(std::string const&, _set_service_notification_period)},
    {"service_notifications_enabled",
     SETTER(bool, _set_service_notifications_enabled)}};

/**
 *  Constructor.
 */
contact::contact() : object(object::contact) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
contact::contact(contact const& right) : object(right) {
  operator=(right);
}

/**
 *  Destructor.
 */
contact::~contact() throw() {}

/**
 *  Copy operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
contact& contact::operator=(contact const& right) {
  if (this != &right) {
    object::operator=(right);
    _contact_name = right._contact_name;
    _customvariables = right._customvariables;
    _host_notification_period = right._host_notification_period;
    _host_notifications_enabled = right._host_notifications_enabled;
    _last_host_notification = right._last_host_notification;
    _last_service_notification = right._last_service_notification;
    _modified_attributes = right._modified_attributes;
    _modified_host_attributes = right._modified_host_attributes;
    _modified_service_attributes = right._modified_service_attributes;
    _service_notification_period = right._service_notification_period;
    _service_notifications_enabled = right._service_notifications_enabled;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool contact::operator==(contact const& right) const throw() {
  return (object::operator==(right) && _contact_name == right._contact_name &&
          std::operator==(_customvariables, right._customvariables) &&
          _host_notification_period == right._host_notification_period &&
          _host_notifications_enabled == right._host_notifications_enabled &&
          _last_host_notification == right._last_host_notification &&
          _last_service_notification == right._last_service_notification &&
          _modified_attributes == right._modified_attributes &&
          _modified_host_attributes == right._modified_host_attributes &&
          _modified_service_attributes == right._modified_service_attributes &&
          _service_notification_period == right._service_notification_period &&
          _service_notifications_enabled ==
              right._service_notifications_enabled);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool contact::operator!=(contact const& right) const throw() {
  return (!operator==(right));
}

/**
 *  Set new value on specific property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool contact::set(char const* key, char const* value) {
  for (unsigned int i(0); i < sizeof(_setters) / sizeof(_setters[0]); ++i)
    if (!strcmp(_setters[i].name, key))
      return ((_setters[i].func)(*this, value));
  if ((key[0] == '_') && (strlen(value) > 3)) {
    _customvariables[key + 1] = customvariable(value + 2);
    return true;
  }
  return false;
}

/**
 * Get contact_name.
 *
 * @return The contact_name.
 */
std::string const& contact::contact_name() const throw() {
  return (_contact_name);
}

/**
 * Get customvariables.
 *
 * @return The customvariables.
 */
map_customvar const& contact::customvariables() const throw() {
  return (_customvariables);
}

/**
 * Get host_notification_period.
 *
 * @return The host_notification_period.
 */
opt<std::string> const& contact::host_notification_period() const throw() {
  return (_host_notification_period);
}

/**
 * Get host_notifications_enabled.
 *
 * @return The host_notifications_enabled.
 */
opt<bool> const& contact::host_notifications_enabled() const throw() {
  return (_host_notifications_enabled);
}

/**
 * Get last_host_notification.
 *
 * @return The last_host_notification.
 */
opt<time_t> const& contact::last_host_notification() const throw() {
  return (_last_host_notification);
}

/**
 * Get last_service_notification.
 *
 * @return The last_service_notification.
 */
opt<time_t> const& contact::last_service_notification() const throw() {
  return (_last_service_notification);
}

/**
 * Get modified_attributes.
 *
 * @return The modified_attributes.
 */
opt<unsigned long> const& contact::modified_attributes() const throw() {
  return (_modified_attributes);
}

/**
 * Get modified_host_attributes.
 *
 * @return The modified_host_attributes.
 */
opt<unsigned long> const& contact::modified_host_attributes() const throw() {
  return (_modified_host_attributes);
}

/**
 * Get modified_service_attributes.
 *
 * @return The modified_service_attributes.
 */
opt<unsigned long> const& contact::modified_service_attributes() const throw() {
  return (_modified_service_attributes);
}

/**
 * Get service_notification_period.
 *
 * @return The service_notification_period.
 */
opt<std::string> const& contact::service_notification_period() const throw() {
  return (_service_notification_period);
}

/**
 * Get service_notifications_enabled.
 *
 * @return The service_notifications_enabled.
 */
opt<bool> const& contact::service_notifications_enabled() const throw() {
  return (_service_notifications_enabled);
}

/**
 *  Set contact_name.
 *
 *  @param[in] value The new contact_name.
 */
bool contact::_set_contact_name(std::string const& value) {
  _contact_name = value;
  return (true);
}

/**
 *  Set host_notification_period.
 *
 *  @param[in] value The new host_notification_period.
 */
bool contact::_set_host_notification_period(std::string const& value) {
  _host_notification_period = value;
  return (true);
}

/**
 *  Set host_notifications_enabled.
 *
 *  @param[in] value The new host_notifications_enabled.
 */
bool contact::_set_host_notifications_enabled(bool value) {
  _host_notifications_enabled = value;
  return (true);
}

/**
 *  Set last_host_notification.
 *
 *  @param[in] value The new last_host_notification.
 */
bool contact::_set_last_host_notification(time_t value) {
  _last_host_notification = value;
  return (true);
}

/**
 *  Set last_service_notification.
 *
 *  @param[in] value The new last_service_notification.
 */
bool contact::_set_last_service_notification(time_t value) {
  _last_service_notification = value;
  return (true);
}

/**
 *  Set modified_attributes.
 *
 *  @param[in] value The new modified_attributes.
 */
bool contact::_set_modified_attributes(unsigned long value) {
  _modified_attributes = value;
  return (true);
}

/**
 *  Set modified_host_attributes.
 *
 *  @param[in] value The new modified_host_attributes.
 */
bool contact::_set_modified_host_attributes(unsigned long value) {
  _modified_host_attributes = value;
  return (true);
}

/**
 *  Set modified_service_attributes.
 *
 *  @param[in] value The new modified_service_attributes.
 */
bool contact::_set_modified_service_attributes(unsigned long value) {
  _modified_service_attributes = value;
  return (true);
}

/**
 *  Set service_notification_period.
 *
 *  @param[in] value The new service_notification_period.
 */
bool contact::_set_service_notification_period(std::string const& value) {
  _service_notification_period = value;
  return (true);
}

/**
 *  Set service_notifications_enabled.
 *
 *  @param[in] value The new service_notifications_enabled.
 */
bool contact::_set_service_notifications_enabled(bool value) {
  _service_notifications_enabled = value;
  return (true);
}

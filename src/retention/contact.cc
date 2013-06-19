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

#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/retention/contact.hh"
#include "com/centreon/engine/statusdata.hh"

using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine;

/**
 *  Constructor.
 *
 *  @param[in] obj The contact to use for retention.
 */
retention::contact::contact(contact_struct* obj)
  : object(object::contact),
    _obj(obj) {

}

/**
 *  Destructor.
 */
retention::contact::~contact() throw () {
  _finished();
}

/**
 *  Set new value on specific property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool retention::contact::set(
       std::string const& key,
       std::string const& value) {
  if (!_obj && key == "contact_name") {
    umap<std::string, shared_ptr<contact_struct> >::const_iterator
      it(state::instance().contacts().find(value));
    if (it != state::instance().contacts().end())
      _obj = it->second.get();
    return (true);
  }
  else if (!_obj)
    return (false);
  if (_modified_attributes(key, value))
    return (true);
  if (_retain_status_information(key, value))
    return (true);
  return (_retain_nonstatus_information(key, value));
}

/**
 *  Finish all contact update.
 */
void retention::contact::_finished() throw () {
  if (!_obj)
    return;

  // adjust modified attributes if necessary.
  if (!_obj->retain_nonstatus_information)
    _obj->modified_attributes = MODATTR_NONE;

  // adjust modified attributes if no custom variables have been changed.
  if (_obj->modified_attributes & MODATTR_CUSTOM_VARIABLE) {
    for (customvariablesmember* member(_obj->custom_variables);
         member;
         member = member->next)
      if (member->has_been_modified) {
        _obj->modified_attributes -= MODATTR_CUSTOM_VARIABLE;
        break;
      }
  }

  // update contact status.
  update_contact_status(_obj, false);
}

/**
 *  Set new value on specific modified attrivute property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool retention::contact::_modified_attributes(
       std::string const& key,
       std::string const& value) {

  if (key == "modified_attributes") {
    misc::to(value, _obj->modified_attributes);
    // mask out attributes we don't want to retain.
    _obj->modified_attributes &= ~0L;
  }
  else if (key == "modified_host_attributes") {
    misc::to(value, _obj->modified_host_attributes);
    // mask out attributes we don't want to retain.
    _obj->modified_host_attributes
      &= ~config->retained_contact_host_attribute_mask();
  }
  else if (key == "modified_service_attributes") {
    misc::to(value, _obj->modified_service_attributes);
    // mask out attributes we don't want to retain.
    _obj->modified_service_attributes
      &= ~config->retained_contact_service_attribute_mask();
  }
  else
    return (false);
  return (true);
}

/**
 *  Set new value on specific retain nonstatus information property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool retention::contact::_retain_nonstatus_information(
       std::string const& key,
       std::string const& value) {
  if (!_obj->retain_nonstatus_information)
    return (false);

  if (key == "host_notification_period") {
    if (_obj->modified_host_attributes & MODATTR_NOTIFICATION_TIMEPERIOD) {
      if (!find_timeperiod(value.c_str()))
        _obj->modified_host_attributes -= MODATTR_NOTIFICATION_TIMEPERIOD;
      else {
        delete[] _obj->host_notification_period;
        _obj->host_notification_period = my_strdup(value);
      }
    }
  }
  else if (key == "service_notification_period") {
    if (_obj->modified_service_attributes & MODATTR_NOTIFICATION_TIMEPERIOD) {
      if (!find_timeperiod(value.c_str()))
        _obj->modified_service_attributes -= MODATTR_NOTIFICATION_TIMEPERIOD;
      else {
        delete[] _obj->service_notification_period;
        _obj->service_notification_period = my_strdup(value);
      }
    }
  }
  else if (key == "host_notifications_enabled") {
    if (_obj->modified_host_attributes & MODATTR_NOTIFICATIONS_ENABLED)
      misc::to<bool, int>(value, _obj->host_notifications_enabled);
  }
  else if (key == "service_notifications_enabled") {
    if (_obj->modified_service_attributes & MODATTR_NOTIFICATIONS_ENABLED)
      misc::to<bool, int>(value, _obj->service_notifications_enabled);
  }
  else if (!key.empty() && key[0] == '_') {
    if (_obj->modified_attributes & MODATTR_CUSTOM_VARIABLE
        && value.size() > 3) {
      char const* cvname(key.c_str() + 1);
      char const* cvvalue(value.c_str() + 2);

      for (customvariablesmember* member = _obj->custom_variables;
           member;
           member = member->next) {
        if (!strcmp(cvname, member->variable_name)) {
          if (strcmp(cvvalue, member->variable_value)) {
            delete[] member->variable_value;
            member->variable_value = my_strdup(cvvalue);
            member->has_been_modified = true;
          }
          break;
        }
      }
    }
  }
  else
    return (false);
  return (true);
}

/**
 *  Set new value on specific retain nonstatus information property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool retention::contact::_retain_status_information(
       std::string const& key,
       std::string const& value) {
  if (!_obj->retain_status_information)
    return (false);

  if (key == "last_host_notification")
    misc::to(value, _obj->last_host_notification);
  else if (key == "last_service_notification")
    misc::to(value, _obj->last_service_notification);
  else
    return (false);
  return (true);
}

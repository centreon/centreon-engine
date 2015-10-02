/*
** Copyright 2011-2013,2015 Merethis
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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/contact.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/commandsmember.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"
#include "com/centreon/engine/objects/contact.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       contact const& obj1,
       contact const& obj2) throw () {
  return (is_equal(obj1.name, obj2.name)
          && is_equal(obj1.alias, obj2.alias)
          && is_equal(obj1.email, obj2.email)
          && is_equal(obj1.pager, obj2.pager)
          && is_equal(obj1.address, obj2.address, MAX_CONTACT_ADDRESSES)
          && is_equal(obj1.host_notification_commands, obj2.host_notification_commands)
          && is_equal(obj1.service_notification_commands, obj2.service_notification_commands)
          && obj1.notify_on_service_unknown == obj2.notify_on_service_unknown
          && obj1.notify_on_service_warning == obj2.notify_on_service_warning
          && obj1.notify_on_service_critical == obj2.notify_on_service_critical
          && obj1.notify_on_service_recovery == obj2.notify_on_service_recovery
          && obj1.notify_on_service_flapping == obj2.notify_on_service_flapping
          && obj1.notify_on_service_downtime == obj2.notify_on_service_downtime
          && obj1.notify_on_host_down == obj2.notify_on_host_down
          && obj1.notify_on_host_unreachable == obj2.notify_on_host_unreachable
          && obj1.notify_on_host_recovery == obj2.notify_on_host_recovery
          && obj1.notify_on_host_flapping == obj2.notify_on_host_flapping
          && obj1.notify_on_host_downtime == obj2.notify_on_host_downtime
          && is_equal(obj1.host_notification_period, obj2.host_notification_period)
          && is_equal(obj1.service_notification_period, obj2.service_notification_period)
          && obj1.host_notifications_enabled == obj2.host_notifications_enabled
          && obj1.service_notifications_enabled == obj2.service_notifications_enabled
          && obj1.can_submit_commands == obj2.can_submit_commands
          && obj1.retain_status_information == obj2.retain_status_information
          && obj1.retain_nonstatus_information == obj2.retain_nonstatus_information
          && is_equal(obj1.custom_variables, obj2.custom_variables)
          && obj1.last_host_notification == obj2.last_host_notification
          && obj1.last_service_notification == obj2.last_service_notification
          && obj1.modified_attributes == obj2.modified_attributes
          && obj1.modified_host_attributes == obj2.modified_host_attributes
          && obj1.modified_service_attributes == obj2.modified_service_attributes);
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool operator!=(
       contact const& obj1,
       contact const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump contact content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The contact to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, contact const& obj) {
  char const* cntctgrp_str(NULL);
  if (obj.contactgroups_ptr)
    cntctgrp_str = chkstr(static_cast<contactgroup const*>(obj.contactgroups_ptr->object_ptr)->group_name);
  char const* hst_notif_str(NULL);
  if (obj.host_notification_period_ptr)
    hst_notif_str = chkstr(obj.host_notification_period_ptr->name);
  char const* svc_notif_str(NULL);
  if (obj.service_notification_period_ptr)
    svc_notif_str = chkstr(obj.service_notification_period_ptr->name);

  os << "contact {\n"
    "  name:                            " << chkstr(obj.name) << "\n"
    "  alias:                           " << chkstr(obj.alias) << "\n"
    "  email:                           " << chkstr(obj.email) << "\n"
    "  pager:                           " << chkstr(obj.pager) << "\n"
    "  address:                         ";
  for (unsigned int i(0); i < MAX_CONTACT_ADDRESSES && obj.address[i]; ++i)
    os << chkstr(obj.address[i])
       << (i + 1 < MAX_CONTACT_ADDRESSES && obj.address[i + 1] ? ", " : "");
  os << (obj.address[0] ? " \n" : "\"NULL\"\n");
  os << "  host_notification_commands:      " << chkobj(obj.host_notification_commands) << "\n"
    "  service_notification_commands:   " << chkobj(obj.service_notification_commands) << "\n"
    "  notify_on_service_unknown:       " << obj.notify_on_service_unknown << "\n"
    "  notify_on_service_warning:       " << obj.notify_on_service_warning << "\n"
    "  notify_on_service_critical:      " << obj.notify_on_service_critical << "\n"
    "  notify_on_service_recovery:      " << obj.notify_on_service_recovery << "\n"
    "  notify_on_service_flapping:      " << obj.notify_on_service_flapping << "\n"
    "  notify_on_service_downtime:      " << obj.notify_on_service_downtime << "\n"
    "  notify_on_host_down:             " << obj.notify_on_host_down << "\n"
    "  notify_on_host_unreachable:      " << obj.notify_on_host_unreachable << "\n"
    "  notify_on_host_recovery:         " << obj.notify_on_host_recovery << "\n"
    "  notify_on_host_flapping:         " << obj.notify_on_host_flapping << "\n"
    "  notify_on_host_downtime:         " << obj.notify_on_host_downtime << "\n"
    "  host_notification_period:        " << chkstr(obj.host_notification_period) << "\n"
    "  service_notification_period:     " << chkstr(obj.service_notification_period) << "\n"
    "  host_notifications_enabled:      " << obj.host_notifications_enabled << "\n"
    "  service_notifications_enabled:   " << obj.service_notifications_enabled << "\n"
    "  can_submit_commands:             " << obj.can_submit_commands << "\n"
    "  retain_status_information:       " << obj.retain_status_information << "\n"
    "  retain_nonstatus_information:    " << obj.retain_nonstatus_information << "\n"
    "  last_host_notification:          " << string::ctime(obj.last_host_notification) << "\n"
    "  last_service_notification:       " << string::ctime(obj.last_service_notification) << "\n"
    "  modified_attributes:             " << obj.modified_attributes << "\n"
    "  modified_host_attributes:        " << obj.modified_host_attributes << "\n"
    "  modified_service_attributes:     " << obj.modified_service_attributes << "\n"
    "  host_notification_period_ptr:    " << chkstr(hst_notif_str) << "\n"
    "  service_notification_period_ptr: " << chkstr(svc_notif_str) << "\n"
    "  contactgroups_ptr:               " << chkstr(cntctgrp_str) << "\n"
     << (obj.custom_variables ? chkobj(obj.custom_variables) : "")
     << "}\n";
  return (os);
}

/**
 *  Add a new contact to the list in memory.
 *
 *  @param[in] name                          Contact name.
 *  @param[in] alias                         Contact alias.
 *  @param[in] email                         Email.
 *  @param[in] pager                         Pager.
 *  @param[in] addresses                     Contact addresses.
 *  @param[in] svc_notification_period       Service notification
 *                                           period.
 *  @param[in] host_notification_period      Host nofication period.
 *  @param[in] notify_service_ok             Contact can be notified
 *                                           when service is ok.
 *  @param[in] notify_service_critical       Contact can be notified
 *                                           when service is critical.
 *  @param[in] notify_service_warning        Contact can be notified
 *                                           when service is warning.
 *  @param[in] notify_service_unknown        Contact can be notified
 *                                           when service is unknown.
 *  @param[in] notify_service_flapping       Contact can be notified
 *                                           when service is flapping.
 *  @param[in] notify_sevice_downtime        Contact can be notified on
 *                                           service downtime.
 *  @param[in] notify_host_up                Contact can be notified
 *                                           when host is up.
 *  @param[in] notify_host_down              Contact can be notified
 *                                           when host is down.
 *  @param[in] notify_host_unreachable       Contact can be notified
 *                                           when host is unreachable.
 *  @param[in] notify_host_flapping          Contact can be notified
 *                                           when host is flapping.
 *  @param[in] notify_host_downtime          Contact can be notified on
 *                                           host downtime.
 *  @param[in] host_notifications_enabled    Are contact host
 *                                           notifications enabled ?
 *  @param[in] service_notifications_enabled Are contact service
 *                                           notifications enabled ?
 *  @param[in] can_submit_commands           Can user submit external
 *                                           commands ?
 *  @param[in] retain_status_information     Shall Engine retain contact
 *                                           status info ?
 *  @param[in] retain_nonstatus_information  Shell Engine retain contact
 *                                           non-status info ?
 *
 *  @return New contact object.
 */
contact* add_contact(
           char const* name,
           char const* alias,
           char const* email,
           char const* pager,
           char const* const* addresses,
           char const* svc_notification_period,
           char const* host_notification_period,
           int notify_service_ok,
           int notify_service_critical,
           int notify_service_warning,
           int notify_service_unknown,
           int notify_service_flapping,
           int notify_service_downtime,
           int notify_host_up,
           int notify_host_down,
           int notify_host_unreachable,
           int notify_host_flapping,
           int notify_host_downtime,
           int host_notifications_enabled,
           int service_notifications_enabled,
           int can_submit_commands,
           int retain_status_information,
           int retain_nonstatus_information) {
  // Make sure we have the data we need.
  if (!name || !name[0]) {
    logger(log_config_error, basic)
      << "Error: Contact name is NULL";
    return (NULL);
  }

  // Check if the contact already exist.
  std::string id(name);
  if (is_contact_exist(id)) {
    logger(log_config_error, basic)
      << "Error: Contact '" << name << "' has already been defined";
    return (NULL);
  }

  // Allocate memory for a new contact.
  shared_ptr<contact> obj(new contact, deleter::contact);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->name = string::dup(name);
    obj->alias = string::dup(!alias ? name : alias);
    if (email)
      obj->email = string::dup(email);
    if (host_notification_period)
      obj->host_notification_period = string::dup(host_notification_period);
    if (pager)
      obj->pager = string::dup(pager);
    if (svc_notification_period)
      obj->service_notification_period = string::dup(svc_notification_period);
    if (addresses) {
      for (unsigned int x(0); x < MAX_CONTACT_ADDRESSES; ++x)
        if (addresses[x])
          obj->address[x] = string::dup(addresses[x]);
    }

    // Set remaining contact properties.
    obj->can_submit_commands = (can_submit_commands > 0);
    obj->host_notifications_enabled = (host_notifications_enabled > 0);
    obj->modified_attributes = MODATTR_NONE;
    obj->modified_host_attributes = MODATTR_NONE;
    obj->modified_service_attributes = MODATTR_NONE;
    obj->notify_on_host_down = (notify_host_down > 0);
    obj->notify_on_host_downtime = (notify_host_downtime > 0);
    obj->notify_on_host_flapping = (notify_host_flapping > 0);
    obj->notify_on_host_recovery = (notify_host_up > 0);
    obj->notify_on_host_unreachable = (notify_host_unreachable > 0);
    obj->notify_on_service_critical = (notify_service_critical > 0);
    obj->notify_on_service_downtime = (notify_service_downtime > 0);
    obj->notify_on_service_flapping = (notify_service_flapping > 0);
    obj->notify_on_service_recovery = (notify_service_ok > 0);
    obj->notify_on_service_unknown = (notify_service_unknown > 0);
    obj->notify_on_service_warning = (notify_service_warning > 0);
    obj->retain_nonstatus_information = (retain_nonstatus_information > 0);
    obj->retain_status_information = (retain_status_information > 0);
    obj->service_notifications_enabled = (service_notifications_enabled > 0);

    // Add new items to the configuration state.
    state::instance().contacts()[id] = obj;

    // Add new items to the list.
    obj->next = contact_list;
    contact_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_contact_data(
      NEBTYPE_CONTACT_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Get contact by name.
 *
 *  @param[in] name The contact name.
 *
 *  @return The struct contact or throw exception if the
 *          contact is not found.
 */
contact& engine::find_contact(std::string const& name) {
  umap<std::string, shared_ptr<contact_struct> >::const_iterator
    it(state::instance().contacts().find(name));
  if (it == state::instance().contacts().end())
    throw (engine_error() << "Contact '" << name << "' was not found");
  return (*it->second);
}

/**
 *  Get contact timezone.
 *
 *  @param[in] name  Contact name.
 *
 *  @return Contact timezone.
 */
char const* engine::get_contact_timezone(char const* name) {
  std::string const& timezone(contact_other_props[name].timezone);
  return (timezone.empty() ? NULL : timezone.c_str());
}

/**
 *  Get if contact exist.
 *
 *  @param[in] name The contact name.
 *
 *  @return True if the contact is found, otherwise false.
 */
bool engine::is_contact_exist(std::string const& name) throw () {
  umap<std::string, shared_ptr<contact_struct> >::const_iterator
    it(state::instance().contacts().find(name));
  return (it != state::instance().contacts().end());
}

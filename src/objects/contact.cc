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

#include "com/centreon/engine/misc/object.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/objects/commandsmember.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"
#include "com/centreon/engine/objects/contact.hh"

using namespace com::centreon::engine::misc;

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
  os << "  host_notification_commands:      " << chkobj(os, obj.host_notification_commands) << "\n"
    "  service_notification_commands:   " << chkobj(os, obj.service_notification_commands) << "\n"
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
    "  custom_variables:                " << chkobj(os, obj.custom_variables) << "\n"
    "  last_host_notification:          " << obj.last_host_notification << "\n"
    "  last_service_notification:       " << obj.last_service_notification << "\n"
    "  modified_attributes:             " << obj.modified_attributes << "\n"
    "  modified_host_attributes:        " << obj.modified_host_attributes << "\n"
    "  modified_service_attributes:     " << obj.modified_service_attributes << "\n"
    "  host_notification_period_ptr:    " << obj.host_notification_period_ptr << "\n"
    "  service_notification_period_ptr: " << obj.service_notification_period_ptr << "\n"
    "  contactgroups_ptr:               " << obj.contactgroups_ptr << "\n"
    "}\n";
  return (os);
}


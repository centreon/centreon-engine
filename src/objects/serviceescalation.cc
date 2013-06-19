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
#include "com/centreon/engine/objects/contactgroupsmember.hh"
#include "com/centreon/engine/objects/contactsmember.hh"
#include "com/centreon/engine/objects/serviceescalation.hh"

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
       serviceescalation const& obj1,
       serviceescalation const& obj2) throw () {
  return (is_equal(obj1.host_name, obj2.host_name)
          && is_equal(obj1.description, obj2.description)
          && obj1.first_notification == obj2.first_notification
          && obj1.last_notification == obj2.last_notification
          && obj1.notification_interval == obj2.notification_interval
          && is_equal(obj1.escalation_period, obj2.escalation_period)
          && obj1.escalate_on_recovery == obj2.escalate_on_recovery
          && obj1.escalate_on_warning == obj2.escalate_on_warning
          && obj1.escalate_on_unknown == obj2.escalate_on_unknown
          && obj1.escalate_on_critical == obj2.escalate_on_critical
          && is_equal(obj1.contact_groups, obj2.contact_groups)
          && is_equal(obj1.contacts, obj2.contacts));
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
       serviceescalation const& obj1,
       serviceescalation const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump serviceescalation content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The serviceescalation to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, serviceescalation const& obj) {
  os << "serviceescalation {\n"
    "  host_name:             " << chkstr(obj.host_name) << "\n"
    "  description:           " << chkstr(obj.description) << "\n"
    "  first_notification:    " << obj.first_notification << "\n"
    "  last_notification:     " << obj.last_notification << "\n"
    "  notification_interval: " << obj.notification_interval << "\n"
    "  escalation_period:     " << chkstr(obj.escalation_period) << "\n"
    "  escalate_on_recovery:  " << obj.escalate_on_recovery << "\n"
    "  escalate_on_warning:   " << obj.escalate_on_warning << "\n"
    "  escalate_on_unknown:   " << obj.escalate_on_unknown << "\n"
    "  escalate_on_critical:  " << obj.escalate_on_critical << "\n"
    "  contact_groups:        " << chkobj(os, obj.contact_groups) << "\n"
    "  contacts:              " << chkobj(os, obj.contacts) << "\n"
    "  service_ptr:           " << obj.service_ptr << "\n"
    "  escalation_period_ptr: " << obj.escalation_period_ptr << "\n"
    "}\n";
  return (os);
}


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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/serviceescalation.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/contactgroupsmember.hh"
#include "com/centreon/engine/objects/contactsmember.hh"
#include "com/centreon/engine/objects/serviceescalation.hh"
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
  char const* escalation_period_str(NULL);
  if (obj.escalation_period_ptr)
    escalation_period_str = chkstr(obj.escalation_period_ptr->name);
  std::string svc_str("\"NULL\"");
  if (obj.service_ptr) {
    svc_str = chkstr(obj.service_ptr->host_name);
    svc_str += ", ";
    svc_str += chkstr(obj.service_ptr->description);
  }

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
    "  contact_groups:        " << chkobj(obj.contact_groups) << "\n"
    "  contacts:              " << chkobj(obj.contacts) << "\n"
    "  service_ptr:           " << svc_str << "\n"
    "  escalation_period_ptr: " << chkstr(escalation_period_str) << "\n"
    "}\n";
  return (os);
}

/**
 *  Add a new service escalation to the list in memory.
 *
 *  @param[in] host_name             Host name.
 *  @param[in] description           Description.
 *  @param[in] first_notification    First notification.
 *  @param[in] last_notification     Last notification.
 *  @param[in] notification_interval Notification interval.
 *  @param[in] escalation_period     Escalation timeperiod name.
 *  @param[in] escalate_on_warning   Do we escalate on warning ?
 *  @param[in] escalate_on_unknown   Do we escalate on unknown ?
 *  @param[in] escalate_on_critical  Do we escalate on critical ?
 *  @param[in] escalate_on_recovery  Do we escalate on recovery ?
 *
 *  @return New service escalation.
 */
serviceescalation* add_service_escalation(
                     char const* host_name,
                     char const* description,
                     int first_notification,
                     int last_notification,
                     double notification_interval,
                     char const* escalation_period,
                     int escalate_on_warning,
                     int escalate_on_unknown,
                     int escalate_on_critical,
                     int escalate_on_recovery) {
  // Make sure we have the data we need.
  if (!host_name
      || !host_name[0]
      || !description
      || !description[0]) {
    logger(log_config_error, basic)
      << "Error: Service escalation host name or description is NULL";
    return (NULL);
  }

  // Check if the service escaltion already exist.
  std::pair<std::string, std::string>
    id(std::make_pair(host_name, description));
  umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> >::const_iterator
    it(state::instance().serviceescalations().find(id));
  if (it != state::instance().serviceescalations().end()) {
    logger(log_config_error, basic)
      << "Error: Serviceescalation '" << description << "' on host '"
      << host_name << "' has already been defined";
    return (NULL);
  }

  // Allocate memory for a new service escalation entry.
  shared_ptr<serviceescalation> obj(new serviceescalation, deleter::serviceescalation);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->host_name = string::dup(host_name);
    obj->description = string::dup(description);
    if (escalation_period)
      obj->escalation_period = string::dup(escalation_period);

    obj->escalate_on_critical = (escalate_on_critical > 0);
    obj->escalate_on_recovery = (escalate_on_recovery > 0);
    obj->escalate_on_unknown = (escalate_on_unknown > 0);
    obj->escalate_on_warning = (escalate_on_warning > 0);
    obj->first_notification = first_notification;
    obj->last_notification = last_notification;
    obj->notification_interval = (notification_interval <= 0) ? 0 : notification_interval;

    // Add new items to the configuration state.
    state::instance().serviceescalations()
      .insert(std::make_pair(id, obj));

    // Add new items to tail the list.
    obj->next = serviceescalation_list;
    serviceescalation_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_escalation_data(
      NEBTYPE_SERVICEESCALATION_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/*
** Copyright 2011-2019 Centreon
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
#include "com/centreon/engine/deleter/hostescalation.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/hostescalation.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

#define CMP_CSTR(str1, str2) \
  if ((!!(str1) ^ !!(str2)) \
      || ((str1) && (str2) && strcmp((str1), (str2)))) \
    return (strcmp((str1), (str2)) < 0);

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       hostescalation const& obj1,
       hostescalation const& obj2) throw () {
  return (is_equal(obj1.host_name, obj2.host_name)
          && obj1.first_notification == obj2.first_notification
          && obj1.last_notification == obj2.last_notification
          && obj1.notification_interval == obj2.notification_interval
          && is_equal(obj1.escalation_period, obj2.escalation_period)
          && obj1.escalate_on_recovery == obj2.escalate_on_recovery
          && obj1.escalate_on_down == obj2.escalate_on_down
          && obj1.escalate_on_unreachable == obj2.escalate_on_unreachable
          && ((obj1.contact_groups.size() == obj2.contact_groups.size()) &&
               std::equal(obj1.contact_groups.begin(),
                          obj1.contact_groups.end(),
                          obj2.contact_groups.begin()))
          && ((obj1.contacts.size() == obj2.contacts.size()) &&
               std::equal(obj1.contacts.begin(),
                          obj1.contacts.end(),
                          obj2.contacts.begin())));
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
       hostescalation const& obj1,
       hostescalation const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Less-than operator.
 *
 *  @param[in] obj1 First object to compare.
 *  @param[in] obj2 Second object to compare.
 *
 *  @return True if the first object is less than the second.
 */
bool operator<(
       hostescalation const& obj1,
       hostescalation const& obj2) {
  CMP_CSTR(obj1.host_name, obj2.host_name)
  else
    CMP_CSTR(obj1.escalation_period, obj2.escalation_period)
  else if (obj1.first_notification != obj2.first_notification)
    return (obj1.first_notification < obj2.first_notification);
  else if (obj1.last_notification != obj2.last_notification)
    return (obj1.last_notification < obj2.last_notification);
  else if (obj1.notification_interval != obj2.notification_interval)
    return (obj1.notification_interval < obj2.notification_interval);
  else if (obj1.escalate_on_recovery != obj2.escalate_on_recovery)
    return (obj1.escalate_on_recovery < obj2.escalate_on_recovery);
  else if (obj1.escalate_on_down != obj2.escalate_on_down)
    return (obj1.escalate_on_down < obj2.escalate_on_down);
  else if (obj1.escalate_on_unreachable
           != obj2.escalate_on_unreachable)
    return (obj1.escalate_on_unreachable
            < obj2.escalate_on_unreachable);
  for (contactgroup_map::const_iterator
         it1(obj1.contact_groups.begin()),
         it2(obj2.contact_groups.begin()),
         end1(obj1.contact_groups.end()),
         end2(obj2.contact_groups.end());
       (it1 != end1) || (it2 != end2);
       ++it1, ++it2) {
    if (it1->second == nullptr || it2->second == nullptr)
      return (!!it1->second < !!it2->second);
    else if (it1->second != it2->second)
      return (it1->second < it2->second);
  }
  for (contact_map::const_iterator
         it1(obj1.contacts.begin()),
         it2(obj2.contacts.begin()),
         end1(obj1.contacts.end()),
         end2(obj2.contacts.end());
       (it1 != end1) || (it2 != end2);
       ++it1, ++it2) {
    if (it1->second == nullptr || it2->second == nullptr)
      return (!!it1->second < !!it2->second);
    else if (it1->second != it2->second)
      return (it1->second < it2->second);
  }
  return (false);
}

/**
 *  Dump hostescalation content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The hostescalation to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, hostescalation const& obj) {
  char const* hst_str(NULL);
  char const* escalation_period_str(NULL);
  if (obj.escalation_period_ptr)
    escalation_period_str = chkstr(obj.escalation_period_ptr->name);

  std::string cg_oss;
  std::string c_oss;

  if (obj.contact_groups.empty())
    cg_oss = "\"NULL\"";
  else {
    std::ostringstream oss;
    oss << obj.contact_groups;
    cg_oss = oss.str();
  }
  if (obj.contacts.empty())
    c_oss = "\"NULL\"";
  else {
    std::ostringstream oss;
    oss << obj.contacts;
    c_oss = oss.str();
  }

  os << "hostescalation {\n"
    "  host_name:               " << chkstr(obj.host_name) << "\n"
    "  first_notification:      " << obj.first_notification << "\n"
    "  last_notification:       " << obj.last_notification << "\n"
    "  notification_interval:   " << obj.notification_interval << "\n"
    "  escalation_period:       " << chkstr(obj.escalation_period) << "\n"
    "  escalate_on_recovery:    " << obj.escalate_on_recovery << "\n"
    "  escalate_on_down:        " << obj.escalate_on_down << "\n"
    "  escalate_on_unreachable: " << obj.escalate_on_unreachable << "\n"
    "  contact_groups:          " << cg_oss << "\n"
    "  contacts:                " << c_oss << "\n"
    "  host_ptr:                " << (obj.host_ptr ?
                                        obj.host_ptr->get_name() :
                                        "\"NULL\"") << "\n"
    "  escalation_period_ptr:   " << chkstr(escalation_period_str) << "\n"
    "}\n";
  return (os);
}

/**
 *  Add a new host escalation to the list in memory.
 *
 *  @param[in] host_name               Host name.
 *  @param[in] first_notification      First notification.
 *  @param[in] last_notification       Last notification.
 *  @param[in] notification_interval   Notification interval.
 *  @param[in] escalation_period       Escalation timeperiod name.
 *  @param[in] escalate_on_down        Escalate on down ?
 *  @param[in] escalate_on_unreachable Escalate on unreachable ?
 *  @param[in] escalate_on_recovery    Escalate on recovery ?
 *
 *  @return New host escalation.
 */
hostescalation* add_host_escalation(
                  char const* host_name,
                  int first_notification,
                  int last_notification,
                  double notification_interval,
                  char const* escalation_period,
                  int escalate_on_down,
                  int escalate_on_unreachable,
                  int escalate_on_recovery) {
  // Make sure we have the data we need.
  if (!host_name || !host_name[0]) {
    logger(log_config_error, basic)
      << "Error: Host escalation host name is NULL";
    return (NULL);
  }

  // Allocate memory for a new host escalation entry.
  std::shared_ptr<hostescalation> obj(new hostescalation,
    deleter::hostescalation);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->host_name = string::dup(host_name);
    if (escalation_period)
      obj->escalation_period = string::dup(escalation_period);
    obj->escalate_on_down = (escalate_on_down > 0);
    obj->escalate_on_recovery = (escalate_on_recovery > 0);
    obj->escalate_on_unreachable = (escalate_on_unreachable > 0);
    obj->first_notification = first_notification;
    obj->last_notification = last_notification;
    obj->notification_interval = (notification_interval <= 0) ? 0 : notification_interval;

    // Add new items to the configuration state.
    state::instance().hostescalations()
      .insert(std::make_pair(obj->host_name, obj));

    // Add new items to the list.
    obj->next = hostescalation_list;
    hostescalation_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_escalation_data(
      NEBTYPE_HOSTESCALATION_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      &tv);
  }
  catch (...) {
    obj.reset();
  }

  return (obj.get());
}

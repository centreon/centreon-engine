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
#include "com/centreon/engine/deleter/hostescalation.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/misc/object.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/objects/contactgroupsmember.hh"
#include "com/centreon/engine/objects/contactsmember.hh"
#include "com/centreon/engine/objects/hostescalation.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
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
       hostescalation const& obj1,
       hostescalation const& obj2) throw () {
  return (!operator==(obj1, obj2));
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
  if (obj.host_ptr)
    hst_str = chkstr(obj.host_ptr->name);
  char const* escalation_period_str(NULL);
  if (obj.escalation_period_ptr)
    escalation_period_str = chkstr(obj.escalation_period_ptr->name);

  os << "hostescalation {\n"
    "  host_name:               " << chkstr(obj.host_name) << "\n"
    "  first_notification:      " << obj.first_notification << "\n"
    "  last_notification:       " << obj.last_notification << "\n"
    "  notification_interval:   " << obj.notification_interval << "\n"
    "  escalation_period:       " << chkstr(obj.escalation_period) << "\n"
    "  escalate_on_recovery:    " << obj.escalate_on_recovery << "\n"
    "  escalate_on_down:        " << obj.escalate_on_down << "\n"
    "  escalate_on_unreachable: " << obj.escalate_on_unreachable << "\n"
    "  contact_groups:          " << chkobj(obj.contact_groups) << "\n"
    "  contacts:                " << chkobj(obj.contacts) << "\n"
    "  host_ptr:                " << chkstr(hst_str) << "\n"
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
  shared_ptr<hostescalation> obj(new hostescalation, deleter::hostescalation);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->host_name = my_strdup(host_name);
    if (escalation_period)
      obj->escalation_period = my_strdup(escalation_period);
    obj->escalate_on_down = (escalate_on_down > 0);
    obj->escalate_on_recovery = (escalate_on_recovery > 0);
    obj->escalate_on_unreachable = (escalate_on_unreachable > 0);
    obj->first_notification = first_notification;
    obj->last_notification = last_notification;
    obj->notification_interval = (notification_interval <= 0) ? 0 : notification_interval;

    // Add new hostescalation to the monitoring engine.
    std::string id(host_name);
    umultimap<std::string, shared_ptr<hostescalation_struct> >::const_iterator
      it(state::instance().hostescalations().find(id));
    if (it != state::instance().hostescalations().end()) {
      logger(log_config_error, basic)
        << "Error: Hostescalation '" << host_name << "' has already been defined";
      return (NULL);
    }

    // Add new items to the configuration state.
    state::instance().hostescalations()
      .insert(std::make_pair(id, obj));

    // Add new items to the list.
    obj->next = hostescalation_list;
    hostescalation_list = obj.get();

    // Notify event broker.
    // XXX
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

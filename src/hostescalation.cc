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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/hostescalation.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

hostescalation_mmap hostescalation::hostescalations;

/**
 *  Create a new host escalation.
 *
 *  @param[in] host_name               Host name.
 *  @param[in] first_notification      First notification.
 *  @param[in] last_notification       Last notification.
 *  @param[in] notification_interval   Notification interval.
 *  @param[in] escalation_period       Escalation timeperiod name.
 *  @param[in] escalate_on_down        Escalate on down ?
 *  @param[in] escalate_on_unreachable Escalate on unreachable ?
 *  @param[in] escalate_on_recovery    Escalate on recovery ?
 */
hostescalation::hostescalation(std::string const& host_name,
                               int first_notification,
                               int last_notification,
                               double notification_interval,
                               std::string const& escalation_period,
                               bool escalate_on_down,
                               bool escalate_on_unreachable,
                               bool escalate_on_recovery)
    : escalation{first_notification, last_notification, notification_interval,
                 escalation_period} {
  if (host_name.empty())
    throw engine_error() << "Could not create escalation "
                          << "on host '" << host_name << "'";
  _escalate_on_down = (escalate_on_down > 0);
  _escalate_on_recovery = (escalate_on_recovery > 0);
  _escalate_on_unreachable = (escalate_on_unreachable > 0);
}

std::string const& hostescalation::get_hostname() const {
  return _host_name;
}

void hostescalation::set_hostname(std::string const& host_name) {
  _host_name = host_name;
}

std::string const& hostescalation::get_escalation_period() const {
  return _escalation_period;
}

void hostescalation::set_escalation_period(
  std::string const& escalation_period) {
  _escalation_period = escalation_period;
}

bool hostescalation::get_escalate_on_recovery() const {
  return _escalate_on_recovery;
}

void hostescalation::set_escalate_on_recovery(bool escalate_on_recovery) {
  _escalate_on_recovery = escalate_on_recovery;
}

bool hostescalation::get_escalate_on_down() const {
  return _escalate_on_down;
}

void hostescalation::set_escalate_on_down(bool escalate_on_down) {
  _escalate_on_down = escalate_on_down;
}

bool hostescalation::get_escalate_on_unreachable() const {
  return _escalate_on_unreachable;
}

void hostescalation::set_escalate_on_unreachable(bool escalate_on_unreachable) {
  _escalate_on_unreachable = escalate_on_unreachable;
}

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool hostescalation::operator==(
       hostescalation const& obj) throw () {
  return (_host_name == obj.get_hostname()
          && get_first_notification() == obj.get_first_notification()
          && get_last_notification() == obj.get_last_notification()
          && get_notification_interval() == obj.get_notification_interval()
          && _escalation_period == obj.get_escalation_period()
          && _escalate_on_recovery == obj.get_escalate_on_recovery()
          && _escalate_on_down == obj.get_escalate_on_down()
          && _escalate_on_unreachable == obj.get_escalate_on_unreachable()
          && ((contact_groups.size() == obj.contact_groups.size()) &&
               std::equal(contact_groups.begin(),
                          contact_groups.end(),
                          obj.contact_groups.begin()))
          && ((contacts.size() == obj.contacts.size()) &&
               std::equal(contacts.begin(),
                          contacts.end(),
                          obj.contacts.begin())));
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool hostescalation::operator!=(hostescalation const& obj) throw () {
  return !((*this) == obj);
}

/**
 *  Less-than operator.
 *
 *  @param[in] obj1 First object to compare.
 *  @param[in] obj2 Second object to compare.
 *
 *  @return True if the first object is less than the second.
 */
bool hostescalation::operator<(hostescalation const& obj) {
  if (_host_name != obj.get_hostname())
    return _host_name != obj.get_hostname();
  else if (_escalation_period !=  obj.get_escalation_period())
    return _escalation_period != obj.get_escalation_period();
  else if (get_first_notification() != obj.get_first_notification())
    return (get_first_notification() < obj.get_first_notification());
  else if (get_last_notification() != obj.get_last_notification())
    return (get_last_notification() < obj.get_last_notification());
  else if (get_notification_interval() != obj.get_notification_interval())
    return get_notification_interval() < obj.get_notification_interval();
  else if (_escalate_on_recovery != obj.get_escalate_on_recovery())
    return (_escalate_on_recovery < obj.get_escalate_on_recovery());
  else if (_escalate_on_down != obj.get_escalate_on_down())
    return (_escalate_on_down < obj.get_escalate_on_down());
  else if (_escalate_on_unreachable != obj.get_escalate_on_unreachable())
    return (_escalate_on_unreachable < obj.get_escalate_on_unreachable());
  for (contactgroup_map::const_iterator
         it1(contact_groups.begin()),
         it2(obj.contact_groups.begin()),
         end1(contact_groups.end()),
         end2(obj.contact_groups.end());
       (it1 != end1) || (it2 != end2);
       ++it1, ++it2) {
    if (it1->second == nullptr || it2->second == nullptr)
      return (!!it1->second < !!it2->second);
    else if (it1->second != it2->second)
      return (it1->second < it2->second);
  }
  for (contact_map::const_iterator
         it1(contacts.begin()),
         it2(obj.contacts.begin()),
         end1(contacts.end()),
         end2(obj.contacts.end());
       (it1 != end1) || (it2 != end2);
       ++it1, ++it2) {
    if (it1->second == nullptr || it2->second == nullptr)
      return (!!it1->second < !!it2->second);
    else if (it1->second != it2->second)
      return (it1->second < it2->second);
  }
  return false;
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
  char const* hst_str(nullptr);
  std::string escalation_period_str;
  if (obj.escalation_period_ptr)
    escalation_period_str = obj.escalation_period_ptr->get_name();

  std::string cg_oss;
  std::string c_oss;

  if (obj.contact_groups.empty())
    cg_oss = "\"nullptr\"";
  else {
    std::ostringstream oss;
    oss << obj.contact_groups;
    cg_oss = oss.str();
  }
  if (obj.contacts.empty())
    c_oss = "\"nullptr\"";
  else {
    std::ostringstream oss;
    oss << obj.contacts;
    c_oss = oss.str();
  }

  os << "hostescalation {\n"
    "  host_name:               " << obj.get_hostname() << "\n"
    "  first_notification:      " << obj.get_first_notification() << "\n"
    "  last_notification:       " << obj.get_last_notification() << "\n"
    "  notification_interval:   " << obj.get_notification_interval() << "\n"
    "  escalation_period:       " << obj.get_escalation_period() << "\n"
    "  escalate_on_recovery:    " << obj.get_escalate_on_recovery() << "\n"
    "  escalate_on_down:        " << obj.get_escalate_on_down() << "\n"
    "  escalate_on_unreachable: " << obj.get_escalate_on_unreachable() << "\n"
    "  contact_groups:          " << cg_oss << "\n"
    "  contacts:                " << c_oss << "\n"
    "  host_ptr:                " << (obj.host_ptr ?
                                        obj.host_ptr->get_name() :
                                        "\"nullptr\"") << "\n"
    "  escalation_period_ptr:   " << escalation_period_str << "\n"
    "}\n";
  return os;
}

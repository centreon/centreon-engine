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

#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/escalation.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/timeperiod.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

escalation::escalation(uint32_t first_notification,
                       uint32_t last_notification,
                       double notification_interval,
                       std::string const& escalation_period,
                       uint32_t escalate_on)
    : _first_notification{first_notification},
      _last_notification{last_notification},
      _notification_interval{
          (notification_interval < 0) ? 0 : notification_interval},
      _escalation_period{escalation_period},
      _escalate_on{escalate_on} {}

std::string const& escalation::get_escalation_period() const {
  return _escalation_period;
}

uint32_t escalation::get_first_notification() const {
  return _first_notification;
}

uint32_t escalation::get_last_notification() const {
  return _last_notification;
}

double escalation::get_notification_interval() const {
  return _notification_interval;
}

void escalation::set_notification_interval(double notification_interval) {
  _notification_interval = notification_interval;
}

void escalation::add_escalate_on(notifier::notification_flag type) {
  _escalate_on |= type;
}

void escalation::remove_escalate_on(notifier::notification_flag type) {
  _escalate_on &= ~type;
}

uint32_t escalation::get_escalate_on() const {
  return _escalate_on;
}

void escalation::set_escalate_on(uint32_t escalate_on) {
  _escalate_on = escalate_on;
}

bool escalation::get_escalate_on(notifier::notification_flag type) const {
  return _escalate_on & type;
}

contactgroup_map_unsafe const& escalation::contact_groups() const {
  return _contact_groups;
}

contactgroup_map_unsafe& escalation::contact_groups() {
  return _contact_groups;
}

/**
 *  This method is called by a notifier to know if this escalation is touched
 *  by the notification to send.
 *
 * @param state The notifier state.
 * @param notification_number The current notifier notification number.
 *
 * @return A boolean.
 */
bool escalation::is_viable(int state, uint32_t notification_number) const {
  std::time_t current_time;
  std::time(&current_time);

  /* In case of a recovery, we are interested by the last notification */
  uint32_t number{state == 0 ?
    notification_number - 1 : notification_number};

  /* Skip this escalation if current_time is outside its timeperiod */
  if (!get_escalation_period().empty() &&
      !check_time_against_period(current_time, escalation_period_ptr))
    return false;

  if (number < _first_notification ||
      (number > _last_notification && _last_notification != 0))
    return false;
  return true;
}

void escalation::resolve(int& w __attribute__((unused)), int& e) {
  int errors{0};
  // Find the timeperiod.
  if (!get_escalation_period().empty()) {
    timeperiod_map::const_iterator
      it{timeperiod::timeperiods.find(get_escalation_period())};

    if (it == timeperiod::timeperiods.end() || !it->second) {
      logger(log_verification_error, basic)
        << "Error: Escalation period '" << get_escalation_period()
        << "' specified in escalation is not defined anywhere!";
      errors++;
    }
    else
      // Save the timeperiod pointer for later.
      escalation_period_ptr = it->second.get();
  }

  // Check all contact groups.
  for (contactgroup_map_unsafe::iterator
         it{contact_groups().begin()},
         end{contact_groups().end()};
       it != end;
       ++it) {
    // Find the contact group.
    contactgroup_map::iterator it_cg{contactgroup::contactgroups.find(it->first)};

    if (it_cg == contactgroup::contactgroups.end() || !it_cg->second) {
      logger(log_verification_error, basic)
        << "Error: Contact group '"
        << it->first
        << "' specified in escalation for this notifier is not defined anywhere!";
      errors++;
    } else {
      // Save the contactgroup pointer for later.
      it->second = it_cg->second.get();
    }
  }

  if (errors) {
    e += errors;
    throw engine_error() << "Cannot resolve notifier escalation";
  }
}

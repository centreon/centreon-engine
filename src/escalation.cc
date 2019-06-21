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

#include "com/centreon/engine/escalation.hh"
#include "com/centreon/engine/timeperiod.hh"

using namespace com::centreon::engine;

escalation::escalation(int first_notification,
                       int last_notification,
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

int escalation::get_first_notification() const {
  return _first_notification;
}

int escalation::get_last_notification() const {
  return _last_notification;
}

double escalation::get_notification_interval() const {
  return _notification_interval;
}

void escalation::set_notification_interval(double notification_interval) {
  _notification_interval = notification_interval;
}

void escalation::add_escalate_on(notifier::notification_type type) {
  _escalate_on |= type;
}

void escalation::remove_escalate_on(notifier::notification_type type) {
  _escalate_on &= ~type;
}

uint32_t escalation::get_escalate_on() const {
  return _escalate_on;
}

void escalation::set_escalate_on(uint32_t escalate_on) {
  _escalate_on = escalate_on;
}

bool escalation::get_escalate_on(notifier::notification_type type) const {
  return _escalate_on & type;
}

contact_map_unsafe const& escalation::contacts() const {
  return _contacts;
}

contact_map_unsafe& escalation::contacts() {
  return _contacts;
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
bool escalation::is_viable(int state, int notification_number) const {
  std::time_t current_time;
  std::time(&current_time);

  /* In case of a recovery, we are interested by the last notification */
  int number{state == 0 ?
    notification_number - 1 : notification_number};

  /* Skip this escalation if current_time is outside its timeperiod */
  if (!get_escalation_period().empty() &&
      !check_time_against_period(current_time, escalation_period_ptr))
    return false;

  if (number < get_first_notification() || number > get_last_notification())
    return false;
  return true;
}

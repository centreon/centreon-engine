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

contact_map const& escalation::contacts() const {
  return _contacts;
}

contact_map& escalation::contacts() {
  return _contacts;
}

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
                       std::string const& escalation_period)
    : _first_notification{first_notification},
      _last_notification{last_notification},
      _notification_interval{
          (notification_interval < 0) ? 0 : notification_interval},
      _escalation_period{escalation_period} {}

std::string const& escalation::get_escalation_period() const {
  return _escalation_period;
}

int escalation::get_first_notification() const {
  return _first_notification;
}

void escalation::set_first_notification(int first_notification) {
  _first_notification = first_notification;
}

int escalation::get_last_notification() const {
  return _last_notification;
}

void escalation::set_last_notification(int last_notification) {
  _last_notification = last_notification;
}

double escalation::get_notification_interval() const {
  return _notification_interval;
}

void escalation::set_notification_interval(double notification_interval) {
  _notification_interval = notification_interval;
}


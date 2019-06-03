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

#ifndef CCE_ESCALATION_HH
#define CCE_ESCALATION_HH

#include <string>
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/contactgroup.hh"
#include "com/centreon/engine/namespace.hh"

/* Forward declaration. */
CCE_BEGIN()
class service;
class timeperiod;

class escalation {
 public:
  escalation(int first_notification,
             int last_notification,
             double notification_interval,
             std::string const& escalation_period);

  std::string const& get_escalation_period() const;
  int get_first_notification() const;
  void set_first_notification(int first_notification);
  int get_last_notification() const;
  void set_last_notification(int last_notification);
  double get_notification_interval() const;
  void set_notification_interval(double notification_interval);

 private:
  int _first_notification;
  int _last_notification;
  double _notification_interval;
  std::string _escalation_period;
};
CCE_END()

#endif  // !CCE_ESCALATION_HH

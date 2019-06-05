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
#include "com/centreon/engine/notifier.hh"

/* Forward declaration. */
CCE_BEGIN()
class service;
class timeperiod;

class escalation {
 public:
  escalation(int first_notification,
             int last_notification,
             double notification_interval,
             std::string const& escalation_period,
             uint32_t escalate_on);

  std::string const& get_escalation_period() const;
  int get_first_notification() const;
  void set_first_notification(int first_notification);
  int get_last_notification() const;
  void set_last_notification(int last_notification);
  double get_notification_interval() const;
  void set_notification_interval(double notification_interval);
  void add_escalate_on(notifier::notification_type type);
  void remove_escalate_on(notifier::notification_type type);
  uint32_t get_escalate_on() const;
  bool get_escalate_on(notifier::notification_type type) const;
  void set_escalate_on(uint32_t escalate_on);
  contact_map const& contacts() const;
  contact_map& contacts();
  contactgroup_map contact_groups;
  notifier* notifier_ptr;
  timeperiod* escalation_period_ptr;

 private:
  int _first_notification;
  int _last_notification;
  double _notification_interval;
  std::string _escalation_period;
  uint32_t _escalate_on;
  contact_map _contacts;
};
CCE_END()

#endif  // !CCE_ESCALATION_HH

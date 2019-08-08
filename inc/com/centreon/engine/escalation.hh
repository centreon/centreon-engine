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
#include "com/centreon/engine/contactgroup.hh"
#include "com/centreon/engine/namespace.hh"
#include "com/centreon/engine/notifier.hh"
#include "com/centreon/engine/shared.hh"

/* Forward declaration. */
CCE_BEGIN()
class timeperiod;

class escalation {
 public:
  escalation(uint32_t first_notification,
             uint32_t last_notification,
             double notification_interval,
             std::string const& escalation_period,
             uint32_t escalate_on,
             Uuid const& uuid);

  std::string const& get_escalation_period() const;
  uint32_t get_first_notification() const;
  uint32_t get_last_notification() const;
  double get_notification_interval() const;
  void set_notification_interval(double notification_interval);
  void add_escalate_on(notifier::notification_flag type);
  void remove_escalate_on(notifier::notification_flag type);
  uint32_t get_escalate_on() const;
  bool get_escalate_on(notifier::notification_flag type) const;
  void set_escalate_on(uint32_t escalate_on);
  virtual bool is_viable(int state, uint32_t notification_number) const;
  Uuid const& get_uuid() const;

  contactgroup_map_unsafe const& get_contactgroups() const;
  contactgroup_map_unsafe& get_contactgroups();
  virtual void resolve(int& w, int& e);

  notifier* notifier_ptr;
  timeperiod* escalation_period_ptr;

 private:
  uint32_t _first_notification;
  uint32_t _last_notification;
  double _notification_interval;
  std::string _escalation_period;
  uint32_t _escalate_on;
  contactgroup_map_unsafe _contact_groups;
  Uuid _uuid;
};
CCE_END()

#endif  // !CCE_ESCALATION_HH

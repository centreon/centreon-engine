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

#ifndef CCE_OBJECTS_HOSTESCALATION_HH
#  define CCE_OBJECTS_HOSTESCALATION_HH

/* Forward declaration. */
struct contactgroupsmember_struct;
struct contactsmember_struct;
struct host_struct;
struct timeperiod_struct;

typedef struct                   hostescalation_struct {
  char*                          host_name;
  int                            first_notification;
  int                            last_notification;
  double                         notification_interval;
  char*                          escalation_period;
  int                            escalate_on_recovery;
  int                            escalate_on_down;
  int                            escalate_on_unreachable;
  contactgroupsmember_struct*    contact_groups;
  contactsmember_struct*         contacts;
  host_struct*                   host_ptr;
  timeperiod_struct*             escalation_period_ptr;
  struct hostescalation_struct*  next;
  struct hostescalation_struct*  nexthash;
}                                hostescalation;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

hostescalation* add_host_escalation(
                  char const* host_name,
                  int first_notification,
                  int last_notification,
                  double notification_interval,
                  char const* escalation_period,
                  int escalate_on_down,
                  int escalate_on_unreachable,
                  int escalate_on_recovery);

#  ifdef __cplusplus
}

#    include <ostream>

bool          operator==(
                hostescalation const& obj1,
                hostescalation const& obj2) throw ();
bool          operator!=(
                hostescalation const& obj1,
                hostescalation const& obj2) throw ();
bool          operator<(
                hostescalation const& obj1,
                hostescalation const& obj2);
std::ostream& operator<<(std::ostream& os, hostescalation const& obj);

#  endif /* C++ */

#endif // !CCE_OBJECTS_HOSTESCALATION_HH



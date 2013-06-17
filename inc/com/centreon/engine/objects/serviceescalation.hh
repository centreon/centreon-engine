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

#ifndef CCE_OBJECTS_SERVICEESCALATION_HH
#  define CCE_OBJECTS_SERVICEESCALATION_HH

/* Forward declaration. */
struct contactgroupsmember_struct;
struct contactsmember_struct;
struct service_struct;
struct timeperiod_struct;

typedef struct                     serviceescalation_struct {
  char*                            host_name;
  char*                            description;
  int                              first_notification;
  int                              last_notification;
  double                           notification_interval;
  char*                            escalation_period;
  int                              escalate_on_recovery;
  int                              escalate_on_warning;
  int                              escalate_on_unknown;
  int                              escalate_on_critical;
  contactgroupsmember_struct*      contact_groups;
  contactsmember_struct*           contacts;
  service_struct*                  service_ptr;
  timeperiod_struct*               escalation_period_ptr;
  struct serviceescalation_struct* next;
  struct serviceescalation_struct* nexthash;
}                                  serviceescalation;

#  ifdef __cplusplus
#    include <ostream>

bool          operator==(
                serviceescalation const& obj1,
                serviceescalation const& obj2) throw ();
bool          operator!=(
                serviceescalation const& obj1,
                serviceescalation const& obj2) throw ();
std::ostream& operator<<(
                std::ostream& os,
                serviceescalation const& obj);

#  endif // C++

#endif // !CCE_OBJECTS_SERVICEESCALATION_HH



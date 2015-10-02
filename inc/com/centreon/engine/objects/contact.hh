/*
** Copyright 2011-2013,2015 Merethis
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

#ifndef CCE_OBJECTS_CONTACT_HH
#  define CCE_OBJECTS_CONTACT_HH

#  include <string>
#  include <time.h>

/* Max number of custom addresses a contact can have. */
#  define MAX_CONTACT_ADDRESSES 6

/* Forward declaration. */
struct commandsmember_struct;
struct customvariablesmember_struct;
struct timeperiod_struct;
struct objectlist_struct;

typedef struct                  contact_struct {
  char*                         name;
  char*                         alias;
  char*                         email;
  char*                         pager;
  char*                         address[MAX_CONTACT_ADDRESSES];
  commandsmember_struct*        host_notification_commands;
  commandsmember_struct*        service_notification_commands;
  int                           notify_on_service_unknown;
  int                           notify_on_service_warning;
  int                           notify_on_service_critical;
  int                           notify_on_service_recovery;
  int                           notify_on_service_flapping;
  int                           notify_on_service_downtime;
  int                           notify_on_host_down;
  int                           notify_on_host_unreachable;
  int                           notify_on_host_recovery;
  int                           notify_on_host_flapping;
  int                           notify_on_host_downtime;
  char*                         host_notification_period;
  char*                         service_notification_period;
  int                           host_notifications_enabled;
  int                           service_notifications_enabled;
  int                           can_submit_commands;
  int                           retain_status_information;
  int                           retain_nonstatus_information;
  customvariablesmember_struct* custom_variables;
  time_t                        last_host_notification;
  time_t                        last_service_notification;
  unsigned long                 modified_attributes;
  unsigned long                 modified_host_attributes;
  unsigned long                 modified_service_attributes;

  timeperiod_struct*            host_notification_period_ptr;
  timeperiod_struct*            service_notification_period_ptr;
  objectlist_struct*            contactgroups_ptr;
  struct contact_struct*        next;
  struct contact_struct*        nexthash;
}                               contact;

/* Other CONTACT structure. */
struct                          contact_other_properties {
  std::string                   timezone;
};

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

contact* add_contact(
           char const* name,
           char const* alias,
           char const* email,
           char const* pager,
           char const* const* addresses,
           char const* svc_notification_period,
           char const* host_notification_period,
           int notify_service_ok,
           int notify_service_critical,
           int notify_service_warning,
           int notify_service_unknown,
           int notify_service_flapping,
           int notify_service_downtime,
           int notify_host_up,
           int notify_host_down,
           int notify_host_unreachable,
           int notify_host_flapping,
           int notify_host_downtime,
           int host_notifications_enabled,
           int service_notifications_enabled,
           int can_submit_commands,
           int retain_status_information,
           int retain_nonstatus_information);

#  ifdef __cplusplus
}

#    include <ostream>
#    include <string>
#    include "com/centreon/engine/namespace.hh"

bool          operator==(
                contact const& obj1,
                contact const& obj2) throw ();
bool          operator!=(
                contact const& obj1,
                contact const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, contact const& obj);

CCE_BEGIN()

contact&      find_contact(std::string const& name);
char const*   get_contact_timezone(char const* name);
bool          is_contact_exist(std::string const& name) throw ();

CCE_END()

#  endif /* C++ */

#endif // !CCE_OBJECTS_CONTACT_HH

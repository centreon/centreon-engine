/*
** Copyright 1999-2008 Ethan Galstad
** Copyright 2009-2010 Nagios Core Development Team and Community Contributors
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

#ifndef CCE_NOTIFICATIONS_HH
#  define CCE_NOTIFICATIONS_HH

#  include <sys/time.h>
#  include "com/centreon/engine/macros/defines.hh"
#  include "com/centreon/engine/objects/contact.hh"
#  include "com/centreon/engine/objects/host.hh"
#  include "com/centreon/engine/objects/hostescalation.hh"
#  include "com/centreon/engine/objects/service.hh"
#  include "com/centreon/engine/objects/serviceescalation.hh"

// Notification Types
#  define HOST_NOTIFICATION               0
#  define SERVICE_NOTIFICATION            1

// Notification Reason Types
#  define NOTIFICATION_NORMAL             0
#  define NOTIFICATION_ACKNOWLEDGEMENT    1
#  define NOTIFICATION_FLAPPINGSTART      2
#  define NOTIFICATION_FLAPPINGSTOP       3
#  define NOTIFICATION_FLAPPINGDISABLED   4
#  define NOTIFICATION_DOWNTIMESTART      5
#  define NOTIFICATION_DOWNTIMEEND        6
#  define NOTIFICATION_DOWNTIMECANCELLED  7
#  define NOTIFICATION_CUSTOM             99

// NOTIFY_LIST structure
typedef struct               notify_list_struct {
  contact*                   cntct;
  struct notify_list_struct* next;
}                            notification;

#  ifdef __cplusplus
extern "C" {
#  endif // C++

// Notification Functions

// notify all contacts about a service (problem or recovery)
int service_notification(
      service* svc,
      unsigned int type,
      char* not_author,
      char* not_data,
      int options);
// checks viability of notifying all contacts about a service
int check_service_notification_viability(
      service* svc,
      unsigned int type,
      int options);
// checks viability of notifying a contact about a service
int check_contact_service_notification_viability(
      contact* cntct,
      service* svc,
      unsigned int type,
      int options);
// notify a single contact about a service
int notify_contact_of_service(
      nagios_macros* mac,
      contact* cntct,
      service* svc,
      int type,
      char* not_author,
      char* not_data,
      int options,
      int escalated);
// checks if an escalation entry is valid for a particular service notification
int is_valid_escalation_for_service_notification(
      service* svc,
      serviceescalation* se,
      int options);
// checks if a service notification should be escalated
int should_service_notification_be_escalated(service* svc);
// given a service, create list of contacts to be notified (remove duplicates)
int create_notification_list_from_service(
      nagios_macros* mac,
      service* svc,
      int options,
      int* escalated);
// notify all contacts about a host (problem or recovery)
int host_notification(
      host* hst,
      unsigned int type,
      char* not_author,
      char* not_data,
      int options);
// checks viability of notifying all contacts about a host
int check_host_notification_viability(
      host* hst,
      unsigned int type,
      int options);
// checks viability of notifying a contact about a host
int check_contact_host_notification_viability(
      contact* cntct,
      host* hst,
      unsigned int type,
      int options);
// notify a single contact about a host
int notify_contact_of_host(
      nagios_macros* mac,
      contact* cntct,
      host* hst,
      int type,
      char* not_author,
      char* not_data,
      int options,
      int escalated);
// checks if an escalation entry is valid for a particular host notification
int is_valid_escalation_for_host_notification(
      host* hst,
      hostescalation* he,
      int options);
// checks if a host notification should be escalated
int should_host_notification_be_escalated(host* hst);
// given a host, create list of contacts to be notified (remove duplicates)
int create_notification_list_from_host(
      nagios_macros* mac,
      host* hst,
      int options,
      int* escalated);
// calculates nex acceptable re-notification time for a service
time_t get_next_service_notification_time(service* svc, time_t offset);
// calculates nex acceptable re-notification time for a host
time_t get_next_host_notification_time(host* hst, time_t offset);
// finds a notification object
notification* find_notification(contact* cntct);
// adds a notification instance
int add_notification(nagios_macros* mac, contact* cntct);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_NOTIFICATIONS_HH

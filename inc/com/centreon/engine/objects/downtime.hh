/*
** Copyright 2000-2008      Ethan Galstad
** Copyright 2011-2013,2016 Centreon
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

#ifndef CCE_OBJECTS_DOWTIME_HH
#  define CCE_OBJECTS_DOWTIME_HH

#  include <time.h>
#  include "com/centreon/engine/objects/host.hh"
#  include "com/centreon/engine/objects/service.hh"

typedef struct                      scheduled_downtime_struct {
  int                               type;
  char*                             host_name;
  char*                             service_description;
  time_t                            entry_time;
  time_t                            start_time;
  time_t                            end_time;
  int                               fixed;
  unsigned long                     triggered_by;
  unsigned long                     duration;
  unsigned long                     downtime_id;
  char*                             author;
  char*                             comment;
  unsigned long                     comment_id;
  int                               is_in_effect;
  int                               start_flex_downtime;
  int                               incremented_pending_downtime;
  struct scheduled_downtime_struct* next;
}                                   scheduled_downtime;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

int                 add_downtime(
                      int downtime_type,
                      char const* host_name,
                      char const* svc_description,
                      time_t entry_time,
                      char const* author,
                      char const* comment_data,
                      time_t start_time,
                      time_t end_time,
                      int fixed,
                      unsigned long triggered_by,
                      unsigned long duration,
                      unsigned long downtime_id);
int                 add_host_downtime(
                      char const* host_name,
                      time_t entry_time,
                      char const* author,
                      char const* comment_data,
                      time_t start_time,
                      time_t end_time,
                      int fixed,
                      unsigned long triggered_by,
                      unsigned long duration,
                      unsigned long downtime_id);
int                 add_new_downtime(
                      int type,
                      char const* host_name,
                      char const* service_description,
                      time_t entry_time,
                      char const* author,
                      char const* comment_data,
                      time_t start_time,
                      time_t end_time,
                      int fixed,
                      unsigned long triggered_by,
                      unsigned long duration,
                      unsigned long* downtime_id);
int                 add_new_host_downtime(
                      char const* host_name,
                      time_t entry_time,
                      char const* author,
                      char const* comment_data,
                      time_t start_time,
                      time_t end_time,
                      int fixed,
                      unsigned long triggered_by,
                      unsigned long duration,
                      unsigned long* downtime_id);
int                 add_new_service_downtime(
                      char const* host_name,
                      char const* service_description,
                      time_t entry_time,
                      char const* author,
                      char const* comment_data,
                      time_t start_time,
                      time_t end_time,
                      int fixed,
                      unsigned long triggered_by,
                      unsigned long duration,
                      unsigned long* downtime_id);
int                 add_service_downtime(
                      char const* host_name,
                      char const* svc_description,
                      time_t entry_time,
                      char const* author,
                      char const* comment_data,
                      time_t start_time,
                      time_t end_time,
                      int fixed,
                      unsigned long triggered_by,
                      unsigned long duration,
                      unsigned long downtime_id);
int                 check_for_expired_downtime();
int                 check_pending_flex_host_downtime(
                      host_struct* hst);
int                 check_pending_flex_service_downtime(
                      service_struct* svc);
int                 delete_downtime(
                      int type,
                      unsigned long downtime_id);
int                 delete_downtime_by_hostname_service_description_start_time_comment(
                      char const* hostname,
                      char const* service_description,
                      time_t start_time,
                      char const* comment);
int                 delete_host_downtime(unsigned long downtime_id);
int                 delete_service_downtime(
                      unsigned long downtime_id);
scheduled_downtime* find_downtime(
                      int type,
                      unsigned long downtime_id);
scheduled_downtime* find_host_downtime(unsigned long downtime_id);
scheduled_downtime* find_service_downtime(unsigned long downtime_id);
void                free_downtime_data();
int                 handle_scheduled_downtime(
                      scheduled_downtime* temp_downtime);
int                 handle_scheduled_downtime_by_id(
                      unsigned long downtime_id);
int                 initialize_downtime_data();
int                 register_downtime(
                      int type,
                      unsigned long downtime_id);
int                 schedule_downtime(
                      int type,
                      char const* host_name,
                      char const* service_description,
                      time_t entry_time,
                      char const* author,
                      char const* comment_data,
                      time_t start_time,
                      time_t end_time,
                      int fixed,
                      unsigned long triggered_by,
                      unsigned long duration,
                      unsigned long* new_downtime_id);
int                 sort_downtime();
int                 unschedule_downtime(
                      int type,
                      unsigned long downtime_id);
#  ifdef __cplusplus
}

#    include <ostream>
#    include "com/centreon/engine/namespace.hh"

bool          operator==(
                scheduled_downtime const& obj1,
                scheduled_downtime const& obj2) throw ();
bool          operator!=(
                scheduled_downtime const& obj1,
                scheduled_downtime const& obj2) throw ();
std::ostream& operator<<(
                std::ostream& os,
                scheduled_downtime const& obj);

#  endif /* C++ */

#endif // !CCE_OBJECTS_DOWTIME_HH

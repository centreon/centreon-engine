/*
** Copyright 1999-2010 Ethan Galstad
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

#ifndef CCE_OBJECTS_COMMENTS_HH
#  define CCE_OBJECTS_COMMENTS_HH

#  include <time.h>
#  include "com/centreon/engine/objects/contact.hh"
#  include "com/centreon/engine/objects/host.hh"
#  include "com/centreon/engine/objects/service.hh"

// Comment sources.
#  define COMMENTSOURCE_INTERNAL  0
#  define COMMENTSOURCE_EXTERNAL  1

// Comment types.
#  define HOST_COMMENT		 1
#  define SERVICE_COMMENT	 2

// Entry types.
#  define USER_COMMENT            1
#  define DOWNTIME_COMMENT        2
#  define FLAPPING_COMMENT        3
#  define ACKNOWLEDGEMENT_COMMENT 4

// Chained hash limits.
#  define COMMENT_HASHSLOTS       1024

// COMMENT structure
typedef struct           comment_struct {
  unsigned int           comment_type;
  unsigned int           entry_type;
  unsigned long          comment_id;
  int                    source;
  int                    persistent;
  time_t 	         entry_time;
  int                    expires;
  time_t                 expire_time;
  char*	                 host_name;
  char*	                 service_description;
  char*	                 author;
  char*	                 comment_data;
  struct comment_struct* next;
  struct comment_struct* nexthash;
}                        comment;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

int      add_comment(
           unsigned int comment_type,
           int entry_type,
           char const* host_name,
           char const* svc_description,
           time_t entry_time,
           char const* author,
           char const* comment_data,
           unsigned long comment_id,
           int persistent,
           int expires,
           time_t expire_time,
           int source);
int      add_comment_to_hashlist(comment* new_comment);
int      add_host_comment(
           int entry_type,
           char const* host_name,
           time_t entry_time,
           char const* author,
           char const* comment_data,
           unsigned long comment_id,
           int persistent,
           int expires,
           time_t expire_time,
           int source);
int      add_new_comment(
           unsigned int type,
           int entry_type,
           char const* host_name,
           char const* svc_description,
           time_t entry_time,
           char const* author_name,
           char const* comment_data,
           int persistent,
           int source,
           int expires,
           time_t expire_time,
           unsigned long* comment_id);
int      add_new_host_comment(
           int entry_type,
           char const* host_name,
           time_t entry_time,
           char const* author_name,
           char const* comment_data,
           int persistent,
           int source,
           int expires,
           time_t expire_time,
           unsigned long* comment_id);
int      add_new_service_comment(
           int entry_type,
           char const* host_name,
           char const* svc_description,
           time_t entry_time,
           char const* author_name,
           char const* comment_data,
           int persistent,
           int source,
           int expires,
           time_t expire_time,
           unsigned long* comment_id);
int      add_service_comment(
           int entry_type,
           char const* host_name,
           char const* svc_description,
           time_t entry_time,
           char const* author,
           char const* comment_data,
           unsigned long comment_id,
           int persistent,
           int expires,
           time_t expire_time,
           int source);
int      check_for_expired_comment(unsigned long comment_id);
int      delete_all_comments(
           unsigned int type,
           char const* host_name,
           char const* svc_description);
int      delete_all_host_comments(char const* host_name);
int      delete_all_service_comments(
           char const* host_name,
           char const* svc_description);
int      delete_comment(unsigned int type, unsigned long comment_id);
int      delete_host_acknowledgement_comments(host* hst);
int      delete_host_comment(unsigned long comment_id);
int      delete_service_acknowledgement_comments(service* svc);
int      delete_service_comment(unsigned long comment_id);
comment* find_comment(
           unsigned long comment_id,
           unsigned int comment_type);
comment* find_host_comment(unsigned long comment_id);
comment* find_service_comment(unsigned long comment_id);
void     free_comment_data();
comment* get_first_comment_by_host(char const* host_name);
comment* get_next_comment_by_host(
           char const* host_name,
           comment* start);
int      initialize_comment_data();
int      number_of_host_comments(char const* host_name);
int      number_of_service_comments(
           char const* host_name,
           char const* svc_description);
int      sort_comments();

#  ifdef __cplusplus
}

#    include <ostream>
#    include "com/centreon/engine/namespace.hh"

bool          operator==(
                comment_struct const& obj1,
                comment_struct const& obj2) throw ();
bool          operator!=(
                comment_struct const& obj1,
                comment_struct const& obj2) throw ();
std::ostream& operator<<(
                std::ostream& os,
                comment_struct const& obj);

#  endif /* C++ */

#endif // !CCE_OBJECTS_COMMENTS_HH

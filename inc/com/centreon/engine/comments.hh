/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011-2012 Merethis
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

#ifndef CCE_COMMENTS_HH
#  define CCE_COMMENTS_HH

#  include <time.h>
#  include "com/centreon/engine/objects.hh"

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
#  endif // C++

int initialize_comment_data(char const* config_file);                            // initializes comment data
int cleanup_comment_data(char const* config_file);                               // cleans up comment data
int add_new_comment(unsigned int type, int entry_type, char const* host_name, char const* svc_description, time_t entry_time, char const* author_name, char const* comment_data, int persistent, int source, int expires, time_t expire_time, unsigned long* comment_id);                      // adds a new host or service comment
int add_new_host_comment(int entry_type, char const* host_name, time_t entry_time, char const* author_name, char const* comment_data, int persistent, int source, int expires, time_t expire_time, unsigned long* comment_id); // adds a new host comment
int add_new_service_comment(int entry_type, char const* host_name, char const* svc_description, time_t entry_time, char const* author_name, char const* comment_data, int persistent, int source, int expires, time_t expire_time, unsigned long* comment_id);                                  // adds a new service comment
int delete_comment(unsigned int type, unsigned long comment_id);           // deletes a host or service comment
int delete_host_comment(unsigned long comment_id);                         // deletes a host comment
int delete_service_comment(unsigned long comment_id);                      // deletes a service comment
int delete_all_comments(unsigned int type, char const* host_name, char const* svc_description); // deletes all comments for a particular host or service
int delete_all_host_comments(char const* host_name);                             // deletes all comments for a specific host
int delete_host_acknowledgement_comments(host* hst);                       // deletes all non-persistent ack comments for a specific host
int delete_all_service_comments(char const* host_name, char const* svc_description);   // deletes all comments for a specific service
int delete_service_acknowledgement_comments(service* svc);                 // deletes all non-persistent ack comments for a specific service
int check_for_expired_comment(unsigned long comment_id);                   // expires a comment

/*
** If you are going to be adding a lot of comments in sequence, set
** defer_comment_sorting to 1 before you start and then call
** sort_comments afterwards. Things will go MUCH faster.
**
** extern int defer_comment_sorting;
*/

int add_comment_to_hashlist(comment* new_comment);
int add_host_comment(int entry_type, char const* host_name, time_t entry_time, char const* author, char const* comment_data, unsigned long comment_id, int persistent, int expires, time_t expire_time, int source); // adds a host comment
int add_service_comment(int entry_type, char const* host_name, char const* svc_description, time_t entry_time, char const* author, char const* comment_data, unsigned long comment_id, int persistent, int expires, time_t expire_time, int source); // adds a service comment
int add_comment(unsigned int comment_type, int entry_type, char const* host_name, char const* svc_description, time_t entry_time, char const* author, char const* comment_data, unsigned long comment_id, int persistent, int expires, time_t expire_time, int source); // adds a comment (host or service)
int sort_comments(void);
void free_comment_data(void);                                               // frees memory allocated to the comment list
int number_of_host_comments(char const* host_name);                               // returns the number comments associated with a particular host
int number_of_service_comments(char const* host_name, char const* svc_description);     // returns the number of comments associated with a particular service
comment* get_first_comment_by_host(char const* host_name);
comment* get_next_comment_by_host(char const* host_name, comment* start);
comment* find_service_comment(unsigned long comment_id);                    // finds a specific service comment
comment* find_host_comment(unsigned long comment_id);                       // finds a specific host comment
comment* find_comment(unsigned long comment_id, unsigned int comment_type); // finds a specific comment

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_COMMENTS_HH

/*
** Copyright 1999-2006 Ethan Galstad
** Copyright 2011      Merethis
**
** This file is part of Centreon Scheduler.
**
** Centreon Scheduler is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Scheduler is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Scheduler. If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef SCHEDULER_COMMENTS_HH
# define SCHEDULER_COMMENTS_HH

# include "config.hh"
# include "common.hh"
# include "objects.hh"


# ifdef __cplusplus
  extern "C" {
# endif

/**************************** COMMENT SOURCES ******************************/

# define COMMENTSOURCE_INTERNAL  0
# define COMMENTSOURCE_EXTERNAL  1



/***************************** COMMENT TYPES *******************************/

# define HOST_COMMENT			1
# define SERVICE_COMMENT		2


/****************************** ENTRY TYPES ********************************/

# define USER_COMMENT                    1
# define DOWNTIME_COMMENT                2
# define FLAPPING_COMMENT                3
# define ACKNOWLEDGEMENT_COMMENT         4


/*************************** CHAINED HASH LIMITS ***************************/

# define COMMENT_HASHSLOTS      1024



/**************************** DATA STRUCTURES ******************************/


/* COMMENT structure */
typedef struct comment_struct{
	int 	comment_type;
	int     entry_type;
	unsigned long comment_id;
	int     source;
	int     persistent;
	time_t 	entry_time;
	int     expires;
	time_t  expire_time;
	char 	*host_name;
	char 	*service_description;
	char 	*author;
	char 	*comment_data;
	struct 	comment_struct *next;
	struct 	comment_struct *nexthash;
        }comment;


int initialize_comment_data(char *);                                /* initializes comment data */
int cleanup_comment_data(char *);                                   /* cleans up comment data */
int add_new_comment(int,int,char const *,char const *,time_t,char const *,char *,int,int,int,time_t,unsigned long *);       /* adds a new host or service comment */
int add_new_host_comment(int,char const *,time_t,char const *,char *,int,int,int,time_t,unsigned long *);             /* adds a new host comment */
int add_new_service_comment(int,char const *,char const *,time_t,char const *,char *,int,int,int,time_t,unsigned long *);   /* adds a new service comment */
int delete_comment(int,unsigned long);                              /* deletes a host or service comment */
int delete_host_comment(unsigned long);                             /* deletes a host comment */
int delete_service_comment(unsigned long);                          /* deletes a service comment */
int delete_all_comments(int,char *,char *);                         /* deletes all comments for a particular host or service */
int delete_all_host_comments(char *);                               /* deletes all comments for a specific host */
int delete_host_acknowledgement_comments(host *);                   /* deletes all non-persistent ack comments for a specific host */
int delete_all_service_comments(char *,char *);                     /* deletes all comments for a specific service */
int delete_service_acknowledgement_comments(service *);             /* deletes all non-persistent ack comments for a specific service */

int check_for_expired_comment(unsigned long);                       /* expires a comment */

comment *find_comment(unsigned long,int);                             /* finds a specific comment */
comment *find_service_comment(unsigned long);                         /* finds a specific service comment */
comment *find_host_comment(unsigned long);                            /* finds a specific host comment */

comment *get_first_comment_by_host(char *);
comment *get_next_comment_by_host(char *,comment *);

int number_of_host_comments(char *);			              /* returns the number of comments associated with a particular host */
int number_of_service_comments(char *, char *);		              /* returns the number of comments associated with a particular service */

/* If you are going to be adding a lot of comments in sequence, set
   defer_comment_sorting to 1 before you start and then call
   sort_comments afterwards. Things will go MUCH faster. */

extern int defer_comment_sorting;
int add_comment(int,int,char const *,char const *,time_t,char const *,char *,unsigned long,int,int,time_t,int);      /* adds a comment (host or service) */
int sort_comments(void);
int add_host_comment(int,char const *,time_t,char const *,char *,unsigned long,int,int,time_t,int);            /* adds a host comment */
int add_service_comment(int,char const *,char const *,time_t,char const *,char *,unsigned long,int,int,time_t,int);  /* adds a service comment */

int add_comment_to_hashlist(comment *);

void free_comment_data(void);                                             /* frees memory allocated to the comment list */

# ifdef __cplusplus
  }
# endif

#endif // !SCHEDULER_COMMENTS_HH

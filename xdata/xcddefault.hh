/*
** Copyright 2000-2006 Ethan Galstad
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

#ifndef SCHEDULER_XCDDEFAULT_HH
# define SCHEDULER_XCDDEFAULT_HH

int xcddefault_initialize_comment_data(char *);
int xcddefault_cleanup_comment_data(char *);
int xcddefault_save_comment_data(void);
int xcddefault_add_new_host_comment(int,char const *,time_t,char const *,char *,int,int,int,time_t,unsigned long *);
int xcddefault_add_new_service_comment(int,char const *,char const *,time_t,char const *,char *,int,int,int,time_t,unsigned long *);
int xcddefault_delete_host_comment(unsigned long);
int xcddefault_delete_service_comment(unsigned long);

#endif // !SCHEDULER_XCDDEFAULT_HH

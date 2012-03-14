/*
** Copyright 2000-2006 Ethan Galstad
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

#ifndef CCE_XCDDEFAULT_HH
#  define CCE_XCDDEFAULT_HH

#  include <time.h>

#  ifdef __cplusplus
extern "C" {
#  endif // C++

int xcddefault_initialize_comment_data(char const* main_config_file);
int xcddefault_cleanup_comment_data(char const* main_config_file);
int xcddefault_save_comment_data();
int xcddefault_add_new_host_comment(int entry_type, char const* host_name, time_t entry_time, char const* author_name, char const* comment_data, int persistent, int source, int expires, time_t expire_time, unsigned long* comment_id);
int xcddefault_add_new_service_comment(int entry_type, char const* host_name, char const* svc_description, time_t entry_time, char const* author_name, char const* comment_data, int persistent, int source, int expires, time_t expire_time, unsigned long* comment_id);
int xcddefault_delete_host_comment(unsigned long comment_id);
int xcddefault_delete_service_comment(unsigned long comment_id);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_XCDDEFAULT_HH

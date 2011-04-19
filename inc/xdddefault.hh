/*
** Copyright 2001-2006 Ethan Galstad
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

#ifndef CCE_XDDDEFAULT_HH
# define CCE_XDDDEFAULT_HH

# ifdef __cplusplus
extern "C" {
# endif

# define XDDDEFAULT_NO_DATA      0
# define XDDDEFAULT_INFO_DATA    1
# define XDDDEFAULT_HOST_DATA    2
# define XDDDEFAULT_SERVICE_DATA 3

int xdddefault_initialize_downtime_data(char* main_config_file);
int xdddefault_validate_downtime_data(void);
int xdddefault_cleanup_downtime_data(char* main_config_file);

int xdddefault_save_downtime_data(void);
int xdddefault_add_new_host_downtime(char* host_name, time_t entry_time, char* author, char* comment, time_t start_time, time_t end_time, int fixed, unsigned long triggered_by, unsigned long duration, unsigned long* downtime_id);
int xdddefault_add_new_service_downtime(char* host_name, char* service_description, time_t entry_time, char* author, char* comment, time_t start_time, time_t end_time, int fixed, unsigned long triggered_by, unsigned long duration, unsigned long* downtime_id);

int xdddefault_delete_host_downtime(unsigned long downtime_id);
int xdddefault_delete_service_downtime(unsigned long downtime_id);
int xdddefault_delete_downtime(int type, unsigned long downtime_id);

# ifdef __cplusplus
}
# endif

#endif // !CCE_XDDDEFAULT_HH

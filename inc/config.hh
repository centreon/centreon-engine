/*
** Copyright 2002-2006 Ethan Galstad
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

#ifndef CCE_CONFIG_HH
# define CCE_CONFIG_HH

# ifdef __cplusplus
extern "C" {
# endif

// Configuration Functions
int read_all_object_data(char* main_config_file); // reads all object config data

// Setup Functions
int pre_flight_check(void);                       // try and verify the configuration data
int pre_flight_object_check(int* w, int* e);      // verify object relationships and settings
int pre_flight_circular_check(int* w, int* e);    // detects circular dependencies and paths

# ifdef __cplusplus
}
# endif

#endif // !CCE_CONFIG_HH

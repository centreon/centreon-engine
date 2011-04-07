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

#ifndef CCS_XRDDEFAULT_HH
# define CCS_XRDDEFAULT_HH

# ifdef __cplusplus
extern "C" {
# endif

static const unsigned int XRDDEFAULT_NO_DATA              = 0;
static const unsigned int XRDDEFAULT_INFO_DATA            = 1;
static const unsigned int XRDDEFAULT_PROGRAMSTATUS_DATA   = 2;
static const unsigned int XRDDEFAULT_HOSTSTATUS_DATA      = 3;
static const unsigned int XRDDEFAULT_SERVICESTATUS_DATA   = 4;
static const unsigned int XRDDEFAULT_CONTACTSTATUS_DATA   = 5;
static const unsigned int XRDDEFAULT_HOSTCOMMENT_DATA     = 6;
static const unsigned int XRDDEFAULT_SERVICECOMMENT_DATA  = 7;
static const unsigned int XRDDEFAULT_HOSTDOWNTIME_DATA    = 8;
static const unsigned int XRDDEFAULT_SERVICEDOWNTIME_DATA = 9;

int xrddefault_initialize_retention_data(char* config_file);
int xrddefault_cleanup_retention_data(char* config_file);
int xrddefault_grab_config_info(char* main_config_file);
int xrddefault_grab_config_directives(char* input);
int xrddefault_save_state_information(void); // saves all host and service state information
int xrddefault_read_state_information(void); // reads in initial host and service state information

# ifdef __cplusplus
}
# endif

#endif // !CCS_XRDDEFAULT_HH

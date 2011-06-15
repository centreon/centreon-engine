/*
** Copyright 2007-2008 Ethan Galstad
** Copyright 2007,2010 Andreas Ericsson
** Copyright 2010      Max Schubert
** Copyright 2011      Merethis
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

#ifndef CCE_COMPATIBILITY_NAGIOS_H
# define CCE_COMPATIBILITY_NAGIOS_H

# include "config.h"
# include "logging.h"
# include "common.h"
# include "locations.h"
# include "objects.h"
# include "macros.h"
# include "broker.h"
# include "nebstructs.h"
# include "nebcallbacks.h"
# include "checks.hh"
# include "flapping.hh"
# include "notifications.hh"
# include "sehandlers.hh"
# include "events.hh"
# include "utils.hh"
# include "comments.hh"
# include "config.hh"
# include "engine.hh"

/******* INTER-CHECK DELAY CALCULATION TYPES **********/

# define ICD_NONE  0 /* no inter-check delay */
# define ICD_DUMB  1 /* dumb delay of 1 second */
# define ICD_SMART 2 /* smart delay */
# define ICD_USER  3 /* user-specified delay */

/******* INTERLEAVE FACTOR CALCULATION TYPES **********/

# define ILF_USER  0 /* user-specified interleave factor */
# define ILF_SMART 1 /* smart interleave */

# ifdef __cplusplus
extern "C" {
# endif


int init_embedded_perl(char** env);       // initialized embedded perl interpreter
int deinit_embedded_perl(void);           // cleans up embedded perl
int file_uses_embedded_perl(char* fname); // tests whether or not the embedded perl interpreter should be used on a file

void service_check_sighandler(int sig);   // handles timeouts when executing service checks
void host_check_sighandler(int sig);      // handles timeouts when executing host checks
void my_system_sighandler(int sig);

// IPC Functions
int process_check_result_queue(char const* dirname);
int process_check_result_file(char* fname);
int delete_check_result_file(char const* fname);
check_result* read_check_result(void);     // reads a host/service check result from the list in memory
int init_check_result(check_result* info);
int add_check_result_to_list(check_result* new_cr);
int free_check_result_list(void);
int move_check_result_to_queue(char* checkresult_file);

# ifdef __cplusplus
}
# endif

#endif /* !CCE_COMPATIBILITY_NAGIOS_H */

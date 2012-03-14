/*
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

#include <limits.h>
#include <stddef.h>
#include "globals.h"

// Features now handled by startup script.
int daemon_dumps_core(0);
char* lock_file(NULL);
char* nagios_user(NULL);
char* nagios_group(NULL);

// Process options.
int nagios_pid(0);
int verify_object_relationships(1);

// Update-related variables.
int bare_update_checks(0);
int check_for_updates(0);
int update_available(0);
unsigned long update_uid(0);
char* last_program_version(NULL);
char* new_program_version(NULL);

// Retention flags.
unsigned long retained_process_service_attribute_mask(0);
unsigned long retained_service_attribute_mask(0);

// Commands execution system.
circular_buffer check_result_buffer;
check_result check_result_info;
check_result* check_result_list(NULL);
unsigned int child_processes_fork_twice(UINT_MAX);
unsigned long max_check_result_file_age(ULONG_MAX);

// Embedded Perl.
unsigned int enable_embedded_perl(false);
unsigned int use_embedded_perl_implicitly(false);

// Old path.
char const* check_result_path("/tmp");
char const* p1_file(NULL);
char const* temp_path("/tmp");

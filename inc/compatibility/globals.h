/*
** Copyright 2012 Merethis
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

#ifndef CCE_COMPATIBILITY_GLOBALS_H
#  define CCE_COMPATIBILITY_GLOBALS_H

#  include "com/centreon/engine/checks.hh"
#  include "com/centreon/engine/circular_buffer.hh"

// Log rotation.
extern char* log_archive_path;
extern unsigned int log_rotation_method;

// Features now handled by startup script.
extern int daemon_dumps_core;
extern char* lock_file;
extern char* nagios_user;
extern char* nagios_group;

// Process options.
extern int nagios_pid;
extern int verify_object_relationships;

// Update-related variables.
extern int bare_update_checks;
extern int check_for_updates;
extern int update_available;
extern unsigned long update_uid;
extern char* last_program_version;
extern char* new_program_version;

// Retention flags.
extern unsigned long retained_process_service_attribute_mask;
extern unsigned long retained_service_attribute_mask;

// Commands execution system.
extern circular_buffer check_result_buffer;
extern check_result check_result_info;
extern check_result* check_result_list;
extern unsigned int child_processes_fork_twice;
extern unsigned long max_check_result_file_age;

// Embedded Perl.
extern unsigned int enable_embedded_perl;
extern unsigned int use_embedded_perl_implicitly;

// Old path.
extern char const* check_result_path;
extern char const* temp_path;

#endif // !CCE_COMPATIBILITY_GLOBALS_H

/*
** Copyright 2012-2014 Merethis
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
extern char const* log_archive_path;
extern unsigned int log_rotation_method;

// Free memory option.
extern unsigned int free_child_process_memory;

// Features now handled by startup script.
extern int daemon_dumps_core;
extern char const* lock_file;
extern char const* nagios_user;
extern char const* nagios_group;

// Process options.
extern int nagios_pid;
extern int verify_object_relationships;

// Retention flags.
extern unsigned long retained_process_service_attribute_mask;
extern unsigned long retained_service_attribute_mask;

// Commands execution system.
extern circular_buffer check_result_buffer;
extern check_result check_result_info;
extern check_result* check_result_list;

// Embedded Perl.
extern unsigned int enable_embedded_perl;
extern unsigned int use_embedded_perl_implicitly;

// Old path.
extern char const* p1_file;
extern char const* temp_path;

#endif // !CCE_COMPATIBILITY_GLOBALS_H

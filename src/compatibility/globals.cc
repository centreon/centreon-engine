/*
** Copyright 2011 Merethis
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

#include <stddef.h>

// Features now handled by startup script.
char*         lock_file = NULL;
char*         nagios_user = NULL;
char*         nagios_group = NULL;

// Startup verifications.
int           verify_object_relationships = 1;

// Update-related variables.
int           bare_update_checks = 0;
int           check_for_updates = 0;
int           update_available = 0;
unsigned long update_uid;
char*         last_program_version = NULL;
char*         new_program_version = NULL;

/*
** Copyright 2011-2014 Merethis
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

#include <cstddef>
#include "globals.h"

// Process options.
int nagios_pid(0);
int verify_object_relationships(1);

// Retention flags.
unsigned long retained_process_service_attribute_mask(0);
unsigned long retained_service_attribute_mask(0);

// Commands execution system.
circular_buffer check_result_buffer;
check_result check_result_info;
check_result* check_result_list(NULL);

// Embedded Perl.
unsigned int use_embedded_perl_implicitly(false);

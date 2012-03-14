/*
** Copyright 2002-2006 Ethan Galstad
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

#ifndef CCE_EXTERNAL_COMMANDS_COMPATIBILITY_HH
#  define CCE_EXTERNAL_COMMANDS_COMPATIBILITY_HH

#  include <time.h>
#  include "com/centreon/engine/objects.hh"

#  ifdef __cplusplus
extern "C" {
#  endif // C++

int process_external_command1(char* cmd);
int process_external_command2(
      int cmd,
      time_t entry_time,
      char* args);
int process_hostgroup_command(
      int cmd,
      time_t entry_time,
      char* args);
int process_host_command(
      int cmd,
      time_t entry_time,
      char* args);
int process_service_command(
      int cmd,
      time_t entry_time,
      char* args);
int process_servicegroup_command(
      int cmd,
      time_t entry_time,
      char* args);
int process_contact_command(
      int cmd,
      time_t entry_time,
      char* args);
int process_contactgroup_command(
      int cmd,
      time_t entry_time,
      char* args);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_EXTERNAL_COMMANDS_COMPATIBILITY_HH

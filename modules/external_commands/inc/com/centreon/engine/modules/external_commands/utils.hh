/*
** Copyright 2011-2013 Merethis
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

#ifndef CCE_MOD_EXTCMD_UTILS_HH
#define CCE_MOD_EXTCMD_UTILS_HH

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif  // C++

int open_command_file();   // creates the external command file as a named pipe
                           // (FIFO) and opens it for reading
int close_command_file();  // closes and deletes the external command file
                           // (FIFO)
int init_command_file_worker_thread(void);
int shutdown_command_file_worker_thread(void);
void cleanup_command_file_worker_thread(void* arg);
void* command_file_worker_thread(void* arg);
int submit_external_command(char const* cmd, int* buffer_items);

#ifdef __cplusplus
}
#endif  // C++

#endif  // !CCE_MOD_EXTCMD_UTILS_HH

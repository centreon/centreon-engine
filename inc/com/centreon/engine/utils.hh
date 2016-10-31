/*
** Copyright 1999-2009 Ethan Galstad
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

#ifndef CCE_UTILS_HH
#  define CCE_UTILS_HH

#  include <sys/time.h>
#  include "com/centreon/engine/checks.hh"
#  include "com/centreon/engine/macros/defines.hh"
#  include "com/centreon/engine/objects/command.hh"
#  include "com/centreon/engine/objects/daterange.hh"
#  include "com/centreon/engine/objects/timeperiod.hh"

// DBUF structure - dynamic string storage
typedef struct  dbuf_struct {
  char*         buf;
  unsigned long used_size;
  unsigned long allocated_size;
  unsigned long chunk_size;
}               dbuf;

#  ifdef __cplusplus
extern "C" {
#  endif // C++

// Monitoring/Event Handler Functions

// thread-safe version of the above
int my_system_r(
      nagios_macros* mac,
      char* cmd,
      int timeout,
      int* early_timeout,
      double* exectime,
      char** output,
      unsigned int max_output_length);
// executes a command via popen(), but also protects against timeouts
int my_system(
      char* cmd,
      int timeout,
      int* early_timeout,
      double* exectime,
      char** output,
      int max_output_length);
// same like unix ctime without the '\n' at the end of the string.
char const* my_ctime(time_t const* t);

// Miscellaneous Functions

// thread-safe version of get_raw_command_line_r()
int get_raw_command_line_r(
      nagios_macros* mac,
      command* cmd_ptr,
      char const* cmd,
      char** full_command,
      int macro_options);
// given a raw command line, determine the actual command to run Manipulates global_macros.argv and is thus not threadsafe
int get_raw_command_line(
      command* cmd_ptr,
      char* cmd,
      char** full_command,
      int macro_options);
// sets/clears and environment variable
int set_environment_var(
      char const* name,
      char const* value,
      int set);
// determine the next time to schedule a log rotation
time_t get_next_log_rotation_time();
// trap signals
void setup_sighandler();
// reset signals to default action
void reset_sighandler();
// handles signals
void sighandler(int sig);
// handles timeouts when executing commands via my_system()
void my_system_sighandler(int sig);
char* get_next_string_from_buf(
        char* buf,
        int* start_index,
        int bufsize);
// tests whether or not an object name (host, service, etc.) contains illegal characters
int contains_illegal_object_chars(char* name);
char* escape_newlines(char* rawbuf);
// compares two strings for equality
int compare_strings(char* val1a, char* val2a);
// renames a file - works across filesystems
int my_rename(char const* source, char const* dest);
// copies a named source to an already opened destination file
int my_fdcopy(char const* source, char const* dest, int dest_fd);
// copies a file - works across filesystems
int my_fcopy(char const* source, char const* dest);
int dbuf_init(dbuf* db, int chunk_size);
int dbuf_free(dbuf* db);
int dbuf_strcat(dbuf* db, char const* buf);
bool set_cloexec(int fd);


// Cleanup Functions

// cleanup after ourselves (before quitting or restarting)
void cleanup();
// free memory allocated to all linked lists in memory
void free_memory(nagios_macros* mac);
// frees all memory allocated to the notification list
void free_notification_list();

// frees memory associated with a host/service check result
int free_check_result(check_result* info);
int parse_check_output(
      char* buf,
      char** short_output,
      char** long_output,
      char** perf_data,
      int escape_newlines_please,
      int newlines_are_escaped);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_UTILS_HH

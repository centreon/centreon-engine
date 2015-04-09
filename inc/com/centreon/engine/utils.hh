/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2011-2015 Merethis
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
#  include "com/centreon/engine/timeperiod.hh"

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
// same like unix ctime without the '\n' at the end of the string.
char const* my_ctime(time_t const* t);

// Miscellaneous Functions

// given a raw command line, determine the actual command to run Manipulates global_macros.argv and is thus not threadsafe
int get_raw_command_line_r(
      nagios_macros* mac,
      command* cmd_ptr,
      char const* cmd,
      char** full_command,
      int macro_options);
int is_daterange_single_day(daterange const* dr);
// calculate_time_from_day_of_month
time_t calculate_time_from_day_of_month(
         int year,
         int month,
         int monthday);
// calculates midnight time of specific (3rd, last, etc.) weekday of a particular month
time_t calculate_time_from_weekday_of_month(
         int year,
         int month,
         int weekday,
         int weekday_offset);
// trap signals
void setup_sighandler();
// reset signals to default action
void reset_sighandler();
// handles signals
void sighandler(int sig);
// handles timeouts when executing commands via my_system()
void my_system_sighandler(int sig);
// tests whether or not an object name (host, service, etc.) contains illegal characters
int contains_illegal_object_chars(char* name);
int dbuf_init(dbuf* db, int chunk_size);
int dbuf_free(dbuf* db);
int dbuf_strcat(dbuf* db, char const* buf);
bool set_cloexec(int fd);


// Cleanup Functions

// cleanup after ourselves (before quitting or restarting)
void cleanup();
// free memory allocated to all linked lists in memory
void free_memory(nagios_macros* mac);

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

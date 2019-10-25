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
#  include "com/centreon/engine/daterange.hh"
#  include "com/centreon/engine/macros/defines.hh"

#  ifdef __cplusplus
extern "C" {
#  endif // C++

// Monitoring/Event Handler Functions

// thread-safe version of the above
int my_system_r(
      nagios_macros* mac,
      std::string const& cmd,
      int timeout,
      int* early_timeout,
      double* exectime,
      std::string & output,
      unsigned int max_output_length);
// same like unix ctime without the '\n' at the end of the string.
char const* my_ctime(time_t const* t);

// Miscellaneous Functions

// thread-safe version of get_raw_command_line_r()
int get_raw_command_line_r(
      nagios_macros* mac,
      com::centreon::engine::commands::command* cmd_ptr,
      std::string const& cmd,
      std::string& full_command,
      int macro_options);
// trap signals
void setup_sighandler();
// handles signals
void sighandler(int sig);
// tests whether or not an object name (host, service, etc.) contains illegal characters
bool contains_illegal_object_chars(char const* name);
// compares two strings for equality
int compare_strings(char* val1a, char* val2a);
bool set_cloexec(int fd);


// Cleanup Functions

// cleanup after ourselves (before quitting or restarting)
void cleanup();
// free memory allocated to all linked lists in memory
void free_memory(nagios_macros* mac);

// frees memory associated with a host/service check result
void parse_check_output(std::string const& buffer,
                       std::string& short_output,
                       std::string& long_output,
                       std::string& perf_data,
                       bool escape_newlines_please,
                       bool newlines_are_escaped);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_UTILS_HH

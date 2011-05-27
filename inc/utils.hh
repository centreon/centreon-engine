/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2011      Merethis
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
# define CCE_UTILS_HH

# include <sys/time.h>
# include "macros.hh"
# include "objects.hh"
# include "checks.hh"

# ifdef __cplusplus
extern "C" {
# endif

// DBUF structure - dynamic string storage
typedef struct  dbuf_struct {
  char*         buf;
  unsigned long used_size;
  unsigned long allocated_size;
  unsigned long chunk_size;
}               dbuf;

// Monitoring/Event Handler Functions
int my_system_r(nagios_macros* mac, char* cmd,int timeout,int* early_timeout,double* exectime,char** output,unsigned int max_output_length); // thread-safe version of the above
int my_system(char* cmd,int timeout,int* early_timeout,double* exectime,char** output,int max_output_length);                                // executes a command via popen(), but also protects against timeouts

// Miscellaneous Functions
int get_raw_command_line_r(nagios_macros* mac, command* cmd_ptr, char const* cmd, char** full_command, int macro_options); // thread-safe version of get_raw_command_line_r()
int get_raw_command_line(command* cmd_ptr, char* cmd, char** full_command, int macro_options);                             // given a raw command line, determine the actual command to run Manipulates global_macros.argv and is thus not threadsafe
int set_environment_var(char const* name, char const* value, int set);                             // sets/clears and environment variable
int check_time_against_period(time_t test_time, timeperiod* tperiod);                              // check to see if a specific time is covered by a time period
void get_next_valid_time(time_t pref_time, time_t* valid_time, timeperiod* tperiod);               // get the next valid time in a time period
int is_daterange_single_day(daterange* dr);
time_t calculate_time_from_day_of_month(int year, int month, int monthday);                        // calculate_time_from_day_of_month
time_t calculate_time_from_weekday_of_month(int year, int month, int weekday, int weekday_offset); // calculates midnight time of specific (3rd, last, etc.) weekday of a particular month
time_t get_next_log_rotation_time(void);                                                           // determine the next time to schedule a log rotation
void setup_sighandler(void);                                                                       // trap signals
void reset_sighandler(void);                                                                       // reset signals to default action
void sighandler(int sig);                                                                          // handles signals
void my_system_sighandler(int sig);                                                                // handles timeouts when executing commands via my_system()
char* get_next_string_from_buf(char* buf, int* start_index, int bufsize);
int contains_illegal_object_chars(char* name);                                                     // tests whether or not an object name (host, service, etc.) contains illegal characters
char* escape_newlines(char* rawbuf);
int compare_strings(char* val1a, char* val2a);                                                     // compares two strings for equality
int my_rename(char const* source, char const* dest);                                               // renames a file - works across filesystems
int my_fdcopy(char const* source, char const* dest, int dest_fd);                                  // copies a named source to an already opened destination file
int my_fcopy(char const* source, char const* dest);                                                // copies a file - works across filesystems
int dbuf_init(dbuf* db, int chunk_size);
int dbuf_free(dbuf* db);
int dbuf_strcat(dbuf* db, char const* buf);

// Check Statistics Functions
int init_check_stats(void);
int update_check_stats(int check_type, time_t check_time);
int generate_check_stats(void);

// Cleanup Functions
void cleanup(void);                        // cleanup after ourselves (before quitting or restarting)
void free_memory(nagios_macros* mac);      // free memory allocated to all linked lists in memory
void free_notification_list(void);         // frees all memory allocated to the notification list
int reset_variables(void);                 // reset all global variables

int free_check_result(check_result* info); // frees memory associated with a host/service check result
int parse_check_output(char* buf, char** short_output, char** long_output, char** perf_data, int escape_newlines_please, int newlines_are_escaped);

# ifdef __cplusplus
}
# endif

#endif // !CCE_UTILS_HH

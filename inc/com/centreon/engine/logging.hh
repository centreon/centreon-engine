/*
** Copyright 1999-2007 Ethan Galstad
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

#ifndef CCE_LOGGING_HH
#  define CCE_LOGGING_HH

#  include <time.h>
#  include "com/centreon/engine/objects.hh"

// State Logging Types
#  define INITIAL_STATES             1
#  define CURRENT_STATES             2

// Logging Types
#  define NSLOG_RUNTIME_ERROR        1
#  define NSLOG_RUNTIME_WARNING      2

#  define NSLOG_VERIFICATION_ERROR   4
#  define NSLOG_VERIFICATION_WARNING 8

#  define NSLOG_CONFIG_ERROR         16
#  define NSLOG_CONFIG_WARNING       32

#  define NSLOG_PROCESS_INFO         64
#  define NSLOG_EVENT_HANDLER        128
// #  define NSLOG_NOTIFICATION       256 //NOT USED ANYMORE - CAN BE REUSED
#  define NSLOG_EXTERNAL_COMMAND     512

#  define NSLOG_HOST_UP              1024
#  define NSLOG_HOST_DOWN            2048
#  define NSLOG_HOST_UNREACHABLE     4096

#  define NSLOG_SERVICE_OK           8192
#  define NSLOG_SERVICE_UNKNOWN      16384
#  define NSLOG_SERVICE_WARNING      32768
#  define NSLOG_SERVICE_CRITICAL     65536

#  define NSLOG_PASSIVE_CHECK        131072

#  define NSLOG_INFO_MESSAGE         262144

#  define NSLOG_HOST_NOTIFICATION    524288
#  define NSLOG_SERVICE_NOTIFICATION 1048576

// Debugging Levels
#  define DEBUGL_ALL                 (unsigned int)-1
#  define DEBUGL_NONE                0
#  define DEBUGL_FUNCTIONS           1
#  define DEBUGL_CONFIG              2
#  define DEBUGL_PROCESS             4
#  define DEBUGL_STATUSDATA          4
#  define DEBUGL_RETENTIONDATA       4
#  define DEBUGL_EVENTS              8
#  define DEBUGL_CHECKS              16
#  define DEBUGL_IPC                 16
#  define DEBUGL_FLAPPING            16
#  define DEBUGL_EVENTHANDLERS       16
#  define DEBUGL_PERFDATA            16
#  define DEBUGL_NOTIFICATIONS       32
#  define DEBUGL_EVENTBROKER         64
#  define DEBUGL_EXTERNALCOMMANDS    128
#  define DEBUGL_COMMANDS            256
#  define DEBUGL_DOWNTIME            512
#  define DEBUGL_COMMENTS            1024
#  define DEBUGL_MACROS              2048

#  define DEBUGV_BASIC               0
#  define DEBUGV_MORE                1
#  define DEBUGV_MOST                2

#  ifdef __cplusplus
extern "C" {
#  endif // C++

// Logging Functions
void logit(int data_type, int display, char const* fmt, ...);
int log_debug_info(int level, unsigned int verbosity,char const* fmt, ...);

int write_to_all_logs(char const* buffer, unsigned long data_type);               // writes a string to main log file and syslog facility
int write_to_log(char const* buffer, unsigned long data_type, time_t* timestamp); // write a string to the main log file
int write_to_syslog(char const* buffer, unsigned long data_type);                 // write a string to the syslog facility
int log_service_event(service* svc);                                              // logs a service event
int log_host_event(host* hst);                                                    // logs a host event
int log_host_states(unsigned int type, time_t* timestamp);                        // logs initial/current host states
int log_service_states(unsigned int type, time_t* timestamp);                     // logs initial/current service states
int rotate_log_file(time_t rotation_time);                                        // rotates the main log file
int write_log_file_info(time_t* timestamp);                                       // records log file/version info
int open_debug_log(void);
int close_debug_log(void);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_LOGGING_HH

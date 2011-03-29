/*
** Copyright 1999-2007 Ethan Galstad
** Copyright 2011      Merethis
**
** This file is part of Centreon Scheduler.
**
** Centreon Scheduler is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Scheduler is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Scheduler. If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef CSS_LOGGING_HH
# define CSS_LOGGING_HH

# include <sys/time.h>
# include "objects.hh"

# ifdef __cplusplus
extern "C" {
# endif

// Logging Types
static const unsigned int NSLOG_RUNTIME_ERROR        = 1;
static const unsigned int NSLOG_RUNTIME_WARNING      = 2;

static const unsigned int NSLOG_VERIFICATION_ERROR   = 4;
static const unsigned int NSLOG_VERIFICATION_WARNING = 8;

static const unsigned int NSLOG_CONFIG_ERROR         = 16;
static const unsigned int NSLOG_CONFIG_WARNING       = 32;

static const unsigned int NSLOG_PROCESS_INFO         = 64;
static const unsigned int NSLOG_EVENT_HANDLER        = 128;
// static const unsigned int NSLOG_NOTIFICATION       = 256; //NOT USED ANYMORE - CAN BE REUSED
static const unsigned int NSLOG_EXTERNAL_COMMAND     = 512;

static const unsigned int NSLOG_HOST_UP              = 1024;
static const unsigned int NSLOG_HOST_DOWN            = 2048;
static const unsigned int NSLOG_HOST_UNREACHABLE     = 4096;

static const unsigned int NSLOG_SERVICE_OK           = 8192;
static const unsigned int NSLOG_SERVICE_UNKNOWN      = 16384;
static const unsigned int NSLOG_SERVICE_WARNING      = 32768;
static const unsigned int NSLOG_SERVICE_CRITICAL     = 65536;

static const unsigned int NSLOG_PASSIVE_CHECK        = 131072;

static const unsigned int NSLOG_INFO_MESSAGE         = 262144;

static const unsigned int NSLOG_HOST_NOTIFICATION    = 524288;
static const unsigned int NSLOG_SERVICE_NOTIFICATION = 1048576;

// Debugging Levels
static const unsigned int DEBUGL_ALL                 = -1;
static const unsigned int DEBUGL_NONE                = 0;
static const unsigned int DEBUGL_FUNCTIONS           = 1;
static const unsigned int DEBUGL_CONFIG              = 2;
static const unsigned int DEBUGL_PROCESS             = 4;
static const unsigned int DEBUGL_STATUSDATA          = 4;
static const unsigned int DEBUGL_RETENTIONDATA       = 4;
static const unsigned int DEBUGL_EVENTS              = 8;
static const unsigned int DEBUGL_CHECKS              = 16;
static const unsigned int DEBUGL_IPC                 = 16;
static const unsigned int DEBUGL_FLAPPING            = 16;
static const unsigned int DEBUGL_EVENTHANDLERS       = 16;
static const unsigned int DEBUGL_PERFDATA            = 16;
static const unsigned int DEBUGL_NOTIFICATIONS       = 32;
static const unsigned int DEBUGL_EVENTBROKER         = 64;
static const unsigned int DEBUGL_EXTERNALCOMMANDS    = 128;
static const unsigned int DEBUGL_COMMANDS            = 256;
static const unsigned int DEBUGL_DOWNTIME            = 512;
static const unsigned int DEBUGL_COMMENTS            = 1024;
static const unsigned int DEBUGL_MACROS              = 2048;

static const unsigned int DEBUGV_BASIC               = 0;
static const unsigned int DEBUGV_MORE		     = 1;
static const unsigned int DEBUGV_MOST                = 2;

// Logging Functions
void logit(int data_type, int display, const char *fmt, ...) __attribute__((__format__(__printf__, 3, 4)));
int log_debug_info(int level, int verbosity, const char *fmt, ...) __attribute__((__format__(__printf__, 3, 4)));

int write_to_all_logs(char *buffer, unsigned long data_type);               // writes a string to main log file and syslog facility
int write_to_log(char *buffer, unsigned long data_type, time_t *timestamp); // write a string to the main log file
int write_to_syslog(char const *buffer, unsigned long data_type);           // write a string to the syslog facility
int log_service_event(service *svc);                                        // logs a service event
int log_host_event(host *hst);                                              // logs a host event
int log_host_states(int type, time_t *timestamp);                           // logs initial/current host states
int log_service_states(int type, time_t *timestamp);                        // logs initial/current service states
int rotate_log_file(time_t rotation_time);                                  // rotates the main log file
int write_log_file_info(time_t *timestamp);                                 // records log file/version info
int open_debug_log(void);
int close_debug_log(void);

# ifdef __cplusplus
}
# endif

#endif // !CSS_LOGGING_HH

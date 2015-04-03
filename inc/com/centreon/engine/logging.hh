/*
** Copyright 1999-2007 Ethan Galstad
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

#ifndef CCE_LOGGING_HH
#  define CCE_LOGGING_HH

#  include <time.h>
#  include "com/centreon/engine/objects/host.hh"
#  include "com/centreon/engine/objects/service.hh"

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
#  define DEBUGL_EVENTBROKER         64
#  define DEBUGL_EXTERNALCOMMANDS    128
#  define DEBUGL_COMMANDS            256
#  define DEBUGL_MACROS              2048

#  define DEBUGV_BASIC               0
#  define DEBUGV_MORE                1
#  define DEBUGV_MOST                2

#  ifdef __cplusplus
extern "C" {
#  endif // C++

// logs a service event
int log_service_event(service const* svc);
// logs a host event
int log_host_event(host const* hst);
// logs initial/current host states
int log_host_states(unsigned int type, time_t* timestamp);
// logs initial/current service states
int log_service_states(unsigned int type, time_t* timestamp);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_LOGGING_HH

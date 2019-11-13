/*
** Copyright 2002-2007 Ethan Galstad
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

#ifndef CCE_NEBCALLBACKS_HH
#define CCE_NEBCALLBACKS_HH

#include "com/centreon/engine/nebmodules.hh"

/* Callback types. */
#define NEBCALLBACK_RESERVED0 0 /* Reserved for future use. */
#define NEBCALLBACK_RESERVED1 1
#define NEBCALLBACK_RESERVED2 2
#define NEBCALLBACK_RESERVED3 3
#define NEBCALLBACK_RESERVED4 4
#define NEBCALLBACK_RAW_DATA 5
#define NEBCALLBACK_NEB_DATA 6
#define NEBCALLBACK_PROCESS_DATA 7
#define NEBCALLBACK_TIMED_EVENT_DATA 8
#define NEBCALLBACK_LOG_DATA 9
#define NEBCALLBACK_SYSTEM_COMMAND_DATA 10
#define NEBCALLBACK_EVENT_HANDLER_DATA 11
#define NEBCALLBACK_NOTIFICATION_DATA 12
#define NEBCALLBACK_SERVICE_CHECK_DATA 13
#define NEBCALLBACK_HOST_CHECK_DATA 14
#define NEBCALLBACK_COMMENT_DATA 15
#define NEBCALLBACK_DOWNTIME_DATA 16
#define NEBCALLBACK_FLAPPING_DATA 17
#define NEBCALLBACK_PROGRAM_STATUS_DATA 18
#define NEBCALLBACK_HOST_STATUS_DATA 19
#define NEBCALLBACK_SERVICE_STATUS_DATA 20
#define NEBCALLBACK_ADAPTIVE_PROGRAM_DATA 21
#define NEBCALLBACK_ADAPTIVE_HOST_DATA 22
#define NEBCALLBACK_ADAPTIVE_SERVICE_DATA 23
#define NEBCALLBACK_EXTERNAL_COMMAND_DATA 24
#define NEBCALLBACK_AGGREGATED_STATUS_DATA 25
#define NEBCALLBACK_RETENTION_DATA 26
#define NEBCALLBACK_CONTACT_NOTIFICATION_DATA 27
#define NEBCALLBACK_CONTACT_NOTIFICATION_METHOD_DATA 28
#define NEBCALLBACK_ACKNOWLEDGEMENT_DATA 29
#define NEBCALLBACK_STATE_CHANGE_DATA 30
#define NEBCALLBACK_CONTACT_STATUS_DATA 31
#define NEBCALLBACK_ADAPTIVE_CONTACT_DATA 32
#define NEBCALLBACK_COMMAND_DATA 33
#define NEBCALLBACK_CUSTOM_VARIABLE_DATA 34
#define NEBCALLBACK_GROUP_DATA 35
#define NEBCALLBACK_GROUP_MEMBER_DATA 36
#define NEBCALLBACK_MODULE_DATA 37
#define NEBCALLBACK_RELATION_DATA 38

#define NEBCALLBACK_ADAPTIVE_DEPENDENCY_DATA 39
#define NEBCALLBACK_ADAPTIVE_ESCALATION_DATA 40
#define NEBCALLBACK_ADAPTIVE_TIMEPERIOD_DATA 41

#define NEBCALLBACK_NUMITEMS 42 /* Total number of callback types we have. */

#ifdef __cplusplus
extern "C" {
#endif /* C++ */

int neb_deregister_callback(int callback_type,
                            int (*callback_func)(int, void*));
int neb_deregister_module_callbacks(void* mod);
int neb_register_callback(int callback_type,
                          void* mod_handle,
                          int priority,
                          int (*callback_func)(int, void*));

#ifdef __cplusplus
}
#endif /* C++ */

#endif /* !CCE_NEBCALLBACKS_HH */

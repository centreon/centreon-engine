/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2011 Nagios Core Development Team and Community Contributors
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

#ifndef CCE_COMMON_HH
#  define CCE_COMMON_HH

#  include "com/centreon/engine/checks/stats.hh"
#  include "com/centreon/engine/shared.hh"

/* Daemon is thread safe. */
#  ifndef _REENTRANT
#    define _REENTRANT
#  endif /* !_REENTRANT */
#  ifndef _THREAD_SAFE
#    define _THREAD_SAFE
#  endif /* !_THREAD_SAFE */

/* Max number of old states to keep track of for flap detection. */
#  define MAX_STATE_HISTORY_ENTRIES 21

/* Commands. */
#  define CMD_NONE                                             0
#  define CMD_ENABLE_SVC_CHECK                                 5
#  define CMD_DISABLE_SVC_CHECK                                6
#  define CMD_SCHEDULE_SVC_CHECK                               7
#  define CMD_RESTART_PROCESS                                 13
#  define CMD_SHUTDOWN_PROCESS                                14
#  define CMD_ENABLE_HOST_SVC_CHECKS                          15
#  define CMD_DISABLE_HOST_SVC_CHECKS                         16
#  define CMD_SCHEDULE_HOST_SVC_CHECKS                        17
#  define CMD_PROCESS_SERVICE_CHECK_RESULT                    30
#  define CMD_SAVE_STATE_INFORMATION                          31
#  define CMD_READ_STATE_INFORMATION                          32
#  define CMD_SAVE_STATUS_INFORMATION                         33
#  define CMD_ENABLE_EVENT_HANDLERS                           41
#  define CMD_DISABLE_EVENT_HANDLERS                          42
#  define CMD_ENABLE_HOST_EVENT_HANDLER                       43
#  define CMD_DISABLE_HOST_EVENT_HANDLER                      44
#  define CMD_ENABLE_SVC_EVENT_HANDLER                        45
#  define CMD_DISABLE_SVC_EVENT_HANDLER                       46
#  define CMD_ENABLE_HOST_CHECK                               47
#  define CMD_DISABLE_HOST_CHECK                              48
#  define CMD_START_OBSESSING_OVER_SVC_CHECKS                 49
#  define CMD_STOP_OBSESSING_OVER_SVC_CHECKS                  50
#  define CMD_SCHEDULE_FORCED_HOST_SVC_CHECKS                 53
#  define CMD_SCHEDULE_FORCED_SVC_CHECK                       54
#  define CMD_ENABLE_HOST_FLAP_DETECTION                      57
#  define CMD_DISABLE_HOST_FLAP_DETECTION                     58
#  define CMD_ENABLE_SVC_FLAP_DETECTION                       59
#  define CMD_DISABLE_SVC_FLAP_DETECTION                      60
#  define CMD_ENABLE_FLAP_DETECTION                           61
#  define CMD_DISABLE_FLAP_DETECTION                          62
#  define CMD_FLUSH_PENDING_COMMANDS                          77
#  define CMD_PROCESS_HOST_CHECK_RESULT                       87
#  define CMD_START_OBSESSING_OVER_HOST_CHECKS                94
#  define CMD_STOP_OBSESSING_OVER_HOST_CHECKS                 95
#  define CMD_SCHEDULE_HOST_CHECK                             96
#  define CMD_SCHEDULE_FORCED_HOST_CHECK                      98
#  define CMD_START_OBSESSING_OVER_SVC                        99
#  define CMD_STOP_OBSESSING_OVER_SVC                        100
#  define CMD_START_OBSESSING_OVER_HOST                      101
#  define CMD_STOP_OBSESSING_OVER_HOST                       102
#  define CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER               123
#  define CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER                124
#  define CMD_CHANGE_HOST_EVENT_HANDLER                      125
#  define CMD_CHANGE_SVC_EVENT_HANDLER                       126
#  define CMD_CHANGE_HOST_CHECK_COMMAND                      127
#  define CMD_CHANGE_SVC_CHECK_COMMAND                       128
#  define CMD_CHANGE_NORMAL_HOST_CHECK_INTERVAL              129
#  define CMD_CHANGE_NORMAL_SVC_CHECK_INTERVAL               130
#  define CMD_CHANGE_RETRY_SVC_CHECK_INTERVAL                131
#  define CMD_CHANGE_MAX_HOST_CHECK_ATTEMPTS                 132
#  define CMD_CHANGE_MAX_SVC_CHECK_ATTEMPTS                  133
#  define CMD_ENABLE_SERVICE_FRESHNESS_CHECKS                138
#  define CMD_DISABLE_SERVICE_FRESHNESS_CHECKS               139
#  define CMD_ENABLE_HOST_FRESHNESS_CHECKS                   140
#  define CMD_DISABLE_HOST_FRESHNESS_CHECKS                  141
#  define CMD_CHANGE_HOST_CHECK_TIMEPERIOD                   144
#  define CMD_CHANGE_SVC_CHECK_TIMEPERIOD                    145
#  define CMD_PROCESS_FILE                                   146
#  define CMD_CHANGE_CUSTOM_HOST_VAR                         147
#  define CMD_CHANGE_CUSTOM_SVC_VAR                          148
#  define CMD_CHANGE_RETRY_HOST_CHECK_INTERVAL               158
#  define CMD_CHANGE_HOST_MODATTR                            165
#  define CMD_CHANGE_SVC_MODATTR                             166
#  define CMD_RELOAD_PROCESS                                 200
#  define CMD_CUSTOM_COMMAND                                 999

/* Service check types. */
#  define SERVICE_CHECK_ACTIVE  0 /* Engine performed the service check. */
#  define SERVICE_CHECK_PASSIVE 1 /* The service check result was submitted by an external source. */

/* Host check types. */
#  define HOST_CHECK_ACTIVE  0 /* Engine performed the host check. */
#  define HOST_CHECK_PASSIVE 1 /* The host check result was submitted by an external source. */

/* Service state types. */
#  define SOFT_STATE 0
#  define HARD_STATE 1

/* Host/service check options. */
#  define CHECK_OPTION_NONE            0 /* No check options. */
#  define CHECK_OPTION_FORCE_EXECUTION 1 /* Force execution of a check (ignores disabled services/hosts, invalid timeperiods). */
#  define CHECK_OPTION_FRESHNESS_CHECK 2 /* This is a freshness check. */

/* Program modes. */
#  define STANDBY_MODE 0
#  define ACTIVE_MODE  1

/* Log versions. */
#  define LOG_VERSION_1 "1.0"
#  define LOG_VERSION_2 "2.0"

/* General definitions. */
#  define OK     0
#  define ERROR  -2 /* Value was changed from -1 so as to not interfere with STATUS_UNKNOWN plugin result. */

#  ifndef TRUE
#    define TRUE  1
#  elif (TRUE!=1)
#    define TRUE  1
#   endif /* !TRUE */
#  ifndef FALSE
#    define FALSE 0
#  elif (FALSE!=0)
#    define FALSE 0
#  endif /* !FALSE */

/* Date range types. */
#  define DATERANGE_CALENDAR_DATE  0  /* 2008-12-25 */
#  define DATERANGE_MONTH_DATE     1  /* july 4 (specific month) */
#  define DATERANGE_MONTH_DAY      2  /* day 21 (generic month) */
#  define DATERANGE_MONTH_WEEK_DAY 3  /* 3rd thursday (specific month) */
#  define DATERANGE_WEEK_DAY       4  /* 3rd thursday (generic month) */
#  define DATERANGE_TYPES          5

/* Date/time types. */
#  define LONG_DATE_TIME  0
#  define SHORT_DATE_TIME 1
#  define SHORT_DATE      2
#  define SHORT_TIME      3
#  define HTTP_DATE_TIME  4 /* Time formatted for use in HTTP headers. */

/* Misc definitions. */
#  define MAX_FILENAME_LENGTH          256 /* Max length of path/filename that Engine will process. */
#  define MAX_INPUT_BUFFER            1024 /* Size in bytes of max. input buffer (for reading files, misc stuff). */
#  define MAX_COMMAND_BUFFER          8192 /* Max length of raw or processed command line. */
#  define MAX_EXTERNAL_COMMAND_LENGTH 8192 /* Max length of an external command. */
#  define MAX_DATETIME_LENGTH           48

/* Modified attributes. */
#  define MODATTR_NONE                       0
#  define MODATTR_ACTIVE_CHECKS_ENABLED      (1 << 1)
#  define MODATTR_EVENT_HANDLER_ENABLED      (1 << 3)
#  define MODATTR_FLAP_DETECTION_ENABLED     (1 << 4)
#  define MODATTR_OBSESSIVE_HANDLER_ENABLED  (1 << 7)
#  define MODATTR_EVENT_HANDLER_COMMAND      (1 << 8)
#  define MODATTR_CHECK_COMMAND              (1 << 9)
#  define MODATTR_NORMAL_CHECK_INTERVAL      (1 << 10)
#  define MODATTR_RETRY_CHECK_INTERVAL       (1 << 11)
#  define MODATTR_MAX_CHECK_ATTEMPTS         (1 << 12)
#  define MODATTR_FRESHNESS_CHECKS_ENABLED   (1 << 13)
#  define MODATTR_CHECK_TIMEPERIOD           (1 << 14)
#  define MODATTR_CUSTOM_VARIABLE            (1 << 15)
#  define MODATTR_ALL                        (~0)

/* Host status. */
#  define HOST_UP          0
#  define HOST_DOWN        1
#  define HOST_UNREACHABLE 2

/* Service state. */
#  define STATE_OK       0
#  define STATE_WARNING  1
#  define STATE_CRITICAL 2
#  define STATE_UNKNOWN  3

/* State change types. */
#  define HOST_STATECHANGE    0
#  define SERVICE_STATECHANGE 1

#endif /* !CCE_COMMON_HH */

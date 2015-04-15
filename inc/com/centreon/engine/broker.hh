/*
** Copyright 2002-2006 Ethan Galstad
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

#ifndef CCE_BROKER_HH
#  define CCE_BROKER_HH

#  include <sys/time.h>
#  include "com/centreon/engine/objects/host.hh"
#  include "com/centreon/engine/objects/service.hh"

/* Event broker options. */
#  define BROKER_NOTHING                           0
#  define BROKER_PROGRAM_STATE                     (1 << 0)
#  define BROKER_TIMED_EVENTS                      (1 << 1)
#  define BROKER_SERVICE_CHECKS                    (1 << 2)
#  define BROKER_HOST_CHECKS                       (1 << 3)
#  define BROKER_EVENT_HANDLERS                    (1 << 4)
#  define BROKER_LOGGED_DATA                       (1 << 5)
#  define BROKER_FLAPPING_DATA                     (1 << 7)
#  define BROKER_SYSTEM_COMMANDS                   (1 << 10)
#  define BROKER_OCP_DATA_UNUSED                   (1 << 11) /* Reusable. */
#  define BROKER_STATUS_DATA                       (1 << 12)
#  define BROKER_ADAPTIVE_DATA                     (1 << 13)
#  define BROKER_EXTERNALCOMMAND_DATA              (1 << 14)
#  define BROKER_RETENTION_DATA                    (1 << 15)
#  define BROKER_STATECHANGE_DATA                  (1 << 17)
#  define BROKER_RESERVED18                        (1 << 18)
#  define BROKER_RESERVED19                        (1 << 19)
#  define BROKER_CUSTOMVARIABLE_DATA               (1 << 20)
#  define BROKER_MODULE_DATA                       (1 << 23)
#  define BROKER_RELATION_DATA                     (1 << 24)
#  define BROKER_COMMAND_DATA                      (1 << 25)
#  define BROKER_EVERYTHING                        (~0)

/*
** Event types.
*/
#  define NEBTYPE_NONE                             0

/* Unused ? */
#  define NEBTYPE_HELLO                            1
#  define NEBTYPE_GOODBYE                          2
#  define NEBTYPE_INFO                             3

/* Process. */
#  define NEBTYPE_PROCESS_START                    100
#  define NEBTYPE_PROCESS_DAEMONIZE                101
#  define NEBTYPE_PROCESS_RESTART                  102
#  define NEBTYPE_PROCESS_SHUTDOWN                 103
#  define NEBTYPE_PROCESS_PRELAUNCH                104 /* Before objects are read or verified. */
#  define NEBTYPE_PROCESS_EVENTLOOPSTART           105
#  define NEBTYPE_PROCESS_EVENTLOOPEND             106

/* Events. */
#  define NEBTYPE_TIMEDEVENT_ADD                   200
#  define NEBTYPE_TIMEDEVENT_DELETE                201
#  define NEBTYPE_TIMEDEVENT_REMOVE                NEBTYPE_TIMEDEVENT_DELETE
#  define NEBTYPE_TIMEDEVENT_EXECUTE               202
#  define NEBTYPE_TIMEDEVENT_DELAY                 203 /* NOT IMPLEMENTED. */
#  define NEBTYPE_TIMEDEVENT_SKIP                  204 /* NOT IMPLEMENTED. */
#  define NEBTYPE_TIMEDEVENT_SLEEP                 205

/* Logs. */
#  define NEBTYPE_LOG_DATA                         300

/* System commands. */
#  define NEBTYPE_SYSTEM_COMMAND_START             400
#  define NEBTYPE_SYSTEM_COMMAND_END               401

/* Event handlers. */
#  define NEBTYPE_EVENTHANDLER_START               500
#  define NEBTYPE_EVENTHANDLER_END                 501

/* Service checks. */
#  define NEBTYPE_SERVICECHECK_INITIATE            700
#  define NEBTYPE_SERVICECHECK_PROCESSED           701
#  define NEBTYPE_SERVICECHECK_RAW_START           702 /* NOT IMPLEMENTED. */
#  define NEBTYPE_SERVICECHECK_RAW_END             703 /* NOT IMPLEMENTED. */
#  define NEBTYPE_SERVICECHECK_ASYNC_PRECHECK      704

/* Host checks. */
#  define NEBTYPE_HOSTCHECK_INITIATE               800 /* A check of the route to the host has been initiated. */
#  define NEBTYPE_HOSTCHECK_PROCESSED              801 /* The processed/final result of a host check. */
#  define NEBTYPE_HOSTCHECK_RAW_START              802 /* The start of a "raw" host check. */
#  define NEBTYPE_HOSTCHECK_RAW_END                803 /* A finished "raw" host check. */
#  define NEBTYPE_HOSTCHECK_ASYNC_PRECHECK         804
#  define NEBTYPE_HOSTCHECK_SYNC_PRECHECK          805

/* Flapping. */
#  define NEBTYPE_FLAPPING_START                   1000
#  define NEBTYPE_FLAPPING_STOP                    1001

/* Statuses. */
#  define NEBTYPE_PROGRAMSTATUS_UPDATE             1200
#  define NEBTYPE_HOSTSTATUS_UPDATE                1201
#  define NEBTYPE_SERVICESTATUS_UPDATE             1202

/* Adaptive modifications. */
#  define NEBTYPE_ADAPTIVEPROGRAM_UPDATE           1300
#  define NEBTYPE_ADAPTIVEHOST_UPDATE              1301
#  define NEBTYPE_ADAPTIVESERVICE_UPDATE           1302

/* External commands. */
#  define NEBTYPE_EXTERNALCOMMAND_START            1400
#  define NEBTYPE_EXTERNALCOMMAND_END              1401
#  define NEBTYPE_EXTERNALCOMMAND_CHECK            1402

/* Retention. */
#  define NEBTYPE_RETENTIONDATA_STARTLOAD          1600
#  define NEBTYPE_RETENTIONDATA_ENDLOAD            1601
#  define NEBTYPE_RETENTIONDATA_STARTSAVE          1602
#  define NEBTYPE_RETENTIONDATA_ENDSAVE            1603

/* State change. */
#  define NEBTYPE_STATECHANGE_START                1800    /* NOT IMPLEMENTED. */
#  define NEBTYPE_STATECHANGE_END                  1801

/* Commands. */
#  define NEBTYPE_COMMAND_ADD                      1900
#  define NEBTYPE_COMMAND_DELETE                   1901
#  define NEBTYPE_COMMAND_UPDATE                   1902

/* Hosts. */
#  define NEBTYPE_HOST_ADD                         2400
#  define NEBTYPE_HOST_DELETE                      2401
#  define NEBTYPE_HOST_UPDATE                      NEBTYPE_ADAPTIVEHOST_UPDATE

/* Host custom variables. */
#  define NEBTYPE_HOSTCUSTOMVARIABLE_ADD           2500
#  define NEBTYPE_HOSTCUSTOMVARIABLE_DELETE        2501
#  define NEBTYPE_HOSTCUSTOMVARIABLE_UPDATE        2502

/* Hostdependencies. */
#  define NEBTYPE_HOSTDEPENDENCY_ADD               2600
#  define NEBTYPE_HOSTDEPENDENCY_DELETE            2601
#  define NEBTYPE_HOSTDEPENDENCY_UPDATE            2602

/* Modules. */
#  define NEBTYPE_MODULE_ADD                       3000
#  define NEBTYPE_MODULE_DELETE                    3000

/* Parents. */
#  define NEBTYPE_PARENT_ADD                       3100
#  define NEBTYPE_PARENT_DELETE                    3101

/* Services. */
#  define NEBTYPE_SERVICE_ADD                      3200
#  define NEBTYPE_SERVICE_DELETE                   3201
#  define NEBTYPE_SERVICE_UPDATE                   NEBTYPE_ADAPTIVESERVICE_UPDATE

/* Service custom variables. */
#  define NEBTYPE_SERVICECUSTOMVARIABLE_ADD        3300
#  define NEBTYPE_SERVICECUSTOMVARIABLE_DELETE     3301
#  define NEBTYPE_SERVICECUSTOMVARIABLE_UPDATE     3302

/* Servicedependencies. */
#  define NEBTYPE_SERVICEDEPENDENCY_ADD            3400
#  define NEBTYPE_SERVICEDEPENDENCY_DELETE         3401
#  define NEBTYPE_SERVICEDEPENDENCY_UPDATE         3402

/* Timeperiod. */
#  define NEBTYPE_TIMEPERIOD_ADD                   3800
#  define NEBTYPE_TIMEPERIOD_DELETE                3801
#  define NEBTYPE_TIMEPERIOD_UPDATE                3802

/*
** Event flags.
*/
#  define NEBFLAG_NONE                             0
#  define NEBFLAG_PROCESS_INITIATED                1 /* Event was initiated by Engine process. */
#  define NEBFLAG_USER_INITIATED                   2 /* Event was initiated by a user request. */
#  define NEBFLAG_MODULE_INITIATED                 3 /* Event was initiated by an event broker module. */

/* Event attributes. */
#  define NEBATTR_NONE                             0

/* Termination. */
#  define NEBATTR_SHUTDOWN_NORMAL                  (1 << 0)
#  define NEBATTR_SHUTDOWN_ABNORMAL                (1 << 1)
#  define NEBATTR_RESTART_NORMAL                   (1 << 2)
#  define NEBATTR_RESTART_ABNORMAL                 (1 << 3)

/* Flapping. */
#  define NEBATTR_FLAPPING_STOP_NORMAL             1
#  define NEBATTR_FLAPPING_STOP_DISABLED           2 /* Flapping stopped because flap detection was disabled. */

// Forward declaration.
struct command_struct;
struct customvariablesmember_struct;
struct timed_event_struct;
struct timeperiod_struct;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

void           broker_adaptive_dependency_data(
                 int type,
                 int flags,
                 int attr,
                 void* data,
                 struct timeval const* timestamp);
void           broker_adaptive_host_data(
                 int type,
                 int flags,
                 int attr,
                 host* hst,
                 int command_type,
                 unsigned long modattr,
                 unsigned long modattrs,
                 struct timeval const* timestamp);
void           broker_adaptive_program_data(
                 int type,
                 int flags,
                 int attr,
                 int command_type,
                 unsigned long modhattr,
                 unsigned long modhattrs,
                 unsigned long modsattr,
                 unsigned long modsattrs,
                 struct timeval const* timestamp);
void           broker_adaptive_service_data(
                 int type,
                 int flags,
                 int attr,
                 service* svc,
                 int command_type,
                 unsigned long modattr,
                 unsigned long modattrs,
                 struct timeval const* timestamp);
void           broker_adaptive_timeperiod_data(
                 int type,
                 int flags,
                 int attr,
                 timeperiod_struct* tp,
                 int command_type,
                 struct timeval const* timestamp);
void           broker_aggregated_status_data(
                 int type,
                 int flags,
                 int attr,
                 struct timeval const* timestamp);
void           broker_command_data(
                 int type,
                 int flags,
                 int attr,
                 command_struct* cmd,
                 struct timeval const* timestamp);
void           broker_custom_variable(
                 int type,
                 int flags,
                 int attr,
                 void* data,
                 char const* varname,
                 char const* varvalue,
                 struct timeval const* timestamp);
int            broker_event_handler(
                 int type,
                 int flags,
                 int attr,
                 unsigned int eventhandler_type,
                 void* data,
                 int state,
                 int state_type,
                 struct timeval start_time,
                 struct timeval end_time,
                 double exectime,
                 int timeout,
                 int early_timeout,
                 int retcode,
                 char const* cmd,
                 char* cmdline,
                 char* output,
                 struct timeval const* timestamp);
void           broker_external_command(
                 int type,
                 int flags,
                 int attr,
                 int command_type,
                 time_t entry_time,
                 char* command_string,
                 char* command_args,
                 struct timeval const* timestamp);
void           broker_flapping_data(
                 int type,
                 int flags,
                 int attr,
                 unsigned int flapping_type,
                 void* data,
                 double percent_change,
                 double high_threshold,
                 double low_threshold,
                 struct timeval const* timestamp);
int            broker_host_check(
                 int type,
                 int flags,
                 int attr,
                 host* hst,
                 int check_type,
                 int state,
                 int state_type,
                 struct timeval start_time,
                 struct timeval end_time,
                 char const* cmd,
                 double latency,
                 double exectime,
                 int timeout,
                 int early_timeout,
                 int retcode,
                 char* cmdline,
                 char* output,
                 char* long_output,
                 char* perfdata,
                 struct timeval const* timestamp);
void           broker_host_status(
                 int type,
                 int flags,
                 int attr,
                 host* hst,
                 struct timeval const* timestamp);
void           broker_log_data(
                 int type,
                 int flags,
                 int attr,
                 char* data,
                 unsigned long data_type,
                 time_t entry_time,
                 struct timeval const* timestamp);
void           broker_module_data(
                 int type,
                 int flags,
                 int attr,
                 char const* module,
                 char const* args,
                 struct timeval const* timestamp);
void           broker_program_state(
                 int type,
                 int flags,
                 int attr,
                 struct timeval const* timestamp);
void           broker_program_status(
                 int type,
                 int flags,
                 int attr,
                 struct timeval const* timestamp);
void           broker_relation_data(
                 int type,
                 int flags,
                 int attr,
                 host* hst,
                 service* svc,
                 host* dep_hst,
                 service* dep_svc,
                 struct timeval const* timestamp);
void           broker_retention_data(
                 int type,
                 int flags,
                 int attr,
                 struct timeval const* timestamp);
int            broker_service_check(
                 int type,
                 int flags,
                 int attr,
                 service* svc,
                 int check_type,
                 struct timeval start_time,
                 struct timeval end_time,
                 char const* cmd,
                 double latency,
                 double exectime,
                 int timeout,
                 int early_timeout,
                 int retcode,
                 char* cmdline,
                 struct timeval const* timestamp);
void           broker_service_status(
                 int type,
                 int flags,
                 int attr,
                 service* svc,
                 struct timeval const* timestamp);
void           broker_statechange_data(
                 int type,
                 int flags,
                 int attr,
                 int statechange_type,
                 void* data,
                 int state,
                 int state_type,
                 int current_attempt,
                 int max_attempts,
                 struct timeval const* timestamp);
void           broker_system_command(
                 int type,
                 int flags,
                 int attr,
                 struct timeval start_time,
                 struct timeval end_time,
                 double exectime,
                 int timeout,
                 int early_timeout,
                 int retcode,
                 char* cmd,
                 char* output,
                 struct timeval const* timestamp);
void           broker_timed_event(
                 int type,
                 int flags,
                 int attr,
                 timed_event_struct* event,
                 struct timeval const* timestamp);
struct timeval get_broker_timestamp(struct timeval const* timestamp);

#  ifdef __cplusplus
}
#  endif /* C++ */

#endif /* !CCE_BROKER_HH */

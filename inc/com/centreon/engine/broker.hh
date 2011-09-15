/*
** Copyright 2002-2006 Ethan Galstad
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

#ifndef CCE_BROKER_HH
# define CCE_BROKER_HH

# include <sys/time.h>
# include "objects.hh"
# include "events.hh"

# ifdef __cplusplus
extern "C" {
# endif

// EVENT BROKER OPTIONS
# define BROKER_NOTHING                           0
# define BROKER_EVERYTHING                        1048575

# define BROKER_PROGRAM_STATE                     1	  // DONE
# define BROKER_TIMED_EVENTS                      2	  // DONE
# define BROKER_SERVICE_CHECKS                    4	  // DONE
# define BROKER_HOST_CHECKS                       8	  // DONE
# define BROKER_EVENT_HANDLERS                    16      // DONE
# define BROKER_LOGGED_DATA                       32      // DONE
# define BROKER_NOTIFICATIONS                     64      // DONE
# define BROKER_FLAPPING_DATA                     128     // DONE
# define BROKER_COMMENT_DATA                      256     // DONE
# define BROKER_DOWNTIME_DATA                     512     // DONE
# define BROKER_SYSTEM_COMMANDS                   1024    // DONE
# define BROKER_OCP_DATA_UNUSED                   2048    // reusable
# define BROKER_STATUS_DATA                       4096    // DONE
# define BROKER_ADAPTIVE_DATA                     8192    // DONE
# define BROKER_EXTERNALCOMMAND_DATA              16384   // DONE
# define BROKER_RETENTION_DATA                    32768   // DONE
# define BROKER_ACKNOWLEDGEMENT_DATA              65536
# define BROKER_STATECHANGE_DATA                  131072
# define BROKER_RESERVED18                        262144
# define BROKER_RESERVED19                        524288


// EVENT TYPES
# define NEBTYPE_NONE                             0

# define NEBTYPE_HELLO                            1
# define NEBTYPE_GOODBYE                          2
# define NEBTYPE_INFO                             3

# define NEBTYPE_PROCESS_START                    100
# define NEBTYPE_PROCESS_DAEMONIZE                101
# define NEBTYPE_PROCESS_RESTART                  102
# define NEBTYPE_PROCESS_SHUTDOWN                 103
# define NEBTYPE_PROCESS_PRELAUNCH                104     // before objects are read or verified
# define NEBTYPE_PROCESS_EVENTLOOPSTART           105
# define NEBTYPE_PROCESS_EVENTLOOPEND             106

# define NEBTYPE_TIMEDEVENT_ADD                   200
# define NEBTYPE_TIMEDEVENT_REMOVE                201
# define NEBTYPE_TIMEDEVENT_EXECUTE               202
# define NEBTYPE_TIMEDEVENT_DELAY                 203     // NOT IMPLEMENTED
# define NEBTYPE_TIMEDEVENT_SKIP                  204     // NOT IMPLEMENTED
# define NEBTYPE_TIMEDEVENT_SLEEP                 205

# define NEBTYPE_LOG_DATA                         300
# define NEBTYPE_LOG_ROTATION                     301

# define NEBTYPE_SYSTEM_COMMAND_START             400
# define NEBTYPE_SYSTEM_COMMAND_END               401

# define NEBTYPE_EVENTHANDLER_START               500
# define NEBTYPE_EVENTHANDLER_END                 501

# define NEBTYPE_NOTIFICATION_START               600
# define NEBTYPE_NOTIFICATION_END                 601
# define NEBTYPE_CONTACTNOTIFICATION_START        602
# define NEBTYPE_CONTACTNOTIFICATION_END          603
# define NEBTYPE_CONTACTNOTIFICATIONMETHOD_START  604
# define NEBTYPE_CONTACTNOTIFICATIONMETHOD_END    605

# define NEBTYPE_SERVICECHECK_INITIATE            700
# define NEBTYPE_SERVICECHECK_PROCESSED           701
# define NEBTYPE_SERVICECHECK_RAW_START           702     // NOT IMPLEMENTED
# define NEBTYPE_SERVICECHECK_RAW_END             703     // NOT IMPLEMENTED
# define NEBTYPE_SERVICECHECK_ASYNC_PRECHECK      704

# define NEBTYPE_HOSTCHECK_INITIATE               800     // a check of the route to the host has been initiated
# define NEBTYPE_HOSTCHECK_PROCESSED              801     // the processed/final result of a host check
# define NEBTYPE_HOSTCHECK_RAW_START              802     // the start of a "raw" host check
# define NEBTYPE_HOSTCHECK_RAW_END                803     // a finished "raw" host check
# define NEBTYPE_HOSTCHECK_ASYNC_PRECHECK         804
# define NEBTYPE_HOSTCHECK_SYNC_PRECHECK          805

# define NEBTYPE_COMMENT_ADD                      900
# define NEBTYPE_COMMENT_DELETE                   901
# define NEBTYPE_COMMENT_LOAD                     902

# define NEBTYPE_FLAPPING_START                   1000
# define NEBTYPE_FLAPPING_STOP                    1001

# define NEBTYPE_DOWNTIME_ADD                     1100
# define NEBTYPE_DOWNTIME_DELETE                  1101
# define NEBTYPE_DOWNTIME_LOAD                    1102
# define NEBTYPE_DOWNTIME_START                   1103
# define NEBTYPE_DOWNTIME_STOP                    1104

# define NEBTYPE_PROGRAMSTATUS_UPDATE             1200
# define NEBTYPE_HOSTSTATUS_UPDATE                1201
# define NEBTYPE_SERVICESTATUS_UPDATE             1202
# define NEBTYPE_CONTACTSTATUS_UPDATE             1203

# define NEBTYPE_ADAPTIVEPROGRAM_UPDATE           1300
# define NEBTYPE_ADAPTIVEHOST_UPDATE              1301
# define NEBTYPE_ADAPTIVESERVICE_UPDATE           1302
# define NEBTYPE_ADAPTIVECONTACT_UPDATE           1303

# define NEBTYPE_EXTERNALCOMMAND_START            1400
# define NEBTYPE_EXTERNALCOMMAND_END              1401
# define NEBTYPE_EXTERNALCOMMAND_CHECK            1402

# define NEBTYPE_AGGREGATEDSTATUS_STARTDUMP       1500
# define NEBTYPE_AGGREGATEDSTATUS_ENDDUMP         1501

# define NEBTYPE_RETENTIONDATA_STARTLOAD          1600
# define NEBTYPE_RETENTIONDATA_ENDLOAD            1601
# define NEBTYPE_RETENTIONDATA_STARTSAVE          1602
# define NEBTYPE_RETENTIONDATA_ENDSAVE            1603

# define NEBTYPE_ACKNOWLEDGEMENT_ADD              1700
# define NEBTYPE_ACKNOWLEDGEMENT_REMOVE           1701    // NOT IMPLEMENTED
# define NEBTYPE_ACKNOWLEDGEMENT_LOAD             1702    // NOT IMPLEMENTED

# define NEBTYPE_STATECHANGE_START                1800    // NOT IMPLEMENTED
# define NEBTYPE_STATECHANGE_END                  1801

// EVENT FLAGS
# define NEBFLAG_NONE                             0
# define NEBFLAG_PROCESS_INITIATED                1       // event was initiated by Engine process
# define NEBFLAG_USER_INITIATED                   2       // event was initiated by a user request
# define NEBFLAG_MODULE_INITIATED                 3       // event was initiated by an event broker module

// EVENT ATTRIBUTES
# define NEBATTR_NONE                             0

# define NEBATTR_SHUTDOWN_NORMAL                  1
# define NEBATTR_SHUTDOWN_ABNORMAL                2
# define NEBATTR_RESTART_NORMAL                   4
# define NEBATTR_RESTART_ABNORMAL                 8

# define NEBATTR_FLAPPING_STOP_NORMAL             1
# define NEBATTR_FLAPPING_STOP_DISABLED           2       // flapping stopped because flap detection was disabled

# define NEBATTR_DOWNTIME_STOP_NORMAL             1
# define NEBATTR_DOWNTIME_STOP_CANCELLED          2

void broker_program_state(int type, int flags, int attr, struct timeval* timestamp);
void broker_timed_event(int type, int flags, int attr, timed_event* event, struct timeval* timestamp);
void broker_log_data(int type, int flags, int attr, char* data, unsigned long data_type, time_t entry_time, struct timeval* timestamp);
void broker_system_command(int type, int flags, int attr, struct timeval start_time, struct timeval end_time, double exectime, int timeout, int early_timeout, int retcode, char* cmd, char* output, struct timeval* timestamp);
int broker_event_handler(int type, int flags, int attr, unsigned int eventhandler_type, void* data, int state, int state_type, struct timeval start_time, struct timeval end_time, double exectime, int timeout, int early_timeout, int retcode, char const* cmd, char* cmdline, char* output, struct timeval* timestamp);
int broker_host_check(int type, int flags, int attr, host* hst, int check_type, int state, int state_type, struct timeval start_time, struct timeval end_time, char* cmd, double latency, double exectime, int timeout, int early_timeout, int retcode, char* cmdline, char* output, char* long_output, char* perfdata, struct timeval* timestamp);
int broker_service_check(int type, int flags, int attr, service* svc, int check_type, struct timeval start_time, struct timeval end_time, char* cmd, double latency, double exectime, int timeout, int early_timeout, int retcode, char* cmdline, struct timeval* timestamp);
void broker_comment_data(int type, int flags, int attr, int comment_type, int entry_type, char const* host_name, char const* svc_description, time_t entry_time, char const* author_name, char* comment_data, int persistent, int source, int expires, time_t expire_time, unsigned long comment_id, struct timeval* timestamp);
void broker_downtime_data(int type, int flags, int attr, int downtime_type, char* host_name, char* svc_description, time_t entry_time, char* author_name, char* comment_data, time_t start_time, time_t end_time, int fixed, unsigned long triggered_by, unsigned long duration, unsigned long downtime_id, struct timeval* timestamp);
void broker_flapping_data(int type, int flags, int attr, unsigned int flapping_type, void* data, double percent_change, double high_threshold, double low_threshold, struct timeval* timestamp);
void broker_program_status(int type, int flags, int attr, struct timeval* timestamp);
void broker_host_status(int type, int flags, int attr, host* hst, struct timeval* timestamp);
void broker_service_status(int type, int flags, int attr, service* svc, struct timeval* timestamp);
void broker_contact_status(int type, int flags, int attr, contact* cntct, struct timeval* timestamp);
int broker_notification_data(int type, int flags, int attr, unsigned int notification_type, int reason_type, struct timeval start_time, struct timeval end_time, void* data, char* ack_author, char* ack_data, int escalated, int contacts_notified, struct timeval* timestamp);
int broker_contact_notification_data(int type, int flags, int attr, unsigned int notification_type, int reason_type, struct timeval start_time, struct timeval end_time, void* data, contact* cntct, char* ack_author, char* ack_data, int escalated, struct timeval* timestamp);
int broker_contact_notification_method_data(int type, int flags, int attr, unsigned int notification_type, int reason_type, struct timeval start_time, struct timeval end_time, void* data, contact* cntct, char* cmd, char* ack_author, char* ack_data, int escalated, struct timeval* timestamp);
void broker_adaptive_program_data(int type, int flags, int attr, int command_type, unsigned long modhattr, unsigned long modhattrs, unsigned long modsattr, unsigned long modsattrs, struct timeval* timestamp);
void broker_adaptive_host_data(int type, int flags, int attr, host* hst, int command_type, unsigned long modattr, unsigned long modattrs, struct timeval* timestamp);
void broker_adaptive_service_data(int type, int flags, int attr, service* svc, int command_type, unsigned long modattr, unsigned long modattrs, struct timeval* timestamp);
void broker_adaptive_contact_data(int type, int flags, int attr, contact* cntct, int command_type, unsigned long modattr, unsigned long modattrs, unsigned long modhattr, unsigned long modhattrs, unsigned long modsattr, unsigned long modsattrs, struct timeval* timestamp);
void broker_external_command(int type, int flags, int attr, int command_type, time_t entry_time, char* command_string, char* command_args, struct timeval* timestamp);
void broker_aggregated_status_data(int type, int flags, int attr, struct timeval* timestamp);
void broker_retention_data(int type, int flags, int attr, struct timeval* timestamp);
void broker_acknowledgement_data(int type, int flags, int attr, int acknowledgement_type, void* data, char* ack_author, char* ack_data, int subtype, int notify_contacts, int persistent_comment, struct timeval* timestamp);
void broker_statechange_data(int type, int flags, int attr, int statechange_type, void* data, int state, int state_type, int current_attempt, int max_attempts, struct timeval* timestamp);
struct timeval get_broker_timestamp(struct timeval* timestamp);

# ifdef __cplusplus
}
# endif

#endif // !CCE_BROKER_HH

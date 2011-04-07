/*
** Copyright 2002-2006 Ethan Galstad
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

#ifndef CCS_BROKER_HH
# define CCS_BROKER_HH

# include <sys/time.h>
# include "objects.hh"
# include "events.hh"

# ifdef __cplusplus
extern "C" {
# endif

// EVENT BROKER OPTIONS
static const unsigned int BROKER_NOTHING                           = 0;
static const unsigned int BROKER_EVERYTHING                        = 1048575;

static const unsigned int BROKER_PROGRAM_STATE                     = 1;	     // DONE
static const unsigned int BROKER_TIMED_EVENTS                      = 2;	     // DONE
static const unsigned int BROKER_SERVICE_CHECKS                    = 4;	     // DONE
static const unsigned int BROKER_HOST_CHECKS                       = 8;	     // DONE
static const unsigned int BROKER_EVENT_HANDLERS                    = 16;     // DONE
static const unsigned int BROKER_LOGGED_DATA                       = 32;     // DONE
static const unsigned int BROKER_NOTIFICATIONS                     = 64;     // DONE
static const unsigned int BROKER_FLAPPING_DATA                     = 128;    // DONE
static const unsigned int BROKER_COMMENT_DATA                      = 256;    // DONE
static const unsigned int BROKER_DOWNTIME_DATA                     = 512;    // DONE
static const unsigned int BROKER_SYSTEM_COMMANDS                   = 1024;   // DONE
static const unsigned int BROKER_OCP_DATA_UNUSED                   = 2048;   // reusable
static const unsigned int BROKER_STATUS_DATA                       = 4096;   // DONE
static const unsigned int BROKER_ADAPTIVE_DATA                     = 8192;   // DONE
static const unsigned int BROKER_EXTERNALCOMMAND_DATA              = 16384;  // DONE
static const unsigned int BROKER_RETENTION_DATA                    = 32768;  // DONE
static const unsigned int BROKER_ACKNOWLEDGEMENT_DATA              = 65536;
static const unsigned int BROKER_STATECHANGE_DATA                  = 131072;
static const unsigned int BROKER_RESERVED18                        = 262144;
static const unsigned int BROKER_RESERVED19                        = 524288;


// EVENT TYPES
static const unsigned int NEBTYPE_NONE                             = 0;

static const unsigned int NEBTYPE_HELLO                            = 1;
static const unsigned int NEBTYPE_GOODBYE                          = 2;
static const unsigned int NEBTYPE_INFO                             = 3;

static const unsigned int NEBTYPE_PROCESS_START                    = 100;
static const unsigned int NEBTYPE_PROCESS_DAEMONIZE                = 101;
static const unsigned int NEBTYPE_PROCESS_RESTART                  = 102;
static const unsigned int NEBTYPE_PROCESS_SHUTDOWN                 = 103;
static const unsigned int NEBTYPE_PROCESS_PRELAUNCH                = 104;    // before objects are read or verified
static const unsigned int NEBTYPE_PROCESS_EVENTLOOPSTART           = 105;
static const unsigned int NEBTYPE_PROCESS_EVENTLOOPEND             = 106;

static const unsigned int NEBTYPE_TIMEDEVENT_ADD                   = 200;
static const unsigned int NEBTYPE_TIMEDEVENT_REMOVE                = 201;
static const unsigned int NEBTYPE_TIMEDEVENT_EXECUTE               = 202;
static const unsigned int NEBTYPE_TIMEDEVENT_DELAY                 = 203;    // NOT IMPLEMENTED
static const unsigned int NEBTYPE_TIMEDEVENT_SKIP                  = 204;    // NOT IMPLEMENTED
static const unsigned int NEBTYPE_TIMEDEVENT_SLEEP                 = 205;

static const unsigned int NEBTYPE_LOG_DATA                         = 300;
static const unsigned int NEBTYPE_LOG_ROTATION                     = 301;

static const unsigned int NEBTYPE_SYSTEM_COMMAND_START             = 400;
static const unsigned int NEBTYPE_SYSTEM_COMMAND_END               = 401;

static const unsigned int NEBTYPE_EVENTHANDLER_START               = 500;
static const unsigned int NEBTYPE_EVENTHANDLER_END                 = 501;

static const unsigned int NEBTYPE_NOTIFICATION_START               = 600;
static const unsigned int NEBTYPE_NOTIFICATION_END                 = 601;
static const unsigned int NEBTYPE_CONTACTNOTIFICATION_START        = 602;
static const unsigned int NEBTYPE_CONTACTNOTIFICATION_END          = 603;
static const unsigned int NEBTYPE_CONTACTNOTIFICATIONMETHOD_START  = 604;
static const unsigned int NEBTYPE_CONTACTNOTIFICATIONMETHOD_END    = 605;

static const unsigned int NEBTYPE_SERVICECHECK_INITIATE            = 700;
static const unsigned int NEBTYPE_SERVICECHECK_PROCESSED           = 701;
static const unsigned int NEBTYPE_SERVICECHECK_RAW_START           = 702;    // NOT IMPLEMENTED
static const unsigned int NEBTYPE_SERVICECHECK_RAW_END             = 703;    // NOT IMPLEMENTED
static const unsigned int NEBTYPE_SERVICECHECK_ASYNC_PRECHECK      = 704;

static const unsigned int NEBTYPE_HOSTCHECK_INITIATE               = 800;    // a check of the route to the host has been initiated
static const unsigned int NEBTYPE_HOSTCHECK_PROCESSED              = 801;    // the processed/final result of a host check
static const unsigned int NEBTYPE_HOSTCHECK_RAW_START              = 802;    // the start of a "raw" host check
static const unsigned int NEBTYPE_HOSTCHECK_RAW_END                = 803;    // a finished "raw" host check
static const unsigned int NEBTYPE_HOSTCHECK_ASYNC_PRECHECK         = 804;
static const unsigned int NEBTYPE_HOSTCHECK_SYNC_PRECHECK          = 805;

static const unsigned int NEBTYPE_COMMENT_ADD                      = 900;
static const unsigned int NEBTYPE_COMMENT_DELETE                   = 901;
static const unsigned int NEBTYPE_COMMENT_LOAD                     = 902;

static const unsigned int NEBTYPE_FLAPPING_START                   = 1000;
static const unsigned int NEBTYPE_FLAPPING_STOP                    = 1001;

static const unsigned int NEBTYPE_DOWNTIME_ADD                     = 1100;
static const unsigned int NEBTYPE_DOWNTIME_DELETE                  = 1101;
static const unsigned int NEBTYPE_DOWNTIME_LOAD                    = 1102;
static const unsigned int NEBTYPE_DOWNTIME_START                   = 1103;
static const unsigned int NEBTYPE_DOWNTIME_STOP                    = 1104;

static const unsigned int NEBTYPE_PROGRAMSTATUS_UPDATE             = 1200;
static const unsigned int NEBTYPE_HOSTSTATUS_UPDATE                = 1201;
static const unsigned int NEBTYPE_SERVICESTATUS_UPDATE             = 1202;
static const unsigned int NEBTYPE_CONTACTSTATUS_UPDATE             = 1203;

static const unsigned int NEBTYPE_ADAPTIVEPROGRAM_UPDATE           = 1300;
static const unsigned int NEBTYPE_ADAPTIVEHOST_UPDATE              = 1301;
static const unsigned int NEBTYPE_ADAPTIVESERVICE_UPDATE           = 1302;
static const unsigned int NEBTYPE_ADAPTIVECONTACT_UPDATE           = 1303;

static const unsigned int NEBTYPE_EXTERNALCOMMAND_START            = 1400;
static const unsigned int NEBTYPE_EXTERNALCOMMAND_END              = 1401;

static const unsigned int NEBTYPE_AGGREGATEDSTATUS_STARTDUMP       = 1500;
static const unsigned int NEBTYPE_AGGREGATEDSTATUS_ENDDUMP         = 1501;

static const unsigned int NEBTYPE_RETENTIONDATA_STARTLOAD          = 1600;
static const unsigned int NEBTYPE_RETENTIONDATA_ENDLOAD            = 1601;
static const unsigned int NEBTYPE_RETENTIONDATA_STARTSAVE          = 1602;
static const unsigned int NEBTYPE_RETENTIONDATA_ENDSAVE            = 1603;

static const unsigned int NEBTYPE_ACKNOWLEDGEMENT_ADD              = 1700;
static const unsigned int NEBTYPE_ACKNOWLEDGEMENT_REMOVE           = 1701;    // NOT IMPLEMENTED
static const unsigned int NEBTYPE_ACKNOWLEDGEMENT_LOAD             = 1702;    // NOT IMPLEMENTED

static const unsigned int NEBTYPE_STATECHANGE_START                = 1800;    // NOT IMPLEMENTED
static const unsigned int NEBTYPE_STATECHANGE_END                  = 1801;

// EVENT FLAGS
static const unsigned int NEBFLAG_NONE                             = 0;
static const unsigned int NEBFLAG_PROCESS_INITIATED                = 1;       // event was initiated by Nagios process
static const unsigned int NEBFLAG_USER_INITIATED                   = 2;       // event was initiated by a user request
static const unsigned int NEBFLAG_MODULE_INITIATED                 = 3;       // event was initiated by an event broker module

// EVENT ATTRIBUTES
static const unsigned int NEBATTR_NONE                             = 0;

static const unsigned int NEBATTR_SHUTDOWN_NORMAL                  = 1;
static const unsigned int NEBATTR_SHUTDOWN_ABNORMAL                = 2;
static const unsigned int NEBATTR_RESTART_NORMAL                   = 4;
static const unsigned int NEBATTR_RESTART_ABNORMAL                 = 8;

static const unsigned int NEBATTR_FLAPPING_STOP_NORMAL             = 1;
static const unsigned int NEBATTR_FLAPPING_STOP_DISABLED           = 2;       // flapping stopped because flap detection was disabled

static const unsigned int NEBATTR_DOWNTIME_STOP_NORMAL             = 1;
static const unsigned int NEBATTR_DOWNTIME_STOP_CANCELLED          = 2;

# ifdef USE_EVENT_BROKER
void broker_program_state(int type, int flags, int attr, timeval* timestamp);
void broker_timed_event(int type, int flags, int attr, timed_event* event, timeval* timestamp);
void broker_log_data(int type, int flags, int attr, char* data, unsigned long data_type, time_t entry_time, timeval* timestamp);
void broker_system_command(int type, int flags, int attr, timeval start_time, timeval end_time, double exectime, int timeout, int early_timeout, int retcode, char* cmd, char* output, timeval* timestamp);
int broker_event_handler(int type, int flags, int attr, unsigned int eventhandler_type, void* data, int state, int state_type, timeval start_time, timeval end_time, double exectime, int timeout, int early_timeout, int retcode, char const* cmd, char* cmdline, char* output, timeval* timestamp);
int broker_host_check(int type, int flags, int attr, host* hst, int check_type, int state, int state_type, timeval start_time, timeval end_time, char* cmd, double latency, double exectime, int timeout, int early_timeout, int retcode, char* cmdline, char* output, char* long_output, char* perfdata, timeval* timestamp);
int broker_service_check(int type, int flags, int attr, service* svc, int check_type, timeval start_time, timeval end_time, char* cmd, double latency, double exectime, int timeout, int early_timeout, int retcode, char* cmdline, timeval* timestamp);
void broker_comment_data(int type, int flags, int attr, int comment_type, int entry_type, char const* host_name, char const* svc_description, time_t entry_time, char const* author_name, char* comment_data, int persistent, int source, int expires, time_t expire_time, unsigned long comment_id, timeval* timestamp);
void broker_downtime_data(int type, int flags, int attr, int downtime_type, char* host_name, char* svc_description, time_t entry_time, char* author_name, char* comment_data, time_t start_time, time_t end_time, int fixed, unsigned long triggered_by, unsigned long duration, unsigned long downtime_id, timeval* timestamp);
void broker_flapping_data(int type, int flags, int attr, unsigned int flapping_type, void* data, double percent_change, double high_threshold, double low_threshold, timeval* timestamp);
void broker_program_status(int type, int flags, int attr, timeval* timestamp);
void broker_host_status(int type, int flags, int attr, host* hst, timeval* timestamp);
void broker_service_status(int type, int flags, int attr, service* svc, timeval* timestamp);
void broker_contact_status(int type, int flags, int attr, contact* cntct, timeval* timestamp);
int broker_notification_data(int type, int flags, int attr, unsigned int notification_type, int reason_type, timeval start_time, timeval end_time, void* data, char* ack_author, char* ack_data, int escalated, int contacts_notified, timeval* timestamp);
int broker_contact_notification_data(int type, int flags, int attr, unsigned int notification_type, int reason_type, timeval start_time, timeval end_time, void* data, contact* cntct, char* ack_author, char* ack_data, int escalated, timeval* timestamp);
int broker_contact_notification_method_data(int type, int flags, int attr, unsigned int notification_type, int reason_type, timeval start_time, timeval end_time, void* data, contact* cntct, char* cmd, char* ack_author, char* ack_data, int escalated, timeval* timestamp);
void broker_adaptive_program_data(int type, int flags, int attr, int command_type, unsigned long modhattr, unsigned long modhattrs, unsigned long modsattr, unsigned long modsattrs, timeval* timestamp);
void broker_adaptive_host_data(int type, int flags, int attr, host* hst, int command_type, unsigned long modattr, unsigned long modattrs, timeval* timestamp);
void broker_adaptive_service_data(int type, int flags, int attr, service* svc, int command_type, unsigned long modattr, unsigned long modattrs, timeval* timestamp);
void broker_adaptive_contact_data(int type, int flags, int attr, contact* cntct, int command_type, unsigned long modattr, unsigned long modattrs, unsigned long modhattr, unsigned long modhattrs, unsigned long modsattr, unsigned long modsattrs, timeval* timestamp);
void broker_external_command(int type, int flags, int attr, int command_type, time_t entry_time, char* command_string, char* command_args, timeval* timestamp);
void broker_aggregated_status_data(int type, int flags, int attr, timeval* timestamp);
void broker_retention_data(int type, int flags, int attr, timeval* timestamp);
void broker_acknowledgement_data(int type, int flags, int attr, int acknowledgement_type, void* data, char* ack_author, char* ack_data, int subtype, int notify_contacts, int persistent_comment, timeval* timestamp);
void broker_statechange_data(int type, int flags, int attr, int statechange_type, void* data, int state, int state_type, int current_attempt, int max_attempts, timeval* timestamp);
timeval get_broker_timestamp(timeval* timestamp);
# endif // !USE_EVENT_BROKER

# ifdef __cplusplus
}
# endif

#endif // !CCS_BROKER_HH

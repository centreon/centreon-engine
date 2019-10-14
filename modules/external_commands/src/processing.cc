/*
** Copyright 2011-2013,2015-2016 Centreon
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

#include <cstdlib>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/modules/external_commands/processing.hh"
#include "com/centreon/engine/retention/applier/state.hh"
#include "com/centreon/engine/retention/dump.hh"
#include "com/centreon/engine/retention/parser.hh"
#include "com/centreon/engine/retention/state.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules::external_commands;

// Dummy command.
void dummy_command() {}

processing::processing()
    : _lst_command{
          {"ENTER_STANDBY_MODE",
           command_info(CMD_DISABLE_NOTIFICATIONS,
                        &_redirector<&disable_all_notifications>)},
          {"DISABLE_NOTIFICATIONS",
           command_info(CMD_DISABLE_NOTIFICATIONS,
                        &_redirector<&disable_all_notifications>)},
          // process commands.
          {"ENTER_ACTIVE_MODE",
           command_info(CMD_ENABLE_NOTIFICATIONS,
                        &_redirector<&enable_all_notifications>)},
          {"ENABLE_NOTIFICATIONS",
           command_info(CMD_ENABLE_NOTIFICATIONS,
                        &_redirector<&enable_all_notifications>)},
          {"SHUTDOWN_PROGRAM", command_info(CMD_SHUTDOWN_PROCESS,
                                            &_redirector<&cmd_signal_process>)},
          {"SHUTDOWN_PROCESS", command_info(CMD_SHUTDOWN_PROCESS,
                                            &_redirector<&cmd_signal_process>)},
          {"RESTART_PROGRAM", command_info(CMD_RESTART_PROCESS,
                                           &_redirector<&cmd_signal_process>)},
          {"RESTART_PROCESS", command_info(CMD_RESTART_PROCESS,
                                           &_redirector<&cmd_signal_process>)},
          {"SAVE_STATE_INFORMATION",
           command_info(CMD_SAVE_STATE_INFORMATION,
                        &_redirector<&_wrapper_save_state_information>)},
          {"READ_STATE_INFORMATION",
           command_info(CMD_READ_STATE_INFORMATION,
                        &_redirector<&_wrapper_read_state_information>)},
          {"ENABLE_EVENT_HANDLERS",
           command_info(CMD_ENABLE_EVENT_HANDLERS,
                        &_redirector<&start_using_event_handlers>)},
          {"DISABLE_EVENT_HANDLERS",
           command_info(CMD_DISABLE_EVENT_HANDLERS,
                        &_redirector<&stop_using_event_handlers>)},
          // _lst_command["FLUSH_PENDING_COMMANDS"] =
          //   command_info(CMD_FLUSH_PENDING_COMMANDS,
          //                &_redirector<&>);
          {"ENABLE_FAILURE_PREDICTION",
           command_info(CMD_ENABLE_FAILURE_PREDICTION,
                        &_redirector<&dummy_command>)},
          {"DISABLE_FAILURE_PREDICTION",
           command_info(CMD_DISABLE_FAILURE_PREDICTION,
                        &_redirector<&dummy_command>)},
          {"ENABLE_PERFORMANCE_DATA",
           command_info(CMD_ENABLE_PERFORMANCE_DATA,
                        &_redirector<&enable_performance_data>)},
          {"DISABLE_PERFORMANCE_DATA",
           command_info(CMD_DISABLE_PERFORMANCE_DATA,
                        &_redirector<&disable_performance_data>)},
          {"START_EXECUTING_HOST_CHECKS",
           command_info(CMD_START_EXECUTING_HOST_CHECKS,
                        &_redirector<&start_executing_host_checks>)},
          {"STOP_EXECUTING_HOST_CHECKS",
           command_info(CMD_STOP_EXECUTING_HOST_CHECKS,
                        &_redirector<&stop_executing_host_checks>)},
          {"START_EXECUTING_SVC_CHECKS",
           command_info(CMD_START_EXECUTING_SVC_CHECKS,
                        &_redirector<&start_executing_service_checks>)},
          {"STOP_EXECUTING_SVC_CHECKS",
           command_info(CMD_STOP_EXECUTING_SVC_CHECKS,
                        &_redirector<&stop_executing_service_checks>)},
          {"START_ACCEPTING_PASSIVE_HOST_CHECKS",
           command_info(CMD_START_ACCEPTING_PASSIVE_HOST_CHECKS,
                        &_redirector<&start_accepting_passive_host_checks>)},
          {"STOP_ACCEPTING_PASSIVE_HOST_CHECKS",
           command_info(CMD_STOP_ACCEPTING_PASSIVE_HOST_CHECKS,
                        &_redirector<&stop_accepting_passive_host_checks>)},
          {"START_ACCEPTING_PASSIVE_SVC_CHECKS",
           command_info(CMD_START_ACCEPTING_PASSIVE_SVC_CHECKS,
                        &_redirector<&start_accepting_passive_service_checks>)},
          {"STOP_ACCEPTING_PASSIVE_SVC_CHECKS",
           command_info(CMD_STOP_ACCEPTING_PASSIVE_SVC_CHECKS,
                        &_redirector<&stop_accepting_passive_service_checks>)},
          {"START_OBSESSING_OVER_HOST_CHECKS",
           command_info(CMD_START_OBSESSING_OVER_HOST_CHECKS,
                        &_redirector<&start_obsessing_over_host_checks>)},
          {"STOP_OBSESSING_OVER_HOST_CHECKS",
           command_info(CMD_STOP_OBSESSING_OVER_HOST_CHECKS,
                        &_redirector<&stop_obsessing_over_host_checks>)},
          {"START_OBSESSING_OVER_SVC_CHECKS",
           command_info(CMD_START_OBSESSING_OVER_SVC_CHECKS,
                        &_redirector<&start_obsessing_over_service_checks>)},
          {"STOP_OBSESSING_OVER_SVC_CHECKS",
           command_info(CMD_STOP_OBSESSING_OVER_SVC_CHECKS,
                        &_redirector<&stop_obsessing_over_service_checks>)},
          {"ENABLE_FLAP_DETECTION",
           command_info(CMD_ENABLE_FLAP_DETECTION,
                        &_redirector<&enable_flap_detection_routines>)},
          {"DISABLE_FLAP_DETECTION",
           command_info(CMD_DISABLE_FLAP_DETECTION,
                        &_redirector<&disable_flap_detection_routines>)},
          {"CHANGE_GLOBAL_HOST_EVENT_HANDLER",
           command_info(CMD_CHANGE_GLOBAL_HOST_EVENT_HANDLER,
                        &_redirector<&cmd_change_object_char_var>)},
          {"CHANGE_GLOBAL_SVC_EVENT_HANDLER",
           command_info(CMD_CHANGE_GLOBAL_SVC_EVENT_HANDLER,
                        &_redirector<&cmd_change_object_char_var>)},
          {"ENABLE_SERVICE_FRESHNESS_CHECKS",
           command_info(CMD_ENABLE_SERVICE_FRESHNESS_CHECKS,
                        &_redirector<&enable_service_freshness_checks>)},
          {"DISABLE_SERVICE_FRESHNESS_CHECKS",
           command_info(CMD_DISABLE_SERVICE_FRESHNESS_CHECKS,
                        &_redirector<&disable_service_freshness_checks>)},
          {"ENABLE_HOST_FRESHNESS_CHECKS",
           command_info(CMD_ENABLE_HOST_FRESHNESS_CHECKS,
                        &_redirector<&enable_host_freshness_checks>)},
          {"DISABLE_HOST_FRESHNESS_CHECKS",
           command_info(CMD_DISABLE_HOST_FRESHNESS_CHECKS,
                        &_redirector<&disable_host_freshness_checks>)},
          // host-related commands.
          {"ADD_HOST_COMMENT",
           command_info(CMD_ADD_HOST_COMMENT, &_redirector<&cmd_add_comment>)},
          {"DEL_HOST_COMMENT", command_info(CMD_DEL_HOST_COMMENT,
                                            &_redirector<&cmd_delete_comment>)},
          {"DEL_ALL_HOST_COMMENTS",
           command_info(CMD_DEL_ALL_HOST_COMMENTS,
                        &_redirector<&cmd_delete_all_comments>)},
          {"DELAY_HOST_NOTIFICATION",
           command_info(CMD_DELAY_HOST_NOTIFICATION,
                        &_redirector<&cmd_delay_notification>)},
          {"ENABLE_HOST_NOTIFICATIONS",
           command_info(CMD_ENABLE_HOST_NOTIFICATIONS,
                        &_redirector_host<&enable_host_notifications>)},
          {"DISABLE_HOST_NOTIFICATIONS",
           command_info(CMD_DISABLE_HOST_NOTIFICATIONS,
                        &_redirector_host<&disable_host_notifications>)},
          {"ENABLE_ALL_NOTIFICATIONS_BEYOND_HOST",
           command_info(CMD_ENABLE_ALL_NOTIFICATIONS_BEYOND_HOST,
                        &_redirector_host<
                             &_wrapper_enable_all_notifications_beyond_host>)},
          {"DISABLE_ALL_NOTIFICATIONS_BEYOND_HOST",
           command_info(CMD_DISABLE_ALL_NOTIFICATIONS_BEYOND_HOST,
                        &_redirector_host<
                             &_wrapper_disable_all_notifications_beyond_host>)},
          {"ENABLE_HOST_AND_CHILD_NOTIFICATIONS",
           command_info(CMD_ENABLE_HOST_AND_CHILD_NOTIFICATIONS,
                        &_redirector_host<
                             &_wrapper_enable_host_and_child_notifications>)},
          {"DISABLE_HOST_AND_CHILD_NOTIFICATIONS",
           command_info(CMD_DISABLE_HOST_AND_CHILD_NOTIFICATIONS,
                        &_redirector_host<
                             &_wrapper_disable_host_and_child_notifications>)},
          {"ENABLE_HOST_SVC_NOTIFICATIONS",
           command_info(
               CMD_ENABLE_HOST_SVC_NOTIFICATIONS,
               &_redirector_host<&_wrapper_enable_host_svc_notifications>)},
          {"DISABLE_HOST_SVC_NOTIFICATIONS",
           command_info(
               CMD_DISABLE_HOST_SVC_NOTIFICATIONS,
               &_redirector_host<&_wrapper_disable_host_svc_notifications>)},
          {"ENABLE_HOST_SVC_CHECKS",
           command_info(CMD_ENABLE_HOST_SVC_CHECKS,
                        &_redirector_host<&_wrapper_enable_host_svc_checks>)},
          {"DISABLE_HOST_SVC_CHECKS",
           command_info(CMD_DISABLE_HOST_SVC_CHECKS,
                        &_redirector_host<&_wrapper_disable_host_svc_checks>)},
          {"ENABLE_PASSIVE_HOST_CHECKS",
           command_info(CMD_ENABLE_PASSIVE_HOST_CHECKS,
                        &_redirector_host<&enable_passive_host_checks>)},
          {"DISABLE_PASSIVE_HOST_CHECKS",
           command_info(CMD_DISABLE_PASSIVE_HOST_CHECKS,
                        &_redirector_host<&disable_passive_host_checks>)},
          {"SCHEDULE_HOST_SVC_CHECKS",
           command_info(CMD_SCHEDULE_HOST_SVC_CHECKS,
                        &_redirector<&cmd_schedule_check>)},
          {"SCHEDULE_FORCED_HOST_SVC_CHECKS",
           command_info(CMD_SCHEDULE_FORCED_HOST_SVC_CHECKS,
                        &_redirector<&cmd_schedule_check>)},
          {"ACKNOWLEDGE_HOST_PROBLEM",
           command_info(CMD_ACKNOWLEDGE_HOST_PROBLEM,
                        &_redirector<&cmd_acknowledge_problem>)},
          {"REMOVE_HOST_ACKNOWLEDGEMENT",
           command_info(CMD_REMOVE_HOST_ACKNOWLEDGEMENT,
                        &_redirector<&cmd_remove_acknowledgement>)},
          {"ENABLE_HOST_EVENT_HANDLER",
           command_info(CMD_ENABLE_HOST_EVENT_HANDLER,
                        &_redirector_host<&enable_host_event_handler>)},
          {"DISABLE_HOST_EVENT_HANDLER",
           command_info(CMD_DISABLE_HOST_EVENT_HANDLER,
                        &_redirector_host<&disable_host_event_handler>)},
          {"ENABLE_HOST_CHECK",
           command_info(CMD_ENABLE_HOST_CHECK,
                        &_redirector_host<&enable_host_checks>)},
          {"DISABLE_HOST_CHECK",
           command_info(CMD_DISABLE_HOST_CHECK,
                        &_redirector_host<&disable_host_checks>)},
          {"SCHEDULE_HOST_CHECK",
           command_info(CMD_SCHEDULE_HOST_CHECK,
                        &_redirector<&cmd_schedule_check>)},
          {"SCHEDULE_FORCED_HOST_CHECK",
           command_info(CMD_SCHEDULE_FORCED_HOST_CHECK,
                        &_redirector<&cmd_schedule_check>)},
          {"SCHEDULE_HOST_DOWNTIME",
           command_info(CMD_SCHEDULE_HOST_DOWNTIME,
                        &_redirector<&cmd_schedule_downtime>)},
          {"SCHEDULE_HOST_SVC_DOWNTIME",
           command_info(CMD_SCHEDULE_HOST_SVC_DOWNTIME,
                        &_redirector<&cmd_schedule_downtime>)},
          {"DEL_HOST_DOWNTIME",
           command_info(CMD_DEL_HOST_DOWNTIME,
                        &_redirector<&cmd_delete_downtime>)},
          {"DEL_HOST_DOWNTIME_FULL",
           command_info(CMD_DEL_HOST_DOWNTIME_FULL,
                        &_redirector<&cmd_delete_downtime_full>)},
          {"DEL_DOWNTIME_BY_HOST_NAME",
           command_info(CMD_DEL_DOWNTIME_BY_HOST_NAME,
                        &_redirector<&cmd_delete_downtime_by_host_name>)},
          {"DEL_DOWNTIME_BY_HOSTGROUP_NAME",
           command_info(CMD_DEL_DOWNTIME_BY_HOSTGROUP_NAME,
                        &_redirector<&cmd_delete_downtime_by_hostgroup_name>)},
          {"DEL_DOWNTIME_BY_START_TIME_COMMENT",
           command_info(
               CMD_DEL_DOWNTIME_BY_START_TIME_COMMENT,
               &_redirector<&cmd_delete_downtime_by_start_time_comment>)},
          {"ENABLE_HOST_FLAP_DETECTION",
           command_info(CMD_ENABLE_HOST_FLAP_DETECTION,
                        &_redirector_host<&enable_host_flap_detection>)},
          {"DISABLE_HOST_FLAP_DETECTION",
           command_info(CMD_DISABLE_HOST_FLAP_DETECTION,
                        &_redirector_host<&disable_host_flap_detection>)},
          {"START_OBSESSING_OVER_HOST",
           command_info(CMD_START_OBSESSING_OVER_HOST,
                        &_redirector_host<&start_obsessing_over_host>)},
          {"STOP_OBSESSING_OVER_HOST",
           command_info(CMD_STOP_OBSESSING_OVER_HOST,
                        &_redirector_host<&stop_obsessing_over_host>)},
          {"CHANGE_HOST_EVENT_HANDLER",
           command_info(CMD_CHANGE_HOST_EVENT_HANDLER,
                        &_redirector<&cmd_change_object_char_var>)},
          {"CHANGE_HOST_CHECK_COMMAND",
           command_info(CMD_CHANGE_HOST_CHECK_COMMAND,
                        &_redirector<&cmd_change_object_char_var>)},
          {"CHANGE_NORMAL_HOST_CHECK_INTERVAL",
           command_info(CMD_CHANGE_NORMAL_HOST_CHECK_INTERVAL,
                        &_redirector<&cmd_change_object_int_var>)},
          {"CHANGE_RETRY_HOST_CHECK_INTERVAL",
           command_info(CMD_CHANGE_RETRY_HOST_CHECK_INTERVAL,
                        &_redirector<&cmd_change_object_int_var>)},
          {"CHANGE_MAX_HOST_CHECK_ATTEMPTS",
           command_info(CMD_CHANGE_MAX_HOST_CHECK_ATTEMPTS,
                        &_redirector<&cmd_change_object_int_var>)},
          {"SCHEDULE_AND_PROPAGATE_TRIGGERED_HOST_DOWNTIME",
           command_info(CMD_SCHEDULE_AND_PROPAGATE_TRIGGERED_HOST_DOWNTIME,
                        &_redirector<&cmd_schedule_downtime>)},
          {"SCHEDULE_AND_PROPAGATE_HOST_DOWNTIME",
           command_info(CMD_SCHEDULE_AND_PROPAGATE_HOST_DOWNTIME,
                        &_redirector<&cmd_schedule_downtime>)},
          {"SET_HOST_NOTIFICATION_NUMBER",
           command_info(
               CMD_SET_HOST_NOTIFICATION_NUMBER,
               &_redirector_host<&_wrapper_set_host_notification_number>)},
          {"CHANGE_HOST_CHECK_TIMEPERIOD",
           command_info(CMD_CHANGE_HOST_CHECK_TIMEPERIOD,
                        &_redirector<&cmd_change_object_char_var>)},
          {"CHANGE_CUSTOM_HOST_VAR",
           command_info(CMD_CHANGE_CUSTOM_HOST_VAR,
                        &_redirector<&cmd_change_object_custom_var>)},
          {"SEND_CUSTOM_HOST_NOTIFICATION",
           command_info(
               CMD_SEND_CUSTOM_HOST_NOTIFICATION,
               &_redirector_host<&_wrapper_send_custom_host_notification>)},
          {"CHANGE_HOST_NOTIFICATION_TIMEPERIOD",
           command_info(CMD_CHANGE_HOST_NOTIFICATION_TIMEPERIOD,
                        &_redirector<&cmd_change_object_char_var>)},
          {"CHANGE_HOST_MODATTR",
           command_info(CMD_CHANGE_HOST_MODATTR,
                        &_redirector<&cmd_change_object_int_var>)},
          // hostgroup-related commands.
          {"ENABLE_HOSTGROUP_HOST_NOTIFICATIONS",
           command_info(CMD_ENABLE_HOSTGROUP_HOST_NOTIFICATIONS,
                        &_redirector_hostgroup<&enable_host_notifications>)},
          {"DISABLE_HOSTGROUP_HOST_NOTIFICATIONS",
           command_info(CMD_DISABLE_HOSTGROUP_HOST_NOTIFICATIONS,
                        &_redirector_hostgroup<&disable_host_notifications>)},
          {"ENABLE_HOSTGROUP_SVC_NOTIFICATIONS",
           command_info(
               CMD_ENABLE_HOSTGROUP_SVC_NOTIFICATIONS,
               &_redirector_hostgroup<&_wrapper_enable_service_notifications>)},
          {"DISABLE_HOSTGROUP_SVC_NOTIFICATIONS",
           command_info(CMD_DISABLE_HOSTGROUP_SVC_NOTIFICATIONS,
                        &_redirector_hostgroup<
                             &_wrapper_disable_service_notifications>)},
          {"ENABLE_HOSTGROUP_HOST_CHECKS",
           command_info(CMD_ENABLE_HOSTGROUP_HOST_CHECKS,
                        &_redirector_hostgroup<&enable_host_checks>)},
          {"DISABLE_HOSTGROUP_HOST_CHECKS",
           command_info(CMD_DISABLE_HOSTGROUP_HOST_CHECKS,
                        &_redirector_hostgroup<&disable_host_checks>)},
          {"ENABLE_HOSTGROUP_PASSIVE_HOST_CHECKS",
           command_info(CMD_ENABLE_HOSTGROUP_PASSIVE_HOST_CHECKS,
                        &_redirector_hostgroup<&enable_passive_host_checks>)},
          {"DISABLE_HOSTGROUP_PASSIVE_HOST_CHECKS",
           command_info(CMD_DISABLE_HOSTGROUP_PASSIVE_HOST_CHECKS,
                        &_redirector_hostgroup<&disable_passive_host_checks>)},
          {"ENABLE_HOSTGROUP_SVC_CHECKS",
           command_info(
               CMD_ENABLE_HOSTGROUP_SVC_CHECKS,
               &_redirector_hostgroup<&_wrapper_enable_service_checks>)},
          {"DISABLE_HOSTGROUP_SVC_CHECKS",
           command_info(
               CMD_DISABLE_HOSTGROUP_SVC_CHECKS,
               &_redirector_hostgroup<&_wrapper_disable_service_checks>)},
          {"ENABLE_HOSTGROUP_PASSIVE_SVC_CHECKS",
           command_info(CMD_ENABLE_HOSTGROUP_PASSIVE_SVC_CHECKS,
                        &_redirector_hostgroup<
                             &_wrapper_enable_passive_service_checks>)},
          {"DISABLE_HOSTGROUP_PASSIVE_SVC_CHECKS",
           command_info(CMD_DISABLE_HOSTGROUP_PASSIVE_SVC_CHECKS,
                        &_redirector_hostgroup<
                             &_wrapper_disable_passive_service_checks>)},
          {"SCHEDULE_HOSTGROUP_HOST_DOWNTIME",
           command_info(CMD_SCHEDULE_HOSTGROUP_HOST_DOWNTIME,
                        &_redirector<&cmd_schedule_downtime>)},
          {"SCHEDULE_HOSTGROUP_SVC_DOWNTIME",
           command_info(CMD_SCHEDULE_HOSTGROUP_SVC_DOWNTIME,
                        &_redirector<&cmd_schedule_downtime>)},
          // service-related commands.
          {"ADD_SVC_COMMENT",
           command_info(CMD_ADD_SVC_COMMENT, &_redirector<&cmd_add_comment>)},
          {"DEL_SVC_COMMENT", command_info(CMD_DEL_SVC_COMMENT,
                                           &_redirector<&cmd_delete_comment>)},
          {"DEL_ALL_SVC_COMMENTS",
           command_info(CMD_DEL_ALL_SVC_COMMENTS,
                        &_redirector<&cmd_delete_all_comments>)},
          {"SCHEDULE_SVC_CHECK",
           command_info(CMD_SCHEDULE_SVC_CHECK,
                        &_redirector<&cmd_schedule_check>)},
          {"SCHEDULE_FORCED_SVC_CHECK",
           command_info(CMD_SCHEDULE_FORCED_SVC_CHECK,
                        &_redirector<&cmd_schedule_check>)},
          {"ENABLE_SVC_CHECK",
           command_info(CMD_ENABLE_SVC_CHECK,
                        &_redirector_service<&enable_service_checks>)},
          {"DISABLE_SVC_CHECK",
           command_info(CMD_DISABLE_SVC_CHECK,
                        &_redirector_service<&disable_service_checks>)},
          {"ENABLE_PASSIVE_SVC_CHECKS",
           command_info(CMD_ENABLE_PASSIVE_SVC_CHECKS,
                        &_redirector_service<&enable_passive_service_checks>)},
          {"DISABLE_PASSIVE_SVC_CHECKS",
           command_info(CMD_DISABLE_PASSIVE_SVC_CHECKS,
                        &_redirector_service<&disable_passive_service_checks>)},
          {"DELAY_SVC_NOTIFICATION",
           command_info(CMD_DELAY_SVC_NOTIFICATION,
                        &_redirector<&cmd_delay_notification>)},
          {"ENABLE_SVC_NOTIFICATIONS",
           command_info(CMD_ENABLE_SVC_NOTIFICATIONS,
                        &_redirector_service<&enable_service_notifications>)},
          {"DISABLE_SVC_NOTIFICATIONS",
           command_info(CMD_DISABLE_SVC_NOTIFICATIONS,
                        &_redirector_service<&disable_service_notifications>)},
          {"PROCESS_SERVICE_CHECK_RESULT",
           command_info(CMD_PROCESS_SERVICE_CHECK_RESULT,
                        &_redirector<&cmd_process_service_check_result>,
                        true)},
          {"PROCESS_HOST_CHECK_RESULT",
           command_info(CMD_PROCESS_HOST_CHECK_RESULT,
                        &_redirector<&cmd_process_host_check_result>,
                        true)},
          {"ENABLE_SVC_EVENT_HANDLER",
           command_info(CMD_ENABLE_SVC_EVENT_HANDLER,
                        &_redirector_service<&enable_service_event_handler>)},
          {"DISABLE_SVC_EVENT_HANDLER",
           command_info(CMD_DISABLE_SVC_EVENT_HANDLER,
                        &_redirector_service<&disable_service_event_handler>)},
          {"ENABLE_SVC_FLAP_DETECTION",
           command_info(CMD_ENABLE_SVC_FLAP_DETECTION,
                        &_redirector_service<&enable_service_flap_detection>)},
          {"DISABLE_SVC_FLAP_DETECTION",
           command_info(CMD_DISABLE_SVC_FLAP_DETECTION,
                        &_redirector_service<&disable_service_flap_detection>)},
          {"SCHEDULE_SVC_DOWNTIME",
           command_info(CMD_SCHEDULE_SVC_DOWNTIME,
                        &_redirector<&cmd_schedule_downtime>)},
          {"DEL_SVC_DOWNTIME",
           command_info(CMD_DEL_SVC_DOWNTIME,
                        &_redirector<&cmd_delete_downtime>)},
          {"DEL_SVC_DOWNTIME_FULL",
           command_info(CMD_DEL_SVC_DOWNTIME_FULL,
                        &_redirector<&cmd_delete_downtime_full>)},
          {"ACKNOWLEDGE_SVC_PROBLEM",
           command_info(CMD_ACKNOWLEDGE_SVC_PROBLEM,
                        &_redirector<&cmd_acknowledge_problem>)},
          {"REMOVE_SVC_ACKNOWLEDGEMENT",
           command_info(CMD_REMOVE_SVC_ACKNOWLEDGEMENT,
                        &_redirector<&cmd_remove_acknowledgement>)},
          {"START_OBSESSING_OVER_SVC",
           command_info(CMD_START_OBSESSING_OVER_SVC,
                        &_redirector_service<&start_obsessing_over_service>)},
          {"STOP_OBSESSING_OVER_SVC",
           command_info(CMD_STOP_OBSESSING_OVER_SVC,
                        &_redirector_service<&stop_obsessing_over_service>)},
          {"CHANGE_SVC_EVENT_HANDLER",
           command_info(CMD_CHANGE_SVC_EVENT_HANDLER,
                        &_redirector<&cmd_change_object_char_var>)},
          {"CHANGE_SVC_CHECK_COMMAND",
           command_info(CMD_CHANGE_SVC_CHECK_COMMAND,
                        &_redirector<&cmd_change_object_char_var>)},
          {"CHANGE_NORMAL_SVC_CHECK_INTERVAL",
           command_info(CMD_CHANGE_NORMAL_SVC_CHECK_INTERVAL,
                        &_redirector<&cmd_change_object_int_var>)},
          {"CHANGE_RETRY_SVC_CHECK_INTERVAL",
           command_info(CMD_CHANGE_RETRY_SVC_CHECK_INTERVAL,
                        &_redirector<&cmd_change_object_int_var>)},
          {"CHANGE_MAX_SVC_CHECK_ATTEMPTS",
           command_info(CMD_CHANGE_MAX_SVC_CHECK_ATTEMPTS,
                        &_redirector<&cmd_change_object_int_var>)},
          {"SET_SVC_NOTIFICATION_NUMBER",
           command_info(CMD_SET_SVC_NOTIFICATION_NUMBER,
                        &_redirector_service<
                             &_wrapper_set_service_notification_number>)},
          {"CHANGE_SVC_CHECK_TIMEPERIOD",
           command_info(CMD_CHANGE_SVC_CHECK_TIMEPERIOD,
                        &_redirector<&cmd_change_object_char_var>)},
          {"CHANGE_CUSTOM_SVC_VAR",
           command_info(CMD_CHANGE_CUSTOM_SVC_VAR,
                        &_redirector<&cmd_change_object_custom_var>)},
          {"CHANGE_CUSTOM_CONTACT_VAR",
           command_info(CMD_CHANGE_CUSTOM_CONTACT_VAR,
                        &_redirector<&cmd_change_object_custom_var>)},
          {"SEND_CUSTOM_SVC_NOTIFICATION",
           command_info(CMD_SEND_CUSTOM_SVC_NOTIFICATION,
                        &_redirector_service<
                             &_wrapper_send_custom_service_notification>)},
          {"CHANGE_SVC_NOTIFICATION_TIMEPERIOD",
           command_info(CMD_CHANGE_SVC_NOTIFICATION_TIMEPERIOD,
                        &_redirector<&cmd_change_object_char_var>)},
          {"CHANGE_SVC_MODATTR",
           command_info(CMD_CHANGE_SVC_MODATTR,
                        &_redirector<&cmd_change_object_int_var>)},
          // servicegroup-related commands.
          {"ENABLE_SERVICEGROUP_HOST_NOTIFICATIONS",
           command_info(CMD_ENABLE_SERVICEGROUP_HOST_NOTIFICATIONS,
                        &_redirector_servicegroup<&enable_host_notifications>)},
          {"DISABLE_SERVICEGROUP_HOST_NOTIFICATIONS",
           command_info(
               CMD_DISABLE_SERVICEGROUP_HOST_NOTIFICATIONS,
               &_redirector_servicegroup<&disable_host_notifications>)},
          {"ENABLE_SERVICEGROUP_SVC_NOTIFICATIONS",
           command_info(CMD_ENABLE_SERVICEGROUP_SVC_NOTIFICATIONS,
                        &_redirector_service<&enable_service_notifications>)},
          {"DISABLE_SERVICEGROUP_SVC_NOTIFICATIONS",
           command_info(CMD_DISABLE_SERVICEGROUP_SVC_NOTIFICATIONS,
                        &_redirector_service<&disable_service_notifications>)},
          {"ENABLE_SERVICEGROUP_HOST_CHECKS",
           command_info(CMD_ENABLE_SERVICEGROUP_HOST_CHECKS,
                        &_redirector_servicegroup<&enable_host_checks>)},
          {"DISABLE_SERVICEGROUP_HOST_CHECKS",
           command_info(CMD_DISABLE_SERVICEGROUP_HOST_CHECKS,
                        &_redirector_servicegroup<&disable_host_checks>)},
          {"ENABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS",
           command_info(
               CMD_ENABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS,
               &_redirector_servicegroup<&enable_passive_host_checks>)},
          {"DISABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS",
           command_info(
               CMD_DISABLE_SERVICEGROUP_PASSIVE_HOST_CHECKS,
               &_redirector_servicegroup<&disable_passive_host_checks>)},
          {"ENABLE_SERVICEGROUP_SVC_CHECKS",
           command_info(CMD_ENABLE_SERVICEGROUP_SVC_CHECKS,
                        &_redirector_service<&enable_service_checks>)},
          {"DISABLE_SERVICEGROUP_SVC_CHECKS",
           command_info(CMD_DISABLE_SERVICEGROUP_SVC_CHECKS,
                        &_redirector_service<&disable_service_checks>)},
          {"ENABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS",
           command_info(CMD_ENABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS,
                        &_redirector_service<&enable_passive_service_checks>)},
          {"DISABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS",
           command_info(CMD_DISABLE_SERVICEGROUP_PASSIVE_SVC_CHECKS,
                        &_redirector_service<&disable_passive_service_checks>)},
          {"SCHEDULE_SERVICEGROUP_HOST_DOWNTIME",
           command_info(CMD_SCHEDULE_SERVICEGROUP_HOST_DOWNTIME,
                        &_redirector<&cmd_schedule_downtime>)},
          {"SCHEDULE_SERVICEGROUP_SVC_DOWNTIME",
           command_info(CMD_SCHEDULE_SERVICEGROUP_SVC_DOWNTIME,
                        &_redirector<&cmd_schedule_downtime>)},
          // contact-related commands.
          {"ENABLE_CONTACT_HOST_NOTIFICATIONS",
           command_info(
               CMD_ENABLE_CONTACT_HOST_NOTIFICATIONS,
               &_redirector_contact<&enable_contact_host_notifications>)},
          {"DISABLE_CONTACT_HOST_NOTIFICATIONS",
           command_info(
               CMD_DISABLE_CONTACT_HOST_NOTIFICATIONS,
               &_redirector_contact<&disable_contact_host_notifications>)},
          {"ENABLE_CONTACT_SVC_NOTIFICATIONS",
           command_info(
               CMD_ENABLE_CONTACT_SVC_NOTIFICATIONS,
               &_redirector_contact<&enable_contact_service_notifications>)},
          {"DISABLE_CONTACT_SVC_NOTIFICATIONS",
           command_info(
               CMD_DISABLE_CONTACT_SVC_NOTIFICATIONS,
               &_redirector_contact<&disable_contact_service_notifications>)},
          {"CHANGE_CONTACT_HOST_NOTIFICATION_TIMEPERIOD",
           command_info(CMD_CHANGE_CONTACT_HOST_NOTIFICATION_TIMEPERIOD,
                        &_redirector<&cmd_change_object_char_var>)},
          {"CHANGE_CONTACT_SVC_NOTIFICATION_TIMEPERIOD",
           command_info(CMD_CHANGE_CONTACT_SVC_NOTIFICATION_TIMEPERIOD,
                        &_redirector<&cmd_change_object_char_var>)},
          {"CHANGE_CONTACT_MODATTR",
           command_info(CMD_CHANGE_CONTACT_MODATTR,
                        &_redirector<&cmd_change_object_int_var>)},
          {"CHANGE_CONTACT_MODHATTR",
           command_info(CMD_CHANGE_CONTACT_MODHATTR,
                        &_redirector<&cmd_change_object_int_var>)},
          {"CHANGE_CONTACT_MODSATTR",
           command_info(CMD_CHANGE_CONTACT_MODSATTR,
                        &_redirector<&cmd_change_object_int_var>)},
          // contactgroup-related commands.
          {"ENABLE_CONTACTGROUP_HOST_NOTIFICATIONS",
           command_info(
               CMD_ENABLE_CONTACTGROUP_HOST_NOTIFICATIONS,
               &_redirector_contactgroup<&enable_contact_host_notifications>)},
          {"DISABLE_CONTACTGROUP_HOST_NOTIFICATIONS",
           command_info(
               CMD_DISABLE_CONTACTGROUP_HOST_NOTIFICATIONS,
               &_redirector_contactgroup<&disable_contact_host_notifications>)},
          {"ENABLE_CONTACTGROUP_SVC_NOTIFICATIONS",
           command_info(CMD_ENABLE_CONTACTGROUP_SVC_NOTIFICATIONS,
                        &_redirector_contactgroup<
                             &enable_contact_service_notifications>)},
          {"DISABLE_CONTACTGROUP_SVC_NOTIFICATIONS",
           command_info(CMD_DISABLE_CONTACTGROUP_SVC_NOTIFICATIONS,
                        &_redirector_contactgroup<
                             &disable_contact_service_notifications>)}, } {
  // misc commands.
  _lst_command["PROCESS_FILE"] = command_info(
      CMD_PROCESS_FILE, &_redirector<&cmd_process_external_commands_from_file>);
}

processing::~processing() throw() {}

bool processing::execute(std::string const& cmdstr) const {
  logger(dbg_functions, basic) << "processing external command";

  char const* cmd{cmdstr.c_str()};
  size_t len{cmdstr.size()};

  // Left trim command
  while (*cmd && isspace(*cmd))
    ++cmd;
  if (*cmd != '[')
    return false;

  // Right trim just by recomputing the optimal length value.
  char const* end{cmd + len - 1};
  while (end != cmd && isspace(*end))
    --end;

  cmd++;
  char* tmp;
  time_t entry_time{static_cast<time_t>(strtoul(cmd, &tmp, 10))};

  while (*tmp && isspace(*tmp))
    ++tmp;
  if (*tmp != ']' || tmp[1] != ' ')
    return false;

  cmd = tmp + 2;
  char const* a;
  for (a = cmd; *a && *a != ';'; ++a)
    ;

  std::string command_name(cmd, a - cmd);
  std::string args;
  if (*a == ';') {
    a++;
    args = std::string(a, end - a + 1);
  }

  int command_id(CMD_CUSTOM_COMMAND);

  std::unordered_map<std::string, command_info>::const_iterator it;
  {
    std::unique_lock<std::mutex> lock(_mutex);
    it = _lst_command.find(command_name);
    if (it != _lst_command.end())
      command_id = it->second.id;
    else if (command_name[0] != '_') {
      lock.unlock();
      logger(log_external_command | log_runtime_warning, basic)
          << "Warning: Unrecognized external command -> " << command_name;
      return false;
    }

    // Update statistics for external commands.
    update_check_stats(EXTERNAL_COMMAND_STATS, std::time(nullptr));
  }

  // Log the external command.
  if (command_id == CMD_PROCESS_SERVICE_CHECK_RESULT ||
      command_id == CMD_PROCESS_HOST_CHECK_RESULT) {
    // Passive checks are logged in checks.c.
    if (config->log_passive_checks())
      logger(log_passive_check, basic) << "EXTERNAL COMMAND: " << command_name
                                       << ';' << args;
  } else if (config->log_external_commands())
    logger(log_external_command, basic) << "EXTERNAL COMMAND: " << command_name
                                        << ';' << args;

  logger(dbg_external_command, more) << "External command id: " << command_id
                                     << "\nCommand entry time: " << entry_time
                                     << "\nCommand arguments: " << args;

  // Send data to event broker.
  broker_external_command(NEBTYPE_EXTERNALCOMMAND_START,
                          NEBFLAG_NONE,
                          NEBATTR_NONE,
                          command_id,
                          entry_time,
                          const_cast<char*>(command_name.c_str()),
                          const_cast<char*>(args.c_str()),
                          nullptr);

  {
    std::lock_guard<std::mutex> lock(_mutex);
    if (it != _lst_command.end())
      (*it->second.func)(
          command_id, entry_time, const_cast<char*>(args.c_str()));
  }

  // Send data to event broker.
  broker_external_command(NEBTYPE_EXTERNALCOMMAND_END,
                          NEBFLAG_NONE,
                          NEBATTR_NONE,
                          command_id,
                          entry_time,
                          const_cast<char*>(command_name.c_str()),
                          const_cast<char*>(args.c_str()),
                          nullptr);
  return true;
}

/**
 *  Check if a command is thread-safe.
 *
 *  @param[in] cmd  Command to check.
 *
 *  @return True if command is thread-safe.
 */
bool processing::is_thread_safe(char const* cmd) const {
  char const* ptr = cmd + strspn(cmd, "[]0123456789 ");
  std::string short_cmd(ptr, strcspn(ptr, ";"));
  std::lock_guard<std::mutex> lock(_mutex);
  std::unordered_map<std::string, command_info>::const_iterator it =
      _lst_command.find(short_cmd);
  return it != _lst_command.end() && it->second.thread_safe;
}

void processing::_wrapper_read_state_information() {
  try {
    retention::state state;
    retention::parser p;
    p.parse(config->state_retention_file(), state);
    retention::applier::state app_state;
    app_state.apply(*config, state);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << "Error: could not load retention file: "
                                     << e.what();
  }
  return;
}

void processing::_wrapper_save_state_information() {
  retention::dump::save(config->state_retention_file());
}

void processing::_wrapper_enable_host_and_child_notifications(host* hst) {
  enable_and_propagate_notifications(hst, 0, true, true, false);
}

void processing::_wrapper_disable_host_and_child_notifications(host* hst) {
  disable_and_propagate_notifications(hst, 0, true, true, false);
}

void processing::_wrapper_enable_all_notifications_beyond_host(host* hst) {
  enable_and_propagate_notifications(hst, 0, false, true, true);
}

void processing::_wrapper_disable_all_notifications_beyond_host(host* hst) {
  disable_and_propagate_notifications(hst, 0, false, true, true);
}

void processing::_wrapper_enable_host_svc_notifications(host* hst) {
  for (service_map_unsafe::iterator it(hst->services.begin()),
       end(hst->services.end());
       it != end;
       ++it)
    if (it->second)
      enable_service_notifications(it->second);
}

void processing::_wrapper_disable_host_svc_notifications(host* hst) {
  for (service_map_unsafe::iterator it(hst->services.begin()),
       end(hst->services.end());
       it != end;
       ++it)
    if (it->second)
      disable_service_notifications(it->second);
}

void processing::_wrapper_disable_host_svc_checks(host* hst) {
  for (service_map_unsafe::iterator it(hst->services.begin()),
       end(hst->services.end());
       it != end;
       ++it)
    if (it->second)
      disable_service_checks(it->second);
}

void processing::_wrapper_enable_host_svc_checks(host* hst) {
  for (service_map_unsafe::iterator it(hst->services.begin()),
       end(hst->services.end());
       it != end;
       ++it)
    if (it->second)
      enable_service_checks(it->second);
}

void processing::_wrapper_set_host_notification_number(host* hst, char* args) {
  if (args)
    set_host_notification_number(hst, atoi(args));
}

void processing::_wrapper_send_custom_host_notification(host* hst, char* args) {
  char* buf[3] = {NULL, NULL, NULL};
  if ((buf[0] = my_strtok(args, ";")) && (buf[1] = my_strtok(NULL, ";")) &&
      (buf[2] = my_strtok(NULL, ";"))) {
    hst->notify(notifier::reason_custom,
                buf[1],
                buf[2],
                static_cast<notifier::notification_option>(atoi(buf[0])));
  }
}

void processing::_wrapper_enable_service_notifications(host* hst) {
  for (service_map_unsafe::iterator it(hst->services.begin()),
       end(hst->services.end());
       it != end;
       ++it)
    if (it->second)
      enable_service_notifications(it->second);
}

void processing::_wrapper_disable_service_notifications(host* hst) {
  for (service_map_unsafe::iterator it(hst->services.begin()),
       end(hst->services.end());
       it != end;
       ++it)
    if (it->second)
      disable_service_notifications(it->second);
}

void processing::_wrapper_enable_service_checks(host* hst) {
  for (service_map_unsafe::iterator it(hst->services.begin()),
       end(hst->services.end());
       it != end;
       ++it)
    if (it->second)
      enable_service_checks(it->second);
}

void processing::_wrapper_disable_service_checks(host* hst) {
  for (service_map_unsafe::iterator it(hst->services.begin()),
       end(hst->services.end());
       it != end;
       ++it)
    if (it->second)
      disable_service_checks(it->second);
}

void processing::_wrapper_enable_passive_service_checks(host* hst) {
  for (service_map_unsafe::iterator it(hst->services.begin()),
       end(hst->services.end());
       it != end;
       ++it)
    if (it->second)
      enable_passive_service_checks(it->second);
}

void processing::_wrapper_disable_passive_service_checks(host* hst) {
  for (service_map_unsafe::iterator it(hst->services.begin()),
       end(hst->services.end());
       it != end;
       ++it)
    if (it->second)
      disable_passive_service_checks(it->second);
}

void processing::_wrapper_set_service_notification_number(service* svc,
                                                          char* args) {
  char* str(my_strtok(args, ";"));
  if (str)
    set_service_notification_number(svc, atoi(str));
}

void processing::_wrapper_send_custom_service_notification(service* svc,
                                                           char* args) {
  char* buf[3] = {NULL, NULL, NULL};
  if ((buf[0] = my_strtok(args, ";")) && (buf[1] = my_strtok(NULL, ";")) &&
      (buf[2] = my_strtok(NULL, ";"))) {
    svc->notify(notifier::reason_custom,
                buf[1],
                buf[2],
                static_cast<notifier::notification_option>(atoi(buf[0])));
  }
}

##
## Copyright 2011-2013,2015 Merethis
##
## This file is part of Centreon Engine.
##
## Centreon Engine is free software: you can redistribute it and/or
## modify it under the terms of the GNU General Public License version 2
## as published by the Free Software Foundation.
##
## Centreon Engine is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Centreon Engine. If not, see
## <http://www.gnu.org/licenses/>.
##

# Set directories.
set(MODULE_DIR "${PROJECT_SOURCE_DIR}/modules/external_commands")
set(TEST_DIR "${TEST_DIR}/modules/external_commands")

# Include directories.
include_directories("${MODULE_DIR}/inc")

# Test calling enter_standby_mode.
set(TEST_NAME "modules_external_commands_enter_standby_mode")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enter_standby_mode.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_notifications.
set(TEST_NAME "modules_external_commands_disable_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enter_active_mode.
set(TEST_NAME "modules_external_commands_enter_active_mode")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enter_active_mode.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_notifications.
set(TEST_NAME "modules_external_commands_enable_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling shutdown_program.
set(TEST_NAME "modules_external_commands_shutdown_program")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/shutdown_program.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling shutdown_process.
set(TEST_NAME "modules_external_commands_shutdown_process")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/shutdown_process.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling restart_program.
set(TEST_NAME "modules_external_commands_restart_program")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/restart_program.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling restart_process.
set(TEST_NAME "modules_external_commands_restart_process")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/restart_process.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling save_state_information.
set(TEST_NAME "modules_external_commands_save_state_information")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/save_state_information.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling read_state_information.
set(TEST_NAME "modules_external_commands_read_state_information")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/read_state_information.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_event_handlers.
set(TEST_NAME "modules_external_commands_enable_event_handlers")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_event_handlers.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_event_handlers.
set(TEST_NAME "modules_external_commands_disable_event_handlers")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_event_handlers.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling flush_pending_commands.
set(TEST_NAME "modules_external_commands_flush_pending_commands")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/flush_pending_commands.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_performance_data.
set(TEST_NAME "modules_external_commands_enable_performance_data")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_performance_data.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_performance_data.
set(TEST_NAME "modules_external_commands_disable_performance_data")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_performance_data.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling start_executing_host_checks.
set(TEST_NAME "modules_external_commands_start_executing_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/start_executing_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling stop_executing_host_checks.
set(TEST_NAME "modules_external_commands_stop_executing_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/stop_executing_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling start_executing_svc_checks.
set(TEST_NAME "modules_external_commands_start_executing_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/start_executing_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling stop_executing_svc_checks.
set(TEST_NAME "modules_external_commands_stop_executing_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/stop_executing_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling start_accepting_passive_host_checks.
set(TEST_NAME "modules_external_commands_start_accepting_passive_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/start_accepting_passive_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling stop_accepting_passive_host_checks.
set(TEST_NAME "modules_external_commands_stop_accepting_passive_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/stop_accepting_passive_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling start_accepting_passive_svc_checks.
set(TEST_NAME "modules_external_commands_start_accepting_passive_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/start_accepting_passive_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling stop_accepting_passive_svc_checks.
set(TEST_NAME "modules_external_commands_stop_accepting_passive_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/stop_accepting_passive_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling start_obsessing_over_host_checks.
set(TEST_NAME "modules_external_commands_start_obsessing_over_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/start_obsessing_over_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling stop_obsessing_over_host_checks.
set(TEST_NAME "modules_external_commands_stop_obsessing_over_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/stop_obsessing_over_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling start_obsessing_over_svc_checks.
set(TEST_NAME "modules_external_commands_start_obsessing_over_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/start_obsessing_over_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling stop_obsessing_over_svc_checks.
set(TEST_NAME "modules_external_commands_stop_obsessing_over_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/stop_obsessing_over_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_flap_detection.
set(TEST_NAME "modules_external_commands_enable_flap_detection")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_flap_detection.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_flap_detection.
set(TEST_NAME "modules_external_commands_disable_flap_detection")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_flap_detection.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_global_host_event_handler.
set(TEST_NAME "modules_external_commands_change_global_host_event_handler")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_global_host_event_handler.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_global_svc_event_handler.
set(TEST_NAME "modules_external_commands_change_global_svc_event_handler")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_global_svc_event_handler.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_service_freshness_checks.
set(TEST_NAME "modules_external_commands_enable_service_freshness_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_service_freshness_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_service_freshness_checks.
set(TEST_NAME "modules_external_commands_disable_service_freshness_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_service_freshness_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_host_freshness_checks.
set(TEST_NAME "modules_external_commands_enable_host_freshness_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_host_freshness_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_host_freshness_checks.
set(TEST_NAME "modules_external_commands_disable_host_freshness_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_host_freshness_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling add_host_comment.
set(TEST_NAME "modules_external_commands_add_host_comment")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/add_host_comment.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling del_host_comment.
set(TEST_NAME "modules_external_commands_del_host_comment")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/del_host_comment.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling del_all_host_comments.
set(TEST_NAME "modules_external_commands_del_all_host_comments")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/del_all_host_comments.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling delay_host_notification.
set(TEST_NAME "modules_external_commands_delay_host_notification")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/delay_host_notification.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_host_notifications.
set(TEST_NAME "modules_external_commands_enable_host_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_host_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_host_notifications.
set(TEST_NAME "modules_external_commands_disable_host_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_host_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_all_notifications_beyond_host.
set(TEST_NAME "modules_external_commands_enable_all_notifications_beyond_host")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_all_notifications_beyond_host.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_all_notifications_beyond_host.
set(TEST_NAME "modules_external_commands_disable_all_notifications_beyond_host")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_all_notifications_beyond_host.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_host_and_child_notifications.
set(TEST_NAME "modules_external_commands_enable_host_and_child_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_host_and_child_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_host_and_child_notifications.
set(TEST_NAME "modules_external_commands_disable_host_and_child_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_host_and_child_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_host_svc_notifications.
set(TEST_NAME "modules_external_commands_enable_host_svc_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_host_svc_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_host_svc_notifications.
set(TEST_NAME "modules_external_commands_disable_host_svc_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_host_svc_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_host_svc_checks.
set(TEST_NAME "modules_external_commands_enable_host_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_host_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_host_svc_checks.
set(TEST_NAME "modules_external_commands_disable_host_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_host_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_passive_host_checks.
set(TEST_NAME "modules_external_commands_enable_passive_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_passive_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_passive_host_checks.
set(TEST_NAME "modules_external_commands_disable_passive_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_passive_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_host_svc_checks.
set(TEST_NAME "modules_external_commands_schedule_host_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_host_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_forced_host_svc_checks.
set(TEST_NAME "modules_external_commands_schedule_forced_host_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_forced_host_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling acknowledge_host_problem.
set(TEST_NAME "modules_external_commands_acknowledge_host_problem")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/acknowledge_host_problem.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling remove_host_acknowledgement.
set(TEST_NAME "modules_external_commands_remove_host_acknowledgement")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/remove_host_acknowledgement.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_host_event_handler.
set(TEST_NAME "modules_external_commands_enable_host_event_handler")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_host_event_handler.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_host_event_handler.
set(TEST_NAME "modules_external_commands_disable_host_event_handler")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_host_event_handler.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_host_check.
set(TEST_NAME "modules_external_commands_enable_host_check")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_host_check.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_host_check.
set(TEST_NAME "modules_external_commands_disable_host_check")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_host_check.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_host_check.
set(TEST_NAME "modules_external_commands_schedule_host_check")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_host_check.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_forced_host_check.
set(TEST_NAME "modules_external_commands_schedule_forced_host_check")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_forced_host_check.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_host_downtime.
set(TEST_NAME "modules_external_commands_schedule_host_downtime")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_host_downtime.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_host_svc_downtime.
set(TEST_NAME "modules_external_commands_schedule_host_svc_downtime")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_host_svc_downtime.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling del_host_downtime.
set(TEST_NAME "modules_external_commands_del_host_downtime")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/del_host_downtime.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling del_downtime_by_host_name.
set(TEST_NAME "modules_external_commands_del_downtime_by_host_name")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/del_downtime_by_host_name.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling del_downtime_by_hostgroup_name.
set(TEST_NAME "modules_external_commands_del_downtime_by_hostgroup_name")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/del_downtime_by_hostgroup_name.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling del_downtime_by_start_time_comment.
set(TEST_NAME "modules_external_commands_del_downtime_by_start_time_comment")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/del_downtime_by_start_time_comment.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_host_flap_detection.
set(TEST_NAME "modules_external_commands_enable_host_flap_detection")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_host_flap_detection.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_host_flap_detection.
set(TEST_NAME "modules_external_commands_disable_host_flap_detection")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_host_flap_detection.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling start_obsessing_over_host.
set(TEST_NAME "modules_external_commands_start_obsessing_over_host")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/start_obsessing_over_host.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling stop_obsessing_over_host.
set(TEST_NAME "modules_external_commands_stop_obsessing_over_host")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/stop_obsessing_over_host.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_host_event_handler.
set(TEST_NAME "modules_external_commands_change_host_event_handler")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_host_event_handler.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_host_check_command.
set(TEST_NAME "modules_external_commands_change_host_check_command")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_host_check_command.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_normal_host_check_interval.
set(TEST_NAME "modules_external_commands_change_normal_host_check_interval")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_normal_host_check_interval.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_retry_host_check_interval.
set(TEST_NAME "modules_external_commands_change_retry_host_check_interval")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_retry_host_check_interval.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_max_host_check_attempts.
set(TEST_NAME "modules_external_commands_change_max_host_check_attempts")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_max_host_check_attempts.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_and_propagate_triggered_host_downtime.
set(TEST_NAME "modules_external_commands_schedule_and_propagate_triggered_host_downtime")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_and_propagate_triggered_host_downtime.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_and_propagate_host_downtime.
set(TEST_NAME "modules_external_commands_schedule_and_propagate_host_downtime")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_and_propagate_host_downtime.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling set_host_notification_number.
set(TEST_NAME "modules_external_commands_set_host_notification_number")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/set_host_notification_number.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_host_check_timeperiod.
set(TEST_NAME "modules_external_commands_change_host_check_timeperiod")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_host_check_timeperiod.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_custom_host_var.
set(TEST_NAME "modules_external_commands_change_custom_host_var")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_custom_host_var.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling send_custom_host_notification.
set(TEST_NAME "modules_external_commands_send_custom_host_notification")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/send_custom_host_notification.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_host_notification_timeperiod.
set(TEST_NAME "modules_external_commands_change_host_notification_timeperiod")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_host_notification_timeperiod.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_host_modattr.
set(TEST_NAME "modules_external_commands_change_host_modattr")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_host_modattr.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_hostgroup_host_notifications.
set(TEST_NAME "modules_external_commands_enable_hostgroup_host_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_hostgroup_host_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_hostgroup_host_notifications.
set(TEST_NAME "modules_external_commands_disable_hostgroup_host_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_hostgroup_host_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_hostgroup_svc_notifications.
set(TEST_NAME "modules_external_commands_enable_hostgroup_svc_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_hostgroup_svc_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_hostgroup_svc_notifications.
set(TEST_NAME "modules_external_commands_disable_hostgroup_svc_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_hostgroup_svc_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_hostgroup_host_checks.
set(TEST_NAME "modules_external_commands_enable_hostgroup_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_hostgroup_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_hostgroup_host_checks.
set(TEST_NAME "modules_external_commands_disable_hostgroup_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_hostgroup_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_hostgroup_passive_host_checks.
set(TEST_NAME "modules_external_commands_enable_hostgroup_passive_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_hostgroup_passive_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_hostgroup_passive_host_checks.
set(TEST_NAME "modules_external_commands_disable_hostgroup_passive_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_hostgroup_passive_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_hostgroup_svc_checks.
set(TEST_NAME "modules_external_commands_enable_hostgroup_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_hostgroup_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_hostgroup_svc_checks.
set(TEST_NAME "modules_external_commands_disable_hostgroup_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_hostgroup_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_hostgroup_passive_svc_checks.
set(TEST_NAME "modules_external_commands_enable_hostgroup_passive_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_hostgroup_passive_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_hostgroup_passive_svc_checks.
set(TEST_NAME "modules_external_commands_disable_hostgroup_passive_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_hostgroup_passive_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_hostgroup_host_downtime.
set(TEST_NAME "modules_external_commands_schedule_hostgroup_host_downtime")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_hostgroup_host_downtime.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_hostgroup_svc_downtime.
set(TEST_NAME "modules_external_commands_schedule_hostgroup_svc_downtime")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_hostgroup_svc_downtime.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling add_svc_comment.
set(TEST_NAME "modules_external_commands_add_svc_comment")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/add_svc_comment.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling del_svc_comment.
set(TEST_NAME "modules_external_commands_del_svc_comment")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/del_svc_comment.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling del_all_svc_comments.
set(TEST_NAME "modules_external_commands_del_all_svc_comments")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/del_all_svc_comments.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_svc_check.
set(TEST_NAME "modules_external_commands_schedule_svc_check")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_svc_check.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_forced_svc_check.
set(TEST_NAME "modules_external_commands_schedule_forced_svc_check")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_forced_svc_check.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_svc_check.
set(TEST_NAME "modules_external_commands_enable_svc_check")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_svc_check.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_svc_check.
set(TEST_NAME "modules_external_commands_disable_svc_check")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_svc_check.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_passive_svc_checks.
set(TEST_NAME "modules_external_commands_enable_passive_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_passive_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_passive_svc_checks.
set(TEST_NAME "modules_external_commands_disable_passive_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_passive_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling delay_svc_notification.
set(TEST_NAME "modules_external_commands_delay_svc_notification")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/delay_svc_notification.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_svc_notifications.
set(TEST_NAME "modules_external_commands_enable_svc_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_svc_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_svc_notifications.
set(TEST_NAME "modules_external_commands_disable_svc_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_svc_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling process_service_check_result.
set(TEST_NAME "modules_external_commands_process_service_check_result")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/process_service_check_result.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling process_host_check_result.
set(TEST_NAME "modules_external_commands_process_host_check_result")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/process_host_check_result.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_svc_event_handler.
set(TEST_NAME "modules_external_commands_enable_svc_event_handler")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_svc_event_handler.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_svc_event_handler.
set(TEST_NAME "modules_external_commands_disable_svc_event_handler")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_svc_event_handler.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_svc_flap_detection.
set(TEST_NAME "modules_external_commands_enable_svc_flap_detection")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_svc_flap_detection.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_svc_flap_detection.
set(TEST_NAME "modules_external_commands_disable_svc_flap_detection")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_svc_flap_detection.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_svc_downtime.
set(TEST_NAME "modules_external_commands_schedule_svc_downtime")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_svc_downtime.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling del_svc_downtime.
set(TEST_NAME "modules_external_commands_del_svc_downtime")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/del_svc_downtime.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling acknowledge_svc_problem.
set(TEST_NAME "modules_external_commands_acknowledge_svc_problem")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/acknowledge_svc_problem.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling remove_svc_acknowledgement.
set(TEST_NAME "modules_external_commands_remove_svc_acknowledgement")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/remove_svc_acknowledgement.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling start_obsessing_over_svc.
set(TEST_NAME "modules_external_commands_start_obsessing_over_svc")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/start_obsessing_over_svc.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling stop_obsessing_over_svc.
set(TEST_NAME "modules_external_commands_stop_obsessing_over_svc")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/stop_obsessing_over_svc.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_svc_event_handler.
set(TEST_NAME "modules_external_commands_change_svc_event_handler")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_svc_event_handler.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_svc_check_command.
set(TEST_NAME "modules_external_commands_change_svc_check_command")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_svc_check_command.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_normal_svc_check_interval.
set(TEST_NAME "modules_external_commands_change_normal_svc_check_interval")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_normal_svc_check_interval.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_retry_svc_check_interval.
set(TEST_NAME "modules_external_commands_change_retry_svc_check_interval")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_retry_svc_check_interval.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_max_svc_check_attempts.
set(TEST_NAME "modules_external_commands_change_max_svc_check_attempts")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_max_svc_check_attempts.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling set_svc_notification_number.
set(TEST_NAME "modules_external_commands_set_svc_notification_number")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/set_svc_notification_number.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_svc_check_timeperiod.
set(TEST_NAME "modules_external_commands_change_svc_check_timeperiod")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_svc_check_timeperiod.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_custom_svc_var.
set(TEST_NAME "modules_external_commands_change_custom_svc_var")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_custom_svc_var.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_custom_contact_var.
set(TEST_NAME "modules_external_commands_change_custom_contact_var")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_custom_contact_var.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling send_custom_svc_notification.
set(TEST_NAME "modules_external_commands_send_custom_svc_notification")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/send_custom_svc_notification.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_svc_notification_timeperiod.
set(TEST_NAME "modules_external_commands_change_svc_notification_timeperiod")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_svc_notification_timeperiod.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_svc_modattr.
set(TEST_NAME "modules_external_commands_change_svc_modattr")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_svc_modattr.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_servicegroup_host_notifications.
set(TEST_NAME "modules_external_commands_enable_servicegroup_host_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_servicegroup_host_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_servicegroup_host_notifications.
set(TEST_NAME "modules_external_commands_disable_servicegroup_host_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_servicegroup_host_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_servicegroup_svc_notifications.
set(TEST_NAME "modules_external_commands_enable_servicegroup_svc_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_servicegroup_svc_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_servicegroup_svc_notifications.
set(TEST_NAME "modules_external_commands_disable_servicegroup_svc_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_servicegroup_svc_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_servicegroup_host_checks.
set(TEST_NAME "modules_external_commands_enable_servicegroup_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_servicegroup_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_servicegroup_host_checks.
set(TEST_NAME "modules_external_commands_disable_servicegroup_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_servicegroup_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_servicegroup_passive_host_checks.
set(TEST_NAME "modules_external_commands_enable_servicegroup_passive_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_servicegroup_passive_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_servicegroup_passive_host_checks.
set(TEST_NAME "modules_external_commands_disable_servicegroup_passive_host_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_servicegroup_passive_host_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_servicegroup_svc_checks.
set(TEST_NAME "modules_external_commands_enable_servicegroup_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_servicegroup_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_servicegroup_svc_checks.
set(TEST_NAME "modules_external_commands_disable_servicegroup_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_servicegroup_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_servicegroup_passive_svc_checks.
set(TEST_NAME "modules_external_commands_enable_servicegroup_passive_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_servicegroup_passive_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_servicegroup_passive_svc_checks.
set(TEST_NAME "modules_external_commands_disable_servicegroup_passive_svc_checks")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_servicegroup_passive_svc_checks.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_servicegroup_host_downtime.
set(TEST_NAME "modules_external_commands_schedule_servicegroup_host_downtime")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_servicegroup_host_downtime.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling schedule_servicegroup_svc_downtime.
set(TEST_NAME "modules_external_commands_schedule_servicegroup_svc_downtime")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/schedule_servicegroup_svc_downtime.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_contact_host_notifications.
set(TEST_NAME "modules_external_commands_enable_contact_host_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_contact_host_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_contact_host_notifications.
set(TEST_NAME "modules_external_commands_disable_contact_host_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_contact_host_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_contact_svc_notifications.
set(TEST_NAME "modules_external_commands_enable_contact_svc_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_contact_svc_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_contact_svc_notifications.
set(TEST_NAME "modules_external_commands_disable_contact_svc_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_contact_svc_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_contact_host_notification_timeperiod.
set(TEST_NAME "modules_external_commands_change_contact_host_notification_timeperiod")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_contact_host_notification_timeperiod.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_contact_svc_notification_timeperiod.
set(TEST_NAME "modules_external_commands_change_contact_svc_notification_timeperiod")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_contact_svc_notification_timeperiod.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_contact_modattr.
set(TEST_NAME "modules_external_commands_change_contact_modattr")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_contact_modattr.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_contact_modhattr.
set(TEST_NAME "modules_external_commands_change_contact_modhattr")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_contact_modhattr.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling change_contact_modsattr.
set(TEST_NAME "modules_external_commands_change_contact_modsattr")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_contact_modsattr.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_contactgroup_host_notifications.
set(TEST_NAME "modules_external_commands_enable_contactgroup_host_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_contactgroup_host_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_contactgroup_host_notifications.
set(TEST_NAME "modules_external_commands_disable_contactgroup_host_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_contactgroup_host_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling enable_contactgroup_svc_notifications.
set(TEST_NAME "modules_external_commands_enable_contactgroup_svc_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/enable_contactgroup_svc_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling disable_contactgroup_svc_notifications.
set(TEST_NAME "modules_external_commands_disable_contactgroup_svc_notifications")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/disable_contactgroup_svc_notifications.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test calling process_file.
set(TEST_NAME "modules_external_commands_process_file")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/process_file.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

##
## Copyright 2011-2015 Merethis
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

# Test calling change_host_modattr.
set(TEST_NAME "modules_external_commands_change_host_modattr")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_host_modattr.cc")
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

# Test calling change_svc_modattr.
set(TEST_NAME "modules_external_commands_change_svc_modattr")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/change_svc_modattr.cc")
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

# Test calling process_file.
set(TEST_NAME "modules_external_commands_process_file")
add_executable("${TEST_NAME}"
  "${TEST_DIR}/process_file.cc")
target_link_libraries("${TEST_NAME}" "externalcmd" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

##
## Copyright 2013 Merethis
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
set(TEST_DIR "${TEST_DIR}")

#
# Error tests.
#

# Integer insertion.
set(TEST_NAME "error_insert_integer")
add_executable("${TEST_NAME}" "${TEST_DIR}/error/insert_integer.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Unsigned integer insertion.
set(TEST_NAME "error_insert_unsigned_integer")
add_executable(
  "${TEST_NAME}"
  "${TEST_DIR}/error/insert_unsigned_integer.cc"
)
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Long insertion.
set(TEST_NAME "error_insert_long")
add_executable("${TEST_NAME}" "${TEST_DIR}/error/insert_long.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Long long insertion.
set(TEST_NAME "error_insert_long_long")
add_executable("${TEST_NAME}" "${TEST_DIR}/error/insert_long_long.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Unsigned long long insertion.
set(TEST_NAME "error_insert_unsigned_long_long")
add_executable(
  "${TEST_NAME}"
  "${TEST_DIR}/error/insert_unsigned_long_long.cc"
)
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Char insertion.
set(TEST_NAME "error_insert_char")
add_executable("${TEST_NAME}" "${TEST_DIR}/error/insert_char.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Char array (C string) insertion.
set(TEST_NAME "error_insert_char_array")
add_executable("${TEST_NAME}" "${TEST_DIR}/error/insert_char_array.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# String insertion.
set(TEST_NAME "error_insert_string")
add_executable("${TEST_NAME}" "${TEST_DIR}/error/insert_string.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Mixed insertions.
set(TEST_NAME "error_insert_mixed")
add_executable("${TEST_NAME}" "${TEST_DIR}/error/insert_mixed.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Copy constructor.
set(TEST_NAME "error_copy_ctor")
add_executable("${TEST_NAME}" "${TEST_DIR}/error/copy_ctor.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Assignment operator.
set(TEST_NAME "error_assignment")
add_executable("${TEST_NAME}" "${TEST_DIR}/error/assignment.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

#
# Notifications tests.
#

# First notification delay on host.
set(TEST_NAME "notifications_first_notif_delay_host")
add_executable(
  "${TEST_NAME}"
  "${TEST_DIR}/notifications/first_notif_delay/common.cc"
  "${TEST_DIR}/notifications/first_notif_delay/common.hh"
  "${TEST_DIR}/notifications/first_notif_delay/host.cc"
)
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# First notification delay on service.
set(TEST_NAME "notifications_first_notif_delay_service")
add_executable(
  "${TEST_NAME}"
  "${TEST_DIR}/notifications/first_notif_delay/common.cc"
  "${TEST_DIR}/notifications/first_notif_delay/common.hh"
  "${TEST_DIR}/notifications/first_notif_delay/service.cc"
)
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# First notification delay on host recovery.
set(TEST_NAME "notifications_first_notif_delay_host_recovery")
add_executable(
  "${TEST_NAME}"
  "${TEST_DIR}/notifications/first_notif_delay/common.cc"
  "${TEST_DIR}/notifications/first_notif_delay/common.hh"
  "${TEST_DIR}/notifications/first_notif_delay/host_recovery.cc"
)
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# First notification delay on service recovery.
set(TEST_NAME "notifications_first_notif_delay_service_recovery")
add_executable(
  "${TEST_NAME}"
  "${TEST_DIR}/notifications/first_notif_delay/common.cc"
  "${TEST_DIR}/notifications/first_notif_delay/common.hh"
  "${TEST_DIR}/notifications/first_notif_delay/service_recovery.cc"
)
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Host escalated notifications.
set(TEST_NAME "notifications_escalations")
add_executable(
  "${TEST_NAME}"
  "${TEST_DIR}/notifications/escalations.cc"
)
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

#
# Stats tests.
#

# Non-existent configuration file.
set(TEST_NAME "running_stats_no_config_file")
add_test(
  "${TEST_NAME}"
  "${CENTENGINESTATS_BINARY}" "-c" "${TEST_DIR}/running/etc/does_not_exist.cfg"
)
set_tests_properties("${TEST_NAME}" PROPERTIES WILL_FAIL TRUE)

# Non-existent status file.
set(TEST_NAME "running_stats_no_status_file")
add_test(
  "${TEST_NAME}"
  "${CENTENGINESTATS_BINARY}" "-c" "${TEST_DIR}/running/etc/no_status_file.cfg"
)
set_tests_properties("${TEST_NAME}" PROPERTIES WILL_FAIL TRUE)

#
# Engine tests.
#

set(CENTENGINE_RUNNER "centengine_must_run")
add_executable(
  "${CENTENGINE_RUNNER}"
  "${TEST_DIR}/running/centengine_must_run.cc"
)
target_link_libraries("${CENTENGINE_RUNNER}" "cce_core")

# Help flag.
add_test("running_engine_help" "${CENTENGINE_BINARY}" "-h")

# License flag.
add_test("running_engine_license" "${CENTENGINE_BINARY}" "-V")

# Without any service.
add_test(
  "running_engine_no_service"
  "${CENTENGINE_BINARY}" "-v" "${TEST_DIR}/running/etc/no_service.cfg"
)
set_tests_properties(
  "running_engine_no_service"
  PROPERTIES WILL_SUCCESS TRUE
)

# Empty host group.
set(TEST_NAME "running_engine_empty_hostgroup")
add_test(
  "${TEST_NAME}"
  "${CENTENGINE_BINARY}" "-v" "${TEST_DIR}/running/etc/empty_hostgroup.cfg"
)

# Empty service group.
set(TEST_NAME "running_engine_empty_servicegroup")
add_test(
  "${TEST_NAME}"
  "${CENTENGINE_BINARY}" "-v" "${TEST_DIR}/running/etc/empty_servicegroup.cfg"
)

# Empty host group assignment error.
set(TEST_NAME "running_engine_empty_hostgroup_assignment_error")
add_test(
  "${TEST_NAME}"
  "${CENTENGINE_BINARY}" "-v" "${TEST_DIR}/running/etc/empty_hostgroup_assignment_error.cfg"
)
set_tests_properties("${TEST_NAME}" PROPERTIES WILL_FAIL TRUE)

# Empty host group assignment success.
set(TEST_NAME "running_engine_empty_hostgroup_assignment_success")
add_test(
  "${TEST_NAME}"
  "${CENTENGINE_BINARY}" "-v" "${TEST_DIR}/running/etc/empty_hostgroup_assignment_success.cfg"
)

# Invalid contactgroup on contact.
set(TEST_NAME "invalid_contactgroup_on_contact")
add_test(
  "${TEST_NAME}"
  "${CENTENGINE_BINARY}" "-v" "${TEST_DIR}/running/etc/invalid_contactgroup_on_contact.cfg"
)
set_tests_properties("${TEST_NAME}" PROPERTIES WILL_FAIL TRUE)

# broker_module with only path to module.
set(TEST_NAME "running_engine_broker_module_without_args")
add_test(
  "${TEST_NAME}"
  "${CENTENGINE_BINARY}" "-v" "${TEST_DIR}/running/etc/broker_module_without_args.cfg"
)

# Check that all symbols required by modules are present.
set(TEST_NAME "running_engine_broker_modules_symbols")
add_library(
  "running_engine_broker_modules_symbols_checker"
  SHARED
  "${TEST_DIR}/running/modules_symbols_checker.cc"
)
set_property(
  TARGET "running_engine_broker_modules_symbols_checker"
  PROPERTY PREFIX ""
)
add_library(
  "running_engine_broker_modules_symbols"
  SHARED
  "${TEST_DIR}/running/modules_symbols.cc"
)
set_property(
  TARGET "running_engine_broker_modules_symbols"
  PROPERTY PREFIX ""
)
get_property(
  MODULE_PATH
  TARGET "running_engine_broker_modules_symbols_checker"
  PROPERTY LOCATION
)
get_property(
  MODULE_ARGS
  TARGET "running_engine_broker_modules_symbols"
  PROPERTY LOCATION
)
set(
  CFG_CONTENT
  "command_file=/tmp/running_engine_broker_modules_symbols.tmp
broker_module=${EXTERNALCMD_MODULE}
broker_module=${MODULE_PATH} ${MODULE_ARGS}"
)
configure_file(
  "${TEST_DIR}/running/etc/minimal.cfg.in"
  "${TEST_DIR}/running/etc/modules_symbols.cfg"
)
add_test(
  "${TEST_NAME}"
  "${CENTENGINE_BINARY}" "${TEST_DIR}/running/etc/modules_symbols.cfg"
)
# Check that the webservice module can be loaded.
if (WITH_WEBSERVICE)
  set(TEST_NAME "running_engine_broker_modules_webservice")
  unset(MODULE_ARGS)
  get_property(MODULE_ARGS TARGET "webservice" PROPERTY LOCATION)
  set(CFG_CONTENT
    "command_file=/tmp/running_engine_broker_modules_webservice.tmp
broker_module=${EXTERNALCMD_MODULE}
broker_module=${MODULE_PATH} ${MODULE_ARGS}"
)
  configure_file(
    "${TEST_DIR}/running/etc/minimal.cfg.in"
    "${TEST_DIR}/running/etc/modules_webservice.cfg")
  add_test(
    "${TEST_NAME}"
    "${CENTENGINE_BINARY}" "${TEST_DIR}/running/etc/modules_webservice.cfg"
  )
endif ()

# Active check of host with no command.
set(
  CFG_CONTENT
  "cfg_file=${TEST_DIR}/running/etc/host_with_no_check_cmd_objects.cfg"
)
configure_file(
  "${TEST_DIR}/running/etc/minimal.cfg.in"
  "${TEST_DIR}/running/etc/host_with_no_check_cmd.cfg"
)
set(TEST_NAME "running_host_check_with_no_check_cmd")
add_test(
  "${TEST_NAME}"
  "${CENTENGINE_RUNNER}" "${TEST_DIR}/running/etc/host_with_no_check_cmd.cfg" "5"
)

# Active check of service with no command.
set(
  CFG_CONTENT
  "cfg_file=${TEST_DIR}/running/etc/service_with_no_check_cmd_objects.cfg"
)
configure_file(
  "${TEST_DIR}/running/etc/minimal.cfg.in"
  "${TEST_DIR}/running/etc/service_with_no_check_cmd.cfg"
)
set(TEST_NAME "running_service_check_with_no_check_cmd")
add_test(
  "${TEST_NAME}"
  "${CENTENGINE_BINARY}" "-v" "${TEST_DIR}/running/etc/service_with_no_check_cmd.cfg"
)
set_tests_properties("${TEST_NAME}" PROPERTIES WILL_FAIL TRUE)

# Logs are generated for broker modules.
set(TEST_NAME "running_logs_for_modules")
add_library(
  "logs_for_modules"
  SHARED
  "${TEST_DIR}/running/logs_for_modules.cc"
)
unset(MODULE_PATH)
get_property(MODULE_PATH TARGET "logs_for_modules" PROPERTY LOCATION)
set(CFG_CONTENT "broker_module=${MODULE_PATH}")
configure_file(
  "${TEST_DIR}/running/etc/minimal.cfg.in"
  "${TEST_DIR}/running/etc/logs_for_modules.cfg"
)
add_test(
  "${TEST_NAME}"
  "${CENTENGINE_BINARY}" "${TEST_DIR}/running/etc/logs_for_modules.cfg"
)

# Precache file generation.
set(TEST_NAME "running_precache_file_generation")
add_executable(
  "${TEST_NAME}"
  "${TEST_DIR}/running/precache_file_generation.cc"
)
target_link_libraries("${TEST_NAME}" "cce_core")
add_test(
  "${TEST_NAME}"
  "${TEST_NAME}"
  "${TEST_DIR}/running/etc/precache/main.cfg"
  "${TEST_DIR}/running/etc/precache/objects.precache.expected"
)

# log without pid
set(TEST_NAME "running_log_without_pid")
add_executable(
  "${TEST_NAME}"
  "${TEST_DIR}/running/log_without_pid.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test(
  "${TEST_NAME}"
  "${TEST_NAME}"
  "${TEST_DIR}/running/etc/log_without_pid/main.cfg"
)

set(TEST_DIR "${TEST_DIR}/core")

## get_next_valid_time
set(TEST_BIN_NAME "get_next_valid_time")
add_executable("${TEST_BIN_NAME}" "${TEST_DIR}/${TEST_BIN_NAME}.cc")
target_link_libraries("${TEST_BIN_NAME}" "cce_core")

#
# exception
#

## calendar_date
set(CONF_DIR "${TEST_DIR}/configuration/exception/calendar_date")

# exception_calendar_date_date_after_period
set(TEST_NAME "exception_calendar_date_date_after_period")
set(TEST_CONF_FILE "date_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_date_before_period
set(TEST_NAME "exception_calendar_date_date_before_period")
set(TEST_CONF_FILE "date_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_date_into_period
set(TEST_NAME "exception_calendar_date_date_into_period")
set(TEST_CONF_FILE "date_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_date_into_period_with_exclude_after
set(TEST_NAME "exception_calendar_date_date_into_period_with_exclude_after")
set(TEST_CONF_FILE "date_into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_date_into_period_with_exclude_before
set(TEST_NAME "exception_calendar_date_date_into_period_with_exclude_before")
set(TEST_CONF_FILE "date_into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_date_into_period_with_exclude_into
set(TEST_NAME "exception_calendar_date_date_into_period_with_exclude_into")
set(TEST_CONF_FILE "date_into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_date_skip_in_after_period
set(TEST_NAME "exception_calendar_date_date_skip_in_after_period")
set(TEST_CONF_FILE "date_skip_in_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_date_skip_in_before_period
set(TEST_NAME "exception_calendar_date_date_skip_in_before_period")
set(TEST_CONF_FILE "date_skip_in_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_date_skip_in_into_period
set(TEST_NAME "exception_calendar_date_date_skip_in_into_period")
set(TEST_CONF_FILE "date_skip_in_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_date_skip_in_into_period_with_exclude_after
set(TEST_NAME "exception_calendar_date_date_skip_in_into_period_with_exclude_after")
set(TEST_CONF_FILE "date_skip_in_into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_date_skip_in_into_period_with_exclude_before
set(TEST_NAME "exception_calendar_date_date_skip_in_into_period_with_exclude_before")
set(TEST_CONF_FILE "date_skip_in_into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_date_skip_in_into_period_with_exclude_into
set(TEST_NAME "exception_calendar_date_date_skip_in_into_period_with_exclude_into")
set(TEST_CONF_FILE "date_skip_in_into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_start_end_date_skip_in_after_period
set(TEST_NAME "exception_calendar_date_start_end_date_skip_in_after_period")
set(TEST_CONF_FILE "start_end_date_skip_in_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_start_end_date_skip_in_before_period
set(TEST_NAME "exception_calendar_date_start_end_date_skip_in_before_period")
set(TEST_CONF_FILE "start_end_date_skip_in_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_start_end_date_skip_in_into_period
set(TEST_NAME "exception_calendar_date_start_end_date_skip_in_into_period")
set(TEST_CONF_FILE "start_end_date_skip_in_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_start_end_date_skip_in_into_period_with_exclude_after
set(TEST_NAME "exception_calendar_date_start_end_date_skip_in_into_period_with_exclude_after")
set(TEST_CONF_FILE "start_end_date_skip_in_into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_start_end_date_skip_in_into_period_with_exclude_before
set(TEST_NAME "exception_calendar_date_start_end_date_skip_in_into_period_with_exclude_before")
set(TEST_CONF_FILE "start_end_date_skip_in_into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_start_end_date_skip_in_into_period_with_exclude_into
set(TEST_NAME "exception_calendar_date_start_end_date_skip_in_into_period_with_exclude_into")
set(TEST_CONF_FILE "start_end_date_skip_in_into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_start_end_date_skip_out_after_period
set(TEST_NAME "exception_calendar_date_start_end_date_skip_out_after_period")
set(TEST_CONF_FILE "start_end_date_skip_out_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_start_end_date_skip_out_before_period
set(TEST_NAME "exception_calendar_date_start_end_date_skip_out_before_period")
set(TEST_CONF_FILE "start_end_date_skip_out_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_start_end_date_skip_out_into_period
set(TEST_NAME "exception_calendar_date_start_end_date_skip_out_into_period")
set(TEST_CONF_FILE "start_end_date_skip_out_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_start_end_date_skip_out_after_period_with_exclude_after
set(TEST_NAME "exception_calendar_date_start_end_date_skip_out_after_period_with_exclude_after")
set(TEST_CONF_FILE "start_end_date_skip_out_after_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_start_end_date_skip_out_after_period_with_exclude_before
set(TEST_NAME "exception_calendar_date_start_end_date_skip_out_after_period_with_exclude_before")
set(TEST_CONF_FILE "start_end_date_skip_out_after_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_calendar_date_start_end_date_skip_out_after_period_with_exclude_into
set(TEST_NAME "exception_calendar_date_start_end_date_skip_out_after_period_with_exclude_into")
set(TEST_CONF_FILE "start_end_date_skip_out_after_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")


## month_day
set(CONF_DIR "${TEST_DIR}/configuration/exception/month_day")

# exception_month_day_day_after_period
set(TEST_NAME "exception_month_day_day_after_period")
set(TEST_CONF_FILE "day_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_day_day_before_period
set(TEST_NAME "exception_month_day_day_before_period")
set(TEST_CONF_FILE "day_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_day_day_into_period
set(TEST_NAME "exception_month_day_day_into_period")
set(TEST_CONF_FILE "day_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_day_day_into_period_with_exclude_after
set(TEST_NAME "exception_month_day_day_into_period_with_exclude_after")
set(TEST_CONF_FILE "day_into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_day_day_into_period_with_exclude_before
set(TEST_NAME "exception_month_day_day_into_period_with_exclude_before")
set(TEST_CONF_FILE "day_into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_day_day_into_period_with_exclude_into
set(TEST_NAME "exception_month_day_day_into_period_with_exclude_into")
set(TEST_CONF_FILE "day_into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")


## month_week_day
set(CONF_DIR "${TEST_DIR}/configuration/exception/month_week_day")

# exception_month_week_day_4_weekday_month_after_period
set(TEST_NAME "exception_month_week_day_4_weekday_month_after_period")
set(TEST_CONF_FILE "4_weekday_month_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_week_day_4_weekday_month_before_period
set(TEST_NAME "exception_month_week_day_4_weekday_month_before_period")
set(TEST_CONF_FILE "4_weekday_month_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_week_day_4_weekday_month_into_period
set(TEST_NAME "exception_month_week_day_4_weekday_month_into_period")
set(TEST_CONF_FILE "4_weekday_month_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_week_day_4_weekday_month_into_period_with_exclude_after
set(TEST_NAME "exception_month_week_day_4_weekday_month_into_period_with_exclude_after")
set(TEST_CONF_FILE "4_weekday_month_into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_week_day_4_weekday_month_into_period_with_exclude_before
set(TEST_NAME "exception_month_week_day_4_weekday_month_into_period_with_exclude_before")
set(TEST_CONF_FILE "4_weekday_month_into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_week_day_4_weekday_month_into_period_with_exclude_into
set(TEST_NAME "exception_month_week_day_4_weekday_month_into_period_with_exclude_into")
set(TEST_CONF_FILE "4_weekday_month_into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_week_day_last_weekday_month_after_period
set(TEST_NAME "exception_month_week_day_last_weekday_month_after_period")
set(TEST_CONF_FILE "last_weekday_month_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_week_day_last_weekday_month_before_period
set(TEST_NAME "exception_month_week_day_last_weekday_month_before_period")
set(TEST_CONF_FILE "last_weekday_month_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_week_day_last_weekday_month_into_period
set(TEST_NAME "exception_month_week_day_last_weekday_month_into_period")
set(TEST_CONF_FILE "last_weekday_month_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_week_day_last_weekday_month_into_period_with_exclude_after
set(TEST_NAME "exception_month_week_day_last_weekday_month_into_period_with_exclude_after")
set(TEST_CONF_FILE "last_weekday_month_into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_week_day_last_weekday_month_into_period_with_exclude_before
set(TEST_NAME "exception_month_week_day_last_weekday_month_into_period_with_exclude_before")
set(TEST_CONF_FILE "last_weekday_month_into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_month_week_day_last_weekday_month_into_period_with_exclude_into
set(TEST_NAME "exception_month_week_day_last_weekday_month_into_period_with_exclude_into")
set(TEST_CONF_FILE "last_weekday_month_into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")


## week_day
set(CONF_DIR "${TEST_DIR}/configuration/exception/week_day")

# exception_week_day_4_weekday_after_period
set(TEST_NAME "exception_week_day_4_weekday_after_period")
set(TEST_CONF_FILE "4_weekday_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_week_day_4_weekday_before_period
set(TEST_NAME "exception_week_day_4_weekday_before_period")
set(TEST_CONF_FILE "4_weekday_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_week_day_4_weekday_into_period
set(TEST_NAME "exception_week_day_4_weekday_into_period")
set(TEST_CONF_FILE "4_weekday_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_week_day_4_weekday_into_period_with_exclude_after
set(TEST_NAME "exception_week_day_4_weekday_into_period_with_exclude_after")
set(TEST_CONF_FILE "4_weekday_into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_week_day_4_weekday_into_period_with_exclude_before
set(TEST_NAME "exception_week_day_4_weekday_into_period_with_exclude_before")
set(TEST_CONF_FILE "4_weekday_into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_week_day_4_weekday_into_period_with_exclude_into
set(TEST_NAME "exception_week_day_4_weekday_into_period_with_exclude_into")
set(TEST_CONF_FILE "4_weekday_into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_week_day_last_weekday_after_period
set(TEST_NAME "exception_week_day_last_weekday_after_period")
set(TEST_CONF_FILE "last_weekday_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_week_day_last_weekday_before_period
set(TEST_NAME "exception_week_day_last_weekday_before_period")
set(TEST_CONF_FILE "last_weekday_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_week_day_last_weekday_into_period
set(TEST_NAME "exception_week_day_last_weekday_into_period")
set(TEST_CONF_FILE "last_weekday_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_week_day_last_weekday_into_period_with_exclude_after
set(TEST_NAME "exception_week_day_last_weekday_into_period_with_exclude_after")
set(TEST_CONF_FILE "last_weekday_into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_week_day_last_weekday_into_period_with_exclude_before
set(TEST_NAME "exception_week_day_last_weekday_into_period_with_exclude_before")
set(TEST_CONF_FILE "last_weekday_into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# exception_week_day_last_weekday_into_period_with_exclude_into
set(TEST_NAME "exception_week_day_last_weekday_into_period_with_exclude_into")
set(TEST_CONF_FILE "last_weekday_into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")


#
# weekday
#
set(CONF_DIR "${TEST_DIR}/configuration/weekday")

# weekday_after_period
set(TEST_NAME "weekday_after_period")
set(TEST_CONF_FILE "after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# weekday_before_period
set(TEST_NAME "weekday_before_period")
set(TEST_CONF_FILE "before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# weekday_into_period
set(TEST_NAME "weekday_into_period")
set(TEST_CONF_FILE "into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# weekday_into_period_for_next_week
set(TEST_NAME "weekday_into_period_for_next_week")
set(TEST_CONF_FILE "into_period_for_next_week.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# weekday_into_period_with_exclude_after
set(TEST_NAME "weekday_into_period_with_exclude_after")
set(TEST_CONF_FILE "into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# weekday_into_period_with_exclude_before
set(TEST_NAME "weekday_into_period_with_exclude_before")
set(TEST_CONF_FILE "into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# weekday_into_period_with_exclude_into
set(TEST_NAME "weekday_into_period_with_exclude_into")
set(TEST_CONF_FILE "into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

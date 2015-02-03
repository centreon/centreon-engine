##
## Copyright 2013-2014 Merethis
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
set(TEST_DIR "${PROJECT_SOURCE_DIR}/test/timeperiod")

# get_next_valid_time is the binary used to check timeperiods.
set(TEST_BIN_NAME "get_next_valid_time")
add_executable("${TEST_BIN_NAME}" "${TEST_DIR}/${TEST_BIN_NAME}.cc")
target_link_libraries("${TEST_BIN_NAME}" "cce_core")

#
# Timeperiod exceptions.
#

## Calendar date (2014-07-30).
set(CONF_DIR "${TEST_DIR}/etc/exception/calendar_date")

# timeperiod_exception_calendar_date_after_period
set(TEST_NAME "timeperiod_exception_calendar_date_after_period")
set(TEST_CONF_FILE "after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_before_period
set(TEST_NAME "timeperiod_exception_calendar_date_before_period")
set(TEST_CONF_FILE "before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_into_period
set(TEST_NAME "timeperiod_exception_calendar_date_into_period")
set(TEST_CONF_FILE "into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_into_period_with_exclude_after
set(TEST_NAME "timeperiod_exception_calendar_date_into_period_with_exclude_after")
set(TEST_CONF_FILE "into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_into_period_with_exclude_before
set(TEST_NAME "timeperiod_exception_calendar_date_into_period_with_exclude_before")
set(TEST_CONF_FILE "into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_into_period_with_exclude_into
set(TEST_NAME "timeperiod_exception_calendar_date_into_period_with_exclude_into")
set(TEST_CONF_FILE "into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_skip_in_after_period
set(TEST_NAME "timeperiod_exception_calendar_date_skip_in_after_period")
set(TEST_CONF_FILE "skip_in_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_skip_in_before_period
set(TEST_NAME "timeperiod_exception_calendar_date_skip_in_before_period")
set(TEST_CONF_FILE "skip_in_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_skip_in_into_period
set(TEST_NAME "timeperiod_exception_calendar_date_skip_in_into_period")
set(TEST_CONF_FILE "skip_in_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_skip_in_into_period_with_exclude_after
set(TEST_NAME "timeperiod_exception_calendar_date_skip_in_into_period_with_exclude_after")
set(TEST_CONF_FILE "skip_in_into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_skip_in_into_period_with_exclude_before
set(TEST_NAME "timeperiod_exception_calendar_date_skip_in_into_period_with_exclude_before")
set(TEST_CONF_FILE "skip_in_into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_skip_in_into_period_with_exclude_into
set(TEST_NAME "timeperiod_exception_calendar_date_skip_in_into_period_with_exclude_into")
set(TEST_CONF_FILE "skip_in_into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_start_end_date_skip_in_after_period
set(TEST_NAME "timeperiod_exception_calendar_date_start_end_skip_in_after_period")
set(TEST_CONF_FILE "start_end_skip_in_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_start_end_skip_in_before_period
set(TEST_NAME "timeperiod_exception_calendar_date_start_end_skip_in_before_period")
set(TEST_CONF_FILE "start_end_skip_in_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_start_end_skip_in_into_period
set(TEST_NAME "timeperiod_exception_calendar_date_start_end_skip_in_into_period")
set(TEST_CONF_FILE "start_end_skip_in_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_start_end_skip_in_into_period_with_exclude_after
set(TEST_NAME "timeperiod_exception_calendar_date_start_end_skip_in_into_period_with_exclude_after")
set(TEST_CONF_FILE "start_end_skip_in_into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_start_end_skip_in_into_period_with_exclude_before
set(TEST_NAME "timeperiod_exception_calendar_date_start_end_skip_in_into_period_with_exclude_before")
set(TEST_CONF_FILE "start_end_skip_in_into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_start_end_skip_in_into_period_with_exclude_into
set(TEST_NAME "timeperiod_exception_calendar_date_start_end_skip_in_into_period_with_exclude_into")
set(TEST_CONF_FILE "start_end_skip_in_into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_start_end_skip_out_after_period
set(TEST_NAME "timeperiod_exception_calendar_date_start_end_skip_out_after_period")
set(TEST_CONF_FILE "start_end_skip_out_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_start_end_skip_out_before_period
set(TEST_NAME "timeperiod_exception_calendar_date_start_end_skip_out_before_period")
set(TEST_CONF_FILE "start_end_skip_out_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_start_end_skip_out_into_period
set(TEST_NAME "timeperiod_exception_calendar_date_start_end_skip_out_into_period")
set(TEST_CONF_FILE "start_end_skip_out_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_start_end_skip_out_after_period_with_exclude_after
set(TEST_NAME "timeperiod_exception_calendar_date_start_end_skip_out_after_period_with_exclude_after")
set(TEST_CONF_FILE "start_end_skip_out_after_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_start_end_skip_out_after_period_with_exclude_before
set(TEST_NAME "timeperiod_exception_calendar_date_start_end_skip_out_after_period_with_exclude_before")
set(TEST_CONF_FILE "start_end_skip_out_after_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_calendar_date_start_end_skip_out_after_period_with_exclude_into
set(TEST_NAME "timeperiod_exception_calendar_date_start_end_skip_out_after_period_with_exclude_into")
set(TEST_CONF_FILE "start_end_skip_out_after_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")


## month_day (july 30).
set(CONF_DIR "${TEST_DIR}/etc/exception/month_day")

# timeperiod_exception_month_day_after_period
set(TEST_NAME "timeperiod_exception_month_day_after_period")
set(TEST_CONF_FILE "after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_day_before_period
set(TEST_NAME "timeperiod_exception_month_day_before_period")
set(TEST_CONF_FILE "before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_day_into_period
set(TEST_NAME "timeperiod_exception_month_day_into_period")
set(TEST_CONF_FILE "into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_day_into_period_with_exclude_after
set(TEST_NAME "timeperiod_exception_month_day_into_period_with_exclude_after")
set(TEST_CONF_FILE "into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_day_into_period_with_exclude_before
set(TEST_NAME "timeperiod_exception_month_day_into_period_with_exclude_before")
set(TEST_CONF_FILE "into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_day_into_period_with_exclude_into
set(TEST_NAME "timeperiod_exception_month_day_into_period_with_exclude_into")
set(TEST_CONF_FILE "into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")


## month_week_day (tuesday 1 april).
set(CONF_DIR "${TEST_DIR}/etc/exception/month_week_day")

# timeperiod_exception_month_week_day_4_after_period
set(TEST_NAME "timeperiod_exception_month_week_day_4_after_period")
set(TEST_CONF_FILE "4_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_week_day_4_before_period
set(TEST_NAME "timeperiod_exception_month_week_day_4_before_period")
set(TEST_CONF_FILE "4_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_week_day_4_into_period
set(TEST_NAME "timeperiod_exception_month_week_day_4_into_period")
set(TEST_CONF_FILE "4_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_week_day_4_into_period_with_exclude_after
set(TEST_NAME "timeperiod_exception_month_week_day_4_into_period_with_exclude_after")
set(TEST_CONF_FILE "4_into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_week_day_4_into_period_with_exclude_before
set(TEST_NAME "timeperiod_exception_month_week_day_4_into_period_with_exclude_before")
set(TEST_CONF_FILE "4_into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_week_day_4_into_period_with_exclude_into
set(TEST_NAME "timeperiod_exception_month_week_day_4_into_period_with_exclude_into")
set(TEST_CONF_FILE "4_into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_week_day_last_after_period
set(TEST_NAME "timeperiod_exception_month_week_day_last_after_period")
set(TEST_CONF_FILE "last_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_week_day_last_before_period
set(TEST_NAME "timeperiod_exception_month_week_day_last_before_period")
set(TEST_CONF_FILE "last_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_week_day_last_into_period
set(TEST_NAME "timeperiod_exception_month_week_day_last_into_period")
set(TEST_CONF_FILE "last_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_week_day_last_into_period_with_exclude_after
set(TEST_NAME "timeperiod_exception_month_week_day_last_into_period_with_exclude_after")
set(TEST_CONF_FILE "last_into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_week_day_last_into_period_with_exclude_before
set(TEST_NAME "timeperiod_exception_month_week_day_last_into_period_with_exclude_before")
set(TEST_CONF_FILE "last_into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_month_week_day_last_into_period_with_exclude_into
set(TEST_NAME "timeperiod_exception_month_week_day_last_into_period_with_exclude_into")
set(TEST_CONF_FILE "last_into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")


## week_day (monday 4).
set(CONF_DIR "${TEST_DIR}/etc/exception/week_day")

# timeperiod_exception_week_day_4_after_period
set(TEST_NAME "timeperiod_exception_week_day_4_after_period")
set(TEST_CONF_FILE "4_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_week_day_4_before_period
set(TEST_NAME "timeperiod_exception_week_day_4_before_period")
set(TEST_CONF_FILE "4_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_week_day_4_into_period
set(TEST_NAME "timeperiod_exception_week_day_4_into_period")
set(TEST_CONF_FILE "4_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_week_day_4_into_period_with_exclude_after
set(TEST_NAME "timeperiod_exception_week_day_4_into_period_with_exclude_after")
set(TEST_CONF_FILE "4_into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_week_day_4_into_period_with_exclude_before
set(TEST_NAME "timeperiod_exception_week_day_4_into_period_with_exclude_before")
set(TEST_CONF_FILE "4_into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_week_day_4_into_period_with_exclude_into
set(TEST_NAME "timeperiod_exception_week_day_4_into_period_with_exclude_into")
set(TEST_CONF_FILE "4_into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_week_day_last_after_period
set(TEST_NAME "timeperiod_exception_week_day_last_after_period")
set(TEST_CONF_FILE "last_after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_week_day_last_before_period
set(TEST_NAME "timeperiod_exception_week_day_last_before_period")
set(TEST_CONF_FILE "last_before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_week_day_last_into_period
set(TEST_NAME "timeperiod_exception_week_day_last_into_period")
set(TEST_CONF_FILE "last_into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_week_day_last_into_period_with_exclude_after
set(TEST_NAME "timeperiod_exception_week_day_last_into_period_with_exclude_after")
set(TEST_CONF_FILE "last_into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_week_day_last_into_period_with_exclude_before
set(TEST_NAME "timeperiod_exception_week_day_last_into_period_with_exclude_before")
set(TEST_CONF_FILE "last_into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_exception_week_day_last_into_period_with_exclude_into
set(TEST_NAME "timeperiod_exception_week_day_last_into_period_with_exclude_into")
set(TEST_CONF_FILE "last_into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")


#
# Timeperiod week days (normal rotating schedule).
#
set(CONF_DIR "${TEST_DIR}/etc/week_day")

# timeperiod_week_day_after_period
set(TEST_NAME "timeperiod_week_day_after_period")
set(TEST_CONF_FILE "after_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_week_day_before_period
set(TEST_NAME "timeperiod_week_day_before_period")
set(TEST_CONF_FILE "before_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_week_day_into_period
set(TEST_NAME "timeperiod_week_day_into_period")
set(TEST_CONF_FILE "into_period.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_week_day_into_period_for_next_week
set(TEST_NAME "timeperiod_week_day_into_period_for_next_week")
set(TEST_CONF_FILE "into_period_for_next_week.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_week_day_into_period_with_exclude_after
set(TEST_NAME "timeperiod_week_day_into_period_with_exclude_after")
set(TEST_CONF_FILE "into_period_with_exclude_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_week_day_into_period_with_exclude_before
set(TEST_NAME "timeperiod_week_day_into_period_with_exclude_before")
set(TEST_CONF_FILE "into_period_with_exclude_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# timeperiod_week_day_into_period_with_exclude_into
set(TEST_NAME "timeperiod_week_day_into_period_with_exclude_into")
set(TEST_CONF_FILE "into_period_with_exclude_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")


#
# Timeperiod exclusions.
#
set(CONF_DIR "${TEST_DIR}/etc/exclusion")

# timeperiod_exclusion_nested_after
set(TEST_NAME "timeperiod_exclusion_nested_after")
set(TEST_CONF_FILE "nested_after.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")
# timeperiod_exclusion_nested_before
set(TEST_NAME "timeperiod_exclusion_nested_before")
set(TEST_CONF_FILE "nested_before.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")
# timeperiod_exclusion_nested_into
set(TEST_NAME "timeperiod_exclusion_nested_into")
set(TEST_CONF_FILE "nested_into.conf")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

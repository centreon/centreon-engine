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

# # Set directories.
# set(SRC_DIR "${SRC_DIR}/core")
# set(INC_DIR "${INC_DIR}/com/centreon/engine/core")

if (WITH_TESTING)
  # Set directories.
  set(TEST_DIR "${TEST_DIR}/core")

  # ## get_next_valid_time
  # set(TEST_BIN_NAME "get_next_valid_time")
  # add_executable("${TEST_BIN_NAME}" "${TEST_DIR}/${TEST_BIN_NAME}.cc")
  # target_link_libraries("${TEST_BIN_NAME}" "cce_core")

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
endif()

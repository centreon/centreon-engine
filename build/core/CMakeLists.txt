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
  set(CONF_DIR "${TEST_DIR}/configuration")

  # get_next_valid_time
  set(TEST_BIN_NAME "get_next_valid_time")
  add_executable("${TEST_BIN_NAME}" "${TEST_DIR}/${TEST_BIN_NAME}.cc")
  target_link_libraries("${TEST_BIN_NAME}" "cce_core")

  # day_exception_after_range
  set(TEST_NAME "day_exception_after_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # day_exception_before_range
  set(TEST_NAME "day_exception_before_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # day_exception_into_range
  set(TEST_NAME "day_exception_into_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")


  # day_exception_exclude_after_range
  set(TEST_NAME "day_exception_exclude_after_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # day_exception_exclude_before_range
  set(TEST_NAME "day_exception_exclude_before_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # day_exception_exclude_into_range
  set(TEST_NAME "day_exception_exclude_into_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")


  # last_monday_exception_after_range
  set(TEST_NAME "last_monday_exception_after_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # last_monday_exception_before_range
  set(TEST_NAME "last_monday_exception_before_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # last_monday_exception_into_range
  set(TEST_NAME "last_monday_exception_into_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")


  # last_monday_exception_exclude_after_range
  set(TEST_NAME "last_monday_exception_exclude_after_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # last_monday_exception_exclude_before_range
  set(TEST_NAME "last_monday_exception_exclude_before_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # last_monday_exception_exclude_into_range
  set(TEST_NAME "last_monday_exception_exclude_into_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")


  # four_monday_exception_after_range
  set(TEST_NAME "four_monday_exception_after_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # four_monday_exception_before_range
  set(TEST_NAME "four_monday_exception_before_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # four_monday_exception_into_range
  set(TEST_NAME "four_monday_exception_into_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")


  # four_monday_exception_exclude_after_range
  set(TEST_NAME "four_monday_exception_exclude_after_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # four_monday_exception_exclude_before_range
  set(TEST_NAME "four_monday_exception_exclude_before_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # four_monday_exception_exclude_into_range
  set(TEST_NAME "four_monday_exception_exclude_into_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")


  # skip_in_day_exception_after_range
  set(TEST_NAME "skip_in_day_exception_after_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # skip_in_day_exception_before_range
  set(TEST_NAME "skip_in_day_exception_before_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # skip_in_day_exception_into_range
  set(TEST_NAME "skip_in_day_exception_into_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")


  # skip_in_day_exception_exclude_after_range
  set(TEST_NAME "skip_in_day_exception_exclude_after_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # skip_in_day_exception_exclude_before_range
  set(TEST_NAME "skip_in_day_exception_exclude_before_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # skip_in_day_exception_exclude_into_range
  set(TEST_NAME "skip_in_day_exception_exclude_into_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")


  # skip_out_day_exception_after_range
  set(TEST_NAME "skip_out_day_exception_after_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # skip_out_day_exception_before_range
  set(TEST_NAME "skip_out_day_exception_before_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # skip_out_day_exception_into_range
  set(TEST_NAME "skip_out_day_exception_into_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")


  # skip_out_day_exception_exclude_after_range
  set(TEST_NAME "skip_out_day_exception_exclude_after_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # skip_out_day_exception_exclude_before_range
  set(TEST_NAME "skip_out_day_exception_exclude_before_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # skip_out_day_exception_exclude_into_range
  set(TEST_NAME "skip_out_day_exception_exclude_into_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")


  # speday_exception_after_range
  set(TEST_NAME "speday_exception_after_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # speday_exception_before_range
  set(TEST_NAME "speday_exception_before_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # speday_exception_into_range
  set(TEST_NAME "speday_exception_into_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")


  # speday_exception_exclude_after_range
  set(TEST_NAME "speday_exception_exclude_after_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # speday_exception_exclude_before_range
  set(TEST_NAME "speday_exception_exclude_before_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # speday_exception_exclude_into_range
  set(TEST_NAME "speday_exception_exclude_into_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")


  # weekday_after_range
  set(TEST_NAME "weekday_after_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # weekday_before_range
  set(TEST_NAME "weekday_before_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # weekday_into_range
  set(TEST_NAME "weekday_into_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")


  # weekday_exclude_after_range
  set(TEST_NAME "weekday_exclude_after_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # weekday_exclude_before_range
  set(TEST_NAME "weekday_exclude_before_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")

  # weekday_exclude_into_range
  set(TEST_NAME "weekday_exclude_into_range")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_NAME}.conf")
endif ()

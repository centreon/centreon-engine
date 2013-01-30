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

  # Create test time library.
  add_library("testtime" STATIC "${TEST_DIR}/libtesttime.cc")
  set_target_properties("testtime" PROPERTIES PREFIX "")
  target_link_libraries("testtime" "cce_core")

  # valid_time_week_day_after_current_time
  set(TEST_NAME "valid_time_week_day_after_current_time")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_NAME}.cc")
  target_link_libraries("${TEST_NAME}" "testtime")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # valid_time_week_day_before_current_time
  set(TEST_NAME "valid_time_week_day_before_current_time")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_NAME}.cc")
  target_link_libraries("${TEST_NAME}" "testtime")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # valid_time_week_day_exclude_after_current_time
  set(TEST_NAME "valid_time_week_day_exclude_after_current_time")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_NAME}.cc")
  target_link_libraries("${TEST_NAME}" "testtime")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # valid_time_week_day_exclude_before_current_time
  set(TEST_NAME "valid_time_week_day_exclude_before_current_time")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_NAME}.cc")
  target_link_libraries("${TEST_NAME}" "testtime")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # valid_time_week_day_exclude_into_current_time
  set(TEST_NAME "valid_time_week_day_exclude_into_current_time")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_NAME}.cc")
  target_link_libraries("${TEST_NAME}" "testtime")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # valid_time_week_day_into_current_time
  set(TEST_NAME "valid_time_week_day_into_current_time")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_NAME}.cc")
  target_link_libraries("${TEST_NAME}" "testtime")
  add_test("${TEST_NAME}" "${TEST_NAME}")
endif ()

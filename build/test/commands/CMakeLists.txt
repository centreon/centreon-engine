##
## Copyright 2011-2013 Merethis
##
## This file is part of Centreon Scheduler.
##
## Centreon Scheduler is free software: you can redistribute it and/or
## modify it under the terms of the GNU General Public License version 2
## as published by the Free Software Foundation.
##
## Centreon Scheduler is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Centreon Scheduler. If not, see
## <http://www.gnu.org/licenses/>.
##

# Set directories.
set(TEST_DIR "${TEST_DIR}/commands")

# Test binary.
add_executable("bin_connector_test_run" "${TEST_DIR}/bin_connector_test_run.cc")
target_link_libraries("bin_connector_test_run" ${CLIB_LIBRARIES})

add_executable("bin_test_run" "${TEST_DIR}/bin_test_run.cc")
target_link_libraries("bin_test_run" ${CLIB_LIBRARIES})

# Test connector.
add_executable("connector_ctor" "${TEST_DIR}/connector_ctor.cc")
target_link_libraries("connector_ctor" "cce_core")
add_test("connector_ctor" "connector_ctor")

add_executable("connector_compare" "${TEST_DIR}/connector_compare.cc")
target_link_libraries("connector_compare" "cce_core")
add_test("connector_compare" "connector_compare")

add_executable("connector_run_sync" "${TEST_DIR}/connector_run_sync.cc")
target_link_libraries("connector_run_sync" "cce_core")
add_test("connector_run_sync" "connector_run_sync")

add_executable(
  "connector_run_async"
  "${TEST_DIR}/connector_run_async.cc"
  "${TEST_DIR}/wait_process.hh"
)
target_link_libraries("connector_run_async" "cce_core")
add_test("connector_run_async" "connector_run_async")

add_executable("connector_get" "${TEST_DIR}/connector_get.cc")
target_link_libraries("connector_get" "cce_core")
add_test("connector_get" "connector_get")

add_executable(
  "connector_restart"
  "${TEST_DIR}/connector_restart.cc"
  "${TEST_DIR}/wait_process.hh"
)
target_link_libraries("connector_restart" "cce_core")
add_test("connector_restart" "connector_restart")

# Test environment.
add_executable("environment_add" "${TEST_DIR}/environment_add.cc")
target_link_libraries("environment_add" "cce_core")
add_test("environment_add" "environment_add")

# Test environment.
set(TEST_NAME "environment_compare")
add_executable("${TEST_NAME}" "${TEST_DIR}/environment_compare.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

# Test environment.
add_executable("environment_ctor" "${TEST_DIR}/environment_ctor.cc")
target_link_libraries("environment_ctor" "cce_core")
add_test("environment_ctor" "environment_ctor")

# Test result.
add_executable("result_ctor" "${TEST_DIR}/result_ctor.cc")
target_link_libraries("result_ctor" "cce_core")
add_test("result_ctor" "result_ctor")

add_executable("result_compare" "${TEST_DIR}/result_compare.cc")
target_link_libraries("result_compare" "cce_core")
add_test("result_compare" "result_compare")

# Test set.
add_executable("set_basic" "${TEST_DIR}/set_basic.cc")
target_link_libraries("set_basic" "cce_core")
add_test("set_basic" "set_basic")

# Test raw.
add_executable("raw_ctor" "${TEST_DIR}/raw_ctor.cc")
target_link_libraries("raw_ctor" "cce_core")
add_test("raw_ctor" "raw_ctor")

add_executable("raw_compare" "${TEST_DIR}/raw_compare.cc")
target_link_libraries("raw_compare" "cce_core")
add_test("raw_compare" "raw_compare")

add_executable("raw_run_sync" "${TEST_DIR}/raw_run_sync.cc")
target_link_libraries("raw_run_sync" "cce_core")
add_test("raw_run_sync" "raw_run_sync")

add_executable(
  "raw_run_async"
  "${TEST_DIR}/raw_run_async.cc"
  "${TEST_DIR}/wait_process.hh"
)
target_link_libraries("raw_run_async" "cce_core")
add_test("raw_run_async" "raw_run_async")

add_executable("raw_process" "${TEST_DIR}/raw_process.cc")
target_link_libraries("raw_process" "cce_core")
add_test("raw_process" "raw_process")

add_executable("raw_get" "${TEST_DIR}/raw_get.cc")
target_link_libraries("raw_get" "cce_core")
add_test("raw_get" "raw_get")

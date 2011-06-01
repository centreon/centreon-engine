##
## Copyright 2011 Merethis
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
set(SRC_DIR "${SRC_DIR}/commands")
set(INC_DIR "${INCLUDE_DIR}/commands")

# Produce moc files.
qt4_wrap_cpp(QT_MOC_FILES
  "${INC_DIR}/command.hh"
  "${INC_DIR}/process.hh"
  "${INC_DIR}/raw.hh"
  "${INC_DIR}/set.hh")

# cce_broker target.
add_library("cce_commands"
  STATIC
  "${SRC_DIR}/command.cc"
  "${SRC_DIR}/process.cc"
  "${SRC_DIR}/raw.cc"
  "${SRC_DIR}/result.cc"
  "${SRC_DIR}/set.cc"
  "${INC_DIR}/command.hh"
  "${INC_DIR}/process.hh"
  "${INC_DIR}/raw.hh"
  "${INC_DIR}/result.hh"
  "${INC_DIR}/set.hh"
  ${QT_MOC_FILES})

# Link target with required libraries.
target_link_libraries("cce_commands" ${QT_LIBRARIES})

if (WITH_TESTING)
  #Set directories.
  set(TEST_DIR "${TEST_DIR}/commands")

  # Test result.
  add_executable("result_ctor" "${TEST_DIR}/result_ctor.cc")
  target_link_libraries("result_ctor" "cce_commands" ${QT_LIBRARIES})
  add_test("result_ctor" "result_ctor")

  add_executable("result_get_set" "${TEST_DIR}/result_get_set.cc")
  target_link_libraries("result_get_set" "cce_commands" ${QT_LIBRARIES})
  add_test("result_get_set" "result_get_set")

  add_executable("result_execution_time" "${TEST_DIR}/result_execution_time.cc")
  target_link_libraries("result_execution_time" "cce_commands" ${QT_LIBRARIES})
  add_test("result_execution_time" "result_execution_time")

  add_executable("result_compare" "${TEST_DIR}/result_compare.cc")
  target_link_libraries("result_compare" "cce_commands" ${QT_LIBRARIES})
  add_test("result_compare" "result_compare")

  # Test set.
  add_executable("set_basic" "${TEST_DIR}/set_basic.cc")
  target_link_libraries("set_basic" "cce_commands" "cce_core" ${QT_LIBRARIES})
  add_test("set_basic" "set_basic")

  add_executable("set_name_change" "${TEST_DIR}/set_name_change.cc")
  target_link_libraries("set_name_change" "cce_commands" "cce_core" ${QT_LIBRARIES})
  add_test("set_name_change" "set_name_change")

  # Test binary.
  add_executable("bin_test_run" "${TEST_DIR}/bin_test_run.cc")
  target_link_libraries("bin_test_run" ${QT_LIBRARIES})

  # Test raw.
  add_executable("raw_ctor" "${TEST_DIR}/raw_ctor.cc")
  target_link_libraries("raw_ctor" "cce_commands" "cce_core" ${QT_LIBRARIES})
  add_test("raw_ctor" "raw_ctor")

  add_executable("raw_compare" "${TEST_DIR}/raw_compare.cc")
  target_link_libraries("raw_compare" "cce_commands" "cce_core" ${QT_LIBRARIES})
  add_test("raw_compare" "raw_compare")

  add_executable("raw_run_sync" "${TEST_DIR}/raw_run_sync.cc")
  target_link_libraries("raw_run_sync" "cce_commands" "cce_core" ${QT_LIBRARIES})
  add_test("raw_run_sync" "raw_run_sync")

  qt4_wrap_cpp(QT_MOC_FILES "${TEST_DIR}/wait_process.hh")
  add_executable("raw_run_async"
    "${TEST_DIR}/raw_run_async.cc"
    "${TEST_DIR}/wait_process.hh"
    ${QT_MOC_FILES})
  target_link_libraries("raw_run_async" "cce_commands" "cce_core" ${QT_LIBRARIES})
  add_test("raw_run_async" "raw_run_async")

  add_executable("raw_process" "${TEST_DIR}/raw_process.cc")
  target_link_libraries("raw_process" "cce_commands" "cce_core" ${QT_LIBRARIES})
  add_test("raw_process" "raw_process")

  add_executable("raw_get_set" "${TEST_DIR}/raw_get_set.cc")
  target_link_libraries("raw_get_set" "cce_commands" "cce_core" ${QT_LIBRARIES})
  add_test("raw_get_set" "raw_get_set")
endif ()

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
set(SRC_DIR "${SRC_DIR}/connector")
set(INC_DIR "${INC_DIR}/connector")

# Produce moc files.
qt4_wrap_cpp(QT_MOC_FILES "${INC_DIR}/command.hh")

# cce_connector target.
add_library("cce_connector"
  STATIC
  "${SRC_DIR}/command.cc"
  "${SRC_DIR}/request.cc"
  "${SRC_DIR}/request_builder.cc"
  "${SRC_DIR}/version_query.cc"
  "${SRC_DIR}/version_response.cc"
  "${SRC_DIR}/quit_query.cc"
  "${SRC_DIR}/quit_response.cc"
  "${SRC_DIR}/execute_query.cc"
  "${SRC_DIR}/execute_response.cc"
  "${INC_DIR}/command.hh"
  "${INC_DIR}/request_builder.hh"
  "${INC_DIR}/request.hh"
  "${INC_DIR}/version_query.hh"
  "${INC_DIR}/version_response.hh"
  "${INC_DIR}/quit_query.hh"
  "${INC_DIR}/quit_response.hh"
  "${INC_DIR}/execute_query.hh"
  "${INC_DIR}/execute_response.hh"
  ${QT_MOC_FILES})

# Link target with libraries.
target_link_libraries("cce_connector" "cce_core" ${QT_LIBRARIES})

if (WITH_TESTING)
  # Set directories.
  set(TEST_DIR "${TEST_DIR}/commands/connector")

  # Produce moc files.
  qt4_wrap_cpp(QT_MOC_FILES "${TEST_DIR}/../wait_process.hh")

  # Test binary.
  add_executable("bin_connector_test_run" "${TEST_DIR}/bin_connector_test_run.cc")
  target_link_libraries("bin_connector_test_run" "cce_commands" ${QT_LIBRARIES})

  # Test connector.
  add_executable("connector_ctor" "${TEST_DIR}/connector_ctor.cc")
  target_link_libraries("connector_ctor" "cce_commands" ${QT_LIBRARIES})
  add_test("connector_ctor" "connector_ctor")

  add_executable("connector_compare" "${TEST_DIR}/connector_compare.cc")
  target_link_libraries("connector_compare" "cce_commands" ${QT_LIBRARIES})
  add_test("connector_compare" "connector_compare")

  add_executable("connector_run_sync" "${TEST_DIR}/connector_run_sync.cc")
  target_link_libraries("connector_run_sync" "cce_commands" ${QT_LIBRARIES})
  add_test("connector_run_sync" "connector_run_sync")

  add_executable("connector_run_async"
    "${TEST_DIR}/connector_run_async.cc"
    "${TEST_DIR}/../wait_process.hh"
    ${QT_MOC_FILES})
  target_link_libraries("connector_run_async" "cce_commands" ${QT_LIBRARIES})
  add_test("connector_run_async" "connector_run_async")

  add_executable("connector_get" "${TEST_DIR}/connector_get.cc")
  target_link_libraries("connector_get" "cce_commands" ${QT_LIBRARIES})
  add_test("connector_get" "connector_get")

  add_executable("request_quit" "${TEST_DIR}/request_quit.cc")
  target_link_libraries("request_quit" "cce_commands" ${QT_LIBRARIES})
  add_test("request_quit" "request_quit")

  add_executable("request_version" "${TEST_DIR}/request_version.cc")
  target_link_libraries("request_version" "cce_commands" ${QT_LIBRARIES})
  add_test("request_version" "request_version")

  add_executable("request_execute" "${TEST_DIR}/request_execute.cc")
  target_link_libraries("request_execute" "cce_commands" ${QT_LIBRARIES})
  add_test("request_execute" "request_execute")

  add_executable("request_builder" "${TEST_DIR}/request_builder.cc")
  target_link_libraries("request_builder" "cce_commands" ${QT_LIBRARIES})
  add_test("request_builder" "request_builder")
endif ()
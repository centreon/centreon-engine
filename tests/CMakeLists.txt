##
## Copyright 2016 Centreon
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

# Enable unit tests or not.
option(WITH_TESTING "Build unit tests." OFF)
if (WITH_TESTING)
  include(ExternalProject)

  set(GOOGLE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
  if (BUILD_OFFLINE)
  ExternalProject_Add( googletest
                       SOURCE_DIR ${CMAKE_BINARY_DIR}/tests/googletest-prefix/src/googletest
                       TIMEOUT 10
                       UPDATE_COMMAND ""
                       INSTALL_COMMAND ""
                       LOG_DOWNLOAD ON
                       LOG_CONFIGURE ON
                       LOG_BUILD ON
                       CMAKE_ARGS -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS})
  else ()
  ExternalProject_Add( googletest
                       GIT_REPOSITORY https://github.com/google/googletest/
                       TIMEOUT 10
                       INSTALL_COMMAND ""
                       LOG_DOWNLOAD ON
                       LOG_CONFIGURE ON
                       LOG_BUILD ON
                       CMAKE_ARGS -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS})
  endif ()

  ExternalProject_Get_Property(googletest source_dir)
  ExternalProject_Get_Property(googletest binary_dir)

  link_directories(${binary_dir}/lib)
  include_directories(${source_dir}/googletest/include)

# Tests directory.
  # Add root inclusion direction.
  set(MODULE_DIR "${PROJECT_SOURCE_DIR}/modules/external_commands")
  set(INC_DIR "${MODULE_DIR}/inc/com/centreon/engine/modules/external_commands")
  include_directories(${PROJECT_SOURCE_DIR} ${MODULE_DIR}/inc)

  # Set directory.
  set(TESTS_DIR "${PROJECT_SOURCE_DIR}/tests")
  if (WITH_GRPC)
    include_directories(${PROJECT_SOURCE_DIR}/enginerpc)
    add_definitions(-DENGINERPC_TESTS_PATH="${TESTS_DIR}/enginerpc")

    add_executable(rpc_client
        ${TESTS_DIR}/enginerpc/client.cc)

    target_link_libraries(rpc_client "${GRPC_STATIC_LDFLAGS}" cerpc)
  endif ()

  add_executable("bin_connector_test_run"
      "${TESTS_DIR}/commands/bin_connector_test_run.cc")
  target_link_libraries(bin_connector_test_run cce_core pthread)

  set(ut_sources
    # Sources.
    "${PROJECT_SOURCE_DIR}/modules/external_commands/src/commands.cc"
    "${PROJECT_SOURCE_DIR}/modules/external_commands/src/internal.cc"
    "${PROJECT_SOURCE_DIR}/modules/external_commands/src/processing.cc"
    "${TESTS_DIR}/parse-check-output.cc"
    "${TESTS_DIR}/commands/simple-command.cc"
    "${TESTS_DIR}/commands/connector.cc"
    "${TESTS_DIR}/configuration/applier/applier-command.cc"
    "${TESTS_DIR}/configuration/applier/applier-connector.cc"
    "${TESTS_DIR}/configuration/applier/applier-contact.cc"
    "${TESTS_DIR}/configuration/applier/applier-contactgroup.cc"
    "${TESTS_DIR}/configuration/applier/applier-global.cc"
    "${TESTS_DIR}/configuration/applier/applier-host.cc"
    "${TESTS_DIR}/configuration/applier/applier-hostescalation.cc"
    "${TESTS_DIR}/configuration/applier/applier-hostdependency.cc"
    "${TESTS_DIR}/configuration/applier/applier-hostgroup.cc"
    "${TESTS_DIR}/configuration/applier/applier-service.cc"
    "${TESTS_DIR}/configuration/applier/applier-serviceescalation.cc"
    "${TESTS_DIR}/configuration/applier/applier-servicegroup.cc"
    "${TESTS_DIR}/configuration/contact.cc"
    "${TESTS_DIR}/configuration/host.cc"
    "${TESTS_DIR}/configuration/object.cc"
    "${TESTS_DIR}/configuration/service.cc"
    "${TESTS_DIR}/contacts/contactgroup-config.cc"
    "${TESTS_DIR}/contacts/simple-contactgroup.cc"
    "${TESTS_DIR}/custom_vars/extcmd.cc"
    "${TESTS_DIR}/downtimes/downtime.cc"
    "${TESTS_DIR}/downtimes/downtime_finder.cc"
    "${TESTS_DIR}/macros/macro.cc"
    "${TESTS_DIR}/macros/url_encode.cc"
    "${TESTS_DIR}/external_commands/host.cc"
    "${TESTS_DIR}/external_commands/service.cc"
    "${TESTS_DIR}/main.cc"
    "${TESTS_DIR}/notifications/host_downtime_notification.cc"
    "${TESTS_DIR}/notifications/host_flapping_notification.cc"
    "${TESTS_DIR}/notifications/host_normal_notification.cc"
    "${TESTS_DIR}/notifications/host_recovery_notification.cc"
    "${TESTS_DIR}/notifications/service_normal_notification.cc"
    "${TESTS_DIR}/notifications/service_flapping_notification.cc"
    "${TESTS_DIR}/perfdata/perfdata.cc"
    "${TESTS_DIR}/retention/host.cc"
    "${TESTS_DIR}/retention/service.cc"
    "${TESTS_DIR}/test_engine.cc"
    "${TESTS_DIR}/timeperiod/get_next_valid_time/between_two_years.cc"
    "${TESTS_DIR}/timeperiod/get_next_valid_time/calendar_date.cc"
    "${TESTS_DIR}/timeperiod/get_next_valid_time/dst_backward.cc"
    "${TESTS_DIR}/timeperiod/get_next_valid_time/dst_forward.cc"
    "${TESTS_DIR}/timeperiod/get_next_valid_time/earliest_daterange_first.cc"
    "${TESTS_DIR}/timeperiod/get_next_valid_time/exclusion.cc"
    "${TESTS_DIR}/timeperiod/get_next_valid_time/generic_month_date.cc"
    "${TESTS_DIR}/timeperiod/get_next_valid_time/normal_weekday.cc"
    "${TESTS_DIR}/timeperiod/get_next_valid_time/offset_weekday_of_generic_month.cc"
    "${TESTS_DIR}/timeperiod/get_next_valid_time/offset_weekday_of_specific_month.cc"
    "${TESTS_DIR}/timeperiod/get_next_valid_time/precedence.cc"
    "${TESTS_DIR}/timeperiod/get_next_valid_time/skip_interval.cc"
    "${TESTS_DIR}/timeperiod/get_next_valid_time/specific_month_date.cc"
    "${TESTS_DIR}/timeperiod/utils.cc"
    # Headers.
    "${TESTS_DIR}/test_engine.hh"
    "${TESTS_DIR}/timeperiod/utils.hh"
  )

  # Unit test executable.

  if (WITH_GRPC)
    add_executable(ut ${ut_sources}
    "${TESTS_DIR}/enginerpc/enginerpc.cc")
  else ()
    add_executable(ut ${ut_sources})
  endif ()

  add_dependencies(ut googletest)
  add_test(NAME tests COMMAND ut)

  if (WITH_COVERAGE)
    set(COVERAGE_EXCLUDES '${PROJECT_BINARY_DIR}/*' '${PROJECT_SOURCE_DIR}/tests/*' '/usr/include/* ')
    SETUP_TARGET_FOR_COVERAGE(
        NAME test-coverage
        EXECUTABLE ut
        DEPENDENCIES ut
    )
    set(GCOV gcov)
  endif ()
  target_link_libraries(ut gtest ${ENGINERPC} cce_core pthread ${GCOV})

endif ()

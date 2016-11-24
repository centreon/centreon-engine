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
  # Add root inclusion direction.
  include_directories("${PROJECT_SOURCE_DIR}")

  # Set directory.
  set(TESTS_DIR "${PROJECT_SOURCE_DIR}/tests")
  add_subdirectory("googletest")

  # Unit test executable.
  add_executable("ut"
    # Sources.
    "${TESTS_DIR}/configuration/host.cc"
    "${TESTS_DIR}/configuration/object.cc"
    "${TESTS_DIR}/configuration/service.cc"
    "${TESTS_DIR}/downtime_finder.cc"
    "${TESTS_DIR}/main.cc"
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
    "${TESTS_DIR}/timeperiod/utils.hh"
  )
  target_link_libraries("ut" "gtest" "cce_core")
endif ()

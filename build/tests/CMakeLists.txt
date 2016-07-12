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
  # Set directory.
  set(TESTS_DIR "${PROJECT_SOURCE_DIR}/tests")
  add_subdirectory("googletest")

  # Unit test executable.
  add_executable("ut"
    "${TESTS_DIR}/configuration/host.cc"
    "${TESTS_DIR}/configuration/service.cc"
    "${TESTS_DIR}/downtime_finder.cc"
  )
  target_link_libraries("ut" "gtest" "gtest_main" "cce_core")
endif ()

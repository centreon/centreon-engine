##
## Copyright 2011-2015 Merethis
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
set(TEST_DIR "${PROJECT_SOURCE_DIR}/test")

include_directories("${PROJECT_SOURCE_DIR}")
configure_file("${TEST_DIR}/paths.hh.in" "${TEST_DIR}/paths.hh")

# Subdirectory.
add_subdirectory("bench")
option(WITH_TESTING "Build unit tests." OFF)
if (WITH_TESTING)
  set(CTEST_TESTING_TIMEOUT 20)
  include(CTest)
  add_subdirectory("broker")
  add_subdirectory("checks")
  add_subdirectory("commands")
  add_subdirectory("compatibility")
  add_subdirectory("configuration")
  add_subdirectory("deleter")
  add_subdirectory("events")
  add_subdirectory("logging")
  add_subdirectory("macros")
  add_subdirectory("modules")
  add_subdirectory("objects")
  add_subdirectory("retention")
  add_subdirectory("core")
endif ()

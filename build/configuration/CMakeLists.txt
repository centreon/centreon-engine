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
set(MY_SRC_DIR "${SRC_DIR}/configuration")
set(MY_INC_DIR "${INCLUDE_DIR}/configuration")
set(MY_TEST_DIR "${TEST_DIR}/configuration")

# cce_configuration target.
add_library("cce_configuration"
  "STATIC"
  "${MY_SRC_DIR}/"
  "${MY_SRC_DIR}/state.cc"
  "${MY_INC_DIR}/state.hh")
get_property(MY_COMPILE_FLAGS
  TARGET "cce_configuration"
  PROPERTY COMPILE_FLAGS)
set(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
set_target_properties("cce_configuration"
  PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
target_link_libraries("cce_configuration"
  ${QT_LIBRARIES}
  "centenginelib")

# Test configuration.
add_executable("test_configuration"
  "${MY_TEST_DIR}/test_configuration.cc")
target_link_libraries("test_configuration"
  ${QT_LIBRARIES}
  "centenginelib")
add_test("test_configuration_basic"
  "test_configuration"
  "test.cfg")

# Test parse argument.
add_executable("test_configuration_parse_argument"
  "${MY_TEST_DIR}/test_parse_argument.cc")
target_link_libraries("test_configuration_parse_argument"
  ${QT_LIBRARIES}
  "centenginelib")
add_test("test_configuration_parse_argument"
  "test_configuration_parse_argument")


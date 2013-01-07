##
## Copyright 2011-2013 Merethis
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
set(SRC_DIR "${SRC_DIR}/configuration")
set(INC_DIR "${INC_DIR}/com/centreon/engine/configuration")

# Subdirectory.
add_subdirectory("applier")

# cce_configuration target.
add_library("cce_configuration"
  STATIC
  "${SRC_DIR}/state.cc"
  "${INC_DIR}/state.hh")

# Link target with subdirectory's library.
target_link_libraries("cce_configuration"
  "cce_configuration_applier"
  "cce_core")

if (WITH_TESTING)
  # Set directories.
  set(TEST_DIR "${TEST_DIR}/configuration")

  # Basic test.
  set(TEST_NAME "configuration_basic")
  add_executable("${TEST_NAME}" "${TEST_DIR}/basic.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}" "test.cfg")

  # Parsing test.
  set(TEST_NAME "configuration_parsing")
  add_executable("${TEST_NAME}" "${TEST_DIR}/parsing.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
endif ()

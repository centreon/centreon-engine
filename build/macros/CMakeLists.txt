##
## Copyright 2011-2012 Merethis
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
set(SRC_DIR "${SRC_DIR}/macros")
set(INC_DIR "${INC_DIR}/com/centreon/engine/macros")

# cce_macros target.
add_library("cce_macros"
  STATIC
  "${SRC_DIR}/clear_host.cc"
  "${SRC_DIR}/clear_hostgroup.cc"
  "${SRC_DIR}/clear_service.cc"
  "${SRC_DIR}/clear_servicegroup.cc"
  "${SRC_DIR}/grab_host.cc"
  "${SRC_DIR}/grab_service.cc"
  "${SRC_DIR}/grab_value.cc"
  "${SRC_DIR}/misc.cc"
  "${SRC_DIR}/process.cc"
  "${INC_DIR}/defines.hh"
  "${INC_DIR}/clear_host.hh"
  "${INC_DIR}/clear_hostgroup.hh"
  "${INC_DIR}/clear_service.hh"
  "${INC_DIR}/clear_servicegroup.hh"
  "${INC_DIR}/grab.hh"
  "${INC_DIR}/grab_host.hh"
  "${INC_DIR}/grab_service.hh"
  "${INC_DIR}/grab_value.hh"
  "${INC_DIR}/misc.hh"
  "${INC_DIR}/process.hh")

# Link target with necessary libraries.
target_link_libraries("cce_macros" ${QT_LIBRARIES} "cce_core")

if (WITH_TESTING)
  # Set directories.
  set(TEST_DIR "${TEST_DIR}/macros")

  # Grab standard host macro.
  set(TEST_NAME "macros_grab_standard_host_macro")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/grab_standard_host_macro.cc"
    "${TEST_DIR}/minimal_setup.cc"
    "${TEST_DIR}/minimal_setup.hh")
  target_link_libraries("${TEST_NAME}" "cce_macros")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Grab standard service macro.
  set(TEST_NAME "macros_grab_standard_service_macro")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/grab_standard_service_macro.cc"
    "${TEST_DIR}/minimal_setup.cc"
    "${TEST_DIR}/minimal_setup.hh")
  target_link_libraries("${TEST_NAME}" "cce_macros")
  add_test("${TEST_NAME}" "${TEST_NAME}")
endif ()

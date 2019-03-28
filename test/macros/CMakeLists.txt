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
set(TEST_DIR "${TEST_DIR}/macros")

# Grab standard host macro.
set(TEST_NAME "macros_grab_standard_host_macro")
add_executable(
  "${TEST_NAME}"
  "${TEST_DIR}/grab_standard_host_macro.cc"
  "${TEST_DIR}/minimal_setup.cc"
  "${TEST_DIR}/minimal_setup.hh"
)
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")
# Grab standard service macro.
set(TEST_NAME "macros_grab_standard_service_macro")
add_executable(
  "${TEST_NAME}"
  "${TEST_DIR}/grab_standard_service_macro.cc"
  "${TEST_DIR}/minimal_setup.cc"
  "${TEST_DIR}/minimal_setup.hh"
)
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

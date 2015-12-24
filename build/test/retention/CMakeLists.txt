##
## Copyright 2011-2013,2015 Merethis
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
set(TEST_DIR "${TEST_DIR}/retention")

# Subdirectory.
add_subdirectory("applier")

## host.
set(TEST_NAME "retention_host")
add_executable("${TEST_NAME}" "${TEST_DIR}/host.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test(NAME "${TEST_NAME}" COMMAND "${TEST_NAME}")

## info.
set(TEST_NAME "retention_info")
add_executable("${TEST_NAME}" "${TEST_DIR}/info.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test(NAME "${TEST_NAME}" COMMAND "${TEST_NAME}")

## program.
set(TEST_NAME "retention_program")
add_executable("${TEST_NAME}" "${TEST_DIR}/program.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test(NAME "${TEST_NAME}" COMMAND "${TEST_NAME}")

## service.
set(TEST_NAME "retention_service")
add_executable("${TEST_NAME}" "${TEST_DIR}/service.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test(NAME "${TEST_NAME}" COMMAND "${TEST_NAME}")

## retention_dump.
set(TEST_BIN_NAME "retention_dump")
add_executable("${TEST_BIN_NAME}" "${TEST_DIR}/dump.cc")
target_link_libraries("${TEST_BIN_NAME}" "cce_core")

# header_dump
set(TEST_NAME "retention_header_dump")
add_test(NAME "${TEST_NAME}" COMMAND "${TEST_BIN_NAME}" "header")

# host_dump
set(TEST_NAME "retention_host_dump")
add_test(NAME "${TEST_NAME}" COMMAND "${TEST_BIN_NAME}" "host")

# info_dump
set(TEST_NAME "retention_info_dump")
add_test(NAME "${TEST_NAME}" COMMAND "${TEST_BIN_NAME}" "info")

# program_dump
set(TEST_NAME "retention_program_dump")
add_test(NAME "${TEST_NAME}" COMMAND "${TEST_BIN_NAME}" "program")

# service_dump
set(TEST_NAME "retention_service_dump")
add_test(NAME "${TEST_NAME}" COMMAND "${TEST_BIN_NAME}" "service")

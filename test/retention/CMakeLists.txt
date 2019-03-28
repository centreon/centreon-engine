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
set(TEST_DIR "${TEST_DIR}/retention")

## comment.
set(TEST_NAME "comment")
add_executable("${TEST_NAME}" "${TEST_DIR}/comment.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

## contact.
set(TEST_NAME "contact")
add_executable("${TEST_NAME}" "${TEST_DIR}/contact.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

## downtime.
set(TEST_NAME "downtime")
add_executable("${TEST_NAME}" "${TEST_DIR}/downtime.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

## host.
set(TEST_NAME "host")
add_executable("${TEST_NAME}" "${TEST_DIR}/host.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

## info.
set(TEST_NAME "info")
add_executable("${TEST_NAME}" "${TEST_DIR}/info.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

## program.
set(TEST_NAME "program")
add_executable("${TEST_NAME}" "${TEST_DIR}/program.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

## service.
set(TEST_NAME "service")
add_executable("${TEST_NAME}" "${TEST_DIR}/service.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

## retention_dump.
set(TEST_BIN_NAME "retention_dump")
add_executable("${TEST_BIN_NAME}" "${TEST_DIR}/dump.cc")
target_link_libraries("${TEST_BIN_NAME}" "cce_core")

# comment_dump
set(TEST_NAME "comment_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "comment")

# contact_dump
set(TEST_NAME "contact_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "contact")

# downtime_dump
set(TEST_NAME "downtime_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "downtime")

# header_dump
set(TEST_NAME "header_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "header")

# host_dump
set(TEST_NAME "host_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "host")

# info_dump
set(TEST_NAME "info_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "info")

# program_dump
set(TEST_NAME "program_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "program")

# service_dump
set(TEST_NAME "service_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "service")

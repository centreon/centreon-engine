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
set(TEST_DIR "${TEST_DIR}/objects")

## object_dump.
set(TEST_BIN_NAME "object_dump")
add_executable("${TEST_BIN_NAME}" "${TEST_DIR}/dump.cc")
target_link_libraries("${TEST_BIN_NAME}" "cce_core")

# command_dump
set(TEST_NAME "command_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "command")

# commandsmember_dump
set(TEST_NAME "commandsmember_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "commandsmember")

# comment_dump
set(TEST_NAME "comment_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "comment")

# contactgroup_dump
set(TEST_NAME "contactgroup_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "contactgroup")

# contactgroupsmember_dump
set(TEST_NAME "contactgroupsmember_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "contactgroupsmember")

# contact_dump
set(TEST_NAME "contact_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "contact")

# customvariablesmember_dump
set(TEST_NAME "customvariablesmember_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "customvariablesmember")

# contactsmember_dump
set(TEST_NAME "contactsmember_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "contactsmember")

# daterange_dump
set(TEST_NAME "daterange_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "daterange")

# downtime_dump
set(TEST_NAME "downtime_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "downtime")

# hostdependency_dump
set(TEST_NAME "hostdependency_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "hostdependency")

# hostescalation_dump
set(TEST_NAME "hostescalation_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "hostescalation")

# hostgroup_dump
set(TEST_NAME "hostgroup_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "hostgroup")

# host_dump
set(TEST_NAME "host_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "host")

# hostsmember_dump
set(TEST_NAME "hostsmember_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "hostsmember")

# servicedependency_dump
set(TEST_NAME "servicedependency_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "servicedependency")

# serviceescalation_dump
set(TEST_NAME "serviceescalation_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "serviceescalation")

# servicegroup_dump
set(TEST_NAME "servicegroup_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "servicegroup")

# service_dump
set(TEST_NAME "service_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "service")

# servicesmember_dump
set(TEST_NAME "servicesmember_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "servicesmember")

# timeperiod_dump
set(TEST_NAME "timeperiod_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "timeperiod")

# timeperiodexclusion_dump
set(TEST_NAME "timeperiodexclusion_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "timeperiodexclusion")

# timerange_dump
set(TEST_NAME "timerange_dump")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "timerange")



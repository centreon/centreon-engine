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
set(TEST_DIR "${TEST_DIR}/configuration")

# Subdirectory.
add_subdirectory("applier")

# # Basic test.
# set(TEST_NAME "configuration_basic")
# add_executable("${TEST_NAME}" "${TEST_DIR}/basic.cc")
# target_link_libraries("${TEST_NAME}" "cce_core")
# add_test("${TEST_NAME}" "${TEST_NAME}" "test.cfg")

# # Parsing test.
# set(TEST_NAME "configuration_parsing")
# add_executable("${TEST_NAME}" "${TEST_DIR}/parsing.cc")
# target_link_libraries("${TEST_NAME}" "cce_core")
# add_test("${TEST_NAME}" "${TEST_NAME}")

# Objects copy.
set(TEST_BIN_NAME "configuration_objects_copy")
add_executable("${TEST_BIN_NAME}" "${TEST_DIR}/objects_copy.cc")
target_link_libraries("${TEST_BIN_NAME}" "cce_core")

## check object copy configuration path.
set(CONF_DIR "${TEST_DIR}/etc")

# parse_and_copy_configuration_command
set(TEST_NAME "parse_and_copy_configuration_command")
set(TEST_CONF_FILE "command.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_copy_configuration_contactgroup
set(TEST_NAME "parse_and_copy_configuration_contactgroup")
set(TEST_CONF_FILE "contactgroup.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_copy_configuration_contact
set(TEST_NAME "parse_and_copy_configuration_contact")
set(TEST_CONF_FILE "contact.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_copy_configuration_hostdependency
set(TEST_NAME "parse_and_copy_configuration_hostdependency")
set(TEST_CONF_FILE "hostdependency.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_copy_configuration_hostescalation
set(TEST_NAME "parse_and_copy_configuration_hostescalation")
set(TEST_CONF_FILE "hostescalation.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_copy_configuration_hostgroup
set(TEST_NAME "parse_and_copy_configuration_hostgroup")
set(TEST_CONF_FILE "hostgroup.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_copy_configuration_host
set(TEST_NAME "parse_and_copy_configuration_host")
set(TEST_CONF_FILE "host.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_copy_configuration_servicedependency
set(TEST_NAME "parse_and_copy_configuration_servicedependency")
set(TEST_CONF_FILE "servicedependency.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_copy_configuration_serviceescalation
set(TEST_NAME "parse_and_copy_configuration_serviceescalation")
set(TEST_CONF_FILE "serviceescalation.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_copy_configuration_servicegroup
set(TEST_NAME "parse_and_copy_configuration_servicegroup")
set(TEST_CONF_FILE "servicegroup.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_copy_configuration_service
set(TEST_NAME "parse_and_copy_configuration_service")
set(TEST_CONF_FILE "service.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_copy_configuration_timeperiod
set(TEST_NAME "parse_and_copy_configuration_timeperiod")
set(TEST_CONF_FILE "timeperiod.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

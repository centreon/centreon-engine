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
set(TEST_DIR "${TEST_DIR}/configuration")

# Subdirectory.
add_subdirectory("applier")

# cce_configuration target.
add_library("cce_configuration"
  STATIC
  "${SRC_DIR}/command.cc"
  "${SRC_DIR}/connector.cc"
  "${SRC_DIR}/contact.cc"
  "${SRC_DIR}/contactgroup.cc"
  "${SRC_DIR}/daterange.cc"
  "${SRC_DIR}/group.cc"
  "${SRC_DIR}/host.cc"
  "${SRC_DIR}/hostdependency.cc"
  "${SRC_DIR}/hostescalation.cc"
  "${SRC_DIR}/hostextinfo.cc"
  "${SRC_DIR}/hostgroup.cc"
  "${SRC_DIR}/object.cc"
  "${SRC_DIR}/parser.cc"
  "${SRC_DIR}/point_2d.cc"
  "${SRC_DIR}/point_3d.cc"
  "${SRC_DIR}/reload.cc"
  "${SRC_DIR}/state.cc"
  "${SRC_DIR}/service.cc"
  "${SRC_DIR}/servicedependency.cc"
  "${SRC_DIR}/serviceescalation.cc"
  "${SRC_DIR}/serviceextinfo.cc"
  "${SRC_DIR}/servicegroup.cc"
  "${SRC_DIR}/timeperiod.cc"
  "${SRC_DIR}/timerange.cc"

  "${INC_DIR}/command.hh"
  "${INC_DIR}/connector.hh"
  "${INC_DIR}/contactgroup.hh"
  "${INC_DIR}/contact.hh"
  "${INC_DIR}/daterange.hh"
  "${INC_DIR}/file_info.hh"
  "${INC_DIR}/group.hh"
  "${INC_DIR}/host.hh"
  "${INC_DIR}/hostdependency.hh"
  "${INC_DIR}/hostescalation.hh"
  "${INC_DIR}/hostextinfo.hh"
  "${INC_DIR}/hostgroup.hh"
  "${INC_DIR}/object.hh"
  "${INC_DIR}/parser.hh"
  "${INC_DIR}/point_2d.hh"
  "${INC_DIR}/point_3d.hh"
  "${INC_DIR}/reload.hh"
  "${INC_DIR}/servicedependency.hh"
  "${INC_DIR}/serviceescalation.hh"
  "${INC_DIR}/serviceextinfo.hh"
  "${INC_DIR}/servicegroup.hh"
  "${INC_DIR}/service.hh"
  "${INC_DIR}/state.hh"
  "${INC_DIR}/timeperiod.hh"
  "${INC_DIR}/timerange.hh")



# Link target with subdirectory's library.
target_link_libraries("cce_configuration"
  "cce_configuration_applier"
  "cce_core")

if (WITH_TESTING)
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

endif ()

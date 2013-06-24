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
set(SRC_DIR "${SRC_DIR}/applier")
set(INC_DIR "${INC_DIR}/applier")
set(TEST_DIR "${TEST_DIR}/applier")

# cce_configuration_applier target.
add_library("cce_configuration_applier"
  STATIC
  "${SRC_DIR}/base.cc"
  "${SRC_DIR}/command.cc"
  "${SRC_DIR}/connector.cc"
  "${SRC_DIR}/contact.cc"
  "${SRC_DIR}/contactgroup.cc"
  "${SRC_DIR}/customvariable.cc"
  "${SRC_DIR}/host.cc"
  "${SRC_DIR}/hostdependency.cc"
  "${SRC_DIR}/hostescalation.cc"
  "${SRC_DIR}/hostgroup.cc"
  "${SRC_DIR}/globals.cc"
  "${SRC_DIR}/logging.cc"
  "${SRC_DIR}/macros.cc"
  "${SRC_DIR}/member.cc"
  "${SRC_DIR}/object.cc"
  "${SRC_DIR}/service.cc"
  "${SRC_DIR}/servicedependency.cc"
  "${SRC_DIR}/serviceescalation.cc"
  "${SRC_DIR}/servicegroup.cc"
  "${SRC_DIR}/state.cc"
  "${SRC_DIR}/timeperiod.cc"

  "${INC_DIR}/base.hh"
  "${INC_DIR}/command.hh"
  "${INC_DIR}/connector.hh"
  "${INC_DIR}/contact.hh"
  "${INC_DIR}/contactgroup.hh"
  "${INC_DIR}/customvariable.hh"
  "${INC_DIR}/difference.hh"
  "${INC_DIR}/host.hh"
  "${INC_DIR}/hostdependency.hh"
  "${INC_DIR}/hostescalation.hh"
  "${INC_DIR}/hostgroup.hh"
  "${INC_DIR}/globals.hh"
  "${INC_DIR}/logging.hh"
  "${INC_DIR}/macros.hh"
  "${INC_DIR}/member.hh"
  "${INC_DIR}/object.hh"
  "${INC_DIR}/service.hh"
  "${INC_DIR}/servicedependency.hh"
  "${INC_DIR}/serviceescalation.hh"
  "${INC_DIR}/servicegroup.hh"
  "${INC_DIR}/state.hh"
  "${INC_DIR}/timeperiod.hh")

# Link target with required libraries.
target_link_libraries("cce_configuration_applier" "cce_logging")

if (WITH_TESTING)
  # check compatibility.
  set(TEST_BIN_NAME "check_compatibility")
  add_executable(
    "${TEST_BIN_NAME}"
    "${TEST_DIR}/check_compatibility.cc"
    "${TEST_DIR}/xodtemplate.cc")
  target_link_libraries("${TEST_BIN_NAME}" "cce_core")

  ## check compatibility configuration path.
  set(CONF_DIR "${TEST_DIR}/configuration/")

  # parse_configuration_command
  set(TEST_NAME "parse_configuration_command")
  set(TEST_CONF_FILE "main_command.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_comment
  set(TEST_NAME "parse_configuration_comment")
  set(TEST_CONF_FILE "main_comment.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_connector
  set(TEST_NAME "parse_configuration_connector")
  set(TEST_CONF_FILE "main_connector.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_contact
  set(TEST_NAME "parse_configuration_contact")
  set(TEST_CONF_FILE "main_contact.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_contact_template
  set(TEST_NAME "parse_configuration_contact_template")
  set(TEST_CONF_FILE "main_contact_template.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_globals
  set(TEST_NAME "parse_configuration_globals")
  set(TEST_CONF_FILE "main_globals.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_host
  set(TEST_NAME "parse_configuration_host")
  set(TEST_CONF_FILE "main_host.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_host_template
  set(TEST_NAME "parse_configuration_host_template")
  set(TEST_CONF_FILE "main_host_template.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_hostdependency
  set(TEST_NAME "parse_configuration_hostdependency")
  set(TEST_CONF_FILE "main_hostdependency.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_hostdependency_template
  set(TEST_NAME "parse_configuration_hostdependency_template")
  set(TEST_CONF_FILE "main_hostdependency_template.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_hostextinfo
  set(TEST_NAME "parse_configuration_hostextinfo")
  set(TEST_CONF_FILE "main_hostextinfo.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_service
  set(TEST_NAME "parse_configuration_service")
  set(TEST_CONF_FILE "main_service.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_service_template
  set(TEST_NAME "parse_configuration_service_template")
  set(TEST_CONF_FILE "main_service_template.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_servicedependency
  set(TEST_NAME "parse_configuration_servicedependency")
  set(TEST_CONF_FILE "main_servicedependency.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_servicedependency_template
  set(TEST_NAME "parse_configuration_servicedependency_template")
  set(TEST_CONF_FILE "main_servicedependency_template.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_serviceextinfo
  set(TEST_NAME "parse_configuration_serviceextinfo")
  set(TEST_CONF_FILE "main_serviceextinfo.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_timeperiod
  set(TEST_NAME "parse_configuration_timeperiod")
  set(TEST_CONF_FILE "main_timeperiod.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

  # parse_configuration_timeperiod_template
  set(TEST_NAME "parse_configuration_timeperiod_template")
  set(TEST_CONF_FILE "main_timeperiod_template.cfg")
  add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")
endif ()

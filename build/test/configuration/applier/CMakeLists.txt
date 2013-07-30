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
set(TEST_DIR "${TEST_DIR}/applier")

## check compatibility configuration path.
set(CONF_DIR "${TEST_DIR}/etc")

# parse_and_apply_configuration_resource
set(TEST_NAME "parse_and_apply_configuration_resource")
add_executable("${TEST_NAME}" "${TEST_DIR}/resource.cc")
target_link_libraries("${TEST_NAME}" "cce_core")
add_test("${TEST_NAME}" "${TEST_NAME}")

## apply configuration and check compatibility.

# check compatibility.
set(TEST_BIN_NAME "check_compatibility")
add_executable(
  "${TEST_BIN_NAME}"
  "${TEST_DIR}/check_compatibility.cc"
  "${TEST_DIR}/xodtemplate.cc"
)
target_link_libraries("${TEST_BIN_NAME}" "cce_core")

# parse_and_apply_configuration_command
set(TEST_NAME "parse_and_apply_configuration_command")
set(TEST_CONF_FILE "main_command.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_comment
set(TEST_NAME "parse_and_apply_configuration_comment")
set(TEST_CONF_FILE "main_comment.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_connector
set(TEST_NAME "parse_and_apply_configuration_connector")
set(TEST_CONF_FILE "main_connector.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_contact
set(TEST_NAME "parse_and_apply_configuration_contact")
set(TEST_CONF_FILE "main_contact.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_contact_template
set(TEST_NAME "parse_and_apply_configuration_contact_template")
set(TEST_CONF_FILE "main_contact_template.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_globals
set(TEST_NAME "parse_and_apply_configuration_globals")
set(TEST_CONF_FILE "main_globals.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_host
set(TEST_NAME "parse_and_apply_configuration_host")
set(TEST_CONF_FILE "main_host.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_host_template
set(TEST_NAME "parse_and_apply_configuration_host_template")
set(TEST_CONF_FILE "main_host_template.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_hostdependency
set(TEST_NAME "parse_and_apply_configuration_hostdependency")
set(TEST_CONF_FILE "main_hostdependency.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_hostdependency_template
set(TEST_NAME "parse_and_apply_configuration_hostdependency_template")
set(TEST_CONF_FILE "main_hostdependency_template.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_hostescalation
set(TEST_NAME "parse_and_apply_configuration_hostescalation")
set(TEST_CONF_FILE "main_hostescalation.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_hostescalation_template
set(TEST_NAME "parse_and_apply_configuration_hostescalation_template")
set(TEST_CONF_FILE "main_hostescalation_template.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_hostextinfo
set(TEST_NAME "parse_and_apply_configuration_hostextinfo")
set(TEST_CONF_FILE "main_hostextinfo.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_service
set(TEST_NAME "parse_and_apply_configuration_service")
set(TEST_CONF_FILE "main_service.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_service_template
set(TEST_NAME "parse_and_apply_configuration_service_template")
set(TEST_CONF_FILE "main_service_template.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_servicedependency
set(TEST_NAME "parse_and_apply_configuration_servicedependency")
set(TEST_CONF_FILE "main_servicedependency.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_servicedependency_template
set(TEST_NAME "parse_and_apply_configuration_servicedependency_template")
set(TEST_CONF_FILE "main_servicedependency_template.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_serviceescalation
set(TEST_NAME "parse_and_apply_configuration_serviceescalation")
set(TEST_CONF_FILE "main_serviceescalation.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_serviceescalation_template
set(TEST_NAME "parse_and_apply_configuration_serviceescalation_template")
set(TEST_CONF_FILE "main_serviceescalation_template.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_serviceextinfo
set(TEST_NAME "parse_and_apply_configuration_serviceextinfo")
set(TEST_CONF_FILE "main_serviceextinfo.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_timeperiod
set(TEST_NAME "parse_and_apply_configuration_timeperiod")
set(TEST_CONF_FILE "main_timeperiod.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_apply_configuration_timeperiod_template
set(TEST_NAME "parse_and_apply_configuration_timeperiod_template")
set(TEST_CONF_FILE "main_timeperiod_template.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

## parse configuration and remove objects.

# check remove objects.
set(TEST_BIN_NAME "remove_objects")
add_executable(
  "${TEST_BIN_NAME}"
  "${TEST_DIR}/remove_objects.cc"
)
target_link_libraries("${TEST_BIN_NAME}" "cce_core")

# parse_and_remove_configuration_command
set(TEST_NAME "parse_and_remove_configuration_command")
set(TEST_CONF_FILE "main_command.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "command" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_remove_configuration_contact
set(TEST_NAME "parse_and_remove_configuration_contact")
set(TEST_CONF_FILE "main_contact.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "contact" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_remove_configuration_contact_template
set(TEST_NAME "parse_and_remove_configuration_contact_template")
set(TEST_CONF_FILE "main_contact_template.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "contact" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_remove_configuration_contactgroup
set(TEST_NAME "parse_and_remove_configuration_contactgroup")
set(TEST_CONF_FILE "main_contact.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "contactgroup" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_remove_configuration_host
set(TEST_NAME "parse_and_remove_configuration_host")
set(TEST_CONF_FILE "main_host.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "hostescalation" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_remove_configuration_host_template
set(TEST_NAME "parse_and_remove_configuration_host_template")
set(TEST_CONF_FILE "main_host_template.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "host" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_remove_configuration_hostescalation
set(TEST_NAME "parse_and_remove_configuration_hostescalation")
set(TEST_CONF_FILE "main_host.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "host" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_remove_configuration_hostgroup
set(TEST_NAME "parse_and_remove_configuration_hostgroup")
set(TEST_CONF_FILE "main_host.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "hostgroup" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_remove_configuration_service
set(TEST_NAME "parse_and_remove_configuration_service")
set(TEST_CONF_FILE "main_service.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "service" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_remove_configuration_service_template
set(TEST_NAME "parse_and_remove_configuration_service_template")
set(TEST_CONF_FILE "main_service_template.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "service" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_remove_configuration_servicegroup
set(TEST_NAME "parse_and_remove_configuration_servicegroup")
set(TEST_CONF_FILE "main_service.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "servicegroup" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_remove_configuration_timeperiod
set(TEST_NAME "parse_and_remove_configuration_timeperiod")
set(TEST_CONF_FILE "main_timeperiod.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "timeperiod" "${CONF_DIR}/${TEST_CONF_FILE}")

# parse_and_remove_configuration_timeperiod_template
set(TEST_NAME "parse_and_remove_configuration_timeperiod_template")
set(TEST_CONF_FILE "main_timeperiod_template.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "timeperiod" "${CONF_DIR}/${TEST_CONF_FILE}")


## apply configuration and schedule objects.

# check scheduler.
set(TEST_BIN_NAME "check_scheduler")
add_executable(
  "${TEST_BIN_NAME}"
  "${TEST_DIR}/check_scheduler.cc"
  "${TEST_DIR}/events.cc"
  "${TEST_DIR}/xodtemplate.cc"
)
target_link_libraries("${TEST_BIN_NAME}" "cce_core")

# check_scheduler_host_configuration
set(TEST_NAME "check_scheduler_host_configuration")
set(TEST_CONF_FILE "main_host.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# check_scheduler_host_template_configuration
set(TEST_NAME "check_scheduler_host_template_configuration")
set(TEST_CONF_FILE "main_host_template.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

# check_scheduler_service_configuration
set(TEST_NAME "check_scheduler_configuration")
set(TEST_CONF_FILE "main_scheduler.cfg")
add_test("${TEST_NAME}" "${TEST_BIN_NAME}" "${CONF_DIR}/${TEST_CONF_FILE}")

## check modify configuration.
add_library("mod_checkmodify" SHARED "${TEST_DIR}/mod_checkmodify.cc")
set_target_properties("mod_checkmodify" PROPERTIES PREFIX "")
get_property(MODULE_PATH TARGET "mod_checkmodify" PROPERTY LOCATION)

# check modify_command
set(TEST_NAME "check_modify_command")
set(TEST_MAIN_CONFIG "chkmodify_command.cfg")
set(TEST_MAIN_MODIFY_CONFIG "chkmodify_command_2.cfg")
add_test("${TEST_NAME}" "${CENTENGINE_PATH}" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(CFG_FILES "cfg_file=base_command.cfg" "cfg_file=base_connector.cfg")
string(REPLACE ";" "\n" CFG_FILES "${CFG_FILES}")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(CFG_FILES "cfg_file=base_command_2.cfg" "cfg_file=base_connector.cfg")
string(REPLACE ";" "\n" CFG_FILES "${CFG_FILES}")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_MODIFY_CONFIG}")

# check modify_connector
set(TEST_NAME "check_modify_connector")
set(TEST_MAIN_CONFIG "chkmodify_connector.cfg")
set(TEST_MAIN_MODIFY_CONFIG "chkmodify_connector_2.cfg")
add_test("${TEST_NAME}" "${CENTENGINE_PATH}" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(CFG_FILES "cfg_file=base_connector.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(CFG_FILES "cfg_file=base_connector_2.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_MODIFY_CONFIG}")

# check modify_contact
set(TEST_NAME "check_modify_contact")
set(TEST_MAIN_CONFIG "chkmodify_contact.cfg")
set(TEST_MAIN_MODIFY_CONFIG "chkmodify_contact_2.cfg")
add_test("${TEST_NAME}" "${CENTENGINE_PATH}" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(
  CFG_FILES
  "cfg_file=base_command.cfg"
  "cfg_file=base_connector.cfg"
  "cfg_file=base_contact.cfg"
  "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(
  CFG_FILES
  "cfg_file=base_command.cfg"
  "cfg_file=base_connector.cfg"
  "cfg_file=base_contact_2.cfg"
  "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_MODIFY_CONFIG}")

# check modify_host
set(TEST_NAME "check_modify_host")
set(TEST_MAIN_CONFIG "chkmodify_host.cfg")
set(TEST_MAIN_MODIFY_CONFIG "chkmodify_host_2.cfg")
add_test("${TEST_NAME}" "${CENTENGINE_PATH}" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(
  CFG_FILES
  "cfg_file=base_command.cfg"
  "cfg_file=base_connector.cfg"
  "cfg_file=base_contact.cfg"
  "cfg_file=base_host.cfg"
  "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(
  CFG_FILES
  "cfg_file=base_command.cfg"
  "cfg_file=base_connector.cfg"
  "cfg_file=base_contact.cfg"
  "cfg_file=base_host_2.cfg"
  "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_MODIFY_CONFIG}")

# check modify_hostdependency
set(TEST_NAME "check_modify_hostdependency")
set(TEST_MAIN_CONFIG "chkmodify_hostdependency.cfg")
set(TEST_MAIN_MODIFY_CONFIG "chkmodify_hostdependency_2.cfg")
add_test("${TEST_NAME}" "${CENTENGINE_PATH}" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(
  CFG_FILES
  "cfg_file=base_command.cfg"
  "cfg_file=base_connector.cfg"
  "cfg_file=base_contact.cfg"
  "cfg_file=base_host.cfg"
  "cfg_file=base_hostdependency.cfg"
  "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(
  CFG_FILES
  "cfg_file=base_command.cfg"
  "cfg_file=base_connector.cfg"
  "cfg_file=base_contact.cfg"
  "cfg_file=base_host.cfg"
  "cfg_file=base_hostdependency_2.cfg"
  "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_MODIFY_CONFIG}")

# check modify_hostescalation
set(TEST_NAME "check_modify_hostescalation")
set(TEST_MAIN_CONFIG "chkmodify_hostescalation.cfg")
set(TEST_MAIN_MODIFY_CONFIG "chkmodify_hostescalation_2.cfg")
add_test("${TEST_NAME}" "${CENTENGINE_PATH}" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(
  CFG_FILES
  "cfg_file=base_command.cfg"
  "cfg_file=base_connector.cfg"
  "cfg_file=base_contact.cfg"
  "cfg_file=base_host.cfg"
  "cfg_file=base_hostescalation.cfg"
  "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(
  CFG_FILES
  "cfg_file=base_command.cfg"
  "cfg_file=base_connector.cfg"
  "cfg_file=base_contact.cfg"
  "cfg_file=base_host.cfg"
  "cfg_file=base_hostescalation_2.cfg"
  "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_MODIFY_CONFIG}")

# check modify_service
set(TEST_NAME "check_modify_service")
set(TEST_MAIN_CONFIG "chkmodify_service.cfg")
set(TEST_MAIN_MODIFY_CONFIG "chkmodify_service_2.cfg")
add_test("${TEST_NAME}" "${CENTENGINE_PATH}" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(
  CFG_FILES
  "cfg_file=base_command.cfg"
  "cfg_file=base_connector.cfg"
  "cfg_file=base_contact.cfg"
  "cfg_file=base_host.cfg"
  "cfg_file=base_service.cfg"
  "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(
  CFG_FILES
  "cfg_file=base_command.cfg"
  "cfg_file=base_connector.cfg"
  "cfg_file=base_contact.cfg"
  "cfg_file=base_host.cfg"
  "cfg_file=base_service_2.cfg"
  "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_MODIFY_CONFIG}")

# check modify_servicedependency
set(TEST_NAME "check_modify_servicedependency")
set(TEST_MAIN_CONFIG "chkmodify_servicedependency.cfg")
set(TEST_MAIN_MODIFY_CONFIG "chkmodify_servicedependency_2.cfg")
add_test("${TEST_NAME}" "${CENTENGINE_PATH}" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(
  CFG_FILES
  "cfg_file=base_command.cfg"
  "cfg_file=base_connector.cfg"
  "cfg_file=base_contact.cfg"
  "cfg_file=base_host.cfg"
  "cfg_file=base_service.cfg"
  "cfg_file=base_servicedependency.cfg"
  "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(
  CFG_FILES
  "cfg_file=base_command.cfg"
  "cfg_file=base_connector.cfg"
  "cfg_file=base_contact.cfg"
  "cfg_file=base_host.cfg"
  "cfg_file=base_service.cfg"
  "cfg_file=base_servicedependency_2.cfg"
  "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_MODIFY_CONFIG}")

# check modify_serviceescalation
set(TEST_NAME "check_modify_serviceescalation")
set(TEST_MAIN_CONFIG "chkmodify_serviceescalation.cfg")
set(TEST_MAIN_MODIFY_CONFIG "chkmodify_serviceescalation_2.cfg")
add_test("${TEST_NAME}" "${CENTENGINE_PATH}" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(
  CFG_FILES
  "cfg_file=base_command.cfg"
  "cfg_file=base_connector.cfg"
  "cfg_file=base_contact.cfg"
  "cfg_file=base_host.cfg"
  "cfg_file=base_service.cfg"
  "cfg_file=base_serviceescalation.cfg"
  "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(
  CFG_FILES
  "cfg_file=base_command.cfg"
  "cfg_file=base_connector.cfg"
  "cfg_file=base_contact.cfg"
  "cfg_file=base_host.cfg"
  "cfg_file=base_service.cfg"
  "cfg_file=base_serviceescalation_2.cfg"
  "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_MODIFY_CONFIG}")

# check modify_timeperiod
set(TEST_NAME "check_modify_timeperiod")
set(TEST_MAIN_CONFIG "chkmodify_timeperiod.cfg")
set(TEST_MAIN_MODIFY_CONFIG "chkmodify_timeperiod_2.cfg")
add_test("${TEST_NAME}" "${CENTENGINE_PATH}" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(CFG_FILES "cfg_file=base_timeperiod.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_CONFIG}")
set(CFG_FILES "cfg_file=base_timeperiod_2.cfg")
configure_file("${CONF_DIR}/chkmodify.cfg.in" "${CONF_DIR}/${TEST_MAIN_MODIFY_CONFIG}")

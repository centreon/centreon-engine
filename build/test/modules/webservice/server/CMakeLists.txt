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
set(TEST_DIR "${TEST_DIR}/modules/webservice/server")

# # Test calling configuration_save_resource.
# set(TEST_NAME "modules_webservice_configuration_save_resource")
# add_executable("${TEST_NAME}"
#   "${TEST_DIR}/configuration/save/resource.cc")
# target_link_libraries("${TEST_NAME}" "cce_core" "webservice")
# add_test("${TEST_NAME}" "${TEST_NAME}")

# # Test calling configuration_save_state.
# set(TEST_NAME "modules_webservice_configuration_save_state")
# add_executable("${TEST_NAME}"
#   "${TEST_DIR}/configuration/save/state.cc")
# target_link_libraries("${TEST_NAME}" "cce_core" "webservice")
# add_test("${TEST_NAME}" "${TEST_NAME}" "${TEST_DIR}/configuration/save/cfg/state.cfg")

# # Test calling configuration_save_objects.
# set(TEST_NAME "modules_webservice_configuration_save_objects")
# add_executable("${TEST_NAME}"
#   "${TEST_DIR}/configuration/save/objects.cc")
# target_link_libraries("${TEST_NAME}" "cce_core" "webservice")
# add_test(
#   "${TEST_NAME}_commands"
#   "${TEST_NAME}"
#   "command"
#   "${TEST_DIR}/configuration/save/cfg/minimal.cfg"
#   "${TEST_DIR}/configuration/save/cfg/commands.cfg")
# add_test(
#   "${TEST_NAME}_contacts"
#   "${TEST_NAME}"
#   "contact"
#   "${TEST_DIR}/configuration/save/cfg/minimal.cfg"
#   "${TEST_DIR}/configuration/save/cfg/contacts.cfg")
# add_test(
#   "${TEST_NAME}_contactgroups"
#   "${TEST_NAME}"
#   "contactgroup"
#   "${TEST_DIR}/configuration/save/cfg/minimal.cfg"
#   "${TEST_DIR}/configuration/save/cfg/contactgroups.cfg")
# add_test(
#   "${TEST_NAME}_hosts"
#   "${TEST_NAME}"
#   "host"
#   "${TEST_DIR}/configuration/save/cfg/minimal.cfg"
#   "${TEST_DIR}/configuration/save/cfg/hosts.cfg")
# add_test(
#   "${TEST_NAME}_hostescalations"
#   "${TEST_NAME}"
#   "hostescalation"
#   "${TEST_DIR}/configuration/save/cfg/minimal.cfg"
#   "${TEST_DIR}/configuration/save/cfg/hostescalations.cfg")
# add_test(
#   "${TEST_NAME}_hostdependencies"
#   "${TEST_NAME}"
#   "hostdependency"
#   "${TEST_DIR}/configuration/save/cfg/minimal.cfg"
#   "${TEST_DIR}/configuration/save/cfg/hostdependencies.cfg")
# add_test(
#   "${TEST_NAME}_hostgroups"
#   "${TEST_NAME}"
#   "hostgroup"
#   "${TEST_DIR}/configuration/save/cfg/minimal.cfg"
#   "${TEST_DIR}/configuration/save/cfg/hostgroups.cfg")
# add_test(
#   "${TEST_NAME}_services"
#   "${TEST_NAME}"
#   "service"
#   "${TEST_DIR}/configuration/save/cfg/minimal.cfg"
#   "${TEST_DIR}/configuration/save/cfg/services.cfg")
# add_test(
#   "${TEST_NAME}_servicedependencies"
#   "${TEST_NAME}"
#   "servicedependency"
#   "${TEST_DIR}/configuration/save/cfg/minimal.cfg"
#   "${TEST_DIR}/configuration/save/cfg/servicedependencies.cfg")
# add_test(
#   "${TEST_NAME}_serviceescalations"
#   "${TEST_NAME}"
#   "serviceescalation"
#   "${TEST_DIR}/configuration/save/cfg/minimal.cfg"
#   "${TEST_DIR}/configuration/save/cfg/serviceescalations.cfg")
# add_test(
#   "${TEST_NAME}_servicegroups"
#   "${TEST_NAME}"
#   "servicegroup"
#   "${TEST_DIR}/configuration/save/cfg/minimal.cfg"
#   "${TEST_DIR}/configuration/save/cfg/servicegroups.cfg")
# add_test(
#   "${TEST_NAME}_timeperiods"
#   "${TEST_NAME}"
#   "timeperiod"
#   "${TEST_DIR}/configuration/save/cfg/minimal.cfg"
#   "${TEST_DIR}/configuration/save/cfg/timeperiods.cfg")


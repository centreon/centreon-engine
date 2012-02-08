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
set(SRC_DIR "${SRC_DIR}/objects")
set(INC_DIR "${INC_DIR}/com/centreon/engine/objects")

# cce_objects target.
add_library("cce_objects"
  STATIC
  "${SRC_DIR}/command.cc"
  "${SRC_DIR}/commandsmember.cc"
  "${SRC_DIR}/contact.cc"
  "${SRC_DIR}/contactgroup.cc"
  "${SRC_DIR}/contactgroupsmember.cc"
  "${SRC_DIR}/contactsmember.cc"
  "${SRC_DIR}/customvariablesmember.cc"
  "${SRC_DIR}/daterange.cc"
  "${SRC_DIR}/host.cc"
  "${SRC_DIR}/hostdependency.cc"
  "${SRC_DIR}/hostescalation.cc"
  "${SRC_DIR}/hostgroup.cc"
  "${SRC_DIR}/hostsmember.cc"
  "${SRC_DIR}/objectlist.cc"
  "${SRC_DIR}/service.cc"
  "${SRC_DIR}/servicedependency.cc"
  "${SRC_DIR}/serviceescalation.cc"
  "${SRC_DIR}/servicegroup.cc"
  "${SRC_DIR}/servicesmember.cc"
  "${SRC_DIR}/timeperiod.cc"
  "${SRC_DIR}/timeperiodexclusion.cc"
  "${SRC_DIR}/timerange.cc"
  "${INC_DIR}/command.hh"
  "${INC_DIR}/commandsmember.hh"
  "${INC_DIR}/contactgroup.hh"
  "${INC_DIR}/contactgroupsmember.hh"
  "${INC_DIR}/contact.hh"
  "${INC_DIR}/contactsmember.hh"
  "${INC_DIR}/customvariablesmember.hh"
  "${INC_DIR}/daterange.hh"
  "${INC_DIR}/hostdependency.hh"
  "${INC_DIR}/hostescalation.hh"
  "${INC_DIR}/hostgroup.hh"
  "${INC_DIR}/host.hh"
  "${INC_DIR}/hostsmember.hh"
  "${INC_DIR}/objectlist.hh"
  "${INC_DIR}/servicedependency.hh"
  "${INC_DIR}/serviceescalation.hh"
  "${INC_DIR}/servicegroup.hh"
  "${INC_DIR}/service.hh"
  "${INC_DIR}/servicesmember.hh"
  "${INC_DIR}/timeperiodexclusion.hh"
  "${INC_DIR}/timeperiod.hh"
  "${INC_DIR}/timerange.hh"
  "${INC_DIR}/utils.hh"
)

# Link target with necessary libraries.
target_link_libraries("cce_objects" ${QT_LIBRARIES} "cce_core")

if (WITH_TESTING)
  # Set directories.
  set(TEST_DIR "${TEST_DIR}/objects")

  # cce_objects_test target.
  add_library("cce_objects_test"
    STATIC
    "${TEST_DIR}/create_object.cc"
  )
#target_link_libraries("cce_objects_test" ${QT_LIBRARIES} "cce_core")

  # add_command_to_member
  set(TEST_SRC "add_command_to_member")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # add_contactgroup_to_member
  set(TEST_SRC "add_contactgroup_to_member")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # add_contact_to_member
  set(TEST_SRC "add_contact_to_member")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # add_custom_variables_to_member
  set(TEST_SRC "add_custom_variables_to_member")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # add_host_to_member
  set(TEST_SRC "add_host_to_member")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # link_contact
  set(TEST_SRC "link_contact")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # link_contactgroup
  set(TEST_SRC "link_contactgroup")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # link_host
  set(TEST_SRC "link_host")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # link_hostdependency
  set(TEST_SRC "link_hostdependency")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # link_hostescalation
  set(TEST_SRC "link_hostescalation")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # link_hostgroup
  set(TEST_SRC "link_hostgroup")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # link_service
  set(TEST_SRC "link_service")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # link_servicedependency
  set(TEST_SRC "link_servicedependency")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # link_serviceescalation
  set(TEST_SRC "link_serviceescalation")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # link_servicegroup
  set(TEST_SRC "link_servicegroup")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_command
  set(TEST_SRC "release_command")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_commandsmember
  set(TEST_SRC "release_commandsmember")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_contact
  set(TEST_SRC "release_contact")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_contactgroup
  set(TEST_SRC "release_contactgroup")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_contactgroupsmember
  set(TEST_SRC "release_contactgroupsmember")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_contactsmember
  set(TEST_SRC "release_contactsmember")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_customvariablesmember
  set(TEST_SRC "release_customvariablesmember")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_daterange
  set(TEST_SRC "release_daterange")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_host
  set(TEST_SRC "release_host")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_hostdependency
  set(TEST_SRC "release_hostdependency")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_hostescalation
  set(TEST_SRC "release_hostescalation")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_hostgroup
  set(TEST_SRC "release_hostgroup")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_hostsmember
  set(TEST_SRC "release_hostsmember")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_objectlist
  set(TEST_SRC "release_objectlist")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_service
  set(TEST_SRC "release_service")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_servicedependency
  set(TEST_SRC "release_servicedependency")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_serviceescalation
  set(TEST_SRC "release_serviceescalation")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_servicegroup
  set(TEST_SRC "release_servicegroup")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_servicesmember
  set(TEST_SRC "release_servicesmember")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_timeperiod
  set(TEST_SRC "release_timeperiod")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_timeperiodexclusion
  set(TEST_SRC "release_timeperiodexclusion")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # release_timerange
  set(TEST_SRC "release_timerange")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core" "cce_objects_test")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # remove_command
  set(TEST_SRC "remove_command")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # remove_contact
  set(TEST_SRC "remove_contact")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # remove_contactgroup
  set(TEST_SRC "remove_contactgroup")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # remove_host
  set(TEST_SRC "remove_host")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # remove_hostdependency
  set(TEST_SRC "remove_hostdependency")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # remove_hostescalation
  set(TEST_SRC "remove_hostescalation")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # remove_hostgroup
  set(TEST_SRC "remove_hostgroup")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # remove_service
  set(TEST_SRC "remove_service")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # remove_servicedependency
  set(TEST_SRC "remove_servicedependency")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # remove_serviceescalation
  set(TEST_SRC "remove_serviceescalation")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # remove_servicegroup
  set(TEST_SRC "remove_servicegroup")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  # remove_timeperiod
  set(TEST_SRC "remove_timeperiod")
  set(TEST_NAME "objects_${TEST_SRC}")
  add_executable("${TEST_NAME}" "${TEST_DIR}/${TEST_SRC}.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
endif ()

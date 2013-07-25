##
## Copyright 2011 Merethis
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

# Test directory.
set(TEST_DIR "${PROJECT_SOURCE_DIR}/test/modules/webservice")

# Subdirectory.
add_subdirectory("server")

# Common library.
set(TEST_LIBRARY "ws_test_lib")
add_library(
  "${TEST_LIBRARY}"
  STATIC

  # Sources.
  "${TEST_DIR}/engine.cc"
  "${TEST_DIR}/query.cc"
  "${PROJECT_SOURCE_DIR}/src/error.cc"

  # Headers.
  "${TEST_DIR}/engine.hh"
  "${TEST_DIR}/query.hh"
  "${PROJECT_SOURCE_DIR}/inc/com/centreon/engine/error.hh"
)
target_link_libraries("${TEST_LIBRARY}" ${CLIB_LIBRARIES})

# External command module.
get_property(EXTERNAL_COMMAND_PATH
  TARGET "externalcmd"
  PROPERTY LOCATION)

# Webservice module.
get_property(WEBSERVICE_PATH
  TARGET "webservice"
  PROPERTY LOCATION)

# Host custom variables.
set(
  CFG_CONTENT
  "broker_module=${WEBSERVICE_PATH} ${TEST_DIR}/webservice.cfg"
)
configure_file(
  "${PROJECT_SOURCE_DIR}/test/running/etc/minimal.cfg.in"
  "${PROJECT_SOURCE_DIR}/test/running/etc/webservice.cfg"
)
set(TEST_NAME "modules_webservice_host_custom_variables")
add_executable("${TEST_NAME}" "${TEST_DIR}/host_custom_variables.cc")
target_link_libraries("${TEST_NAME}" "${TEST_LIBRARY}")
add_test("${TEST_NAME}" "${TEST_NAME}")

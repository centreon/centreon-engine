##
## Copyright 2011 Merethis
##
## This file is part of Centreon Scheduler.
##
## Centreon Scheduler is free software: you can redistribute it and/or
## modify it under the terms of the GNU General Public License version 2
## as published by the Free Software Foundation.
##
## Centreon Scheduler is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Centreon Scheduler. If not, see
## <http://www.gnu.org/licenses/>.
##

add_custom_command(OUTPUT "${MODULE_DIR}/client/centreon-engine-ws.hh"
  COMMAND "${WSDL2H_PROGRAM}" "-o" "${MODULE_DIR}/client/centreon-engine-ws.hh" "-t${MODULE_DIR}/typemap.dat" "${MODULE_DIR}/centreon-engine.wsdl")

add_custom_command(OUTPUT
  "${MODULE_DIR}/client/centreonengine.nsmap"
  "${MODULE_DIR}/client/soapC.cpp"
  "${MODULE_DIR}/client/soapH.h"
  "${MODULE_DIR}/client/soapClient.cpp"
  "${MODULE_DIR}/client/soapStub.h"
  COMMAND "${SOAPCPP2_PROGRAM}" "-C" "-L" "-x" "-d${MODULE_DIR}/client" "-I${GSOAP_INCLUDE_DIR}" "${MODULE_DIR}/client/centreon-engine-ws.hh"
  DEPENDS "${MODULE_DIR}/client/centreon-engine-ws.hh")

set(WS_STUB_BUILDER "${PROJECT_SOURCE_DIR}/build/scripts/ws_stub_builder/ws_stub_builder")

add_custom_command(OUTPUT
  "${MODULE_DIR}/client/auto_gen.cc"
  "${MODULE_DIR}/client/auto_gen.hh"
  COMMAND "${WS_STUB_BUILDER}" "${MODULE_DIR}/client/soapStub.h" "${MODULE_DIR}/client/auto_gen.hh" "${MODULE_DIR}/client/auto_gen.cc"
  DEPENDS "${MODULE_DIR}/client/soapStub.h"
  "ws_stub_builder")

# Include Directories
include_directories("${INCLUDE_DIR}")
include_directories("${MODULE_DIR}/client")

# ws_cmd_utility target.
add_executable("ws_cmd_utility"
  "${MODULE_DIR}/client/soapC.cpp"
  "${MODULE_DIR}/client/soapClient.cpp"
  "${MODULE_DIR}/client/auto_gen.cc"
  "${MODULE_DIR}/client/webservice.cc"
  "${MODULE_DIR}/client/error.cc"
  "${MODULE_DIR}/client/ssl.cc"
  "${MODULE_DIR}/client/main.cc"
  "${MODULE_DIR}/client/auto_gen.hh"
  "${MODULE_DIR}/client/webservice.hh"
  "${MODULE_DIR}/client/error.hh"
  "${MODULE_DIR}/client/ssl.hh")
get_property(MODULE_COMPILE_FLAGS
  TARGET "ws_cmd_utility"
  PROPERTY COMPILE_FLAGS)
set(MODULE_COMPILE_FLAGS "${MODULE_COMPILE_FLAGS}")
if (CMAKE_COMPILER_IS_GNUCXX)
  set_source_files_properties("${MODULE_DIR}/client/soapC.cpp" PROPERTIES
    COMPILE_FLAGS "-Wno-unused-parameter")
  set_source_files_properties("${MODULE_DIR}/client/auto_gen.cc" PROPERTIES
    COMPILE_FLAGS "-Wno-unused-function")
endif ()
set_target_properties("ws_cmd_utility" PROPERTIES
  COMPILE_FLAGS "${MODULE_COMPILE_FLAGS}")
target_link_libraries("ws_cmd_utility"
  ${GSOAP_LIBRARIES}
  ${SSL_LIBRARIES}
  ${CRYPTO_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${QT_LIBRARIES})

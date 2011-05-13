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

add_custom_command(OUTPUT "${MODULE_DIR}/server/centreon-engine-ws.hh"
  COMMAND "${WSDL2H_PROGRAM}" "-o" "${MODULE_DIR}/server/centreon-engine-ws.hh" "-t${MODULE_DIR}/typemap.dat" "${MODULE_DIR}/centreon-engine.wsdl")

add_custom_command(OUTPUT
  "${MODULE_DIR}/server/centreonengine.nsmap"
  "${MODULE_DIR}/server/soapC.cpp"
  "${MODULE_DIR}/server/soapH.h"
  "${MODULE_DIR}/server/soapServer.cpp"
  "${MODULE_DIR}/server/soapStub.h"
  COMMAND "${SOAPCPP2_PROGRAM}" "-S" "-L" "-x" "-d${MODULE_DIR}/server" "-I${GSOAP_INCLUDE_DIR}" "${MODULE_DIR}/server/centreon-engine-ws.hh"
  DEPENDS "${MODULE_DIR}/server/centreon-engine-ws.hh")

# Include Directories.
include_directories("${MODULE_DIR}/server")
include_directories("${MODULE_DIR}/../external_cmd")

# Qt moc files.
qt4_wrap_cpp(QT_WS_MOC_FILES
  "${MODULE_DIR}/server/webservice.hh")

# centscheduler target.
add_library("webservice"
  "SHARED"
  "${MODULE_DIR}/server/soapC.cpp"
  "${MODULE_DIR}/server/soapServer.cpp"
  "${MODULE_DIR}/server/commands.cc"
  "${MODULE_DIR}/server/webservice.cc"
  "${MODULE_DIR}/server/configuration.cc"
  "${MODULE_DIR}/server/ssl.cc"
  "${MODULE_DIR}/server/main.cc"
  "${MODULE_DIR}/server/syncro.cc"
  "${MODULE_DIR}/server/webservice.hh"
  "${MODULE_DIR}/server/configuration.hh"
  "${MODULE_DIR}/server/ssl.hh"
  "${MODULE_DIR}/server/syncro.hh"
  ${QT_WS_MOC_FILES})
get_property(MODULE_COMPILE_FLAGS
  TARGET "webservice"
  PROPERTY COMPILE_FLAGS)
set(MODULE_COMPILE_FLAGS "${MODULE_COMPILE_FLAGS}")
if (CMAKE_COMPILER_IS_GNUCXX)
  set_source_files_properties("${MODULE_DIR}/server/soapC.cpp" PROPERTIES
    COMPILE_FLAGS "-Wno-unused-parameter")
endif ()
set_target_properties("webservice" PROPERTIES
  COMPILE_FLAGS "${MODULE_COMPILE_FLAGS}")
target_link_libraries("webservice"
  ${GSOAP_LIBRARIES}
  ${SSL_LIBRARIES}
  ${CRYPTO_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${QT_LIBRARIES}
  "external_cmd")
install(TARGETS "webservice"
  DESTINATION "${libdir}/"
  COMPONENT "runtime")

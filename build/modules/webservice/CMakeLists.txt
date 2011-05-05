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

# Set directories.
set(MODULE_DIR "${PROJECT_SOURCE_DIR}/modules/webservice")

# Find and execute gSOAP's wsdl2h.
find_program(WSDL2H_PROGRAM "wsdl2h")
if (NOT WSDL2H_PROGRAM)
  message(FATAL_ERROR "Could not find gSOAP's wsdl2h binary.")
endif ()
add_custom_command(OUTPUT "${MODULE_DIR}/centreon-engine-ws.hh"
  COMMAND "${WSDL2H_PROGRAM}" "-o" "${MODULE_DIR}/centreon-engine-ws.hh" "-t${MODULE_DIR}/typemap.dat" "${MODULE_DIR}/centreon-engine.wsdl")

# Find and execute gSOAP's soapcpp2.
find_program(SOAPCPP2_PROGRAM "soapcpp2")
if (NOT SOAPCPP2_PROGRAM)
  message(FATAL_ERROR "Could not find gSOAP's soapcpp2 binary.")
endif ()
find_path(GSOAP_INCLUDE_DIR NAMES "stlvector.h"
  HINTS "/usr/share/gsoap/import"
  PATH_SUFFIXES "" "gsoap")
if (NOT GSOAP_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find gSOAP's stlvector.h header file.")
endif ()

add_custom_command(OUTPUT
  "${MODULE_DIR}/centreonengine.nsmap"
  "${MODULE_DIR}/soapC.cpp"
  "${MODULE_DIR}/soapH.h"
  "${MODULE_DIR}/soapServer.cpp"
  "${MODULE_DIR}/soapServerLib.cpp"
  "${MODULE_DIR}/soapStub.h"
  COMMAND "${SOAPCPP2_PROGRAM}" "-x" "-d${MODULE_DIR}/" "-S" "-I${GSOAP_INCLUDE_DIR}" "${MODULE_DIR}/centreon-engine-ws.hh"
  DEPENDS "${MODULE_DIR}/centreon-engine-ws.hh")

# Find library.
find_library(GSOAPSSL_LIBRARIES "gsoapssl++")
if (NOT GSOAPSSL_LIBRARIES)
  message(FATAL_ERROR "Could not find gSOAP's libgsoapssl++.")
endif ()

find_library(SSL_LIBRARIES "ssl")
if (NOT SSL_LIBRARIES)
  message(FATAL_ERROR "Could not find libssl.")
endif ()

find_library(CRYPTO_LIBRARIES "crypto")
if (NOT CRYPTO_LIBRARIES)
  message(FATAL_ERROR "Could not find libcrypto.")
endif ()

find_library(ZLIB_LIBRARIES "z")
if (NOT ZLIB_LIBRARIES)
  message(FATAL_ERROR "Could not find zlib's libz.")
endif ()

# Include Directories
include_directories("${INCLUDE_DIR}")
include_directories("${MODULE_DIR}")

# Qt moc files.
qt4_wrap_cpp(QT_WS_MOC_FILES
  "${MODULE_DIR}/webservice.hh")

# Define flags.
add_definitions(-DWITH_OPENSSL)
add_definitions(-DWITH_GZIP)

# centscheduler target.
add_library("webservice"
  "SHARED"
  "${MODULE_DIR}/soapC.cpp"
  "${MODULE_DIR}/soapServer.cpp"
  "${MODULE_DIR}/commands.cc"
  "${MODULE_DIR}/webservice.cc"
  "${MODULE_DIR}/configuration.cc"
  "${MODULE_DIR}/ssl.cc"
  "${MODULE_DIR}/syncro.cc"
  "${MODULE_DIR}/main.cc"
  ${QT_WS_MOC_FILES})
get_property(MODULE_COMPILE_FLAGS
  TARGET "webservice"
  PROPERTY COMPILE_FLAGS)
set(MODULE_COMPILE_FLAGS "${MODULE_COMPILE_FLAGS}")
if (CMAKE_COMPILER_IS_GNUCXX)
  set_source_files_properties("${MODULE_DIR}/soapC.cpp" PROPERTIES
    COMPILE_FLAGS "-Wno-unused-parameter")
endif ()
set_target_properties("webservice" PROPERTIES
  COMPILE_FLAGS "${MODULE_COMPILE_FLAGS}")
target_link_libraries("webservice"
  ${GSOAPSSL_LIBRARIES}
  ${SSL_LIBRARIES}
  ${CRYPTO_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${QT_LIBRARIES})
install(TARGETS "webservice"
  DESTINATION "${libdir}/"
  COMPONENT "runtime")

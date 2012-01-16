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

# Generate webservice header from WSDL.
add_custom_command(OUTPUT "${MODULE_DIR}/server/centreon-engine-ws.hh"
  COMMAND "${WSDL2H_PROGRAM}" "-o" "${MODULE_DIR}/server/centreon-engine-ws.hh" "-t${MODULE_DIR}/typemap.dat" "${MODULE_DIR}/centreon-engine.wsdl")

# Generate webservice sources from header.
add_custom_command(OUTPUT
  "${MODULE_DIR}/server/centreonengine.nsmap"
  "${MODULE_DIR}/server/soapC.cpp"
  "${MODULE_DIR}/server/soapH.h"
  "${MODULE_DIR}/server/soapServer.cpp"
  "${MODULE_DIR}/server/soapStub.h"
  COMMAND "${SOAPCPP2_PROGRAM}" "-S" "-L" "-x" "-d${MODULE_DIR}/server" "-I${GSOAP_INCLUDE_DIR}" "${MODULE_DIR}/server/centreon-engine-ws.hh"
  DEPENDS "${MODULE_DIR}/server/centreon-engine-ws.hh")

# Include directories.
include_directories("${MODULE_DIR}/server")
include_directories("${MODULE_DIR}/../external_cmd")

# Qt moc files.
qt4_wrap_cpp(QT_WS_MOC_FILES
  "${MODULE_DIR}/server/webservice.hh")

# webservice target.
add_library("webservice"
  SHARED
  "${MODULE_DIR}/server/schedule_object.cc"
  "${MODULE_DIR}/server/create_object.cc"
  "${MODULE_DIR}/server/soapC.cpp"
  "${MODULE_DIR}/server/soapServer.cpp"
  "${MODULE_DIR}/server/commands.cc"
  "${MODULE_DIR}/server/webservice.cc"
  "${MODULE_DIR}/server/configuration.cc"
  "${MODULE_DIR}/server/ssl.cc"
  "${MODULE_DIR}/server/main.cc"
  "${MODULE_DIR}/server/syncro.cc"
  "${MODULE_DIR}/server/schedule_object.hh"
  "${MODULE_DIR}/server/create_object.hh"
  "${MODULE_DIR}/server/webservice.hh"
  "${MODULE_DIR}/server/configuration.hh"
  "${MODULE_DIR}/server/ssl.hh"
  "${MODULE_DIR}/server/syncro.hh"
  ${QT_WS_MOC_FILES})

# Prettier name.
set_target_properties("webservice" PROPERTIES PREFIX "")

# Compiler-specific addon to avoid some warnings.
if (CMAKE_COMPILER_IS_GNUCXX)
  # Avoid warnings on autogenerate soapC.cpp file.
  unset(compile_flags)
  get_source_file_property(compile_flags
    "${MODULE_DIR}/server/soapC.cpp"
    COMPILE_FLAGS)
  if (compile_flags)
    set(compile_flags "${compile_flags} -Wno-unused-parameter")
  else ()
    set(compile_flags "-Wno-unused-parameter")
  endif ()
  set_source_files_properties("${MODULE_DIR}/server/soapC.cpp"
    PROPERTIES COMPILE_FLAGS "${compile_flags}")
endif ()

# Link target with libraries.
target_link_libraries("webservice"
  ${GSOAP_LIBRARIES}
  ${SSL_LIBRARIES}
  ${CRYPTO_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${QT_LIBRARIES}
  "externalcmd")

# Install rule.
install(TARGETS "webservice"
  DESTINATION "${libdir}/"
  COMPONENT "runtime")

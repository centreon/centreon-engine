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

# Should we build webservice.
option(WITH_WEBSERVICE "Enable or disable webservice module compilation." OFF)
if (WITH_WEBSERVICE)

  # Set directories.
  set(MODULE_DIR "${PROJECT_SOURCE_DIR}/modules/webservice")

  # Find gSOAP's wsdl2h.
  find_program(WSDL2H_PROGRAM "wsdl2h")
  if (NOT WSDL2H_PROGRAM)
    message(FATAL_ERROR "Could not find gSOAP's wsdl2h binary.")
  endif ()

  # Find gSOAP's soapcpp2.
  find_program(SOAPCPP2_PROGRAM "soapcpp2")
  if (NOT SOAPCPP2_PROGRAM)
    message(FATAL_ERROR "Could not find gSOAP's soapcpp2 binary.")
  endif ()
  find_path(GSOAP_INC_DIR NAMES "stdsoap2.h"
    PATH_SUFFIXES "" "gsoap")
  if (NOT GSOAP_INC_DIR)
    message(FATAL_ERROR "Could not find gSOAP's stdsoap2.h header file.")
  endif ()
  include_directories("${GSOAP_INC_DIR}")
  find_path(GSOAP_INCLUDE_DIR NAMES "stlvector.h"
    HINTS "/usr/share/gsoap/import" "/usr/local/share/gsoap/import" "/opt/local/share/gsoap/import"
    PATH_SUFFIXES "" "gsoap")
  if (NOT GSOAP_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find gSOAP's stlvector.h header file.")
  endif ()

  # Check for SSL support.
  option(WITH_SSL "Enable or disable SSL support in webservice." OFF)
  if (WITH_SSL)
    # Find libgsoapssl++.
    find_library(GSOAP_LIBRARIES "gsoapssl++")
    if (NOT GSOAP_LIBRARIES)
      message(FATAL_ERROR "Could not find gSOAP's libgsoapssl++.")
    endif ()

    # Find libssl.
    find_library(SSL_LIBRARIES "ssl")
    if (NOT SSL_LIBRARIES)
      message(FATAL_ERROR "Could not find libssl.")
    endif ()

    # Find libcrypto.
    find_library(CRYPTO_LIBRARIES "crypto")
    if (NOT CRYPTO_LIBRARIES)
      message(FATAL_ERROR "Could not find libcrypto.")
    endif ()

    # Compilation flag.
    add_definitions(-DWITH_OPENSSL)
  else ()
    # Find libgsoap++.
    find_library(GSOAP_LIBRARIES "gsoap++")
    if (NOT GSOAP_LIBRARIES)
      message(FATAL_ERROR "Could not find gSOAP's libgsoap++.")
    endif ()
    set(SSL_LIBRARIES "")
    set(CRYPTO_LIBRARIES "")
  endif ()

  # Check for compression support.
  option(WITH_ZLIB "Enable or disable compression in webservice." ON)
  if (WITH_ZLIB)
    # Search for libz.
    find_library(ZLIB_LIBRARIES "z")
    if (NOT ZLIB_LIBRARIES)
      message(FATAL_ERROR "Could not find zlib's libz.")
    endif ()

    # Enable deflate and gzip compression.
    add_definitions(-DWITH_GZIP)
  endif ()

  # Add subdirectories.
  add_subdirectory("client")
  add_subdirectory("server")
  set(CENTENGINEWS_BINARY "${CENTENGINEWS_BINARY}" PARENT_SCOPE)
endif ()

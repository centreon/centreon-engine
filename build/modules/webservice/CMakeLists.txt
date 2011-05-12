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

# Find and execute gSOAP's soapcpp2.
find_program(SOAPCPP2_PROGRAM "soapcpp2")
if (NOT SOAPCPP2_PROGRAM)
  message(FATAL_ERROR "Could not find gSOAP's soapcpp2 binary.")
endif ()
find_path(GSOAP_INCLUDE_DIR NAMES "stlvector.h"
  HINTS "/usr/share/gsoap/import" "/usr/local/share/gsoap/import"
  PATH_SUFFIXES "" "gsoap")
if (NOT GSOAP_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find gSOAP's stlvector.h header file.")
endif ()

# Find library.
if (WITH_SSL)
  find_library(GSOAP_LIBRARIES "gsoapssl++")
  if (NOT GSOAP_LIBRARIES)
    message(FATAL_ERROR "Could not find gSOAP's libgsoapssl++.")
  endif ()

  find_library(SSL_LIBRARIES "ssl")
  if (NOT SSL_LIBRARIES)
    message(FATAL_ERROR "Could not find libssl.")
  endif ()
else ()
  find_library(GSOAP_LIBRARIES "gsoap++")
  if (NOT GSOAP_LIBRARIES)
    message(FATAL_ERROR "Could not find gSOAP's libgsoap++.")
  endif ()
  set(SSL_LIBRARIES "")
endif ()

find_library(CRYPTO_LIBRARIES "crypto")
if (NOT CRYPTO_LIBRARIES)
  message(FATAL_ERROR "Could not find libcrypto.")
endif ()

find_library(ZLIB_LIBRARIES "z")
if (NOT ZLIB_LIBRARIES)
  message(FATAL_ERROR "Could not find zlib's libz.")
endif ()

# Define flags.
if (WITH_SSL)
  add_definitions(-DWITH_OPENSSL)
endif ()
add_definitions(-DWITH_GZIP)

add_subdirectory("./client")
add_subdirectory("./server")
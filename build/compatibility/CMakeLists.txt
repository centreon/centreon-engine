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

# Set directories.
set(SRC_DIR "${SRC_DIR}/compatibility")
set(INC_DIR "${INCLUDE_DIR}/compatibility")

# Configure files.
configure_file("${INCLUDE_DIR}/compatibility/locations.h.in"
  "${INCLUDE_DIR}/compatibility/locations.h")

# cce_compatibility target.
add_library("cce_compatibility"
  STATIC
  "${SRC_DIR}/common.cc"
  "${SRC_DIR}/globals.cc"
  "${SRC_DIR}/logging.cc")

# Link target.
target_link_libraries("cce_compatibility" "cce_core")

# Install header files for devel.
install(DIRECTORY "${INC_DIR}"
  DESTINATION "${localstatedir}/include/centreon-engine/"
  COMPONENT "development"
  FILES_MATCHING PATTERN "*.h")

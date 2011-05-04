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
set(MY_SRC_DIR "${SRC_DIR}/compatibility")
set(MY_INC_DIR "${INCLUDE_DIR}/compatibility")

# Configure files.
configure_file("${INCLUDE_DIR}/compatibility/locations.h.in"
  "${INCLUDE_DIR}/compatibility/locations.h")

# cce_compatibility target.
add_library("cce_compatibility"
  "STATIC"
  "${MY_SRC_DIR}/common.cc"
  "${MY_SRC_DIR}/globals.cc"
  "${MY_SRC_DIR}/logging.cc")
get_property(MY_COMPILE_FLAGS
  TARGET "cce_compatibility"
  PROPERTY COMPILE_FLAGS)
set(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
set_target_properties("cce_compatibility"
  PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
target_link_libraries("cce_compatibility"
  "cce_logging")

# Install header files for devel.
install(DIRECTORY "${MY_INC_DIR}"
  DESTINATION "${localstatedir}/include/centreon-engine/"
  COMPONENT "development"
  FILES_MATCHING PATTERN "*.h")

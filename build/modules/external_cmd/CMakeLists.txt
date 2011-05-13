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
set(MODULE_DIR "${PROJECT_SOURCE_DIR}/modules/external_cmd")

# Include Directories
include_directories("${MODULE_DIR}")

# cce_logging target.
add_library("external_cmd"
  "SHARED"
  "${MODULE_DIR}/main.cc"
  "${MODULE_DIR}/commands.cc"
  "${MODULE_DIR}/utils.cc"
  "${MODULE_DIR}/commands.hh"
  "${MODULE_DIR}/utils.hh")
get_property(MY_COMPILE_FLAGS
  TARGET "external_cmd"
  PROPERTY COMPILE_FLAGS)
set(MODULE_COMPILE_FLAGS_FLAGS "${MODULE_COMPILE_FLAGS}")
set_target_properties("external_cmd"
  PROPERTIES COMPILE_FLAGS "${MODULE_COMPILE_FLAGS}")
target_link_libraries("external_cmd"
  ${QT_LIBRARIES})
install(TARGETS "external_cmd"
  DESTINATION "${libdir}/"
  COMPONENT "runtime")
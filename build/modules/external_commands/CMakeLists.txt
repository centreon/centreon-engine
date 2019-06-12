##
## Copyright 2011-2013 Merethis
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
set(MODULE_DIR "${PROJECT_SOURCE_DIR}/modules/external_commands")
set(SRC_DIR "${MODULE_DIR}/src")
set(INC_DIR "${MODULE_DIR}/inc/com/centreon/engine/modules/external_commands")

# Include directories.
include_directories("${MODULE_DIR}/inc")

# mod_externalcmd target.
add_library(
  "externalcmd"
  SHARED

  # Sources.
  "${SRC_DIR}/commands.cc"
  "${SRC_DIR}/internal.cc"
  "${SRC_DIR}/main.cc"
  "${SRC_DIR}/processing.cc"
  "${SRC_DIR}/utils.cc"

  # Headers.
  "${INC_DIR}/commands.hh"
  "${INC_DIR}/internal.hh"
  "${INC_DIR}/processing.hh"
  "${INC_DIR}/utils.hh"
)
# Prettier name.
set_property(TARGET "externalcmd" PROPERTY PREFIX "")
# Location.
get_property(
  EXTERNALCMD_MODULE
  TARGET "externalcmd"
  PROPERTY LOCATION)
set(EXTERNALCMD_MODULE "${EXTERNALCMD_MODULE}" PARENT_SCOPE)
# Link target with libraries.
target_link_libraries("externalcmd" "${CLIB_LIBRARIES}")

# Install rule.
install(TARGETS "externalcmd"
  DESTINATION "${PREFIX_LIB}"
  COMPONENT "runtime")

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
set(MODULE_DIR "${PROJECT_SOURCE_DIR}/modules/external_cmd")

# Include directories
include_directories("${MODULE_DIR}")

# externalcmd target.
add_library("externalcmd"
  SHARED
  "${MODULE_DIR}/main.cc"
  "${MODULE_DIR}/commands.cc"
  "${MODULE_DIR}/utils.cc"
  "${MODULE_DIR}/commands.hh"
  "${MODULE_DIR}/utils.hh")

# Prettier name.
set_target_properties("externalcmd" PROPERTIES PREFIX "")

# Link target with libraries.
target_link_libraries("externalcmd"
  ${QT_LIBRARIES})

# Install rule.
install(TARGETS "externalcmd"
  DESTINATION "${libdir}/"
  COMPONENT "runtime")

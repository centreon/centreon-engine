##
## Copyright 2009-2013,2015 Centreon
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##
## For more information : contact@centreon.com
##

# Global options.
set(SRC_DIR "${SRC_DIR}/simumod")
set(INC_DIR "${INC_DIR}/com/centreon/engine/simumod")
include_directories("${INC_DIR}")

# Simu module.
set(SIMUMOD "simumod")

add_custom_command(
  OUTPUT ${SRC_DIR}/neb.cc
  COMMAND ${SRC_DIR}/coder.pl > ${SRC_DIR}/neb.cc
  DEPENDS ${SRC_DIR}/coder.pl
)
add_library("${SIMUMOD}" SHARED
  # Sources.
  "${SRC_DIR}/neb.cc"
  # Headers.
)
# Prettier name.
set_target_properties("${SIMUMOD}" PROPERTIES PREFIX "")

# Location
get_property(SIMUMOD_MODULE
  TARGET "${SIMUMOD}"
  PROPERTY LOCATION)
set(SIMUMOD_MODULE "${SIMUMOD_MODULE}" PARENT_SCOPE)

target_link_libraries("${SIMUMOD}" "${CLIB_LIBRARIES}")

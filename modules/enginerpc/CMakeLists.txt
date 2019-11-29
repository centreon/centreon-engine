##
## Copyright 2019 Centreon (https://www.centreon.com/)
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
## http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##
## For more information : contact@centreon.com
##
##

# Set directories.
set(MODULE_DIR "${PROJECT_SOURCE_DIR}/modules/enginerpc")
set(SRC_DIR "${MODULE_DIR}/src")
set(INC_DIR "${MODULE_DIR}/inc")

# Include directories.
include_directories("${INC_DIR}")

include(ExternalProject)

ExternalProject_Add(c-ares
    PREFIX c-ares
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cares/cares"
    GIT_REPOSITORY https://github.com/c-ares/c-ares.git
    GIT_TAG cares-1_15_0
    CMAKE_CACHE_ARGS
      -DCARES_SHARED:BOOL=OFF
      -DCARES_STATIC:BOOL=ON
      -DCARES_STATIC_PIC:BOOL=ON
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/c-ares
)

ExternalProject_Add(protobuf
    PREFIX protobuf
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/protobuf"
    CONFIGURE_COMMAND ${CMAKE_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/third_party/protobuf/cmake
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v3.11.0
    CMAKE_CACHE_ARGS
      -Dprotobuf_BUILD_TESTS:BOOL=OFF
      -Dprotobuf_WITH_ZLIB:BOOL=OFF
      -Dprotobuf_MSVC_STATIC_RUNTIME:BOOL=OFF
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY}/protobuf
)

ExternalProject_Add(zlib
  PREFIX zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG v1.2.11
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/zlib"
  CMAKE_CACHE_ARGS
        -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_CURRENT_BINARY_DIR}/zlib
)

# mod_enginerpc target.
add_library(
  "enginerpc"
  SHARED

  # Sources.
  "${SRC_DIR}/main.cc"

  # Headers.
)
# Prettier name.
set_property(TARGET "enginerpc" PROPERTY PREFIX "")
# Location.
get_property(
  ENGINERPC_MODULE
  TARGET "enginerpc"
  PROPERTY LOCATION)
set(ENGINERPC_MODULE "${ENGINERPC_MODULE}" PARENT_SCOPE)
# Link target with libraries.
target_link_libraries("enginerpc" "${CLIB_LIBRARIES}")

# Install rule.
install(TARGETS "enginerpc"
  DESTINATION "${PREFIX_LIB}"
  COMPONENT "runtime")

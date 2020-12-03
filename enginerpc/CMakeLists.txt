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
set(ENGINERPC_DIR "${PROJECT_SOURCE_DIR}/enginerpc")
set(ENGINERPC_DIR "${PROJECT_SOURCE_DIR}/enginerpc" PARENT_SCOPE)

set(ENGINERPC enginerpc)
set(ENGINERPC enginerpc PARENT_SCOPE)
# Include directories.
include_directories(
  ${ENGINERPC_DIR}
)


set(protobuf_MODULE_COMPATIBLE True)
find_package(protobuf REQUIRED)
find_package(protoc_installer REQUIRED)
find_package(gRPC REQUIRED)

include_directories(${gRPC_INCLUDE_DIRS})
include_directories(${protobuf_INCLUDE_DIRS})

link_directories(${protobuf_LIB_DIRS})
link_directories(${gRPC_LIB_DIRS})

message(STATUS "Using protobuf ${gRPC_VERSION}")
set(PROTOBUF_PREFIX "${protoc_installer_LIB_DIRS}/..")
message(STATUS "${PROTOBUF_PREFIX}/bin/protoc")
set(GRPC_PREFIX "${gRPC_LIB_DIRS}/..")

add_custom_command(
  DEPENDS ${ENGINERPC_DIR}/engine.proto
  COMMENT "Generating interface files of the proto file (grpc)"
  OUTPUT ${ENGINERPC_DIR}/engine.grpc.pb.cc ${ENGINERPC_DIR}/engine.grpc.pb.h
  COMMAND ${PROTOBUF_PREFIX}/bin/protoc
  ARGS --plugin=protoc-gen-grpc=${GRPC_PREFIX}/bin/grpc_cpp_plugin --proto_path=${ENGINERPC_DIR} --grpc_out="${ENGINERPC_DIR}" ${ENGINERPC_DIR}/engine.proto

  DEPENDS ${ENGINERPC_DIR}/engine.proto
  COMMENT "Generating interface files of the proto file (protobuf)"
  OUTPUT ${ENGINERPC_DIR}/engine.pb.cc ${ENGINERPC_DIR}/engine.pb.h
  COMMAND ${PROTOBUF_PREFIX}/bin/protoc
  ARGS --cpp_out="${ENGINERPC_DIR}" --proto_path=${ENGINERPC_DIR} ${ENGINERPC_DIR}/engine.proto
)

add_library(
    cerpc
    STATIC

  ${ENGINERPC_DIR}/engine.grpc.pb.cc
  ${ENGINERPC_DIR}/engine.pb.cc
  ${ENGINERPC_DIR}/engine.grpc.pb.h
  ${ENGINERPC_DIR}/engine.pb.h)
target_link_libraries(cerpc ${protobuf_LIBS})

# mod_enginerpc target.
add_library(
  ${ENGINERPC}
  STATIC

  # Sources.
  engine_impl.cc
  enginerpc.cc

  # Headers.
  engine_impl.hh
  enginerpc.hh
)

# Prettier name.
set_property(TARGET ${ENGINERPC} PROPERTY PREFIX "" PROPERTY POSITION_INDEPENDENT_CODE ON)
# Link target with libraries.
target_link_libraries(${ENGINERPC} cerpc)

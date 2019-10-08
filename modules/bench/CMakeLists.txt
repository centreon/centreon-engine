##
## Copyright 2015-2016 Centreon
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

option(WITH_BENCH "Build benchmarking tools." OFF)
if (WITH_BENCH)
  # Set directories.
  set(SRC_DIR "${PROJECT_SOURCE_DIR}/modules/bench")
  include_directories("${SRC_DIR}/passive")
  include_directories("${SRC_DIR}/plugins")
  configure_file("${SRC_DIR}/passive/paths.hh.in" "${SRC_DIR}/passive/paths.hh")

  # Passive checks benchmarking module.
  add_library("bench_passive_module" SHARED
    "${SRC_DIR}/passive/module.cc")
  set_target_properties("bench_passive_module"
    PROPERTIES PREFIX "")
  install(TARGETS "bench_passive_module"
    DESTINATION "${PREFIX_LIB}"
    COMPONENT "bench")

  # Passive checks benchmarking command line tool.
  add_executable("centengine_bench_passive"
    "${PROJECT_SOURCE_DIR}/src/process_manager.cc"
    "${PROJECT_SOURCE_DIR}/src/process.cc"
    "${PROJECT_SOURCE_DIR}/src/logging/temp_logger.cc"
    "${PROJECT_SOURCE_DIR}/src/logging/engine.cc"
    "${SRC_DIR}/passive/engine_cfg.cc"
    "${SRC_DIR}/passive/main.cc")
  target_link_libraries("centengine_bench_passive" ${CLIB_LIBRARIES} pthread)
  install(TARGETS "centengine_bench_passive"
    DESTINATION "${PREFIX_BIN}"
    COMPONENT "bench")
endif ()

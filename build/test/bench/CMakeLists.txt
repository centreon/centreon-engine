##
## Copyright 2015 Merethis
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
  add_library("bench_passive_module" SHARED
    "${TEST_DIR}/bench/passive/module.cc")
  set_target_properties("bench_passive_module"
    PROPERTIES PREFIX "")
  install(TARGETS "bench_passive_module"
    DESTINATION "${PREFIX_LIB}"
    COMPONENT "bench")

  add_executable("centengine_bench_passive"
    "${TEST_DIR}/bench/passive/engine_cfg.cc"
    "${TEST_DIR}/bench/passive/main.cc")
  target_link_libraries("centengine_bench_passive" ${CLIB_LIBRARIES})
  install(TARGETS "centengine_bench_passive"
    DESTINATION "${PREFIX_BIN}"
    COMPONENT "bench")

endif ()

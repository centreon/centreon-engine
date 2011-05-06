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
set(MY_SRC_DIR "${SRC_DIR}/broker")
set(MY_INC_DIR "${INCLUDE_DIR}/broker")
set(MY_TEST_DIR "${TEST_DIR}/broker")

# Produce moc files.
qt4_wrap_cpp(QT_MOC_FILES
  "${MY_INC_DIR}/loader.hh"
  "${MY_INC_DIR}/handle.hh"
  "${MY_INC_DIR}/compatibility.hh")

# cce_broker target.
add_library("cce_broker"
  "STATIC"
  "${MY_SRC_DIR}/compatibility.cc"
  "${MY_SRC_DIR}/loader.cc"
  "${MY_SRC_DIR}/handle.cc"
  "${MY_INC_DIR}/compatibility.hh"
  "${MY_INC_DIR}/handle.hh"
  "${MY_INC_DIR}/loader.hh"
  ${QT_MOC_FILES})
get_property(MY_COMPILE_FLAGS
  TARGET "cce_broker"
  PROPERTY COMPILE_FLAGS)
set(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
set_target_properties("cce_broker"
  PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
target_link_libraries("cce_broker"
  ${QT_LIBRARIES}
  "centenginelib")

# Test module.
add_library("mod_test_load"
  "SHARED"
  "${MY_TEST_DIR}/mod_test_load.cc")
add_library("mod_test_load_compatibility"
  "SHARED"
  "${MY_TEST_DIR}/mod_test_load_compatibility.cc")

# Test loader.
add_executable("test_broker_loader"
  "${MY_TEST_DIR}/test_loader.cc")
target_link_libraries("test_broker_loader"
  ${QT_LIBRARIES}
  "centenginelib")
add_test("test_broker_loader"
  "test_broker_loader")

# Test handle.
add_executable("test_broker_handle"
  "${MY_TEST_DIR}/test_handle.cc")
target_link_libraries("test_broker_handle"
  ${QT_LIBRARIES}
  "centenginelib")
add_test("test_broker_handle"
  "test_broker_handle")

# Test nebmodule.
add_executable("test_broker_nebmodule"
  "${MY_TEST_DIR}/test_nebmodule.cc")
target_link_libraries("test_broker_nebmodule"
  ${QT_LIBRARIES}
  "centenginelib")
add_test("test_broker_nebmodule"
  "test_broker_nebmodule")

# Test compatibility.
add_executable("test_broker_compatibility"
  "${MY_TEST_DIR}/test_compatibility.cc")
target_link_libraries("test_broker_compatibility"
  ${QT_LIBRARIES}
  "centenginelib")
add_test("test_broker_compatibility"
  "test_broker_compatibility")

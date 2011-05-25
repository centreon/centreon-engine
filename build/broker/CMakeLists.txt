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
set(SRC_DIR "${SRC_DIR}/broker")
set(INC_DIR "${INCLUDE_DIR}/broker")
set(TEST_DIR "${TEST_DIR}/broker")

# Produce moc files.
qt4_wrap_cpp(QT_MOC_FILES
  "${INC_DIR}/loader.hh"
  "${INC_DIR}/handle.hh"
  "${INC_DIR}/compatibility.hh")

# cce_broker target.
add_library("cce_broker"
  STATIC
  "${SRC_DIR}/compatibility.cc"
  "${SRC_DIR}/loader.cc"
  "${SRC_DIR}/handle.cc"
  "${INC_DIR}/compatibility.hh"
  "${INC_DIR}/handle.hh"
  "${INC_DIR}/loader.hh"
  ${QT_MOC_FILES})

# Link target with required libraries.
target_link_libraries("cce_broker" ${QT_LIBRARIES} "cce_core")

if (WITH_TESTING)
  # Test module.
  add_library("broker_mod_load" SHARED "${TEST_DIR}/mod_load.cc")
  set_target_properties("broker_mod_load" PROPERTIES PREFIX "")
  add_library("broker_mod_compatibility"
    SHARED
    "${TEST_DIR}/mod_compatibility.cc")
  set_target_properties("broker_mod_compatibility" PROPERTIES PREFIX "")

  # Test loader.
  add_executable("broker_loader" "${TEST_DIR}/loader.cc")
  target_link_libraries("broker_loader" ${QT_LIBRARIES} "cce_core")
  add_test("broker_loader" "broker_loader")

  # Test handle.
  add_executable("broker_handle" "${TEST_DIR}/handle.cc")
  target_link_libraries("broker_handle" ${QT_LIBRARIES} "cce_core")
  add_test("broker_handle" "broker_handle")

  # Test nebmodule.
  add_executable("broker_nebmodule" "${TEST_DIR}/nebmodule.cc")
  target_link_libraries("broker_nebmodule" ${QT_LIBRARIES} "cce_core")
  add_test("broker_nebmodule" "broker_nebmodule")

  # Test compatibility.
  add_executable("broker_compatibility" "${TEST_DIR}/compatibility.cc")
  target_link_libraries("broker_compatibility" ${QT_LIBRARIES} "cce_core")
  add_test("broker_compatibility" "broker_compatibility")
endif ()
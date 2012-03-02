##
## Copyright 2011-2012 Merethis
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
set(SRC_DIR "${SRC_DIR}/logging")
set(INC_DIR "${INC_DIR}/com/centreon/engine/logging")

# cce_logging target.
add_library("cce_logging"
  STATIC
  "${SRC_DIR}/standard.cc"
  "${SRC_DIR}/file.cc"
  "${SRC_DIR}/syslog.cc"
  "${SRC_DIR}/logger.cc"
  "${SRC_DIR}/broker.cc"
  "${SRC_DIR}/engine.cc"
#  "${SRC_DIR}/dumpers.cc"  # XXX: todo.
  "${INC_DIR}/object.hh"
  "${INC_DIR}/standard.hh"
  "${INC_DIR}/file.hh"
  "${INC_DIR}/syslog.hh"
  "${INC_DIR}/logger.hh"
  "${INC_DIR}/broker.hh"
  "${INC_DIR}/engine.hh"
  "${INC_DIR}/dumpers.hh"
)

# Link target with necessary libraries.
target_link_libraries("cce_logging" ${QT_LIBRARIES} "cce_core")

if (WITH_TESTING)
  # Set directories.
  set(TEST_DIR "${TEST_DIR}/logging")

  # Test engine.
  add_executable("logging_engine"
    "${TEST_DIR}/engine.cc"
    "${TEST_DIR}/test.cc"
    "${TEST_DIR}/test.hh")
  target_link_libraries("logging_engine" ${QT_LIBRARIES} "cce_core")
  add_test("logging_engine" "logging_engine")

  # Test broker.
  add_library("dummymod" SHARED "${TEST_DIR}/dummymod.cc")
  set_target_properties("dummymod" PROPERTIES PREFIX "")
  add_executable("logging_broker" "${TEST_DIR}/broker.cc" ${QT_MOC_FILES})
  target_link_libraries("logging_broker" ${QT_LIBRARIES} "cce_core")
  add_test("logging_broker" "logging_broker")

  # Test file.
  add_executable("logging_file" "${TEST_DIR}/file.cc")
  target_link_libraries("logging_file" ${QT_LIBRARIES} "cce_core")
  add_test("logging_file" "logging_file")
endif ()

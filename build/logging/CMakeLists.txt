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
set(SRC_DIR "${SRC_DIR}/logging")
set(INC_DIR "${INC_DIR}/com/centreon/engine/logging")

# cce_logging target.
add_library("cce_logging"
  STATIC
  "${SRC_DIR}/broker.cc"
  # "${SRC_DIR}/dumpers.cc"
  "${INC_DIR}/logger.hh"
  "${INC_DIR}/broker.hh"
  # "${INC_DIR}/dumpers.hh"
)

# Link target with necessary libraries.
target_link_libraries("cce_logging" "cce_core")

if (WITH_TESTING)
  # Set directories.
  set(TEST_DIR "${TEST_DIR}/logging")

  # Test broker log.
  add_library("dummymod" SHARED "${TEST_DIR}/dummymod.cc")
  set_target_properties("dummymod" PROPERTIES PREFIX "")
  add_executable("logging_broker_log" "${TEST_DIR}/broker_log.cc")
  target_link_libraries("logging_broker_log" "cce_core")
  add_test("logging_broker_log" "logging_broker_log")

  # Test broker show information.
  add_executable("logging_broker_show_info" "${TEST_DIR}/broker_show_info.cc")
  target_link_libraries("logging_broker_show_info" "cce_core")
  add_test("logging_broker_show_info" "logging_broker_show_info")

endif ()

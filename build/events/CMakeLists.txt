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
set(SRC_DIR "${SRC_DIR}/events")
set(INC_DIR "${INC_DIR}/com/centreon/engine/events")

# cce_macros target.
add_library("cce_events" STATIC
  # Sources.
  "${SRC_DIR}/loop.cc"
  # Headers.
  "${INC_DIR}/loop.hh"
  "${INC_DIR}/sched_info.hh")

# Link target with necessary libraries.
target_link_libraries("cce_events" "cce_core")

if (WITH_TESTING)
  # Set directories.
  set(TEST_DIR "${TEST_DIR}/events")

  # Events tests.
  set(TEST_NAME "events_handle_timed_event")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/handle_timed_event.cc")
  target_link_libraries("${TEST_NAME}" "cce_events")
  add_test("${TEST_NAME}" "${TEST_NAME}")
endif ()

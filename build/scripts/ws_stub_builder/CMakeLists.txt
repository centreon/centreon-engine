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

# ws_stub_builder target.
add_executable("ws_stub_builder"
  "${SCRIPT_DIR}/ws_stub_builder/main.cc"
  "${SCRIPT_DIR}/ws_stub_builder/builder.cc"
  "${SCRIPT_DIR}/ws_stub_builder/function.cc"
  "${SCRIPT_DIR}/ws_stub_builder/argument.cc"
  "${SCRIPT_DIR}/ws_stub_builder/arg_definition.cc"
  "${SCRIPT_DIR}/ws_stub_builder/error.cc"
  "${SCRIPT_DIR}/ws_stub_builder/builder.hh"
  "${SCRIPT_DIR}/ws_stub_builder/function.hh"
  "${SCRIPT_DIR}/ws_stub_builder/argument.hh"
  "${SCRIPT_DIR}/ws_stub_builder/arg_definition.hh"
  "${SCRIPT_DIR}/ws_stub_builder/error.hh")
get_property(MODULE_COMPILE_FLAGS
  TARGET "ws_stub_builder"
  PROPERTY COMPILE_FLAGS)
set(MODULE_COMPILE_FLAGS "${MODULE_COMPILE_FLAGS}")
set_target_properties("ws_stub_builder" PROPERTIES
  COMPILE_FLAGS "${MODULE_COMPILE_FLAGS}")
target_link_libraries("ws_stub_builder"
  ${QT_LIBRARIES})



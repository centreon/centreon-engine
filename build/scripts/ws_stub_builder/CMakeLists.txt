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


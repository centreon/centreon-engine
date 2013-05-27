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
set(SRC_DIR "${SRC_DIR}/applier")
set(INC_DIR "${INC_DIR}/applier")

# cce_configuration_applier target.
add_library("cce_configuration_applier"
  STATIC
  "${SRC_DIR}/base.cc"
  "${SRC_DIR}/globals.cc"
  "${SRC_DIR}/logging.cc"
  "${SRC_DIR}/macros.cc"
  "${INC_DIR}/base.hh"
  "${INC_DIR}/globals.hh"
  "${INC_DIR}/logging.hh"
  "${INC_DIR}/macros.hh")

# Link target with required libraries.
target_link_libraries("cce_configuration_applier" "cce_logging")

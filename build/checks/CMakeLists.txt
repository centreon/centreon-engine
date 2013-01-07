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
set(SRC_DIR "${SRC_DIR}/checks")
set(INC_DIR "${INC_DIR}/com/centreon/engine/checks")
set(TEST_DIR "${TEST_DIR}/checks")

# cce_checks target.
add_library("cce_checks" STATIC
  # Sources.
  "${SRC_DIR}/checker.cc"
  "${SRC_DIR}/stats.cc"
  # Headers.
  "${INC_DIR}/checker.hh"
  "${INC_DIR}/stats.hh")

# Link target with required libraries.
target_link_libraries("cce_checks" "cce_commands")

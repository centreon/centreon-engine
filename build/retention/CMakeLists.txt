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
set(SRC_DIR "${SRC_DIR}/retention")
set(INC_DIR "${INC_DIR}/com/centreon/engine/retention")

# cce_retention target.
add_library("cce_retention"
  STATIC
  "${SRC_DIR}/contact.cc"
  "${SRC_DIR}/host.cc"
  "${SRC_DIR}/parser.cc"

  "${INC_DIR}/contact.hh"
  "${INC_DIR}/host.hh"
  "${INC_DIR}/parser.hh"
  "${INC_DIR}/object.hh"
)

# Link target with necessary libraries.
target_link_libraries("cce_retention" "cce_core")

if (WITH_TESTING)
  # Set directories.
  set(TEST_DIR "${TEST_DIR}/retention")
endif ()

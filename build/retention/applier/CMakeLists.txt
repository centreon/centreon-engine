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
set(SRC_DIR "${SRC_DIR}/applier")
set(INC_DIR "${INC_DIR}/applier")
set(TEST_DIR "${TEST_DIR}/applier")

# cce_retention_applier target.
add_library("cce_retention_applier"
  STATIC
  "${SRC_DIR}/comment.cc"
  "${SRC_DIR}/contact.cc"
  "${SRC_DIR}/downtime.cc"
  "${SRC_DIR}/host.cc"
  "${SRC_DIR}/program.cc"
  "${SRC_DIR}/service.cc"

  "${INC_DIR}/comment.hh"
  "${INC_DIR}/contact.hh"
  "${INC_DIR}/downtime.hh"
  "${INC_DIR}/host.hh"
  "${INC_DIR}/program.hh"
  "${INC_DIR}/service.hh")

# Link target with required libraries.
target_link_libraries("cce_retention_applier" "cce_core")

if (WITH_TESTING)

endif ()

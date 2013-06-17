##
## Copyright 2011-2013 Merethis
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

# Set directories.
set(SRC_DIR "${SRC_DIR}/misc")
set(INC_DIR "${INC_DIR}/com/centreon/engine/misc")

# cce_broker target.
add_library("cce_misc"
  STATIC
  "${SRC_DIR}/string.cc"
  "${INC_DIR}/object.hh"
  "${INC_DIR}/string.hh")

if (WITH_TESTING)
  #Set directories.
  set(TEST_DIR "${TEST_DIR}/string")
endif ()

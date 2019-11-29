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
set(SRC_DIR "${SRC_DIR}/events")
set(INC_DIR "${INC_DIR}/com/centreon/engine/events")

# Set files.
set(
  FILES
  ${FILES}

  # Sources.
  "${SRC_DIR}/loop.cc"
  "${SRC_DIR}/sched_info.cc"
  "${SRC_DIR}/timed_event.cc"

  # Headers.
  "${INC_DIR}/loop.hh"
  "${INC_DIR}/sched_info.hh"
  "${INC_DIR}/timed_event.hh"

  PARENT_SCOPE
)

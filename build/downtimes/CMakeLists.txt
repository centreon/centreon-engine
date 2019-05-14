##
## Copyright 2011-2019 Centreon
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
set(SRC_DIR "${SRC_DIR}/downtimes")
set(INC_DIR "${INC_DIR}/com/centreon/engine/downtimes")

# Set files.
set(
  FILES
  ${FILES}

  # Sources.
  "${SRC_DIR}/downtime.cc"
  "${SRC_DIR}/downtime_manager.cc"
  "${SRC_DIR}/host_downtime.cc"
  "${SRC_DIR}/service_downtime.cc"
  "${SRC_DIR}/downtime_finder.cc"

  # Headers.
  "${INC_DIR}/downtime.hh"
  "${INC_DIR}/downtime_manager.hh"
  "${INC_DIR}/host_downtime.hh"
  "${INC_DIR}/service_downtime.hh"
  "${INC_DIR}/downtime_finder.hh"

  PARENT_SCOPE
)

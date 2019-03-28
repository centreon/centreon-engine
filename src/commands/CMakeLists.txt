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
set(SRC_DIR "${SRC_DIR}/commands")
set(INC_DIR "${INC_DIR}/com/centreon/engine/commands")

# Set files.
set(
  FILES
  ${FILES}

  # Sources.
  "${SRC_DIR}/command.cc"
  "${SRC_DIR}/connector.cc"
  "${SRC_DIR}/environment.cc"
  "${SRC_DIR}/forward.cc"
  "${SRC_DIR}/raw.cc"
  "${SRC_DIR}/result.cc"

  # Headers.
  "${INC_DIR}/command.hh"
  "${INC_DIR}/command_listener.hh"
  "${INC_DIR}/connector.hh"
  "${INC_DIR}/environment.hh"
  "${INC_DIR}/forward.hh"
  "${INC_DIR}/raw.hh"
  "${INC_DIR}/result.hh"

  PARENT_SCOPE
)

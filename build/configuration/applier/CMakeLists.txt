##
## Copyright 2011-2015 Merethis
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

# Set files.
set(
  FILES
  ${FILES}

  # Sources.
  "${SRC_DIR}/command.cc"
  "${SRC_DIR}/connector.cc"
  "${SRC_DIR}/host.cc"
  "${SRC_DIR}/hostdependency.cc"
  "${SRC_DIR}/globals.cc"
  "${SRC_DIR}/logging.cc"
  "${SRC_DIR}/macros.cc"
  "${SRC_DIR}/member.cc"
  "${SRC_DIR}/object.cc"
  "${SRC_DIR}/scheduler.cc"
  "${SRC_DIR}/service.cc"
  "${SRC_DIR}/servicedependency.cc"
  "${SRC_DIR}/state.cc"
  "${SRC_DIR}/timeperiod.cc"

  # Headers.
  "${INC_DIR}/command.hh"
  "${INC_DIR}/connector.hh"
  "${INC_DIR}/difference.hh"
  "${INC_DIR}/host.hh"
  "${INC_DIR}/hostdependency.hh"
  "${INC_DIR}/globals.hh"
  "${INC_DIR}/logging.hh"
  "${INC_DIR}/macros.hh"
  "${INC_DIR}/member.hh"
  "${INC_DIR}/object.hh"
  "${INC_DIR}/scheduler.hh"
  "${INC_DIR}/service.hh"
  "${INC_DIR}/servicedependency.hh"
  "${INC_DIR}/state.hh"
  "${INC_DIR}/timeperiod.hh"

  PARENT_SCOPE
)

##
## Copyright 2011-2014 Merethis
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
set(SRC_DIR "${SRC_DIR}/logging")
set(INC_DIR "${INC_DIR}/com/centreon/engine/logging")

# Set files.
set(
  FILES
  ${FILES}

  # Sources.
  "${SRC_DIR}/backend.cc"
  "${SRC_DIR}/broker.cc"
  "${SRC_DIR}/debug_file.cc"
  "${SRC_DIR}/engine.cc"
  "${SRC_DIR}/file.cc"
  "${SRC_DIR}/syslogger.cc"
  "${SRC_DIR}/temp_logger.cc"

  # Headers.
  "${INC_DIR}/backend.hh"
  "${INC_DIR}/broker.hh"
  "${INC_DIR}/debug_file.hh"
  "${INC_DIR}/engine.hh"
  "${INC_DIR}/file.hh"
  "${INC_DIR}/logger.hh"
  "${INC_DIR}/syslogger.hh"
  "${INC_DIR}/temp_logger.hh"
  # "${INC_DIR}/dumpers.hh"

  PARENT_SCOPE
)

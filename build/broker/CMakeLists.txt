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
set(SRC_DIR "${SRC_DIR}/broker")
set(INC_DIR "${INC_DIR}/com/centreon/engine/broker")

# Set files.
set(
  FILES
  ${FILES}

  # Sources.
  "${SRC_DIR}/compatibility.cc"
  "${SRC_DIR}/loader.cc"
  "${SRC_DIR}/handle.cc"

  # Headers.
  "${INC_DIR}/compatibility.hh"
  "${INC_DIR}/handle.hh"
  "${INC_DIR}/loader.hh"

  PARENT_SCOPE
)

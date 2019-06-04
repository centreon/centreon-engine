##
## Copyright 2011-2019 Centreon
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
set(SRC_DIR "${SRC_DIR}/objects")
set(INC_DIR "${INC_DIR}/com/centreon/engine/objects")

# Set files.
set(
  FILES
  ${FILES}

  # Sources.
  "${SRC_DIR}/objectlist.cc"
  "${SRC_DIR}/servicesmember.cc"

  # Headers.
  "${INC_DIR}/defines.hh"
  "${INC_DIR}/objectlist.hh"
  "${INC_DIR}/servicesmember.hh"
  "${INC_DIR}/tool.hh"

  PARENT_SCOPE
)

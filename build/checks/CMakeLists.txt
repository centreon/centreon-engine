##
## Copyright 2011 Merethis
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
set(SRC_DIR "${SRC_DIR}/checks")
set(INC_DIR "${INCLUDE_DIR}/checks")
set(TEST_DIR "${TEST_DIR}/checks")

# Produce moc files.
qt4_wrap_cpp(QT_MOC_FILES
  "${INC_DIR}/checker.hh")

# cce_checks target.
add_library("cce_checks"
  STATIC
  "${SRC_DIR}/checker.cc"
  "${INC_DIR}/checker.hh"
  ${QT_MOC_FILES})

# Link target with required libraries.
target_link_libraries("cce_checks"
  ${QT_LIBRARIES}
  "cce_commands")

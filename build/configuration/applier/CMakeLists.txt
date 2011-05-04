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
set(MY_SRC_DIR "${SRC_DIR}/configuration/applier")
set(MY_INC_DIR "${INCLUDE_DIR}/configuration/applier")

# cce_configurationapplier target.
add_library("cce_configurationapplier"
  "STATIC"
  "${MY_SRC_DIR}/logging.cc"
  "${MY_INC_DIR}/logging.hh"
  "${MY_INC_DIR}/base.hh")
get_property(MY_COMPILE_FLAGS
  TARGET "cce_configurationapplier"
  PROPERTY COMPILE_FLAGS)
set(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
set_target_properties("cce_configurationapplier"
  PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
target_link_libraries("cce_configurationapplier"
  ${QT_LIBRARIES}
  "cce_logging")

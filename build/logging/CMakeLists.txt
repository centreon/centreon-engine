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
set(MY_SRC_DIR "${SRC_DIR}/logging")
set(MY_INC_DIR "${INCLUDE_DIR}/logging")

# cce_logging target.
add_library("cce_logging"
  "STATIC"
  "${MY_SRC_DIR}/standard.cc"
  "${MY_SRC_DIR}/file.cc"
  "${MY_SRC_DIR}/syslog.cc"
  "${MY_SRC_DIR}/logger.cc"
  "${MY_SRC_DIR}/broker.cc"
  "${MY_SRC_DIR}/engine.cc"
  "${MY_INC_DIR}/object.hh"
  "${MY_INC_DIR}/standard.hh"
  "${MY_INC_DIR}/file.hh"
  "${MY_INC_DIR}/syslog.hh"
  "${MY_INC_DIR}/logger.hh"
  "${MY_INC_DIR}/broker.hh"
  "${MY_INC_DIR}/engine.hh")
get_property(MY_COMPILE_FLAGS
  TARGET "cce_logging"
  PROPERTY COMPILE_FLAGS)
set(MY_COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
set_target_properties("cce_logging"
  PROPERTIES COMPILE_FLAGS "${MY_COMPILE_FLAGS}")
target_link_libraries("cce_logging" ${QT_LIBRARIES})

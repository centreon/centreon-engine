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

# Global options.
cmake_minimum_required(VERSION 2.8)
project("Centreon Scheduler" C)

# Testing.
set(PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/..")
include(CTest)

# scheduler target.
set(BASE_DIR "${PROJECT_SOURCE_DIR}/base")
set(COMMON_DIR "${PROJECT_SOURCE_DIR}/common")
set(XDATA_DIR "${PROJECT_SOURCE_DIR}/xdata")
add_executable("scheduler"
  "${BASE_DIR}/broker.c"
  "${BASE_DIR}/checks.c"
  "${BASE_DIR}/commands.c"
  "${BASE_DIR}/config.c"
  "${BASE_DIR}/events.c"
  "${BASE_DIR}/flapping.c"
  "${BASE_DIR}/logging.c"
  "${BASE_DIR}/nagios.c"
  "${BASE_DIR}/nebmods.c"
  "${BASE_DIR}/netutils.c"
  "${BASE_DIR}/notifications.c"
  "${BASE_DIR}/perfdata.c"
  "${BASE_DIR}/sehandlers.c"
  "${BASE_DIR}/sretention.c"
  "${BASE_DIR}/utils.c"
  "${COMMON_DIR}/comments.c"
  "${COMMON_DIR}/downtime.c"
  "${COMMON_DIR}/macros.c"
  "${COMMON_DIR}/objects.c"
  "${COMMON_DIR}/skiplist.c"
  "${COMMON_DIR}/snprintf.c"
  "${COMMON_DIR}/statusdata.c"
  "${XDATA_DIR}/xcddefault.c"
  "${XDATA_DIR}/xdddefault.c"
  "${XDATA_DIR}/xodtemplate.c"
  "${XDATA_DIR}/xpddefault.c"
  "${XDATA_DIR}/xrddefault.c"
  "${XDATA_DIR}/xsddefault.c"
)

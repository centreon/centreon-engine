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
set(MY_SRC_DIR "${PROJECT_SOURCE_DIR}/conf")

# Configure files.
configure_file("${MY_SRC_DIR}/centengine.cfg.in"
  "${MY_SRC_DIR}/centengine.cfg")
configure_file("${MY_SRC_DIR}/resource.cfg.in"
  "${MY_SRC_DIR}/resource.cfg")
configure_file("${MY_SRC_DIR}/objects/commands.cfg.in"
  "${MY_SRC_DIR}/objects/commands.cfg")
configure_file("${MY_SRC_DIR}/objects/contacts.cfg.in"
  "${MY_SRC_DIR}/objects/contacts.cfg")
configure_file("${MY_SRC_DIR}/objects/localhost.cfg.in"
  "${MY_SRC_DIR}/objects/localhost.cfg")
configure_file("${MY_SRC_DIR}/objects/printer.cfg.in"
  "${MY_SRC_DIR}/objects/printer.cfg")
configure_file("${MY_SRC_DIR}/objects/switch.cfg.in"
  "${MY_SRC_DIR}/objects/switch.cfg")
configure_file("${MY_SRC_DIR}/objects/templates.cfg.in"
  "${MY_SRC_DIR}/objects/templates.cfg")
configure_file("${MY_SRC_DIR}/objects/timeperiods.cfg.in"
  "${MY_SRC_DIR}/objects/timeperiods.cfg")
configure_file("${MY_SRC_DIR}/objects/windows.cfg.in"
  "${MY_SRC_DIR}/objects/windows.cfg")

#install files.
install(DIRECTORY "${MY_SRC_DIR}/"
  DESTINATION "${sysconfdir}/"
  COMPONENT "runtime"
  FILES_MATCHING PATTERN "*.cfg")

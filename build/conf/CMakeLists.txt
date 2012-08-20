##
## Copyright 2011-2012 Merethis
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
set(SRC_DIR "${PROJECT_SOURCE_DIR}/conf")

# Configure files.
configure_file("${SRC_DIR}/centengine.cfg.in"
  "${SRC_DIR}/centengine.cfg")
configure_file("${SRC_DIR}/resource.cfg.in"
  "${SRC_DIR}/resource.cfg")
configure_file("${SRC_DIR}/objects/commands.cfg.in"
  "${SRC_DIR}/objects/commands.cfg")
configure_file("${SRC_DIR}/objects/contacts.cfg.in"
  "${SRC_DIR}/objects/contacts.cfg")
configure_file("${SRC_DIR}/objects/localhost.cfg.in"
  "${SRC_DIR}/objects/localhost.cfg")
configure_file("${SRC_DIR}/objects/printer.cfg.in"
  "${SRC_DIR}/objects/printer.cfg")
configure_file("${SRC_DIR}/objects/switch.cfg.in"
  "${SRC_DIR}/objects/switch.cfg")
configure_file("${SRC_DIR}/objects/templates.cfg.in"
  "${SRC_DIR}/objects/templates.cfg")
configure_file("${SRC_DIR}/objects/timeperiods.cfg.in"
  "${SRC_DIR}/objects/timeperiods.cfg")
configure_file("${SRC_DIR}/objects/windows.cfg.in"
  "${SRC_DIR}/objects/windows.cfg")

# Install files if necessary.
option(WITH_SAMPLE_CONFIG "Install sample configuration files." ON)
if (WITH_SAMPLE_CONFIG)
  install(DIRECTORY "${SRC_DIR}/"
    DESTINATION "${PREFIX_CONF}"
    COMPONENT "runtime"
    FILES_MATCHING PATTERN "*.cfg")

  if (CREATE_FILES)
    install(CODE "
    file(GLOB_RECURSE cfg_files RELATIVE \"${SRC_DIR}\" \"${SRC_DIR}/*.cfg\")
    foreach (cfg_file IN LISTS cfg_files)
      execute_process(COMMAND \"chown\" \"${USER}:${GROUP}\" \"${PREFIX_CONF}/\${cfg_file}\")
    endforeach ()
    ")
  endif ()
endif ()

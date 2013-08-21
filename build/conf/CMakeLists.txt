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

  install(CODE "
  function(my_chown user group file)
    if (APPLE OR (UNIX AND NOT CYGWIN))
      execute_process(COMMAND \"chown\" \"\${user}:\${group}\" \"\${file}\")
    endif ()
  endfunction()

  my_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_CONF}/centengine.cfg\")
  my_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_CONF}/resource.cfg\")
  my_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_CONF}/objects/commands.cfg\")
  my_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_CONF}/objects/contacts.cfg\")
  my_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_CONF}/objects/localhost.cfg\")
  my_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_CONF}/objects/printer.cfg\")
  my_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_CONF}/objects/switch.cfg\")
  my_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_CONF}/objects/templates.cfg\")
  my_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_CONF}/objects/timeperiods.cfg\")
  my_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_CONF}/objects/windows.cfg\")
  ")
endif ()

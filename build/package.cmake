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

# Packaging.
option(WITH_PACKAGE_SH "Build shell-installable package." OFF)
option(WITH_PACKAGE_TGZ "Build gziped tarball package." OFF)
option(WITH_PACKAGE_TBZ2 "Build bzip2'd tarball package." OFF)
option(WITH_PACKAGE_DEB "Build DEB package." OFF)
option(WITH_PACKAGE_RPM "Build RPM package." OFF)
option(WITH_PACKAGE_NSIS "Build NSIS package." OFF)
if (WITH_PACKAGE_SH
    OR WITH_PACKAGE_TGZ
    OR WITH_PACKAGE_TBZ2
    OR WITH_PACKAGE_DEB
    OR WITH_PACKAGE_RPM
    OR WITH_PACKAGE_NSIS)
  # Default settings.
  set(CPACK_PACKAGE_VENDOR "Merethis")
  set(CPACK_PACKAGE_VERSION_MAJOR "${CENTREON_ENGINE_MAJOR}")
  set(CPACK_PACKAGE_VERSION_MINOR "${CENTREON_ENGINE_MINOR}")
  set(CPACK_PACKAGE_VERSION_PATCH "${CENTREON_ENGINE_PATCH}")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Centreon Engine is a monitoring engine fully Nagios-compatible but with additionnal features and performance improvements.")
  set(CPACK_PACKAGE_FILE_NAME
    "centreon-engine-${CENTREON_ENGINE_VERSION}")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "centreon-engine")
  set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/license.txt")
  set(CPACK_PACKAGE_CONTACT
    "Matthieu Kermagoret <mkermagoret@merethis.com>")

  # Generators.
  unset(PACKAGE_LIST)
  if (WITH_PACKAGE_SH)
    list(APPEND CPACK_GENERATOR "STGZ")
    list(APPEND PACKAGE_LIST "Shell-installable package (.sh)")
  endif ()
  if (WITH_PACKAGE_TGZ)
    list(APPEND CPACK_GENERATOR "TGZ")
    list(APPEND PACKAGE_LIST "gziped tarball (.tar.gz)")
  endif ()
  if (WITH_PACKAGE_TBZ2)
    list(APPEND CPACK_GENERATOR "TBZ2")
    list(APPEND PACKAGE_LIST "bzip2'd tarball (.tar.bz2)")
  endif ()
  if (WITH_PACKAGE_DEB)
    list(APPEND CPACK_GENERATOR "DEB")
    list(APPEND PACKAGE_LIST "DEB package (.deb)")
    set(CPACK_DEBIAN_PACKAGE_SECTION "net")
    configure_file("${SCRIPT_DIR}/deb/postinst.in" "${SCRIPT_DIR}/deb/postinst")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
      "${SCRIPT_DIR}/deb/postinst"
      "${SCRIPT_DIR}/deb/prerm")
  endif ()
  if (WITH_PACKAGE_RPM)
    list(APPEND CPACK_GENERATOR "RPM")
    list(APPEND PACKAGE_LIST "RPM package (.rpm)")
    set(CPACK_RPM_PACKAGE_RELEASE 1)
    set(CPACK_RPM_PACKAGE_LICENSE "GPLv2")
    configure_file("${SCRIPT_DIR}/rpm/postinst.in" "${SCRIPT_DIR}/rpm/postinst")
    set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${SCRIPT_DIR}/rpm/postinst")
    set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${SCRIPT_DIR}/rpm/prerm")
  endif ()
  if (WITH_PACKAGE_NSIS)
    list(APPEND CPACK_GENERATOR "NSIS")
    list(APPEND PACKAGE_LIST "NSIS package (.exe)")
  endif ()
  string(REPLACE ";" ", " PACKAGE_LIST "${PACKAGE_LIST}")

  # CPack module.
  include(CPack)
else ()
  set(PACKAGE_LIST "None")
endif ()

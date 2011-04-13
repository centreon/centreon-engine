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
project("Centreon Scheduler" C CXX)

# Testing.
set(PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/..")
include(CTest)

# Find Qt.
find_package(Qt4 COMPONENTS QtCore REQUIRED)
include(${QT_USE_FILE})

# Set directories.
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/inc")
set(TEST_DIR "${PROJECT_SOURCE_DIR}/test")
set(SCRIPT_DIR "${PROJECT_SOURCE_DIR}/scripts")

# Check headers.
include(CheckIncludeFiles)
include(CheckCSourceCompiles)
check_include_files("dirent.h" HAVE_DIRENT_H)
check_include_files("dlfcn.h" HAVE_DLFCN_H)
check_include_files("errno.h" HAVE_ERRNO_H)
check_include_files("fcntl.h" HAVE_FCNTL_H)
check_include_files("getopt.h" HAVE_GETOPT_H)
check_include_files("libgen.h" HAVE_LIBGEN_H)
check_include_files("limits.h" HAVE_LIMITS_H)
check_include_files("math.h" HAVE_MATH_H)
check_include_files("netdb.h" HAVE_NETDB_H)
check_include_files("pthread.h" HAVE_PTHREAD_H)
check_include_files("regex.h" HAVE_REGEX_H)
check_include_files("signal.h" HAVE_SIGNAL_H)
check_include_files("stdarg.h" HAVE_STDARG_H)
check_include_files("stdio.h" HAVE_STDIO_H)
check_include_files("stdlib.h" HAVE_STDLIB_H)
check_include_files("string.h" HAVE_STRING_H)
check_include_files("sys/mman.h" HAVE_SYS_MMAN_H)
check_include_files("sys/poll.h" HAVE_SYS_POLL_H)
check_include_files("sys/socket.h" HAVE_SYS_SOCKET_H)
check_include_files("sys/stat.h" HAVE_SYS_STAT_H)
check_include_files("sys/time.h" HAVE_SYS_TIME_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("sys/wait.h" HAVE_SYS_WAIT_H)
check_include_files("syslog.h" HAVE_SYSLOG_H)
check_include_files("time.h" HAVE_TIME_H)
check_include_files("unistd.h" HAVE_UNISTD_H)

# Check functions.
include(CheckFunctionExists)
check_function_exists("setenv" HAVE_SETENV)
check_function_exists("socket" HAVE_SOCKET)
check_function_exists("snprintf" HAVE_SNPRINTF)
check_function_exists("strdup" HAVE_STRDUP)
check_function_exists("strstr" HAVE_STRSTR)
check_function_exists("strtoul" HAVE_STRTOUL)
check_function_exists("unsetenv" HAVE_UNSETENV)
check_function_exists("vasprintf" HAVE_VASPRINTF)
check_function_exists("nanosleep" HAVE_NANOSLEEP)

# Check timezone informations.
include(CheckStructHasMember)
check_struct_has_member("tm" "tm_zone" "time.h" HAVE_STRUCT_TM_TM_ZONE)
include(CheckSymbolExists)
check_symbol_exists("tzname" "time.h" HAVE_TZNAME)

# Check libraries to link with.
include(CheckLibraryExists)
check_library_exists("dl" "dlopen" "${CMAKE_LIBRARY_PATH}" DLLIB)
if (DLLIB)
  set(DL_LIBRARIES "dl")
endif ()
check_library_exists("m" "ceil" "${CMAKE_LIBRARY_PATH}" MATHLIB)
if (MATHLIB)
  set(MATH_LIBRARIES "m")
endif ()
check_library_exists("nsl" "getservbyname" "${CMAKE_LIBRARY_PATH}" NSLLIB)
if (NSLLIBS)
  set(SOCKET_LIBRARIES "${SOCKET_LIBRARIES} nsl")
endif ()
check_library_exists("socket" "connect" "${CMAKE_LIBRARY_PATH}" SOCKETLIBS)
if (SOCKETLIBS)
  set(SOCKET_LIBRARIES "${SOCKET_LIBRARIES} socket")
endif ()

# Find pthreads.
set(CMAKE_THREAD_PREFER_PTHREAD)
include(FindThreads)
if (NOT CMAKE_USE_PTHREADS_INIT)
  message(FATAL_ERROR "Could not find pthreads.")
endif ()
set(THREAD_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")

# localstatedir is the prefix.
if (WITH_PREFIX)
  set(localstatedir "${WITH_PREFIX}")
else ()
  set(localstatedir "${CMAKE_INSTALL_PREFIX}")
endif ()
set(bindir "${localstatedir}/bin")
set(sysconfdir "${localstatedir}/etc")

# Check result dir.
if (WITH_CHECK_RESULT_DIR)
  set(CHECK_RESULT_DIR "${WITH_CHECK_RESULT_DIR}")
else ()
  set(CHECK_RESULT_DIR "${localstatedir}/spool/checkresults")
endif ()

# Temporary check result dir.
if (WITH_TEMP_DIR)
  set(TMPDIR "${WITH_TEMP_DIR}")
else ()
  set(TMPDIR "/tmp")
endif ()

# User used to run Centreon Scheduler.
if (WITH_USER)
  set(USER "${WITH_USER}")
else ()
  set(USER "root")
endif ()

# Group used to run Centreon Scheduler.
if (WITH_GROUP)
  set(GROUP "${WITH_GROUP}")
else ()
  set(GROUP "root")
endif ()

# Check which startup script to use.
option(WITH_STARTUP_SCRIPT "Generate and install startup script." ON)
set(STARTUP_SCRIPT "No")
if (WITH_STARTUP_SCRIPT)
  if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Attempting to determine OS distributor.")
    execute_process(COMMAND "lsb_release" "--short" "--id"
      RESULT_VARIABLE RETCODE
      OUTPUT_VARIABLE OS_DISTRIBUTOR
      ERROR_QUIET)
    if (RETCODE EQUAL 0)
      if (OS_DISTRIBUTOR STREQUAL "CentOS\n")
        message(STATUS "Generating CentOS startup script.")
        set(STARTUP_SCRIPT "CentOS init script")
	install(PROGRAMS "${SCRIPT_DIR}/centscheduler.sh"
	  DESTINATION "/etc/init.d"
	  RENAME "centscheduler")
      elseif (OS_DISTRIBUTOR STREQUAL "Ubuntu\n")
        message(STATUS "Generating Ubuntu startup script.")
        set(STARTUP_SCRIPT "Ubuntu upstart script")
	install(PROGRAMS "${SCRIPT_DIR}/ubuntu.conf"
	  DESTINATION "/etc/init"
	  RENAME "centscheduler.conf")
      else ()
        message(STATUS "Unsupported distributor: ${OS_DISTRIBUTOR}")
      endif ()
    else ()
      message(STATUS "Could not get Linux distributor name, not startup script will be installed.")
    endif ()
  else ()
    message(STATUS "Centreon Scheduler does not provide startup script for ${CMAKE_SYSTEM_NAME}")
    set(OS_DISTRIBUTOR "${CMAKE_SYSTEM_NAME}")
  endif ()
endif ()

# Lock file location.
# XXX

# Embedded Perl.
option(WITH_EMBEDDED_PERL "Enable embedded Perl" OFF)
option(WITH_PERL_CACHE "Enable cacheing of internally compiled Perl scripts.")
if (WITH_EMBEDDED_PERL)
  message(STATUS "Embedded Perl is being compiled in.")
  set(EMBEDDEDPERL TRUE)
  set(EMBEDDED_PERL "Yes")
  if (WITH_PERL_CACHE)
    message(STATUS "Internally compiled Perl scripts will be cached.")
    set(DOCLEAN 0)
  else ()
    message(STATUS "Internally compiled Perl scripts will NOT be cached.")
    set(DOCLEAN 1)
  endif ()
  execute_process(COMMAND "perl" "-e" "use Config;exit -1 unless (\$Config{usethreads});"
    RESULT_VARIABLE RETVAL)
  if (RETVAL EQUAL 0)
    message(STATUS "Using threaded Perl.")
    set(THREADEDPERL TRUE)
    set(THREADED_PERL "Yes")
  else ()
    message(STATUS "NOT using threaded Perl.")
    set(THREADEDPERL FALSE)
    set(THREADED_PERL "No")
  endif ()
  execute_process(COMMAND "perl" "-MExtUtils::Embed" "-e" "ccopts"
    RESULT_VARIABLE RETVAL
    OUTPUT_VARIABLE PERL_COMPILE_FLAGS)
  if (NOT RETVAL EQUAL 0)
    message(FATAL_ERROR "Could not retrieve embedded Perl compile flags.")
  endif ()
  execute_process(COMMAND "perl" "-MExtUtils::Embed" "-e" "ldopts"
    RESULT_VARIABLE RETVAL
    OUTPUT_VARIABLE PERL_LINK_FLAGS)
  if (NOT RETVAL EQUAL 0)
    message(FATAL_ERROR "Could not retrieve embedded Perl link flags.")
  endif ()
  string(STRIP "${PERL_LINK_FLAGS}" PERL_LINK_FLAGS)
  set(PERL_SOURCES "${SRC_DIR}/perlxsi.c")
  execute_process(COMMAND "perl" "-MExtUtils::Embed" "-e" "xsinit" "--" "-o" "${PERL_SOURCES}"
    RESULT_VARIABLE RETVAL)
  if (NOT RETVAL EQUAL 0)
    message(FATAL_ERROR "Could not generate embedded Perl source file.")
  endif ()
else ()
  set(EMBEDDED_PERL "No")
endif ()

# Define flags.
#  Misc definitions.
if (HAVE_STRUCT_TM_TM_ZONE)
  add_definitions(-DHAVE_STRUCT_TM_TM_ZONE)
elseif (HAVE_TZNAME)
  add_definitions(-DHAVE_TZNAME)
endif ()

#  Enmbed PERL interpreter.
if (EMBEDDEDPERL)
  add_definitions(-DEMBEDDEDPERL)
endif ()
if (THREADEDPERL)
  add_definitions(-DTHREADEDPERL)
endif()
if (DOCLEAN)
  add_definitions(-DDOCLEAN)
endif ()

#  Locations definitions
add_definitions(-DDEFAULT_TEMP_FILE="${localstatedir}/tempfile")
add_definitions(-DDEFAULT_TEMP_PATH="/tmp")
add_definitions(-DDEFAULT_CHECK_RESULT_PATH="${localstatedir}/spool/checkresults")
add_definitions(-DDEFAULT_STATUS_FILE="${localstatedir}/status.dat")
add_definitions(-DDEFAULT_LOG_FILE="${localstatedir}/nagios.log")
add_definitions(-DDEFAULT_LOG_ARCHIVE_PATH="${localstatedir}/archives/")
add_definitions(-DDEFAULT_DEBUG_FILE="${localstatedir}/nagios.debug")
add_definitions(-DDEFAULT_COMMENT_FILE="${localstatedir}/comments.dat")
add_definitions(-DDEFAULT_DOWNTIME_FILE="${localstatedir}/downtime.dat")
add_definitions(-DDEFAULT_RETENTION_FILE="${localstatedir}/retention.dat")
add_definitions(-DDEFAULT_COMMAND_FILE="${localstatedir}/rw/nagios.cmd")
add_definitions(-DDEFAULT_CONFIG_FILE="${sysconfdir}/nagios.cfg")
add_definitions(-DDEFAULT_OBJECT_CACHE_FILE="${localstatedir}/objects.cache")
add_definitions(-DDEFAULT_PRECACHED_OBJECT_FILE="${localstatedir}/objects.precache")
add_definitions(-DDEFAULT_EVENT_BROKER_FILE="${localstatedir}/broker.socket")
add_definitions(-DDEFAULT_P1_FILE="${bindir}/p1.pl")
add_definitions(-DDEFAULT_AUTH_FILE="")

# Configuration file.
configure_file("${INCLUDE_DIR}/compatibility/locations.h.in"
  "${INCLUDE_DIR}/compatibility/locations.h")
configure_file("${SCRIPT_DIR}/centscheduler.sh.in"
  "${SCRIPT_DIR}/centscheduler.sh")
configure_file("${SCRIPT_DIR}/ubuntu.conf.in"
  "${SCRIPT_DIR}/ubuntu.conf")

# Include Directories
include_directories("${INCLUDE_DIR}")

# Qt moc files.
qt4_wrap_cpp(QT_MOC_FILES
  "${INCLUDE_DIR}/modules/loader.hh"
  "${INCLUDE_DIR}/modules/handle.hh"
  "${INCLUDE_DIR}/modules/compatibility.hh"
)

# Library scheduler target.
add_library("schedulerlib"
  "STATIC"
  "${SRC_DIR}/broker.cc"
  "${SRC_DIR}/checks.cc"
  "${SRC_DIR}/commands.cc"
  "${SRC_DIR}/config.cc"
  "${SRC_DIR}/configuration.cc"
  "${SRC_DIR}/events.cc"
  "${SRC_DIR}/flapping.cc"
  "${SRC_DIR}/logging.cc"
  "${SRC_DIR}/nebmods.cc"
  "${SRC_DIR}/netutils.cc"
  "${SRC_DIR}/notifications.cc"
  "${SRC_DIR}/perfdata.cc"
  "${SRC_DIR}/sehandlers.cc"
  "${SRC_DIR}/sretention.cc"
  "${SRC_DIR}/utils.cc"
  "${SRC_DIR}/comments.cc"
  "${SRC_DIR}/downtime.cc"
  "${SRC_DIR}/macros.cc"
  "${SRC_DIR}/objects.cc"
  "${SRC_DIR}/shared.cc"
  "${SRC_DIR}/skiplist.cc"
  "${SRC_DIR}/statusdata.cc"
  "${SRC_DIR}/error.cc"
  "${SRC_DIR}/xcddefault.cc"
  "${SRC_DIR}/xdddefault.cc"
  "${SRC_DIR}/xodtemplate.cc"
  "${SRC_DIR}/xpddefault.cc"
  "${SRC_DIR}/xrddefault.cc"
  "${SRC_DIR}/xsddefault.cc"
  "${SRC_DIR}/modules/loader.cc"
  "${SRC_DIR}/modules/handle.cc"
  "${SRC_DIR}/modules/compatibility.cc"
  "${PERL_SOURCES}"
  ${QT_MOC_FILES}
)

# Centscheduler target.
add_executable("centscheduler"
  "${SRC_DIR}/nagios.cc"
)
get_property(SCHEDULER_COMPILE_FLAGS
  TARGET "centscheduler"
  PROPERTY COMPILE_FLAGS)
set(SCHEDULER_COMPILE_FLAGS "${SCHEDULER_COMPILE_FLAGS} ${PERL_COMPILE_FLAGS}")
set_target_properties("centscheduler" PROPERTIES
  COMPILE_FLAGS "${SCHEDULER_COMPILE_FLAGS}")
target_link_libraries("centscheduler"
  ${DL_LIBRARIES}
  ${MATH_LIBRARIES}
  ${SOCKET_LIBRARIES}
  ${THREAD_LIBRARIES}
  ${PERL_LINK_FLAGS}
  ${QT_LIBRARIES}
  "schedulerlib"
)

# Centschedulerstats target.
add_executable("centschedulerstats"
  "${SRC_DIR}/nagiostats.cc"
)

# Test configuration target.
add_executable("test_configuration"
  "${TEST_DIR}/test_configuration.cc"
  "${SRC_DIR}/configuration.cc"
  "${SRC_DIR}/error.cc")
target_link_libraries("test_configuration"
  ${QT_LIBRARIES}
)

add_test("test_configuration_basic"
  "test_configuration"
  "test.cfg")

# Package.
set(CPACK_PACKAGE_NAME "Centreon-Scheduler")
set(CPACK_PACKAGE_VENDOR "Merethis")
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 0)
set(CPACK_PACKAGE_VERSION_PATCH "0beta")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "Centreon Scheduler monitors your IT infrastructure.")
set(CPACK_PACKAGE_FILE_NAME
  "centreon-scheduler-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Centreon-Scheduler")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/license.txt")
set(CPACK_PACKAGE_CONTACT "Matthieu Kermagoret <mkermagoret@merethis.com>")
if (CPACK_BINARY_DEB)
  # Configure files.
  configure_file("${SCRIPT_DIR}/deb/postinst.in" "${SCRIPT_DIR}/deb/postinst")

  set(DEB_PACKAGE "Yes")
  set(CPACK_DEBIAN_PACKAGE_SECTION "net")
  set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${SCRIPT_DIR}/deb/postinst" )
else ()
  set(DEB_PACKAGE "No")
endif ()
if (CPACK_BINARY_RPM)
  # Configure files.
  configure_file("${SCRIPT_DIR}/rpm/postinst.in" "${SCRIPT_DIR}/rpm/postinst")

  set(RPM_PACKAGE "Yes")
  set(CPACK_RPM_PACKAGE_RELEASE 1)
  set(CPACK_RPM_PACKAGE_LICENSE "GPLv2")
  set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${SCRIPT_DIR}/rpm/postinst")
else ()
  set(RPM_PACKAGE "No")
endif ()
include(CPack)

# Print summary.
message(STATUS "")
message(STATUS "")
message(STATUS "Configuration Summary")
message(STATUS "---------------------")
message(STATUS "")
message(STATUS "    Version:                 ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
message(STATUS "    Embedded Perl:           ${EMBEDDED_PERL}")
if (EMBEDDEDPERL)
  message(STATUS "    Threaded Perl:           ${THREADED_PERL}")
endif ()
message(STATUS "    Extra Compilation Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "    Prefix:                  ${localstatedir}")
message(STATUS "    Binary Directory:        ${bindir}")
message(STATUS "    Configuration Directory: ${sysconfdir}")
message(STATUS "    Check Result Directory:  ${CHECK_RESULT_DIR}")
message(STATUS "    Temporary Directory:     ${TMPDIR}")
message(STATUS "    Startup Script:          ${STARTUP_SCRIPT}")
message(STATUS "    User:                    ${USER}")
message(STATUS "    Group:                   ${GROUP}")
message(STATUS "    Deb Package Creation:    ${DEB_PACKAGE}")
message(STATUS "    RPM Package Creation:    ${RPM_PACKAGE}")
message(STATUS "")
message(STATUS "")

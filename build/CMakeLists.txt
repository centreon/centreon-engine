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

#
# Global settings.
#

# Set necessary settings.
cmake_minimum_required(VERSION 2.8)
project("Centreon Engine" C CXX)
set(PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/..")
set(INC_DIR "${PROJECT_SOURCE_DIR}/inc")
set(SCRIPT_DIR "${PROJECT_SOURCE_DIR}/scripts")
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(TEST_DIR "${PROJECT_SOURCE_DIR}/test")
include_directories("${INC_DIR}")
include_directories("${INC_DIR}/compatibility")

# Version.
set(CENTREON_ENGINE_MAJOR 1)
set(CENTREON_ENGINE_MINOR 3)
set(CENTREON_ENGINE_PATCH 0)
set(CENTREON_ENGINE_VERSION "${CENTREON_ENGINE_MAJOR}.${CENTREON_ENGINE_MINOR}.${CENTREON_ENGINE_PATCH}")
message(STATUS "Generating version header (${CENTREON_ENGINE_VERSION}).")
configure_file("${INC_DIR}/com/centreon/engine/version.hh.in"
  "${INC_DIR}/com/centreon/engine/version.hh")

#
# Check and/or find required components.
#

# Check libraries to link with.
include(CheckLibraryExists)
message(STATUS "Checking for libm.")
check_library_exists("m" "ceil" "${CMAKE_LIBRARY_PATH}" MATH_LIB_FOUND)
if (MATH_LIB_FOUND)
  set(MATH_LIBRARIES "m")
endif ()
message(STATUS "Checking for libnsl.")
check_library_exists("nsl" "getservbyname" "${CMAKE_LIBRARY_PATH}" NSL_LIB_FOUND)
if (NSL_LIB_FOUND)
  set(NSL_LIBRARIES "nsl")
endif ()
message(STATUS "Checking for libsocket.")
check_library_exists("socket" "connect" "${CMAKE_LIBRARY_PATH}" SOCKET_LIB_FOUND)
if (SOCKET_LIB_FOUND)
  set(SOCKET_LIBRARIES "socket")
endif ()

# Find pthreads.
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
include(FindThreads)
if (NOT CMAKE_USE_PTHREADS_INIT)
  message(FATAL_ERROR "Could not find pthread's library.")
endif ()
set(PTHREAD_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")

# Find Centreon Clib's headers.
if (WITH_CENTREON_CLIB_INCLUDE_DIR)
  find_file(
    CLIB_HEADER_FOUND
    "com/centreon/clib/version.hh"
    PATHS "${WITH_CENTREON_CLIB_INCLUDE_DIR}"
    NO_DEFAULT_PATH)
  if (NOT CLIB_HEADER_FOUND)
    message(FATAL_ERROR "Could not find Centreon Clib's headers in ${WITH_CENTREON_CLIB_INCLUDE_DIR}.")
  endif ()
  include_directories("${WITH_CENTREON_CLIB_INCLUDE_DIR}")
elseif (CLIB_FOUND) # Was Centreon Clib detected with pkg-config ?
  if (CMAKE_CXX_FLAGS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CLIB_CFLAGS}")
  else ()
    set(CMAKE_CXX_FLAGS "${CLIB_CFLAGS}")
  endif ()
else ()
  find_path(CLIB_INCLUDE_DIR "com/centreon/clib/version.hh")
  if (NOT CLIB_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find Centreon Clib's headers (try WITH_CENTREON_CLIB_INCLUDE_DIR).")
  endif ()
  include_directories("${CLIB_INCLUDE_DIR}")
endif ()

# Find Centreon Clib's library.
if (WITH_CENTREON_CLIB_LIBRARIES)
  set(CLIB_LIBRARIES "${WITH_CENTREON_CLIB_LIBRARIES}")
elseif (WITH_CENTREON_CLIB_LIBRARY_DIR)
  find_library(
    CLIB_LIBRARIES
    "centreon_clib"
    PATHS "${WITH_CENTREON_CLIB_LIBRARY_DIR}"
    NO_DEFAULT_PATH)
  if (NOT CLIB_LIBRARIES)
    message(FATAL_ERROR "Could not find Centreon Clib's library in ${WITH_CENTREON_CLIB_LIBRARY_DIR}.")
  endif ()
elseif (CLIB_FOUND) # Was Centreon Clib detected with pkg-config ?
  set(CLIB_LIBRARIES "${CLIB_LDFLAGS}")
else ()
  find_library(CLIB_LIBRARIES "centreon_clib")
  if (NOT CLIB_LIBRARIES)
    message(FATAL_ERROR "Could not find Centreon Clib's library (try WITH_CENTREON_CLIB_LIBRARY_DIR or WITH_CENTREON_CLIB_LIBRARIES).")
  endif ()
endif ()

# Check headers.
include(CheckIncludeFileCXX)
check_include_file_cxx("algorithm" HAVE_ALGORITHM)
check_include_file_cxx("ctype.h" HAVE_CTYPE_H)
check_include_file_cxx("dirent.h" HAVE_DIRENT_H)
check_include_file_cxx("errno.h" HAVE_ERRNO_H)
check_include_file_cxx("exception" HAVE_EXCEPTION)
check_include_file_cxx("fcntl.h" HAVE_FCNTL_H)
check_include_file_cxx("fstream" HAVE_FSTREAM)
check_include_file_cxx("getopt.h" HAVE_GETOPT_H)
check_include_file_cxx("iomanip" HAVE_IOMANIP)
check_include_file_cxx("iostream" HAVE_IOSTREAM)
check_include_file_cxx("libgen.h" HAVE_LIBGEN_H)
check_include_file_cxx("limits.h" HAVE_LIMITS_H)
check_include_file_cxx("map" HAVE_MAP)
check_include_file_cxx("math.h" HAVE_MATH_H)
check_include_file_cxx("pthread.h" HAVE_PTHREAD_H)
check_include_file_cxx("regex.h" HAVE_REGEX_H)
check_include_file_cxx("signal.h" HAVE_SIGNAL_H)
check_include_file_cxx("sstream" HAVE_SSTREAM)
check_include_file_cxx("stdarg.h" HAVE_STDARG_H) # In logging.
check_include_file_cxx("stddef.h" HAVE_STDDEF_H)
check_include_file_cxx("stdio.h" HAVE_STDIO_H)
check_include_file_cxx("stdlib.h" HAVE_STDLIB_H)
check_include_file_cxx("string" HAVE_STRING)
check_include_file_cxx("string.h" HAVE_STRING_H)
check_include_file_cxx("syslog.h" HAVE_SYSLOG_H)
check_include_file_cxx("sys/ioctl.h" HAVE_SYS_IOCTL_H)
check_include_file_cxx("sys/mman.h" HAVE_SYS_MMAN_H)
check_include_file_cxx("sys/stat.h" HAVE_SYS_STAT_H)
check_include_file_cxx("sys/time.h" HAVE_SYS_TIME_H)
check_include_file_cxx("sys/types.h" HAVE_SYS_TYPES_H)
check_include_file_cxx("sys/wait.h" HAVE_SYS_WAIT_H)
check_include_file_cxx("time.h" HAVE_TIME_H)
check_include_file_cxx("unistd.h" HAVE_UNISTD_H)
if (NOT HAVE_ALGORITHM
    OR NOT HAVE_CTYPE_H
    OR NOT HAVE_DIRENT_H
    OR NOT HAVE_ERRNO_H
    OR NOT HAVE_EXCEPTION
    OR NOT HAVE_FCNTL_H
    OR NOT HAVE_FSTREAM
    OR NOT HAVE_IOMANIP
    OR NOT HAVE_IOSTREAM
    OR NOT HAVE_LIBGEN_H
    OR NOT HAVE_LIMITS_H
    OR NOT HAVE_MAP
    OR NOT HAVE_MATH_H
    OR NOT HAVE_PTHREAD_H
    OR NOT HAVE_REGEX_H
    OR NOT HAVE_SIGNAL_H
    OR NOT HAVE_SSTREAM
    OR NOT HAVE_STDARG_H
    OR NOT HAVE_STDDEF_H
    OR NOT HAVE_STDIO_H
    OR NOT HAVE_STDLIB_H
    OR NOT HAVE_STRING
    OR NOT HAVE_STRING_H
    OR NOT HAVE_SYSLOG_H
    OR NOT HAVE_SYS_IOCTL_H
    OR NOT HAVE_SYS_MMAN_H
    OR NOT HAVE_SYS_STAT_H
    OR NOT HAVE_SYS_TIME_H
    OR NOT HAVE_SYS_TYPES_H
    OR NOT HAVE_SYS_WAIT_H
    OR NOT HAVE_TIME_H
    OR NOT HAVE_UNISTD_H)
  message(FATAL_ERROR "Could not find all mandatory headers.")
endif ()

# Check functions.
include(CheckFunctionExists)
set(CMAKE_REQUIRED_LIBRARIES
  "${MATH_LIBRARIES}"
  "${SOCKET_LIBRARIES}"
  "${PTHREAD_LIBRARIES}")
check_function_exists("atoi" HAVE_ATOI)
check_function_exists("ceil" HAVE_CEIL)
check_function_exists("chdir" HAVE_CHDIR)
check_function_exists("close" HAVE_CLOSE)
check_function_exists("closedir" HAVE_CLOSEDIR)
check_function_exists("ctime" HAVE_CTIME)
check_function_exists("dirname" HAVE_DIRNAME)
check_function_exists("dup2" HAVE_DUP2)
check_function_exists("execvp" HAVE_EXECVP)
check_function_exists("exit" HAVE_EXIT)
check_function_exists("_exit" HAVE__EXIT)
check_function_exists("fclose" HAVE_FCLOSE)
check_function_exists("fdopen" HAVE_FDOPEN)
check_function_exists("fflush" HAVE_FFLUSH)
check_function_exists("fgets" HAVE_FGETS)
check_function_exists("floor" HAVE_FLOOR)
check_function_exists("fopen" HAVE_FOPEN)
check_function_exists("fork" HAVE_FORK)
check_function_exists("fprintf" HAVE_FPRINTF)
check_function_exists("fputc" HAVE_FPUTC)
check_function_exists("fputs" HAVE_FPUTS)
check_function_exists("fsync" HAVE_FSYNC)
check_function_exists("ftruncate" HAVE_FTRUNCATE)
check_function_exists("fwrite" HAVE_FWRITE)
check_function_exists("getopt" HAVE_GETOPT)
check_function_exists("getpid" HAVE_GETPID)
check_function_exists("gettimeofday" HAVE_GETTIMEOFDAY)
check_function_exists("gmtime_r" HAVE_GMTIME_R)
check_function_exists("ioctl" HAVE_IOCTL)
check_function_exists("kill" HAVE_KILL)
check_function_exists("localtime_r" HAVE_LOCALTIME_R)
check_function_exists("lseek" HAVE_LSEEK)
check_function_exists("memcpy" HAVE_MEMCPY)
check_function_exists("memmove" HAVE_MEMMOVE)
check_function_exists("memset" HAVE_MEMSET)
check_function_exists("mkstemp" HAVE_MKSTEMP)
check_function_exists("mktime" HAVE_MKTIME)
check_function_exists("mmap" HAVE_MMAP)
check_function_exists("munmap" HAVE_MUNMAP)
check_function_exists("open" HAVE_OPEN)
check_function_exists("opendir" HAVE_OPENDIR)
check_function_exists("openlog" HAVE_OPENLOG)
check_function_exists("pipe" HAVE_PIPE)
check_function_exists("printf" HAVE_PRINTF)
check_function_exists("pthread_mutex_lock" HAVE_PTHREAD_MUTEX_LOCK)
check_function_exists("pthread_mutex_unlock" HAVE_PTHREAD_MUTEX_UNLOCK)
check_function_exists("putchar" HAVE_PUTCHAR)
check_function_exists("putenv" HAVE_PUTENV)
check_function_exists("puts" HAVE_PUTS)
check_function_exists("qsort" HAVE_QSORT)
check_function_exists("rand" HAVE_RAND)
check_function_exists("read" HAVE_READ)
check_function_exists("readdir" HAVE_READDIR)
check_function_exists("regcomp" HAVE_REGCOMP)
check_function_exists("regexec" HAVE_REGEXEC)
check_function_exists("regfree" HAVE_REGFREE)
check_function_exists("rename" HAVE_RENAME)
check_function_exists("setbuf" HAVE_SETBUF)
check_function_exists("setenv" HAVE_SETENV)
check_function_exists("signal" HAVE_SIGNAL)
check_function_exists("snprintf" HAVE_SNPRINTF)
check_function_exists("sprintf" HAVE_SPRINTF)
check_function_exists("sscanf" HAVE_SSCANF)
check_function_exists("strcat" HAVE_STRCAT)
check_function_exists("strchr" HAVE_STRCHR)
check_function_exists("strcmp" HAVE_STRCMP)
check_function_exists("strcpy" HAVE_STRCPY)
check_function_exists("strcspn" HAVE_STRCSPN)
check_function_exists("strerror" HAVE_STRERROR)
check_function_exists("strftime" HAVE_STRFTIME)
check_function_exists("strlen" HAVE_STRLEN)
check_function_exists("strpbrk" HAVE_STRPBRK)
check_function_exists("strstr" HAVE_STRSTR)
check_function_exists("strtod" HAVE_STRTOD)
check_function_exists("strtok" HAVE_STRTOK)
check_function_exists("strtoul" HAVE_STRTOUL)
check_function_exists("syslog" HAVE_SYSLOG)
check_function_exists("time" HAVE_TIME)
check_function_exists("toupper" HAVE_TOUPPER)
check_function_exists("tzset" HAVE_TZSET)
check_function_exists("umask" HAVE_UMASK)
check_function_exists("unlink" HAVE_UNLINK)
check_function_exists("unsetenv" HAVE_UNSETENV)
check_function_exists("vsnprintf" HAVE_VSNPRINTF)
check_function_exists("waitpid" HAVE_WAITPID)
check_function_exists("write" HAVE_WRITE)
if (NOT HAVE_ATOI
    OR NOT HAVE_CEIL
    OR NOT HAVE_CHDIR
    OR NOT HAVE_CLOSE
    OR NOT HAVE_CLOSEDIR
    OR NOT HAVE_CTIME
    OR NOT HAVE_DIRNAME
    OR NOT HAVE_DUP2
    OR NOT HAVE_EXECVP
    OR NOT HAVE_EXIT
    OR NOT HAVE__EXIT
    OR NOT HAVE_FCLOSE
    OR NOT HAVE_FDOPEN
    OR NOT HAVE_FFLUSH
    OR NOT HAVE_FGETS
    OR NOT HAVE_FLOOR
    OR NOT HAVE_FOPEN
    OR NOT HAVE_FORK
#    OR NOT HAVE_FPRINTF # Does not work with Clang.
    OR NOT HAVE_FPUTC
    OR NOT HAVE_FPUTS
    OR NOT HAVE_FSYNC
    OR NOT HAVE_FTRUNCATE
    OR NOT HAVE_FWRITE
    OR NOT HAVE_GETOPT
    OR NOT HAVE_GETPID
    OR NOT HAVE_GETTIMEOFDAY
    OR NOT HAVE_GMTIME_R
    OR NOT HAVE_IOCTL
    OR NOT HAVE_KILL
    OR NOT HAVE_LOCALTIME_R
    OR NOT HAVE_LSEEK
    OR NOT HAVE_MEMCPY
    OR NOT HAVE_MEMMOVE
    OR NOT HAVE_MEMSET
    OR NOT HAVE_MKSTEMP
    OR NOT HAVE_MKTIME
    OR NOT HAVE_MMAP
    OR NOT HAVE_MUNMAP
    OR NOT HAVE_OPEN
    OR NOT HAVE_OPENDIR
    OR NOT HAVE_OPENLOG
    OR NOT HAVE_PIPE
    OR NOT HAVE_PRINTF
    OR NOT HAVE_PTHREAD_MUTEX_LOCK
    OR NOT HAVE_PTHREAD_MUTEX_UNLOCK
    OR NOT HAVE_PUTCHAR
    OR NOT HAVE_PUTENV
    OR NOT HAVE_PUTS
    OR NOT HAVE_QSORT
    OR NOT HAVE_RAND
    OR NOT HAVE_READ
    OR NOT HAVE_READDIR
    OR NOT HAVE_REGCOMP
    OR NOT HAVE_REGEXEC
    OR NOT HAVE_REGFREE
    OR NOT HAVE_RENAME
    OR NOT HAVE_SETBUF
    OR NOT HAVE_SETENV
    OR NOT HAVE_SIGNAL
    OR NOT HAVE_SNPRINTF
    OR NOT HAVE_SPRINTF
    OR NOT HAVE_SSCANF
    OR NOT HAVE_STRCAT
    OR NOT HAVE_STRCHR
    OR NOT HAVE_STRCMP
    OR NOT HAVE_STRCPY
    OR NOT HAVE_STRCSPN
    OR NOT HAVE_STRERROR
    OR NOT HAVE_STRFTIME
    OR NOT HAVE_STRLEN
    OR NOT HAVE_STRPBRK
    OR NOT HAVE_STRSTR
    OR NOT HAVE_STRTOD
    OR NOT HAVE_STRTOK
    OR NOT HAVE_STRTOUL
    OR NOT HAVE_SYSLOG
    OR NOT HAVE_TIME
    OR NOT HAVE_TOUPPER
    OR NOT HAVE_TZSET
    OR NOT HAVE_UMASK
    OR NOT HAVE_UNLINK
    OR NOT HAVE_UNSETENV
    OR NOT HAVE_VSNPRINTF
    OR NOT HAVE_WAITPID
    OR NOT HAVE_WRITE)
  message(FATAL_ERROR "Could not find all mandatory functions.")
endif ()

# Check timezone informations.
include(CheckStructHasMember)
message(STATUS "Checking for tm_zone member in tm struct.")
check_struct_has_member("tm" "tm_zone" "time.h" HAVE_TM_ZONE)
if (HAVE_TM_ZONE)
  add_definitions(-DHAVE_TM_ZONE)
endif ()
include(CheckSymbolExists)
message(STATUS "Checking for symbol tzname.")
check_symbol_exists("tzname" "time.h" HAVE_TZNAME)
if (HAVE_TZNAME)
  add_definitions(-DHAVE_TZNAME)
endif ()

#
# Options.
#

# Main directory.
if (WITH_PREFIX)
  set(CMAKE_INSTALL_PREFIX "${WITH_PREFIX}")
endif ()

# Executable directory.
if (WITH_PREFIX_BIN)
  set(PREFIX_BIN "${WITH_PREFIX_BIN}")
else ()
  set(PREFIX_BIN "${CMAKE_INSTALL_PREFIX}/bin")
endif ()

# Configuration directory.
if (WITH_PREFIX_CONF)
  set(PREFIX_CONF "${WITH_PREFIX_CONF}")
else ()
  set(PREFIX_CONF "${CMAKE_INSTALL_PREFIX}/etc")
endif ()

# Library directory.
if (WITH_PREFIX_LIB)
  set(PREFIX_LIB "${WITH_PREFIX_LIB}")
else ()
  set(PREFIX_LIB "${CMAKE_INSTALL_PREFIX}/lib/centreon-engine")
endif ()

# Development headers directory.
if (WITH_PREFIX_INC)
  set(PREFIX_INC "${WITH_PREFIX_INC}")
else ()
  set(PREFIX_INC "${CMAKE_INSTALL_PREFIX}/include/centreon-engine")
endif ()

# pkg-config directory.
option(WITH_PKGCONFIG_SCRIPT "Generate and install pkg-config script." ON)
if (WITH_PKGCONFIG_SCRIPT)
  # Generate pkg-config file.
  message(STATUS "Generating pkg-config file.")
  configure_file("${SCRIPT_DIR}/centengine.pc.in"
    "${SCRIPT_DIR}/centengine.pc")

  # pkg-config file install directory.
  if (WITH_PKGCONFIG_DIR)
    set(PKGCONFIG_DIR "${WITH_PKGCONFIG_DIR}")
  else ()
    set(PKGCONFIG_DIR "/usr/lib/pkgconfig")
  endif ()

  # Install rule.
  install(FILES "${SCRIPT_DIR}/centengine.pc"
    DESTINATION "${PKGCONFIG_DIR}"
    COMPONENT "runtime")

endif ()

# var directory.
if (WITH_VAR_DIR)
  set(VAR_DIR "${WITH_VAR_DIR}")
else ()
  set(VAR_DIR "${CMAKE_INSTALL_PREFIX}/var")
endif ()

# Log archive dir.
if (WITH_LOG_ARCHIVE_DIR)
  set(LOG_ARCHIVE_DIR "${WITH_LOG_ARCHIVE_DIR}")
else ()
  set(LOG_ARCHIVE_DIR "${VAR_DIR}/archives")
endif ()

# RW dir.
if (WITH_RW_DIR)
  set(RW_DIR "${WITH_RW_DIR}")
else ()
  set(RW_DIR "${VAR_DIR}/rw")
endif ()

# User used to run Centreon Engine.
if (WITH_USER)
  set(USER "${WITH_USER}")
else ()
  set(USER "root")
endif ()

# Group used to run Centreon Engine.
if (WITH_GROUP)
  set(GROUP "${WITH_GROUP}")
else ()
  set(GROUP "root")
endif ()

# Check OS distributor.
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(STATUS "Attempting to determine OS distributor.")
  execute_process(COMMAND "lsb_release" "--short" "--id"
    RESULT_VARIABLE RETCODE
    OUTPUT_VARIABLE OS_DISTRIBUTOR
    ERROR_QUIET)
  if (RETCODE EQUAL 0)
    string(REGEX REPLACE "\n$" "" OS_DISTRIBUTOR "${OS_DISTRIBUTOR}")
  else ()
    set(OS_DISTRIBUTOR "${CMAKE_SYSTEM_NAME}")
  endif ()
elseif ()
  set(OS_DISTRIBUTOR "${CMAKE_SYSTEM_NAME}")
endif ()

# Set startup script to auto if not define.
if (NOT WITH_STARTUP_SCRIPT)
  set(WITH_STARTUP_SCRIPT "auto")
endif ()

# Check which startup script to use.
if (WITH_STARTUP_SCRIPT STREQUAL "auto")
  if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (OS_DISTRIBUTOR STREQUAL "Ubuntu")
      set(WITH_STARTUP_SCRIPT "upstart")
    else ()
      set(WITH_STARTUP_SCRIPT "sysv")
    endif ()
  else ()
    message(STATUS "Centreon Engine does not provide startup script for ${CMAKE_SYSTEM_NAME}.")
  endif ()
endif ()

# Create upstart file.
if (WITH_STARTUP_SCRIPT STREQUAL "upstart")
  # Generate Upstart script.
  message(STATUS "Generating upstart script.")
  configure_file("${SCRIPT_DIR}/upstart.conf.in"
    "${SCRIPT_DIR}/upstart.conf")

  # Startup dir.
  if (WITH_STARTUP_DIR)
    set(STARTUP_DIR "${WITH_STARTUP_DIR}")
  else ()
    set(STARTUP_DIR "/etc/init")
  endif ()

  # Script install rule.
  install(FILES "${SCRIPT_DIR}/upstart.conf"
    DESTINATION "${STARTUP_DIR}"
    COMPONENT "runtime"
    RENAME "centengine.conf")

  # String printed in summary.
  set(STARTUP_SCRIPT "Upstart configuration file")

# Create SysV start script.
elseif (WITH_STARTUP_SCRIPT STREQUAL "sysv")
  # Lock file.
  if (WITH_LOCK_FILE)
    set(LOCK_FILE "${WITH_LOCK_FILE}")
  else ()
    set(LOCK_FILE "/var/lock/subsys/centengine.lock")
  endif ()
  string(REGEX REPLACE "/[^/]*$" "" LOCK_DIR "${LOCK_FILE}")

  # PID file.
  if (WITH_PID_FILE)
    set(PID_FILE "${WITH_PID_FILE}")
  else ()
    set(PID_FILE "/var/run/centengine.pid")
  endif ()
  string(REGEX REPLACE "/[^/]*$" "" PID_DIR "${PID_FILE}")

  # Generate SysV script.
  message(STATUS "Generating generic startup script.")
  configure_file("${SCRIPT_DIR}/centengine.sh.in"
    "${SCRIPT_DIR}/centengine.sh")

  # Startup dir.
  if (WITH_STARTUP_DIR)
    set(STARTUP_DIR "${WITH_STARTUP_DIR}")
  else ()
    set(STARTUP_DIR "/etc/init.d")
  endif ()

  # Script install rule.
  install(PROGRAMS "${SCRIPT_DIR}/centengine.sh"
    DESTINATION "${STARTUP_DIR}"
    COMPONENT "runtime"
    RENAME "centengine")

  # String printed in summary.
  set(STARTUP_SCRIPT "SysV-style script")

else ()
  # Default.
  message(STATUS "Invalid value for option WITH_STARTUP_SCRIPT (must be one of 'auto', 'sysv' or 'upstart').")
  set(STARTUP_SCRIPT "disabled")
endif ()

# Unit test.
option(WITH_TESTING "Build unit tests." OFF)
if (WITH_TESTING)
  set(CTEST_TESTING_TIMEOUT 20)
  include(CTest)
  include_directories("${PROJECT_SOURCE_DIR}")
endif ()

# Configure files.
configure_file("${PROJECT_SOURCE_DIR}/build/doc/Doxyfile.in"
  "${PROJECT_SOURCE_DIR}/build/doc/Doxyfile")
configure_file("${PROJECT_SOURCE_DIR}/build/doc/header.tex.in"
  "${PROJECT_SOURCE_DIR}/build/doc/header.tex")
configure_file("${INC_DIR}/compatibility/common.h.in"
  "${INC_DIR}/compatibility/common.h")

# Locations definitions
add_definitions(-DDEFAULT_STATUS_FILE="${VAR_DIR}/status.dat")
add_definitions(-DDEFAULT_LOG_FILE="${VAR_DIR}/centengine.log")
add_definitions(-DDEFAULT_LOG_ARCHIVE_PATH="${LOG_ARCHIVE_DIR}")
add_definitions(-DDEFAULT_DEBUG_FILE="${VAR_DIR}/centengine.debug")
add_definitions(-DDEFAULT_RETENTION_FILE="${VAR_DIR}/retention.dat")
add_definitions(-DDEFAULT_COMMAND_FILE="${RW_DIR}/centengine.cmd")
add_definitions(-DDEFAULT_CONFIG_FILE="${PREFIX_CONF}/centengine.cfg")
add_definitions(-DDEFAULT_OBJECT_CACHE_FILE="${VAR_DIR}/objects.cache")
add_definitions(-DDEFAULT_PRECACHED_OBJECT_FILE="${VAR_DIR}/objects.precache")

#
# Targets.
#

# Library engine target.
add_library("cce_core" STATIC
  # Sources.
  "${SRC_DIR}/broker.cc"
  "${SRC_DIR}/checks.cc"
  "${SRC_DIR}/config.cc"
  "${SRC_DIR}/events.cc"
  "${SRC_DIR}/flapping.cc"
  "${SRC_DIR}/globals.cc"
  "${SRC_DIR}/hash_timed_event.cc"
  "${SRC_DIR}/nebmods.cc"
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
  # Headers.
  "${INC_DIR}/com/centreon/engine/broker.hh"
  "${INC_DIR}/com/centreon/engine/checks.hh"
  "${INC_DIR}/com/centreon/engine/circular_buffer.hh"
  "${INC_DIR}/com/centreon/engine/comments.hh"
  "${INC_DIR}/com/centreon/engine/common.hh"
  "${INC_DIR}/com/centreon/engine/config.hh"
  "${INC_DIR}/com/centreon/engine/downtime.hh"
  "${INC_DIR}/com/centreon/engine/error.hh"
  "${INC_DIR}/com/centreon/engine/events.hh"
  "${INC_DIR}/com/centreon/engine/flapping.hh"
  "${INC_DIR}/com/centreon/engine/globals.hh"
  "${INC_DIR}/com/centreon/engine/hash_timed_event.hh"
  "${INC_DIR}/com/centreon/engine/logging.hh"
  "${INC_DIR}/com/centreon/engine/macros.hh"
  "${INC_DIR}/com/centreon/engine/nebcallbacks.hh"
  "${INC_DIR}/com/centreon/engine/neberrors.hh"
  "${INC_DIR}/com/centreon/engine/nebmods.hh"
  "${INC_DIR}/com/centreon/engine/nebmodules.hh"
  "${INC_DIR}/com/centreon/engine/nebstructs.hh"
  "${INC_DIR}/com/centreon/engine/notifications.hh"
  "${INC_DIR}/com/centreon/engine/objects.hh"
  "${INC_DIR}/com/centreon/engine/perfdata.hh"
  "${INC_DIR}/com/centreon/engine/sehandlers.hh"
  "${INC_DIR}/com/centreon/engine/shared.hh"
  "${INC_DIR}/com/centreon/engine/skiplist.hh"
  "${INC_DIR}/com/centreon/engine/sretention.hh"
  "${INC_DIR}/com/centreon/engine/statusdata.hh"
  "${INC_DIR}/com/centreon/engine/utils.hh"
  "${INC_DIR}/com/centreon/engine/version.hh"
  "${INC_DIR}/com/centreon/engine/xcddefault.hh"
  "${INC_DIR}/com/centreon/engine/xdddefault.hh"
  "${INC_DIR}/com/centreon/engine/xodtemplate.hh"
  "${INC_DIR}/com/centreon/engine/xpddefault.hh"
  "${INC_DIR}/com/centreon/engine/xrddefault.hh"
  "${INC_DIR}/com/centreon/engine/xsddefault.hh"
)

# Subdirectories with core features.
add_subdirectory("broker")
add_subdirectory("checks")
add_subdirectory("conf")
add_subdirectory("configuration")
add_subdirectory("commands")
add_subdirectory("compatibility")
add_subdirectory("doc")
add_subdirectory("events")
add_subdirectory("logging")
add_subdirectory("macros")
add_subdirectory("modules")
add_subdirectory("objects")
add_subdirectory("scripts")

# Link target with required libraries.
target_link_libraries("cce_core"
  ${MATH_LIBRARIES}
  ${PTHREAD_LIBRARIES}
  ${SOCKET_LIBRARIES}
  ${CLIB_LIBRARIES}
  "cce_configuration_applier"
  "cce_events"
  "cce_logging"
  "cce_broker"
  "cce_configuration"
  "cce_checks"
  "cce_macros"
  "cce_commands"
  "cce_compatibility"
  "cce_objects"
)

# centengine target.
add_executable("centengine"
  "${SRC_DIR}/main.cc")
if (CMAKE_COMPILER_IS_GNUCXX)
  target_link_libraries("centengine"
    "-Wl,-whole-archive -lcce_objects -L./objects -lcce_compatibility -L./compatibility -Wl,-no-whole-archive")
endif ()
get_property(CENTENGINE_BINARY
    TARGET "centengine"
    PROPERTY LOCATION)
configure_file("${TEST_DIR}/paths.hh.in" "${TEST_DIR}/paths.hh")

# Link centengine with required libraries.
target_link_libraries("centengine"
  "cce_core")

# centenginestats target.
add_executable("centenginestats"
  "${SRC_DIR}/centenginestats.cc")
target_link_libraries("centenginestats" ${CLIB_LIBRARIES})

#
# Install stuff.
#

# Install rules.
install(TARGETS "centengine" "centenginestats"
  DESTINATION "${PREFIX_BIN}"
  COMPONENT "runtime")

# Create directories.
install(CODE "
  function(mkdir_chown user group path)
    if (APPLE OR (UNIX AND NOT CYGWIN))
      if (NOT EXISTS \"\${path}\")
        file(MAKE_DIRECTORY \"\${path}\")
        execute_process(COMMAND \"chown\" \"\${user}:\${group}\" \"\${path}\")
      endif ()
    else()
      file(MAKE_DIRECTORY \"\${path}\")
    endif ()
  endfunction()

  function(touch_chown user group file)
    if (APPLE OR (UNIX AND NOT CYGWIN))
      if (NOT EXISTS \"\${file}\")
        file(WRITE \"\${file}\" \"\")
        execute_process(COMMAND \"chown\" \"\${user}:\${group}\" \"\${file}\")
      endif ()
    else()
      file(WRITE \"{file}\" \"\")
    endif ()
  endfunction()

  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${CMAKE_INSTALL_PREFIX}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_BIN}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_CONF}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_CONF}/objects\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_INC}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${PREFIX_LIB}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${VAR_DIR}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${LOG_ARCHIVE_DIR}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${RW_DIR}\")
  if (LOCK_DIR)
    mkdir_chown(\"${USER}\" \"${GROUP}\" \"${LOCK_DIR}\")
  endif ()
  if (PID_DIR)
    mkdir_chown(\"${USER}\" \"${GROUP}\" \"${PID_DIR}\")
  endif ()

  touch_chown(\"${USER}\" \"${GROUP}\" \"${VAR_DIR}/status.dat\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${VAR_DIR}/centengine.log\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${VAR_DIR}/centengine.debug\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${VAR_DIR}/retention.dat\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${RW_DIR}/centengine.cmd\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${VAR_DIR}/objects.cache\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${VAR_DIR}/objects.precache\")
")

# Install header files for development.
install(DIRECTORY "${INC_DIR}/"
  DESTINATION "${PREFIX_INC}"
  COMPONENT "development"
  FILES_MATCHING PATTERN "*.hh"
  PATTERN "${INC_DIR}/compatibility/" EXCLUDE)

#
# Packaging.
#

include("package.cmake")

#
# Unit tests.
#

if (WITH_TESTING)
  #
  # Error tests.
  #
  # Integer insertion.
  set(TEST_NAME "error_insert_integer")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/error/insert_integer.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Unsigned integer insertion.
  set(TEST_NAME "error_insert_unsigned_integer")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/error/insert_unsigned_integer.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Long insertion.
  set(TEST_NAME "error_insert_long")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/error/insert_long.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Long long insertion.
  set(TEST_NAME "error_insert_long_long")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/error/insert_long_long.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Unsigned long long insertion.
  set(TEST_NAME "error_insert_unsigned_long_long")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/error/insert_unsigned_long_long.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Char insertion.
  set(TEST_NAME "error_insert_char")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/error/insert_char.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Char array (C string) insertion.
  set(TEST_NAME "error_insert_char_array")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/error/insert_char_array.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # String insertion.
  set(TEST_NAME "error_insert_string")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/error/insert_string.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Mixed insertions.
  set(TEST_NAME "error_insert_mixed")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/error/insert_mixed.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Copy constructor.
  set(TEST_NAME "error_copy_ctor")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/error/copy_ctor.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Assignment operator.
  set(TEST_NAME "error_assignment")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/error/assignment.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")


  #
  # Notifications tests.
  #
  # First notification delay on host.
  set(TEST_NAME "notifications_first_notif_delay_host")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/notifications/first_notif_delay/common.cc"
    "${TEST_DIR}/notifications/first_notif_delay/common.hh"
    "${TEST_DIR}/notifications/first_notif_delay/host.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # First notification delay on service.
  set(TEST_NAME "notifications_first_notif_delay_service")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/notifications/first_notif_delay/common.cc"
    "${TEST_DIR}/notifications/first_notif_delay/common.hh"
    "${TEST_DIR}/notifications/first_notif_delay/service.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # First notification delay on host recovery.
  set(TEST_NAME "notifications_first_notif_delay_host_recovery")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/notifications/first_notif_delay/common.cc"
    "${TEST_DIR}/notifications/first_notif_delay/common.hh"
    "${TEST_DIR}/notifications/first_notif_delay/host_recovery.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # First notification delay on service recovery.
  set(TEST_NAME "notifications_first_notif_delay_service_recovery")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/notifications/first_notif_delay/common.cc"
    "${TEST_DIR}/notifications/first_notif_delay/common.hh"
    "${TEST_DIR}/notifications/first_notif_delay/service_recovery.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")

  #
  # Stats tests.
  #
  # Non-existent configuration file.
  set(TEST_NAME "running_stats_no_config_file")
  add_test("${TEST_NAME}"
    "centenginestats" "-c" "${TEST_DIR}/running/etc/does_not_exist.cfg")
  set_tests_properties("${TEST_NAME}" PROPERTIES WILL_FAIL TRUE)
  # Non-existent status file.
  set(TEST_NAME "running_stats_no_status_file")
  add_test("${TEST_NAME}"
    "centenginestats" "-c" "${TEST_DIR}/running/etc/no_status_file.cfg")
  set_tests_properties("${TEST_NAME}" PROPERTIES WILL_FAIL TRUE)

  #
  # Engine tests.
  #
  set(CENTENGINE_RUNNER "centengine_must_run")
  add_executable("${CENTENGINE_RUNNER}"
    "${TEST_DIR}/running/centengine_must_run.cc")
  target_link_libraries("${CENTENGINE_RUNNER}" "cce_core")
  # Help flag.
  add_test("running_engine_help"
    "centengine" "-h")
  # License flag.
  add_test("running_engine_license"
    "centengine" "-V")
  # Without any service.
  add_test("running_engine_no_service"
    "centengine" "-v" "${TEST_DIR}/running/etc/no_service.cfg")
  set_tests_properties("running_engine_no_service"
    PROPERTIES WILL_SUCCESS TRUE)
  # Empty host group.
  set(TEST_NAME "running_engine_empty_hostgroup")
  add_test("${TEST_NAME}"
    "centengine" "-v" "${TEST_DIR}/running/etc/empty_hostgroup.cfg")
  # Empty service group.
  set(TEST_NAME "running_engine_empty_servicegroup")
  add_test("${TEST_NAME}"
    "centengine" "-v" "${TEST_DIR}/running/etc/empty_servicegroup.cfg")
  # Empty host group assignment error.
  set(TEST_NAME "running_engine_empty_hostgroup_assignment_error")
  add_test("${TEST_NAME}"
    "centengine" "-v" "${TEST_DIR}/running/etc/empty_hostgroup_assignment_error.cfg")
  set_tests_properties("${TEST_NAME}" PROPERTIES WILL_FAIL TRUE)
  # Empty host group assignment success.
  set(TEST_NAME "running_engine_empty_hostgroup_assignment_success")
  add_test("${TEST_NAME}"
    "centengine" "-v" "${TEST_DIR}/running/etc/empty_hostgroup_assignment_success.cfg")
  # Invalid contactgroup on contact.
  set(TEST_NAME "invalid_contactgroup_on_contact")
  add_test("${TEST_NAME}"
    "centengine" "-v" "${TEST_DIR}/running/etc/invalid_contactgroup_on_contact.cfg")
  set_tests_properties("${TEST_NAME}"
    PROPERTIES WILL_FAIL TRUE)
  # broker_module with only path to module.
  set(TEST_NAME "running_engine_broker_module_without_args")
  add_test("${TEST_NAME}"
    "centengine" "-v" "${TEST_DIR}/running/etc/broker_module_without_args.cfg")
  # Check that all symbols required by modules are present.
  set(TEST_NAME "running_engine_broker_modules_symbols")
  add_library("running_engine_broker_modules_symbols_checker" SHARED
    "${TEST_DIR}/running/modules_symbols_checker.cc")
  set_property(TARGET "running_engine_broker_modules_symbols_checker"
    PROPERTY PREFIX "")
  add_library("running_engine_broker_modules_symbols" SHARED
    "${TEST_DIR}/running/modules_symbols.cc")
  set_property(TARGET "running_engine_broker_modules_symbols"
    PROPERTY PREFIX "")
  get_property(EXTERNAL_COMMAND_PATH
    TARGET "externalcmd"
    PROPERTY LOCATION)
  get_property(MODULE_PATH
    TARGET "running_engine_broker_modules_symbols_checker"
    PROPERTY LOCATION)
  get_property(MODULE_ARGS
    TARGET "running_engine_broker_modules_symbols"
    PROPERTY LOCATION)
  set(CFG_CONTENT
"command_file=/tmp/running_engine_broker_modules_symbols.tmp
broker_module=${EXTERNAL_COMMAND_PATH}
broker_module=${MODULE_PATH} ${MODULE_ARGS}")
  configure_file("${TEST_DIR}/running/etc/minimal.cfg.in"
    "${TEST_DIR}/running/etc/modules_symbols.cfg")
  add_test("${TEST_NAME}"
    "centengine" "${TEST_DIR}/running/etc/modules_symbols.cfg")
  # Check that the webservice module can be loaded.
  if (WITH_WEBSERVICE)
    set(TEST_NAME "running_engine_broker_modules_webservice")
    unset(MODULE_ARGS)
    get_property(MODULE_ARGS
      TARGET "webservice"
      PROPERTY LOCATION)
    set(CFG_CONTENT
"command_file=/tmp/running_engine_broker_modules_webservice.tmp
broker_module=${EXTERNAL_COMMAND_PATH}
broker_module=${MODULE_PATH} ${MODULE_ARGS}")
    configure_file("${TEST_DIR}/running/etc/minimal.cfg.in"
      "${TEST_DIR}/running/etc/modules_webservice.cfg")
    add_test("${TEST_NAME}"
      "centengine" "${TEST_DIR}/running/etc/modules_webservice.cfg")
  endif ()
  # Active check of host with no command.
  set(CFG_CONTENT
    "cfg_file=${TEST_DIR}/running/etc/host_with_no_check_cmd_objects.cfg")
  configure_file("${TEST_DIR}/running/etc/minimal.cfg.in"
    "${TEST_DIR}/running/etc/host_with_no_check_cmd.cfg")
  set(TEST_NAME "running_host_check_with_no_check_cmd")
  add_test("${TEST_NAME}"
    "${CENTENGINE_RUNNER}" "${TEST_DIR}/running/etc/host_with_no_check_cmd.cfg" "5")
  # Active check of service with no command.
  set(CFG_CONTENT
    "cfg_file=${TEST_DIR}/running/etc/service_with_no_check_cmd_objects.cfg")
  configure_file("${TEST_DIR}/running/etc/minimal.cfg.in"
    "${TEST_DIR}/running/etc/service_with_no_check_cmd.cfg")
  set(TEST_NAME "running_service_check_with_no_check_cmd")
  add_test("${TEST_NAME}"
    "centengine" "-v" "${TEST_DIR}/running/etc/service_with_no_check_cmd.cfg")
  set_tests_properties("${TEST_NAME}"
    PROPERTIES WILL_FAIL TRUE)
  # Logs are generated for broker modules.
  set(TEST_NAME "running_logs_for_modules")
  add_library("logs_for_modules" SHARED
    "${TEST_DIR}/running/logs_for_modules.cc")
  unset(MODULE_PATH)
  get_property(MODULE_PATH
    TARGET "logs_for_modules"
    PROPERTY LOCATION)
  set(CFG_CONTENT
    "broker_module=${MODULE_PATH}")
  configure_file("${TEST_DIR}/running/etc/minimal.cfg.in"
    "${TEST_DIR}/running/etc/logs_for_modules.cfg")
  add_test("${TEST_NAME}"
    "centengine" "${TEST_DIR}/running/etc/logs_for_modules.cfg")
endif ()

#
# Print summary.
#

message(STATUS "")
message(STATUS "")
message(STATUS "Configuration Summary")
message(STATUS "---------------------")
message(STATUS "")
message(STATUS "  Project")
message(STATUS "    - Name                       Centreon Engine")
message(STATUS "    - Version                    ${CENTREON_ENGINE_VERSION}")
message(STATUS "")
message(STATUS "  System")
message(STATUS "    - Name                       ${CMAKE_SYSTEM_NAME}")
message(STATUS "    - Version                    ${CMAKE_SYSTEM_VERSION}")
message(STATUS "    - Processor                  ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "")
message(STATUS "  Build")
message(STATUS "    - Compiler                   ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID})")
message(STATUS "    - Extra compilation flags    ${CMAKE_CXX_FLAGS}")
message(STATUS "    - External commands module   enabled")
if (WITH_WEBSERVICE)
  message(STATUS "    - Webservice module          enabled")
else ()
  message(STATUS "    - Webservice module          disabled")
endif ()
if (WITH_TESTING)
  message(STATUS "    - Unit tests                 enabled")
else ()
  message(STATUS "    - Unit tests                 disabled")
endif ()
if (WITH_PKGCONFIG_SCRIPT)
  message(STATUS "    - pkg-config script          enabled")
else ()
  message(STATUS "    - pkg-config script          disabled")
endif ()
message(STATUS "    - Startup script             ${STARTUP_SCRIPT}")
message(STATUS "")
message(STATUS "  Install")
message(STATUS "    - Prefix                     ${CMAKE_INSTALL_PREFIX}")
message(STATUS "    - Binary prefix              ${PREFIX_BIN}")
message(STATUS "    - Configuration prefix       ${PREFIX_CONF}")
message(STATUS "    - Library prefix             ${PREFIX_LIB}")
message(STATUS "    - Include prefix             ${PREFIX_INC}")
message(STATUS "    - var directory              ${VAR_DIR}")
message(STATUS "    - Log archive directory      ${LOG_ARCHIVE_DIR}")
message(STATUS "    - RW directory               ${RW_DIR}")
if (WITH_PKGCONFIG_SCRIPT)
  message(STATUS "    - pkg-config directory       ${PKGCONFIG_DIR}")
endif ()
if (STARTUP_DIR)
  message(STATUS "    - Startup directory          ${STARTUP_DIR}")
endif ()
message(STATUS "    - User                       ${USER}")
message(STATUS "    - Group                      ${GROUP}")
message(STATUS "    - Package                    ${PACKAGE_LIST}")
message(STATUS "")

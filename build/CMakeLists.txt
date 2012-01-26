##
## Copyright 2011 Merethis
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

# Global options.
cmake_minimum_required(VERSION 2.8)
project("Centreon Engine" C CXX)
set(ENGINE_MAJOR 1)
set(ENGINE_MINOR 1)
set(ENGINE_PATCH 0)
set(ENGINE_VERSION "${ENGINE_MAJOR}.${ENGINE_MINOR}.${ENGINE_PATCH}")
add_definitions("-DENGINE_MAJOR=${ENGINE_MAJOR}")
add_definitions("-DENGINE_MINOR=${ENGINE_MINOR}")
add_definitions("-DENGINE_PATCH=\"${ENGINE_PATCH}\"")
add_definitions("-DENGINE_VERSION=\"${ENGINE_VERSION}\"")

# Testing.
set(PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/..")
if (WITH_TESTING)
  include(CTest)
endif ()

# Check libraries to link with.
include(CheckLibraryExists)
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

# Find Qt.
find_package(Qt4 COMPONENTS QtCore REQUIRED)
include(${QT_USE_FILE})

# Set directories.
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/inc")
set(SCRIPT_DIR "${PROJECT_SOURCE_DIR}/scripts")
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
if (WITH_TESTING)
  set(TEST_DIR "${PROJECT_SOURCE_DIR}/test")
endif ()

# Check headers.
include(CheckIncludeFileCXX)
check_include_file_cxx("arpa/inet.h" HAVE_ARPA_INET_H)
check_include_file_cxx("ctype.h" HAVE_CTYPE_H)
check_include_file_cxx("dirent.h" HAVE_DIRENT_H)
check_include_file_cxx("errno.h" HAVE_ERRNO_H)
check_include_file_cxx("exception" HAVE_EXCEPTION)
check_include_file_cxx("fcntl.h" HAVE_FCNTL_H)
check_include_file_cxx("fstream" HAVE_FSTREAM)
check_include_file_cxx("getopt.h" HAVE_GETOPT_H)
check_include_file_cxx("grp.h" HAVE_GRP_H)
check_include_file_cxx("iomanip" HAVE_IOMANIP)
check_include_file_cxx("iostream" HAVE_IOSTREAM)
check_include_file_cxx("libgen.h" HAVE_LIBGEN_H)
check_include_file_cxx("limits.h" HAVE_LIMITS_H)
check_include_file_cxx("map" HAVE_MAP)
check_include_file_cxx("math.h" HAVE_MATH_H)
check_include_file_cxx("netdb.h" HAVE_NETDB_H)
check_include_file_cxx("netinet/in.h" HAVE_NETINET_IN_H)
check_include_file_cxx("poll.h" HAVE_POLL_H)
check_include_file_cxx("pthread.h" HAVE_PTHREAD_H)
check_include_file_cxx("pwd.h" HAVE_PWD_H)
check_include_file_cxx("regex.h" HAVE_REGEX_H)
check_include_file_cxx("signal.h" HAVE_SIGNAL_H)
check_include_file_cxx("sstream" HAVE_SSTREAM)
check_include_file_cxx("stdarg.h" HAVE_STDARG_H) # In logging.
check_include_file_cxx("stdio.h" HAVE_STDIO_H)
check_include_file_cxx("stdlib.h" HAVE_STDLIB_H)
check_include_file_cxx("string" HAVE_STRING)
check_include_file_cxx("string.h" HAVE_STRING_H)
check_include_file_cxx("syslog.h" HAVE_SYSLOG_H)
check_include_file_cxx("sys/mman.h" HAVE_SYS_MMAN_H)
check_include_file_cxx("sys/socket.h" HAVE_SYS_SOCKET_H)
check_include_file_cxx("sys/stat.h" HAVE_SYS_STAT_H)
check_include_file_cxx("sys/time.h" HAVE_SYS_TIME_H)
check_include_file_cxx("sys/types.h" HAVE_SYS_TYPES_H)
check_include_file_cxx("sys/wait.h" HAVE_SYS_WAIT_H)
check_include_file_cxx("time.h" HAVE_TIME_H)
check_include_file_cxx("unistd.h" HAVE_UNISTD_H)
if (NOT HAVE_ARPA_INET_H
    OR NOT HAVE_CTYPE_H
    OR NOT HAVE_DIRENT_H
    OR NOT HAVE_ERRNO_H
    OR NOT HAVE_EXCEPTION
    OR NOT HAVE_FCNTL_H
    OR NOT HAVE_FSTREAM
#    OR NOT HAVE_GETOPT_H Optionnal header
    OR NOT HAVE_GRP_H
    OR NOT HAVE_IOMANIP
    OR NOT HAVE_IOSTREAM
    OR NOT HAVE_LIBGEN_H
    OR NOT HAVE_LIMITS_H
    OR NOT HAVE_MAP
    OR NOT HAVE_MATH_H
    OR NOT HAVE_NETDB_H
    OR NOT HAVE_NETINET_IN_H
    OR NOT HAVE_POLL_H
    OR NOT HAVE_PTHREAD_H
    OR NOT HAVE_PWD_H
    OR NOT HAVE_REGEX_H
    OR NOT HAVE_SIGNAL_H
    OR NOT HAVE_SSTREAM
    OR NOT HAVE_STDARG_H
    OR NOT HAVE_STDIO_H
    OR NOT HAVE_STDLIB_H
    OR NOT HAVE_STRING
    OR NOT HAVE_STRING_H
    OR NOT HAVE_SYSLOG_H
    OR NOT HAVE_SYS_MMAN_H
    OR NOT HAVE_SYS_SOCKET_H
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
  "${THREAD_LIBRARIES}")
check_function_exists("alarm" HAVE_ALARM)
check_function_exists("atoi" HAVE_ATOI)
check_function_exists("ceil" HAVE_CEIL)
check_function_exists("chmod" HAVE_CHMOD)
check_function_exists("chown" HAVE_CHOWN)
check_function_exists("clearerr" HAVE_CLEARERR)
check_function_exists("closedir" HAVE_CLOSEDIR)
check_function_exists("close" HAVE_CLOSE)
check_function_exists("ctime" HAVE_CTIME)
check_function_exists("dirname" HAVE_DIRNAME)
check_function_exists("dup2" HAVE_DUP2)
check_function_exists("execvp" HAVE_EXECVP)
check_function_exists("exit" HAVE_EXIT)
check_function_exists("_exit" HAVE__EXIT)
check_function_exists("fchmod" HAVE_FCHMOD)
check_function_exists("fclose" HAVE_FCLOSE)
check_function_exists("fcntl" HAVE_FCNTL)
check_function_exists("fdopen" HAVE_FDOPEN)
check_function_exists("fflush" HAVE_FFLUSH)
check_function_exists("fgets" HAVE_FGETS)
check_function_exists("floor" HAVE_FLOOR)
check_function_exists("fopen" HAVE_FOPEN)
check_function_exists("fork" HAVE_FORK)
check_function_exists("fprintf" HAVE_FPRINTF)
check_function_exists("fputc" HAVE_FPUTC)
check_function_exists("fputs" HAVE_FPUTS)
check_function_exists("free" HAVE_FREE)
check_function_exists("fsync" HAVE_FSYNC)
check_function_exists("ftell" HAVE_FTELL)
check_function_exists("fwrite" HAVE_FWRITE)
check_function_exists("getcwd" HAVE_GETCWD)
check_function_exists("getgrent" HAVE_GETGRENT)
check_function_exists("getopt" HAVE_GETOPT)
check_function_exists("getpid" HAVE_GETPID)
check_function_exists("getpwent" HAVE_GETPWENT)
check_function_exists("gettimeofday" HAVE_GETTIMEOFDAY)
check_function_exists("gmtime_r" HAVE_GMTIME_R)
check_function_exists("isspace" HAVE_ISSPACE)
check_function_exists("kill" HAVE_KILL)
check_function_exists("localtime_r" HAVE_LOCALTIME_R)
check_function_exists("memcpy" HAVE_MEMCPY)
check_function_exists("memmove" HAVE_MEMMOVE)
check_function_exists("memset" HAVE_MEMSET)
check_function_exists("mkfifo" HAVE_MKFIFO)
check_function_exists("mkstemp" HAVE_MKSTEMP)
check_function_exists("mktime" HAVE_MKTIME)
check_function_exists("mmap" HAVE_MMAP)
check_function_exists("munmap" HAVE_MUNMAP)
check_function_exists("nanosleep" HAVE_NANOSLEEP)
check_function_exists("open" HAVE_OPEN)
check_function_exists("opendir" HAVE_OPENDIR)
check_function_exists("pathconf" HAVE_PATHCONF)
check_function_exists("pclose" HAVE_PCLOSE)
check_function_exists("perror" HAVE_PERROR)
check_function_exists("pipe" HAVE_PIPE)
check_function_exists("poll" HAVE_POLL)
check_function_exists("popen" HAVE_POPEN)
check_function_exists("pow" HAVE_POW)
check_function_exists("printf" HAVE_PRINTF)
check_function_exists("pthread_cancel" HAVE_PTHREAD_CANCEL)
check_function_exists("pthread_create" HAVE_PTHREAD_CREATE)
check_function_exists("pthread_join" HAVE_PTHREAD_JOIN)
check_function_exists("pthread_mutex_init" HAVE_PTHREAD_MUTEX_INIT)
check_function_exists("pthread_mutex_lock" HAVE_PTHREAD_MUTEX_LOCK)
check_function_exists("pthread_mutex_trylock" HAVE_PTHREAD_MUTEX_TRYLOCK)
check_function_exists("pthread_mutex_unlock" HAVE_PTHREAD_MUTEX_UNLOCK)
check_function_exists("pthread_setcancelstate" HAVE_PTHREAD_SETCANCELSTATE)
check_function_exists("pthread_setcanceltype" HAVE_PTHREAD_SETCANCELTYPE)
check_function_exists("pthread_sigmask" HAVE_PTHREAD_SIGMASK)
check_function_exists("pthread_testcancel" HAVE_PTHREAD_TESTCANCEL)
check_function_exists("putchar" HAVE_PUTCHAR)
check_function_exists("putenv" HAVE_PUTENV)
check_function_exists("puts" HAVE_PUTS)
check_function_exists("qsort" HAVE_QSORT)
check_function_exists("rand" HAVE_RAND)
check_function_exists("readdir" HAVE_READDIR)
check_function_exists("read" HAVE_READ)
check_function_exists("regcomp" HAVE_REGCOMP)
check_function_exists("regexec" HAVE_REGEXEC)
check_function_exists("regfree" HAVE_REGFREE)
check_function_exists("remove" HAVE_REMOVE)
check_function_exists("rename" HAVE_RENAME)
check_function_exists("select" HAVE_SELECT)
check_function_exists("setbuf" HAVE_SETBUF)
check_function_exists("setenv" HAVE_SETENV)
check_function_exists("setpgid" HAVE_SETPGID)
check_function_exists("sigfillset" HAVE_SIGFILLSET)
check_function_exists("signal" HAVE_SIGNAL)
check_function_exists("sleep" HAVE_SLEEP)
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
check_function_exists("strncat" HAVE_STRNCAT)
check_function_exists("strncpy" HAVE_STRNCPY)
check_function_exists("strpbrk" HAVE_STRPBRK)
check_function_exists("strstr" HAVE_STRSTR)
check_function_exists("strtod" HAVE_STRTOD)
check_function_exists("strtok" HAVE_STRTOK)
check_function_exists("strtol" HAVE_STRTOL)
check_function_exists("strtoul" HAVE_STRTOUL)
check_function_exists("syslog" HAVE_SYSLOG)
check_function_exists("time" HAVE_TIME)
check_function_exists("toupper" HAVE_TOUPPER)
check_function_exists("umask" HAVE_UMASK)
check_function_exists("unlink" HAVE_UNLINK)
check_function_exists("unsetenv" HAVE_UNSETENV)
check_function_exists("vasprintf" HAVE_VASPRINTF)
check_function_exists("vfprintf" HAVE_VFPRINTF)
check_function_exists("waitpid" HAVE_WAITPID)
check_function_exists("write" HAVE_WRITE)
if (NOT HAVE_ALARM
    OR NOT HAVE_ATOI
    OR NOT HAVE_CEIL
    OR NOT HAVE_CHMOD
    OR NOT HAVE_CHOWN
    OR NOT HAVE_CLEARERR
    OR NOT HAVE_CLOSEDIR
    OR NOT HAVE_CLOSE
    OR NOT HAVE_CTIME
    OR NOT HAVE_DIRNAME
    OR NOT HAVE_DUP2
    OR NOT HAVE_EXECVP
    OR NOT HAVE_EXIT
    OR NOT HAVE__EXIT
    OR NOT HAVE_FCHMOD
    OR NOT HAVE_FCLOSE
    OR NOT HAVE_FCNTL
    OR NOT HAVE_FDOPEN
    OR NOT HAVE_FFLUSH
    OR NOT HAVE_FGETS
    OR NOT HAVE_FLOOR
    OR NOT HAVE_FOPEN
    OR NOT HAVE_FORK
#    OR NOT HAVE_FPRINTF # Does not work with Clang.
    OR NOT HAVE_FPUTC
    OR NOT HAVE_FPUTS
    OR NOT HAVE_FREE
    OR NOT HAVE_FSYNC
    OR NOT HAVE_FTELL
    OR NOT HAVE_FWRITE
    OR NOT HAVE_GETCWD
    OR NOT HAVE_GETGRENT
    OR NOT HAVE_GETOPT
    OR NOT HAVE_GETPID
    OR NOT HAVE_GETPWENT
    OR NOT HAVE_GETTIMEOFDAY
    OR NOT HAVE_GMTIME_R
    OR NOT HAVE_ISSPACE
    OR NOT HAVE_KILL
    OR NOT HAVE_LOCALTIME_R
    OR NOT HAVE_MEMCPY
    OR NOT HAVE_MEMMOVE
    OR NOT HAVE_MEMSET
    OR NOT HAVE_MKFIFO
    OR NOT HAVE_MKSTEMP
    OR NOT HAVE_MKTIME
    OR NOT HAVE_MMAP
    OR NOT HAVE_MUNMAP
    OR NOT HAVE_NANOSLEEP
    OR NOT HAVE_OPEN
    OR NOT HAVE_OPENDIR
    OR NOT HAVE_PATHCONF
    OR NOT HAVE_PCLOSE
    OR NOT HAVE_PERROR
    OR NOT HAVE_PIPE
    OR NOT HAVE_POLL
    OR NOT HAVE_POPEN
    OR NOT HAVE_POW
    OR NOT HAVE_PRINTF
    OR NOT HAVE_PTHREAD_CANCEL
    OR NOT HAVE_PTHREAD_CREATE
    OR NOT HAVE_PTHREAD_JOIN
    OR NOT HAVE_PTHREAD_MUTEX_INIT
    OR NOT HAVE_PTHREAD_MUTEX_LOCK
    OR NOT HAVE_PTHREAD_MUTEX_TRYLOCK
    OR NOT HAVE_PTHREAD_MUTEX_UNLOCK
    OR NOT HAVE_PTHREAD_SETCANCELSTATE
    OR NOT HAVE_PTHREAD_SETCANCELTYPE
    OR NOT HAVE_PTHREAD_SIGMASK
    OR NOT HAVE_PTHREAD_TESTCANCEL
    OR NOT HAVE_PUTCHAR
    OR NOT HAVE_PUTENV
    OR NOT HAVE_PUTS
    OR NOT HAVE_QSORT
    OR NOT HAVE_RAND
    OR NOT HAVE_READDIR
    OR NOT HAVE_READ
    OR NOT HAVE_REGCOMP
    OR NOT HAVE_REGEXEC
    OR NOT HAVE_REGFREE
    OR NOT HAVE_REMOVE
    OR NOT HAVE_RENAME
    OR NOT HAVE_SELECT
    OR NOT HAVE_SETBUF
    OR NOT HAVE_SETENV
    OR NOT HAVE_SETPGID
    OR NOT HAVE_SIGFILLSET
    OR NOT HAVE_SIGNAL
    OR NOT HAVE_SLEEP
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
    OR NOT HAVE_STRNCAT
    OR NOT HAVE_STRNCPY
    OR NOT HAVE_STRPBRK
    OR NOT HAVE_STRSTR
    OR NOT HAVE_STRTOD
    OR NOT HAVE_STRTOK
    OR NOT HAVE_STRTOL
    OR NOT HAVE_STRTOUL
    OR NOT HAVE_SYSLOG
    OR NOT HAVE_TIME
    OR NOT HAVE_TOUPPER
    OR NOT HAVE_UMASK
    OR NOT HAVE_UNLINK
    OR NOT HAVE_UNSETENV
    OR NOT HAVE_VASPRINTF
    OR NOT HAVE_VFPRINTF
    OR NOT HAVE_WAITPID
    OR NOT HAVE_WRITE)
  message(FATAL_ERROR "Could not find all mandatory functions.")
endif ()

# Check timezone informations.
include(CheckStructHasMember)
check_struct_has_member("tm" "tm_zone" "time.h" HAVE_TM_ZONE)
if (HAVE_TM_ZONE)
  add_definitions(-DHAVE_TM_ZONE)
endif ()
include(CheckSymbolExists)
check_symbol_exists("tzname" "time.h" HAVE_TZNAME)
if (HAVE_TZNAME)
  add_definitions(-DHAVE_TZNAME)
endif ()

# localstatedir is the prefix.
if (WITH_PREFIX)
  set(localstatedir "${WITH_PREFIX}")
else ()
  set(localstatedir "${CMAKE_INSTALL_PREFIX}")
endif ()

# bindir.
if (WITH_PREFIX_BIN)
  set(bindir "${WITH_PREFIX_BIN}")
else ()
  set(bindir "${localstatedir}/bin")
endif ()

# confdir.
if (WITH_PREFIX_CONF)
  set(confdir "${WITH_PREFIX_CONF}")
else ()
  set(confdir "${localstatedir}/etc")
endif ()

# libdir
if (WITH_PREFIX_LIB)
  set(libdir "${WITH_PREFIX_LIB}")
else ()
  set(libdir "${localstatedir}/lib/centreon-engine")
endif ()

# vardir.
if (WITH_PREFIX_VAR)
  set(vardir "${WITH_PREFIX_VAR}")
else ()
  set(vardir "${localstatedir}/var")
endif ()

# incdir
if (WITH_INCLUDE_DIR)
  set(incdir "${WITH_INCLUDE_DIR}")
else ()
  set(incdir "${localstatedir}/include/centreon-engine")
endif ()

# pkg-config dir.
option(WITH_PKGCONFIG_SCRIPT "Generate and install pkg-config script." ON)
if (WITH_PKGCONFIG_SCRIPT)
  if (WITH_PKGCONFIG_DIR)
    set(pkgconfigdir "${WITH_PKGCONFIG_DIR}")
  else ()
    set(pkgconfigdir "/usr/lib/pkgconfig")
  endif ()
else ()
  set(pkgconfigdir "Not installed")
endif ()

# Check result dir.
if (WITH_CHECK_RESULT_DIR)
  set(CHECK_RESULT_DIR "${WITH_CHECK_RESULT_DIR}")
else ()
  set(CHECK_RESULT_DIR "${localstatedir}/spool/checkresults")
endif ()

# Log archive dir.
if (WITH_LOG_ARCHIVE_DIR)
  set(LOG_ARCHIVE_DIR "${WITH_LOG_ARCHIVE_DIR}")
else ()
  set(LOG_ARCHIVE_DIR "${vardir}/archives")
endif ()

# RW dir.
if (WITH_RW_DIR)
  set(RW_DIR "${WITH_RW_DIR}")
else ()
  set(RW_DIR "${vardir}/rw")
endif ()

# Temporary check result dir.
if (WITH_TEMP_DIR)
  set(TMPDIR "${WITH_TEMP_DIR}")
else ()
  set(TMPDIR "/tmp")
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

# Path for Centreon Engine plugin.
if (WITH_LIBEXECDIR)
  set(libexecdir "${WITH_LIBEXECDIR}")
else ()
  set(libexecdir "/usr/lib/nagios/plugins")
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
set(STARTUP_SCRIPT "No")
if (WITH_STARTUP_SCRIPT STREQUAL "auto")
  if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (OS_DISTRIBUTOR STREQUAL "Ubuntu")
      set(WITH_STARTUP_SCRIPT "upstart")
    else ()
      set(WITH_STARTUP_SCRIPT "sysv")
    endif ()
  else ()
    message(STATUS "Centreon Engine does not provide startup script for ${CMAKE_SYSTEM_NAME}")
  endif ()
endif ()

# Create upstart file.
if (WITH_STARTUP_SCRIPT STREQUAL "upstart")
  message(STATUS "Generating upstart script.")
  set(STARTUP_SCRIPT "Generic upstart script")

  # Startup dir.
  if (WITH_STARTUP_DIR)
    set(STARTUP_DIR "${WITH_STARTUP_DIR}")
  else ()
    set(STARTUP_DIR "/etc/init")
  endif ()

  install(FILES "${SCRIPT_DIR}/upstart.conf"
    DESTINATION "${STARTUP_DIR}"
    COMPONENT "runtime"
    RENAME "centengine.conf")
# Create sysv start script.
elseif (WITH_STARTUP_SCRIPT STREQUAL "sysv")
  message(STATUS "Generating generic startup script.")
  set(STARTUP_SCRIPT "Generic init script")

  # Lock file.
  if (WITH_LOCK_FILE)
    set(LOCK_FILE "${WITH_LOCK_FILE}")
  else ()
    set(LOCK_FILE "/var/lock/subsys/centengine.lock")
  endif ()
  string(REGEX REPLACE "/[^/]*$" "" LOCK_DIR "${LOCK_FILE}")
  file(MAKE_DIRECTORY "${LOCK_DIR}")

  # PID file.
  if (WITH_PID_FILE)
    set(PID_FILE "${WITH_PID_FILE}")
  else ()
    set(PID_FILE "/var/run/centengine.pid")
  endif ()
  string(REGEX REPLACE "/[^/]*$" "" PID_DIR "${PID_FILE}")
  file(MAKE_DIRECTORY "${PID_DIR}")

  # Startup dir.
  if (WITH_STARTUP_DIR)
    set(STARTUP_DIR "${WITH_STARTUP_DIR}")
  else ()
    set(STARTUP_DIR "/etc/init.d")
  endif ()

  install(PROGRAMS "${SCRIPT_DIR}/centengine.sh"
    DESTINATION "${STARTUP_DIR}"
    COMPONENT "runtime"
    RENAME "centengine")
else ()
  message(STATUS "Invalid option WITH_STARTUP_SCRIPT")
  set(STARTUP_SCRIPT "No")
endif ()

# Unit test.
option(WITH_TESTING "Enable unit test" OFF)
if (WITH_TESTING)
  set(UNIT_TEST "Yes")
else ()
  set(UNIT_TEST "No")
endif ()

# Configure files.
configure_file("${PROJECT_SOURCE_DIR}/build/doc/Doxyfile.in"
  "${PROJECT_SOURCE_DIR}/build/doc/Doxyfile")
configure_file("${PROJECT_SOURCE_DIR}/build/doc/header.tex.in"
  "${PROJECT_SOURCE_DIR}/build/doc/header.tex")
configure_file("${INCLUDE_DIR}/compatibility/common.h.in"
  "${INCLUDE_DIR}/compatibility/common.h")
configure_file("${SCRIPT_DIR}/centengine.sh.in"
  "${SCRIPT_DIR}/centengine.sh")
configure_file("${SCRIPT_DIR}/upstart.conf.in"
  "${SCRIPT_DIR}/upstart.conf")
configure_file("${SCRIPT_DIR}/centengine.pc.in"
  "${SCRIPT_DIR}/centengine.pc")

#  Locations definitions
add_definitions(-DDEFAULT_TEMP_FILE="${localstatedir}/tempfile")
add_definitions(-DDEFAULT_TEMP_PATH="${TMPDIR}")
add_definitions(-DDEFAULT_CHECK_RESULT_PATH="${CHECK_RESULT_DIR}")
add_definitions(-DDEFAULT_STATUS_FILE="${vardir}/status.dat")
add_definitions(-DDEFAULT_LOG_FILE="${vardir}/centengine.log")
add_definitions(-DDEFAULT_LOG_ARCHIVE_PATH="${LOG_ARCHIVE_DIR}")
add_definitions(-DDEFAULT_DEBUG_FILE="${vardir}/centengine.debug")
add_definitions(-DDEFAULT_COMMENT_FILE="${vardir}/comments.dat")
add_definitions(-DDEFAULT_DOWNTIME_FILE="${vardir}/downtime.dat")
add_definitions(-DDEFAULT_RETENTION_FILE="${vardir}/retention.dat")
add_definitions(-DDEFAULT_COMMAND_FILE="${RW_DIR}/centengine.cmd")
add_definitions(-DDEFAULT_CONFIG_FILE="${confdir}/centengine.cfg")
add_definitions(-DDEFAULT_OBJECT_CACHE_FILE="${vardir}/objects.cache")
add_definitions(-DDEFAULT_PRECACHED_OBJECT_FILE="${vardir}/objects.precache")
add_definitions(-DDEFAULT_EVENT_BROKER_FILE="${vardir}/broker.socket")
add_definitions(-DDEFAULT_P1_FILE="${bindir}/p1.pl")
add_definitions(-DDEFAULT_AUTH_FILE="")

# Include directories.
include_directories("${INCLUDE_DIR}")
include_directories("${INCLUDE_DIR}/com/centreon/engine")
if (WITH_TESTING)
  include_directories("${PROJECT_SOURCE_DIR}")
endif ()


# Library engine target.
add_library("cce_core"
  STATIC
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
  "${INCLUDE_DIR}/com/centreon/engine/broker.hh"
  "${INCLUDE_DIR}/com/centreon/engine/checks.hh"
  "${INCLUDE_DIR}/com/centreon/engine/comments.hh"
  "${INCLUDE_DIR}/com/centreon/engine/common.hh"
  "${INCLUDE_DIR}/com/centreon/engine/config.hh"
  "${INCLUDE_DIR}/com/centreon/engine/downtime.hh"
  "${INCLUDE_DIR}/com/centreon/engine/engine.hh"
  "${INCLUDE_DIR}/com/centreon/engine/error.hh"
  "${INCLUDE_DIR}/com/centreon/engine/events.hh"
  "${INCLUDE_DIR}/com/centreon/engine/flapping.hh"
  "${INCLUDE_DIR}/com/centreon/engine/globals.hh"
  "${INCLUDE_DIR}/com/centreon/engine/hash_timed_event.hh"
  "${INCLUDE_DIR}/com/centreon/engine/logging.hh"
  "${INCLUDE_DIR}/com/centreon/engine/macros.hh"
  "${INCLUDE_DIR}/com/centreon/engine/nebcallbacks.hh"
  "${INCLUDE_DIR}/com/centreon/engine/neberrors.hh"
  "${INCLUDE_DIR}/com/centreon/engine/nebmods.hh"
  "${INCLUDE_DIR}/com/centreon/engine/nebmodules.hh"
  "${INCLUDE_DIR}/com/centreon/engine/nebstructs.hh"
  "${INCLUDE_DIR}/com/centreon/engine/notifications.hh"
  "${INCLUDE_DIR}/com/centreon/engine/objects.hh"
  "${INCLUDE_DIR}/com/centreon/engine/perfdata.hh"
  "${INCLUDE_DIR}/com/centreon/engine/sehandlers.hh"
  "${INCLUDE_DIR}/com/centreon/engine/shared.hh"
  "${INCLUDE_DIR}/com/centreon/engine/skiplist.hh"
  "${INCLUDE_DIR}/com/centreon/engine/sretention.hh"
  "${INCLUDE_DIR}/com/centreon/engine/statusdata.hh"
  "${INCLUDE_DIR}/com/centreon/engine/utils.hh"
  "${INCLUDE_DIR}/com/centreon/engine/xcddefault.hh"
  "${INCLUDE_DIR}/com/centreon/engine/xdddefault.hh"
  "${INCLUDE_DIR}/com/centreon/engine/xodtemplate.hh"
  "${INCLUDE_DIR}/com/centreon/engine/xpddefault.hh"
  "${INCLUDE_DIR}/com/centreon/engine/xrddefault.hh"
  "${INCLUDE_DIR}/com/centreon/engine/xsddefault.hh"
  ${QT_MOC_FILES})

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
  ${SOCKET_LIBRARIES}
  ${THREAD_LIBRARIES}
  ${QT_LIBRARIES}
  "cce_events"
  "cce_logging"
  "cce_broker"
  "cce_configuration"
  "cce_checks"
  "cce_macros"
  "cce_commands"
  "cce_compatibility"
  "cce_objects")

# centengine target.
add_executable("centengine"
  "${SRC_DIR}/main.cc")
get_property(ENGINE_COMPILE_FLAGS
  TARGET "centengine"
  PROPERTY COMPILE_FLAGS)
set(ENGINE_COMPILE_FLAGS "${ENGINE_COMPILE_FLAGS}")
set_target_properties("centengine"
  PROPERTIES COMPILE_FLAGS "${ENGINE_COMPILE_FLAGS}")
get_property(ENGINE_LINK_FLAGS
  TARGET "centengine"
  PROPERTY LINK_FLAGS)
set(ENGINE_LINK_FLAGS "${ENGINE_LINK_FLAGS}")

if (CMAKE_COMPILER_IS_GNUCXX)
  set(ENGINE_LINK_FLAGS
    "${ENGINE_LINK_FLAGS} -Wl,-whole-archive -lcce_objects -L./objects -Wl,-no-whole-archive")
endif ()

set_target_properties("centengine"
  PROPERTIES LINK_FLAGS "${ENGINE_LINK_FLAGS}")
# Link centengine with required libraries.
target_link_libraries("centengine"
  "cce_core")
# Install rule.
install(TARGETS "centengine"
  DESTINATION "${bindir}"
  COMPONENT "runtime")

# centenginestats target.
add_executable("centenginestats"
  "${SRC_DIR}/centenginestats.cc")
# Install rule.
install(TARGETS "centenginestats"
  DESTINATION "${bindir}"
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

  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${localstatedir}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${RW_DIR}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${bindir}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${confdir}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${libdir}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${vardir}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${CHECK_RESULT_DIR}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${TMPDIR}\")
  mkdir_chown(\"${USER}\" \"${GROUP}\" \"${LOG_ARCHIVE_DIR}\")

  touch_chown(\"${USER}\" \"${GROUP}\" \"${vardir}/status.dat\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${RW_DIR}/centengine.cmd\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${vardir}/centengine.log\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${vardir}/centengine.debug\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${vardir}/comments.dat\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${vardir}/downtime.dat\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${vardir}/retention.dat\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${vardir}/objects.cache\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${vardir}/objects.precache\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${vardir}/broker.socket\")
  touch_chown(\"${USER}\" \"${GROUP}\" \"${bindir}/p1.pl\")")

# Install header files for devel.
install(DIRECTORY "${INCLUDE_DIR}/"
  DESTINATION "${incdir}"
  COMPONENT "development"
  FILES_MATCHING PATTERN "*.hh"
  PATTERN "${INCLUDE_DIR}/compatibility/" EXCLUDE)

# Install pkg-config files.
if (WITH_PKGCONFIG_SCRIPT)
  install(FILES "${SCRIPT_DIR}/centengine.pc"
    DESTINATION "${pkgconfigdir}"
    COMPONENT "runtime")
endif ()

#
# Error tests.
#
if (WITH_TESTING)
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


  # Remove command.
  set(TEST_NAME "remove_command")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/objects/remove_command.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Remove contact.
  set(TEST_NAME "remove_contact")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/objects/remove_contact.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Remove contactgroup.
  set(TEST_NAME "remove_contactgroup")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/objects/remove_contactgroup.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Remove servicegroup.
  set(TEST_NAME "remove_servicegroup")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/objects/remove_servicegroup.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Remove hostgroup.
  set(TEST_NAME "remove_hostgroup")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/objects/remove_hostgroup.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Remove serviceescalation.
  set(TEST_NAME "remove_serviceescalation")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/objects/remove_serviceescalation.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Remove servicedependency.
  set(TEST_NAME "remove_servicedependency")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/objects/remove_servicedependency.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Remove host.
  set(TEST_NAME "remove_host")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/objects/remove_host.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Remove service.
  set(TEST_NAME "remove_service")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/objects/remove_service.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Remove hostescalation.
  set(TEST_NAME "remove_hostescalation")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/objects/remove_hostescalation.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Remove hostdependency.
  set(TEST_NAME "remove_hostdependency")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/objects/remove_hostdependency.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
  # Remove timeperiod.
  set(TEST_NAME "remove_timeperiod")
  add_executable("${TEST_NAME}"
    "${TEST_DIR}/objects/remove_timeperiod.cc")
  target_link_libraries("${TEST_NAME}" "cce_core")
  add_test("${TEST_NAME}" "${TEST_NAME}")
endif ()

#
# Notifications tests.
#
if (WITH_TESTING)
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
endif ()

#
# Stats tests.
#
if (WITH_TESTING)
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
endif ()

#
# Engine tests.
#
# Without any service.
if (WITH_TESTING)
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
endif ()

# Package.
set(CPACK_PACKAGE_NAME "Centreon-Engine")
set(CPACK_PACKAGE_VENDOR "Merethis")
set(CPACK_PACKAGE_VERSION_MAJOR "${ENGINE_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${ENGINE_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${ENGINE_PATCH}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "Centreon Engine monitors your IT infrastructure.")
set(CPACK_PACKAGE_FILE_NAME
  "centreon-engine-${ENGINE_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Centreon-Engine")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/license.txt")
set(CPACK_PACKAGE_CONTACT "Matthieu Kermagoret <mkermagoret@merethis.com>")
if (CPACK_BINARY_DEB)
  # Configure files.
  configure_file("${SCRIPT_DIR}/deb/postinst.in" "${SCRIPT_DIR}/deb/postinst")

  set(DEB_PACKAGE "Yes")
  set(CPACK_DEBIAN_PACKAGE_SECTION "net")
  set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${SCRIPT_DIR}/deb/postinst"
    "${SCRIPT_DIR}/deb/prerm")
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
  set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${SCRIPT_DIR}/rpm/prerm")
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
message(STATUS "    Unit test:               ${UNIT_TEST}")
message(STATUS "    Extra Compilation Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "    Prefix:                  ${localstatedir}")
message(STATUS "    Binary Directory:        ${bindir}")
message(STATUS "    Configuration Directory: ${confdir}")
message(STATUS "    Library Directory:       ${libdir}")
message(STATUS "    Var Directory:           ${vardir}")
message(STATUS "    Include Directory:       ${incdir}")
message(STATUS "    Pkg-config Directory:    ${pkgconfigdir}")
message(STATUS "    Check Result Directory:  ${CHECK_RESULT_DIR}")
message(STATUS "    RW Directory:            ${RW_DIR}")
message(STATUS "    Temporary Directory:     ${TMPDIR}")
message(STATUS "    Startup Script:          ${STARTUP_SCRIPT}")
message(STATUS "    User:                    ${USER}")
message(STATUS "    Group:                   ${GROUP}")
message(STATUS "    Deb Package Creation:    ${DEB_PACKAGE}")
message(STATUS "    RPM Package Creation:    ${RPM_PACKAGE}")
message(STATUS "")
message(STATUS "")

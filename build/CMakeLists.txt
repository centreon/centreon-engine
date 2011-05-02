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

# Testing.
set(PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/..")
include(CTest)

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
set(TEST_DIR "${PROJECT_SOURCE_DIR}/test")

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
    OR NOT HAVE_FPRINTF
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
set(bindir "${localstatedir}/bin")
set(sysconfdir "${localstatedir}/etc")
set(vardir "${localstatedir}/var")

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

# Check which startup script to use.
option(WITH_STARTUP_SCRIPT "Generate and install startup script." ON)
set(STARTUP_SCRIPT "No")
if (WITH_STARTUP_SCRIPT)
  # Linux platform.
  if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(STATUS "Attempting to determine OS distributor.")
    execute_process(COMMAND "lsb_release" "--short" "--id"
      RESULT_VARIABLE RETCODE
      OUTPUT_VARIABLE OS_DISTRIBUTOR
      ERROR_QUIET)
    if (RETCODE EQUAL 0)
      # CentOS
      if (OS_DISTRIBUTOR STREQUAL "CentOS\n")
        message(STATUS "Generating CentOS startup script.")
        set(STARTUP_SCRIPT "CentOS init script")
        install(PROGRAMS "${SCRIPT_DIR}/centengine.sh"
          DESTINATION "/etc/init.d"
          COMPONENT "runtime"
          RENAME "centengine")
      # Ubuntu
      elseif (OS_DISTRIBUTOR STREQUAL "Ubuntu\n")
        message(STATUS "Generating Ubuntu startup script.")
        set(STARTUP_SCRIPT "Ubuntu upstart script")
        install(FILES "${SCRIPT_DIR}/ubuntu.conf"
          DESTINATION "/etc/init"
          COMPONENT "runtime"
          RENAME "centengine.conf")
      else ()
        message(STATUS "Unsupported distributor: ${OS_DISTRIBUTOR}")
      endif ()
    else ()
      message(STATUS "Could not get Linux distributor name, not startup script will be installed.")
    endif ()
  else ()
    message(STATUS "Centreon Engine does not provide startup script for ${CMAKE_SYSTEM_NAME}")
    set(OS_DISTRIBUTOR "${CMAKE_SYSTEM_NAME}")
  endif ()
endif ()

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

if (EMBEDDEDPERL)
  add_definitions(-DEMBEDDEDPERL)
endif ()
if (THREADEDPERL)
  add_definitions(-DTHREADEDPERL)
endif()
if (DOCLEAN)
  add_definitions(-DDOCLEAN)
endif ()

# Configure files.
configure_file("${INCLUDE_DIR}/compatibility/locations.h.in"
  "${INCLUDE_DIR}/compatibility/locations.h")
configure_file("${SCRIPT_DIR}/centengine.sh.in"
  "${SCRIPT_DIR}/centengine.sh")
configure_file("${SCRIPT_DIR}/ubuntu.conf.in"
  "${SCRIPT_DIR}/ubuntu.conf")

#  Locations definitions
add_definitions(-DDEFAULT_TEMP_FILE="${localstatedir}/tempfile")
add_definitions(-DDEFAULT_TEMP_PATH="/tmp")
add_definitions(-DDEFAULT_CHECK_RESULT_PATH="${localstatedir}/spool/checkresults")
add_definitions(-DDEFAULT_STATUS_FILE="${localstatedir}/status.dat")
add_definitions(-DDEFAULT_LOG_FILE="${localstatedir}/centengine.log")
add_definitions(-DDEFAULT_LOG_ARCHIVE_PATH="${localstatedir}/archives/")
add_definitions(-DDEFAULT_DEBUG_FILE="${localstatedir}/centengine.debug")
add_definitions(-DDEFAULT_COMMENT_FILE="${localstatedir}/comments.dat")
add_definitions(-DDEFAULT_DOWNTIME_FILE="${localstatedir}/downtime.dat")
add_definitions(-DDEFAULT_RETENTION_FILE="${localstatedir}/retention.dat")
add_definitions(-DDEFAULT_COMMAND_FILE="${localstatedir}/rw/centengine.cmd")
add_definitions(-DDEFAULT_CONFIG_FILE="${sysconfdir}/centengine.cfg")
add_definitions(-DDEFAULT_OBJECT_CACHE_FILE="${localstatedir}/objects.cache")
add_definitions(-DDEFAULT_PRECACHED_OBJECT_FILE="${localstatedir}/objects.precache")
add_definitions(-DDEFAULT_EVENT_BROKER_FILE="${localstatedir}/broker.socket")
add_definitions(-DDEFAULT_P1_FILE="${bindir}/p1.pl")
add_definitions(-DDEFAULT_AUTH_FILE="")

# Configuration file.
configure_file("${INCLUDE_DIR}/compatibility/locations.h.in"
  "${INCLUDE_DIR}/compatibility/locations.h")
configure_file("${SCRIPT_DIR}/centengine.sh.in"
  "${SCRIPT_DIR}/centengine.sh")
configure_file("${SCRIPT_DIR}/ubuntu.conf.in"
  "${SCRIPT_DIR}/ubuntu.conf")

# Include Directories
include_directories("${INCLUDE_DIR}")

# Produce moc files.
qt4_wrap_cpp(QT_MOC_FILES
  "${INCLUDE_DIR}/modules/loader.hh"
  "${INCLUDE_DIR}/modules/handle.hh"
  "${INCLUDE_DIR}/modules/compatibility.hh")

# Library engine target.
add_library("centenginelib"
  "STATIC"
  "${SRC_DIR}/broker.cc"
  "${SRC_DIR}/checks.cc"
  "${SRC_DIR}/commands.cc"
  "${SRC_DIR}/compatibility/common.cc"
  "${SRC_DIR}/compatibility/globals.cc"
  "${SRC_DIR}/config.cc"
  "${SRC_DIR}/events.cc"
  "${SRC_DIR}/flapping.cc"
  "${SRC_DIR}/globals.cc"
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
  "${SRC_DIR}/configuration/state.cc"
  "${SRC_DIR}/configuration/applier/logging.cc"
  "${SRC_DIR}/modules/loader.cc"
  "${SRC_DIR}/modules/handle.cc"
  "${SRC_DIR}/modules/compatibility.cc"
  "${SRC_DIR}/logging/standard.cc"
  "${SRC_DIR}/logging/file.cc"
  "${SRC_DIR}/logging/syslog.cc"
  "${SRC_DIR}/logging/logger.cc"
  "${SRC_DIR}/logging/broker.cc"
  "${SRC_DIR}/logging/engine.cc"
  "${SRC_DIR}/compatibility/logging.cc"
  "${INCLUDE_DIR}/broker.hh"
  "${INCLUDE_DIR}/checks.hh"
  "${INCLUDE_DIR}/commands.hh"
  "${INCLUDE_DIR}/comments.hh"
  "${INCLUDE_DIR}/common.hh"
  "${INCLUDE_DIR}/config.hh"
  "${INCLUDE_DIR}/downtime.hh"
  "${INCLUDE_DIR}/engine.hh"
  "${INCLUDE_DIR}/epn_engine.hh"
  "${INCLUDE_DIR}/error.hh"
  "${INCLUDE_DIR}/events.hh"
  "${INCLUDE_DIR}/flapping.hh"
  "${INCLUDE_DIR}/globals.hh"
  "${INCLUDE_DIR}/logging.hh"
  "${INCLUDE_DIR}/macros.hh"
  "${INCLUDE_DIR}/nebcallbacks.hh"
  "${INCLUDE_DIR}/neberrors.hh"
  "${INCLUDE_DIR}/nebmods.hh"
  "${INCLUDE_DIR}/nebmodules.hh"
  "${INCLUDE_DIR}/nebstructs.hh"
  "${INCLUDE_DIR}/notifications.hh"
  "${INCLUDE_DIR}/objects.hh"
  "${INCLUDE_DIR}/perfdata.hh"
  "${INCLUDE_DIR}/sehandlers.hh"
  "${INCLUDE_DIR}/shared.hh"
  "${INCLUDE_DIR}/skiplist.hh"
  "${INCLUDE_DIR}/sretention.hh"
  "${INCLUDE_DIR}/statusdata.hh"
  "${INCLUDE_DIR}/utils.hh"
  "${INCLUDE_DIR}/xcddefault.hh"
  "${INCLUDE_DIR}/xdddefault.hh"
  "${INCLUDE_DIR}/xodtemplate.hh"
  "${INCLUDE_DIR}/xpddefault.hh"
  "${INCLUDE_DIR}/xrddefault.hh"
  "${INCLUDE_DIR}/xsddefault.hh"
  "${INCLUDE_DIR}/configuration/state.hh"
  "${INCLUDE_DIR}/configuration/applier/base.hh"
  "${INCLUDE_DIR}/configuration/applier/logging.hh"
  "${INCLUDE_DIR}/modules/compatibility.hh"
  "${INCLUDE_DIR}/modules/handle.hh"
  "${INCLUDE_DIR}/modules/loader.hh"
  "${INCLUDE_DIR}/logging/object.hh"
  "${INCLUDE_DIR}/logging/standard.hh"
  "${INCLUDE_DIR}/logging/file.hh"
  "${INCLUDE_DIR}/logging/syslog.hh"
  "${INCLUDE_DIR}/logging/logger.hh"
  "${INCLUDE_DIR}/logging/broker.hh"
  "${INCLUDE_DIR}/logging/engine.hh"
  "${PERL_SOURCES}"
  ${QT_MOC_FILES})
get_property(LIB_COMPILE_FLAGS
  TARGET "centenginelib"
  PROPERTY COMPILE_FLAGS)
set(LIB_COMPILE_FLAGS "${LIB_COMPILE_FLAGS}")
set_target_properties("centenginelib" PROPERTIES
  COMPILE_FLAGS "${LIB_COMPILE_FLAGS}")
target_link_libraries("centenginelib"
  ${MATH_LIBRARIES}
  ${SOCKET_LIBRARIES}
  ${THREAD_LIBRARIES}
  ${PERL_LINK_FLAGS}
  ${QT_LIBRARIES})

# Centengine target.
add_executable("centengine"
  "${SRC_DIR}/main.cc")
get_property(ENGINE_COMPILE_FLAGS
  TARGET "centengine"
  PROPERTY COMPILE_FLAGS)
set(ENGINE_COMPILE_FLAGS "${ENGINE_COMPILE_FLAGS} ${PERL_COMPILE_FLAGS}")
set_target_properties("centengine" PROPERTIES
  COMPILE_FLAGS "${ENGINE_COMPILE_FLAGS}")
target_link_libraries("centengine"
  "centenginelib")
install(TARGETS "centengine"
  DESTINATION "${bindir}"
  COMPONENT "runtime")

# Centenginestats target.
add_executable("centenginestats"
  "${SRC_DIR}/centenginestats.cc")
install(TARGETS "centenginestats"
  DESTINATION "${bindir}"
  COMPONENT "runtime")

# Test configuration target.
add_executable("test_configuration"
  "${TEST_DIR}/test_configuration.cc"
  "${SRC_DIR}/globals.cc"
  "${SRC_DIR}/error.cc"
  "${SRC_DIR}/configuration/state.cc")
target_link_libraries("test_configuration"
  ${QT_LIBRARIES})

add_test("test_configuration_basic"
  "test_configuration"
  "test.cfg")

# Install header files for devel.
install(DIRECTORY "${INCLUDE_DIR}/"
  DESTINATION "${localstatedir}/include/centreon-engine/"
  COMPONENT "development"
  FILES_MATCHING PATTERN "*.hh"
  PATTERN "${INCLUDE_DIR}/compatibility/" EXCLUDE)

install(DIRECTORY "${INCLUDE_DIR}/compatibility/"
  DESTINATION "${localstatedir}/include/centreon-engine/"
  COMPONENT "development"
  FILES_MATCHING PATTERN "*.h")

# Add subdirectories.
add_subdirectory("doc")
add_subdirectory("modules")

# Package.
set(CPACK_PACKAGE_NAME "Centreon-Engine")
set(CPACK_PACKAGE_VENDOR "Merethis")
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH "0beta")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
  "Centreon Engine monitors your IT infrastructure.")
set(CPACK_PACKAGE_FILE_NAME
  "centreon-engine-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
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

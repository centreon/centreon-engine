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
## along with Centreon Scheduler. If not, see
## <http://www.gnu.org/licenses/>.
##

# Global options.
cmake_minimum_required(VERSION 2.8)
project("Centreon Engine" C)

# Testing.
set(PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}/..")
include(CTest)

# Set directories.
set(BASE_DIR "${PROJECT_SOURCE_DIR}/base")
set(COMMON_DIR "${PROJECT_SOURCE_DIR}/common")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(XDATA_DIR "${PROJECT_SOURCE_DIR}/xdata")
set(TEST_DIR "${PROJECT_SOURCE_DIR}/test")
set(SCRIPT_DIR "${PROJECT_SOURCE_DIR}/scripts")

# Check headers.
include(CheckIncludeFiles)
include(CheckCSourceCompiles)
check_include_files("arpa/inet.h" HAVE_ARPA_INET_H)
check_include_files("ctype.h" HAVE_CTYPE_H)
check_include_files("dirent.h" HAVE_DIRENT_H)
check_include_files("dlfcn.h" HAVE_DLFCN_H)
check_include_files("errno.h" HAVE_ERRNO_H)
check_include_files("fcntl.h" HAVE_FCNTL_H)
check_include_files("float.h" HAVE_FLOAT_H)
check_include_files("getopt.h" HAVE_GETOPT_H)
check_include_files("grp.h" HAVE_GRP_H)
check_include_files("libgen.h" HAVE_LIBGEN_H)
check_include_files("limits.h" HAVE_LIMITS_H)
check_include_files("linux/module.h" HAVE_LINUX_MODULE_H)
check_include_files("math.h" HAVE_MATH_H)
check_include_files("netdb.h" HAVE_NETDB_H)
check_include_files("netinet/in.h" HAVE_NETINET_IN_H)
check_include_files("pthread.h" HAVE_PTHREAD_H)
check_include_files("pthreads.h" HAVE_PTHREADS_H)
check_include_files("pwd.h" HAVE_PWD_H)
check_include_files("regex.h" HAVE_REGEX_H)
check_include_files("signal.h" HAVE_SIGNAL_H)
check_include_files("socket.h" HAVE_SOCKET_H)
check_include_files("stdarg.h" HAVE_STDARG_H)
check_include_files("stdio.h" HAVE_STDIO_H)
check_include_files("stdlib.h" HAVE_STDLIB_H)
check_include_files("string.h" HAVE_STRING_H)
check_include_files("strings.h" HAVE_STRINGS_H)
check_include_files("sys/ipc.h" HAVE_SYS_IPC_H)
check_include_files("sys/msg.h" HAVE_SYS_MSG_H)
check_include_files("sys/mman.h" HAVE_SYS_MMAN_H)
check_include_files("sys/poll.h" HAVE_SYS_POLL_H)
check_include_files("sys/resource.h" HAVE_SYS_RESOURCE_H)
check_include_files("sys/socket.h" HAVE_SYS_SOCKET_H)
check_include_files("sys/stat.h" HAVE_SYS_STAT_H)
check_include_files("sys/time.h" HAVE_SYS_TIME_H)
check_include_files("sys/timeb.h" HAVE_SYS_TIMEB_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files("sys/un.h" HAVE_SYS_UN_H)
check_include_files("sys/wait.h" HAVE_SYS_WAIT_H)
check_include_files("syslog.h" HAVE_SYSLOG_H)
check_include_files("time.h" HAVE_TIME_H)
check_include_files("uio.h" HAVE_UIO_H)
check_include_files("unistd.h" HAVE_UNISTD_H)
set(STDC_HEADERS FALSE)
if (HAVE_FLOAT_H)
  if (HAVE_STDARG_H)
    if (HAVE_STDLIB_H)
      if (HAVE_STRING_H)
        set(STDC_HEADERS TRUE)
      endif ()
    endif ()
  endif()
endif ()

# Check functions.
include(CheckFunctionExists)
check_function_exists("asprintf" HAVE_ASPRINTF)
check_function_exists("getloadavg" HAVE_GETLOADAVG)
check_function_exists("initgroups" HAVE_INITGROUPS)
check_function_exists("setenv" HAVE_SETENV)
check_function_exists("socket" HAVE_SOCKET)
check_function_exists("snprintf" HAVE_SNPRINTF)
check_function_exists("strdup" HAVE_STRDUP)
check_function_exists("strstr" HAVE_STRSTR)
check_function_exists("strtoul" HAVE_STRTOUL)
check_function_exists("unsetenv" HAVE_UNSETENV)
check_function_exists("vasprintf" HAVE_VASPRINTF)
check_function_exists("vsnprintf" HAVE_VSNPRINTF)
check_c_source_compiles("
  #include <sys/types.h>
  #include <stdarg.h>
  void foo(const char *format, ...) {
    va_list ap;
    int len;
    char buf[5];

    va_start(ap, format);
    len = vsnprintf(buf, 0, format, ap);
    va_end(ap);
    if (len != 5) exit(1);

    va_start(ap, format);
    len = vsnprintf(0, 0, format, ap);
    va_end(ap);
    if (len != 5) exit(1);

    if (snprintf(buf, 3, \"hello\") != 5 || strcmp(buf, \"he\") != 0) exit(1);

    exit(0);
  }
  main() { foo(\"hello\"); }
" HAVE_C99_VSNPRINTF)

# Should we use nanosleep.
option(WITH_NANOSLEEP "Use nanosleep instead of sleep." ON)
if (WITH_NANOSLEEP)
  set(NANOSLEEP "Yes")
  check_function_exists("nanosleep" USE_NANOSLEEP)
else ()
  set(NANOSLEEP "No")
endif ()

# Check timezone informations.
include(CheckStructHasMember)
check_struct_has_member("tm" "tm_zone" "time.h" HAVE_STRUCT_TM_TM_ZONE)
include(CheckSymbolExists)
check_symbol_exists("tzname" "time.h" HAVE_TZNAME)

# Check varargs features for Nagios.
check_c_source_compiles("
  #ifdef __STDC__
  # include <stdio.h>
  # include <stdlib.h>
  # include <stdarg.h>
  #else
  # include <sys/types.h>
  # include <stdio.h>
  # include <varargs.h>
  #endif
  int main() {
    va_list args;
    (void)args;
    return (0);
  }
" NOT_NEED_VA_LIST)
if (NOT_NEED_VA_LIST)
  set(NEED_VA_LIST FALSE)
else ()
  set(NEED_VA_LIST TRUE)
endif ()
check_c_source_compiles("
  #ifdef __STDC__
  # include <stdio.h>
  # include <stdlib.h>
  # include <stdarg.h>
  #else
  # include <sys/types.h>
  # include <stdio.h>
  # include <varargs.h>
  #endif
  int main() {
    va_list ap1, ap2;
    va_copy(ap1, ap2);
    return (0);
  }
" HAVE_VA_COPY)
check_c_source_compiles("
  #ifdef __STDC__
  # include <stdio.h>
  # include <stdlib.h>
  # include <stdarg.h>
  #else
  # include <sys/types.h>
  # include <stdio.h>
  # include <varargs.h>
  #endif
  int main() {
    va_list ap1, ap2;
    __va_copy(ap1, ap2);
    return (0);
  }
" HAVE___VA_COPY)

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

# Event broker option.
option(WITH_EVENT_BROKER "Allow event broker modules use." ON)
if (WITH_EVENT_BROKER)
  set(EVENT_BROKER "Yes")
  set(USE_EVENT_BROKER TRUE)
else ()
  set(EVENT_BROKER "No")
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
	install(PROGRAMS "${SCRIPT_DIR}/centengine.sh"
	  DESTINATION "/etc/init.d"
	  RENAME "centengine")
      elseif (OS_DISTRIBUTOR STREQUAL "Ubuntu\n")
        message(STATUS "Generating Ubuntu startup script.")
        set(STARTUP_SCRIPT "Ubuntu upstart script")
	install(FILES "${SCRIPT_DIR}/ubuntu.conf"
	  DESTINATION "/etc/init"
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
  set(PERL_SOURCES "${BASE_DIR}/perlxsi.c")
  execute_process(COMMAND "perl" "-MExtUtils::Embed" "-e" "xsinit" "--" "-o" "${PERL_SOURCES}"
    RESULT_VARIABLE RETVAL)
  if (NOT RETVAL EQUAL 0)
    message(FATAL_ERROR "Could not generate embedded Perl source file.")
  endif ()
else ()
  set(EMBEDDED_PERL "No")
endif ()

# Configure files.
configure_file("${INCLUDE_DIR}/config.h.in"
  "${INCLUDE_DIR}/config.h")
configure_file("${INCLUDE_DIR}/locations.h.in"
  "${INCLUDE_DIR}/locations.h")
configure_file("${INCLUDE_DIR}/snprintf.h.in"
  "${INCLUDE_DIR}/snprintf.h")
configure_file("${SCRIPT_DIR}/centengine.sh.in"
  "${SCRIPT_DIR}/centengine.sh")
configure_file("${SCRIPT_DIR}/ubuntu.conf.in"
  "${SCRIPT_DIR}/ubuntu.conf")

# centengine target.
add_executable("centengine"
  "${BASE_DIR}/broker.c"
  "${BASE_DIR}/checks.c"
  "${BASE_DIR}/commands.c"
  "${BASE_DIR}/config.c"
  "${BASE_DIR}/events.c"
  "${BASE_DIR}/flapping.c"
  "${BASE_DIR}/logging.c"
  "${BASE_DIR}/main.c"
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
  "${COMMON_DIR}/shared.c"
  "${COMMON_DIR}/skiplist.c"
  "${COMMON_DIR}/snprintf.c"
  "${COMMON_DIR}/statusdata.c"
  "${XDATA_DIR}/xcddefault.c"
  "${XDATA_DIR}/xdddefault.c"
  "${XDATA_DIR}/xodtemplate.c"
  "${XDATA_DIR}/xpddefault.c"
  "${XDATA_DIR}/xrddefault.c"
  "${XDATA_DIR}/xsddefault.c"
  "${INCLUDE_DIR}/broker.h"
  "${INCLUDE_DIR}/comments.h"
  "${INCLUDE_DIR}/common.h"
  "${INCLUDE_DIR}/config.h"
  "${INCLUDE_DIR}/downtime.h"
  "${INCLUDE_DIR}/epn_nagios.h"
  "${INCLUDE_DIR}/locations.h"
  "${INCLUDE_DIR}/logging.h"
  "${INCLUDE_DIR}/macros.h"
  "${INCLUDE_DIR}/nagios.h"
  "${INCLUDE_DIR}/nebcallbacks.h"
  "${INCLUDE_DIR}/neberrors.h"
  "${INCLUDE_DIR}/nebmods.h"
  "${INCLUDE_DIR}/nebmodules.h"
  "${INCLUDE_DIR}/nebstructs.h"
  "${INCLUDE_DIR}/netutils.h"
  "${INCLUDE_DIR}/objects.h"
  "${INCLUDE_DIR}/perfdata.h"
  "${INCLUDE_DIR}/shared.h"
  "${INCLUDE_DIR}/skiplist.h"
  "${INCLUDE_DIR}/snprintf.h"
  "${INCLUDE_DIR}/sretention.h"
  "${INCLUDE_DIR}/statusdata.h"
  "${PERL_SOURCES}"
)
get_property(ENGINE_COMPILE_FLAGS
  TARGET "centengine"
  PROPERTY COMPILE_FLAGS)
set(ENGINE_COMPILE_FLAGS "${ENGINE_COMPILE_FLAGS} ${PERL_COMPILE_FLAGS}")
set_target_properties("centengine" PROPERTIES
  COMPILE_FLAGS "${ENGINE_COMPILE_FLAGS}")
target_link_libraries("centengine"
  ${DL_LIBRARIES}
  ${MATH_LIBRARIES}
  ${SOCKET_LIBRARIES}
  ${THREAD_LIBRARIES}
  ${PERL_LINK_FLAGS}
)
install(TARGETS "centengine" DESTINATION "${bindir}")

# centenginestats target.
add_executable("centenginestats"
  "${BASE_DIR}/centenginestats.c"
)
install(TARGETS "centenginestats" DESTINATION "${bindir}")

# Documentation.
add_subdirectory("doc")

# Package.
set(CPACK_PACKAGE_NAME "Centreon-Engine")
set(CPACK_PACKAGE_VENDOR "Merethis")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 9)
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
message(STATUS "    Nanosleep enabled:       ${NANOSLEEP}")
message(STATUS "    Event-broker enabled:    ${EVENT_BROKER}")
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

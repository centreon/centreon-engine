/*
** Copyright 2011 Merethis
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <string.h>
#include <stdio.h>

#include "logging/logger.hh"
#include "error.hh"
#include "common.hh"

using namespace com::centreon::engine::logging;

/**************************************
 *                                     *
 *           Global Objects            *
 *                                     *
 **************************************/

static const unsigned int BUFFER_SIZE = 4096;
static nagios_macros      global_macros;

unsigned int test::_nb_instance = 0;

extern "C" {
  /**
   *  Define Symbole to compile, but unused.
   */
  nagios_macros* get_global_macros(void) { return (&global_macros); }
  char* my_strdup(char const* str) {
    char* ptr = new char[strlen(str) + 1];
    return (strcpy(ptr, str));
  }
  int set_environment_var(char const* name, char const* value, int set) {
    (void)name; (void)value; (void)set;
    return (0);
  }
  void logit(int type, int display, char const* fmt, ...) {
    (void)display;
    char buffer[BUFFER_SIZE];
    va_list ap;
    va_start(ap, fmt);
    if (vsnprintf(buffer, sizeof(buffer), fmt, ap) > 0) {
      logger(type, object::basic) << buffer;
    }
    va_end(ap);
  }
  int log_debug_info(int type, unsigned int verbosity, char const* fmt, ...) {
    char buffer[BUFFER_SIZE];
    va_list ap;

    va_start(ap, fmt);
    if (vsnprintf(buffer, sizeof(buffer), fmt, ap) > 0) {
      timeval now;
      if (gettimeofday(&now, NULL) == -1) {
	now.tv_sec = 0;
	now.tv_usec = 0;
      }

      if (verbosity > object::most) {
	verbosity = object::most;
      }

      logger(static_cast<unsigned long long>(type) << 32, verbosity)
	<< "[" << now.tv_sec << "." << now.tv_usec << "] "
	<< "[" << type << "." << verbosity << "] "
	<< "[pid=" << getpid() << "] " << buffer;
    }
    va_end(ap);

    return (OK);
  }
}

/**************************************
 *                                     *
 *           Public Methods            *
 *                                     *
 **************************************/

/**
 *  Default constructor.
 *
 *  @param[in] msg        The message.
 *  @param[in] type       The logging types.
 *  @param[in] verbosity  The verbosity level.
 *  @param[in] total_call The number of log are call.
 */
test::test(std::string const& msg,
	   unsigned long long type,
	   unsigned int verbosity,
	   unsigned int total_call)
  : _msg(msg),
    _type(type),
    _verbosity(verbosity),
    _total_call(total_call),
    _nb_call(0) {
  ++_nb_instance;
}

/**
 *  Default destructor
 */
test::~test() {
  --_nb_instance;
  if (_total_call != _nb_call) {
    throw (engine_error() << _total_call
	   << " " << _nb_call
	   << " " << _type
	   << " " << _verbosity
	   << " bad job call.");
  }
}

/**
 *  Log message.
 *
 *  @param[in] message   The message.
 *  @param[in] type      The logging types.
 *  @param[in] verbosity The verbosity level.
 */
void test::log(char const* message,
	       unsigned long long type,
	       unsigned int verbosity) throw() {
  if (message == _msg && (type & _type) && verbosity <= _verbosity) {
    ++_nb_call;
  }
}

/**
 *  Get the current number instance.
 *
 *  @return The current number instance.
 */
unsigned int test::get_nb_instance() {
  return (_nb_instance);
}

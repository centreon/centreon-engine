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

#include "error.hh"
#include "test.hh"

using namespace com::centreon::engine::logging;

/**************************************
 *                                     *
 *           Static Objects            *
 *                                     *
 **************************************/

unsigned int test::_nb_instance = 0;

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

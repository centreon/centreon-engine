/*
** Copyright 2011      Merethis
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

#include <stdio.h>
#include <string.h>
#include "logging/standard.hh"

using namespace com::centreon::engine::logging;

/**************************************
 *                                     *
 *           Public Methods            *
 *                                     *
 **************************************/

/**
 *  Default constructor.
 *
 *  @param[in] is_stdout Select stdout or stderr to logging the message.
 */
standard::standard(bool is_stdout) {
  _file = (is_stdout == true ? stdout : stderr);
}

/**
 *  Default destructor.
 */
standard::~standard() throw() {

}

/**
 *  Default copy constructor.
 *
 *  @param[in] right The class to copy.
 */
standard::standard(standard const& right)
  : object(right),
    _file(0) {
  operator=(right);
}

/**
 *  Default copy operator.
 *
 *  @param[in] right The class to copy.
 */
standard& standard::operator=(standard const& right) {
  if (this != &right) {
    _file = right._file;
  }
  return (*this);
}

/**
 *  Write log in stdout or stderr.
 *
 *  @param[in] message   Message to log.
 *  @param[in] type      Logging types.
 *  @param[in] verbosity Verbosity level.
 */
void standard::log(char const* message,
		   unsigned long long type,
		   unsigned int verbosity) throw() {
  (void)type;
  (void)verbosity;

  _mutex.lock();
  fwrite(message, strlen(message), 1, _file);
  _mutex.unlock();
}

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

#include <string.h>
#include "error.hh"

using namespace com::centreon::engine::script;

/**
 *  Default constructor.
 *
 *  @param[in] message The error message.
 */
error::error(char const* message) throw() {
  strncpy(_message, message, sizeof(_message) - 1);
  _message[sizeof(_message) - 1] = 0;
}

/**
 *  Copy constructor.
 *
 *  @param[in] e Object to copy.
 */
error::error(error const& e) throw ()
  : std::exception(e) {
  strncpy(_message, e._message, sizeof(_message) - 1);
  _message[sizeof(_message) - 1] = 0;
}

/**
 *  Destructor.
 */
error::~error() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] e Object to copy.
 *
 *  @return This object.
 */
error& error::operator=(error const& e) throw () {
  if (this != &e) {
    strncpy(_message, e._message, sizeof(_message) - 1);
    _message[sizeof(_message) - 1] = 0;
  }
  return (*this);
}

/**
 *  Get the error message.
 *
 *  @return Error message.
 */
char const* error::what() const throw () {
  return (_message);
}

/*
** Copyright 2015 Merethis
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

#include <cstdlib>
#include "com/centreon/engine/configuration/duration.hh"
#include "com/centreon/engine/error.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
duration::duration() {}

/**
 *  Copy constructor.
 *
 *  @param[in] other  Object to copy.
 */
duration::duration(duration const& other) : _value(other._value) {}

/**
 *  Build a duration with a value.
 *
 *  @param[in] value  Value.
 */
duration::duration(long value) : _value(value) {}

/**
 *  Build a duration from a string.
 *
 *  @param[in] value  String.
 *
 *  @see set()
 */
duration::duration(std::string const& value) {
  set(value);
}

/**
 *  Destructor.
 */
duration::~duration() {}

/**
 *  Assignment operator.
 *
 *  @param[in] other  Object to copy.
 *
 *  @return This object.
 */
duration& duration::operator=(duration const& other) {
  if (this != &other)
    _value = other._value;
  return (*this);
}

/**
 *  Set duration value.
 *
 *  @param[in] value  New duration.
 *
 *  @see set()
 */
duration& duration::operator=(long value) {
  _value = value;
  return (*this);
}

/**
 *  Set duration value.
 *
 *  @param[in] value  New duration value.
 *
 *  @see set()
 */
duration& duration::operator=(std::string const& value) {
  set(value);
  return (*this);
}

/**
 *  Get the value of the duration.
 *
 *  @return Value of the duration in seconds.
 *
 *  @see get()
 */
duration::operator long() const {
  return (get());
}

/**
 *  Get the value of the duration expressed in seconds.
 *
 *  @return Value of the duration in seconds.
 */
long duration::get() const {
  return (_value);
}

/**
 *  Set duration.
 *
 *  @param[in] value  A number eventually followed by a time unit
 *                    suffix. This suffix is one of "s" for seconds, "m"
 *                    for minutes, "h" for hours and "d" for days.
 */
void duration::set(std::string const& value) {
  char* endptr(NULL);
  _value = std::strtol(value.c_str(), &endptr, 0);
  if (*endptr && _value > 0) {
    switch (*endptr) {
    case 's':
      // Do nothing.
      break ;
    case 'm':
      _value *= 60;
      break ;
    case 'h':
      _value *= 60 * 60;
      break ;
    case 'd':
      _value *= 24 * 60 * 60;
      break ;
    default:
      throw (engine_error() << "unknown duration suffix: "
             << endptr);
    }
  }
  return ;
}

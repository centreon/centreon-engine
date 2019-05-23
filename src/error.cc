/*
** Copyright 2011-2013 Merethis
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

#include <cstdio>
#include <cstring>
#include "com/centreon/engine/error.hh"

#undef error

using namespace com::centreon::engine;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
error::error() throw () : _current(0) {}

/**
 *  Constructor with debugging informations.
 */
error::error(char const* file, char const* function, int line) throw ()
  : _current(0) {
  *this << "[" << file << ":" << line << "(" << function << ")] ";
}

/**
 *  Copy constructor.
 *
 *  @param[in] e Object to copy.
 */
error::error(error const& e) throw ()
  : std::exception(e),
    _current(e._current) {
  memcpy(_buffer, e._buffer, _current * sizeof(*_buffer));
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
  std::exception::operator=(e);
  _current = e._current;
  memcpy(_buffer, e._buffer, _current * sizeof(*_buffer));
  return *this;
}

/**
 *  Insertion operator.
 *
 *  @param[in] c Char to add to error message.
 *
 *  @return This object.
 */
error& error::operator<<(char c) throw () {
  char buffer[2];
  buffer[0] = c;
  buffer[1] = '\0';
  return operator<<(buffer);
}

/**
 *  Insertion operator.
 *
 *  @param[in] str String to concatenate to error message.
 *
 *  @return This object.
 */
error& error::operator<<(char const* str) throw () {
  // Detect NULL string.
  if (!str)
    str = "(null)";

  // Compute maximum number of bytes to append.
  unsigned int to_copy = strlen(str);
  unsigned int rem = sizeof(_buffer) / sizeof(*_buffer) - _current - 1;
  if (rem < to_copy)
    to_copy = rem;

  // Data copy.
  memcpy(_buffer + _current, str, to_copy * sizeof(*_buffer));
  _current += to_copy;

  return *this;
}

/**
 *  Insertion operator.
 *
 *  @param[in] i Integer to concatenate to error message.
 *
 *  @return This object.
 */
error& error::operator<<(int i) throw () {
  _insert_with_snprintf(i, "%d%n");
  return *this;
}

/**
 *  Insertion operator.
 *
 *  @param[in] u Unsigned long to concatenate to error message.
 *
 *  @return This object.
 */
error& error::operator<<(unsigned long u) throw () {
  _insert_with_snprintf(u, "%u%n");
  return *this;
}

/**
 *  Insertion operator.
 *
 *  @param[in] u Unsigned integer to concatenate to error message.
 *
 *  @return This object.
 */
error& error::operator<<(unsigned int u) throw () {
  _insert_with_snprintf(u, "%u%n");
  return *this;
}

/**
 *  Insertion operator.
 *
 *  @param[in] u Long integer to concatenate to error message.
 *
 *  @return This object.
 */
error& error::operator<<(long l) throw () {
  _insert_with_snprintf(l, "%ld%n");
  return *this;
}

/**
 *  Insertion operator.
 *
 *  @param[in] u Lon lon integer to concatenate to error message.
 *
 *  @return This object.
 */
error& error::operator<<(long long ll) throw () {
  _insert_with_snprintf(ll, "%lld%n");
  return *this;
}

/**
 *  Insertion operator.
 *
 *  @param[in] u Unsigned long long integer to concatenate to error message.
 *
 *  @return This object.
 */
error& error::operator<<(unsigned long long ull) throw () {
  _insert_with_snprintf(ull, "%llu%n");
  return *this;
}

/**
 *  Insertion operator.
 *
 *  @param[in] d Double to concatenate to error message.
 *
 *  @return This object.
 */
error& error::operator<<(double d) throw () {
  _insert_with_snprintf(d, "%lf%n");
  return *this;
}

/**
 *  Insertion operator.
 *
 *  @param[in] str String to concatenate to error message.
 *
 *  @return This object.
 */
error& error::operator<<(std::string const& str) throw () {
  return operator<<(str.c_str());
}

/**
 *  Get the error message.
 *
 *  @return Error message.
 */
char const* error::what() const throw () {
  _buffer[_current] = '\0';
  return _buffer;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Insert data in buffer using snprintf.
 *
 *  @param[in] t      Object to stringify.
 *  @param[in] format Format used by snprintf to stringify argument.
 *                    This format must be terminated by a %n.
 */
template <typename T>
void error::_insert_with_snprintf(T& t, char const* format) {
  int wc;
  if (snprintf(
        _buffer + _current,
        sizeof(_buffer) / sizeof(*_buffer) - _current,
        format,
        t,
        &wc) > 0)
    _current += wc;
  return;
}

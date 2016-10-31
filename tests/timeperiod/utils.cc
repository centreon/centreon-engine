/*
** Copyright 2016 Centreon
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

#include <cstring>
#include <ctime>
#include "com/centreon/engine/error.hh"
#include "tests/timeperiod/utils.hh"

// Global time.
static time_t gl_now((time_t)-1);

/**
 *  Convert hour and minutes to a number of seconds.
 *
 *  @param[in] h  Hours.
 *  @param[in] m  Minutes.
 *
 *  @return The number of seconds.
 */
int hmtos(int h, int m) {
  return (h * 60 * 60 + m * 60);
}

/**
 *  Set system time for testing purposes.
 *
 *  The real system time is not changed but time() returns the requested
 *  value.
 *
 *  @param now  New system time.
 */
void set_time(time_t now) {
  gl_now = now;
  return ;
}

/**
 *  Convert a string to time_t.
 *
 *  @param str  String to convert.
 *
 *  @return The converted string.
 */
time_t strtotimet(std::string const& str) {
  tm t;
  memset(&t, 0, sizeof(t));
  if (!strptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &t))
    throw (engine_error() << "invalid date format");
  t.tm_isdst = -1;
  return (mktime(&t));
}

/**
 *  Overload of libc time function.
 */

#ifndef __THROW
#  define __THROW
#endif // !__THROW

extern "C" time_t time(time_t *t) __THROW {
  if (t)
    *t = gl_now;
  return (gl_now);
}

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

#include <limits.h>
#include <string.h>
#include <sstream>
#include "com/centreon/engine/error.hh"

using namespace com::centreon::engine;

/**
 *  Check that unsigned integer insertion works properly in error.
 *
 *  @return 0 on success.
 */
int main() {
  // Insert unsigned integers.
  error e;
  e << 0u << 89u << UINT_MAX - 10000 << 7896541u;
  e << UINT_MAX << 1;

  // Reference object.
  std::ostringstream oss;
  oss << 0u << 89u << UINT_MAX - 10000 << 7896541u << UINT_MAX << 1;

  // Check.
  return (strcmp(oss.str().c_str(), e.what()));
}

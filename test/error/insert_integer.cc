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
 *  Check that integer insertion works properly in error.
 *
 *  @return 0 on success.
 */
int main() {
  // Insert integers.
  error e;
  e << 0 << -1 << 42;
  e << 753159 << INT_MAX << 1 << INT_MIN;

  // Conversion reference.
  std::ostringstream oss;
  oss << 0 << -1 << 42 << 753159 << INT_MAX << 1 << INT_MIN;

  // Check.
  return (strcmp(oss.str().c_str(), e.what()));
}

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
 *  Check that insertion of longs works properly in error.
 *
 *  @return 0 on success.
 */
int main() {
  // Insert longs.
  error e;
  e << 78l << 96321l << 0l << -1l << LONG_MIN;
  e << LONG_MAX;
  e << LONG_MAX - 42l;

  // Conversion reference.
  std::ostringstream oss;
  oss << 78l << 96321l << 0l << -1l << LONG_MIN << LONG_MAX << LONG_MAX - 42l;

  // Check.
  return (strcmp(oss.str().c_str(), e.what()));
}

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
 *  Check that long long insertion works properly in error.
 *
 *  @return 0 on success.
 */
int main() {
  // Insert long longs.
  error e;
  e << 74654646541515ll << 0ll << -1ll << 5465451141125787ll;

#ifdef LLONG_MAX
  e << LLONG_MAX << 42ll << LLONG_MIN;
#endif  // !LLONG_MAX

  // Conversion reference.
  std::ostringstream oss;
  oss << 74654646541515ll << 0ll << -1ll << 5465451141125787ll;

#ifdef LLONG_MAX
  oss << LLONG_MAX << 42ll << LLONG_MIN;
#endif  // !LLONG_MAX

  // Check.
  return (strcmp(oss.str().c_str(), e.what()));
}

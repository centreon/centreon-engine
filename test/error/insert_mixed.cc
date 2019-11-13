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

#include <string.h>
#include <string>
#include "com/centreon/engine/error.hh"

using namespace com::centreon::engine;

/**
 *  Check that mixed insertions work with error.
 *
 *  @return 0 on success.
 */
int main() {
  // Insertions.
  error e;
  e << 'f' << "oo" << std::string(" bar baz") << 42 << 789654321456ull
    << 3000000000u << std::string() << 'q' << 'u' << "x";

  // Check.
  return (strcmp("foo bar baz427896543214563000000000qux", e.what()));
}

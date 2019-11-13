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
#include "com/centreon/engine/error.hh"

using namespace com::centreon::engine;

/**
 *  Check that the assignment operator works properly on error.
 *
 *  @return 0 on success.
 */
int main() {
  // Base object.
  error e1;
  e1 << "foo "
     << "bar" << 42u;

  // Copy object.
  error e2;
  e2 << "baz"
     << " qux" << 21l;
  e2 = e1;

  // Change object object.
  e1 << 3612;

  // Check.
  return (strcmp(e1.what(), "foo bar423612") || strcmp(e2.what(), "foo bar42"));
}

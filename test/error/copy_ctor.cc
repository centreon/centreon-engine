/*
** Copyright 2011 Merethis
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

using namespace com::centreon::engine;

/**
 *  Check that error copy constructor works properly.
 *
 *  @return 0 on success.
 */
int main() {
  // Base object.
  error e1;
  e1 << "foo" << 42 << "bar" << 23459498498748ull << " baz";
  e1.set_fatal(false);

  // Copy base object.
  error e2(e1);

  // Change base object.
  e1 << 0;
  e1.set_fatal(true);

  // Check.
  return (strcmp(e1.what(), "foo42bar23459498498748 baz0")
          || (e1.is_fatal() != true)
          || strcmp(e2.what(), "foo42bar23459498498748 baz")
          || (e2.is_fatal() != false));
}

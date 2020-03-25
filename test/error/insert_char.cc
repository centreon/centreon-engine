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
#include "com/centreon/engine/exceptions/error.hh"

using namespace com::centreon::engine;

/**
 *  Check that chars can be properly inserted in error.
 *
 *  @return 0 on success.
 */
int main() {
  // Insert chars.
  error e;
  e << 'f' << 'o' << 'o';
  e << ' ';
  e << 'b' << 'a' << 'r';

  // Check insertion.
  return (strcmp("foo bar", e.what()));
}

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

#include "com/centreon/engine/commands/environment.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

// External environment array.
extern "C" char** environ;

/**
 *  Check environment constructor.
 */
static int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  environment env1(environ);
  environment env2(environ);
  if (!(env1 == env2))
    throw(engine_error() << "invalid equal operator");
  if (env1 != env2)
    throw(engine_error() << "invalid not equal operator");
  environment env3;
  if (env1 == env3)
    throw(engine_error() << "invalid equal operator");
  if (!(env1 != env3))
    throw(engine_error() << "invalid not equal operator");
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

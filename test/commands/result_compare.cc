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

#include <cstdlib>
#include <exception>
#include "com/centreon/engine/commands/result.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/process.hh"
#include "com/centreon/timestamp.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

#define DEFAULT_ID 42
#define DEFAULT_OUTPUT "output string test"
#define DEFAULT_RETURN 0
#define DEFAULT_STATUS process::normal

/**
 *  Check the comparison operator.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Prepare.
  timestamp now(timestamp::now());
  result res;
  res.command_id = DEFAULT_ID;
  res.output = DEFAULT_OUTPUT;
  res.start_time = now;
  res.end_time = now;
  res.exit_code = DEFAULT_RETURN;
  res.exit_status = DEFAULT_STATUS;

  // Tests.
  if (!(res == res))
    throw(engine_error() << "error: operator== failed");
  if (res != res)
    throw(engine_error() << "error: operator!= failed");

  return (EXIT_SUCCESS);
}

/**
 *  Init unit test.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return Return value of main_test().
 *
 *  @see main_test
 */
int main(int argc, char* argv[]) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

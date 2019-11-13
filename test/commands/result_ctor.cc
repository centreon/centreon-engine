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
#include <ctime>
#include <exception>
#include "com/centreon/engine/commands/result.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/process.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

#define DEFAULT_ID 42
#define DEFAULT_OUTPUT "output string test"
#define DEFAULT_RETURN 0
#define DEFAULT_STATUS process::normal

/**
 *  Check the constructor and copy object.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Default constructor.
  result res1;
  if ((res1.command_id != 0) || (res1.output != "") || (res1.exit_code != 0) ||
      (res1.exit_status != process::normal) || (res1.start_time != 0) ||
      (res1.end_time != 0))
    throw(engine_error() << "error: default constructor failed");

  // Constructor.
  com::centreon::timestamp now(com::centreon::timestamp::now());
  result res2;
  res2.command_id = DEFAULT_ID;
  res2.output = DEFAULT_OUTPUT;
  res2.start_time = now;
  res2.end_time = now;
  res2.exit_code = DEFAULT_RETURN;
  res2.exit_status = DEFAULT_STATUS;
  if ((res2.command_id != DEFAULT_ID) || (res2.output != DEFAULT_OUTPUT) ||
      (res2.exit_code != DEFAULT_RETURN) ||
      (res2.exit_status != DEFAULT_STATUS) || (res2.start_time != now) ||
      (res2.end_time != now))
    throw(engine_error() << "error: constructor failed");

  // Copy constructor.
  result res3(res2);
  if (res2 != res3)
    throw(engine_error() << "error: copy constructor failed");

  // Assignment operator.
  result res4;
  res4 = res3;
  if (res2 != res4)
    throw(engine_error() << "error: assignment operator failed");

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

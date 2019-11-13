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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/logging/engine.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Run acknowledge_host_problem test.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return EXIT_SUCCESS on success.
 */
static int check_acknowledge_host_problem(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Create target host.
  host* hst(add_host("name", NULL, NULL, "localhost", NULL, 0, 0.0, 0.0, 42, 0,
                     0, 0, 0, 0, 0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0, 0, 0.0,
                     0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL,
                     NULL, NULL, NULL, NULL, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0,
                     0));
  if (!hst)
    throw(engine_error() << "host creation failed");
  hst->current_state = HOST_DOWN;
  hst->problem_has_been_acknowledged = false;

  // Send external command.
  char const* cmd(
      "[1317196300] ACKNOWLEDGE_HOST_PROBLEM;name;1;1;1;user;comment");
  process_external_command(cmd);

  // Check.
  if (!hst->problem_has_been_acknowledged)
    throw(engine_error() << "acknowledge_host_problem failed");

  // Cleanup.
  cleanup();

  return (EXIT_SUCCESS);
}

/**
 *  Init unit test.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &check_acknowledge_host_problem);
  return (utest.run());
}

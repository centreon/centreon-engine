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
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/logging/engine.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Run add_svc_comment test.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return EXIT_SUCCESS on success.
 */
static int check_add_svc_comment(int argc, char** argv) {
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

  // Create target service.
  service* svc(add_service(
      "name", "description", NULL, NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL,
      0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, "command", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, 0, 0));
  if (!svc)
    throw(engine_error() << "service creation failed");

  // Send external command.
  char const* cmd(
      "[1317196300] ADD_SVC_COMMENT;name;description;1;user;comment");
  process_external_command(cmd);

  // Check.
  if (!comment_list)
    throw(engine_error() << "add_svc_comment failed");

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
  unittest utest(argc, argv, &check_add_svc_comment);
  return (utest.run());
}

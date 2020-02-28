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
 *  Run schedule_and_propagate_triggered_host_downtime test.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return EXIT_SUCCESS on success.
 */
static int check_schedule_and_propagate_triggered_host_downtime(int argc,
                                                                char** argv) {
  (void)argc;
  (void)argv;

  // Create target hosts.
  host* hst_parent(add_host("parent", NULL, NULL, "localhost", NULL, 0, 0.0,
                            0.0, 42, 0, 0, 0, 0, 0, 0.0, 0.0, NULL, 0, NULL, 0,
                            0, NULL, 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0,
                            NULL, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL,
                            NULL, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0));
  host* hst_child(add_host("child", NULL, NULL, "localhost", NULL, 0, 0.0, 0.0,
                           42, 0, 0, 0, 0, 0, 0.0, 0.0, NULL, 0, NULL, 0, 0,
                           NULL, 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL,
                           0, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0,
                           0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0));
  if (!hst_parent || !hst_child)
    throw(engine_error() << "hosts creation failed");
  add_parent_host_to_host(hst_child, "parent");
  add_child_link_to_host(hst_parent, hst_child);

  // Send external command.
  char const* cmd(
      "[1317196300] "
      "SCHEDULE_AND_PROPAGATE_TRIGGERED_HOST_DOWNTIME;parent;1317196300;"
      "2000000000;0;0;7200;user;comment");
  process_external_command(cmd);

  // Check.
  if (!scheduled_downtime_list)
    throw(engine_error()
          << "schedule_and_propagate_triggered_host_downtime failed");

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
  unittest utest(argc, argv,
                 &check_schedule_and_propagate_triggered_host_downtime);
  return (utest.run());
}

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

#include <exception>
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/logging/engine.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Run disable_all_notifications_beyond_host test.
 */
static int check_disable_all_notifications_beyond_host(int argc, char** argv) {
  (void)argc;
  (void)argv;

  host* hst_parent =
      add_host("parent", NULL, NULL, "localhost", NULL, 0, 0.0, 0.0, 42, 0, 0,
               0, 0, 0, 0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0, 0, 0.0, 0.0, 0,
               0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL, NULL, NULL,
               NULL, NULL, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  host* hst_child =
      add_host("child", NULL, NULL, "localhost", NULL, 0, 0.0, 0.0, 42, 0, 0, 0,
               0, 0, 0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0, 0, 0.0, 0.0, 0, 0,
               0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL,
               NULL, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  if (!hst_parent || !hst_child)
    throw(engine_error() << "create host failed.");

  add_parent_host_to_host(hst_child, "parent");
  add_child_link_to_host(hst_parent, hst_child);
  hst_child->notifications_enabled = true;
  char const* cmd("[1317196300] DISABLE_ALL_NOTIFICATIONS_BEYOND_HOST;parent");
  process_external_command(cmd);

  if (hst_child->notifications_enabled)
    throw(engine_error() << "disable_all_notifications_beyond_host failed.");
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &check_disable_all_notifications_beyond_host);
  return (utest.run());
}

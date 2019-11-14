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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/logging/engine.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Run enter_active_mode test.
 */
static int check_enter_active_mode(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Action.
  config->enable_notifications(false);
  char const* cmd("[1317196300] ENTER_ACTIVE_MODE");
  process_external_command(cmd);

  // Check.
  if (!config->enable_notifications())
    throw(engine_error() << "enter_active_mode failed");

  // Success.
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &check_enter_active_mode);
  return (utest.run());
}

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
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/logging/engine.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Run process_service_check_result test.
 */
static int check_process_service_check_result(int argc, char** argv) {
  (void)argc;
  (void)argv;

  service* svc = add_service(
      "name", "description", NULL, NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL,
      0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, "command", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, 0, 0);
  if (!svc)
    throw(engine_error() << "create service failed.");

  host* hst =
      add_host("name", NULL, NULL, "localhost", NULL, 0, 0.0, 0.0, 42, 0, 0, 0,
               0, 0, 0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0, 0, 0.0, 0.0, 0, 0,
               0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL, NULL, NULL, NULL,
               NULL, 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  if (!hst)
    throw(engine_error() << "create host failed.");

  svc->accept_passive_service_checks = true;
  char const* cmd(
      "[1317196300] PROCESS_SERVICE_CHECK_RESULT;name;description;0;output");
  process_external_command(cmd);

  if (com::centreon::engine::checks::checker::instance().reaper_is_empty())
    throw(engine_error() << "process_service_check_result failed.");
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &check_process_service_check_result);
  return (utest.run());
}

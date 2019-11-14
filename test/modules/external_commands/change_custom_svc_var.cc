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
 *  Run change_custom_svc_var test.
 */
static int check_change_custom_svc_var(int argc, char** argv) {
  (void)argc;
  (void)argv;

  service* svc = add_service(
      "name", "description", NULL, NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL,
      0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, "command", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL, NULL, NULL, 0, 0, 0);
  if (!svc)
    throw(engine_error() << "create service failed.");

  customvariablesmember* member =
      add_custom_variable_to_service(svc, "_VAR", "TEST");
  if (!member)
    throw(engine_error() << "create customvariablesmember failed.");

  char const* cmd(
      "[1317196300] CHANGE_CUSTOM_SVC_VAR;name;description;_VAR;VALUE");
  process_external_command(cmd);

  if (strcmp(member->variable_value, "VALUE"))
    throw(engine_error() << "change_custom_svc_var failed.");
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &check_change_custom_svc_var);
  return (utest.run());
}

/*
** Copyright 2011-2012 Merethis
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
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/shared_ptr.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

/**
 *  Check if the command exist in the set command.
 *
 *  @param[in] name The command name.
 *
 *  @return True if the command exist, false otherwise.
 */
static bool command_exist(std::string const& name) {
  try {
    set::instance().get_command(name);
  }
  catch (...) {
    return (false);
  }
  return (true);
}

/**
 *  Check if the change name system works.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Get instance.
  set& cmd_set(set::instance());

  // Add command.
  raw raw("raw", "raw argv1 argv2");
  cmd_set.add_command(raw);

  // Get command.
  shared_ptr<commands::command> cmd(cmd_set.get_command("raw"));

  // Change command name.
  cmd->set_name("cmd");

  // Get command with new name.
  shared_ptr<commands::command> new_cmd(cmd_set.get_command("cmd"));

  // Check if the old command name is not found.
  if (command_exist("raw"))
    throw (engine_error() << "error: command name changed failed");

  // Remove new name.
  cmd_set.remove_command("cmd");

  // Check if the old command name is not found.
  if (command_exist("cmd"))
    throw (engine_error() << "error: command name changed failed");

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

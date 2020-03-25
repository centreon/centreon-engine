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
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "test/unittest.hh"

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
  } catch (...) {
    return (false);
  }
  return (true);
}

/**
 *  Check if the set command works.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Get instance.
  set& cmd_set(set::instance());

  // Add commands.
  raw raw1("raw1", "raw1 argv1 argv2");
  cmd_set.add_command(raw1);
  std::shared_ptr<commands::command> pcmd2(raw1.clone());
  cmd_set.add_command(pcmd2);
  std::shared_ptr<commands::command> pcmd3(
      new raw("pcmd3", "pcmd3 argv1 argv2"));
  cmd_set.add_command(pcmd3);

  // Get commands.
  if (!command_exist("raw1"))
    throw(engine_error() << "error: get_command failed, 'raw1' not found");
  if (!command_exist("pcmd3"))
    throw(engine_error() << "error: get_command failed, 'pcmd3' not found");
  if (command_exist("undef"))
    throw(engine_error() << "error: get_command failed, 'undef' found");

  // Remove commands.
  cmd_set.remove_command("pcmd3");
  if (command_exist("pcmd3"))
    throw(engine_error() << "error: remove_command failed, 'pcmd3' found");
  cmd_set.remove_command("raw1");
  if (command_exist("raw1"))
    throw(engine_error() << "error: remove_command failed, 'raw1' found");
  cmd_set.remove_command("undef");

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

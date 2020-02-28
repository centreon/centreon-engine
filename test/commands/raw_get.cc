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
#include "com/centreon/engine/exceptions/error.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

#define DEFAULT_CMD_NAME "cmd"
#define DEFAULT_CMD_LINE "ls -la /tmp"

/**
 *  Check getter return.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  raw cmd(DEFAULT_CMD_NAME, DEFAULT_CMD_LINE);

  if (cmd.get_name() != DEFAULT_CMD_NAME)
    throw(engine_error() << "error: name invalid value.");

  if (cmd.get_command_line() != DEFAULT_CMD_LINE)
    throw(engine_error() << "error: command_line invalid value.");

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

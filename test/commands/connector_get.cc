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
#include "com/centreon/engine/commands/connector.hh"
#include "com/centreon/engine/error.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

#define DEFAULT_CONNECTOR_NAME __func__
#define DEFAULT_CONNECTOR_LINE "./bin_connector_test_run"

/**
 *  Check getter return.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  connector cmd(DEFAULT_CONNECTOR_NAME, DEFAULT_CONNECTOR_LINE);

  if (cmd.get_name() != DEFAULT_CONNECTOR_NAME)
    throw(engine_error() << "error: name invalid value.");

  if (cmd.get_command_line() != DEFAULT_CONNECTOR_LINE)
    throw(engine_error() << "error: command_line invalid value.");

  return (0);
}

/**
 *  Init the unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

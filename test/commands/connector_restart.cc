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

#include <ctime>
#include <exception>
#include <sstream>
#include "com/centreon/engine/commands/connector.hh"
#include "com/centreon/engine/commands/forward.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/process.hh"
#include "test/commands/wait_process.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

#define DEFAULT_CONNECTOR_NAME __func__
#define DEFAULT_CONNECTOR_LINE "./bin_connector_test_run"
#define DEFAULT_CMD_NAME __FILE__

/**
 *  Check the restart of the connector when it stop.
 *
 *  @return True if connector restart after segfault.
 */
static bool restart_with_segfault() {
  nagios_macros macros = nagios_macros();
  std::string command_line;
  {
    std::ostringstream oss;
    oss << "./bin_connector_test_run --kill=" << time(NULL);
    command_line = oss.str();
  }
  connector cmd_connector(DEFAULT_CONNECTOR_NAME, DEFAULT_CONNECTOR_LINE);
  forward cmd_forward(DEFAULT_CMD_NAME, command_line, cmd_connector);
  wait_process wait_proc(&cmd_connector);

  unsigned long id(cmd_forward.run(cmd_forward.get_command_line(), macros, 0));
  wait_proc.wait();

  result const& res(wait_proc.get_result());
  if (res.command_id != id || res.exit_code != STATE_OK ||
      res.output != cmd_forward.get_command_line() ||
      res.exit_status == process::timeout)
    return (false);
  return (true);
}

/**
 *  Check the restart of the connector.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  if (restart_with_segfault() == false)
    throw(engine_error() << "error: restart connector after segfault failed.");

  return (0);
}

/**
 *  Init the unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

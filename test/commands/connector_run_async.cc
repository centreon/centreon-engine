/*
** Copyright 2011-2013,2015 Merethis
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
 *  Check if the connector result are ok without timeout.
 *
 *  @return true if ok, false otherwise.
 */
static bool run_without_timeout() {
  nagios_macros macros = nagios_macros();
  connector cmd_connector(DEFAULT_CONNECTOR_NAME, DEFAULT_CONNECTOR_LINE);
  forward cmd_forward(DEFAULT_CMD_NAME,
                      "./bin_connector_test_run --timeout=off", cmd_connector);
  wait_process wait_proc(&cmd_connector);

  unsigned long id(cmd_forward.run(cmd_forward.get_command_line(), macros, 0));
  wait_proc.wait();

  result const& res(wait_proc.get_result());
  if (res.command_id != id || res.exit_code != STATE_OK ||
      res.output != cmd_forward.get_command_line() ||
      res.exit_status != process::normal)
    return (false);
  return (true);
}

/**
 *  Check if the connector result are ok with timeout.
 *
 *  @return true if ok, false otherwise.
 */
static bool run_with_timeout() {
  nagios_macros macros = nagios_macros();
  connector cmd_connector(DEFAULT_CONNECTOR_NAME, DEFAULT_CONNECTOR_LINE);
  forward cmd_forward(DEFAULT_CMD_NAME, "./bin_connector_test_run --timeout=on",
                      cmd_connector);
  wait_process wait_proc(&cmd_connector);

  unsigned long id(cmd_forward.run(cmd_forward.get_command_line(), macros, 1));
  wait_proc.wait();

  result const& res(wait_proc.get_result());
  if (res.command_id != id || res.exit_code != STATE_UNKNOWN ||
      res.output != "(Process Timeout)" || res.exit_status != process::timeout)
    return (false);
  return (true);
}

/**
 *  Check if the connector result are ok with some macros arguments.
 *
 *  @return true if ok, false otherwise.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  if (run_without_timeout() == false)
    throw(engine_error() << "error: raw::run without timeout failed.");

  if (run_with_timeout() == false)
    throw(engine_error() << "error: raw::run with timeout failed.");

  return (0);
}

/**
 *  Init the unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

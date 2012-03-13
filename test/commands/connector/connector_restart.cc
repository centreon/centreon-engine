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
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include "com/centreon/engine/commands/connector/command.hh"
#include "com/centreon/engine/error.hh"
#include "test/commands/wait_process.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

/**
 *  Check the restart of the connector when it stop.
 *
 *  @return True if connector restart after segfault.
 */
static bool restart_with_segfault() {
  nagios_macros macros = nagios_macros();
  QString command_line = "./bin_connector_test_run --kill="
    + QString("%1").arg(QDateTime::currentDateTime().toTime_t());
  connector::command cmd(__func__,
			 "./bin_connector_test_run",
                         __func__,
			 command_line);

  wait_process wait_proc(cmd);

  unsigned long id = cmd.run(cmd.get_command_line(), macros, 0);
  wait_proc.wait();

  result const& cmd_res = wait_proc.get_result();
  if (cmd_res.get_command_id() != id
      || cmd_res.get_exit_code() != STATE_OK
      || cmd_res.get_stdout() != cmd.get_command_line()
      || cmd_res.get_stderr() != ""
      || cmd_res.get_is_executed() == false
      || cmd_res.get_is_timeout() == true) {
    return (false);
  }
  return (true);
}

/**
 *  Check the restart of the connector with a limit of execution.
 *
 *  @return True if connector restart after a number limit of execution.
 */
static bool restart_with_execution_limit() {
  nagios_macros macros = nagios_macros();
  connector::command cmd(__func__,
        		 "./bin_connector_test_run",
                         __func__,
        		 "./bin_connector_test_run --timeout=off");

  cmd.set_max_check_for_restart(2);
  wait_process wait_proc(cmd);

  for (unsigned int i = 0; i < 3; ++i) {
    unsigned long id = cmd.run(cmd.get_command_line(), macros, 0);
    wait_proc.wait();

    result const& cmd_res = wait_proc.get_result();
    if (cmd_res.get_command_id() != id
        || cmd_res.get_exit_code() != STATE_OK
        || cmd_res.get_stdout() != cmd.get_command_line()
        || cmd_res.get_stderr() != ""
        || cmd_res.get_is_executed() == false
        || cmd_res.get_is_timeout() == true) {
      return (false);
    }
  }
  return (true);
}

/**
 *  Check the restart of the connector.
 */
int main_test() {
  if (restart_with_segfault() == false)
    throw (engine_error() << "error: restart connector after segfault failed.");

  if (restart_with_execution_limit() == false)
    throw (engine_error() << "error: restart connector after execution limit failed.");

  return (0);
}

/**
 *  Init the unit test.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  unittest utest(&main_test);
  QObject::connect(&utest, SIGNAL(finished()), &app, SLOT(quit()));
  utest.start();
  app.exec();
  return (utest.ret());
}

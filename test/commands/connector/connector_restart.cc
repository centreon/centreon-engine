/*
** Copyright 2011 Merethis
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

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <exception>
#include "commands/connector/command.hh"
#include "engine.hh"
#include "../wait_process.hh"

using namespace com::centreon::engine::commands;

/**
 *  Check the restart of the connector when it stop.
 *
 *  @return True if connector restart after segfault.
 */
static bool restart_with_segfault() {
  nagios_macros macros = nagios_macros();
  QString command_line = "./bin_connector_test_run --kill="
    + QString("%1").arg(QDateTime::currentDateTime().toMSecsSinceEpoch());
  connector::command cmd(__func__,
			 command_line,
			 "./bin_connector_test_run");
  wait_process wait_proc(cmd);

  unsigned long id = cmd.run(cmd.get_command_line(), macros, -1);
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
			 "./bin_connector_test_run --timeout=off",
			 "./bin_connector_test_run");

  cmd.set_max_check_for_restart(2);
  wait_process wait_proc(cmd);

  for (unsigned int i = 0; i < 3; ++i) {
    unsigned long id = cmd.run(cmd.get_command_line(), macros, -1);
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
int main(int argc, char** argv) {
  try {
    QCoreApplication app(argc, argv);

    if (restart_with_segfault() == false) {
      qDebug() << "error: restart connector after segfault failed.";
      return (1);
    }

    if (restart_with_execution_limit() == false) {
      qDebug() << "error: restart connector after execution limit failed.";
      return (1);
    }
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}

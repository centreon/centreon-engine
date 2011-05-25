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

#include <QDebug>
#include <exception>
#include "commands/raw.hh"
#include "engine.hh"
#include "wait_process.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

bool run_without_timeout() {
  nagios_macros macros = nagios_macros();
  raw cmd(__func__, "./bin_test_run --timeout=off");
  end_process end_proc(cmd);

  unsigned long id = cmd.run(cmd.get_command_line(), macros, -1);
  end_proc.wait();

  result const& cmd_res = end_proc.get_result();
  if (cmd_res.get_cmd_id() != id
      || cmd_res.get_retval() != STATE_OK
      || cmd_res.get_execution_time() == 0
      || cmd_res.get_stdout() != cmd.get_command_line()
      || cmd_res.get_stderr() != ""
      || cmd_res.get_exited_ok() == false
      || cmd_res.get_is_timeout() == true) {
    return (false);
  }
  return (true);
}

bool run_with_timeout() {
  nagios_macros macros = nagios_macros();
  raw cmd(__func__, "./bin_test_run --timeout=on");
  end_process end_proc(cmd);

  unsigned long id = cmd.run(cmd.get_command_line(), macros, 1);
  end_proc.wait();

  result const& cmd_res = end_proc.get_result();
  if (cmd_res.get_cmd_id() != id
      || cmd_res.get_retval() != STATE_CRITICAL
      || cmd_res.get_execution_time() == 0
      || cmd_res.get_stdout() != ""
      || cmd_res.get_stderr() != "Process Timeout"
      || cmd_res.get_exited_ok() == true
      || cmd_res.get_is_timeout() == false) {
    return (false);
  }
  return (true);
}

bool run_with_environement_macros() {
  nagios_macros macros = nagios_macros();
  raw cmd(__func__, "./bin_test_run --check_macros");
  end_process end_proc(cmd);

  char const* argv = "default_arg";
  macros.argv[0] = new char[strlen(argv) + 1];
  strcpy(macros.argv[0], argv);

  unsigned long id = cmd.run(cmd.get_command_line(), macros, -1);
  end_proc.wait();

  result const& cmd_res = end_proc.get_result();
  if (cmd_res.get_cmd_id() != id
      || cmd_res.get_retval() != STATE_OK
      || cmd_res.get_execution_time() == 0
      || cmd_res.get_stdout() != cmd.get_command_line()
      || cmd_res.get_stderr() != ""
      || cmd_res.get_exited_ok() == false
      || cmd_res.get_is_timeout() == true) {
    return (false);
  }
  return (true);
}

int main(int argc, char** argv) {
  try {
    QCoreApplication app(argc, argv);

    if (run_without_timeout() == false) {
      qDebug() << "error: raw::run without timeout failed.";
      return (1);
    }
    if (run_with_timeout() == false) {
      qDebug() << "error: raw::run with timeout failed.";
      return (1);
    }
    if (run_with_environement_macros() == false) {
      qDebug() << "error: raw::run with macros failed.";
      return (1);
    }
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }

  return (0);
}

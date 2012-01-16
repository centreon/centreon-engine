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
#include <QDebug>
#include <exception>
#include "error.hh"
#include "test/unittest.hh"
#include "commands/raw.hh"
#include "engine.hh"
#include "test/commands/wait_process.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

/**
 *  Check if the command line result are ok without timeout.
 *
 *  @return true if ok, false otherwise.
 */
static bool run_without_timeout() {
  nagios_macros macros = nagios_macros();
  raw cmd(__func__, "./bin_test_run --timeout=off");
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
 *  Check if the command line result are ok with timeout.
 *
 *  @return true if ok, false otherwise.
 */
static bool run_with_timeout() {
  nagios_macros macros = nagios_macros();
  raw cmd(__func__, "./bin_test_run --timeout=on");
  wait_process wait_proc(cmd);

  unsigned long id = cmd.run(cmd.get_command_line(), macros, 1);
  wait_proc.wait();

  result const& cmd_res = wait_proc.get_result();
  if (cmd_res.get_command_id() != id
      || cmd_res.get_exit_code() != STATE_CRITICAL
      || cmd_res.get_stdout() != ""
      || cmd_res.get_stderr() != "(Process Timeout)"
      || cmd_res.get_is_executed() == false
      || cmd_res.get_is_timeout() == false) {
    return (false);
  }
  return (true);
}

/**
 *  Check if the command line result are ok with some macros arguments.
 *
 *  @return true if ok, false otherwise.
 */
static bool run_with_environement_macros() {
  nagios_macros macros = nagios_macros();
  raw cmd(__func__, "./bin_test_run --check_macros");
  wait_process wait_proc(cmd);

  char const* argv = "default_arg";
  macros.argv[0] = new char[strlen(argv) + 1];
  strcpy(macros.argv[0], argv);

  unsigned long id = cmd.run(cmd.get_command_line(), macros, 0);
  wait_proc.wait();
  delete[] macros.argv[0];

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
 *  Check the asynchrone system for the raw command.
 */
int main_test() {
  if (run_without_timeout() == false)
    throw (engine_error() << "error: raw::run without timeout failed.");

  if (run_with_timeout() == false)
    throw (engine_error() << "error: raw::run with timeout failed.");

  if (run_with_environement_macros() == false)
    throw (engine_error() << "error: raw::run with macros failed.");
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  unittest utest(&main_test);
  QObject::connect(&utest, SIGNAL(finished()), &app, SLOT(quit()));
  utest.start();
  app.exec();
  return (utest.ret());
}

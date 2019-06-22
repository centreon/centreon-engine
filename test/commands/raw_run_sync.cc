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

#include <cstdlib>
#include <cstring>
#include <exception>
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/process.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

/**
 *  Check if the command line result are ok without timeout.
 *
 *  @return true if ok, false otherwise.
 */
static bool run_without_timeout() {
  // Raw command object.
  raw cmd(__func__, "./bin_test_run --timeout=off");

  // Run command.
  nagios_macros mac;
  result res;
  cmd.run(cmd.get_command_line(), mac, 0, res);

  // Check result.
  return (!((res.command_id == 0)
            || (res.exit_code != STATE_OK)
            || (res.exit_status != process::normal)
            || (res.output != cmd.get_command_line())));
}

/**
 *  Check if the command line result are ok with timeout.
 *
 *  @return true if ok, false otherwise.
 */
static bool run_with_timeout() {
  // Raw command object.
  raw cmd(__func__, "./bin_test_run --timeout=on");

  // Run command.
  nagios_macros mac;
  result res;
  cmd.run(cmd.get_command_line(), mac, 1, res);

  // Check result.
  return (!((res.command_id == 0)
            || (res.exit_code != STATE_UNKNOWN)
            || (res.exit_status != process::timeout)
            || (res.output != "(Process Timeout)")));
}

/**
 *  Check if the command line result are ok with some macros arguments.
 *
 *  @return true if ok, false otherwise.
 */
static bool run_with_environment_macros() {
  // Enable environment macros in configuration.
  config->enable_environment_macros(true);

  // Get environment macros.
  nagios_macros mac;
  char const* argv("default_arg");
  mac.argv[0] = new char[strlen(argv) + 1];
  strcpy(mac.argv[0], argv);

  // Raw command object.
  raw cmd(__func__, "./bin_test_run --check_macros");

  // Run command.
  result res;
  cmd.run(cmd.get_command_line(), mac, 0, res);
  delete [] mac.argv[0];

  // Check result.
  return (!((res.command_id == 0)
            || (res.exit_code != STATE_OK)
            || (res.exit_status != process::normal)
            || (res.output != cmd.get_command_line())));
}

/**
 *  Check if single quotes are supported.
 *
 *  @return true if ok, false otherwise.
 */
static bool run_with_single_quotes() {
  // Raw command object.
  raw cmd(__func__, "'./bin_test_run' '--timeout'='off'");

  // Run command.
  nagios_macros mac;
  result res;
  cmd.run(cmd.get_command_line(), mac, 0, res);

  // Check result.
  return (!((res.command_id == 0)
            || (res.exit_code != STATE_OK)
            || (res.exit_status != process::normal)
            || (res.output != "./bin_test_run --timeout=off")));
}

/**
 *  Check if double quotes are supported.
 *
 *  @return true if ok, false otherwise.
 */
static bool run_with_double_quotes() {
  // Raw command object.
  raw cmd(__func__, "\"./bin_test_run\" \"--timeout\"=\"off\"");

  // Run command.
  nagios_macros mac;
  result res;
  cmd.run(cmd.get_command_line(), mac, 0, res);

  // Check result.
  return (!((res.command_id == 0)
            || (res.exit_code != STATE_OK)
            || (res.exit_status != process::normal)
            || (res.output != "./bin_test_run --timeout=off")));
}

/**
 *  Check the synchronous system for the raw command.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;
  if (!run_without_timeout())
    throw (engine_error() << "raw::run without timeout failed");
  if (!run_with_timeout())
    throw (engine_error() << "raw::run with timeout failed");
  if (!run_with_environment_macros())
    throw (engine_error() << "raw::run with macros failed");
  if (!run_with_single_quotes())
    throw (engine_error() << "raw::run with single quotes failed");
  if (!run_with_double_quotes())
    throw (engine_error() << "raw::run with double quotes failed");
  return (EXIT_SUCCESS);
}

/**
 *  Process entry point.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return Return value from main_test().
 *
 *  @see main_test
 */
int main(int argc, char* argv[]) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

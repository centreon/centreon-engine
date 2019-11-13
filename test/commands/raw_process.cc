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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/string.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

#define CMD_HOSTADDR "localhost"
#define CMD_USER1 "/usr/bin"
#define CMD_ARG1 "default_arg"
#define CMD_LINE "$USER1$/test -w $ARG1$ -c $$ARG1$$ $HOSTADDRESS$ $EMPTY$"
#define CMD_PROCESSED \
  CMD_USER1 "/test -w " CMD_ARG1 " -c $ARG1$ " CMD_HOSTADDR " "

/**
 *  Check the process command line replacement macros.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  nagios_macros macros = nagios_macros();

  // add macros arg1.
  macros.argv[0] = new char[strlen(CMD_ARG1) + 1];
  strcpy(macros.argv[0], CMD_ARG1);

  // add macros user1.
  string::setstr(macro_user[0], CMD_USER1);

  // add macros hostaddress.
  string::setstr(macro_x_names[MACRO_HOSTADDRESS], "HOSTADDRESS");

  host hst = host();
  hst.address = new char[strlen(CMD_HOSTADDR) + 1];
  strcpy(hst.address, CMD_HOSTADDR);
  macros.host_ptr = &hst;

  // process command.
  raw cmd(__func__, CMD_LINE);
  std::string cmd_processed(cmd.process_cmd(&macros));

  delete[] hst.address;
  hst.address = NULL;
  delete[] macros.argv[0];
  macros.argv[0] = NULL;

  if (cmd_processed != CMD_PROCESSED)
    throw(engine_error() << "command::process failed.");

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

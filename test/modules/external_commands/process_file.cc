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
#include <fstream>
#include <stdio.h>
#include <string>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/logging/engine.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Run process_file test.
 */
static int check_process_file(int argc, char** argv) {
  (void)argc;
  (void)argv;

  char* tmp(tempnam("./", "extc."));
  try {
    if (!tmp)
      throw (engine_error() << "bad_alloc");

    std::ofstream file(tmp, std::ios_base::trunc | std::ios_base::out);
    if (!file.is_open())
      throw (engine_error() << "impossible to create temporary file.");
    file << "[1317196300] ENABLE_NOTIFICATIONS" << std::endl;
    file.close();

    config->enable_notifications(false);
    std::string cmd("[1317196300] PROCESS_FILE;");
    cmd.append(tmp);
    cmd.append(";0\n");
    process_external_command(cmd.c_str());

    remove(tmp);
    free(tmp);
    tmp = NULL;
    if (!config->enable_notifications())
      throw (engine_error() << "process_file failed.");
  }
  catch (...) {
    if (tmp)
      free(tmp);
    throw;
  }
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &check_process_file);
  return (utest.run());
}

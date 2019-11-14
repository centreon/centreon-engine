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
#include <string>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/io/file_stream.hh"
#include "com/centreon/logging/engine.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

/**
 *  Run process_file test.
 */
static int check_process_file(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Generate temporary file name.
  std::string tmp;
  {
    char* ptr(io::file_stream::temp_path());
    if (!ptr)
      throw(engine_error() << "cannot generate temporary name");
    tmp = ptr;
  }

  // Open file stream.
  std::ofstream file(tmp.c_str(), std::ios_base::trunc | std::ios_base::out);
  if (!file.is_open())
    throw(engine_error() << "cannot create temporary file");
  file << "[1317196300] ENABLE_NOTIFICATIONS" << std::endl;
  file.close();

  // Send external command.
  config->enable_notifications(false);
  std::string cmd("[1317196300] PROCESS_FILE;");
  cmd.append(tmp);
  cmd.append(";0\n");
  process_external_command(cmd.c_str());

  // Cleanup.
  io::file_stream::remove(tmp);
  if (!config->enable_notifications())
    throw(engine_error() << "process_file failed");

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &check_process_file);
  return (utest.run());
}

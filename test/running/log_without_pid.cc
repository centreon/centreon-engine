/*
** Copyright 2012-2013 Merethis
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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include "com/centreon/engine/error.hh"
#include "com/centreon/io/file_stream.hh"
#include "com/centreon/engine/process.hh"
#include "test/paths.hh"

using namespace com::centreon::engine;

/**
 *  See if the log contain pid
 *
 *  @param[in] content  The content of the log.
 *
 *  @return             True if the log contain the pid.
 */
static bool test_log(std::string const& content) {
  unsigned long timestamp;
  unsigned long pid;
  return (::sscanf(content.c_str(), "[%lu] [%lu]", &timestamp, &pid) == 1);
}

/**
 *  Check that log files don't contain [pid] when this option is not set.
 *
 *  As it stands, probably overkill vs a simple parsing check.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main() {
  int retval(EXIT_FAILURE);

  // Temporary file names.
  std::string conf_file = TEST_DIR "/running/etc/log_without_pid/main.cfg";
  std::string log_file = "nagios_without_pid.log";

  try {
    // Run centengine to generate precache file.
    {
      // Generate command-line.
      std::string cmdline;
      cmdline = CENTENGINE_BINARY;
      cmdline.append(" '");
      cmdline.append(conf_file);
      cmdline.append("'");

      // Launch process.
      process centengine;
      centengine.exec(cmdline);
      centengine.wait(1000);
      centengine.terminate();
      centengine.wait(1000);
      centengine.kill();
    }

    // See if the log file contains a pid.
    {
      // Read log file.
      std::string log_content;
      io::file_stream log_gen;
      log_gen.open(log_file.c_str(), "r");
      char buffer[1024];
      unsigned long bytes;
      while ((bytes = log_gen.read(buffer, sizeof(buffer))) > 0)
        log_content.append(buffer, bytes);

      // test log contents.
      if (!test_log(log_content))
        throw (engine_error() << "log with pid");

      // Success.
      retval = EXIT_SUCCESS;
    }
  }
  catch (std::exception const& e) {
    std::cout << e.what() << std::endl;
  }


  // Remove temporary files.
  ::remove(log_file.c_str());

  return (retval);
}

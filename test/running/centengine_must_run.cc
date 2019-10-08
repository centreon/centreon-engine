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

#include <cstdlib>
#include <ctime>
#include <iostream>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/process.hh"
#include "test/paths.hh"

using namespace com::centreon::engine;

/**
 *  Check that Centreon Engine runs a configuration for some time.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main(int argc, char* argv[]) {
  int retval(EXIT_FAILURE);
  try {
    // Check arguments.
    if (argc < 2)
      throw (engine_error() << "USAGE: "
             << argv[0] << " <config> [timeout]");
    time_t timeout;
    if (argc >= 3)
      timeout = strtoul(argv[2], NULL, 0);
    else
      timeout = 5;

    // Build command line.
    std::string cmd(CENTENGINE_BINARY);
    cmd.append(" '");
    cmd.append(argv[1]);
    cmd.append("'");

    // Run process.
    com::centreon::engine::process centengined;
    centengined.enable_stream(com::centreon::engine::process::in, false);
    centengined.enable_stream(com::centreon::engine::process::out, false);
    centengined.enable_stream(com::centreon::engine::process::err, false);
    centengined.exec(cmd);
    bool reached_timeout(!centengined.wait(timeout * 1000));
    if (!reached_timeout)
      throw (engine_error() << "timeout has not been reached");

    // Terminate process gracefully.
    centengined.terminate();
    if (!centengined.wait(5000))
      centengined.kill();

    // Reaching here means success.
    retval = (((centengined.exit_status()
                == com::centreon::engine::process::normal)
               && (centengined.exit_code() == EXIT_SUCCESS))
              ? EXIT_SUCCESS
              : EXIT_FAILURE);
  }
  catch (std::exception const& e) {
    std::cerr << e.what() << std::endl;
  }
  catch (...) {
    std::cerr << "unknown error" << std::endl;
  }

  return (retval);
}

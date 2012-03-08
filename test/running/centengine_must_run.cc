/*
** Copyright 2012 Merethis
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

#include <iostream>
#include <QCoreApplication>
#include <QProcess>
#include <QStringList>
#include <stdlib.h>
#include <time.h>
#include "com/centreon/engine/error.hh"
#include "test/paths.hh"

using namespace com::centreon::engine;

/**
 *  Check that Centreon Engine runs a configuration for some time.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return 0 on success.
 */
int main(int argc, char* argv[]) {
  int retval(EXIT_FAILURE);
  try {
    // Qt base object.
    QCoreApplication app(argc, argv);

    // Check arguments.
    if (argc < 2)
      throw (engine_error() << "USAGE: "
             << argv[0] << " <config> [timeout]");
    time_t timeout;
    if (argc >= 3)
      timeout = strtoul(argv[2], NULL, 0);
    else
      timeout = 5;

    // Build argument list.
    QStringList args;
    args.push_back(argv[1]);

    // Run process.
    QProcess centengined;
    centengined.start(CENTENGINE_BINARY, args, QIODevice::NotOpen);
    if (!centengined.waitForStarted())
      throw (engine_error() << "could not start " CENTENGINE_BINARY);
    bool reached_timeout(!centengined.waitForFinished(timeout * 1000));
    if (!reached_timeout)
      throw (engine_error() << "timeout has not been reached");

    // Terminate process gracefully.
    centengined.terminate();
    if (!centengined.waitForFinished(5000))
      centengined.kill();

    // Reaching here means success.
    retval = (((centengined.exitStatus() == QProcess::NormalExit)
               && (centengined.exitCode() == EXIT_SUCCESS))
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

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
#include <stdlib.h>
#include "com/centreon/engine/error.hh"
#include "test/modules/webservice/engine.hh"
#include "test/modules/webservice/query.hh"

#define HOST_NAME "my_host"

/**
 *  Check host custom variables.
 *
 *  @return 0 on success.
 */
int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // Return value.
  int retval;

  // Start Engine.
  engine e;
  e.start();

  try {
    // Webservice query.
    query q;
    std::string output;
    int retcode;

    // Create host.
    output.clear();
    retcode = q.execute(
                  output,
                  "host_add "
                  "host_id=" HOST_NAME " "
                  "alias=" HOST_NAME " "
                  "address=localhost "
                  "max_check_attempts=5 "
                  "check_period=mytimeperiod "
                  "notification_interval=5 "
                  "notification_period=mytimeperiod "
                  "contacts=mycontact ");
    if (retcode)
      throw (engine_error() << "cannot add host: " << output);

    // XXX

    // Shutdown Engine.
    e.stop();

    // Success.
    retval = EXIT_SUCCESS;
  }
  catch (std::exception const& x) {
    std::cerr << x.what() << std::endl;
    retval = EXIT_FAILURE;
    e.stop();
  }
  catch (...) {
    std::cerr << "unknown exception" << std::endl;
    retval = EXIT_FAILURE;
    e.stop();
  }

  return (retval);
}

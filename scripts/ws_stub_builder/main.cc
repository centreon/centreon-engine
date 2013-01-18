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

#include <iostream>
#include <exception>
#include <stdlib.h>

#include "builder.hh"

using namespace com::centreon::engine::script;

/**
 *  This function show the application's usage.
 *
 *  @param[in] appname The application name.
 */
static void usage(char const* appname) {
  std::cerr << "usage: " << appname
	    << " soapStub.hh auto_gen.hh auto_gen.cc"
	    << std::endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
  // Check the number of arguments.
  if (argc != 4) {
    usage(argv[0]);
    return (EXIT_FAILURE);
  }

  try {
    // Init builder.
    builder build(argv[1], argv[2], argv[3]);

    build.parse(); // Parse the soapStub.hh.
    build.build(); // Build auto_gen header and source.
  }
  catch (std::exception const& e) {
    // Display error.
    std::cerr << "error: " << e.what() << std::endl;
    return (EXIT_FAILURE);
  }
  catch (...) {
    // Display error.
    std::cerr << "error: catch all." << std::endl;
    return (EXIT_FAILURE);
  }
  return (EXIT_SUCCESS);
}

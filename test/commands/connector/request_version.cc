/*
** Copyright 2011-2012 Merethis
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
#include "com/centreon/engine/commands/connector/version_query.hh"
#include "com/centreon/engine/commands/connector/version_response.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/version.hh"
#include "test/commands/connector/check_request.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands::connector;

#define QUERY         "0" CMD_END
#define RESPONSE      "1\0" TOSTR(CENTREON_ENGINE_VERSION_MAJOR) \
                      "\0" TOSTR(CENTREON_ENGINE_VERSION_MINOR) CMD_END

/**
 *  Check the version request.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  version_query query;
  if (check_request_valid(&query, REQUEST(QUERY)) == false)
    throw (engine_error() << "error: query is valid failed.");
  if (check_request_invalid(&query) == false)
    throw (engine_error() << "error: query is invalid failed.");
  if (check_request_clone(&query) == false)
    throw (engine_error() << "error: query clone failed");

  version_response response(
                     CENTREON_ENGINE_VERSION_MAJOR,
                     CENTREON_ENGINE_VERSION_MINOR);
  if (check_request_valid(&response, REQUEST(RESPONSE)) == false)
    throw (engine_error() << "error: response is valid failed.");
  if (check_request_invalid(&response) == false)
    throw (engine_error() << "error: response is invalid failed.");
  if (check_request_clone(&response) == false)
    throw (engine_error() << "error: response clone failed");

  return (0);
}

/**
 *  Init the unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

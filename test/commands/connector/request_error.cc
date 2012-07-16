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
#include "com/centreon/engine/commands/connector/error_response.hh"
#include "com/centreon/engine/error.hh"
#include "test/commands/connector/check_request.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands::connector;

#define MESSAGE  "warning"
#define CODE     1
#define RESPONSE "6\0" TOSTR(CODE) "\0" MESSAGE CMD_END

/**
 *  Check the error request.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  error_response response(MESSAGE, static_cast<error_response::e_code>(CODE));
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

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

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <thread>
#include "com/centreon/engine/common.hh"

using namespace com::centreon;

/**
 *  Simulate some behavior of plugin.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument values.
 *
 *  @return STATE_OK on success.
 */
int main(int argc, char* argv[]) {
  // Output.
  std::cout << argv[0];
  for (int i(1); i < argc; ++i)
    std::cout << " " << argv[i];

  // Not enough or too much argument = warning.
  if (argc != 2)
    return (STATE_WARNING);

  // Never return to test the timeout.
  if (!strcmp(argv[1], "--timeout=on"))
    while (true)
      std::this_thread::sleep_for(std::chrono::seconds(1));

  // Check a classic return.
  if (!strcmp(argv[1], "--timeout=off"))
    return (STATE_OK);

  // Check macros argument are ok.
  if (!strcmp(argv[1], "--check_macros")) {
    char const* arg = getenv("NAGIOS_ARG1");
    if (arg != NULL && !strcmp(arg, "default_arg"))
      return (STATE_OK);
    return (STATE_CRITICAL);
  }

  return (STATE_UNKNOWN);
}

/*
** Copyright 2011 Merethis
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
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "engine.hh"

int main(int argc, char** argv) {
  for (int i = 0; i < argc; ++i) {
    std::cout << argv[i];
    if (i + 1 != argc) {
      std::cout << " ";
    }
  }

  if (argc != 2) {
    return (STATE_WARNING);
  }

  if (!strcmp(argv[1], "--timeout=on")) {
    while (true) {
      sleep(1);
    }
  }

  if (!strcmp(argv[1], "--timeout=off")) {
    return (STATE_OK);
  }

  if (!strcmp(argv[1], "--check_macros")) {
    char const* arg = getenv("NAGIOS_ARG1");
    if (arg != NULL && !strcmp(arg, "default_arg")) {
      return (STATE_OK);
    }
    return (STATE_CRITICAL);
  }

  return (STATE_UNKNOWN);
}

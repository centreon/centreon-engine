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

#include <libgen.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

static int const STATUS_OK(0);
static int const STATUS_WARNING(1);
static int const STATUS_CRITICAL(2);
static int const STATUS_UNKNOWN(3);

static void usage(char const* appname) {
  std::cout << appname << " [-h] [-s status] [-t timeout] [text]" << std::endl;
  std::cout << " -h: This help." << std::endl
            << " -s: The status return at the end of the execution."
            << std::endl
            << " -t: The plugin timeout." << std::endl;
  exit(STATUS_UNKNOWN);
}

int main(int argc, char** argv) {
  char* appname(basename(argv[0]));
  int status(-1);
  int timeout(-1);
  int last_state(STATUS_UNKNOWN);

  int opt;
  while ((opt = getopt(argc, argv, "s:t:l:")) != -1) {
    switch (opt) {
      case 's':
        status = atoi(optarg);
        break;

      case 't':
        timeout = atoi(optarg);
        break;

      case 'l':
        last_state = atoi(optarg);
        if (last_state > STATUS_UNKNOWN || last_state < STATUS_OK)
          last_state = STATUS_UNKNOWN;
        break;

      default:
        usage(appname);
        break;
    }
  }

  srand(getpid());

  if (status == -1) {
    if (last_state && (rand() % 9))
      status = last_state;
    else {
      int randomval(rand() % 100);
      if (randomval < 2)
        status = STATUS_UNKNOWN;
      else if (randomval < 4)
        status = STATUS_WARNING;
      else if (randomval < 6)
        status = STATUS_CRITICAL;
      else
        status = STATUS_OK;
    }
  }
  if (timeout == -1)
    timeout = ((rand() % 9) % 6);

  if (timeout)
    sleep(timeout);

  switch (status) {
    case STATUS_OK:
      std::cout << "OK";
      break;

    case STATUS_WARNING:
      std::cout << "WARNING";
      break;

    case STATUS_CRITICAL:
      std::cout << "CRITICAL";
      break;

    default:
      std::cout << "UNKNOWN";
      break;
  }

  std::cout << ": timeout=" << timeout << ", status=" << status;
  if (optind < argc)
    std::cout << ", output='" << argv[optind] << "'";
  std::cout << "|timeout=" << timeout << ";status=" << status;
  std::cout << std::endl;
  return (status);
}

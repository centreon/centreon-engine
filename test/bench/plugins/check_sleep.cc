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
#include <libgen.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

static int const STATUS_OK(0);
static int const STATUS_WARNING(1);
static int const STATUS_CRITICAL(2);
static int const STATUS_UNKNOWN(3);

static void usage(char const* appname) {
  std::cout << appname
            << " [-h] [-s status] [-t timeout] [text]"
            << std::endl;
  std::cout << " -h: This help." << std::endl
            << " -s: The status return at the end of the execution." << std::endl
            << " -t: The plugin timeout." << std::endl;
  exit(STATUS_UNKNOWN);
}

int main (int argc, char **argv) {
  char* appname(basename(argv[0]));
  int status(-1);
  int timeout(-1);

  int opt;
  while ((opt = getopt(argc, argv, "s:t:")) != -1) {
    switch (opt) {
    case 's':
      status = atoi(optarg);
      break;

    case 't':
      timeout = atoi(optarg);
      break;

    default:
      usage(appname);
      break;
    }
  }

  srand(getpid());

  if (status == -1) {
    if ((status = rand() % 10) > STATUS_UNKNOWN)
      status = STATUS_OK;
  }
  if (timeout == -1)
    timeout = ((rand() % 15) % 10) + 1;

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

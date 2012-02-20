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

#include <stdlib.h>
#include <unistd.h>
#include "com/centreon/engine/engine.hh"
#include "sighandlers.h"

#define HOST_TIMEOUT_OUTPUT "(Host Check Timed Out)\n"
#define SERVICE_TIMEOUT_OUTPUT "(Service Check Timed Out)\n"

extern "C" {
  /**
   *  Handle timeouts when executing host checks.
   *
   *  @param[in] sig Signal number.
   */
  void host_check_sighandler(int sig) {
    (void)sig;

    // Write output.
    char const* output(HOST_TIMEOUT_OUTPUT);
    size_t size(sizeof(HOST_TIMEOUT_OUTPUT) - 1);
    ssize_t wb(42);
    while ((wb > 0) && size) {
      wb = write(STDOUT_FILENO, output, size);
      size -= wb;
      output += wb;
    }

    // Exit.
    _Exit(STATE_CRITICAL);

    return ;
  }

  /**
   *  Handle timeouts when executing commands via my_system_r().
   *
   *  @param[in] sig Signal number.
   */
  void my_system_sighandler(int sig) {
    (void)sig;

    // Exit.
    _Exit(STATE_CRITICAL);

    return ;
  }

  /**
   *  Handle timeouts when executing service checks.
   *
   *  @param[in] sig Signal number.
   */
  void service_check_sighandler(int sig) {
    (void)sig;

    // Write output.
    char const* output(SERVICE_TIMEOUT_OUTPUT);
    size_t size(sizeof(SERVICE_TIMEOUT_OUTPUT) - 1);
    ssize_t wb(42);
    while ((wb > 0) && size) {
      wb = write(STDOUT_FILENO, output, size);
      size -= wb;
      output += wb;
    }

    // Exit.
    _Exit(STATE_UNKNOWN);

    return ;
  }
}

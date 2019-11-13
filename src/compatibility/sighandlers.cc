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

#include "sighandlers.h"
#include <unistd.h>
#include <com/centreon/engine/notifier.hh>
#include <com/centreon/engine/objects.hh>
#include <cstdlib>
#include "common.h"

#define HOST_TIMEOUT_OUTPUT "(Host Check Timed Out)\n"
#define SERVICE_TIMEOUT_OUTPUT "(Service Check Timed Out)\n"

using namespace com::centreon::engine;

/**
 *  Helper signal handler function.
 *
 *  @param[in] msg  Message to write on output.
 *  @param[in] size Size of msg.
 *  @param[in] ret  Exit code.
 */
static void sighandler_helper(char const* msg, unsigned int size, int ret) {
  // Write output.
  ssize_t wb(42);
  while ((wb > 0) && size) {
    wb = write(STDOUT_FILENO, msg, size);
    size -= wb;
    msg += wb;
  }

  // Exit.
  _Exit(ret);
  return;
}

extern "C" {
/**
 *  Handle timeouts when executing host checks.
 *
 *  @param[in] sig Signal number.
 */
void host_check_sighandler(int sig) {
  (void)sig;
  sighandler_helper(HOST_TIMEOUT_OUTPUT, sizeof(HOST_TIMEOUT_OUTPUT) - 1,
                    service::state_unknown);
  return;
}

/**
 *  Handle timeouts when executing commands via my_system_r().
 *
 *  @param[in] sig Signal number.
 */
void my_system_sighandler(int sig) {
  (void)sig;
  sighandler_helper(NULL, 0, service::state_unknown);
  return;
}

/**
 *  Handle timeouts when executing service checks.
 *
 *  @param[in] sig Signal number.
 */
void service_check_sighandler(int sig) {
  (void)sig;
  sighandler_helper(SERVICE_TIMEOUT_OUTPUT, sizeof(SERVICE_TIMEOUT_OUTPUT) - 1,
                    service::state_unknown);
  return;
}
}

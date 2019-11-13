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

#include <stdlib.h>
#include <iostream>
#include "com/centreon/engine/globals.hh"
#include "com/centreon/library.hh"
#include "common.h"
#include "nebmodules.h"

using namespace com::centreon;

// Specify the event broker API version.
NEB_API_VERSION(CURRENT_NEB_API_VERSION)

extern "C" {
/**
 *  @brief Module exit point.
 *
 *  @param[in] flags  Unused.
 *  @param[in] reason Unused.
 *
 *  @return OK.
 */
int nebmodule_deinit(int flags, int reason) {
  (void)flags;
  (void)reason;
  return (OK);
}

/**
 *  @brief Module entry point.
 *
 *  Use multiple symbols that should be defined by Centreon Engine.
 *  Create a file after successful execution.
 *
 *  @param[in] flags  Unused.
 *  @param[in] args   Unused.
 *  @param[in] handle MOdule handle.
 *
 *  @return Will have exit()'ed before.
 */
int nebmodule_init(int flags, char const* args, void* handle) {
  // Unused args.
  (void)flags;
  (void)handle;

  // We will exit right after module loading.
  try {
    // Load module with required symbols.
    library lib(args);
    lib.load();
  } catch (std::exception const& e) {
    std::cerr << "error: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  } catch (...) {
    // Exception means failure.
    exit(EXIT_FAILURE);
  }
  sigshutdown = true;
  return (OK);
}
}

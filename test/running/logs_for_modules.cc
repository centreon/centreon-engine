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

#include <cstdlib>
#include <cstring>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/nebcallbacks.hh"
#include "com/centreon/engine/nebmodules.hh"

using namespace com::centreon::engine;

/**
 *  Check that Centreon Engine generate some logging and debug events
 *  for broker modules.
 */

extern "C" {
// NEB API.
NEB_API_VERSION(CURRENT_NEB_API_VERSION)

/**
 *  Log callback.
 *
 *  @param[in]
 */
int log_callback(int type, void* data) {
  (void)type;
  (void)data;
  sigshutdown = true;
  return (0);
}

/**
 *  Module exit point.
 *
 *  @param[in] flags  Flags.
 *  @param[in] reason Unload reason.
 *
 *  @return 0 on success.
 */
int nebmodule_deinit(unsigned int flags, unsigned int reason) {
  (void)flags;
  (void)reason;
  return (0);
}

/**
 *  Module entry point.
 *
 *  @param[in] flags  Flags.
 *  @param[in] args   Arguments.
 *  @param[in] handle Module handle.
 *
 *  @return 0 on success.
 */
int nebmodule_init(unsigned int flags, char const* args, void* handle) {
  (void)flags;
  (void)args;

  // Register callback.
  if (neb_register_callback(NEBCALLBACK_LOG_DATA, handle, 0, &log_callback))
    throw(engine_error() << "cannot register log callback");

  return (0);
}
}

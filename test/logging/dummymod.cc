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
#include <iostream>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/nebmods.hh"
#include "com/centreon/engine/nebstructs.hh"

using namespace com::centreon::engine;

/**************************************
 *                                     *
 *           Global Objects            *
 *                                     *
 **************************************/

// Specify the event broker API version.
NEB_API_VERSION(CURRENT_NEB_API_VERSION)

// Test message.
static const char* LOG_MESSAGE = "~!@#$%^&*()_+09/qwerty \n";

/**************************************
 *                                     *
 *         Callback Function           *
 *                                     *
 **************************************/

/**
 *  @brief Function that process log data.
 *
 *  This function is called by broker when an event log are
 *  received.
 *
 *  @param[in] callback_type Type of the callback.
 *  @param[in] data          A pointer to a nebstruct_log_data
 *
 *  @return 0 on success.
 */
int callback(int callback_type, void* data) {
  (void)callback_type;

  nebstruct_log_data* neb_log = static_cast<nebstruct_log_data*>(data);
  if (neb_log->type != NEBTYPE_LOG_DATA || neb_log->flags != NEBFLAG_NONE ||
      neb_log->attr != NEBATTR_NONE) {
    return (0);
  }

  if (strcmp(neb_log->data, LOG_MESSAGE) ||
      (neb_log->data_type & logging::log_all) == 0) {
    std::cerr << "error: bad value in module" << std::endl;
    exit(EXIT_FAILURE);
  }

  return (0);
}

/**************************************
 *                                     *
 *         Exported Functions          *
 *                                     *
 **************************************/

/**
 *  @brief Module exit point.
 *
 *  This function is called when the module gets unloaded by Centreon-Engine.
 *  It will deregister all previously registered callbacks and perform
 *  some shutdown stuff.
 *
 *  @param[in] flags  XXX
 *  @param[in] reason XXX
 *
 *  @return 0 on success, any other value on failure.
 */
extern "C" int nebmodule_deinit(int flags, int reason) {
  (void)flags;
  (void)reason;

  neb_deregister_callback(NEBCALLBACK_LOG_DATA, callback);
  return (0);
}

/**
 *  @brief Module entry point.
 *
 *  This function is called when the module gets loaded by Centreon-Engine. It
 *  will register callbacks to catch events and perform some initialization
 *  stuff like config file parsing, thread creation, ...
 *
 *  @param[in] flags  XXX
 *  @param[in] args   The argument string of the module (shall contain the
 *                    configuration file name).
 *  @param[in] handle The module handle.
 *
 *  @return 0 on success, any other value on failure.
 */
extern "C" int nebmodule_init(int flags, char const* args, void* handle) {
  (void)flags;
  (void)args;

  // Set module informations.
  neb_set_module_info(handle, NEBMODULE_MODINFO_TITLE, "Dummy module");
  neb_set_module_info(handle, NEBMODULE_MODINFO_AUTHOR, "Merethis");
  neb_set_module_info(handle, NEBMODULE_MODINFO_COPYRIGHT,
                      "Copyright 2011 Merethis");
  neb_set_module_info(handle, NEBMODULE_MODINFO_VERSION, "1.0.0");
  neb_set_module_info(handle, NEBMODULE_MODINFO_LICENSE, "GPL version 2");
  neb_set_module_info(handle, NEBMODULE_MODINFO_DESC, "Dummy module.");

  // Register callbacks.
  if (neb_register_callback(NEBCALLBACK_LOG_DATA, handle, 0, callback) != 0) {
    std::cerr << "register callback failed" << std::endl;
    exit(EXIT_FAILURE);
  }

  return (0);
}

/*
** Copyright 2011-2013,2017 Centreon
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

#include <sys/types.h>
#include <unistd.h>
#include <cstddef>
#include <exception>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/modules/external_commands/utils.hh"
#include "com/centreon/engine/nebcallbacks.hh"
#include "com/centreon/engine/nebmodules.hh"
#include "com/centreon/engine/nebstructs.hh"

using namespace com::centreon::engine::logging;

/**************************************
 *                                     *
 *           Global Objects            *
 *                                     *
 **************************************/

// Specify the event broker API version.
NEB_API_VERSION(CURRENT_NEB_API_VERSION)

// Module handle
static void* gl_mod_handle(NULL);

/**************************************
 *                                     *
 *         Callback Function           *
 *                                     *
 **************************************/

/**
 *  @brief Function that process external command.
 *
 *  This function is called by Centreon-Engine when external
 *  command are check.
 *
 *  @param[in] callback_type Type of the callback.
 *  @param[in] data          A pointer to a nebstruct_external_command_data.
 *
 *  @return 0 on success.
 */
int callback_external_command(int callback_type, void* data) {
  (void)callback_type;

  nebstruct_external_command_data* neb_data =
      static_cast<nebstruct_external_command_data*>(data);
  if (neb_data->type != NEBTYPE_EXTERNALCOMMAND_CHECK ||
      neb_data->flags != NEBFLAG_NONE || neb_data->attr != NEBATTR_NONE ||
      neb_data->command_type != CMD_NONE || neb_data->command_string != NULL ||
      neb_data->command_args != NULL)
    return (0);

  try {
    check_for_external_commands();
  } catch (...) {
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
 *  @param[in] flags  Unused.
 *  @param[in] reason Unused.
 *
 *  @return 0 on success, any other value on failure.
 */
extern "C" int nebmodule_deinit(int flags, int reason) {
  (void)flags;
  (void)reason;

  try {
    neb_deregister_callback(NEBCALLBACK_EXTERNAL_COMMAND_DATA,
                            callback_external_command);

    // Close and delete the external command file FIFO.
    shutdown_command_file_worker_thread();
    close_command_file();
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
        << "external command runtime error `" << e.what() << "'.";
  } catch (...) {
    logger(log_runtime_error, basic)
        << "external command runtime error `unknown'";
  }
  return (0);
}

/**
 *  @brief Module entry point.
 *
 *  This function is called when the module gets loaded by Centreon-Engine. It
 *  will register callbacks to catch events and perform some initialization
 *  stuff like config file parsing, thread creation, ...
 *
 *  @param[in] flags  Unused.
 *  @param[in] args   Unused.
 *  @param[in] handle The module handle.
 *
 *  @return 0 on success, any other value on failure.
 */
extern "C" int nebmodule_init(int flags, char const* args, void* handle) {
  (void)args;
  (void)flags;

  // Save module handle for future use.
  gl_mod_handle = handle;

  // Set module informations.
  neb_set_module_info(gl_mod_handle, NEBMODULE_MODINFO_TITLE,
                      "Centreon-Engine's external command");
  neb_set_module_info(gl_mod_handle, NEBMODULE_MODINFO_AUTHOR, "Merethis");
  neb_set_module_info(gl_mod_handle, NEBMODULE_MODINFO_COPYRIGHT,
                      "Copyright 2011 Merethis");
  neb_set_module_info(gl_mod_handle, NEBMODULE_MODINFO_VERSION, "1.0.0");
  neb_set_module_info(gl_mod_handle, NEBMODULE_MODINFO_LICENSE,
                      "GPL version 2");
  neb_set_module_info(gl_mod_handle, NEBMODULE_MODINFO_DESC,
                      "Centreon-Engine's external command provide system to "
                      "execute commands over a pipe.");

  try {
    // Open the command file (named pipe) for reading.
    if (open_command_file() != OK) {
      logger(log_process_info | log_runtime_error, basic)
          << "Bailing out due to errors encountered while trying to "
          << "initialize the external command file ... "
          << "(PID=" << getpid() << ")";
      return (1);
    }

    // Register callbacks.
    if (neb_register_callback(NEBCALLBACK_EXTERNAL_COMMAND_DATA, gl_mod_handle,
                              0, callback_external_command)) {
      throw(engine_error() << "register callback failed");
    }
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
        << "external command runtime error `" << e.what() << "'.";
    return (1);
  } catch (...) {
    logger(log_runtime_error, basic)
        << "external command runtime error `unknown'.";
    return (1);
  }

  return (0);
}

/**
 *  @brief Module reloading.
 *
 *  This function is a placeholder to prevent Centreon Engine from
 *  complaining about impossible module reload.
 */
extern "C" int nebmodule_reload() {
  return (0);
}

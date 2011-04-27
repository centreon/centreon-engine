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

#include <exception>
#include <stddef.h>
#include "nebmodules.hh"
#include "nebcallbacks.hh"
#include "nebstructs.hh"
#include "broker.hh"
#include "logging.hh"
#include "configuration.hh"
#include "error.hh"
#include "syncro.hh"
#include "webservice.hh"

using namespace com::centreon::engine::modules;

/**************************************
 *                                     *
 *           Global Objects            *
 *                                     *
 **************************************/

// Specify the event broker API version.
NEB_API_VERSION(CURRENT_NEB_API_VERSION)

// Module handle
static void*          gl_mod_handle = NULL;
static webservice*    gl_webservice = NULL;
static configuration* gl_config = NULL;

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

  nebstruct_external_command_data* neb_data = static_cast<nebstruct_external_command_data*>(data);
  if (neb_data->type != NEBTYPE_EXTERNALCOMMAND_CHECK_END
      || neb_data->flags != NEBFLAG_NONE
      || neb_data->attr != NEBATTR_NONE
      || neb_data->command_type != CMD_NONE
      || neb_data->command_string != NULL
      || neb_data->command_args != NULL)
    return (0);

  try {
    syncro::instance().wakeup_worker();
  }
  catch (...) {

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

  try {
    delete gl_webservice;
    delete gl_config;

    neb_deregister_callback(NEBCALLBACK_EXTERNAL_COMMAND_DATA,
			    callback_external_command);
  }
  catch (std::exception const& e) {
      logit(NSLOG_RUNTIME_ERROR, false,
	    "webservice runtime error `%s'.\n",
	    e.what());
  }
  catch (...) {
      logit(NSLOG_RUNTIME_ERROR, false,
	    "webservice runtime error `unknown'\n");
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
 *  @param[in] flags  XXX
 *  @param[in] args   The argument string of the module (shall contain the
 *                    configuration file name).
 *  @param[in] handle The module handle.
 *
 *  @return 0 on success, any other value on failure.
 */
extern "C" int nebmodule_init(int flags, char const* args, void* handle) {
  (void)flags;

  // Save module handle for future use.
  gl_mod_handle = handle;

  // Set module informations.
  neb_set_module_info(gl_mod_handle,
		      NEBMODULE_MODINFO_TITLE,
		      "Centreon-Engine's Webservice");
  neb_set_module_info(gl_mod_handle,
		      NEBMODULE_MODINFO_AUTHOR,
		      "Merethis");
  neb_set_module_info(gl_mod_handle,
		      NEBMODULE_MODINFO_COPYRIGHT,
		      "Copyright 2011 Merethis");
  neb_set_module_info(gl_mod_handle,
		      NEBMODULE_MODINFO_VERSION,
		      "1.0.0");
  neb_set_module_info(gl_mod_handle,
		      NEBMODULE_MODINFO_LICENSE,
		      "GPL version 2");
  neb_set_module_info(gl_mod_handle,
		      NEBMODULE_MODINFO_DESC,
		      "Centreon-Engine's Webservice provide system to " \
		      "execute commands over a network with a Simple Object" \
		      "Access Protocol.");

  try {
    // Initialise configuration.
    gl_config = new configuration;
    if (args != NULL) {
      gl_config->set_filename(args);
      gl_config->parse();
    }

    // Register callbacks.
    if (neb_register_callback(NEBCALLBACK_EXTERNAL_COMMAND_DATA,
			      gl_mod_handle,
			      0,
			      callback_external_command) != 0) {
      throw (engine_error() << "register callback failed");
    }

    // Initialise webservice.
    gl_webservice = new webservice(*gl_config);
    gl_webservice->start();
  }
  catch (std::exception const& e) {
      logit(NSLOG_RUNTIME_ERROR, false,
	    "webservice runtime error `%s'.\n",
	    e.what());
    return (1);
  }
  catch (...) {
      logit(NSLOG_RUNTIME_ERROR, false,
	    "webservice runtime error `unknown'.\n");
    return (1);
  }

  return (0);
}


/*
** Copyright 2011-2014 Merethis
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
#include <cstdlib>
#include <iostream>
#include <string>
#include "backup.hh"
#include "com/centreon/concurrency/thread.hh"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/reload.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/nebmods.hh"
#include "com/centreon/engine/nebstructs.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

/**************************************
 *                                     *
 *           Global Objects            *
 *                                     *
 **************************************/

// Specify the event broker API version.
NEB_API_VERSION(CURRENT_NEB_API_VERSION)
static std::string gl_config_path;

/**************************************
 *                                     *
 *               Class                 *
 *                                     *
 **************************************/

class checkmodify {
 public:
  checkmodify(std::string const& cfg_path) : _cfg_path(cfg_path) {}
  ~checkmodify() throw() {}
  void load_configuration() {
    backup::set_to_null();
    configuration::applier::state::unload();
    delete config;
    config = new configuration::state;
    configuration::applier::state::load();
    ::config->cfg_main(_cfg_path);
    reload_configuration();
  }
  static void reload_configuration() {
    configuration::reload reload_configuration;
    reload_configuration.start();
    while (true) {
      concurrency::thread::yield();
      if (reload_configuration.is_finished())
        break;
      reload_configuration.try_lock();
    }
  }
  void save_current_configuration() {
    _backup = backup(*::config, configuration::applier::state::instance());
  }
  void verify() {
    _backup.is_equal(*::config);
    _backup.is_equal(configuration::applier::state::instance());
  }

 private:
  std::string _cfg_path;
  backup _backup;
};

/**************************************
 *                                     *
 *         Callback Function           *
 *                                     *
 **************************************/

/**
 *  @brief Function that process log data.
 *
 *  This function is called by broker when an event on the event loop
 *  are received.
 *
 *  @param[in] callback_type Type of the callback.
 *  @param[in] neb_data      A pointer to a nebstruct_process_data
 *
 *  @return 0 on success.
 */
int callback_event_loop(int callback_type, void* neb_data) {
  try {
    if (!neb_data || callback_type != NEBCALLBACK_PROCESS_DATA)
      throw(engine_error() << "bad arguments into callback_event_loop_end");

    nebstruct_process_data& data(
        *static_cast<nebstruct_process_data*>(neb_data));
    if (data.type == NEBTYPE_PROCESS_EVENTLOOPSTART) {
      ::config->cfg_main(gl_config_path);
      sigshutdown = true;
    } else if (data.type == NEBTYPE_PROCESS_EVENTLOOPEND) {
      checkmodify::reload_configuration();

      checkmodify chkm(gl_config_path);
      chkm.save_current_configuration();
      chkm.load_configuration();
      chkm.verify();
    }
  } catch (std::exception const& e) {
    std::cerr << "error: " << e.what() << std::endl;
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

  neb_deregister_callback(NEBCALLBACK_PROCESS_DATA, callback_event_loop);
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
 *  @param[in] args   The argument string of the module (shall contain the
 *                    first configuration file and the second configuration
 *                    files).
 *  @param[in] handle The module handle.
 *
 *  @return 0 on success, any other value on failure.
 */
extern "C" int nebmodule_init(int flags, char const* args, void* handle) {
  (void)flags;

  try {
    if (!args)
      throw(engine_error() << "can not load module checkmodify: "
                              "bad argument");

    gl_config_path = args;

    // Set module informations.
    neb_set_module_info(handle, NEBMODULE_MODINFO_TITLE,
                        "Check modify configuration module");
    neb_set_module_info(handle, NEBMODULE_MODINFO_AUTHOR, "Merethis");
    neb_set_module_info(handle, NEBMODULE_MODINFO_COPYRIGHT,
                        "Copyright 2013 Merethis");
    neb_set_module_info(handle, NEBMODULE_MODINFO_VERSION, "1.0.0");
    neb_set_module_info(handle, NEBMODULE_MODINFO_LICENSE, "GPL version 2");
    neb_set_module_info(handle, NEBMODULE_MODINFO_DESC,
                        "Check modify configuration module.");

    // Register callbacks event loop.
    if (neb_register_callback(NEBCALLBACK_PROCESS_DATA, handle, 0,
                              callback_event_loop))
      throw(engine_error() << "can not load module checkmodify: "
                              "register callback failed");
  } catch (std::exception const& e) {
    std::cerr << "error: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  return (0);
}

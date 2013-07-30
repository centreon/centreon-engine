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
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include "com/centreon/concurrency/thread.hh"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/configuration/reload.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/nebmods.hh"
#include "com/centreon/engine/nebstructs.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

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
*              Function               *
*                                     *
**************************************/

template<class Key, class T, class Hash, class Pred>
bool compare_with_true_contents(
                umap<Key, T, Hash, Pred> const& lhs,
                umap<Key, T, Hash, Pred> const& rhs) {
  if (lhs.size() != rhs.size())
    return (false);
  for (typename umap<Key, T, Hash, Pred>::const_iterator
         it(lhs.begin()), end(lhs.end());
       it != end;
       ++it) {
    typename umap<Key, T, Hash, Pred>::const_iterator
      it_find(rhs.find(it->first));
    if (it_find == rhs.end() || *it_find->second != *it->second)
      return (false);
  }
  return (true);
}

template<class Key, class T, class Hash, class Pred>
bool compare_with_true_contents(
       umultimap<Key, T, Hash, Pred> const& lhs,
       umultimap<Key, T, Hash, Pred> const& rhs) {
  if (lhs.size() != rhs.size())
    return (false);
  for (typename umap<Key, T, Hash, Pred>::const_iterator
         it(lhs.begin()), end(lhs.end());
       it != end;
       ++it) {
    bool find(false);
    for (typename umap<Key, T, Hash, Pred>::const_iterator
           it_find(rhs.find(it->first)), end(rhs.end());
         it_find != end && it_find->first == it->first;
         ++it_find) {
      if (*it_find->second == *it->second) {
        find = true;
        break;
      }
    }
    if (!find)
      return (false);
  }
  return (true);
}

/**************************************
*                                     *
*               Class                 *
*                                     *
**************************************/

struct                 obj_state {
  umap<std::string, shared_ptr<command_struct> >
                       commands;
  umap<std::string, shared_ptr<commands::connector> >
                       connectors;
  umap<std::string, shared_ptr<contact_struct> >
                       contacts;
  umap<std::string, shared_ptr<contactgroup_struct> >
                       contactgroups;
  umap<std::string, shared_ptr<host_struct> >
                       hosts;
  umultimap<std::string, shared_ptr<hostdependency_struct> >
                       hostdependencies;
  umultimap<std::string, shared_ptr<hostescalation_struct> >
                       hostescalations;
  umap<std::string, shared_ptr<hostgroup_struct> >
                       hostgroups;
  umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >
                       services;
  umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >
                       servicedependencies;
  umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> >
                       serviceescalations;
  umap<std::string, shared_ptr<servicegroup_struct> >
                       servicegroups;
  umap<std::string, shared_ptr<timeperiod_struct> >
                       timeperiods;
};

class                   checkmodify {
public:
                        checkmodify(std::string const& cfg_path)
    : _cfg_path(cfg_path), _current_state(NULL) {}
                        ~checkmodify() throw () { delete _current_state; }
  void                  load_configuration() {
    ::config = NULL;
    configuration::applier::state::unload();
    configuration::applier::state::load();
    ::config->cfg_main(_cfg_path);
    reload_configuration();
  }
  static void           reload_configuration() {
    configuration::reload reload_configuration;
    reload_configuration.exec();
    while (true) {
      concurrency::thread::yield();
      if (reload_configuration.is_finished())
        break;
      reload_configuration.try_lock();
    }
    // configuration::state config;
    // {
    //   configuration::parser p;
    //   std::string path(::config->cfg_main());
    //   p.parse(path, config);
    // }
    // configuration::applier::state::instance().apply(config);
  }
  void                  save_current_configuration() {
    configuration::applier::state&
      app_state(configuration::applier::state::instance());
    _current_state = ::config;
    _obj_state.commands = app_state.commands();
    _obj_state.connectors = app_state.connectors();
    _obj_state.contacts = app_state.contacts();
    _obj_state.contactgroups = app_state.contactgroups();
    _obj_state.hosts = app_state.hosts();
    _obj_state.hostdependencies = app_state.hostdependencies();
    _obj_state.hostescalations = app_state.hostescalations();
    _obj_state.hostgroups = app_state.hostgroups();
    _obj_state.services = app_state.services();
    _obj_state.servicedependencies = app_state.servicedependencies();
    _obj_state.serviceescalations = app_state.serviceescalations();
    _obj_state.servicegroups = app_state.servicegroups();
    _obj_state.timeperiods = app_state.timeperiods();
  }
  void                  verify() {
    configuration::applier::state&
      app_state(configuration::applier::state::instance());
    if (*_current_state != *::config)
      throw (engine_error() << "check modify configuration failed: "
             "state are not equal");
    if (!compare_with_true_contents(_obj_state.commands, app_state.commands()))
      throw (engine_error() << "check modify configuration failed: "
             "commands are not equal");
    if (!compare_with_true_contents(_obj_state.connectors, app_state.connectors()))
      throw (engine_error() << "check modify configuration failed: "
             "connectors are not equal");
    if (!compare_with_true_contents(_obj_state.contacts, app_state.contacts()))
      throw (engine_error() << "check modify configuration failed: "
             "contacts are not equal");
    if (!compare_with_true_contents(_obj_state.contactgroups, app_state.contactgroups()))
      throw (engine_error() << "check modify configuration failed: "
             "contactgroups are not equal");
    if (!compare_with_true_contents(_obj_state.hosts, app_state.hosts()))
      throw (engine_error() << "check modify configuration failed: "
             "hosts are not equal");
    if (!compare_with_true_contents(_obj_state.hostdependencies, app_state.hostdependencies()))
      throw (engine_error() << "check modify configuration failed: "
             "hostdependencies are not equal");
    if (!compare_with_true_contents(_obj_state.hostescalations, app_state.hostescalations()))
      throw (engine_error() << "check modify configuration failed: "
             "hostescalations are not equal");
    if (!compare_with_true_contents(_obj_state.hostgroups, app_state.hostgroups()))
      throw (engine_error() << "check modify configuration failed: "
             "hostgroups are not equal");
    if (!compare_with_true_contents(_obj_state.services, app_state.services()))
      throw (engine_error() << "check modify configuration failed: "
             "services are not equal");
    if (!compare_with_true_contents(_obj_state.servicedependencies, app_state.servicedependencies()))
      throw (engine_error() << "check modify configuration failed: "
             "servicedependencies are not equal");
    if (!compare_with_true_contents(_obj_state.serviceescalations, app_state.serviceescalations()))
      throw (engine_error() << "check modify configuration failed: "
             "serviceescalations are not equal");
    if (!compare_with_true_contents(_obj_state.servicegroups, app_state.servicegroups()))
      throw (engine_error() << "check modify configuration failed: "
             "servicegroups are not equal");
    if (!compare_with_true_contents(_obj_state.timeperiods, app_state.timeperiods()))
      throw (engine_error() << "check modify configuration failed: "
             "timeperiods are not equal");
  }

private:
  std::string           _cfg_path;
  configuration::state* _current_state;
  obj_state             _obj_state;
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
      throw (engine_error() << "bad arguments into callback_event_loop_end");

    nebstruct_process_data&
      data(*static_cast<nebstruct_process_data*>(neb_data));
    if (data.type == NEBTYPE_PROCESS_EVENTLOOPSTART) {
      ::config->cfg_main(gl_config_path);
      sigshutdown = true;
    }
    else if (data.type == NEBTYPE_PROCESS_EVENTLOOPEND) {
      checkmodify::reload_configuration();

      checkmodify chkm(gl_config_path);
      chkm.save_current_configuration();
      chkm.load_configuration();
      chkm.verify();
    }
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, most)
      << "error: " << e.what();
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
      throw (engine_error() << "can not load module checkmodify: "
             "bad argument");

    gl_config_path = args;

    // Set module informations.
    neb_set_module_info(
      handle,
      NEBMODULE_MODINFO_TITLE,
      "Check modify configuration module");
    neb_set_module_info(
      handle,
      NEBMODULE_MODINFO_AUTHOR,
      "Merethis");
    neb_set_module_info(
      handle,
      NEBMODULE_MODINFO_COPYRIGHT,
      "Copyright 2013 Merethis");
    neb_set_module_info(
      handle,
      NEBMODULE_MODINFO_VERSION,
      "1.0.0");
    neb_set_module_info(
      handle,
      NEBMODULE_MODINFO_LICENSE,
      "GPL version 2");
    neb_set_module_info(
      handle,
      NEBMODULE_MODINFO_DESC,
      "Check modify configuration module.");

    // Register callbacks event loop.
    if (neb_register_callback(
          NEBCALLBACK_PROCESS_DATA,
          handle,
          0,
          callback_event_loop))
      throw (engine_error() << "can not load module checkmodify: "
             "register callback failed");
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, most)
      << "error: " << e.what();
    exit(EXIT_FAILURE);
  }
  return (0);
}

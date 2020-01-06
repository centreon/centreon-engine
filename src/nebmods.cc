/*
** Copyright 2002-2008           Ethan Galstad
** Copyright 2011-2013,2016-2017 Centreon
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

#include "com/centreon/engine/nebmods.hh"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include "com/centreon/engine/broker/handle.hh"
#include "com/centreon/engine/broker/loader.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/****************************************************************************/
/****************************************************************************/
/* INITIALIZATION/CLEANUP FUNCTIONS                                         */
/****************************************************************************/
/****************************************************************************/

/* initialize module routines */
int neb_init_modules() {
  return OK;
}

/* deinitialize module routines */
int neb_deinit_modules() {
  return OK;
}

/* add a new module to module list */
int neb_add_module(char const* filename,
                   char const* args,
                   int should_be_loaded) {
  (void)should_be_loaded;

  if (filename == NULL)
    return ERROR;

  try {
    broker::loader::instance().add_module(filename, args);
    logger(dbg_eventbroker, basic)
        << "Added module: name='" << filename << "', args='" << args << "'";
  } catch (...) {
    logger(dbg_eventbroker, basic) << "Counld not add module: name='"
                                   << filename << "', args='" << args << "'";
    return ERROR;
  }
  return OK;
}

/* free memory allocated to module list */
int neb_free_module_list() {
  try {
    logger(dbg_eventbroker, basic) << "unload all modules success.";
  } catch (...) {
    logger(dbg_eventbroker, basic) << "unload all modules failed.";
    return ERROR;
  }
  return OK;
}

/****************************************************************************/
/****************************************************************************/
/* LOAD/UNLOAD FUNCTIONS                                                    */
/****************************************************************************/
/****************************************************************************/

/* load all modules */
int neb_load_all_modules() {
  int unloaded(0);
  try {
    broker::loader& loader(broker::loader::instance());

    std::string const& mod_dir(config->broker_module_directory());
    if (!mod_dir.empty())
      loader.load_directory(mod_dir);

    std::list<std::shared_ptr<broker::handle> > modules(loader.get_modules());
    for (std::list<std::shared_ptr<broker::handle> >::const_iterator
             it(modules.begin()),
         end(modules.end());
         it != end; ++it)
      if (neb_load_module(&(*(*it))))
        ++unloaded;
  } catch (...) {
    logger(dbg_eventbroker, basic) << "Could not load all modules";
    return -1;
  }
  return unloaded;
}

/* load a particular module */
int neb_load_module(void* mod) {
  if (mod == NULL)
    return ERROR;

  broker::handle* module = static_cast<broker::handle*>(mod);

  /* don't reopen the module */
  if (module->is_loaded() == true)
    return OK;

  try {
    module->open();
    logger(log_info_message, basic)
        << "Event broker module '" << module->get_filename()
        << "' initialized successfully";
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
        << "Error: Could not load module '" << module->get_filename()
        << "': " << e.what();
    return ERROR;
  } catch (...) {
    logger(log_runtime_error, basic)
        << "Error: Could not load module '" << module->get_filename() << "'";
    return ERROR;
  }
  return OK;
}

/**
 *  Reload all modules that are currently loaded.
 */
int neb_reload_all_modules() {
  int retval;
  try {
    broker::loader* loader(&broker::loader::instance());
    if (loader) {
      std::list<std::shared_ptr<broker::handle> > modules(
          loader->get_modules());
      for (std::list<std::shared_ptr<broker::handle> >::const_iterator
               it(modules.begin()),
           end(modules.end());
           it != end; ++it) {
        neb_reload_module(&**it);
      }
      logger(dbg_eventbroker, basic) << "All modules got successfully reloaded";
    }
    retval = OK;
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
        << "Warning: Module reloading failed: " << e.what();
    retval = ERROR;
  } catch (...) {
    logger(log_runtime_error, basic)
        << "Warning: Module reloading failed: unknown error";
    retval = ERROR;
  }
  return retval;
}

/* Reload a particular module. */
int neb_reload_module(void* mod) {
  if (mod == NULL)
    return ERROR;

  broker::handle* module(static_cast<broker::handle*>(mod));
  if (module->is_loaded() == false)
    return OK;

  logger(dbg_eventbroker, basic)
      << "Attempting to reload module '" << module->get_filename() << "'";
  module->reload();
  logger(dbg_eventbroker, basic)
      << "Module '" << module->get_filename() << "' reloaded successfully";

  return OK;
}

/**
 *  Close (unload) all modules that are currently loaded.
 *
 *  @param[in] flags  Unload flags.
 *  @param[in] reason Unload reason (reload, shutdown, ...).
 *
 *  @return OK on success.
 */
int neb_unload_all_modules(int flags, int reason) {
  int retval;
  try {
    broker::loader* loader(&broker::loader::instance());
    if (loader) {
      std::list<std::shared_ptr<broker::handle> > modules(
          loader->get_modules());
      for (std::list<std::shared_ptr<broker::handle> >::const_iterator
               it(modules.begin()),
           end(modules.end());
           it != end; ++it)
        neb_unload_module(&**it, flags, reason);
      loader->unload_modules();
      logger(dbg_eventbroker, basic) << "All modules got successfully unloaded";
    }
    retval = OK;
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic)
        << "Error: Module unloading failed: " << e.what();
    retval = ERROR;
  } catch (...) {
    logger(log_runtime_error, basic)
        << "Error: unloading of all modules failed";
    retval = ERROR;
  }
  return retval;
}

/* close (unload) a particular module */
int neb_unload_module(void* mod, int flags, int reason) {
  (void)flags;
  (void)reason;

  if (mod == NULL)
    return ERROR;

  broker::handle* module = static_cast<broker::handle*>(mod);

  if (module->is_loaded() == false)
    return OK;

  logger(dbg_eventbroker, basic)
      << "Attempting to unload module '" << module->get_filename() << "'";

  module->close();

  /* deregister all of the module's callbacks */
  neb_deregister_module_callbacks(module);

  logger(dbg_eventbroker, basic)
      << "Module '" << module->get_filename() << "' unloaded successfully";

  logger(log_info_message, basic)
      << "Event broker module '" << module->get_filename()
      << "' deinitialized successfully";

  return OK;
}

/****************************************************************************/
/****************************************************************************/
/* INFO FUNCTIONS                                                           */
/****************************************************************************/
/****************************************************************************/

/* sets module information */
int neb_set_module_info(void* handle, int type, char const* data) {
  if (handle == NULL)
    return NEBERROR_NOMODULE;

  /* check type */
  if (type < 0 || type >= NEBMODULE_MODINFO_NUMITEMS)
    return NEBERROR_MODINFOBOUNDS;

  try {
    broker::handle* module = static_cast<broker::handle*>(handle);

    switch (type) {
      case NEBMODULE_MODINFO_TITLE:
        module->set_name(data);
        break;

      case NEBMODULE_MODINFO_AUTHOR:
        module->set_author(data);
        break;

      case NEBMODULE_MODINFO_COPYRIGHT:
        module->set_copyright(data);
        break;

      case NEBMODULE_MODINFO_VERSION:
        module->set_version(data);
        break;

      case NEBMODULE_MODINFO_LICENSE:
        module->set_license(data);
        break;

      case NEBMODULE_MODINFO_DESC:
        module->set_description(data);
        break;
    }

    logger(dbg_eventbroker, basic)
        << "set module info success: filename='" << module->get_filename()
        << "', type='" << type << "'";
  } catch (...) {
    logger(dbg_eventbroker, basic) << "Counld not set module info.";
    return ERROR;
  }

  return OK;
}

/****************************************************************************/
/****************************************************************************/
/* CALLBACK FUNCTIONS                                                       */
/****************************************************************************/
/****************************************************************************/

/* allows a module to register a callback function */
int neb_register_callback(int callback_type,
                          void* mod_handle,
                          int priority,
                          int (*callback_func)(int, void*)) {
  if (callback_func == NULL)
    return NEBERROR_NOCALLBACKFUNC;

  if (mod_handle == NULL)
    return NEBERROR_NOMODULEHANDLE;

  /* make sure the callback type is within bounds */
  if (callback_type < 0 || callback_type >= NEBCALLBACK_NUMITEMS) {
    return NEBERROR_CALLBACKBOUNDS;
  }

  union {
    int (*func)(int, void*);
    void* data;
  } callback;
  callback.func = callback_func;

  /* allocate memory */
  nebcallback* new_callback(new nebcallback);

  new_callback->priority = priority;
  new_callback->module_handle = (void*)mod_handle;
  new_callback->callback_func = callback.data;

  /* add new function to callback list, sorted by priority (first come, first
   * served for same priority) */
  new_callback->next = NULL;
  if (neb_callback_list[callback_type] == NULL)
    neb_callback_list[callback_type] = new_callback;
  else {
    nebcallback* last_callback(NULL);
    nebcallback* temp_callback(NULL);
    for (temp_callback = neb_callback_list[callback_type];
         temp_callback != NULL; temp_callback = temp_callback->next) {
      if (temp_callback->priority > new_callback->priority)
        break;
      last_callback = temp_callback;
    }
    if (last_callback == NULL)
      neb_callback_list[callback_type] = new_callback;
    else {
      if (temp_callback == NULL)
        last_callback->next = new_callback;
      else {
        new_callback->next = temp_callback;
        last_callback->next = new_callback;
      }
    }
  }
  return OK;
}

/* dregisters all callback functions for a given module */
int neb_deregister_module_callbacks(void* mod) {
  nebcallback* temp_callback;
  nebcallback* next_callback{NULL};
  int callback_type;

  if (!mod)
    return NEBERROR_NOMODULE;

  for (callback_type = 0; callback_type < NEBCALLBACK_NUMITEMS;
       callback_type++) {
    for (temp_callback = neb_callback_list[callback_type];
         temp_callback != NULL; temp_callback = next_callback) {
      next_callback = temp_callback->next;
      if ((void*)temp_callback->module_handle == (void*)mod) {
        union {
          int (*func)(int, void*);
          void* data;
        } callback;
        callback.data = temp_callback->callback_func;
        neb_deregister_callback(callback_type, callback.func);
      }
    }
  }
  return OK;
}

/* allows a module to deregister a callback function */
int neb_deregister_callback(int callback_type,
                            int (*callback_func)(int, void*)) {
  nebcallback* temp_callback = NULL;
  nebcallback* last_callback = NULL;
  nebcallback* next_callback = NULL;

  if (!callback_func)
    return NEBERROR_NOCALLBACKFUNC;

  /* find the callback to remove */
  for (temp_callback = last_callback = neb_callback_list[callback_type];
       temp_callback != NULL; temp_callback = next_callback) {
    next_callback = temp_callback->next;

    /* we found it */
    union {
      void* data;
      int (*code)(int, void*);
    } temp_callback_func;
    temp_callback_func.data = temp_callback->callback_func;
    if (temp_callback_func.code == callback_func)
      break;

    last_callback = temp_callback;
  }

  /* we couldn't find the callback */
  if (temp_callback == NULL)
    return NEBERROR_CALLBACKNOTFOUND;

  else {
    /* only one item in the list */
    if (temp_callback != last_callback->next)
      neb_callback_list[callback_type] = NULL;
    else
      last_callback->next = next_callback;
    delete temp_callback;
  }

  return OK;
}

/* make callbacks to modules */
int neb_make_callbacks(int callback_type, void* data) {
  nebcallback* temp_callback;
  nebcallback* next_callback;
  int cbresult = 0;
  int total_callbacks = 0;

  /* make sure the callback type is within bounds */
  if (callback_type < 0 || callback_type >= NEBCALLBACK_NUMITEMS)
    return ERROR;

  logger(dbg_eventbroker, more)
      << "Making callbacks (type " << callback_type << ")...";

  /* make the callbacks... */
  for (temp_callback = neb_callback_list[callback_type]; temp_callback != NULL;
       temp_callback = next_callback) {
    next_callback = temp_callback->next;

    union {
      int (*func)(int, void*);
      void* data;
    } neb;
    neb.data = temp_callback->callback_func;
    cbresult = (*neb.func)(callback_type, data);

    temp_callback = next_callback;

    total_callbacks++;
    logger(dbg_eventbroker, most)
        << "Callback #" << total_callbacks << " (type " << callback_type
        << ") return (code = " << cbresult << ")";

    /* module wants to cancel callbacks to other modules (and potentially cancel
     * the default handling of an event) */
    if (cbresult == NEBERROR_CALLBACKCANCEL)
      break;

    /* module wants to override default handling of an event */
    /* not sure if we should bail out here just because one module wants to
     * override things - what about other modules? EG 12/11/2006 */
    else if (cbresult == NEBERROR_CALLBACKOVERRIDE)
      break;
  }
  return cbresult;
}

/* initialize callback list */
int neb_init_callback_list() {
  /* initialize list pointers */
  for (int x = 0; x < NEBCALLBACK_NUMITEMS; x++)
    neb_callback_list[x] = nullptr;
  return OK;
}

/* free memory allocated to callback list */
int neb_free_callback_list() {
  nebcallback* temp_callback = NULL;
  nebcallback* next_callback = NULL;

  for (int x = 0; x < NEBCALLBACK_NUMITEMS; x++) {
    for (temp_callback = neb_callback_list[x]; temp_callback != nullptr;
         temp_callback = next_callback) {
      next_callback = temp_callback->next;
      delete temp_callback;
    }

    neb_callback_list[x] = nullptr;
  }

  return OK;
}

/*
** Copyright 2002-2008 Ethan Galstad
** Copyright 2011      Merethis
**
** This file is part of Centreon Scheduler.
**
** Centreon Scheduler is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Scheduler is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Scheduler. If not, see
** <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "neberrors.hh"
#include "utils.hh"
#include "logging.hh"
#include "configuration.hh"
#include "modules/loader.hh"
#include "modules/handle.hh"
#include "nebmods.hh"

#ifdef USE_EVENT_BROKER

using namespace com::centreon::scheduler;

extern configuration config;

nebcallback*         neb_callback_list[NEBCALLBACK_NUMITEMS];

/*#define DEBUG*/

/****************************************************************************/
/****************************************************************************/
/* INITIALIZATION/CLEANUP FUNCTIONS                                         */
/****************************************************************************/
/****************************************************************************/

/* initialize module routines */
int neb_init_modules(void) {
  return (OK);
}

/* deinitialize module routines */
int neb_deinit_modules(void) {
  return (OK);
}

/* add a new module to module list */
int neb_add_module(char const* filename, char const* args, int should_be_loaded) {
  (void)should_be_loaded;

  if (filename == NULL)
    return (ERROR);

  try {
    modules::loader& loader = modules::loader::instance();
    modules::handle module(filename, args);
    loader.add_module(module);
    log_debug_info(DEBUGL_EVENTBROKER, 0,
		   "Added module: name=`%s', args=`%s'\n",
		   filename,
		   args);
  }
  catch (...) {
    log_debug_info(DEBUGL_EVENTBROKER, 0,
		   "Counld not add module: name=`%s', args=`%s'\n",
		   filename,
		   args);
    return (ERROR);
  }
  return (OK);
}

/* free memory allocated to module list */
int neb_free_module_list(void) {
  try {
    modules::loader::instance().unload();
    log_debug_info(DEBUGL_EVENTBROKER, 0, "unload all modules success.\n");
  }
  catch (...) {
    log_debug_info(DEBUGL_EVENTBROKER, 0, "unload all modules failed.\n");
    return (ERROR);
  }
  return (OK);
}

/****************************************************************************/
/****************************************************************************/
/* LOAD/UNLOAD FUNCTIONS                                                    */
/****************************************************************************/
/****************************************************************************/

/* load all modules */
int neb_load_all_modules(void) {
  try {
    modules::loader& loader = modules::loader::instance();
    QMultiHash<QString, modules::handle>& modules = loader.get_modules();

    for (QMultiHash<QString, modules::handle>::iterator it = modules.begin(), end = modules.end();
	 it != end;
	 ++it) {
      neb_load_module(&it.value());
    }
  }
  catch (...) {
    log_debug_info(DEBUGL_EVENTBROKER, 0, "Counld not load all modules\n");
    return (ERROR);
  }
  return (OK);
}

/* load a particular module */
int neb_load_module(void* mod) {
  if (mod == NULL)
    return (ERROR);

  modules::handle* module = static_cast<modules::handle*>(mod);

  /* don't reopen the module */
  if (module->is_loaded() == true)
    return (OK);

  try {
    module->open();
    logit(NSLOG_INFO_MESSAGE, false,
	  "Event broker module `%s' initialized syccessfully.\n",
	  module->get_filename().toStdString().c_str());
  }
  catch (error const& e) {
    logit(NSLOG_RUNTIME_ERROR, false,
	  "Error: Could not load module `%s' -> %s\n",
	  module->get_filename().toStdString().c_str(),
	  e.what());
    return (ERROR);
  }
  catch (...) {
    logit(NSLOG_RUNTIME_ERROR, false,
	  "Error: Could not load module `%s'\n",
	  module->get_filename().toStdString().c_str());
    return (ERROR);
  }
  return (OK);
}

/* close (unload) all modules that are currently loaded */
int neb_unload_all_modules(int flags, int reason) {
  try {
    modules::loader& loader = modules::loader::instance();
    QMultiHash<QString, modules::handle>& modules = loader.get_modules();

    for (QMultiHash<QString, modules::handle>::iterator it = modules.begin(), end = modules.end();
	 it != end;
	 ++it) {
      neb_unload_module(&it.value(), flags, reason);
    }
    log_debug_info(DEBUGL_EVENTBROKER, 0, "load all modules success.\n");
  }
  catch (...) {
    log_debug_info(DEBUGL_EVENTBROKER, 0, "load all modules failed.\n");
    return (ERROR);
  }
  return (OK);
}

/* close (unload) a particular module */
int neb_unload_module(void* mod, int flags, int reason) {
  (void)flags;
  (void)reason;

  if (mod == NULL)
    return (ERROR);

  modules::handle* module = static_cast<modules::handle*>(mod);

  if (module->is_loaded() == false)
    return (OK);

  log_debug_info(DEBUGL_EVENTBROKER, 0,
                 "Attempting to unload module `%s'\n",
                 module->get_filename().toStdString().c_str());

  module->close();

  /* deregister all of the module's callbacks */
  neb_deregister_module_callbacks(module);

  log_debug_info(DEBUGL_EVENTBROKER, 0,
                 "Module '%s' unloaded successfully.\n",
		 module->get_filename().toStdString().c_str());

  logit(NSLOG_INFO_MESSAGE, false,
        "Event broker module '%s' deinitialized successfully.\n",
        module->get_filename().toStdString().c_str());

  return (OK);
}

/****************************************************************************/
/****************************************************************************/
/* INFO FUNCTIONS                                                           */
/****************************************************************************/
/****************************************************************************/

/* sets module information */
int neb_set_module_info(void* handle, int type, char* data) {
  if (handle == NULL)
    return (NEBERROR_NOMODULE);

  /* check type */
  if (type < 0 || type >= NEBMODULE_MODINFO_NUMITEMS)
    return (NEBERROR_MODINFOBOUNDS);

  try {
    modules::handle* module = static_cast<modules::handle*>(handle);
    modules::loader& loader = modules::loader::instance();
    QMultiHash<QString, modules::handle> const& modules = loader.get_modules();

    /* find the module */
    if (modules.find(module->get_filename(), *module) == modules.end()) {
      log_debug_info(DEBUGL_EVENTBROKER, 0,
		     "set module info failed: filename=%s, type=`%d'\n",
		     module->get_filename().toStdString().c_str(),
		     type);
      return (NEBERROR_BADMODULEHANDLE);
    }

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

    log_debug_info(DEBUGL_EVENTBROKER, 0,
		   "set module info success: filename=%s, type=`%d'\n",
		   module->get_filename().toStdString().c_str(),
		   type);
  }
  catch (...) {
    log_debug_info(DEBUGL_EVENTBROKER, 0, "Counld not set module info.\n");
    return (ERROR);
  }

  return (OK);
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
			  int (*callback_func) (int, void*)) {
  nebcallback* new_callback = NULL;
  nebcallback* temp_callback = NULL;
  nebcallback* last_callback = NULL;

  if (callback_func == NULL)
    return (NEBERROR_NOCALLBACKFUNC);

  if (mod_handle == NULL)
    return (NEBERROR_NOMODULEHANDLE);

  /* make sure the callback type is within bounds */
  if (callback_type < 0 || callback_type >= NEBCALLBACK_NUMITEMS)
    return (NEBERROR_CALLBACKBOUNDS);

  try {
    modules::handle* module = static_cast<modules::handle*>(mod_handle);
    modules::loader& loader = modules::loader::instance();
    QMultiHash<QString, modules::handle> const& modules = loader.get_modules();

    /* make sure module handle is valid */
    if (modules.find(module->get_filename(), *module) == modules.end())
      return (NEBERROR_BADMODULEHANDLE);
  }
  catch (...) {
    return (NEBERROR_BADMODULEHANDLE);
  }

  /* allocate memory */
  new_callback = new nebcallback;

  new_callback->priority = priority;
  new_callback->module_handle = (void*)mod_handle;
  new_callback->callback_func = *(void**)(&callback_func);

  /* add new function to callback list, sorted by priority (first come, first served for same priority) */
  new_callback->next = NULL;
  if (neb_callback_list[callback_type] == NULL)
    neb_callback_list[callback_type] = new_callback;
  else {
    last_callback = NULL;
    for (temp_callback = neb_callback_list[callback_type];
         temp_callback != NULL;
	 temp_callback = temp_callback->next) {
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

  return (OK);
}

/* dregisters all callback functions for a given module */
int neb_deregister_module_callbacks(void* mod) {
  nebcallback* temp_callback = NULL;
  nebcallback* next_callback = NULL;
  int callback_type = 0;

  if (mod == NULL)
    return (NEBERROR_NOMODULE);

  for (callback_type = 0; callback_type < NEBCALLBACK_NUMITEMS; callback_type++) {
    for (temp_callback = neb_callback_list[callback_type];
	 temp_callback != NULL;
	 temp_callback = next_callback) {
      next_callback = temp_callback->next;
      if ((void*)temp_callback->module_handle == (void*)mod)
        neb_deregister_callback(callback_type,
				(int (*)(int, void*))temp_callback->callback_func);
    }
  }
  return (OK);
}

/* allows a module to deregister a callback function */
int neb_deregister_callback(int callback_type, int (*callback_func)(int, void*)) {
  nebcallback* temp_callback = NULL;
  nebcallback* last_callback = NULL;
  nebcallback* next_callback = NULL;

  if (callback_func == NULL)
    return (NEBERROR_NOCALLBACKFUNC);

  /* make sure the callback type is within bounds */
  if (callback_type < 0 || callback_type >= NEBCALLBACK_NUMITEMS)
    return (NEBERROR_CALLBACKBOUNDS);

  /* find the callback to remove */
  for (temp_callback = last_callback = neb_callback_list[callback_type];
       temp_callback != NULL;
       temp_callback = next_callback) {
    next_callback = temp_callback->next;

    /* we found it */
    if (temp_callback->callback_func == *(void**)(&callback_func))
      break;

    last_callback = temp_callback;
  }

  /* we couldn't find the callback */
  if (temp_callback == NULL)
    return (NEBERROR_CALLBACKNOTFOUND);

  else {
    /* only one item in the list */
    if (temp_callback != last_callback->next)
      neb_callback_list[callback_type] = NULL;
    else
      last_callback->next = next_callback;
    delete temp_callback;
  }

  return (OK);
}

/* make callbacks to modules */
int neb_make_callbacks(int callback_type, void* data) {
  nebcallback* temp_callback;
  nebcallback* next_callback;
  int (*callbackfunc) (int, void*);
  int cbresult = 0;
  int total_callbacks = 0;

  /* make sure the callback type is within bounds */
  if (callback_type < 0 || callback_type >= NEBCALLBACK_NUMITEMS)
    return (ERROR);

  log_debug_info(DEBUGL_EVENTBROKER, 1, "Making callbacks (type %d)...\n", callback_type);

  /* make the callbacks... */
  for (temp_callback = neb_callback_list[callback_type];
       temp_callback != NULL;
       temp_callback = next_callback) {
    next_callback = temp_callback->next;
    *(void**)(&callbackfunc) = temp_callback->callback_func;
    cbresult = callbackfunc(callback_type, data);
    temp_callback = next_callback;

    total_callbacks++;
    log_debug_info(DEBUGL_EVENTBROKER, 2,
                   "Callback #%d (type %d) return (code = %d\n",
                   total_callbacks,
		   callback_type,
		   cbresult);

    /* module wants to cancel callbacks to other modules (and potentially cancel the default Nagios handling of an event) */
    if (cbresult == NEBERROR_CALLBACKCANCEL)
      break;

    /* module wants to override default Nagios handling of an event */
    /* not sure if we should bail out here just because one module wants to override things - what about other modules? EG 12/11/2006 */
    else if (cbresult == NEBERROR_CALLBACKOVERRIDE)
      break;
  } return (cbresult);
}

/* initialize callback list */
int neb_init_callback_list(void) {
  int x = 0;

  /* initialize list pointers */
  for (x = 0; x < NEBCALLBACK_NUMITEMS; x++)
    neb_callback_list[x] = NULL;

  return (OK);
}

/* free memory allocated to callback list */
int neb_free_callback_list(void) {
  nebcallback* temp_callback = NULL;
  nebcallback* next_callback = NULL;
  int x = 0;

  for (x = 0; x < NEBCALLBACK_NUMITEMS; x++) {
    for (temp_callback = neb_callback_list[x];
	 temp_callback != NULL;
         temp_callback = next_callback) {
      next_callback = temp_callback->next;
      delete temp_callback;
    }

    neb_callback_list[x] = NULL;
  }

  return (OK);
}

#endif

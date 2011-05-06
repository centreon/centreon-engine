/*
** Copyright 2011      Merethis
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

#include <string.h>
#include "shared.hh"
#include "broker/compatibility.hh"

using namespace com::centreon::engine::broker;

nebmodule* neb_module_list = NULL;

/**************************************
 *                                     *
 *           Public Methods            *
 *                                     *
 **************************************/

/**
 *  Get instance of compatibility singleton.
 */
compatibility& compatibility::instance() {
  static compatibility instance;
  return (instance);
}

/**
 *  Slot for notify when module was create.
 *
 *  @param module The module object.
 */
void compatibility::create_module(broker::handle* module) {
  if (module == NULL)
    return;

  // allocate memory.
  nebmodule* new_module = new nebmodule;

  // initialize vars.
  new_module->filename = my_strdup(module->get_filename().toStdString().c_str());
  new_module->args = my_strdup(module->get_args().toStdString().c_str());
  new_module->should_be_loaded = true;
  new_module->is_currently_loaded = module->is_loaded();

  new_module->info[NEBMODULE_MODINFO_TITLE] = my_strdup(module->get_name().toStdString().c_str());
  new_module->info[NEBMODULE_MODINFO_AUTHOR] = my_strdup(module->get_author().toStdString().c_str());
  new_module->info[NEBMODULE_MODINFO_COPYRIGHT] = my_strdup(module->get_copyright().toStdString().c_str());
  new_module->info[NEBMODULE_MODINFO_VERSION] = my_strdup(module->get_version().toStdString().c_str());
  new_module->info[NEBMODULE_MODINFO_LICENSE] = my_strdup(module->get_license().toStdString().c_str());
  new_module->info[NEBMODULE_MODINFO_DESC] = my_strdup(module->get_description().toStdString().c_str());

  new_module->module_handle = module;
  new_module->init_func=NULL;
  new_module->deinit_func=NULL;
#ifdef HAVE_PTHREAD_H
  new_module->thread_id = (pthread_t)NULL;
#endif // !HAVE_PTHREAD_H

  // add module to head of list
  new_module->next = neb_module_list;
  neb_module_list = new_module;
}

/**
 *  Slot for notify when module was destroy.
 *
 *  @param module The module object.
 */
void compatibility::destroy_module(broker::handle* module) {
  if (module == NULL)
    return;

  nebmodule* tmp = neb_module_list;
  nebmodule* last = NULL;
  while (tmp != NULL) {
    if (tmp->module_handle == module) {
      nebmodule* next = tmp->next;
      delete[] tmp->filename;
      delete[] tmp->args;
      for (unsigned int x = 0; x < NEBMODULE_MODINFO_NUMITEMS; ++x) {
	delete[] tmp->info[x];
      }
      delete tmp;
      tmp = next;
      if (last != NULL)
	last->next = tmp;
      else
	neb_module_list = tmp;
    }
    else {
      last = tmp;
      tmp = tmp->next;
    }
  }
}

/**
 *  Slot for notify when module name changed.
 *
 *  @param module The module object.
 */
void compatibility::name_module(broker::handle* module) {
  if (module == NULL) {
    return;
  }

  for (nebmodule* tmp = neb_module_list; tmp != NULL; tmp = tmp->next) {
    if (tmp->module_handle == module) {
      delete[] tmp->info[NEBMODULE_MODINFO_TITLE];
      tmp->info[NEBMODULE_MODINFO_TITLE] = my_strdup(module->get_name().toStdString().c_str());
      return;
    }
  }
}

/**
 *  Slot for notify when module author changed.
 *
 *  @param module The module object.
 */
void compatibility::author_module(broker::handle* module) {
  if (module == NULL)
    return;

  for (nebmodule* tmp = neb_module_list; tmp != NULL; tmp = tmp->next) {
    if (tmp->module_handle == module) {
      delete[] tmp->info[NEBMODULE_MODINFO_AUTHOR];
      tmp->info[NEBMODULE_MODINFO_AUTHOR] = my_strdup(module->get_author().toStdString().c_str());
      return;
    }
  }
}

/**
 *  Slot for notify when module copyright changed.
 *
 *  @param module The module object.
 */
void compatibility::copyright_module(broker::handle* module) {
  if (module == NULL)
    return;

  for (nebmodule* tmp = neb_module_list; tmp != NULL; tmp = tmp->next) {
    if (tmp->module_handle == module) {
      delete[] tmp->info[NEBMODULE_MODINFO_COPYRIGHT];
      tmp->info[NEBMODULE_MODINFO_COPYRIGHT] = my_strdup(module->get_copyright().toStdString().c_str());
      return;
    }
  }
}

/**
 *  Slot for notify when module version changed.
 *
 *  @param module The module object.
 */
void compatibility::version_module(broker::handle* module) {
  if (module == NULL)
    return;

  for (nebmodule* tmp = neb_module_list; tmp != NULL; tmp = tmp->next) {
    if (tmp->module_handle == module) {
      delete[] tmp->info[NEBMODULE_MODINFO_VERSION];
      tmp->info[NEBMODULE_MODINFO_VERSION] = my_strdup(module->get_version().toStdString().c_str());
      return;
    }
  }
}

/**
 *  Slot for notify when module license changed.
 *
 *  @param module The module object.
 */
void compatibility::license_module(broker::handle* module) {
  if (module == NULL)
    return;

  for (nebmodule* tmp = neb_module_list; tmp != NULL; tmp = tmp->next) {
    if (tmp->module_handle == module) {
      delete[] tmp->info[NEBMODULE_MODINFO_LICENSE];
      tmp->info[NEBMODULE_MODINFO_LICENSE] = my_strdup(module->get_license().toStdString().c_str());
      return;
    }
  }
}

/**
 *  Slot for notify when module description changed.
 *
 *  @param module The module object.
 */
void compatibility::description_module(broker::handle* module) {
  if (module == NULL)
    return;

  for (nebmodule* tmp = neb_module_list; tmp != NULL; tmp = tmp->next) {
    if (tmp->module_handle == module) {
      delete[] tmp->info[NEBMODULE_MODINFO_DESC];
      tmp->info[NEBMODULE_MODINFO_DESC] = my_strdup(module->get_description().toStdString().c_str());
      return;
    }
  }
}

/**
 *  Slot for notify when module was loaded.
 *
 *  @param module The module object.
 */
void compatibility::loaded_module(broker::handle* module) {
  if (module == NULL)
    return;

  for (nebmodule* tmp = neb_module_list; tmp != NULL; tmp = tmp->next) {
    if (tmp->module_handle == module) {
      tmp->is_currently_loaded = module->is_loaded();
      return;
    }
  }
}

/**
 *  Slot for notify when module was unloaded.
 *
 *  @param module The module object.
 */
void compatibility::unloaded_module(broker::handle* module) {
  if (module == NULL)
    return;

  for (nebmodule* tmp = neb_module_list; tmp != NULL; tmp = tmp->next) {
    if (tmp->module_handle == module) {
      tmp->is_currently_loaded = module->is_loaded();
      return;
    }
  }
}

/**************************************
 *                                     *
 *           Private Methods           *
 *                                     *
 **************************************/

/**
 *  Default constructor.
 */
compatibility::compatibility() {}

/**
 *  Default destructor.
 */
compatibility::~compatibility() throw() {}

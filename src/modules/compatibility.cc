/*
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

#include <string.h>
#include "shared.hh"
#include "modules/compatibility.hh"

using namespace com::centreon::scheduler::modules;

nebmodule* neb_module_list = NULL;

compatibility& compatibility::instance() {
  static compatibility instance;
  return (instance);
}

void compatibility::create_module(modules::handle* module) {
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

void compatibility::destroy_module(modules::handle* module) {
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

void compatibility::name_module(modules::handle* module) {
  if (module == NULL)
    return;

  for (nebmodule* tmp = neb_module_list; tmp != NULL; tmp = tmp->next) {
    if (tmp->module_handle == module) {
      delete[] tmp->info[NEBMODULE_MODINFO_TITLE];
      tmp->info[NEBMODULE_MODINFO_TITLE] = my_strdup(module->get_name().toStdString().c_str());
      return;
    }
  }
}

void compatibility::author_module(modules::handle* module) {
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

void compatibility::copyright_module(modules::handle* module) {
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

void compatibility::version_module(modules::handle* module) {
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

void compatibility::license_module(modules::handle* module) {
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

void compatibility::description_module(modules::handle* module) {
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

void compatibility::loaded_module(modules::handle* module) {
  if (module == NULL)
    return;

  for (nebmodule* tmp = neb_module_list; tmp != NULL; tmp = tmp->next) {
    if (tmp->module_handle == module) {
      tmp->is_currently_loaded = module->is_loaded();
      return;
    }
  }
}

void compatibility::unloaded_module(modules::handle* module) {
  if (module == NULL)
    return;

  for (nebmodule* tmp = neb_module_list; tmp != NULL; tmp = tmp->next) {
    if (tmp->module_handle == module) {
      tmp->is_currently_loaded = module->is_loaded();
      return;
    }
  }
}

compatibility::compatibility() {}

compatibility::~compatibility() throw() {}

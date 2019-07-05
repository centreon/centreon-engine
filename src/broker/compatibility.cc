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

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <memory>
#include "com/centreon/engine/broker/compatibility.hh"
#include "com/centreon/engine/nebmodules.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::broker;

// Class instance.
static compatibility* _instance = NULL;

// NEB module list maintained for compatibility.
extern "C" {
  nebmodule* neb_module_list = NULL;
}

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Get instance of compatibility singleton.
 */
compatibility& compatibility::instance() {
  assert(_instance);
  return *_instance;
}

/**
 *  Load singleton instance.
 */
void compatibility::load() {
  if (!_instance)
    _instance = new compatibility;
}

/**
 *  Cleanup instance of compatibility singleton.
 */
void compatibility::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Slot that gets notified when module author change.
 *
 *  @param mod The module object.
 */
void compatibility::author_module(broker::handle* mod) {
  if (mod) {
    for (nebmodule* tmp = neb_module_list; tmp; tmp = tmp->next)
      if (tmp->module_handle == mod)
        string::setstr(tmp->info[NEBMODULE_MODINFO_AUTHOR], mod->get_author());
  }
  return;
}

/**
 *  Slot that gets notified when module copyright change.
 *
 *  @param mod The module object.
 */
void compatibility::copyright_module(broker::handle* mod) {
  if (mod) {
    for (nebmodule* tmp = neb_module_list; tmp; tmp = tmp->next)
      if (tmp->module_handle == mod)
        string::setstr(tmp->info[NEBMODULE_MODINFO_COPYRIGHT], mod->get_copyright());
  }
  return;
}

/**
 *  Slot that gets notified when module is created.
 *
 *  @param mod The module object.
 */
void compatibility::create_module(broker::handle* mod) {
  if (mod) {
    // Allocate memory.
    std::unique_ptr<nebmodule> new_module(new nebmodule);

    // Module parameters.
    new_module->filename = string::dup(mod->get_filename());
    new_module->args = string::dup(mod->get_args());
    new_module->deinit_func = NULL;
    new_module->init_func = NULL;
    new_module->is_currently_loaded = mod->is_loaded();
    new_module->module_handle = mod;
    new_module->should_be_loaded = true;
    new_module->thread_id = 0;

    // Module information.
    new_module->info[NEBMODULE_MODINFO_AUTHOR]
      = string::dup(mod->get_author());
    new_module->info[NEBMODULE_MODINFO_COPYRIGHT]
      = string::dup(mod->get_copyright());
    new_module->info[NEBMODULE_MODINFO_DESC]
      = string::dup(mod->get_description());
    new_module->info[NEBMODULE_MODINFO_LICENSE]
      = string::dup(mod->get_license());
    new_module->info[NEBMODULE_MODINFO_TITLE]
      = string::dup(mod->get_name());
    new_module->info[NEBMODULE_MODINFO_VERSION]
      = string::dup(mod->get_version());

    // Add module to head of list.
    new_module->next = neb_module_list;
    neb_module_list = new_module.release();
  }
  return;
}

/**
 *  Slot that gets notified when module description change.
 *
 *  @param mod The module object.
 */
void compatibility::description_module(broker::handle* mod) {
  if (mod) {
    for (nebmodule* tmp = neb_module_list; tmp; tmp = tmp->next)
      if (tmp->module_handle == mod)
        string::setstr(tmp->info[NEBMODULE_MODINFO_DESC], mod->get_description());
  }
  return;
}

/**
 *  Slot that gets notified when module is destroyed.
 *
 *  @param mod The module object.
 */
void compatibility::destroy_module(broker::handle* mod) {
  if (mod) {
    nebmodule* tmp(neb_module_list);
    nebmodule* last(NULL);
    while (tmp) {
      if (tmp->module_handle == mod) {
        nebmodule* next(tmp->next);
        delete[] tmp->filename;
        delete[] tmp->args;
        for (unsigned int x = 0; x < NEBMODULE_MODINFO_NUMITEMS; ++x)
          delete[] tmp->info[x];
        delete tmp;
        tmp = next;
        if (last)
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
  return;
}

/**
 *  Slot that gets notified when module license change.
 *
 *  @param mod The module object.
 */
void compatibility::license_module(broker::handle* mod) {
  if (mod) {
    for (nebmodule* tmp = neb_module_list; tmp; tmp = tmp->next)
      if (tmp->module_handle == mod)
        string::setstr(tmp->info[NEBMODULE_MODINFO_LICENSE], mod->get_license());
  }
  return;
}

/**
 *  Slot that gets notified when module is loaded.
 *
 *  @param mod The module object.
 */
void compatibility::loaded_module(broker::handle* mod) {
  if (mod) {
    for (nebmodule* tmp = neb_module_list; tmp; tmp = tmp->next)
      if (tmp->module_handle == mod)
        tmp->is_currently_loaded = mod->is_loaded();
  }
  return;
}

/**
 *  Slot that gets notified when module name change.
 *
 *  @param mod The module object.
 */
void compatibility::name_module(broker::handle* mod) {
  if (mod) {
    for (nebmodule* tmp = neb_module_list; tmp; tmp = tmp->next)
      if (tmp->module_handle == mod)
        string::setstr(tmp->info[NEBMODULE_MODINFO_TITLE], mod->get_name());
  }
  return;
}

/**
 *  Slot that gets notified when module is unloaded.
 *
 *  @param mod The module object.
 */
void compatibility::unloaded_module(broker::handle* mod) {
  if (mod) {
    for (nebmodule* tmp = neb_module_list; tmp; tmp = tmp->next)
      if (tmp->module_handle == mod)
        tmp->is_currently_loaded = mod->is_loaded();
  }
  return;
}

/**
 *  Slot that gets notified when module version change.
 *
 *  @param mod The module object.
 */
void compatibility::version_module(broker::handle* mod) {
  if (mod) {
    for (nebmodule* tmp = neb_module_list; tmp; tmp = tmp->next)
      if (tmp->module_handle == mod)
        string::setstr(tmp->info[NEBMODULE_MODINFO_VERSION], mod->get_version());
  }
  return;
}

/**************************************
 *                                     *
 *           Private Methods           *
 *                                     *
 **************************************/

/**
 *  Default constructor.
 */
compatibility::compatibility() {

}

/**
 *  Destructor.
 */
compatibility::~compatibility() throw () {
  try {
    while (neb_module_list)
      destroy_module(
        static_cast<broker::handle*>(neb_module_list->module_handle));
  }
  catch (...) {}
}

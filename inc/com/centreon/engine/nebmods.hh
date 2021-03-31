/*
** Copyright 2002-2006      Ethan Galstad
** Copyright 2011-2013,2016 Centreon
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

#ifndef CCE_NEBMODS_HH
#define CCE_NEBMODS_HH

#include "com/centreon/engine/broker/handle.hh"
#include "com/centreon/engine/nebcallbacks.hh"

// Module Structures
typedef struct nebcallback_struct {
  void* callback_func;
  void* module_handle;
  int priority;
  struct nebcallback_struct* next;
} nebcallback;

#ifdef __cplusplus
extern "C" {
#endif  // C++

// Module Functions
int neb_init_modules();
int neb_deinit_modules();
int neb_add_module(char const* filename,
                   char const* args,
                   int should_be_loaded);
int neb_free_module_list();
int neb_load_all_modules();
int neb_load_module(void* mod);
int neb_reload_all_modules();
int neb_reload_module(void* mod);
int neb_unload_all_modules(int flags, int reason);
int neb_unload_module(com::centreon::engine::broker::handle* mod,
                      int flags,
                      int reason);

// Callback Functions
int neb_make_callbacks(int callback_type, void* data);
int neb_init_callback_list();
int neb_free_callback_list();

#ifdef __cplusplus
}
#endif  // C++

#endif  // !CCE_NEBMODS_HH

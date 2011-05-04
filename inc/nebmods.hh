/*
** Copyright 2002-2006 Ethan Galstad
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

#ifndef CCE_NEBMODS_HH
# define CCE_NEBMODS_HH

# include "nebmodules.hh"
# include "nebcallbacks.hh"

# ifdef __cplusplus
extern "C" {
# endif

// Module Structures
typedef struct               nebcallback_struct {
  void*                      callback_func;
  void*                      module_handle;
  int                        priority;
  struct nebcallback_struct* next;
}                            nebcallback;

// Module Functions
int neb_init_modules(void);
int neb_deinit_modules(void);
int neb_add_module(char const* filename,char const* args,int should_be_loaded);
int neb_free_module_list(void);
int neb_load_all_modules(void);
int neb_load_module(void* mod);
int neb_unload_all_modules(int flags, int reason);
int neb_unload_module(void* mod, int flags, int reason);

// int neb_set_module_info(void *handle, int type, char *data);

// int neb_register_callback(int callback_type, void *mod_handle, int priority, int (*callback_func)(int,void *));
// int neb_deregister_module_callbacks(nebmodule *mod);
// int neb_deregister_callback(int callback_type, int (*callback_func)(int,void *));

// Callback Functions
int neb_make_callbacks(int callback_type, void* data);
int neb_init_callback_list(void);
int neb_free_callback_list(void);

# ifdef __cplusplus
}
# endif

#endif // !CCE_NEBMODS_HH

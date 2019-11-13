/*
** Copyright 2002-2006 Ethan Galstad
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

#ifndef CCE_NEBMODULES_HH
#define CCE_NEBMODULES_HH

#include <pthread.h>

/* Module version information. */
#define NEB_API_VERSION(x) int __neb_api_version = x;
#define CURRENT_NEB_API_VERSION 3

/* Module information. */
#define NEBMODULE_MODINFO_NUMITEMS 6
#define NEBMODULE_MODINFO_TITLE 0
#define NEBMODULE_MODINFO_AUTHOR 1
#define NEBMODULE_MODINFO_COPYRIGHT 2
#define NEBMODULE_MODINFO_VERSION 3
#define NEBMODULE_MODINFO_LICENSE 4
#define NEBMODULE_MODINFO_DESC 5

/* Module load/unload flags. */
#define NEBMODULE_NORMAL_LOAD 0
#define NEBMODULE_ENGINE (1 << 13)
#define NEBMODULE_REQUEST_UNLOAD 0
#define NEBMODULE_FORCE_UNLOAD (1 << 0)

/* Modules unload reasons. */
#define NEBMODULE_NEB_SHUTDOWN 1
#define NEBMODULE_NEB_RESTART 2
#define NEBMODULE_ERROR_NO_INIT 3
#define NEBMODULE_ERROR_BAD_INIT 4
#define NEBMODULE_ERROR_API_VERSION 5

#ifdef __cplusplus
extern "C" {
#endif /* C++ */

int neb_set_module_info(void* handle, int type, char const* data);

#ifdef __cplusplus
}
#endif /* C++ */

/**
 *  @struct nebmodule nebmodules.hh "com/centreon/engine/nebmodules.hh"
 *  @brief  NEB module structure.
 *
 *  Store information related to a NEB module.
 */
typedef struct nebmodule_struct {
  char* filename;
  char* args;
  char* info[NEBMODULE_MODINFO_NUMITEMS];
  int should_be_loaded;
  int is_currently_loaded;
  void* module_handle;
  void* init_func;
  void* deinit_func;
  pthread_t thread_id;
  struct nebmodule_struct* next;
} nebmodule;

#endif /* !CCE_NEBMODULES_HH */

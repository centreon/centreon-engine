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

#include "com/centreon/engine/broker/compatibility.hh"
#include <climits>
#include <exception>
#include "com/centreon/engine/broker/loader.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/nebmodules.hh"
#include "test/broker/mod_load.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine::broker;

static const char* MOD_LIB_COMPT_NAME = "./broker_mod_compatibility.so";

extern nebmodule* neb_module_list;

/**
 *  Check broker compatibility.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  loader& loader = loader::instance();

  loader.add_module(MOD_LIB_COMPT_NAME, MOD_LIB_COMPT_NAME)->open();

  if (loader.get_modules().size() != 1) {
    throw(engine_error() << __func__ << ": broker loader failed.");
  }

  for (nebmodule* tmp = neb_module_list; tmp != NULL; tmp = tmp->next) {
    if (tmp->module_handle != NULL) {
      if (strcmp(tmp->info[NEBMODULE_MODINFO_TITLE], MOD_TITLE)) {
        throw(engine_error() << __func__ << ": invalid name.");
      }
      if (strcmp(tmp->info[NEBMODULE_MODINFO_AUTHOR], MOD_AUTHOR)) {
        throw(engine_error() << __func__ << ": invalide author.");
      }
      if (strcmp(tmp->info[NEBMODULE_MODINFO_COPYRIGHT], MOD_COPYRIGHT)) {
        throw(engine_error() << __func__ << ": invalide copyright.");
      }
      if (strcmp(tmp->info[NEBMODULE_MODINFO_DESC], MOD_DESCRIPTION)) {
        throw(engine_error() << __func__ << ": invalide description.");
      }
      if (strcmp(tmp->info[NEBMODULE_MODINFO_VERSION], MOD_VERSION)) {
        throw(engine_error() << __func__ << ": invalide version.");
      }
      if (strcmp(tmp->info[NEBMODULE_MODINFO_LICENSE], MOD_LICENSE)) {
        throw(engine_error() << __func__ << ": invalide license.");
      }
    }
  }

  loader.unload();
  return (0);
}

/**
 *  Init the unit test.
 */
int main(int argc, char** argv) {
  com::centreon::engine::unittest utest(argc, argv, &main_test);
  return (utest.run());
}

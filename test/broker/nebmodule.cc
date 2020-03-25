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

#include <climits>
#include <exception>
#include "com/centreon/engine/broker/loader.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/nebmods.hh"
#include "test/broker/mod_load.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::broker;

static const char* MOD_LIB_NAME = "./broker_mod_load.so";
static const char* MOD_LIB_COMPAT_NAME = "./broker_mod_compatibility.so";

bool mod_test_load_quit = false;

/**
 *  Check nebmodule compatibility.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  loader& loader(loader::instance());

  if (neb_init_modules() != 0)
    throw(engine_error() << "neb_init_modules failed.");

  if (neb_add_module(MOD_LIB_NAME, MOD_LIB_NAME, true) != 0 ||
      neb_add_module(MOD_LIB_COMPAT_NAME, MOD_LIB_COMPAT_NAME, true) != 0)
    throw(engine_error() << "neb_add_module failed.");

  if (neb_load_all_modules() != 0)
    throw(engine_error() << "neb_load_all_modules failed.");

  std::list<std::shared_ptr<handle> > modules(loader.get_modules());

  if (modules.size() != 2)
    throw(engine_error() << "invalid modules size.");

  for (std::list<std::shared_ptr<handle> >::const_iterator it(modules.begin()),
       end(modules.end());
       it != end; ++it) {
    if ((*it)->get_name() != MOD_TITLE)
      throw(engine_error() << "invalid name.");

    if ((*it)->get_author() != MOD_AUTHOR)
      throw(engine_error() << "invalide author.");

    if ((*it)->get_copyright() != MOD_COPYRIGHT)
      throw(engine_error() << "invalide copyright.");

    if ((*it)->get_description() != MOD_DESCRIPTION)
      throw(engine_error() << "invalide description.");

    if ((*it)->get_name() != MOD_TITLE)
      throw(engine_error() << "invalide name.");

    if ((*it)->get_version() != MOD_VERSION)
      throw(engine_error() << "invalide version.");

    if ((*it)->get_license() != MOD_LICENSE)
      throw(engine_error() << "invalide license.");
  }

  if (neb_unload_all_modules(NEBMODULE_FORCE_UNLOAD, true) != 0)
    throw(engine_error() << "neb_unload_all_modules failed.");

  if (loader.get_modules().size() != 0)
    throw(engine_error() << "invalid modules size.");

  if (neb_free_module_list() != 0)
    throw(engine_error() << "neb_free_module_list failed.");

  if (neb_deinit_modules() != 0)
    throw(engine_error() << "neb_deinit_modules failed.");

  return (0);
}

/**
 *  Init the unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

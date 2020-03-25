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

#include "com/centreon/engine/broker/loader.hh"
#include <climits>
#include <exception>
#include <list>
#include "com/centreon/engine/exceptions/error.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine::broker;

static char const* MOD_LIB_NAME = "./broker_mod_load.so";
static char const* MOD_LIB_COMPATIBILITY_NAME = "./broker_mod_compatibility.so";

bool mod_test_load_quit = false;

/**
 *  Check loader load method.
 */
void check_load() {
  loader& loader(loader::instance());

  // Load module.
  std::shared_ptr<handle> mod1(loader.add_module(MOD_LIB_NAME));
  mod1->open();
  std::shared_ptr<handle> mod2(loader.add_module(MOD_LIB_COMPATIBILITY_NAME));
  mod2->open();

  std::list<std::shared_ptr<handle> > modules(loader.get_modules());
  if (modules.size() != 2)
    throw(engine_error() << __func__ << ": load modules failed");
}

/**
 *  Check loader unload method.
 */
void check_unload() {
  loader& loader(loader::instance());
  loader.unload_modules();
  std::list<std::shared_ptr<handle> > modules(loader.get_modules());
  if ((false == mod_test_load_quit) || (modules.size() != 0))
    throw(engine_error() << __func__ << ": unload modules failed");
}

/**
 *  Check loader change name.
 */
void check_change_name() {
  // Load module with initial name.
  loader& loader(loader::instance());
  std::shared_ptr<handle> module(loader.add_module(MOD_LIB_NAME, MOD_LIB_NAME));
  if (loader.get_modules().size() != 1)
    throw(engine_error() << __func__ << ": add module failed");

  // Change name.
  std::string new_name("New Name");
  module->set_name(new_name);

  // Check content.
  std::list<std::shared_ptr<handle> > modules(loader.get_modules());
  if (modules.size() != 1)
    throw(engine_error() << __func__ << ": set name failed");
  if ((*modules.begin())->get_name() != new_name)
    throw(engine_error() << __func__ << ": set name failed");
  loader.del_module(module);
}

/**
 *  Check the broker loader working.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  check_load();
  check_unload();
  check_change_name();
  return (0);
}

/**
 *  Init the unit test.
 */
int main(int argc, char** argv) {
  com::centreon::engine::unittest utest(argc, argv, &main_test);
  return (utest.run());
}

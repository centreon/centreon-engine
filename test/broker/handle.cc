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

#include "com/centreon/engine/broker/handle.hh"
#include <exception>
#include "com/centreon/engine/error.hh"
#include "test/broker/mod_load.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::broker;

static const char* MOD_LIB_UNKNOW = "./broker_mod_unknow.so";
static const char* MOD_LIB_NAME = "./broker_mod_load.so";

bool mod_test_load_quit = false;

/**
 *  Check the open mothod with unknow module.
 */
void check_open_noexist_module() {
  try {
    handle hwd;
    hwd.open(MOD_LIB_UNKNOW, MOD_LIB_UNKNOW);
  } catch (std::exception const& e) {
    (void)e;
    return;
  }
  throw(engine_error() << __func__ << ": try to open noexist module.");
}

/**
 *  Check the open mothod with existing module.
 */
void check_open_exist_module() {
  handle hwd;
  hwd.open(MOD_LIB_NAME, MOD_LIB_NAME);
  if (hwd.is_loaded() == false) {
    throw(engine_error() << __func__ << ": open module failed.");
  }
  if (hwd.get_handle() == NULL) {
    throw(engine_error() << __func__ << ": invalide handle.");
  }
  if (hwd.get_author() != MOD_AUTHOR) {
    throw(engine_error() << __func__ << ": invalide author.");
  }
  if (hwd.get_copyright() != MOD_COPYRIGHT) {
    throw(engine_error() << __func__ << ": invalide copyright.");
  }
  if (hwd.get_description() != MOD_DESCRIPTION) {
    throw(engine_error() << __func__ << ": invalide description.");
  }
  if (hwd.get_name() != MOD_TITLE) {
    throw(engine_error() << __func__ << ": invalide name.");
  }
  if (hwd.get_version() != MOD_VERSION) {
    throw(engine_error() << __func__ << ": invalide version.");
  }
  if (hwd.get_license() != MOD_LICENSE) {
    throw(engine_error() << __func__ << ": invalide license.");
  }
  if (hwd.get_args() != MOD_LIB_NAME) {
    throw(engine_error() << __func__ << ": invalide args.");
  }

  hwd.close();
  if (mod_test_load_quit == false) {
    throw(engine_error() << __func__ << ": close failed.");
  }
}

/**
 *  Check the copy constructor.
 */
void check_copy() {
  handle hwd(MOD_LIB_NAME, MOD_LIB_NAME);
  hwd.open();
  handle hwd_cpy(hwd);

  if ((hwd != hwd_cpy) || !(hwd == hwd_cpy))
    throw(engine_error() << __func__ << ": hwd and hwd_cpy are different");

  hwd_cpy.close();
  if (!mod_test_load_quit)
    throw(engine_error() << __func__ << ": close failed");

  if (hwd.is_loaded())
    throw(engine_error() << __func__ << ": is_loaded is true");

  return;
}

/**
 *  Check the broker handle working.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // check_open_noexist_module();
  // check_open_exist_module();
  check_copy();
  return (0);
}

/**
 *  Init the unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

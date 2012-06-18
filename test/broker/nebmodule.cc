/*
** Copyright 2011-2012 Merethis
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

#include <exception>
#include <limits.h>
#include <QCoreApplication>
#include <QDebug>
#include "com/centreon/engine/broker/loader.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/nebmods.hh"
#include "test/broker/mod_load.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::broker;

static const char* MOD_LIB_NAME        = "./broker_mod_load.so";
static const char* MOD_LIB_COMPAT_NAME = "./broker_mod_compatibility.so";

bool mod_test_load_quit = false;

/**
 *  Check nebmodule compatibility.
 */
int main_test() {
  loader& loader = loader::instance();

  if (neb_init_modules() != 0)
    throw (engine_error() << __func__ << ": neb_init_modules failed.");

  if (neb_add_module(MOD_LIB_NAME, MOD_LIB_NAME, true) != 0
      || neb_add_module(MOD_LIB_COMPAT_NAME, MOD_LIB_COMPAT_NAME, true) != 0)
    throw (engine_error() << __func__ << ": neb_add_module failed.");

  if (neb_load_all_modules() != 0)
    throw (engine_error() << __func__ << ": neb_load_all_modules failed.");

  if (loader.get_modules().size() != 2)
    throw (engine_error() << __func__ << ": invalid modules size.");

  std::list<QSharedPointer<handle> > modules(loader.get_modules());
  for (std::list<QSharedPointer<handle> >::const_iterator
         it(modules.begin()),
         end(modules.end());
       it != end;
       ++it) {
    if ((*it)->get_name() != MOD_TITLE)
      throw (engine_error() << __func__ << ": invalid name.");

    if ((*it)->get_author() != MOD_AUTHOR)
      throw (engine_error() << __func__ << ": invalide author.");

    if ((*it)->get_copyright() != MOD_COPYRIGHT)
      throw (engine_error() << __func__ << ": invalide copyright.");

    if ((*it)->get_description() != MOD_DESCRIPTION)
      throw (engine_error() << __func__ << ": invalide description.");

    if ((*it)->get_name() != MOD_TITLE)
      throw (engine_error() << __func__ << ": invalide name.");

    if ((*it)->get_version() != MOD_VERSION)
      throw (engine_error() << __func__ << ": invalide version.");

    if ((*it)->get_license() != MOD_LICENSE)
      throw (engine_error() << __func__ << ": invalide license.");
  }

  if (neb_unload_all_modules(NEBMODULE_FORCE_UNLOAD, true) != 0)
    throw (engine_error() << __func__ << ": neb_unload_all_modules failed.");

  if (neb_free_module_list() != 0)
    throw (engine_error() << __func__ << ": neb_free_module_list failed.");

  if (loader.get_modules().size() != 0)
    throw (engine_error() << __func__ << ": invalid modules size.");

  if (neb_deinit_modules() != 0)
    throw (engine_error() << __func__ << ": neb_deinit_modules failed.");

  return (0);
}

/**
 *  Init the unit test.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  unittest utest(&main_test);
  QObject::connect(&utest, SIGNAL(finished()), &app, SLOT(quit()));
  utest.start();
  app.exec();
  return (utest.ret());
}

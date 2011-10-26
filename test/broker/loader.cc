/*
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

#include <QCoreApplication>
#include <QDebug>
#include <exception>
#include <limits.h>
#include "test/unittest.hh"
#include "error.hh"
#include "broker/loader.hh"

using namespace com::centreon::engine::broker;

static const char* MOD_LIB_NAME = "./broker_mod_load.so";

bool mod_test_load_quit = false;

/**
 *  Check loader get/set directory.
 */
void check_directory() {
  loader& loader = loader::instance();

  if (loader.get_directory() != "") {
    throw (engine_error() << __func__ << ": directory is already set.");
  }

  loader.set_directory("./");
  if (loader.get_directory() != "./") {
    throw (engine_error() << __func__ << ": set directory failed.");
  }
}

/**
 *  Check loader load method.
 */
void check_load() {
  loader& loader = loader::instance();

  loader.load();
  QList<QSharedPointer<handle> > modules = loader.get_modules();
  if (modules.size() != 2) {
    throw (engine_error() << __func__ << ": load modules failed.");
  }
}

/**
 *  Check loader unload method.
 */
void check_unload() {
  loader& loader = loader::instance();

  loader.unload();
  QList<QSharedPointer<handle> > modules = loader.get_modules();
  if (mod_test_load_quit == false || modules.size() != 0) {
    throw (engine_error() << __func__ << ": unload modules failed.");
  }
}

/**
 *  Check loader change name.
 */
void check_chage_name() {
  loader& loader = loader::instance();

  QSharedPointer<handle> module = loader.add_module(MOD_LIB_NAME,
						    MOD_LIB_NAME);
  if (loader.get_modules().size() != 1) {
    throw (engine_error() << __func__ << ": add module failed.");
  }

  QString new_name = "New Name";
  module->set_name(new_name);

  QList<QSharedPointer<handle> > modules = loader.get_modules();
  if (modules.size() != 1) {
    throw (engine_error() << __func__ << ": set name failed.");
  }

  if ((*modules.begin())->get_name() != new_name) {
    throw (engine_error() << __func__ << ": set name failed.");
  }

  loader.del_module(module);
}

/**
 *  Check the broker loader working.
 */
int main_test() {
  check_directory();
  check_load();
  check_unload();
  check_chage_name();

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

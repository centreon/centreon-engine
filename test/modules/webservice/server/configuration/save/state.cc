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

#include <fstream>
#include <QCoreApplication>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/modules/webservice/configuration/save/state.hh"
#include "com/centreon/engine/globals.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::modules;

/**
 *  Run add_host_comment test.
 */
static int check_save_state() {
  std::ifstream file("./config.cfg", std::ios::binary);
  std::string ref;
  while (file.good() && !file.eof()) {
    char buffer[1024];
    file.read(buffer, sizeof(buffer));
    ref.append(buffer, file.gcount());
  }
  config.parse("./config.cfg");
  webservice::configuration::save::state save;
  save << config;
  if (save.to_string() != ref)
    throw (engine_error() << "check_update_condifuration failed.");
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  com::centreon::engine::unittest utest(&check_save_state);
  QObject::connect(&utest, SIGNAL(finished()), &app, SLOT(quit()));
  utest.start();
  app.exec();
  return (utest.ret());
}

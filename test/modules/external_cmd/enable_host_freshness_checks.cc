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
#include <QCoreApplication>
#include <QDebug>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/engine.hh"
#include "commands.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Run enable_host_freshness_checks test.
 */
static int check_enable_host_freshness_checks() {
  // Action.
  config.set_check_host_freshness(false);
  char const* cmd("[1317196300] ENABLE_HOST_FRESHNESS_CHECKS");
  process_external_command(cmd);

  // Check.
  if (!config.get_check_host_freshness())
    throw (engine_error() << "enable_host_freshness_checks failed.");

  // Success.
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  unittest utest(&check_enable_host_freshness_checks);
  QObject::connect(&utest, SIGNAL(finished()), &app, SLOT(quit()));
  utest.start();
  app.exec();
  return (utest.ret());
}

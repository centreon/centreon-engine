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
#include <QTemporaryFile>
#include <string>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/engine.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Run process_file test.
 */
static int check_process_file() {
  QTemporaryFile tmp("external_commands.cmd");
  if (!tmp.open())
    throw (engine_error() << "impossible to create temporary file.");
  tmp.write("[1317196300] ENABLE_NOTIFICATIONS\n");
  tmp.close();

  config.set_enable_notifications(false);
  std::string cmd("[1317196300] PROCESS_FILE;");
  cmd.append(tmp.fileName().toStdString());
  cmd.append(";0\n");
  process_external_command(cmd.c_str());

  if (!config.get_enable_notifications())
    throw (engine_error() << "process_file failed.");

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  unittest utest(&check_process_file);
  QObject::connect(&utest, SIGNAL(finished()), &app, SLOT(quit()));
  utest.start();
  app.exec();
  return (utest.ret());
}

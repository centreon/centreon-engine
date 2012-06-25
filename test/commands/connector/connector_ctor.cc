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
#include "com/centreon/engine/commands/connector/command.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/shared_ptr.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

#define CMD_NAME       "command_name"
#define CMD_LINE       "command_name arg1 arg2"
#define CONNECTOR_NAME "connector_test"
#define CONNECTOR_LINE "./bin_connector_test_run"

/**
 *  Check constructor and copy object.
 */
int main_test() {
  connector::command
    cmd1(CONNECTOR_NAME, CONNECTOR_LINE, CMD_NAME, CMD_LINE);
  if ((cmd1.get_name() != CMD_NAME)
      || (cmd1.get_connector_name() != CONNECTOR_NAME)
      || (cmd1.get_command_line() != CMD_LINE)
      || (cmd1.get_connector_line() != CONNECTOR_LINE))
    throw (engine_error() << "error: constructor failed");

  connector::command cmd2(cmd1);
  if (cmd1 != cmd2)
    throw (engine_error() << "error: copy constructor failed");

  connector::command cmd3 = cmd2;
  if (cmd3 != cmd2)
    throw (engine_error() << "error: assignment operator failed");

  com::centreon::shared_ptr<commands::command> cmd4(cmd3.clone());
  if (!cmd4.get())
    throw (engine_error() << "error: clone failed");

  if (*cmd4 != cmd3)
    throw (engine_error() << "error: clone failed");

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

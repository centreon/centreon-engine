/*
** Copyright 2011 Merethis
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
#include "error.hh"
#include "commands/connector/command.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

#define CMD_NAME    "command_name"
#define CMD_LINE    "command_name arg1 arg2"
#define CMD_PROCESS "./bin_connector_test_run"

/**
 *  Check constructor and copy object.
 */
int main(int argc, char** argv) {
  try {
    QCoreApplication app(argc, argv);

    connector::command cmd1(CMD_NAME, CMD_LINE, CMD_PROCESS);
    if (cmd1.get_name() != CMD_NAME
	|| cmd1.get_command_line() != CMD_LINE
	|| cmd1.get_process() != CMD_PROCESS)
      throw (engine_error() << "Constructor failed.");

    connector::command cmd2(cmd1);
    if (cmd1 != cmd2)
      throw (engine_error() << "Default copy constructor failed.");

    connector::command cmd3 = cmd2;
    if (cmd3 != cmd2)
      throw (engine_error() << "Default copy operator failed.");

    QSharedPointer<commands::command> cmd4(cmd3.clone());
    if (cmd4.isNull() == true)
      throw (engine_error() << "clone failed.");
    if (*cmd4 != cmd3)
      throw (engine_error() << "clone failed.");
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}

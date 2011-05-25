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

#include <QDebug>
#include <exception>
#include "commands/raw.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

#define CMD_NAME "command_name"
#define CMD_LINE "command_name arg1 arg2"

int main() {
  try {
    raw cmd1;
    if (cmd1.get_name() != ""
	|| cmd1.get_command_line() != "") {
      qDebug() << "error: Default constructor failed.";
      return (1);
    }

    raw cmd2(CMD_NAME, CMD_LINE);
    if (cmd2.get_name() != CMD_NAME
	|| cmd2.get_command_line() != CMD_LINE) {
      qDebug() << "error: Constructor failed.";
      return (1);
    }

    raw cmd3(cmd2);
    if (cmd3 != cmd2) {
      qDebug() << "error: Default copy constructor failed.";
      return (1);
    }

    raw cmd4 = cmd3;
    if (cmd4 != cmd3) {
      qDebug() << "error: Default copy operator failed.";
      return (1);
    }

    QSharedPointer<commands::command> cmd5(cmd4.clone());
    if (cmd5.isNull() == true) {
      qDebug() << "error: clone failed.";
      return (1);
    }
    if (*cmd5 != cmd4) {
      qDebug() << "error: clone failed.";
      return (1);
    }
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  return (0);
}

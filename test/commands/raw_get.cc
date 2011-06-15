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

using namespace com::centreon::engine::commands;

#define DEFAULT_CMD_NAME "cmd"
#define DEFAULT_CMD_LINE "ls -la /tmp"

int main() {
  try {
    raw cmd(DEFAULT_CMD_NAME, DEFAULT_CMD_LINE);

    if (cmd.get_name() != DEFAULT_CMD_NAME) {
      qDebug() << "error: name invalid value.";
      return (1);
    }

    if (cmd.get_command_line() != DEFAULT_CMD_LINE) {
      qDebug() << "error: command_line invalid value.";
      return (1);
    }
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }

  return (0);
}

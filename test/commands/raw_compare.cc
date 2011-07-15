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
#include "error.hh"
#include "commands/raw.hh"

using namespace com::centreon::engine::commands;

#define CMD_NAME "command_name"
#define CMD_LINE "command_name arg1 arg2"

/**
 * Check comparison operator.
 */
int main() {
  try {
    raw cmd(CMD_NAME, CMD_LINE);
    if (!(cmd == cmd))
      throw (engine_error() << "operator== failed.");
    if (cmd != cmd)
      throw (engine_error() << "operator!= failed.");
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }

  return (0);
}

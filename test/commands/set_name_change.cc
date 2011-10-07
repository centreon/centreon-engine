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
#include "commands/set.hh"
#include "commands/raw.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

/**
 *  Check if the command exist in the set command.
 *
 *  @param[in] name The command name.
 *
 *  @return True if the command exist, false otherwise.
 */
static bool command_exist(QString const& name) {
  try {
    set::instance().get_command(name);
  }
  catch (...) {
    return (false);
  }
  return (true);
}

/**
 *  Check if the change name system works.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  try {
    // get instance.
    set& cmd_set = set::instance();

    // add command.
    raw raw("raw", "raw argv1 argv2");
    cmd_set.add_command(raw);

    // get command.
    QSharedPointer<commands::command> cmd = cmd_set.get_command("raw");

    // change command name.
    cmd->set_name("cmd");

    // get command with new name.
    QSharedPointer<commands::command> new_cmd = cmd_set.get_command("cmd");

    // check if the old command name is not found.
    if (command_exist("raw") == true) {
      qDebug() << "error: command name changed failed.";
      return (1);
    }

    // remove new name.
    cmd_set.remove_command("cmd");

    // check if the old command name is not found.
    if (command_exist("cmd") == true) {
      qDebug() << "error: command name changed failed.";
      return (1);
    }
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }

  return (0);
}

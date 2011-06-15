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


#include <QDebug>
#include <exception>
#include "commands/set.hh"
#include "commands/raw.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

static bool command_exit(QString const& name) {
  try {
    set::instance().get_command(name);
  }
  catch (...) {
    return (false);
  }
  return (true);
}

int main() {
  try {
    // get instance.
    set& cmd_set = set::instance();

    // add commands.
    raw raw1("raw1", "raw1 argv1 argv2");
    cmd_set.add_command(raw1);

    QSharedPointer<commands::command> pcmd2(raw1.clone());
    cmd_set.add_command(pcmd2);

    QSharedPointer<commands::command> pcmd3(new raw("pcmd3", "pcmd3 argv1 argv2"));
    cmd_set.add_command(pcmd3);

    // get commands.
    if (command_exit("raw1") == false) {
      qDebug() << "error: get_command failed, 'raw1' not found.";
      return (1);
    }
    if (command_exit("pcmd3") == false) {
      qDebug() << "error: get_command failed, 'pcmd3' not found.";
      return (1);
    }

    if (command_exit("undef") == true) {
      qDebug() << "error: get_command failed, 'undef' found.";
      return (1);
    }

    // remove commands.
    cmd_set.remove_command("pcmd3");
    if (command_exit("pcmd3") == true) {
      qDebug() << "error: remove_command failed, 'pcmd3' found.";
      return (1);
    }

    cmd_set.remove_command("raw1");
    if (command_exit("raw1") == true) {
      qDebug() << "error: remove_command failed, 'raw1' found.";
      return (1);
    }

    cmd_set.remove_command("undef");
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }

  return (0);
}

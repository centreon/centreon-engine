/*
** Copyright 2011-2013 Merethis
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

#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/deleter/command.hh"
#include "com/centreon/engine/objects.hh"

using namespace com::centreon::engine;

/**
 *  Delete command.
 *
 *  @param[in] ptr The command to delete.
 */
void deleter::command(void* ptr) throw () {
  if (!ptr)
    return;

  command_struct* obj(static_cast<command_struct*>(ptr));

  try {
    // update command executon system.
    commands::set::instance().remove_command(obj->name);
  }
  catch (...) {
  }

  delete[] obj->name;
  obj->name = NULL;
  delete[] obj->command_line;
  obj->command_line = NULL;

  delete obj;
}

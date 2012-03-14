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
#include <QDebug>
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/engine/macros.hh"
#include "test/objects/add_object_to_member.hh"
#include "test/objects/create_object.hh"

using namespace com::centreon::engine::objects;
using namespace test::objects;

int main() {
  com::centreon::engine::logging::engine::load();
  com::centreon::engine::commands::set::load();
  try {
    add_with_null_member(&add_commands_to_object);
    add_without_objects(&add_commands_to_object);
    add_with_objects(&add_commands_to_object,
                     &create_command,
                     1);
    add_with_objects(&add_commands_to_object,
                     &create_command,
                     10);
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    free_memory(get_global_macros());
    return (1);
  }
  return (0);
}

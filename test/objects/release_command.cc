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

#include <cstdlib>
#include <exception>
#include <iostream>
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/engine/macros.hh"
#include "test/objects/create_object.hh"
#include "test/objects/release.hh"

using namespace test::objects;

/**
 *  Check that command can be properly released.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main() {
  // Initialization.
  com::centreon::engine::logging::engine::load();
  com::centreon::engine::commands::set::load();

  // Tests.
  try {
    release_null_pointer(static_cast<command const*>(NULL));
    release_objects(
      &create_command,
      command_list,
      command_list_tail);
    release_objects(
      &create_command,
      command_list,
      command_list_tail,
      10);
  }
  // Error.
  catch (std::exception const& e) {
    std::cerr << "error: " << e.what() << std::endl;
    free_memory(get_global_macros());
    return (EXIT_FAILURE);
  }

  // Success.
  return (EXIT_SUCCESS);
}

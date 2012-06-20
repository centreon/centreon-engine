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
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/utils.hh"
#include "test/objects/add_object_to_member.hh"
#include "test/objects/create_object.hh"

using namespace com::centreon::engine;
using namespace test::objects;

/**
 *  Check that host can be added to member.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main() {
  // Initialization.
  logging::engine::load();

  try {
    // Tests.
    add_with_null_member(&objects::add_hosts_to_object);
    add_without_objects(&objects::add_hosts_to_object);
    add_with_objects(
      &objects::add_hosts_to_object,
      &create_host,
      1);
    add_with_objects(
      &objects::add_hosts_to_object,
      &create_host,
      10);
  }
  catch (std::exception const& e) {
    // Exception handling.
    std::cerr << "error: " << e.what() << std::endl;
    free_memory(get_global_macros());
    return (EXIT_FAILURE);
  }

  return (EXIT_SUCCESS);
}

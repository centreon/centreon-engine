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

#include <cstdlib>
#include <exception>
#include <iostream>
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/utils.hh"
#include "test/objects/add_object_to_member.hh"
#include "test/objects/create_object.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace test::objects;

/**
 *  Check that contact can be added to member.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;
  try {
    // Tests.
    add_with_null_member(&objects::add_contacts_to_object);
    add_without_objects(&objects::add_contacts_to_object);
    add_with_objects(
      &objects::add_contacts_to_object,
      &create_contact,
      1);
    add_with_objects(
      &objects::add_contacts_to_object,
      &create_contact,
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

/**
 *  Init the unit test.
 */
int main(int argc, char** argv) {
  com::centreon::engine::unittest utest(argc, argv, &main_test);
  return (utest.run());
}

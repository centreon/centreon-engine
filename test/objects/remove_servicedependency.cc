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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/utils.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Check if remove servicedependency works with some servicedependency.
 */
static void remove_all_service_dependency() {
  init_object_skiplists();

  add_service_dependency("service_dependency_dependent_host_name_1",
			 "service_dependency_dependent_service_description",
			 "service_dependency_host_name_1",
			 "service_dependency_service_description",
			 0, 0, 0, 0, 0, 0, 0,
			 "service_dependency_dependency_period");
  add_service_dependency("service_dependency_dependent_host_name_2",
			 "service_dependency_dependent_service_description",
			 "service_dependency_host_name_2",
			 "service_dependency_service_description",
			 0, 0, 0, 0, 0, 0, 0,
			 "service_dependency_dependency_period");
  add_service_dependency("service_dependency_dependent_host_name_3",
			 "service_dependency_dependent_service_description",
			 "service_dependency_host_name_3",
			 "service_dependency_service_description",
			 0, 0, 0, 0, 0, 0, 0,
			 "service_dependency_dependency_period");

  if (remove_service_dependency_by_id("service_dependency_host_name_1",
                                      "service_dependency_service_description",
                                      "service_dependency_dependent_host_name_1",
                                      "service_dependency_dependent_service_description") != 1
      || remove_service_dependency_by_id("service_dependency_host_name_2",
                                         "service_dependency_service_description",
                                         "service_dependency_dependent_host_name_2",
                                         "service_dependency_dependent_service_description") != 1
      || remove_service_dependency_by_id("service_dependency_host_name_3",
                                         "service_dependency_service_description",
                                         "service_dependency_dependent_host_name_3",
                                         "service_dependency_dependent_service_description") != 1
      || servicedependency_list != NULL
      || servicedependency_list_tail != NULL)
    throw (engine_error() << "remove all service dependency failed.");

  free_object_skiplists();
}

/**
 *  Check if remove servicedependency works with invalid call.
 */
static void remove_service_dependency_failed() {
  init_object_skiplists();

  if (remove_service_dependency_by_id("", "", "", "") == 1)
    throw (engine_error() << "service dependency remove but dosen't exist.");
  if (remove_service_dependency_by_id(NULL, NULL, NULL, NULL) == 1)
    throw (engine_error() << "service dependency remove but pointer is NULL.");

  free_object_skiplists();
}

/**
 *  Check if service dependency removal works.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;
  try {
    // Tests.
    remove_all_service_dependency();
    remove_service_dependency_failed();
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

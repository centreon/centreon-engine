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
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects/service.hh"
#include "com/centreon/engine/objects/servicegroup.hh"
#include "com/centreon/engine/utils.hh"
#include "test/objects/create_object.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::objects;
using namespace test::objects;

static bool create_and_link(bool has_services, bool has_servicegroups) {
  init_object_skiplists();
  servicegroup* obj = create_servicegroup(1);
  std::vector<service*> services;
  std::vector<servicegroup*> servicegroups;
  bool ret = true;

  for (unsigned int i = 0; i < 10; ++i) {
    if (has_services == true)
      services.push_back(create_service(i + 1));
    if (has_servicegroups == true) {
      servicegroups.push_back(create_servicegroup(i + 2));
    }
  }

  try {
    link(obj,
         services,
         servicegroups);
    if (has_services && !obj->members)
      ret = false;
  }
  catch (std::exception const& e) {
    (void)e;
    ret = false;
  }
  release(obj);
  for (unsigned int i = 0; i < 10; ++i) {
    if (has_services == true)
      release(services[i]);
    if (has_servicegroups == true)
      release(servicegroups[i]);
  }
  free_object_skiplists();
  return (ret);
}

static void link_null_pointer() {
  try {
    std::vector<servicegroup*> servicegroups;
    std::vector<service*> services;
    link(static_cast<servicegroup*>(NULL),
         services,
         servicegroups);
  }
  catch (std::exception const& e) {
    (void)e;
  }
}

static void link_null_name() {
  init_object_skiplists();
  servicegroup* obj = NULL;
  try {
    obj = create_servicegroup(1);
    std::vector<servicegroup*> servicegroups;
    std::vector<service*> services;

    delete[] obj->group_name;
    obj->group_name = NULL;

    link(obj,
         services,
         servicegroups);
  }
  catch (std::exception const& e) {
    (void)e;
  }
  release(obj);
  free_object_skiplists();
}

static void link_without_services() {
  if (!create_and_link(false, true))
    throw (engine_error() << __func__ << " failed: invalid return");
  return ;
}

static void link_without_servicegroups() {
  if (!create_and_link(true, false))
    throw (engine_error() << __func__ << " failed: invalid return");
  return ;
}

static void link_with_valid_objects() {
  if (!create_and_link(true, true))
    throw (engine_error() << __func__ << " failed: invalid return");
  return ;
}

/**
 *  Check linkage of service group.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;
  try {
    // Tests.
    link_null_pointer();
    link_null_name();
    link_without_services();
    link_without_servicegroups();
    link_with_valid_objects();
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

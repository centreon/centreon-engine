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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects/host.hh"
#include "com/centreon/engine/objects/hostgroup.hh"
#include "com/centreon/engine/utils.hh"
#include "test/objects/create_object.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::objects;
using namespace test::objects;

static bool create_and_link(bool has_hosts, bool has_hostgroups) {
  init_object_skiplists();
  hostgroup* obj = create_hostgroup(1);
  std::vector<host*> hosts;
  std::vector<hostgroup*> hostgroups;
  bool ret = true;

  for (unsigned int i = 0; i < 10; ++i) {
    if (has_hosts == true)
      hosts.push_back(create_host(i + 1));
    if (has_hostgroups == true) {
      hostgroups.push_back(create_hostgroup(i + 2));
    }
  }

  try {
    link(obj,
         hosts,
         hostgroups);
    if (has_hosts && !obj->members)
      ret = false;
  }
  catch (std::exception const& e) {
    (void)e;
    ret = false;
  }
  release(obj);
  for (unsigned int i = 0; i < 10; ++i) {
    if (has_hosts == true)
      release(hosts[i]);
    if (has_hostgroups == true)
      release(hostgroups[i]);
  }
  free_object_skiplists();
  return (ret);
}

static void link_null_pointer() {
  try {
    std::vector<hostgroup*> hostgroups;
    std::vector<host*> hosts;
    link(static_cast<hostgroup*>(NULL),
         hosts,
         hostgroups);
  }
  catch (std::exception const& e) {
    (void)e;
  }
}

static void link_null_name() {
  init_object_skiplists();
  hostgroup* obj = NULL;
  try {
    obj = create_hostgroup(1);
    std::vector<hostgroup*> hostgroups;
    std::vector<host*> hosts;

    delete[] obj->group_name;
    obj->group_name = NULL;

    link(obj,
         hosts,
         hostgroups);
  }
  catch (std::exception const& e) {
    (void)e;
  }
  release(obj);
  free_object_skiplists();
}

static void link_without_hosts() {
  if (!create_and_link(false, true))
    throw (engine_error() << __func__ << " failed: invalid return");
  return ;
}

static void link_without_hostgroups() {
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
 *  Check linkage of host group.
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
    link_without_hosts();
    link_without_hostgroups();
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

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
#include "com/centreon/engine/objects/contact.hh"
#include "com/centreon/engine/objects/contactgroup.hh"
#include "com/centreon/engine/utils.hh"
#include "test/objects/create_object.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::objects;
using namespace test::objects;

static bool create_and_link(bool has_contacts, bool has_contactgroups) {
  init_object_skiplists();
  contactgroup* obj = create_contactgroup(1);
  std::vector<contact*> contacts;
  std::vector<contactgroup*> contactgroups;
  bool ret = true;

  for (unsigned int i = 0; i < 10; ++i) {
    if (has_contacts == true)
      contacts.push_back(create_contact(i + 1));
    if (has_contactgroups == true) {
      contactgroups.push_back(create_contactgroup(i + 2));
    }
  }

  try {
    link(obj,
         contacts,
         contactgroups);
    if (has_contacts && !obj->members)
      ret = false;
  }
  catch (std::exception const& e) {
    (void)e;
    ret = false;
  }
  release(obj);
  for (unsigned int i = 0; i < 10; ++i) {
    if (has_contacts == true)
      release(contacts[i]);
    if (has_contactgroups == true)
      release(contactgroups[i]);
  }
  free_object_skiplists();
  return (ret);
}

static void link_null_pointer() {
  try {
    std::vector<contactgroup*> contactgroups;
    std::vector<contact*> contacts;
    link(
      static_cast<contactgroup*>(NULL),
      contacts,
      contactgroups);
  }
  catch (std::exception const& e) {
    (void)e;
  }
}

static void link_null_name() {
  init_object_skiplists();
  contactgroup* obj = NULL;
  try {
    obj = create_contactgroup(1);
    std::vector<contactgroup*> contactgroups;
    std::vector<contact*> contacts;

    delete [] obj->group_name;
    obj->group_name = NULL;

    link(obj,
         contacts,
         contactgroups);
  }
  catch (std::exception const& e) {
    (void)e;
  }
  release(obj);
  free_object_skiplists();
}

static void link_without_contacts() {
  if (create_and_link(false, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

static void link_without_contactgroups() {
  if (create_and_link(true, false) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

static void link_with_valid_objects() {
  if (create_and_link(true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

/**
 *  Check linkage of contact group.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main() {
  // Initialization.
  logging::engine::load();

  try {
    // Tests.
    link_null_pointer();
    link_null_name();
    link_without_contacts();
    link_without_contactgroups();
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

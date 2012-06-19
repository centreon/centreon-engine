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
#include "com/centreon/engine/objects/service.hh"
#include "com/centreon/engine/objects/serviceescalation.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/utils.hh"
#include "test/objects//create_object.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::objects;
using namespace test::objects;

static bool create_and_link(bool has_contacts,
                            bool has_contactgroups,
                            bool has_escalation_period) {
  init_object_skiplists();
  service* hst = create_service(1);
  serviceescalation* obj = create_serviceescalation(1);
  std::vector<contact*> contacts;
  std::vector<contactgroup*> contactgroups;
  timeperiod* escalation_period = NULL;
  bool ret = true;

  if (has_escalation_period == true)
    escalation_period = create_timeperiod(1);

  for (unsigned int i = 0; i < 10; ++i) {
    if (has_contacts == true)
      contacts.push_back(create_contact(i + 1));
    if (has_contactgroups == true)
      contactgroups.push_back(create_contactgroup(i + 1));
  }

  try {
    link(obj,
         contacts,
         contactgroups,
         escalation_period);
    if (!obj->service_ptr
        || (has_contacts && !obj->contacts)
        || (has_contactgroups && !obj->contact_groups)
        || obj->escalation_period_ptr != escalation_period)
      ret = false;
  }
  catch (std::exception const& e) {
    (void)e;
    ret = false;
  }

  release(hst);
  release(obj);
  release(escalation_period);
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
    link(static_cast<serviceescalation*>(NULL),
         contacts,
         contactgroups,
         NULL);
  }
  catch (std::exception const& e) {
    (void)e;
  }
}

static void link_null_name() {
  init_object_skiplists();
  serviceescalation* obj = NULL;
  try {
    obj = create_serviceescalation(1);
    std::vector<contact*> contacts;
    std::vector<contactgroup*> contactgroups;

    delete[] obj->host_name;
    obj->host_name = NULL;

    link(obj,
         contacts,
         contactgroups,
         NULL);
  }
  catch (std::exception const& e) {
    (void)e;
  }
  release(obj);
  free_object_skiplists();
}

static void link_without_contacts() {
  if (create_and_link(false, true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

static void link_without_contactgroups() {
  if (create_and_link(true, false, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

static void link_without_contacts_and_contactgroups() {
  if (create_and_link(false, false, true) == true)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

static void link_without_escalation_period() {
  if (create_and_link(true, true, false) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

static void link_with_valid_objects() {
  if (create_and_link(true, true, true) == false)
    throw (engine_error() << Q_FUNC_INFO << " invalid return");
}

/**
 *  Check linkage of service escalation.
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
    link_without_contacts_and_contactgroups();
    link_without_escalation_period();
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

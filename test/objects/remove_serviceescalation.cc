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
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon::engine;

/**
 *  Check if remove serviceescalation works with some serviceescalation.
 */
static void remove_all_serviceescalation() {
  init_object_skiplists();

  add_service_escalation("serviceescalation_host_name_1",
			"serviceescalation_description",
			0, 0, 0.0,
			"serviceescalation_escalation_period",
			0, 0, 0, 0);
  add_service_escalation("serviceescalation_host_name_1",
			"serviceescalation_description",
			0, 0, 0.0,
			"serviceescalation_escalation_period",
			0, 0, 0, 0);
  add_service_escalation("serviceescalation_host_name_1",
			"serviceescalation_description",
			0, 0, 0.0,
			"serviceescalation_escalation_period",
			0, 0, 0, 0);

  if (remove_service_escalation_by_id("serviceescalation_name_2",
				     "serviceescalation_description") != 1
      || remove_service_escalation_by_id("serviceescalation_name_1",
					"serviceescalation_description") != 1
      || remove_service_escalation_by_id("serviceescalation_name_3",
					"serviceescalation_description") != 1
      || serviceescalation_list != NULL
      || serviceescalation_list_tail != NULL)
    throw (engine_error() << "remove all serviceescalation failed.");

  free_object_skiplists();
}

/**
 *  Check if remove serviceescalation works with invalid call.
 */
static void remove_serviceescalation_failed() {
  init_object_skiplists();

  if (remove_service_escalation_by_id("", "") == 1)
    throw (engine_error() << "serviceescalation remove but dosen't exist.");
  if (remove_service_escalation_by_id(NULL, NULL) == 1)
    throw (engine_error() << "serviceescalation remove but pointer is NULL.");

  if (remove_service_escalation_by_id(NULL, "") == 1)
    throw (engine_error() << "serviceescalation remove but name is NULL.");

  if (remove_service_escalation_by_id("", NULL) == 1)
    throw (engine_error() << "serviceescalation remove but description is NULL.");

  free_object_skiplists();
}

/**
 *  Check if remove serviceescalation works with contactgroups.
 */
static void remove_serviceescalation_with_contactgroups() {
  init_object_skiplists();

  serviceescalation* se = add_service_escalation("serviceescalation_host_name_1",
						"serviceescalation_description",
						0, 0, 0.0,
						"serviceescalation_escalation_period",
						0, 0, 0, 0);
  contactgroup* cgroup = add_contactgroup("contactgroup_name", "contactgroup_alias");
  contactgroupsmember* cgm = add_contactgroup_to_serviceescalation(se, "contactgroup_name");
  cgm->group_ptr = cgroup;

  if (remove_service_escalation_by_id("serviceescalation_host_name_1",
				     "serviceescalation_description") != 1
      || serviceescalation_list != NULL
      || serviceescalation_list_tail != NULL)
    throw (engine_error() << "remove serviceescalation with serviceescalation failed.");

  delete[] cgroup->group_name;
  delete[] cgroup->alias;
  delete cgroup;

  free_object_skiplists();
}

/**
 *  Check if remove serviceescalation works with some contacts.
 */
static void remove_serviceescalation_with_contacts() {
  init_object_skiplists();

  serviceescalation* se = add_service_escalation("serviceescalation_host_name_1",
						"serviceescalation_description",
						0, 0, 0.0,
						"serviceescalation_escalation_period",
						0, 0, 0, 0);
  contact* cntct = add_contact("contact_name", NULL, NULL, NULL, NULL, NULL,
			       NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  contactsmember* cm = add_contact_to_serviceescalation(se, "contact_name");
  cm->contact_ptr = cntct;

  if (remove_service_escalation_by_id("serviceescalation_host_name_1",
				     "serviceescalation_description") != 1
      || serviceescalation_list != NULL
      || serviceescalation_list_tail != NULL)
    throw (engine_error() << "remove serviceescalation with serviceescalation failed.");

  delete[] cntct->name;
  delete[] cntct->alias;
  delete cntct;

  free_object_skiplists();
}

/**
 *  Check if remove serviceescalation works.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main() {
  // Initialization.
  logging::engine::load();

  try {
    // Tests.
    remove_all_serviceescalation();
    remove_serviceescalation_failed();
    remove_serviceescalation_with_contactgroups();
    remove_serviceescalation_with_contacts();
  }
  catch (std::exception const& e) {
    // Exception handling.
    std::cerr << "error: " << e.what() << std::endl;
    free_memory(get_global_macros());
    return (EXIT_FAILURE);
  }

  return (EXIT_SUCCESS);
}

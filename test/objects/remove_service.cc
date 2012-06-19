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
 *  Check if remove service works with some services.
 */
static void remove_all_service() {
  init_object_skiplists();

  add_service("service_host_name_1", "service_description", "display_name",
	      "check_period", 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, "notification_period",
	      0, 0, 0, 0, 0, 0, 0, 0, "event_handler", 0, "check_command", 0, 0,
	      0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "failure_prediction_options",
	      0, 0, "notes", "notes_url", "action_url", "icon_image", "icon_image_alt",
	      0, 0, 0);
  add_service("service_host_name_2", "service_description", "display_name",
	      "check_period", 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, "notification_period",
	      0, 0, 0, 0, 0, 0, 0, 0, "event_handler", 0, "check_command", 0, 0,
	      0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "failure_prediction_options",
	      0, 0, "notes", "notes_url", "action_url", "icon_image", "icon_image_alt",
	      0, 0, 0);
  add_service("service_host_name_3", "service_description", "display_name",
	      "check_period", 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, "notification_period",
	      0, 0, 0, 0, 0, 0, 0, 0, "event_handler", 0, "check_command", 0, 0,
	      0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "failure_prediction_options",
	      0, 0, "notes", "notes_url", "action_url", "icon_image", "icon_image_alt",
	      0, 0, 0);

  if (remove_service_by_id("service_host_name_1", "service_description") != 1
      || remove_service_by_id("service_host_name_2", "service_description") != 1
      || remove_service_by_id("service_host_name_3", "service_description") != 1
      || service_list != NULL
      || service_list_tail != NULL)
    throw ("remove all service failed.");

  free_object_skiplists();
}

/**
 *  Check if remove service works with invalid call.
 */
static void remove_service_failed() {
  init_object_skiplists();

  if (remove_service_by_id("", "") == 1)
    throw (engine_error() << "service remove but dosen't exist.");
  if (remove_service_by_id(NULL, NULL) == 1)
    throw (engine_error() << "service remove but pointer is NULL.");
  if (remove_service_by_id(NULL, "") == 1)
    throw (engine_error() << "service remove but host name is NULL.");
  if (remove_service_by_id("", NULL) == 1)
    throw (engine_error() << "service remove but service description is NULL.");

  free_object_skiplists();
}

/**
 *  Check if remove service works with some contactgroups.
 */
static void remove_service_with_contactgroups() {
  init_object_skiplists();

  service* svc = add_service("service_host_name_1", "service_description", "display_name",
			     "check_period", 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, "notification_period",
			     0, 0, 0, 0, 0, 0, 0, 0, "event_handler", 0, "check_command", 0, 0,
			     0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "failure_prediction_options",
			     0, 0, "notes", "notes_url", "action_url", "icon_image", "icon_image_alt",
			     0, 0, 0);

  contactgroup* cgroup = add_contactgroup("contactgroup_name", "contactgroup_alias");
  contactgroupsmember* cgm = add_contactgroup_to_service(svc, "contactgroup_name");
  cgm->group_ptr = cgroup;

  if (remove_service_by_id("service_host_name_1", "service_description") != 1
      || service_list != NULL
      || service_list_tail != NULL)
    throw (engine_error() << "remove service with contact groups failed.");

  delete[] cgroup->group_name;
  delete[] cgroup->alias;
  delete cgroup;

  free_object_skiplists();
}

/**
 *  Check if remove service works with some contacts.
 */
static void remove_service_with_contacts() {
  init_object_skiplists();

  service* svc = add_service("service_host_name_1", "service_description", "display_name",
			     "check_period", 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, "notification_period",
			     0, 0, 0, 0, 0, 0, 0, 0, "event_handler", 0, "check_command", 0, 0,
			     0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "failure_prediction_options",
			     0, 0, "notes", "notes_url", "action_url", "icon_image", "icon_image_alt",
			     0, 0, 0);
  contact* cntct = add_contact("contact_name", NULL, NULL, NULL, NULL, NULL,
			       NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  contactsmember* cm = add_contact_to_service(svc, "contact_name");
  cm->contact_ptr = cntct;

  if (remove_service_by_id("service_host_name_1", "service_description") != 1
      || service_list != NULL
      || service_list_tail != NULL)
    throw (engine_error() << "remove service with contacts failed.");

  delete[] cntct->name;
  delete[] cntct->alias;
  delete cntct;

  free_object_skiplists();
}

/**
 *  Check if remove service works with some customvariables.
 */
static void remove_service_with_customvariables() {
  init_object_skiplists();

  service* svc = add_service("service_host_name_1", "service_description", "display_name",
  			     "check_period", 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, "notification_period",
  			     0, 0, 0, 0, 0, 0, 0, 0, "event_handler", 0, "check_command", 0, 0,
  			     0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "failure_prediction_options",
  			     0, 0, "notes", "notes_url", "action_url", "icon_image", "icon_image_alt",
  			     0, 0, 0);
  add_custom_variable_to_service(svc, "varname", "varvalue");

  if (remove_service_by_id("service_host_name_1", "service_description") != 1
      || service_list != NULL
      || service_list_tail != NULL)
    throw (engine_error() << "remove service with custom variables failed.");

  free_object_skiplists();
}

/**
 *  Check if remove service works with some serviceescalation.
 */
static void remove_service_with_serviceescalation() {
  init_object_skiplists();

  service* svc = add_service("service_host_name_1", "service_description", "display_name",
  			     "check_period", 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, "notification_period",
  			     0, 0, 0, 0, 0, 0, 0, 0, "event_handler", 0, "check_command", 0, 0,
  			     0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "failure_prediction_options",
  			     0, 0, "notes", "notes_url", "action_url", "icon_image", "icon_image_alt",
  			     0, 0, 0);
  serviceescalation* se = add_service_escalation("serviceescalation_host_name_1",
						"serviceescalation_description",
						0, 0, 0.0,
						"serviceescalation_escalation_period",
						0, 0, 0, 0);
  se->service_ptr = svc;

  if (remove_service_by_id("service_host_name_1", "service_description") != 1
      || service_list != NULL
      || service_list_tail != NULL)
    throw (engine_error() << "remove service with serviceescalation failed.");

  free_object_skiplists();
}

/**
 *  Check if remove service works with some servicedependency.
 */
static void remove_service_with_servicedependency() {
  init_object_skiplists();

  service* svc = add_service("service_host_name_1", "service_description", "display_name",
  			     "check_period", 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, "notification_period",
  			     0, 0, 0, 0, 0, 0, 0, 0, "event_handler", 0, "check_command", 0, 0,
  			     0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "failure_prediction_options",
  			     0, 0, "notes", "notes_url", "action_url", "icon_image", "icon_image_alt",
  			     0, 0, 0);
  servicedependency* sd = add_service_dependency("service_dependency_dependent_host_name_3",
						 "service_dependency_dependent_service_description",
						 "service_dependency_host_name_3",
						 "service_dependency_service_description",
						 0, 0, 0, 0, 0, 0, 0,
						 "service_dependency_dependency_period");
  sd->master_service_ptr = svc;

  if (remove_service_by_id("service_host_name_1", "service_description") != 1
      || service_list != NULL
      || service_list_tail != NULL)
    throw (engine_error() << "remove service with servicedependency (master) failed.");

  svc = add_service("service_host_name_1", "service_description", "display_name",
  		    "check_period", 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, "notification_period",
  		    0, 0, 0, 0, 0, 0, 0, 0, "event_handler", 0, "check_command", 0, 0,
  		    0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "failure_prediction_options",
  		    0, 0, "notes", "notes_url", "action_url", "icon_image", "icon_image_alt",
  		    0, 0, 0);
  sd = add_service_dependency("service_dependency_dependent_host_name_3",
  			      "service_dependency_dependent_service_description",
  			      "service_dependency_host_name_3",
  			      "service_dependency_service_description",
  			      0, 0, 0, 0, 0, 0, 0,
  			      "service_dependency_dependency_period");
  sd->dependent_service_ptr = svc;

  if (remove_service_by_id("service_host_name_1", "service_description") != 1
      || service_list != NULL
      || service_list_tail != NULL)
    throw (engine_error() << "remove service with servicedependency (dependency) failed.");

  free_object_skiplists();
}

/**
 *  Check if remove service works with some host.
 */
static void remove_service_with_host() {
  init_object_skiplists();

  service* svc = add_service("host_name_1", "service_description", "display_name",
  			     "check_period", 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, "notification_period",
  			     0, 0, 0, 0, 0, 0, 0, 0, "event_handler", 0, "check_command", 0, 0,
  			     0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "failure_prediction_options",
  			     0, 0, "notes", "notes_url", "action_url", "icon_image", "icon_image_alt",
  			     0, 0, 0);
  host* hst = add_host("host_name_1", "host_display_name", "host_alias",
  		       "localhost", NULL, 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
  		       0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0, 0, 0.0,
  		       0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL,
  		       NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0,
  		       0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  add_service_link_to_host(hst, svc);

  if (remove_service_by_id("host_name_1", "service_description") != 1
      || service_list != NULL
      || service_list_tail != NULL)
    throw (engine_error() << "remove service with host failed.");


  delete[] hst->name;
  delete[] hst->display_name;
  delete[] hst->alias;
  delete[] hst->address;
  delete hst;

  free_object_skiplists();
}

/**
 *  Check if remove service works with.
 */
int main() {
  // Initialization.
  logging::engine::load();

  try {
    // Tests.
    remove_all_service();
    remove_service_failed();
    remove_service_with_contactgroups();
    remove_service_with_contacts();
    remove_service_with_customvariables();
    remove_service_with_serviceescalation();
    remove_service_with_servicedependency();
    remove_service_with_host();
  }
  catch (std::exception const& e) {
    // Exception handling.
    std::cerr << "error: " << e.what() << std::endl;
    free_memory(get_global_macros());
    return (EXIT_FAILURE);
  }

  return (EXIT_SUCCESS);
}

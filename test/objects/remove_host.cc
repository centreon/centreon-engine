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
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Check if remove host works with some hosts.
 */
static void remove_all_host() {
  init_object_skiplists();

  add_host("host_name_1", "host_display_name", "host_alias",
	   "localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
	   0.0, 0.0, "notification_period", 0, "check_command", 0,
	   0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
	   0, "failure_prediction_options", 0, 0, "notes", "notes_url",
	   "action_url", "icon_image", "icon_image_alt", "vrml_image",
	   "statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  add_host("host_name_2", "host_display_name", "host_alias",
	   "localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
	   0.0, 0.0, "notification_period", 0, "check_command", 0,
	   0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
	   0, "failure_prediction_options", 0, 0, "notes", "notes_url",
	   "action_url", "icon_image", "icon_image_alt", "vrml_image",
	   "statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  add_host("host_name_3", "host_display_name", "host_alias",
	   "localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
	   0.0, 0.0, "notification_period", 0, "check_command", 0,
	   0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
	   0, "failure_prediction_options", 0, 0, "notes", "notes_url",
	   "action_url", "icon_image", "icon_image_alt", "vrml_image",
	   "statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);

  if (remove_host_by_id("host_name_1") != 1
      || remove_host_by_id("host_name_2") != 1
      || remove_host_by_id("host_name_3") != 1
      || host_list != NULL
      || host_list_tail != NULL)
    throw ("remove all host failed.");

  free_object_skiplists();
}

/**
 *  Check if remove host works with invalid call.
 */
static void remove_host_failed() {
  init_object_skiplists();

  if (remove_host_by_id("") == 1)
    throw (engine_error() << "host remove but dosen't exist.");
  if (remove_host_by_id(NULL) == 1)
    throw (engine_error() << "host remove but pointer is NULL.");

  free_object_skiplists();
}

/**
 *  Check if remove host works with some contactgroups.
 */
static void remove_host_with_contactgroups() {
  init_object_skiplists();

  host* hst = add_host("host_name_1", "host_display_name", "host_alias",
		       "localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
		       0.0, 0.0, "notification_period", 0, "check_command", 0,
		       0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
		       0, "failure_prediction_options", 0, 0, "notes", "notes_url",
		       "action_url", "icon_image", "icon_image_alt", "vrml_image",
		       "statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  contactgroup* cgroup = add_contactgroup("contactgroup_name", "contactgroup_alias");
  contactgroupsmember* cgm = add_contactgroup_to_host(hst, "contactgroup_name");
  cgm->group_ptr = cgroup;

  if (remove_host_by_id("host_name_1") != 1
      || host_list != NULL
      || host_list_tail != NULL)
    throw (engine_error() << "remove host with contact groups failed.");

  delete[] cgroup->group_name;
  delete[] cgroup->alias;
  delete cgroup;

  free_object_skiplists();
}

/**
 *  Check if remove host works with some contacts.
 */
static void remove_host_with_contacts() {
  init_object_skiplists();

  host* hst = add_host("host_name_1", "host_display_name", "host_alias",
		       "localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
		       0.0, 0.0, "notification_period", 0, "check_command", 0,
		       0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
		       0, "failure_prediction_options", 0, 0, "notes", "notes_url",
		       "action_url", "icon_image", "icon_image_alt", "vrml_image",
		       "statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  contact* cntct = add_contact("contact_name", NULL, NULL, NULL, NULL, NULL,
			       NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  contactsmember* cm = add_contact_to_host(hst, "contact_name");
  cm->contact_ptr = cntct;

  if (remove_host_by_id("host_name_1") != 1
      || host_list != NULL
      || host_list_tail != NULL)
    throw (engine_error() << "remove host with contacts failed.");

  delete[] cntct->name;
  delete[] cntct->alias;
  delete cntct;

  free_object_skiplists();
}

/**
 *  Check if remove host works with some customvariables.
 */
static void remove_host_with_customvariables() {
  init_object_skiplists();

  host* hst = add_host("host_name_1", "host_display_name", "host_alias",
		       "localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
		       0.0, 0.0, "notification_period", 0, "check_command", 0,
		       0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
		       0, "failure_prediction_options", 0, 0, "notes", "notes_url",
		       "action_url", "icon_image", "icon_image_alt", "vrml_image",
		       "statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  add_custom_variable_to_host(hst, "varname", "varvalue");

  if (remove_host_by_id("host_name_1") != 1
      || host_list != NULL
      || host_list_tail != NULL)
    throw (engine_error() << "remove host with custom variables failed.");

  free_object_skiplists();
}

/**
 *  Check if remove host works with some hostescalation.
 */
static void remove_host_with_hostescalation() {
  init_object_skiplists();

  host* hst = add_host("host_name_1", "host_display_name", "host_alias",
		       "localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
		       0.0, 0.0, "notification_period", 0, "check_command", 0,
		       0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
		       0, "failure_prediction_options", 0, 0, "notes", "notes_url",
		       "action_url", "icon_image", "icon_image_alt", "vrml_image",
		       "statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  hostescalation* he = add_host_escalation("hostescalation_host_name_1",
					  0, 0, 0.0,
					  "hostescalation_escalation_period",
					  0, 0, 0);
  he->host_ptr = hst;

  if (remove_host_by_id("host_name_1") != 1
      || host_list != NULL
      || host_list_tail != NULL)
    throw (engine_error() << "remove host with hostescalation failed.");

  free_object_skiplists();
}

/**
 *  Check if remove host works with some hostdependency.
 */
static void remove_host_with_hostdependency() {
  init_object_skiplists();

  host* hst = add_host("host_name_1", "host_display_name", "host_alias",
		       "localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
		       0.0, 0.0, "notification_period", 0, "check_command", 0,
		       0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
		       0, "failure_prediction_options", 0, 0, "notes", "notes_url",
		       "action_url", "icon_image", "icon_image_alt", "vrml_image",
		       "statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  hostdependency* sd = add_host_dependency("host_dependency_dependent_host_name_3",
					   "host_dependency_host_name_3",
					   0, 0, 0, 0, 0, 0,
					   "host_dependency_dependency_period");
  sd->master_host_ptr = hst;

  if (remove_host_by_id("host_name_1") != 1
      || host_list != NULL
      || host_list_tail != NULL)
    throw (engine_error() << "remove host with hostdependency (master) failed.");

  hst = add_host("host_name_1", "host_display_name", "host_alias",
		 "localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
		 0.0, 0.0, "notification_period", 0, "check_command", 0,
		 0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
		 0, "failure_prediction_options", 0, 0, "notes", "notes_url",
		 "action_url", "icon_image", "icon_image_alt", "vrml_image",
		 "statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  sd = add_host_dependency("host_dependency_dependent_host_name_3",
			   "host_dependency_host_name_3",
			   0, 0, 0, 0, 0, 0,
			   "host_dependency_dependency_period");
  sd->dependent_host_ptr = hst;

  if (remove_host_by_id("host_name_1") != 1
      || host_list != NULL
      || host_list_tail != NULL)
    throw (engine_error() << "remove host with hostdependency (dependency) failed.");

  free_object_skiplists();
}

/**
 *  Check if remove host works with some service.
 */
static void remove_host_with_service() {
  init_object_skiplists();

  host* hst = add_host("host_name_1", "host_display_name", "host_alias",
		       "localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
		       0.0, 0.0, "notification_period", 0, "check_command", 0,
		       0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
		       0, "failure_prediction_options", 0, 0, "notes", "notes_url",
		       "action_url", "icon_image", "icon_image_alt", "vrml_image",
		       "statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  service* svc = add_service("service_host_name", "service_host_description", NULL,
			     NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL, 0, 0, 0, 0,
			     0, 0, 0, 0, NULL, 0, "check_command", 0, 0, 0.0, 0.0, 0,
			     0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL,
			     NULL, NULL, 0, 0, 0);
  add_service_link_to_host(hst, svc);

  if (remove_host_by_id("host_name_1") != 1
      || host_list != NULL
      || host_list_tail != NULL)
    throw (engine_error() << "remove host with service failed.");

  free_object_skiplists();
}

/**
 *  Check if remove host works with some host parent.
 */
static void remove_host_with_host_parent() {
  init_object_skiplists();

  host* hst = add_host("host_name_1", "host_display_name", "host_alias",
		       "localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
		       0.0, 0.0, "notification_period", 0, "check_command", 0,
		       0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
		       0, "failure_prediction_options", 0, 0, "notes", "notes_url",
		       "action_url", "icon_image", "icon_image_alt", "vrml_image",
		       "statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  add_parent_host_to_host(hst, "host_name_1");

  if (remove_host_by_id("host_name_1") != 1
      || host_list != NULL
      || host_list_tail != NULL)
    throw (engine_error() << "remove host with host parent failed.");

  free_object_skiplists();
}

/**
 *  Check if remove host works with some host child.
 */
static void remove_host_with_host_child() {
  init_object_skiplists();

  host* hst1 = add_host("host_name_1", "host_display_name", "host_alias",
			"localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
			0.0, 0.0, "notification_period", 0, "check_command", 0,
			0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
			0, "failure_prediction_options", 0, 0, "notes", "notes_url",
			"action_url", "icon_image", "icon_image_alt", "vrml_image",
			"statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  host* hst2 = add_host("host_name_2", "host_display_name", "host_alias",
			"localhost", "address", 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
			0.0, 0.0, "notification_period", 0, "check_command", 0,
			0, "event_handler", 0, 0, 0.0, 0.0, 0, 0, 0, 0, 0, 0, 0,
			0, "failure_prediction_options", 0, 0, "notes", "notes_url",
			"action_url", "icon_image", "icon_image_alt", "vrml_image",
			"statusmap_image", 0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0, 0, 0);
  add_child_link_to_host(hst1, hst2);

  if (remove_host_by_id("host_name_1") != 1
      || remove_host_by_id("host_name_2") != 1
      || host_list != NULL
      || host_list_tail != NULL)
    throw (engine_error() << "remove host with host parent failed.");

  free_object_skiplists();
}

/**
 *  Check if remove host works with.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;
  try {
    // Tests.
    remove_all_host();
    remove_host_failed();
    remove_host_with_contactgroups();
    remove_host_with_contacts();
    remove_host_with_customvariables();
    remove_host_with_hostescalation();
    remove_host_with_hostdependency();
    remove_host_with_service();
    remove_host_with_host_parent();
    remove_host_with_host_child();
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

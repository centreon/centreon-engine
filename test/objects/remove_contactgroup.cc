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
 *  Reset object list.
 */
static void reset_list() {
  host_list = NULL;
  host_list_tail = NULL;
  service_list = NULL;
  service_list_tail = NULL;
  serviceescalation_list = NULL;
  serviceescalation_list_tail = NULL;
  hostescalation_list = NULL;
  hostescalation_list_tail = NULL;
}

/**
 *  Check if remove contactgroup works with some contactgroups.
 */
static void remove_all_contactgroups() {
  reset_list();
  init_object_skiplists();

  add_contactgroup("contactgroup_name_1", "contactgroup_alias");
  add_contactgroup("contactgroup_name_2", "contactgroup_alias");
  add_contactgroup("contactgroup_name_3", "contactgroup_alias");

  if (remove_contactgroup_by_id("contactgroup_name_2") != 1
      || remove_contactgroup_by_id("contactgroup_name_3") != 1
      || remove_contactgroup_by_id("contactgroup_name_1") != 1
      || contactgroup_list != NULL
      || contactgroup_list_tail != NULL)
    throw (engine_error() << "remove all contactgroups failed.");

  free_object_skiplists();
}

/**
 *  Check if remove contactgroup works with invalid call.
 */
static void remove_contactgroup_failed() {
  init_object_skiplists();

  if (remove_contactgroup_by_id("") == 1)
    throw (engine_error() << "contactgroup remove but dosen't exist.");
  if (remove_contactgroup_by_id(NULL) == 1)
    throw (engine_error() << "contactgroup remove but pointer is NULL.");

  free_object_skiplists();
}

/**
 *  Check if remove contactgroup works with some host.
 */
static void remove_contactgroup_with_host() {
  reset_list();
  init_object_skiplists();

  contactgroup* cgroup = add_contactgroup("contactgroup_name", "contactgroup_alias");

  host* hst = add_host("host_name_1", "host_display_name", "host_alias",
  		       "localhost", NULL, 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
  		       0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0, 0, 0.0,
  		       0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL,
  		       NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0,
  		       0.0, 0.0, 0.0, 0, 0, 0, 0, 0);

  add_contactgroup_to_host(hst, "contactgroup_name");
  hst->contact_groups->group_ptr = cgroup;

  if (remove_contactgroup_by_id("contactgroup_name") != 1
      || contactgroup_list != NULL
      || contactgroup_list_tail != NULL)
    throw (engine_error() << "remove contactgroup with host failed.");

  delete[] hst->plugin_output;
  delete[] hst->long_plugin_output;
  delete[] hst->perf_data;
  delete[] hst->statusmap_image;
  delete[] hst->vrml_image;
  delete[] hst->icon_image_alt;
  delete[] hst->icon_image;
  delete[] hst->action_url;
  delete[] hst->notes_url;
  delete[] hst->notes;
  delete[] hst->failure_prediction_options;
  delete[] hst->event_handler;
  delete[] hst->host_check_command;
  delete[] hst->notification_period;
  delete[] hst->check_period;
  delete[] hst->address;
  delete[] hst->alias;
  delete[] hst->display_name;
  delete[] hst->name;
  delete hst;

  free_object_skiplists();
}

/**
 *  Check if remove contactgroup works with some service.
 */
static void remove_contactgroup_with_service() {
  reset_list();
  init_object_skiplists();

  contactgroup* cgroup = add_contactgroup("contactgroup_name", "contactgroup_alias");

  service* svc = add_service("service_host_name", "service_host_description", NULL,
			     NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL, 0, 0, 0, 0,
			     0, 0, 0, 0, NULL, 0, "check_command", 0, 0, 0.0, 0.0, 0,
			     0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL,
			     NULL, NULL, 0, 0, 0);

  add_contactgroup_to_service(svc, "contactgroup_name");
  svc->contact_groups->group_ptr = cgroup;

  if (remove_contactgroup_by_id("contactgroup_name") != 1
      || contactgroup_list != NULL
      || contactgroup_list_tail != NULL)
    throw (engine_error() << "remove contactgroup with host failed.");

  delete[] svc->perf_data;
  delete[] svc->plugin_output;
  delete[] svc->long_plugin_output;
  delete[] svc->failure_prediction_options;
  delete[] svc->notification_period;
  delete[] svc->event_handler;
  delete[] svc->service_check_command;
  delete[] svc->display_name;
  delete[] svc->description;
  delete[] svc->host_name;
  delete svc;

  free_object_skiplists();
}

/**
 *  Check if remove contactgroup works with some hostescalation.
 */
static void remove_contactgroup_with_hostescalation() {
  reset_list();
  init_object_skiplists();

  contactgroup* cgroup = add_contactgroup("contactgroup_name", "contactgroup_alias");

  hostescalation* he = add_host_escalation("host_name", 0, 0, 0.0, NULL, 0, 0, 0);
  add_contactgroup_to_host_escalation(he, "contactgroup_name");
  he->contact_groups->group_ptr = cgroup;

  if (remove_contactgroup_by_id("contactgroup_name") != 1
      || contactgroup_list != NULL
      || contactgroup_list_tail != NULL)
    throw (engine_error() << "remove contactgroup with hostescalation failed.");

  delete[] he->host_name;
  delete[] he->escalation_period;
  delete he;

  free_object_skiplists();
}

/**
 *  Check if remove contactgroup works with some serviceescalation.
 */
static void remove_contactgroup_with_serviceescalation() {
  reset_list();
  init_object_skiplists();

  contactgroup* cgroup = add_contactgroup("contactgroup_name", "contactgroup_alias");

  serviceescalation* se = add_service_escalation("service_name", "service_description",
						0, 0, 0.0, NULL, 0, 0, 0, 0);
  add_contactgroup_to_serviceescalation(se, "contactgroup_name");
  se->contact_groups->group_ptr = cgroup;

  if (remove_contactgroup_by_id("contactgroup_name") != 1
      || contactgroup_list != NULL
      || contactgroup_list_tail != NULL)
    throw (engine_error() << "remove contactgroup with serviceescalation failed.");

  delete[] se->host_name;
  delete[] se->escalation_period;
  delete[] se->description;
  delete se;

  free_object_skiplists();
}

/**
 *  Check if remove contactgroup works.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;
  try {
    // Tests.
    remove_all_contactgroups();
    remove_contactgroup_failed();
    remove_contactgroup_with_host();
    remove_contactgroup_with_service();
    remove_contactgroup_with_hostescalation();
    remove_contactgroup_with_serviceescalation();
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

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

#include <exception>
#include <QDebug>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon::engine;

/**
 *  Cleanup service memory.
 */
static void free_service(service* svc) {
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
}

/**
 *  Check if remove servicegroup works with some servicegroup.
 */
static void remove_all_servicegroup() {
  init_object_skiplists();

  add_servicegroup("servicegroup_name_1",
		   "servicegroup_alias",
		   "servicegroup_notes",
		   "servicegroup_notes_url",
		   "servicegroup_action_url");

  add_servicegroup("servicegroup_name_2",
		   "servicegroup_alias",
		   "servicegroup_notes",
		   "servicegroup_notes_url",
		   "servicegroup_action_url");

  add_servicegroup("servicegroup_name_3",
		   "servicegroup_alias",
		   "servicegroup_notes",
		   "servicegroup_notes_url",
		   "servicegroup_action_url");

  if (remove_servicegroup_by_id("servicegroup_name_2") != 1
      || remove_servicegroup_by_id("servicegroup_name_1") != 1
      || remove_servicegroup_by_id("servicegroup_name_3") != 1
      || servicegroup_list != NULL
      || servicegroup_list_tail != NULL)
    throw (engine_error() << "remove all servicegroup failed.");

  free_object_skiplists();
}

/**
 *  Check the return value with invalid call.
 */
static void remove_servicegroup_failed() {
  init_object_skiplists();

  if (remove_servicegroup_by_id("") == 1)
    throw (engine_error() << "servicegroup remove but dosen't exist.");
  if (remove_servicegroup_by_id(NULL) == 1)
    throw (engine_error() << "servicegroup remove but pointer is NULL.");

  free_object_skiplists();
}

/**
 *  Check if remove servicegroup works with some services.
 */
static void remove_servicegroup_with_services() {
  init_object_skiplists();

  service* svc1 = add_service("service_host_name_1", "service_host_description", NULL,
			     NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL, 0, 0, 0, 0,
			     0, 0, 0, 0, NULL, 0, "check_command", 0, 0, 0.0, 0.0, 0,
			     0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL,
			     NULL, NULL, 0, 0, 0);
  service* svc2 = add_service("service_host_name_2", "service_host_description", NULL,
			     NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL, 0, 0, 0, 0,
			     0, 0, 0, 0, NULL, 0, "check_command", 0, 0, 0.0, 0.0, 0,
			     0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL,
			     NULL, NULL, 0, 0, 0);
  service* svc3 = add_service("service_host_name_3", "service_host_description", NULL,
			     NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL, 0, 0, 0, 0,
			     0, 0, 0, 0, NULL, 0, "check_command", 0, 0, 0.0, 0.0, 0,
			     0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL,
			     NULL, NULL, 0, 0, 0);

  servicegroup* sg = add_servicegroup("servicegroup_name",
				      "servicegroup_alias",
				      "servicegroup_notes",
				      "servicegroup_notes_url",
				      "servicegroup_action_url");

  add_service_to_servicegroup(sg, "service_host_name_1", "service_host_description");
  add_service_to_servicegroup(sg, "service_host_name_2", "service_host_description");
  add_service_to_servicegroup(sg, "service_host_name_3", "service_host_description");

  sg->members->service_ptr = svc1;
  sg->members->next->service_ptr = svc2;
  sg->members->next->next->service_ptr = svc3;

  if (remove_servicegroup_by_id("servicegroup_name") != 1
      || servicegroup_list != NULL
      || servicegroup_list_tail != NULL)
    throw (engine_error() << "remove servicegroup with servicesfailed.");

  free_service(svc1);
  free_service(svc2);
  free_service(svc3);
  free_object_skiplists();
}

/**
 *  Check if the remove service groups works.
 *  For better check execute this test with valgrind.
 */
int main(void) {
  try {
    remove_all_servicegroup();
    remove_servicegroup_failed();
    remove_servicegroup_with_services();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    free_memory(get_global_macros());
    return (1);
  }
  return (0);
}

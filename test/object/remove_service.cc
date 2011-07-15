/*
** Copyright 2011 Merethis
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

#include <QDebug>
#include <exception>
#include "error.hh"
#include "objects.hh"
#include "utils.hh"
#include "macros.hh"
#include "globals.hh"

using namespace com::centreon::engine;

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

  if (remove_service_by_id("service_host_name_1", "service_description") != 1)
    throw (engine_error() << "remove service with contact groups failed.");

  delete[] cgroup->group_name;
  delete[] cgroup->alias;
  delete cgroup;

  free_object_skiplists();
}

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

  if (remove_service_by_id("service_host_name_1", "service_description") != 1)
    throw (engine_error() << "remove service with contact groups failed.");

  delete[] cntct->name;
  delete[] cntct->alias;
  delete cntct;

  free_object_skiplists();
}

static void remove_service_with_customvaraiables() {
  init_object_skiplists();

  // service* svc = add_service("service_host_name_1", "service_description", "display_name",
  // 			     "check_period", 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, "notification_period",
  // 			     0, 0, 0, 0, 0, 0, 0, 0, "event_handler", 0, "check_command", 0, 0,
  // 			     0.0, 0.0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "failure_prediction_options",
  // 			     0, 0, "notes", "notes_url", "action_url", "icon_image", "icon_image_alt",
  // 			     0, 0, 0);
  // contact* cntct = add_contact("contact_name", NULL, NULL, NULL, NULL, NULL,
  // 			       NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  // contactsmember* cm = add_contact_to_service(svc, "contact_name");
  // cm->contact_ptr = cntct;

  // if (remove_service_by_id("service_host_name_1", "service_description") != 1)
  //   throw (engine_error() << "remove service with contact groups failed.");

  // delete[] cntct->name;
  // delete[] cntct->alias;
  // delete cntct;

  free_object_skiplists();
}

int main(void) {
  try {
    remove_all_service();
    remove_service_failed();
    remove_service_with_contactgroups();
    remove_service_with_contacts();
    remove_service_with_customvaraiables();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    free_memory(get_global_macros());
    return (1);
  }
  return (0);
}

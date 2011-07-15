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

static contact* create_contact(char const* name) {
  char* addr[MAX_CONTACT_ADDRESSES];;
  for (unsigned int i = 0; i < MAX_CONTACT_ADDRESSES; ++i)
    addr[i] = const_cast<char*>("addresse");

  return (add_contact(name, "contact_alias", "contact_email",
		      "contact_pager", addr, "contact_svc_notification",
		      "host_notification_period", 0, 0, 0, 0, 0, 0, 0, 0,
		      0, 0, 0, 0, 0, 0, 0, 0));
}

static void free_contactgroup(contactgroup* group) {
  delete[] group->group_name;
  delete[] group->alias;
  delete group;
}

static void reset_list() {
  host_list = NULL;
  host_list_tail = NULL;
  service_list = NULL;
  service_list_tail = NULL;
  contactgroup_list = NULL;
  contactgroup_list_tail = NULL;
  serviceescalation_list = NULL;
  serviceescalation_list_tail = NULL;
  hostescalation_list = NULL;
  hostescalation_list_tail = NULL;
}

static void remove_all_contacts() {
  reset_list();
  init_object_skiplists();

  create_contact("contact_name_1");
  create_contact("contact_name_2");
  create_contact("contact_name_3");

  if (remove_contact_by_id("contact_name_2") != 1
      || remove_contact_by_id("contact_name_3") != 1
      || remove_contact_by_id("contact_name_1") != 1
      || contact_list != NULL
      || contact_list_tail != NULL)
    throw (engine_error() << "remove all contacts failed.");

  free_object_skiplists();
}

static void remove_contact_failed() {
  init_object_skiplists();

  if (remove_contact_by_id("") == 1)
    throw (engine_error() << "contact remove but dosen't exist.");
  if (remove_contact_by_id(NULL) == 1)
    throw (engine_error() << "contact remove but pointer is NULL.");

  free_object_skiplists();
}

static void remove_contact_with_contactgroup() {
  reset_list();
  init_object_skiplists();

  contact* cntct = create_contact("contact_name_1");
  contactgroup* cgroup = add_contactgroup("contactgroup_name", "contactgroup_alias");
  add_contact_to_contactgroup(cgroup, "contact_name_1");
  cgroup->members->contact_ptr = cntct;

  if (remove_contact_by_id("contact_name_1") != 1
      || contact_list != NULL
      || contact_list_tail != NULL)
    throw (engine_error() << "remove contact with contactgroup failed.");

  free_contactgroup(cgroup);
  free_object_skiplists();
}

static void remove_contact_with_host() {
  reset_list();
  init_object_skiplists();

  contact* cntct = create_contact("contact_name_1");
  contactgroup* cgroup = add_contactgroup("contactgroup_name", "contactgroup_alias");
  add_contact_to_contactgroup(cgroup, "contact_name_1");
  cgroup->members->contact_ptr = cntct;

  host* hst = add_host("host_name_1", "host_display_name", "host_alias",
  		       "localhost", NULL, 0, 0.0, 0.0, 42, 0, 0, 0, 0, 0,
  		       0.0, 0.0, NULL, 0, NULL, 0, 0, NULL, 0, 0, 0.0,
  		       0.0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL,
  		       NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0,
  		       0.0, 0.0, 0.0, 0, 0, 0, 0, 0);

  contactgroupsmember* cgm = add_contactgroup_to_host(hst, "contactgroup_name");
  add_contact_to_host(hst, "contact_name_1");

  hst->contacts->contact_ptr = cntct;
  hst->contact_groups->group_ptr = cgroup;

  if (remove_contact_by_id("contact_name_1") != 1
      || contact_list != NULL
      || contact_list_tail != NULL)
    throw (engine_error() << "remove contact with host failed.");

  delete[] cgm->group_name;
  delete cgm;

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

  free_contactgroup(cgroup);
  free_object_skiplists();
}

static void remove_contact_with_service() {
  reset_list();
  init_object_skiplists();

  contact* cntct = create_contact("contact_name_1");
  contactgroup* cgroup = add_contactgroup("contactgroup_name", "contactgroup_alias");
  add_contact_to_contactgroup(cgroup, "contact_name_1");
  cgroup->members->contact_ptr = cntct;

  service* svc = add_service("service_host_name", "service_host_description", NULL,
			     NULL, 0, 42, 0, 0, 0, 42.0, 0.0, 0.0, NULL, 0, 0, 0, 0,
			     0, 0, 0, 0, NULL, 0, "check_command", 0, 0, 0.0, 0.0, 0,
			     0, 0, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, NULL, NULL, NULL,
			     NULL, NULL, 0, 0, 0);

  contactgroupsmember* cgm = add_contactgroup_to_service(svc, "contactgroup_name");
  add_contact_to_service(svc, "contact_name_1");

  svc->contacts->contact_ptr = cntct;
  svc->contact_groups->group_ptr = cgroup;

  if (remove_contact_by_id("contact_name_1") != 1
      || contact_list != NULL
      || contact_list_tail != NULL)
    throw (engine_error() << "remove contact with host failed.");

  delete[] cgm->group_name;
  delete cgm;

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

  free_contactgroup(cgroup);
  free_object_skiplists();
}

static void remove_contact_with_hostescalation() {
  reset_list();
  init_object_skiplists();

  contact* cntct = create_contact("contact_name_1");
  contactgroup* cgroup = add_contactgroup("contactgroup_name", "contactgroup_alias");
  add_contact_to_contactgroup(cgroup, "contact_name_1");
  cgroup->members->contact_ptr = cntct;

  hostescalation* he = add_hostescalation("host_name", 0, 0, 0.0, NULL, 0, 0, 0);
  contactgroupsmember* cgm = add_contactgroup_to_hostescalation(he, "contactgroup_name");
  add_contact_to_hostescalation(he, "contact_name_1");

  he->contacts->contact_ptr = cntct;
  he->contact_groups->group_ptr = cgroup;

  if (remove_contact_by_id("contact_name_1") != 1
      || contact_list != NULL
      || contact_list_tail != NULL)
    throw (engine_error() << "remove contact with hostescalation failed.");

  delete[] he->host_name;
  delete[] he->escalation_period;
  delete he;

  delete[] cgm->group_name;
  delete cgm;

  free_contactgroup(cgroup);
  free_object_skiplists();
}

static void remove_contact_with_serviceescalation() {
  reset_list();
  init_object_skiplists();

  contact* cntct = create_contact("contact_name_1");
  contactgroup* cgroup = add_contactgroup("contactgroup_name", "contactgroup_alias");
  add_contact_to_contactgroup(cgroup, "contact_name_1");
  cgroup->members->contact_ptr = cntct;

  serviceescalation* se = add_serviceescalation("service_name", "service_description",
						0, 0, 0.0, NULL, 0, 0, 0, 0);
  contactgroupsmember* cgm = add_contactgroup_to_serviceescalation(se, "contactgroup_name");
  add_contact_to_serviceescalation(se, "contact_name_1");

  se->contacts->contact_ptr = cntct;
  se->contact_groups->group_ptr = cgroup;

  if (remove_contact_by_id("contact_name_1") != 1
      || contact_list != NULL
      || contact_list_tail != NULL)
    throw (engine_error() << "remove contact with serviceescalation failed.");

  delete[] se->host_name;
  delete[] se->escalation_period;
  delete[] se->description;
  delete se;

  delete[] cgm->group_name;
  delete cgm;

  free_contactgroup(cgroup);
  free_object_skiplists();
}

static void remove_contact_with_command() {
  reset_list();
  init_object_skiplists();

  contact* cntct = create_contact("contact_name_1");
  command* cmd = add_command("command_name_1", "command_value");
  commandsmember* hmember = add_host_notification_command_to_contact(cntct, "command_name_1");
  commandsmember* smember = add_service_notification_command_to_contact(cntct, "command_name_1");
  hmember->command_ptr = cmd;
  smember->command_ptr = cmd;

  if (remove_contact_by_id("contact_name_1") != 1
      || contact_list != NULL
      || contact_list_tail != NULL)
    throw (engine_error() << "remove contact with serviceescalation failed.");

  delete[] cmd->command_line;
  delete[] cmd->name;
  delete cmd;

  free_object_skiplists();
}

static void remove_contact_with_customvariable() {
  reset_list();
  init_object_skiplists();

  contact* cntct = create_contact("contact_name_1");
  add_custom_variable_to_contact(cntct, "varname", "varvalue");

  if (remove_contact_by_id("contact_name_1") != 1
      || contact_list != NULL
      || contact_list_tail != NULL)
    throw (engine_error() << "remove contact with serviceescalation failed.");

  free_object_skiplists();
}

int main(void) {
  try {
    remove_all_contacts();
    remove_contact_failed();
    remove_contact_with_contactgroup();
    remove_contact_with_host();
    remove_contact_with_service();
    remove_contact_with_hostescalation();
    remove_contact_with_serviceescalation();
    remove_contact_with_command();
    remove_contact_with_customvariable();
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    free_memory(get_global_macros());
    return (1);
  }
  return (0);
}

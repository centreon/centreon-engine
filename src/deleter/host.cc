/*
** Copyright 2011-2019 Centreon
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

#include "com/centreon/engine/deleter/contactsmember.hh"
#include "com/centreon/engine/deleter/customvariablesmember.hh"
#include "com/centreon/engine/deleter/host.hh"
#include "com/centreon/engine/deleter/hostsmember.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/deleter/servicesmember.hh"
#include "com/centreon/engine/objects/contactsmember.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"
#include "com/centreon/engine/objects/host.hh"
#include "com/centreon/engine/objects/hostsmember.hh"
#include "com/centreon/engine/objects/objectlist.hh"
#include "com/centreon/engine/objects/servicesmember.hh"

using namespace com::centreon::engine;

/**
 *  Delete host.
 *
 *  @param[in] ptr The host to delete.
 */
void deleter::host(void* ptr) throw () {
  if (!ptr)
    return;

  host_struct* obj(static_cast<host_struct*>(ptr));

    // Add contact groups to host.
  obj->contact_groups.clear();
  listmember(obj->parent_hosts, &hostsmember);
  listmember(obj->child_hosts, &hostsmember);
  listmember(obj->services, &servicesmember);
  listmember(obj->contacts, &contactsmember);
  listmember(obj->custom_variables, &customvariablesmember);
  listmember(obj->hostgroups_ptr, &objectlist);

  delete[] obj->name;
  obj->name = NULL;
  delete[] obj->display_name;
  obj->display_name = NULL;
  delete[] obj->alias;
  obj->alias = NULL;
  delete[] obj->address;
  obj->address = NULL;
  delete[] obj->host_check_command;
  obj->host_check_command = NULL;
  delete[] obj->event_handler;
  obj->event_handler = NULL;
  delete[] obj->notification_period;
  obj->notification_period = NULL;
  delete[] obj->check_period;
  obj->check_period = NULL;
  delete[] obj->failure_prediction_options;
  obj->failure_prediction_options = NULL;
  delete[] obj->notes;
  obj->notes = NULL;
  delete[] obj->notes_url;
  obj->notes_url = NULL;
  delete[] obj->action_url;
  obj->action_url = NULL;
  delete[] obj->icon_image;
  obj->icon_image = NULL;
  delete[] obj->icon_image_alt;
  obj->icon_image_alt = NULL;
  delete[] obj->vrml_image;
  obj->vrml_image = NULL;
  delete[] obj->statusmap_image;
  obj->statusmap_image = NULL;
  delete[] obj->plugin_output;
  obj->plugin_output = NULL;
  delete[] obj->long_plugin_output;
  obj->long_plugin_output = NULL;
  delete[] obj->perf_data;
  obj->perf_data = NULL;

  // event_handler_ptr not free.
  // check_command_ptr not free.
  // check_period_ptr not free.
  // notification_period_ptr not free.

  delete obj;
}

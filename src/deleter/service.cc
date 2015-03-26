/*
** Copyright 2011-2015 Merethis
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

#include "com/centreon/engine/deleter/customvariablesmember.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/deleter/service.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"
#include "com/centreon/engine/objects/objectlist.hh"
#include "com/centreon/engine/objects/service.hh"

using namespace com::centreon::engine;

/**
 *  Delete service.
 *
 *  @param[in] ptr The service to delete.
 */
void deleter::service(void* ptr) throw () {
  if (!ptr)
    return;

  service_struct* obj(static_cast<service_struct*>(ptr));

  listmember(obj->custom_variables, &customvariablesmember);
  listmember(obj->servicegroups_ptr, &objectlist);

  delete[] obj->host_name;
  obj->host_name = NULL;
  delete[] obj->description;
  obj->description = NULL;
  delete[] obj->display_name;
  obj->display_name = NULL;
  delete[] obj->service_check_command;
  obj->service_check_command = NULL;
  delete[] obj->event_handler;
  obj->event_handler = NULL;
  delete[] obj->check_period;
  obj->check_period = NULL;
  delete[] obj->plugin_output;
  obj->plugin_output = NULL;
  delete[] obj->long_plugin_output;
  obj->long_plugin_output = NULL;
  delete[] obj->perf_data;
  obj->perf_data = NULL;
  delete[] obj->event_handler_args;
  obj->event_handler_args = NULL;
  delete[] obj->check_command_args;
  obj->check_command_args = NULL;

  // host_ptr not free.
  // event_handler_ptr not free.
  // check_command_ptr not free.
  // check_period_ptr not free.

  delete obj;
}

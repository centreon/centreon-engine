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

#include "free_object.hh"

using namespace com::centreon::engine;

contactsmember const* modules::free_contactsmember(contactsmember const* member) {
  if (member == NULL)
    return (NULL);

  contactsmember const* next = member->next;
  delete[] member->contact_name;
  delete member;
  return (next);
}

contactgroupsmember const* modules::free_contactgroupsmember(contactgroupsmember const* member) {
  if (member == NULL)
    return (NULL);

  contactgroupsmember const* next = member->next;
  delete[] member->group_name;
  delete member;
  return (next);
}

hostsmember const* modules::free_hostsmember(hostsmember const* member) {
  if (member == NULL)
    return (NULL);

  hostsmember const* next = member->next;
  delete[] member->host_name;
  delete member;
  return (next);
}

servicesmember const* modules::free_servicesmember(servicesmember const* member) {
  if (member == NULL)
    return (NULL);

  servicesmember const* next = member->next;
  delete[] member->host_name;
  delete[] member->service_description;
  delete member;
  return (next);
}

commandsmember const* modules::free_commandsmember(commandsmember const* member) {
  if (member == NULL)
    return (NULL);

  commandsmember const* next = member->next;
  delete[] member->cmd;
  delete member;
  return (next);
}

customvariablesmember const* modules::free_customvariablesmember(customvariablesmember const* member) {
  if (member == NULL)
    return (NULL);

  customvariablesmember const* next = member->next;
  delete[] member->variable_name;
  delete[] member->variable_value;
  delete member;
  return (next);
}

void modules::free_objectlist(objectlist const* objlist) {
  if (objlist == NULL)
    return;

  while (objlist) {
    objectlist const* tmp = objlist->next;
    delete objlist;
    objlist = tmp;
  }
}

void modules::free_contactgroup(contactgroup const* group) {
  if (group == NULL)
    return;

  contactsmember const* member = group->members;
  while ((member = free_contactsmember(member)));

  delete[] group->group_name;
  delete[] group->alias;
  delete group;
}

void modules::free_hostgroup(hostgroup const* group) {
  if (group == NULL)
    return;

  hostsmember const* member = group->members;
  while ((member = free_hostsmember(member)));

  delete[] group->group_name;
  delete[] group->alias;
  delete[] group->notes;
  delete[] group->notes_url;
  delete[] group->action_url;
  delete group;
}

void modules::free_servicegroup(servicegroup const* group) {
  if (group == NULL)
    return;

  servicesmember const* member = group->members;
  while ((member = free_servicesmember(member)));

  delete[] group->group_name;
  delete[] group->alias;
  delete[] group->notes;
  delete[] group->notes_url;
  delete[] group->action_url;
  delete group;
}

void modules::free_host(host const* hst) {
  if (hst == NULL)
    return;

  hostsmember const* hstmember = hst->parent_hosts;
  while ((hstmember = free_hostsmember(hstmember)));

  hstmember = hst->child_hosts;
  while ((hstmember = free_hostsmember(hstmember)));

  servicesmember const* svcmember = hst->services;
  while ((svcmember = free_servicesmember(svcmember)));

  contactgroupsmember const* cgmember = hst->contact_groups;
  while ((cgmember = free_contactgroupsmember(cgmember)));

  contactsmember const* cntctmember = hst->contacts;
  while ((cntctmember = free_contactsmember(cntctmember)));

  customvariablesmember const* varmember = hst->custom_variables;
  while ((varmember = free_customvariablesmember(varmember)));

  free_objectlist(hst->hostgroups_ptr);

  delete[] hst->name;
  delete[] hst->display_name;
  delete[] hst->alias;
  delete[] hst->address;
  delete[] hst->host_check_command;
  delete[] hst->event_handler;
  delete[] hst->notification_period;
  delete[] hst->check_period;
  delete[] hst->failure_prediction_options;
  delete[] hst->notes;
  delete[] hst->notes_url;
  delete[] hst->action_url;
  delete[] hst->icon_image;
  delete[] hst->icon_image_alt;
  delete[] hst->vrml_image;
  delete[] hst->statusmap_image;
  delete[] hst->plugin_output;
  delete[] hst->long_plugin_output;
  delete[] hst->perf_data;

  // XXX: event_handler_ptr;
  // XXX: check_command_ptr;
  // XXX: check_period_ptr;
  // XXX: notification_period_ptr;

  delete hst;
}

void modules::free_service(service const* svc) {
  if (svc == NULL)
    return;

  contactgroupsmember const* cgmember = svc->contact_groups;
  while ((cgmember = free_contactgroupsmember(cgmember)));

  contactsmember const* cntctmember = svc->contacts;
  while ((cntctmember = free_contactsmember(cntctmember)));

  customvariablesmember const* varmember = svc->custom_variables;
  while ((varmember = free_customvariablesmember(varmember)));

  free_objectlist(svc->servicegroups_ptr);

  delete[] svc->host_name;
  delete[] svc->description;
  delete[] svc->display_name;
  delete[] svc->service_check_command;
  delete[] svc->event_handler;
  delete[] svc->notification_period;
  delete[] svc->check_period;
  delete[] svc->failure_prediction_options;
  delete[] svc->notes;
  delete[] svc->notes_url;
  delete[] svc->action_url;
  delete[] svc->icon_image;
  delete[] svc->icon_image_alt;
  delete[] svc->plugin_output;
  delete[] svc->long_plugin_output;
  delete[] svc->perf_data;
  delete[] svc->event_handler_args;
  delete[] svc->check_command_args;

  // XXX: host_ptr;
  // XXX: event_handler_ptr;
  // XXX: check_command_ptr;
  // XXX: check_period_ptr;
  // XXX: notification_period_ptr;

  delete svc;
}

void modules::free_contact(contact const* cntct) {
  if (cntct == NULL)
    return;

  commandsmember const* cmdmember = cntct->host_notification_commands;
  while ((cmdmember = free_commandsmember(cmdmember)));

  cmdmember = cntct->service_notification_commands;
  while ((cmdmember = free_commandsmember(cmdmember)));

  customvariablesmember const* varmember = cntct->custom_variables;
  while ((varmember = free_customvariablesmember(varmember)));

  free_objectlist(cntct->contactgroups_ptr);

  // host_notification_period_ptr: not free;
  // service_notification_period_ptr: not free;

  delete[] cntct->name;
  delete[] cntct->alias;
  delete[] cntct->email;
  delete[] cntct->pager;
  for (unsigned int i = 0; i < MAX_CONTACT_ADDRESSES; ++i)
    delete[] cntct->address[i];
  delete[] cntct->host_notification_period;
  delete[] cntct->service_notification_period;
  delete cntct;
}

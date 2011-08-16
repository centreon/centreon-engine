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

#include "globals.hh"
#include "free_object.hh"

using namespace com::centreon::engine;

template <class T>
static void _remove_object_list(T const* obj, T** head, T** tail) {
  T* current = *head;
  T* prev = NULL;
  while (current != NULL) {
    if (current == obj) {
      if (prev == NULL)
        *head = current->next;
      else
        prev->next = current->next;
      if (current->next == NULL)
        *tail = prev;
      break;
    }
    prev = current;
    current = current->next;
  }
}

/**
 *  Cleanup memory of contactsmember.
 *
 *  @param[in] member The contact member to cleanup memory.
 *
 *  @return The next contactsmember.
 */
contactsmember const* modules::free_contactsmember(contactsmember const* member) {
  if (member == NULL)
    return (NULL);

  contactsmember const* next = member->next;
  delete[] member->contact_name;
  delete member;
  return (next);
}

/**
 *  Cleanup memory of contactgroupsmember.
 *
 *  @param[in] member The contact groups member to cleanup memory.
 *
 *  @return The next contactgroupsmember.
 */
contactgroupsmember const* modules::free_contactgroupsmember(contactgroupsmember const* member) {
  if (member == NULL)
    return (NULL);

  contactgroupsmember const* next = member->next;
  delete[] member->group_name;
  delete member;
  return (next);
}

/**
 *  Cleanup memory of hostsmember.
 *
 *  @param[in] member The host member to cleanup memory.
 *
 *  @return The next hostsmember.
 */
hostsmember const* modules::free_hostsmember(hostsmember const* member) {
  if (member == NULL)
    return (NULL);

  hostsmember const* next = member->next;
  delete[] member->host_name;
  delete member;
  return (next);
}

/**
 *  Cleanup memory of servicesmember.
 *
 *  @param[in] member The service member to cleanup memory.
 *
 *  @return The next servicesmember.
 */
servicesmember const* modules::free_servicesmember(servicesmember const* member) {
  if (member == NULL)
    return (NULL);

  servicesmember const* next = member->next;
  delete[] member->host_name;
  delete[] member->service_description;
  delete member;
  return (next);
}

/**
 *  Cleanup memory of commandsmember.
 *
 *  @param[in] member The command member to cleanup memory.
 *
 *  @return The next commandsmember.
 */
commandsmember const* modules::free_commandsmember(commandsmember const* member) {
  if (member == NULL)
    return (NULL);

  commandsmember const* next = member->next;
  delete[] member->cmd;
  delete member;
  return (next);
}

/**
 *  Cleanup memory of customvariablesmember.
 *
 *  @param[in] member The customvariable member to cleanup memory.
 *
 *  @return The next customvariablesmember.
 */
customvariablesmember const* modules::free_customvariablesmember(customvariablesmember const* member) {
  if (member == NULL)
    return (NULL);

  customvariablesmember const* next = member->next;
  delete[] member->variable_name;
  delete[] member->variable_value;
  delete member;
  return (next);
}

/**
 *  Cleanup memory of objectlist.
 *
 *  @param[in] objlist The object to cleanup memory.
 */
void modules::free_objectlist(objectlist const* objlist) {
  if (objlist == NULL)
    return;

  while (objlist) {
    objectlist const* tmp = objlist->next;
    delete objlist;
    objlist = tmp;
  }
}

/**
 *  Cleanup memory of contactgroup.
 *
 *  @param[in] group The contactgroup to cleanup memory.
 */
void modules::free_contactgroup(contactgroup const* group) {
  if (group == NULL)
    return;

  contactsmember const* member = group->members;
  while ((member = free_contactsmember(member)));

  skiplist_delete(object_skiplists[CONTACTGROUP_SKIPLIST], group);
  _remove_object_list(group, &contactgroup_list, &contactgroup_list_tail);

  delete[] group->group_name;
  delete[] group->alias;
  delete group;
}

/**
 *  Cleanup memory of hostgroup.
 *
 *  @param[in] group The hostgroup to cleanup memory.
 */
void modules::free_hostgroup(hostgroup const* group) {
  if (group == NULL)
    return;

  hostsmember const* member = group->members;
  while ((member = free_hostsmember(member)));

  skiplist_delete(object_skiplists[HOSTGROUP_SKIPLIST], group);
  _remove_object_list(group, &hostgroup_list, &hostgroup_list_tail);

  delete[] group->group_name;
  delete[] group->alias;
  delete[] group->notes;
  delete[] group->notes_url;
  delete[] group->action_url;
  delete group;
}

/**
 *  Cleanup memory of servicegroup.
 *
 *  @param[in] group The servicegroup to cleanup memory.
 */
void modules::free_servicegroup(servicegroup const* group) {
  if (group == NULL)
    return;

  servicesmember const* member = group->members;
  while ((member = free_servicesmember(member)));

  skiplist_delete(object_skiplists[SERVICEGROUP_SKIPLIST], group);
  _remove_object_list(group, &servicegroup_list, &servicegroup_list_tail);

  delete[] group->group_name;
  delete[] group->alias;
  delete[] group->notes;
  delete[] group->notes_url;
  delete[] group->action_url;
  delete group;
}

/**
 *  Cleanup memory of host.
 *
 *  @param[in] hst The host to cleanup memory.
 */
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
  skiplist_delete(object_skiplists[HOST_SKIPLIST], hst);
  _remove_object_list(hst, &host_list, &host_list_tail);

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

/**
 *  Cleanup memory of service.
 *
 *  @param[in] svc The service to cleanup memory.
 */
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
  skiplist_delete(object_skiplists[SERVICE_SKIPLIST], svc);
  _remove_object_list(svc, &service_list, &service_list_tail);

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

/**
 *  Cleanup memory of contact.
 *
 *  @param[in] cntct The contact to cleanup memory.
 */
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
  skiplist_delete(object_skiplists[CONTACT_SKIPLIST], cntct);
  _remove_object_list(cntct, &contact_list, &contact_list_tail);

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

/**
 *  Cleanup memory of hostescalation.
 *
 *  @param[in] hstescalation The hostescalation to cleanup memory.
 */
void modules::free_hostescalation(hostescalation const* hstescalation) {
  if (hstescalation == NULL)
    return;

  contactgroupsmember const* cgmember = hstescalation->contact_groups;
  while ((cgmember = free_contactgroupsmember(cgmember)));

  contactsmember const* cntctmember = hstescalation->contacts;
  while ((cntctmember = free_contactsmember(cntctmember)));

  skiplist_delete(object_skiplists[HOSTESCALATION_SKIPLIST], hstescalation);
  _remove_object_list(hstescalation, &hostescalation_list, &hostescalation_list_tail);

  // host_ptr: not free;
  // escalation_period_ptr: not free;

  delete[] hstescalation->host_name;
  delete[] hstescalation->escalation_period;
  delete hstescalation;
}

/**
 *  Cleanup memory of serviceescalation.
 *
 *  @param[in] svcescalation The serviceescalation to cleanup memory.
 */
void modules::free_serviceescalation(serviceescalation const* svcescalation) {
  if (svcescalation == NULL)
    return;

  contactgroupsmember const* cgmember = svcescalation->contact_groups;
  while ((cgmember = free_contactgroupsmember(cgmember)));

  contactsmember const* cntctmember = svcescalation->contacts;
  while ((cntctmember = free_contactsmember(cntctmember)));

  skiplist_delete(object_skiplists[SERVICEESCALATION_SKIPLIST], svcescalation);
  _remove_object_list(svcescalation, &serviceescalation_list, &serviceescalation_list_tail);

  // service_ptr: not free;
  // escalation_period_ptr: not free;

  delete[] svcescalation->host_name;
  delete[] svcescalation->description;
  delete[] svcescalation->escalation_period;
  delete svcescalation;
}

/**
 *  Cleanup memory of timeperiodexclusion.
 *
 *  @param[in] tpexclusions The timeperiodexclusion to cleanup memory.
 */
void modules::free_timeperiodexclusion(timeperiodexclusion const* tpexclusions) {
  if (tpexclusions == NULL)
    return;

  delete[] tpexclusions->timeperiod_name;
  delete tpexclusions;
}

/**
 *  Cleanup memory of daterange.
 *
 *  @param[in] tpexclusions The daterange to cleanup memory.
 */
void modules::free_daterange(daterange const* drange) {
  if (drange == NULL)
    return;

  free_timerange(drange->times);
  delete drange;
}

/**
 *  Cleanup memory of timerange.
 *
 *  @param[in] tpexclusions The timerange to cleanup memory.
 */
void modules::free_timerange(timerange const* trange) {
  while (trange != NULL) {
    timerange const* tmp = trange->next;
    delete trange;
    trange = tmp;
  }
}

/**
 *  Cleanup memory of serviceescalation.
 *
 *  @param[in] svcescalation The serviceescalation to cleanup memory.
 */
void modules::free_timeperiod(timeperiod const* tperiod) {
  if (tperiod == NULL)
    return;

  skiplist_delete(object_skiplists[TIMEPERIOD_SKIPLIST], tperiod);
  _remove_object_list(tperiod, &timeperiod_list, &timeperiod_list_tail);

  free_timerange(*tperiod->days);
  for (unsigned int i = 0; i < DATERANGE_TYPES; ++i)
    free_daterange(tperiod->exceptions[i]);
  free_timeperiodexclusion(tperiod->exclusions);

  delete[] tperiod->name;
  delete[] tperiod->alias;
  delete tperiod;
}

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

#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/contact.hh"
#include "com/centreon/engine/objects/contactgroup.hh"
#include "com/centreon/engine/objects/contactgroupsmember.hh"
#include "com/centreon/engine/objects/contactsmember.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"
#include "com/centreon/engine/objects/host.hh"
#include "com/centreon/engine/objects/hostsmember.hh"
#include "com/centreon/engine/objects/objectlist.hh"
#include "com/centreon/engine/objects/servicesmember.hh"
#include "com/centreon/engine/objects/utils.hh"
#include "com/centreon/engine/skiplist.hh"
#include "com/centreon/engine/statusdata.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::objects::utils;

static void _schedule_host(host* hst);

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::link
 */
bool link_host(host* obj,
               host** parents,
               contact** contacts,
               contactgroup** contactgroups,
               hostgroup** hostgroups,
               char** custom_variables,
               int initial_state,
               timeperiod* check_period,
               timeperiod* notification_period,
               command* cmd_event_handler,
               command* cmd_check_command) {
  try {
    objects::link(obj,
                  tab2vec(parents),
                  tab2vec(contacts),
                  tab2vec(contactgroups),
                  tab2vec(hostgroups),
                  tab2vec(custom_variables),
                  initial_state,
                  check_period,
                  notification_period,
                  cmd_event_handler,
                  cmd_check_command);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << e.what();
    return (false);
  }
  catch (...) {
    logger(log_runtime_error, basic) << __func__ << " unknow exception";
    return (false);
  }
  return (true);
}

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::release
 */
void release_host(host const* obj) {
  try {
    objects::release(obj);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic) << __func__ << " unknow exception";
  }
}

/**
 *  Link an host with commands, notification period, contacts, contactgroups,
 *  hostgroups, custom variable and host into the engine.
 *
 *  @param[in,out] obj                 Object to link with a correct name.
 *  @param[in]     parents             Set host parent.
 *  @param[in]     contacts            Set host contacts.
 *  @param[in]     contactgroups       Set host contactgroups.
 *  @param[in]     hostgroups          Set host hostgroups.
 *  @param[in]     custom_variables    Set host custom variables.
 *  @param[in]     initial_state       Set host initial state.
 *  @param[in]     check_period        Set host check timeperiod.
 *  @param[in]     notification_period Set host notification timeperiod.
 *  @param[in]     cmd_event_handler   Set host event handler command.
 *  @param[in]     cmd_check_command   Set host check command.
 */
void objects::link(host* obj,
                   std::vector<host*> const& parents,
                   std::vector<contact*> const& contacts,
                   std::vector<contactgroup*> const& contactgroups,
                   std::vector<hostgroup*> const& hostgroups,
                   std::vector<std::string> const& custom_variables,
                   int initial_state,
                   timeperiod* check_period,
                   timeperiod* notification_period,
                   command* cmd_event_handler,
                   command* cmd_check_command) {
  // check object contents.
  if (obj == NULL)
    throw (engine_error() << "host is a NULL pointer.");
  if (obj->name == NULL)
    throw (engine_error() << "host invalid name.");

  // update initial state.
  obj->initial_state = initial_state;

  // update timeperiod pointer.
  if ((obj->check_period_ptr = check_period) == NULL)
    throw (engine_error() << "host '" << obj->name << "' invalid check period.");

  // update timeperiod pointer.
  if ((obj->notification_period_ptr = notification_period) == NULL)
    throw (engine_error() << "host '" << obj->name << "' invalid notification period.");

  // update command pointer.
  obj->event_handler_ptr = cmd_event_handler;

  // update command pointer.
  obj->check_command_ptr = cmd_check_command;

  // add host parents.
  if (add_hosts_to_object(parents, &obj->parent_hosts) == false)
    throw (engine_error() << "host '" << obj->name << "' invalid parent.");

  // add host childs.
  for (hostsmember* member = obj->parent_hosts; member != NULL; member = member->next)
    add_child_link_to_host(member->host_ptr, obj);

  if (add_custom_variables_to_object(custom_variables,
                                     &obj->custom_variables) == false)
    throw (engine_error() << "host '" << obj->name << "' invalid custom variable.");

  if (add_contacts_to_object(contacts, &obj->contacts) == false)
    throw (engine_error() << "host '" << obj->name << "' invalid contact.");

  if (add_contactgroups_to_object(contactgroups, &obj->contact_groups) == false)
    throw (engine_error() << "host '" << obj->name << "' invalid contact groups.");

  for (std::vector<hostgroup*>::const_iterator it = hostgroups.begin(),
         end = hostgroups.end();
       it != end;
       ++it) {
    if (*it == NULL)
      throw (engine_error() << "host '" << obj->name << "' invalid hostgroup.");
    add_object_to_objectlist(&obj->hostgroups_ptr, *it);
  }

  // add into scheduler.
  _schedule_host(obj);

  // host services are update by add service.
}

/**
 *  Cleanup memory of servicegroup.
 *
 *  @param[in] obj The servicegroup to cleanup memory.
 */
void objects::release(host const* obj) {
  if (obj == NULL)
    return;

  hostsmember const* hstmember = obj->parent_hosts;
  while ((hstmember = release(hstmember)));

  hstmember = obj->child_hosts;
  while ((hstmember = release(hstmember)));

  servicesmember const* svcmember = obj->services;
  while ((svcmember = release(svcmember)));

  contactgroupsmember const* cgmember = obj->contact_groups;
  while ((cgmember = release(cgmember)));

  contactsmember const* cntctmember = obj->contacts;
  while ((cntctmember = release(cntctmember)));

  customvariablesmember const* varmember = obj->custom_variables;
  while ((varmember = release(varmember)));

  release(obj->hostgroups_ptr);
  skiplist_delete(object_skiplists[HOST_SKIPLIST], obj);
  remove_object_list(obj, &host_list, &host_list_tail);

  // update the event list low.
  for (timed_event* temp_event = event_list_low;
    temp_event != NULL;
    temp_event = temp_event->next) {
    if (temp_event->event_data == obj) {
      remove_event(temp_event, &event_list_low, &event_list_low_tail);
      delete temp_event;
      break;
    }
  }

  // update the event list high.
  for (timed_event* temp_event = event_list_high;
       temp_event != NULL;
       temp_event = temp_event->next) {
    if (temp_event->event_data == obj) {
      remove_event(temp_event, &event_list_high, &event_list_high_tail);
      delete temp_event;
      break;
    }
  }

  delete[] obj->name;
  delete[] obj->display_name;
  delete[] obj->alias;
  delete[] obj->address;
  delete[] obj->host_check_command;
  delete[] obj->event_handler;
  delete[] obj->notification_period;
  delete[] obj->check_period;
  delete[] obj->failure_prediction_options;
  delete[] obj->notes;
  delete[] obj->notes_url;
  delete[] obj->action_url;
  delete[] obj->icon_image;
  delete[] obj->icon_image_alt;
  delete[] obj->vrml_image;
  delete[] obj->statusmap_image;
  delete[] obj->plugin_output;
  delete[] obj->long_plugin_output;
  delete[] obj->perf_data;

  // event_handler_ptr not free.
  // check_command_ptr not free.
  // check_period_ptr not free.
  // notification_period_ptr not free.

  delete obj;
}

/**
 *  Add somme hosts to a generic object with hosts member list.
 *
 *  @param[in]  hosts     The hosts to insert.
 *  @param[out] list_host The object host.
 *
 *  @return True if insert sucessfuly, false otherwise.
 */
bool objects::add_hosts_to_object(std::vector<host*> const& hosts,
                                  hostsmember** list_host) {
  if (list_host == NULL)
    return (false);

  for (std::vector<host*>::const_iterator it = hosts.begin(),
         end = hosts.end();
       it != end;
       ++it) {
    if (*it == NULL)
      return (false);

    // create a new hostsmember and add it into the host list.
    hostsmember* member = new hostsmember;
    memset(member, 0, sizeof(*member));

    member->host_name = my_strdup((*it)->name);
    member->next = *list_host;
    *list_host = member;

    // add host to the hostsmember.
    member->host_ptr = *it;
  }
  return (true);
}

static void _update_schedule_info(host const* hst) {
  logger(dbg_events, most) << "Determining host scheduling parameters.";

  if (hst == NULL)
    return;

  ++scheduling_info.total_hosts;
  ++scheduling_info.total_scheduled_hosts;

  if (scheduling_info.first_host_check == 0
      || hst->next_check < scheduling_info.first_host_check)
    scheduling_info.first_host_check = hst->next_check;
  if (hst->next_check > scheduling_info.last_host_check)
    scheduling_info.last_host_check = hst->next_check;

  scheduling_info.host_check_interval_total += hst->check_interval * config.get_interval_length();
  scheduling_info.average_services_per_host =
    (double)scheduling_info.total_services / (double)scheduling_info.total_hosts;
  scheduling_info.average_scheduled_services_per_host =
    (double)scheduling_info.total_scheduled_services / (double)scheduling_info.total_hosts;

  scheduling_info.max_host_check_spread = config.get_max_host_check_spread();

  // we determine the host inter-check delay.
  if (config.get_host_inter_check_delay_method() == configuration::state::icd_smart
      && scheduling_info.host_check_interval_total > 0) {

    scheduling_info.average_host_check_interval =
      (double)scheduling_info.host_check_interval_total
      / (double)scheduling_info.total_scheduled_hosts;

    scheduling_info.average_host_inter_check_delay =
      (double)scheduling_info.average_host_check_interval
      / (double)scheduling_info.total_scheduled_hosts;

    scheduling_info.host_inter_check_delay = scheduling_info.average_host_inter_check_delay;

    // calculate max inter check delay and see if we should use that instead.
    double max_inter_check_delay =
      (double)(scheduling_info.max_host_check_spread * 60.0)
      / (double)scheduling_info.total_scheduled_hosts;
    if (scheduling_info.host_inter_check_delay > max_inter_check_delay)
      scheduling_info.host_inter_check_delay = max_inter_check_delay;

    logger(dbg_events, most)
      << "Total scheduled host checks: " << scheduling_info.total_scheduled_hosts << "\n"
      << "Host check interval total:   " << scheduling_info.host_check_interval_total << "\n"
      << "Average host check interval: " << scheduling_info.average_host_check_interval << " sec\n"
      << "Host inter-check delay:      " << scheduling_info.host_inter_check_delay << " sec";
  }
}

static void _schedule_host(host* hst) {
  if (hst == NULL) {
    logger(dbg_events, most) << "Scheduling host. Host pointer is NULL.";
    return;
  }

  logger(dbg_events, most) << "Scheduling host '" << hst->name << "'.";

  // skip host that shouldn't be scheduled.
  if (hst->should_be_scheduled == false) {
    logger(dbg_events, most) << "Host check should not be scheduled.";
    return;
  }

  // skip host are already scheduled.
  if (hst->next_check != 0) {
    logger(dbg_events, most) << "Host is already scheduled.";
    return;
  }

  // get current time.
  hst->next_check = time(NULL);

  logger(dbg_events, most)
    << "Preferred check time: " << hst->next_check << " --> " << ctime(&hst->next_check);

  // check if the current time is ok.
  if (check_time_against_period(hst->next_check, hst->check_period_ptr) == ERROR) {
    time_t valid_time = 0L;
    get_next_valid_time(hst->next_check, &valid_time, hst->check_period_ptr);
    hst->next_check = valid_time;
  }

  logger(dbg_events, most)
    << "Actual check time: " << hst->next_check << " --> " << ctime(&hst->next_check);

  // update scheduling info.
  _update_schedule_info(hst);

  // update status of all hosts.
  update_host_status(hst, false);

  // skip most host that shouldn't be scheduled.
  if (hst->should_be_scheduled == false
      && !(hst->checks_enabled == false
           && hst->next_check != 0
           && (hst->check_options & CHECK_OPTION_FORCE_EXECUTION)))
    return;

  // add scheduled host checks to event queue.
  schedule_new_event(EVENT_HOST_CHECK,
                     false,
                     hst->next_check,
                     false,
                     0,
                     NULL,
                     true,
                     (void*)hst,
                     NULL,
                     hst->check_options);
}

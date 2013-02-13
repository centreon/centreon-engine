/*
** Copyright 2011-2013 Merethis
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

#include <cmath>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/contact.hh"
#include "com/centreon/engine/objects/contactgroup.hh"
#include "com/centreon/engine/objects/contactgroupsmember.hh"
#include "com/centreon/engine/objects/contactsmember.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"
#include "com/centreon/engine/objects/objectlist.hh"
#include "com/centreon/engine/objects/service.hh"
#include "com/centreon/engine/objects/utils.hh"
#include "com/centreon/engine/statusdata.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::objects::utils;

static void _schedule(service* svc);

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::link
 */
bool link_service(
       service* obj,
       contact** contacts,
       contactgroup** contactgroups,
       servicegroup** servicegroups,
       char** custom_variables,
       int initial_state,
       timeperiod* check_period,
       timeperiod* notification_period,
       command* cmd_event_handler,
       command* cmd_check_command) {
  try {
    objects::link(
      obj,
      tab2vec(contacts),
      tab2vec(contactgroups),
      tab2vec(servicegroups),
      tab2vec(custom_variables),
      initial_state,
      check_period,
      notification_period,
      cmd_event_handler,
      cmd_check_command);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << "error: " << e.what();
    return (false);
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << "error: link_service: unknow exception";
    return (false);
  }
  return (true);
}

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::release
 */
void release_service(service const* obj) {
  try {
    objects::release(obj);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << "error: " << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << "error: release_service: unknow exception";
  }
  return;
}

/**
 *  Link a service with commands, notification period, contacts, contactgroups,
 *  hostgroups, custom variable and host into the engine.
 *
 *  @param[in,out] obj                 Object to link with a correct name.
 *  @param[in]     contacts            Set service contacts.
 *  @param[in]     contactgroups       Set service contactgroups.
 *  @param[in]     servicegroups       Set service hostgroups.
 *  @param[in]     custom_variables    Set service custom variables.
 *  @param[in]     initial_state       Set service initial state.
 *  @param[in]     check_period        Set service check timeperiod.
 *  @param[in]     notification_period Set service notification timeperiod.
 *  @param[in]     cmd_event_handler   Set service event handler command.
 *  @param[in]     cmd_check_command   Set service check command.
 */
void objects::link(
       service* obj,
       std::vector<contact*> const& contacts,
       std::vector<contactgroup*> const& contactgroups,
       std::vector<servicegroup*> const& servicegroups,
       std::vector<std::string> const& custom_variables,
       int initial_state,
       timeperiod* check_period,
       timeperiod* notification_period,
       command* cmd_event_handler,
       command* cmd_check_command) {
  // check object contents.
  if (obj == NULL)
    throw (engine_error() << "service is a NULL pointer.");
  if (obj->host_name == NULL)
    throw (engine_error() << "service invalid host name.");
  if (obj->description == NULL)
    throw (engine_error() << "service invalid description.");

  // update initial state.
  obj->initial_state = initial_state;

  // update host pointer.
  if ((obj->host_ptr = find_host(obj->host_name)) == NULL)
    throw (engine_error() << "service '" << obj->host_name << ", "
	   << obj->description << "' invalid host name.");

  // update timeperiod pointer.
  if ((obj->check_period_ptr = check_period) == NULL)
    throw (engine_error() << "service '" << obj->host_name << ", "
	   << obj->description << "' invalid check period.");

  // update timeperiod pointer.
  if ((obj->notification_period_ptr = notification_period) == NULL)
    throw (engine_error() << "service '" << obj->host_name << ", "
	   << obj->description << "' invalid notification period.");

  // update command pointer.
  obj->event_handler_ptr = cmd_event_handler;

  // update command pointer.
  obj->check_command_ptr = cmd_check_command;

  if (add_custom_variables_to_object(
       custom_variables,
       &obj->custom_variables) == false)
    throw (engine_error() << "service '" << obj->host_name << ", "
	   << obj->description << "' invalid custom variable.");

  if (add_contacts_to_object(contacts, &obj->contacts) == false)
    throw (engine_error() << "service '" << obj->host_name << ", "
	   << obj->description << "' invalid contact.");

  if (add_contactgroups_to_object(
        contactgroups,
        &obj->contact_groups) == false)
    throw (engine_error() << "service '" << obj->host_name << ", "
	   << obj->description << "' invalid contact groups.");

  for (std::vector<servicegroup*>::const_iterator
         it = servicegroups.begin(), end = servicegroups.end();
       it != end;
       ++it) {
    if (*it == NULL)
      throw (engine_error() << "service '" << obj->host_name << ", "
             << obj->description << "' invalid service group.");
    add_object_to_objectlist(&obj->servicegroups_ptr, *it);
  }

  // update host services.
  add_service_link_to_host(obj->host_ptr, obj);

  ++obj->host_ptr->total_services;
  obj->host_ptr->total_service_check_interval
    += static_cast<unsigned long>(obj->check_interval);

  _schedule(obj);
  return;
}

/**
 *  Cleanup memory of service.
 *
 *  @param[in] obj The service to cleanup memory.
 */
void objects::release(service const* obj) {
  if (obj == NULL)
    return;

  contactgroupsmember const* cgmember = obj->contact_groups;
  while ((cgmember = release(cgmember))) {}

  contactsmember const* cntctmember = obj->contacts;
  while ((cntctmember = release(cntctmember))) {}

  customvariablesmember const* varmember = obj->custom_variables;
  while ((varmember = release(varmember))) {}

  release(obj->servicegroups_ptr);
  skiplist_delete(object_skiplists[SERVICE_SKIPLIST], obj);
  remove_object_list(obj, &service_list, &service_list_tail);

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

  delete[] obj->host_name;
  delete[] obj->description;
  delete[] obj->display_name;
  delete[] obj->service_check_command;
  delete[] obj->event_handler;
  delete[] obj->notification_period;
  delete[] obj->check_period;
  delete[] obj->failure_prediction_options;
  delete[] obj->notes;
  delete[] obj->notes_url;
  delete[] obj->action_url;
  delete[] obj->icon_image;
  delete[] obj->icon_image_alt;
  delete[] obj->plugin_output;
  delete[] obj->long_plugin_output;
  delete[] obj->perf_data;
  delete[] obj->event_handler_args;
  delete[] obj->check_command_args;

  // host_ptr not free.
  // event_handler_ptr not free.
  // check_command_ptr not free.
  // check_period_ptr not free.
  // notification_period_ptr not free.

  delete obj;
  return;
}

static void _update_schedule_info(service const* svc) {
  logger(dbg_events, most)
    << "Determining service scheduling parameters.";

  if (svc == NULL)
    return;

  ++scheduling_info.total_services;
  ++scheduling_info.total_scheduled_services;

  if (scheduling_info.first_service_check == 0
      || svc->next_check < scheduling_info.first_service_check)
    scheduling_info.first_service_check = svc->next_check;
  if (svc->next_check > scheduling_info.last_service_check)
    scheduling_info.last_service_check = svc->next_check;

  scheduling_info.service_check_interval_total
    += (unsigned long)(svc->check_interval * config->get_interval_length());
  scheduling_info.average_service_check_interval
    = (double)scheduling_info.service_check_interval_total
    / (double)scheduling_info.total_scheduled_services;

  scheduling_info.average_service_execution_time
    = (double)((scheduling_info.average_service_execution_time
                * (scheduling_info.total_scheduled_services - 1))
               + svc->execution_time)
    / (double)scheduling_info.total_scheduled_services;


  scheduling_info.average_services_per_host
    = (double)scheduling_info.total_services
    / (double)scheduling_info.total_hosts;
  scheduling_info.average_scheduled_services_per_host
    = (double)scheduling_info.total_scheduled_services
    / (double)scheduling_info.total_hosts;

  scheduling_info.max_service_check_spread
    = config->get_max_service_check_spread();

  // we determine the service inter-check delay.
  if (config->get_service_inter_check_delay_method() == configuration::state::icd_smart
      && scheduling_info.service_check_interval_total > 0) {

    scheduling_info.average_service_inter_check_delay
      = (double)scheduling_info.average_service_check_interval
      / (double)scheduling_info.total_scheduled_services;

    scheduling_info.service_inter_check_delay
      = scheduling_info.average_service_inter_check_delay;

    // calculate max inter check delay and see if we should use that instead.
    double max_inter_check_delay
      = (double)(scheduling_info.max_service_check_spread * 60.0)
      / (double)scheduling_info.total_scheduled_services;
    if (scheduling_info.service_inter_check_delay > max_inter_check_delay)
      scheduling_info.service_inter_check_delay = max_inter_check_delay;

    logger(dbg_events, most)
      << "Total scheduled service checks: "
      << scheduling_info.total_scheduled_services << "\n"
      << "Average service check interval: "
      << scheduling_info.average_service_check_interval << " sec\n"
      << "Service inter-check delay:      "
      << scheduling_info.service_inter_check_delay << " sec\n";
  }

  // we determine the service interleave factor.
  if (config->get_service_interleave_factor_method() == configuration::state::ilf_smart) {
    scheduling_info.service_interleave_factor
      = (int)(ceil(scheduling_info.average_scheduled_services_per_host));

    logger(dbg_events, most)
      << "Total scheduled service checks: " << scheduling_info.total_scheduled_services << "\n"
      << "Total hosts:                    " << scheduling_info.total_hosts << "\n"
      << "Service Interleave factor:      " << scheduling_info.service_interleave_factor;
  }

  logger(dbg_events, most)
    << "Total scheduled services:        " << scheduling_info.total_scheduled_services << "\n"
    << "Service Interleave factor:       " << scheduling_info.service_interleave_factor << "\n"
    << "Service inter-check delay:       " << scheduling_info.service_inter_check_delay;
  return;
}

static void _schedule(service* svc) {
  if (svc == NULL) {
    logger(dbg_events, most)
      << "Scheduling service. Service pointer is NULL.";
    return;
  }

  logger(dbg_events, most)
    << "Scheduling Service '" << svc->description
    << "' on host '" << svc->host_name << "'.";

  // skip serivce that shouldn't be scheduled.
  if (svc->should_be_scheduled == false) {
    logger(dbg_events, most)
      << "Service check should not be scheduled.";
    return;
  }

  // skip service are already scheduled.
  if (svc->next_check != 0) {
    logger(dbg_events, most)
      << "Service is already scheduled.";
    return;
  }

  // get current time.
  svc->next_check = time(NULL);

  logger(dbg_events, most)
    << "Preferred Check Time: " << svc->next_check
    << " --> " << ctime(&svc->next_check);

  // check if current time is ok.
  if (check_time_against_period(
        svc->next_check,
        svc->check_period_ptr) == ERROR) {
    time_t valid_time = 0L;
    get_next_valid_time(
      svc->next_check,
      &valid_time,
      svc->check_period_ptr);
    svc->next_check = valid_time;
  }

  logger(dbg_events, most)
    << "Actual Check Time: " << svc->next_check
    << " --> " << ctime(&svc->next_check);

  // update scheduling info.
  _update_schedule_info(svc);

  // update status of all services.
  update_service_status(svc, false);

  // skip most service to shouldn't be scheduled.
  if (svc->should_be_scheduled == false
      && !(svc->checks_enabled == false
           && svc->next_check != 0L
           && (svc->check_options & CHECK_OPTION_FORCE_EXECUTION)))
    return;

  // add scheduled service checks to event queue.
  schedule_new_event(
    EVENT_SERVICE_CHECK,
    false,
    svc->next_check,
    false,
    0,
    NULL,
    true,
    (void*)svc,
    NULL,
    svc->check_options);
  return;
}

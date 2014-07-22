/*
** Copyright 2011-2014 Merethis
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
#include <ctime>
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/webservice/schedule_object.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules;

static void _update_host_schedule_info(host const* hst) {
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

  scheduling_info.host_check_interval_total
    += (unsigned long)(hst->check_interval * (double)config->interval_length());
  scheduling_info.average_services_per_host
    = (double)scheduling_info.total_services / (double)scheduling_info.total_hosts;
  scheduling_info.average_scheduled_services_per_host
    = (double)scheduling_info.total_scheduled_services / (double)scheduling_info.total_hosts;

  scheduling_info.max_host_check_spread = config->max_host_check_spread();

  // we determine the host inter-check delay.
  if (config->host_inter_check_delay_method() == configuration::state::icd_smart
      && scheduling_info.host_check_interval_total > 0) {

    scheduling_info.average_host_check_interval
      = (double)scheduling_info.host_check_interval_total
      / (double)scheduling_info.total_scheduled_hosts;

    scheduling_info.average_host_inter_check_delay
      = (double)scheduling_info.average_host_check_interval
      / (double)scheduling_info.total_scheduled_hosts;

    scheduling_info.host_inter_check_delay
      = scheduling_info.average_host_inter_check_delay;

    // calculate max inter check delay and see if we should use that instead.
    double max_inter_check_delay
      = (double)(scheduling_info.max_host_check_spread * 60.0)
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

static void _update_service_schedule_info(service const* svc) {
  logger(dbg_events, most) << "Determining service scheduling parameters.";

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
    += (unsigned long)(svc->check_interval * (double)config->interval_length());
  scheduling_info.average_service_check_interval
    = (double)scheduling_info.service_check_interval_total
    / (double)scheduling_info.total_scheduled_services;

  scheduling_info.average_service_execution_time
    = (double)((scheduling_info.average_service_execution_time
              * (scheduling_info.total_scheduled_services - 1))
             + svc->execution_time)
    / (double)scheduling_info.total_scheduled_services;


  scheduling_info.average_services_per_host
    = (double)scheduling_info.total_services / (double)scheduling_info.total_hosts;
  scheduling_info.average_scheduled_services_per_host
    = (double)scheduling_info.total_scheduled_services / (double)scheduling_info.total_hosts;

  scheduling_info.max_service_check_spread
    = config->max_service_check_spread();

  // we determine the service inter-check delay.
  if (config->service_inter_check_delay_method() == configuration::state::icd_smart
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
  if (config->service_interleave_factor_method() == configuration::state::ilf_smart) {
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
}

void webservice::schedule_host(host* hst) {
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
  _update_host_schedule_info(hst);

  // Update status of all hosts.
  update_host_status(hst);

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

void webservice::schedule_service(service* svc) {
  if (svc == NULL) {
    logger(dbg_events, most) << "Scheduling service. Service pointer is NULL.";
    return;
  }

  logger(dbg_events, most)
    << "Scheduling Service '" << svc->description << "' on host '" << svc->host_name << "'.";

  // skip serivce that shouldn't be scheduled.
  if (svc->should_be_scheduled == false) {
    logger(dbg_events, most) << "Service check should not be scheduled.";
    return;
  }

  // skip service are already scheduled.
  if (svc->next_check != 0) {
    logger(dbg_events, most) << "Service is already scheduled.";
    return;
  }

  // get current time.
  svc->next_check = time(NULL);

  logger(dbg_events, most)
    << "Preferred Check Time: " << svc->next_check << " --> " << ctime(&svc->next_check);

  // check if current time is ok.
  if (check_time_against_period(svc->next_check, svc->check_period_ptr) == ERROR) {
    time_t valid_time = 0L;
    get_next_valid_time(svc->next_check, &valid_time, svc->check_period_ptr);
    svc->next_check = valid_time;
  }

  logger(dbg_events, most)
    << "Actual Check Time: " << svc->next_check << " --> " << ctime(&svc->next_check);

  // update scheduling info.
  _update_service_schedule_info(svc);

  // Update status of all services.
  update_service_status(svc);

  // skip most service to shouldn't be scheduled.
  if (svc->should_be_scheduled == false
      && !(svc->checks_enabled == false
           && svc->next_check != 0L
           && (svc->check_options & CHECK_OPTION_FORCE_EXECUTION)))
    return;

  // add scheduled service checks to event queue.
  schedule_new_event(EVENT_SERVICE_CHECK,
                     false,
                     svc->next_check,
                     false,
                     0,
                     NULL,
                     true,
                     (void*)svc,
                     NULL,
                     svc->check_options);
}

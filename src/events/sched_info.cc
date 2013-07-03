/*
** Copyright 2007-2008 Ethan Galstad
** Copyright 2007,2010 Andreas Ericsson
** Copyright 2010      Max Schubert
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
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/sched_info.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/statusdata.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

// /**
//  *  Internal update host schedule info.
//  *
//  *  @param[in] hst The host to add into the event list.
//  */
// static void _update_host_schedule_info(host const& hst) {
//   logger(dbg_events, most)
//     << "Determining host scheduling parameters.";

//   ++scheduling_info.total_hosts;
//   ++scheduling_info.total_scheduled_hosts;

//   if (!scheduling_info.first_host_check
//       || hst.next_check < scheduling_info.first_host_check)
//     scheduling_info.first_host_check = hst.next_check;
//   if (hst.next_check > scheduling_info.last_host_check)
//     scheduling_info.last_host_check = hst.next_check;

//   scheduling_info.host_check_interval_total
//     += (unsigned long)(hst.check_interval * (double)config->interval_length());
//   scheduling_info.average_services_per_host
//     = (double)scheduling_info.total_services / (double)scheduling_info.total_hosts;
//   scheduling_info.average_scheduled_services_per_host
//     = (double)scheduling_info.total_scheduled_services / (double)scheduling_info.total_hosts;

//   scheduling_info.max_host_check_spread = config->max_host_check_spread();

//   // we determine the host inter-check delay.
//   if (config->host_inter_check_delay_method() == configuration::state::icd_smart
//       && scheduling_info.host_check_interval_total > 0) {

//     scheduling_info.average_host_check_interval
//       = (double)scheduling_info.host_check_interval_total
//       / (double)scheduling_info.total_scheduled_hosts;

//     scheduling_info.average_host_inter_check_delay
//       = (double)scheduling_info.average_host_check_interval
//       / (double)scheduling_info.total_scheduled_hosts;

//     scheduling_info.host_inter_check_delay
//       = scheduling_info.average_host_inter_check_delay;

//     // calculate max inter check delay and see if we should use that instead.
//     double max_inter_check_delay
//       = (double)(scheduling_info.max_host_check_spread * 60.0)
//       / (double)scheduling_info.total_scheduled_hosts;
//     if (scheduling_info.host_inter_check_delay > max_inter_check_delay)
//       scheduling_info.host_inter_check_delay = max_inter_check_delay;

//     logger(dbg_events, most)
//       << "Total scheduled host checks: " << scheduling_info.total_scheduled_hosts << "\n"
//       << "Host check interval total:   " << scheduling_info.host_check_interval_total << "\n"
//       << "Average host check interval: " << scheduling_info.average_host_check_interval << " sec\n"
//       << "Host inter-check delay:      " << scheduling_info.host_inter_check_delay << " sec";
//   }
// }

// /**
//  *  Internal update service schedule info.
//  *
//  *  @param[in] hst The service to add into the event list.
//  */
// static void _update_service_schedule_info(service const& svc) {
//   logger(dbg_events, most)
//     << "Determining service scheduling parameters.";

//   ++scheduling_info.total_services;
//   ++scheduling_info.total_scheduled_services;

//   if (scheduling_info.first_service_check == 0
//       || svc.next_check < scheduling_info.first_service_check)
//     scheduling_info.first_service_check = svc.next_check;
//   if (svc.next_check > scheduling_info.last_service_check)
//     scheduling_info.last_service_check = svc.next_check;

//   scheduling_info.service_check_interval_total
//     += (unsigned long)(svc.check_interval * (double)config->interval_length());
//   scheduling_info.average_service_check_interval
//     = (double)scheduling_info.service_check_interval_total
//     / (double)scheduling_info.total_scheduled_services;

//   scheduling_info.average_service_execution_time
//     = (double)((scheduling_info.average_service_execution_time
//               * (scheduling_info.total_scheduled_services - 1))
//              + svc.execution_time)
//     / (double)scheduling_info.total_scheduled_services;


//   scheduling_info.average_services_per_host
//     = (double)scheduling_info.total_services / (double)scheduling_info.total_hosts;
//   scheduling_info.average_scheduled_services_per_host
//     = (double)scheduling_info.total_scheduled_services / (double)scheduling_info.total_hosts;

//   scheduling_info.max_service_check_spread
//     = config->max_service_check_spread();

//   // we determine the service inter-check delay.
//   if (config->service_inter_check_delay_method() == configuration::state::icd_smart
//       && scheduling_info.service_check_interval_total > 0) {

//     scheduling_info.average_service_inter_check_delay
//       = (double)scheduling_info.average_service_check_interval
//       / (double)scheduling_info.total_scheduled_services;

//     scheduling_info.service_inter_check_delay
//       = scheduling_info.average_service_inter_check_delay;

//     // calculate max inter check delay and see if we should use that instead.
//     double max_inter_check_delay
//       = (double)(scheduling_info.max_service_check_spread * 60.0)
//       / (double)scheduling_info.total_scheduled_services;
//     if (scheduling_info.service_inter_check_delay > max_inter_check_delay)
//       scheduling_info.service_inter_check_delay = max_inter_check_delay;

//     logger(dbg_events, most)
//       << "Total scheduled service checks: "
//       << scheduling_info.total_scheduled_services << "\n"
//       << "Average service check interval: "
//       << scheduling_info.average_service_check_interval << " sec\n"
//       << "Service inter-check delay:      "
//       << scheduling_info.service_inter_check_delay << " sec\n";
//   }

//   // we determine the service interleave factor.
//   if (config->service_interleave_factor_method() == configuration::state::ilf_smart) {
//     scheduling_info.service_interleave_factor
//       = (int)(ceil(scheduling_info.average_scheduled_services_per_host));

//     logger(dbg_events, most)
//       << "Total scheduled service checks: " << scheduling_info.total_scheduled_services << "\n"
//       << "Total hosts:                    " << scheduling_info.total_hosts << "\n"
//       << "Service Interleave factor:      " << scheduling_info.service_interleave_factor;
//   }

//   logger(dbg_events, most)
//     << "Total scheduled services:        " << scheduling_info.total_scheduled_services << "\n"
//     << "Service Interleave factor:       " << scheduling_info.service_interleave_factor << "\n"
//     << "Service inter-check delay:       " << scheduling_info.service_inter_check_delay;
// }

/**
 *  Adjusts scheduling of host and service checks.
 */
void adjust_check_scheduling() {
  static double const projected_host_check_overhead(0.1);
  static double const projected_service_check_overhead(0.1);
  double current_exec_time(0.0);
  double current_exec_time_offset(0.0);
  double exec_time_factor(0.0);
  double inter_check_delay(0.0);
  double last_check_exec_time(0.0);
  double total_check_exec_time(0.0);
  int adjust_scheduling(false);
  int current_check(0);
  int total_checks(0);
  time_t last_check_time(0L);
  host* hst(NULL);
  service* svc(NULL);

  logger(dbg_functions, basic)
    << "adjust_check_scheduling()";

  /* TODO:
     - Track host check overhead on a per-host basis
     - Figure out how to calculate service check overhead
  */

  // determine our adjustment window.
  time_t current_time(time(NULL));
  time_t first_window_time(current_time);
  time_t last_window_time(first_window_time + config->auto_rescheduling_window());

  // get current scheduling data.
  for (timed_event* tmp(event_list_low); tmp; tmp = tmp->next) {
    // skip events outside of our current window.
    if (tmp->run_time <= first_window_time)
      continue;
    if (tmp->run_time > last_window_time)
      break;

    if (tmp->event_type == EVENT_HOST_CHECK) {

      if (!(hst = (host*)tmp->event_data))
        continue;

      // ignore forced checks.
      if (hst->check_options & CHECK_OPTION_FORCE_EXECUTION)
        continue;

      // does the last check "bump" into this one?
      if ((last_check_time + last_check_exec_time) > tmp->run_time)
        adjust_scheduling = true;

      last_check_time = tmp->run_time;

      // calculate time needed to perform check.
      // NOTE: host check execution time is not taken into account,
      // as scheduled host checks are run in parallel.
      last_check_exec_time = projected_host_check_overhead;
      total_check_exec_time += last_check_exec_time;
    }

    else if (tmp->event_type == EVENT_SERVICE_CHECK) {

      if (!((service*)tmp->event_data))
        continue;

      // ignore forced checks.
      if (svc->check_options & CHECK_OPTION_FORCE_EXECUTION)
        continue;

      // does the last check "bump" into this one?
      if ((last_check_time + last_check_exec_time) > tmp->run_time)
        adjust_scheduling = true;

      last_check_time = tmp->run_time;

      // calculate time needed to perform check.
      // NOTE: service check execution time is not taken into
      // account, as service checks are run in parallel.
      last_check_exec_time = projected_service_check_overhead;
      total_check_exec_time += last_check_exec_time;
    }
    else
      continue;

    ++total_checks;
  }

  // nothing to do...
  if (total_checks == 0 || adjust_scheduling == false)
    return;

  if ((unsigned long)total_check_exec_time > config->auto_rescheduling_window()) {
    inter_check_delay = 0.0;
    exec_time_factor
      = (double)((double)config->auto_rescheduling_window()
                 / total_check_exec_time);
  }
  else {
    inter_check_delay
      = (double)((((double)config->auto_rescheduling_window())
                  - total_check_exec_time) / (double)(total_checks * 1.0));
    exec_time_factor = 1.0;
  }

  // adjust check scheduling.
  double current_icd_offset(inter_check_delay / 2.0);
  for (timed_event* tmp(event_list_low); tmp; tmp = tmp->next) {

    // skip events outside of our current window.
    if (tmp->run_time <= first_window_time)
      continue;
    if (tmp->run_time > last_window_time)
      break;

    if (tmp->event_type == EVENT_HOST_CHECK) {

      if (!(hst = (host*)tmp->event_data))
        continue;

      // ignore forced checks.
      if (hst->check_options & CHECK_OPTION_FORCE_EXECUTION)
        continue;

      current_exec_time
        = ((hst->execution_time
            + projected_host_check_overhead)
           * exec_time_factor);
    }
    else if (tmp->event_type == EVENT_SERVICE_CHECK) {
      if (!(svc = (service*)tmp->event_data))
        continue;

      // ignore forced checks.
      if (svc->check_options & CHECK_OPTION_FORCE_EXECUTION)
        continue;

      // NOTE: service check execution time is not taken into
      // account, as service checks are run in parallel.
      current_exec_time
        = (projected_service_check_overhead * exec_time_factor);
    }
    else
      continue;

    ++current_check;
    double new_run_time_offset(current_exec_time_offset + current_icd_offset);
    time_t new_run_time(
             (time_t)(first_window_time
                      + (unsigned long)new_run_time_offset));

    if (tmp->event_type == EVENT_HOST_CHECK) {
      tmp->run_time = new_run_time;
      hst->next_check = new_run_time;
      update_host_status(hst, false);
    }
    else {
      tmp->run_time = new_run_time;
      svc->next_check = new_run_time;
      update_service_status(svc, false);
    }

    current_icd_offset += inter_check_delay;
    current_exec_time_offset += current_exec_time;
  }

  // resort event list (some events may be out of order at
  // this point).
  resort_event_list(&event_list_low, &event_list_low_tail);
  return;
}

/**
 *  Displays service check scheduling information.
 */
void display_scheduling_info() {
  float max_reaper_interval(0.0);
  float minimum_concurrent_checks(0.0);
  float minimum_concurrent_checks1(0.0);
  float minimum_concurrent_checks2(0.0);
  int suggestions(0);

  printf(
    "Projected scheduling information for host and service "
    "checks\n"
    "is listed below.  This information assumes that you are "
    "going\n"
    "to start running Centreon Engine with your current config "
    "files.\n\n");

  printf(
    "HOST SCHEDULING INFORMATION\n"
    "---------------------------\n"
    "Total hosts:                     %d\n",
    scheduling_info.total_hosts);
  printf(
    "Total scheduled hosts:           %d\n",
    scheduling_info.total_scheduled_hosts);

  printf("Host inter-check delay method:   ");
  if (config->host_inter_check_delay_method() == configuration::state::icd_none)
    printf("NONE\n");
  else if (config->host_inter_check_delay_method() == configuration::state::icd_dumb)
    printf("DUMB\n");
  else if (config->host_inter_check_delay_method() == configuration::state::icd_smart) {
    printf("SMART\n");
    printf(
      "Average host check interval:     %.2f sec\n",
      scheduling_info.average_host_check_interval);
  }
  else
    printf("USER-SUPPLIED VALUE\n");
  printf(
    "Host inter-check delay:          %.2f sec\n",
    scheduling_info.host_inter_check_delay);
  printf(
    "Max host check spread:           %d min\n",
    scheduling_info.max_host_check_spread);
  printf(
    "First scheduled check:           %s",
    (scheduling_info.total_scheduled_hosts == 0)
    ? "N/A\n" : ctime(&scheduling_info.first_host_check));
  printf(
    "Last scheduled check:            %s",
    (scheduling_info.total_scheduled_hosts == 0)
    ? "N/A\n" : ctime(&scheduling_info.last_host_check));
  printf("\n\n");

  printf(
    "SERVICE SCHEDULING INFORMATION\n"
    "-------------------------------\n");
  printf(
    "Total services:                     %d\n",
    scheduling_info.total_services);
  printf(
    "Total scheduled services:           %d\n",
    scheduling_info.total_scheduled_services);

  printf("Service inter-check delay method:   ");
  if (config->service_inter_check_delay_method() == configuration::state::icd_none)
    printf("NONE\n");
  else if (config->service_inter_check_delay_method() == configuration::state::icd_dumb)
    printf("DUMB\n");
  else if (config->service_inter_check_delay_method() == configuration::state::icd_smart) {
    printf("SMART\n");
    printf(
      "Average service check interval:     %.2f sec\n",
      scheduling_info.average_service_check_interval);
  }
  else
    printf("USER-SUPPLIED VALUE\n");
  printf(
    "Inter-check delay:                  %.2f sec\n",
    scheduling_info.service_inter_check_delay);

  printf(
    "Interleave factor method:           %s\n",
    (config->service_interleave_factor_method() == configuration::state::ilf_user)
    ? "USER-SUPPLIED VALUE" : "SMART");
  if (config->service_interleave_factor_method() == configuration::state::ilf_smart)
    printf(
      "Average services per host:          %.2f\n",
      scheduling_info.average_services_per_host);
  printf(
    "Service interleave factor:          %d\n",
    scheduling_info.service_interleave_factor);

  printf(
   "Max service check spread:           %d min\n",
   scheduling_info.max_service_check_spread);
  printf(
    "First scheduled check:              %s",
    ctime(&scheduling_info.first_service_check));
  printf(
    "Last scheduled check:               %s",
    ctime(&scheduling_info.last_service_check));
  printf("\n\n");

  printf(
    "CHECK PROCESSING INFORMATION\n"
    "----------------------------\n");
  printf(
    "Check result reaper interval:       %d sec\n",
    config->check_reaper_interval());
  printf("Max concurrent service checks:      ");
  if (config->max_parallel_service_checks() == 0)
    printf("Unlimited\n");
  else
    printf("%d\n", config->max_parallel_service_checks());
  printf("\n\n");

  printf("PERFORMANCE SUGGESTIONS\n");
  printf("-----------------------\n");

  /***** MAX REAPER INTERVAL RECOMMENDATION *****/

  // assume a 100% (2x) check burst for check reaper.
  // assume we want a max of 2k files in the result queue
  // at any given time.
  max_reaper_interval
    = floor(2000 * scheduling_info.service_inter_check_delay);
  if (max_reaper_interval < 2.0)
    max_reaper_interval = 2.0;
  if (max_reaper_interval > 30.0)
    max_reaper_interval = 30.0;
  if (max_reaper_interval < config->check_reaper_interval()) {
    printf(
      "* Value for 'check_result_reaper_frequency' should be <= %d "
      "seconds\n", (int)max_reaper_interval);
    suggestions++;
  }
  if (config->check_reaper_interval() < 2) {
    printf(
      "* Value for 'check_result_reaper_frequency' should be >= 2 "
      "seconds\n");
    suggestions++;
  }

  /***** MINIMUM CONCURRENT CHECKS RECOMMENDATION *****/

  // first method (old) - assume a 100% (2x) service check
  // burst for max concurrent checks.
  if (scheduling_info.service_inter_check_delay == 0.0)
    minimum_concurrent_checks1
      = ceil(config->check_reaper_interval() * 2.0);
  else
    minimum_concurrent_checks1
      = ceil((config->check_reaper_interval() * 2.0)
             / scheduling_info.service_inter_check_delay);

  // second method (new) - assume a 25% (1.25x) service check
  // burst for max concurrent checks.
  minimum_concurrent_checks2
    = ceil((((double)scheduling_info.total_scheduled_services)
            / scheduling_info.average_service_check_interval) * 1.25
           * config->check_reaper_interval()
           * scheduling_info.average_service_execution_time);

  // use max of computed values.
  if (minimum_concurrent_checks1 > minimum_concurrent_checks2)
    minimum_concurrent_checks = minimum_concurrent_checks1;
  else
    minimum_concurrent_checks = minimum_concurrent_checks2;

  // compare with configured value.
  if ((minimum_concurrent_checks > config->max_parallel_service_checks())
      && config->max_parallel_service_checks() != 0) {
    printf(
      "* Value for 'max_concurrent_checks' option should be >= %d\n",
      (int)minimum_concurrent_checks);
    suggestions++;
  }

  if (suggestions == 0)
    printf("I have no suggestions - things look okay.\n");
  printf("\n");
  return;
}

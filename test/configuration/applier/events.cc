/*
** Copyright 1999-2010 Ethan Galstad
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

#include <sys/time.h>
#include <cmath>
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/statusdata.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/* initialize the event timing loop before we start monitoring */
void init_timing_loop() {
  host* temp_host = NULL;
  service* temp_service = NULL;
  time_t current_time = 0L;
  unsigned long interval_to_use = 0L;
  int total_interleave_blocks = 0;
  int current_interleave_block = 1;
  int interleave_block_index = 0;
  int mult_factor = 0;
  int is_valid_time = 0;
  time_t next_valid_time = 0L;
  int schedule_check = 0;
  double max_inter_check_delay = 0.0;
  struct timeval tv[9];
  double runtime[9];

  logger(dbg_functions, basic) << "init_timing_loop()";

  /* get the time right now */
  time(&current_time);

  /******** GET BASIC HOST/SERVICE INFO  ********/

  scheduling_info.total_services = 0;
  scheduling_info.total_scheduled_services = 0;
  scheduling_info.total_hosts = 0;
  scheduling_info.total_scheduled_hosts = 0;
  scheduling_info.average_services_per_host = 0.0;
  scheduling_info.average_scheduled_services_per_host = 0.0;
  scheduling_info.average_service_execution_time = 0.0;
  scheduling_info.service_check_interval_total = 0;
  scheduling_info.average_service_inter_check_delay = 0.0;
  scheduling_info.host_check_interval_total = 0;
  scheduling_info.average_host_inter_check_delay = 0.0;

  if (test_scheduling == true)
    gettimeofday(&tv[0], NULL);

  /* get info on service checks to be scheduled */
  for (temp_service = service_list; temp_service != NULL;
       temp_service = temp_service->next) {
    schedule_check = true;

    /* service has no check interval */
    if (temp_service->check_interval == 0)
      schedule_check = false;

    /* active checks are disabled */
    if (temp_service->checks_enabled == false)
      schedule_check = false;

    /* are there any valid times this service can be checked? */
    is_valid_time =
        check_time_against_period(current_time, temp_service->check_period_ptr);
    if (is_valid_time == ERROR) {
      get_next_valid_time(current_time, &next_valid_time,
                          temp_service->check_period_ptr);
      if (current_time == next_valid_time)
        schedule_check = false;
    }

    if (schedule_check == true) {
      scheduling_info.total_scheduled_services++;

      /* used later in inter-check delay calculations */
      scheduling_info.service_check_interval_total +=
          static_cast<unsigned long>(temp_service->check_interval);

      /* calculate rolling average execution time (available from retained state
       * information) */
      scheduling_info.average_service_execution_time =
          (double)(((scheduling_info.average_service_execution_time *
                     (scheduling_info.total_scheduled_services - 1)) +
                    temp_service->execution_time) /
                   (double)scheduling_info.total_scheduled_services);
    } else {
      temp_service->should_be_scheduled = false;

      logger(dbg_events, more)
          << "Service '" << temp_service->description << "' on host '"
          << temp_service->host_name << "' should not be scheduled.";
    }

    scheduling_info.total_services++;
  }

  if (test_scheduling == true)
    gettimeofday(&tv[1], NULL);

  /* get info on host checks to be scheduled */
  for (temp_host = host_list; temp_host != NULL; temp_host = temp_host->next) {
    schedule_check = true;

    /* host has no check interval */
    if (temp_host->check_interval == 0)
      schedule_check = false;

    /* active checks are disabled */
    if (temp_host->checks_enabled == false)
      schedule_check = false;

    /* are there any valid times this host can be checked? */
    is_valid_time =
        check_time_against_period(current_time, temp_host->check_period_ptr);
    if (is_valid_time == ERROR) {
      get_next_valid_time(current_time, &next_valid_time,
                          temp_host->check_period_ptr);
      if (current_time == next_valid_time)
        schedule_check = false;
    }

    if (schedule_check == true) {
      scheduling_info.total_scheduled_hosts++;

      /* this is used later in inter-check delay calculations */
      scheduling_info.host_check_interval_total +=
          static_cast<unsigned long>(temp_host->check_interval);
    } else {
      temp_host->should_be_scheduled = false;

      logger(dbg_events, more)
          << "Host '" << temp_host->name << "' should not be scheduled.";
    }

    scheduling_info.total_hosts++;
  }

  if (test_scheduling == true)
    gettimeofday(&tv[2], NULL);

  if (scheduling_info.total_hosts == 0) {
    scheduling_info.average_services_per_host = 0;
    scheduling_info.average_scheduled_services_per_host = 0;
  } else {
    scheduling_info.average_services_per_host =
        (double)((double)scheduling_info.total_services /
                 (double)scheduling_info.total_hosts);
    scheduling_info.average_scheduled_services_per_host =
        (double)((double)scheduling_info.total_scheduled_services /
                 (double)scheduling_info.total_hosts);
  }

  /* adjust the check interval total to correspond to the interval length */
  scheduling_info.service_check_interval_total =
      scheduling_info.service_check_interval_total * ::interval_length;

  /* calculate the average check interval for services */
  if (scheduling_info.total_scheduled_services == 0)
    scheduling_info.average_service_check_interval = 0;
  else {
    scheduling_info.average_service_check_interval =
        (double)((double)scheduling_info.service_check_interval_total /
                 (double)scheduling_info.total_scheduled_services);
  }

  /******** DETERMINE SERVICE SCHEDULING PARAMS  ********/

  logger(dbg_events, most) << "Determining service scheduling parameters...";

  /* default max service check spread (in minutes) */
  scheduling_info.max_service_check_spread = ::max_service_check_spread;

  /* how should we determine the service inter-check delay to use? */
  switch (::service_inter_check_delay_method) {
    case configuration::state::icd_none:
      /* don't spread checks out - useful for testing parallelization code */
      scheduling_info.service_inter_check_delay = 0.0;
      break;

    case configuration::state::icd_dumb:
      /* be dumb and just schedule checks 1 second apart */
      scheduling_info.service_inter_check_delay = 1.0;
      break;

    case configuration::state::icd_user:
      /* the user specified a delay, so don't try to calculate one */
      break;

    case configuration::state::icd_smart:
    default:
      /* be smart and calculate the best delay to use to minimize local load...
       */
      if (scheduling_info.total_scheduled_services > 0 &&
          scheduling_info.service_check_interval_total > 0) {
        /* calculate the average inter check delay (in seconds) needed to evenly
         * space the service checks out */
        scheduling_info.average_service_inter_check_delay =
            (double)(scheduling_info.average_service_check_interval /
                     (double)scheduling_info.total_scheduled_services);

        /* set the global inter check delay value */
        scheduling_info.service_inter_check_delay =
            scheduling_info.average_service_inter_check_delay;

        /* calculate max inter check delay and see if we should use that instead
         */
        max_inter_check_delay =
            (double)((scheduling_info.max_service_check_spread * 60.0) /
                     (double)scheduling_info.total_scheduled_services);
        if (scheduling_info.service_inter_check_delay > max_inter_check_delay)
          scheduling_info.service_inter_check_delay = max_inter_check_delay;
      } else
        scheduling_info.service_inter_check_delay = 0.0;

      logger(dbg_events, more) << "Total scheduled service checks:  "
                               << scheduling_info.total_scheduled_services;
      logger(dbg_events, more)
          << com::centreon::logging::setprecision(2)
          << "Average service check interval:  "
          << scheduling_info.average_service_check_interval << " sec";
      logger(dbg_events, more)
          << com::centreon::logging::setprecision(2)
          << "Service inter-check delay:       "
          << scheduling_info.service_inter_check_delay << " sec";
  }

  /* how should we determine the service interleave factor? */
  switch (::service_interleave_factor_method) {
    case configuration::state::ilf_user:
      /* the user supplied a value, so don't do any calculation */
      break;

    case configuration::state::ilf_smart:
    default:
      scheduling_info.service_interleave_factor =
          (int)(ceil(scheduling_info.average_scheduled_services_per_host));

      logger(dbg_events, more) << "Total scheduled service checks: "
                               << scheduling_info.total_scheduled_services;
      logger(dbg_events, more)
          << "Total hosts:                    " << scheduling_info.total_hosts;
      logger(dbg_events, more) << "Service Interleave factor:      "
                               << scheduling_info.service_interleave_factor;
  }

  /* calculate number of service interleave blocks */
  if (scheduling_info.service_interleave_factor == 0)
    total_interleave_blocks = scheduling_info.total_scheduled_services;
  else
    total_interleave_blocks =
        (int)ceil((double)scheduling_info.total_scheduled_services /
                  (double)scheduling_info.service_interleave_factor);

  scheduling_info.first_service_check = (time_t)0L;
  scheduling_info.last_service_check = (time_t)0L;

  logger(dbg_events, more) << "Total scheduled services: "
                           << scheduling_info.total_scheduled_services;
  logger(dbg_events, more) << "Service Interleave factor: "
                           << scheduling_info.service_interleave_factor;
  logger(dbg_events, more) << "Total service interleave blocks: "
                           << total_interleave_blocks;
  logger(dbg_events, more) << com::centreon::logging::setprecision(1)
                           << "Service inter-check delay: "
                           << scheduling_info.service_inter_check_delay;

  if (test_scheduling == true)
    gettimeofday(&tv[3], NULL);

  /******** SCHEDULE SERVICE CHECKS  ********/

  logger(dbg_events, most) << "Scheduling service checks...";

  /* determine check times for service checks (with interleaving to minimize
   * remote load) */
  current_interleave_block = 0;
  for (temp_service = service_list;
       temp_service != NULL && scheduling_info.service_interleave_factor > 0;) {
    logger(dbg_events, most)
        << "Current Interleave Block: " << current_interleave_block;

    for (interleave_block_index = 0;
         interleave_block_index < scheduling_info.service_interleave_factor &&
         temp_service != NULL;
         temp_service = temp_service->next) {
      logger(dbg_events, most)
          << "Service '" << temp_service->description << "' on host '"
          << temp_service->host_name << "'";

      /* skip this service if it shouldn't be scheduled */
      if (temp_service->should_be_scheduled == false) {
        logger(dbg_events, most) << "Service check should not be scheduled.";
        continue;
      }

      /* skip services that are already scheduled for the future (from retention
       * data), but reschedule ones that were supposed to happen while we
       * weren't running... */
      if (temp_service->next_check > current_time) {
        logger(dbg_events, most)
            << "Service is already scheduled to be checked in "
               "the future: "
            << my_ctime(&temp_service->next_check);
        continue;
      }

      /* interleave block index should only be increased when we find a
       * schedulable service */
      /* moved from for() loop 11/05/05 EG */
      interleave_block_index++;

      mult_factor = current_interleave_block +
                    (interleave_block_index * total_interleave_blocks);

      logger(dbg_events, most)
          << "CIB: " << current_interleave_block
          << ", IBI: " << interleave_block_index
          << ", TIB: " << total_interleave_blocks
          << ", SIF: " << scheduling_info.service_interleave_factor;

      logger(dbg_events, most) << "Mult factor: " << mult_factor;

      /* set the preferred next check time for the service */
      temp_service->next_check =
          (time_t)(current_time +
                   (mult_factor * scheduling_info.service_inter_check_delay));

      logger(dbg_events, most)
          << "Preferred Check Time: " << temp_service->next_check << " --> "
          << my_ctime(&temp_service->next_check);

      /* make sure the service can actually be scheduled when we want */
      is_valid_time = check_time_against_period(temp_service->next_check,
                                                temp_service->check_period_ptr);
      if (is_valid_time == ERROR) {
        logger(dbg_events, most) << "Preferred Time is Invalid In Timeperiod '"
                                 << temp_service->check_period_ptr->name
                                 << "': " << temp_service->next_check << " --> "
                                 << my_ctime(&temp_service->next_check);

        get_next_valid_time(temp_service->next_check, &next_valid_time,
                            temp_service->check_period_ptr);
        temp_service->next_check = next_valid_time;
      }

      logger(dbg_events, most)
          << "Actual Check Time: " << temp_service->next_check << " --> "
          << my_ctime(&temp_service->next_check);

      if (scheduling_info.first_service_check == (time_t)0 ||
          (temp_service->next_check < scheduling_info.first_service_check))
        scheduling_info.first_service_check = temp_service->next_check;
      if (temp_service->next_check > scheduling_info.last_service_check)
        scheduling_info.last_service_check = temp_service->next_check;
    }

    current_interleave_block++;
  }

  if (test_scheduling == true)
    gettimeofday(&tv[4], NULL);

  /* add scheduled service checks to event queue */
  for (temp_service = service_list; temp_service != NULL;
       temp_service = temp_service->next) {
    /* Nagios XI/NDOUtils MOD */
    /* update status of all services (scheduled or not) */
    update_service_status(temp_service, false);

    /* skip most services that shouldn't be scheduled */
    if (temp_service->should_be_scheduled == false) {
      /* passive checks are an exception if a forced check was scheduled before
       * Centreon Engine was restarted */
      if (!(temp_service->checks_enabled == false &&
            temp_service->next_check != (time_t)0L &&
            (temp_service->check_options & CHECK_OPTION_FORCE_EXECUTION)))
        continue;
    }

    /* create a new service check event */
    schedule_new_event(EVENT_SERVICE_CHECK, false, temp_service->next_check,
                       false, 0, NULL, true, (void*)temp_service, NULL,
                       temp_service->check_options);
  }

  if (test_scheduling == true)
    gettimeofday(&tv[5], NULL);

  /******** DETERMINE HOST SCHEDULING PARAMS  ********/

  logger(dbg_events, most) << "Determining host scheduling parameters...";

  scheduling_info.first_host_check = (time_t)0L;
  scheduling_info.last_host_check = (time_t)0L;

  /* default max host check spread (in minutes) */
  scheduling_info.max_host_check_spread = ::max_host_check_spread;

  /* how should we determine the host inter-check delay to use? */
  switch (::host_inter_check_delay_method) {
    case configuration::state::icd_none:
      /* don't spread checks out */
      scheduling_info.host_inter_check_delay = 0.0;
      break;

    case configuration::state::icd_dumb:
      /* be dumb and just schedule checks 1 second apart */
      scheduling_info.host_inter_check_delay = 1.0;
      break;

    case configuration::state::icd_user:
      /* the user specified a delay, so don't try to calculate one */
      break;

    case configuration::state::icd_smart:
    default:
      /* be smart and calculate the best delay to use to minimize local load...
       */
      if (scheduling_info.total_scheduled_hosts > 0 &&
          scheduling_info.host_check_interval_total > 0) {
        /* adjust the check interval total to correspond to the interval length
         */
        scheduling_info.host_check_interval_total =
            scheduling_info.host_check_interval_total * ::interval_length;

        /* calculate the average check interval for hosts */
        scheduling_info.average_host_check_interval =
            (double)((double)scheduling_info.host_check_interval_total /
                     (double)scheduling_info.total_scheduled_hosts);

        /* calculate the average inter check delay (in seconds) needed to evenly
         * space the host checks out */
        scheduling_info.average_host_inter_check_delay =
            (double)(scheduling_info.average_host_check_interval /
                     (double)scheduling_info.total_scheduled_hosts);

        /* set the global inter check delay value */
        scheduling_info.host_inter_check_delay =
            scheduling_info.average_host_inter_check_delay;

        /* calculate max inter check delay and see if we should use that instead
         */
        max_inter_check_delay =
            (double)((scheduling_info.max_host_check_spread * 60.0) /
                     (double)scheduling_info.total_scheduled_hosts);
        if (scheduling_info.host_inter_check_delay > max_inter_check_delay)
          scheduling_info.host_inter_check_delay = max_inter_check_delay;
      } else
        scheduling_info.host_inter_check_delay = 0.0;

      logger(dbg_events, most) << "Total scheduled host checks:  "
                               << scheduling_info.total_scheduled_hosts;
      logger(dbg_events, most) << "Host check interval total:    "
                               << scheduling_info.host_check_interval_total;
      logger(dbg_events, most)
          << com::centreon::logging::setprecision(2)
          << "Average host check interval:  "
          << scheduling_info.average_host_check_interval << " sec";
      logger(dbg_events, most)
          << com::centreon::logging::setprecision(2)
          << "Host inter-check delay:       "
          << scheduling_info.host_inter_check_delay << " sec";
  }

  if (test_scheduling == true)
    gettimeofday(&tv[6], NULL);

  /******** SCHEDULE HOST CHECKS  ********/

  logger(dbg_events, most) << "Scheduling host checks...";

  /* determine check times for host checks */
  mult_factor = 0;
  for (temp_host = host_list; temp_host != NULL; temp_host = temp_host->next) {
    logger(dbg_events, most) << "Host '" << temp_host->name << "'";

    /* skip hosts that shouldn't be scheduled */
    if (temp_host->should_be_scheduled == false) {
      logger(dbg_events, most) << "Host check should not be scheduled.";
      continue;
    }

    /* skip hosts that are already scheduled for the future (from retention
     * data), but reschedule ones that were supposed to be checked before we
     * started */
    if (temp_host->next_check > current_time) {
      logger(dbg_events, most)
          << "Host is already scheduled to be checked in the future: "
          << my_ctime(&temp_host->next_check);
      continue;
    }

    /* calculate preferred host check time */
    temp_host->next_check = (time_t)(
        current_time + (mult_factor * scheduling_info.host_inter_check_delay));

    logger(dbg_events, most)
        << "Preferred Check Time: " << temp_host->next_check << " --> "
        << my_ctime(&temp_host->next_check);

    /* make sure the host can actually be scheduled at this time */
    is_valid_time = check_time_against_period(temp_host->next_check,
                                              temp_host->check_period_ptr);
    if (is_valid_time == ERROR) {
      get_next_valid_time(temp_host->next_check, &next_valid_time,
                          temp_host->check_period_ptr);
      temp_host->next_check = next_valid_time;
    }

    logger(dbg_events, most) << "Actual Check Time: " << temp_host->next_check
                             << " --> " << my_ctime(&temp_host->next_check);

    if (scheduling_info.first_host_check == (time_t)0 ||
        (temp_host->next_check < scheduling_info.first_host_check))
      scheduling_info.first_host_check = temp_host->next_check;
    if (temp_host->next_check > scheduling_info.last_host_check)
      scheduling_info.last_host_check = temp_host->next_check;

    mult_factor++;
  }

  if (test_scheduling == true)
    gettimeofday(&tv[7], NULL);

  /* add scheduled host checks to event queue */
  for (temp_host = host_list; temp_host != NULL; temp_host = temp_host->next) {
    /* Nagios XI/NDOUtils Mod */
    /* update status of all hosts (scheduled or not) */
    update_host_status(temp_host, false);

    /* skip most hosts that shouldn't be scheduled */
    if (temp_host->should_be_scheduled == false) {
      /* passive checks are an exception if a forced check was scheduled before
       * Centreon Engine was restarted */
      if (!(temp_host->checks_enabled == false &&
            temp_host->next_check != (time_t)0L &&
            (temp_host->check_options & CHECK_OPTION_FORCE_EXECUTION)))
        continue;
    }

    /* schedule a new host check event */
    schedule_new_event(EVENT_HOST_CHECK, false, temp_host->next_check, false, 0,
                       NULL, true, (void*)temp_host, NULL,
                       temp_host->check_options);
  }

  if (test_scheduling == true)
    gettimeofday(&tv[8], NULL);

  /******** SCHEDULE MISC EVENTS ********/

  /* add a check result reaper event */
  schedule_new_event(EVENT_CHECK_REAPER, true,
                     current_time + ::check_reaper_interval, true,
                     ::check_reaper_interval, NULL, true, NULL, NULL, 0);

  /* add an external command check event if needed */
  if (::check_external_commands == true) {
    if (::command_check_interval == -1)
      interval_to_use = (unsigned long)5;
    else
      interval_to_use = (unsigned long)::command_check_interval;
    schedule_new_event(EVENT_COMMAND_CHECK, true,
                       current_time + interval_to_use, true, interval_to_use,
                       NULL, true, NULL, NULL, 0);
  }

  /* add a host result "freshness" check event */
  if (::check_host_freshness == true)
    schedule_new_event(EVENT_HFRESHNESS_CHECK, true,
                       current_time + ::host_freshness_check_interval, true,
                       ::host_freshness_check_interval, NULL, true, NULL, NULL,
                       0);

  /* add an orphaned check event */
  if (::check_orphaned_services == true || ::check_orphaned_hosts == true)
    schedule_new_event(
        EVENT_ORPHAN_CHECK, true, current_time + DEFAULT_ORPHAN_CHECK_INTERVAL,
        true, DEFAULT_ORPHAN_CHECK_INTERVAL, NULL, true, NULL, NULL, 0);

  /* add a host and service check rescheduling event */
  if (::auto_reschedule_checks == true)
    schedule_new_event(EVENT_RESCHEDULE_CHECKS, true,
                       current_time + ::auto_rescheduling_interval, true,
                       ::auto_rescheduling_interval, NULL, true, NULL, NULL, 0);

  /* add a retention data save event if needed */
  if (::retain_state_information == true && ::retention_update_interval > 0)
    schedule_new_event(EVENT_RETENTION_SAVE, true,
                       current_time + (::retention_update_interval * 60), true,
                       (::retention_update_interval * 60), NULL, true, NULL,
                       NULL, 0);

  /* add a service result "freshness" check event */
  if (::check_service_freshness == true)
    schedule_new_event(EVENT_SFRESHNESS_CHECK, true,
                       current_time + ::service_freshness_check_interval, true,
                       ::service_freshness_check_interval, NULL, true, NULL,
                       NULL, 0);

  /* add a status save event */
  schedule_new_event(EVENT_STATUS_SAVE, true,
                     current_time + ::status_update_interval, true,
                     ::status_update_interval, NULL, true, NULL, NULL, 0);

  if (test_scheduling == true) {
    runtime[0] =
        (double)((double)(tv[1].tv_sec - tv[0].tv_sec) +
                 (double)((tv[1].tv_usec - tv[0].tv_usec) / 1000.0) / 1000.0);
    runtime[1] =
        (double)((double)(tv[2].tv_sec - tv[1].tv_sec) +
                 (double)((tv[2].tv_usec - tv[1].tv_usec) / 1000.0) / 1000.0);
    runtime[2] =
        (double)((double)(tv[3].tv_sec - tv[2].tv_sec) +
                 (double)((tv[3].tv_usec - tv[2].tv_usec) / 1000.0) / 1000.0);
    runtime[3] =
        (double)((double)(tv[4].tv_sec - tv[3].tv_sec) +
                 (double)((tv[4].tv_usec - tv[3].tv_usec) / 1000.0) / 1000.0);
    runtime[4] =
        (double)((double)(tv[5].tv_sec - tv[4].tv_sec) +
                 (double)((tv[5].tv_usec - tv[4].tv_usec) / 1000.0) / 1000.0);
    runtime[5] =
        (double)((double)(tv[6].tv_sec - tv[5].tv_sec) +
                 (double)((tv[6].tv_usec - tv[5].tv_usec) / 1000.0) / 1000.0);
    runtime[6] =
        (double)((double)(tv[7].tv_sec - tv[6].tv_sec) +
                 (double)((tv[7].tv_usec - tv[6].tv_usec) / 1000.0) / 1000.0);
    runtime[7] =
        (double)((double)(tv[8].tv_sec - tv[7].tv_sec) +
                 (double)((tv[8].tv_usec - tv[7].tv_usec) / 1000.0) / 1000.0);
    runtime[8] =
        (double)((double)(tv[8].tv_sec - tv[0].tv_sec) +
                 (double)((tv[8].tv_usec - tv[0].tv_usec) / 1000.0) / 1000.0);

    printf("EVENT SCHEDULING TIMES\n");
    printf("-------------------------------------\n");
    printf("Get service info:        %.6f sec\n", runtime[0]);
    printf("Get host info info:      %.6f sec\n", runtime[1]);
    printf("Get service params:      %.6f sec\n", runtime[2]);
    printf("Schedule service times:  %.6f sec\n", runtime[3]);
    printf("Schedule service events: %.6f sec\n", runtime[4]);
    printf("Get host params:         %.6f sec\n", runtime[5]);
    printf("Schedule host times:     %.6f sec\n", runtime[6]);
    printf("Schedule host events:    %.6f sec\n", runtime[7]);
    printf("                         ============\n");
    printf("TOTAL:                   %.6f sec\n", runtime[8]);
    printf("\n\n");
  }

  logger(dbg_functions, basic) << "init_timing_loop()";
  return;
}

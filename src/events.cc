/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011      Merethis
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

#include <QCoreApplication>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include "engine.hh"
#include "downtime.hh"
#include "comments.hh"
#include "globals.hh"
#include "statusdata.hh"
#include "broker.hh"
#include "sretention.hh"
#include "utils.hh"
#include "notifications.hh"
#include "logging/file.hh"
#include "logging/logger.hh"
#include "events.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/******************************************************************/
/************ EVENT SCHEDULING/HANDLING FUNCTIONS *****************/
/******************************************************************/

static void _exec_event_service_check(timed_event* event) {
  service* svc = reinterpret_cast<service*>(event->event_data);

  /* get check latency */
  timeval tv;
  gettimeofday(&tv, NULL);
  double latency = (double)((double)(tv.tv_sec - event->run_time)
                            + (double)(tv.tv_usec / 1000) / 1000.0);

  logger(dbg_events, basic)
    << "** Service Check Event ==> Host: '" << svc->host_name
    << "', Service: '" << svc->description << "', Options: "
    << event->event_options << ", Latency: " << latency << " sec";

  /* run the service check */
  run_scheduled_service_check(svc, event->event_options, latency);
}

static void _exec_event_command_check(timed_event* event) {
  (void)event;
  logger(dbg_events, basic) << "** External Command Check Event";

  /* send data to event broker */
  broker_external_command(NEBTYPE_EXTERNALCOMMAND_CHECK,
                          NEBFLAG_NONE,
                          NEBATTR_NONE,
                          CMD_NONE,
                          time(NULL),
                          NULL,
                          NULL,
                          NULL);
}

static void _exec_event_log_rotation(timed_event* event) {
  (void)event;

  logger(dbg_events, basic) << "** Log File Rotation Event";

  /* rotate the log file */
  file::rotate_all();
}

static void _exec_event_program_shutdown(timed_event* event) {
  (void)event;
  logger(dbg_events, basic) << "** Program Shutdown Event";

  /* set the shutdown flag */
  sigshutdown = TRUE;

  /* log the shutdown */
  logger(log_process_info, basic)
    << "PROGRAM_SHUTDOWN event encountered, shutting down...";
}

static void _exec_event_program_restart(timed_event* event) {
  (void)event;
  logger(dbg_events, basic) << "** Program Restart Event";

  /* set the restart flag */
  sigrestart = TRUE;

  /* log the restart */
  logger(log_process_info, basic)
    << "PROGRAM_RESTART event encountered, restarting...";
}

static void _exec_event_check_reaper(timed_event* event) {
  (void)event;
  logger(dbg_events, basic) << "** Check Result Reaper";

  /* reap host and service check results */
  reap_check_results();
}

static void _exec_event_orphan_check(timed_event* event) {
  (void)event;
  logger(dbg_events, basic) << "** Orphaned Host and Service Check Event";

  /* check for orphaned hosts and services */
  if (config.get_check_orphaned_hosts() == true)
    check_for_orphaned_hosts();
  if (config.get_check_orphaned_services() == true)
    check_for_orphaned_services();
}

static void _exec_event_retention_save(timed_event* event) {
  (void)event;
  logger(dbg_events, basic) << "** Retention Data Save Event";

  /* save state retention data */
  save_state_information(TRUE);
}

static void _exec_event_status_save(timed_event* event) {
  (void)event;
  logger(dbg_events, basic) << "** Status Data Save Event";

  /* save all status data (program, host, and service) */
  update_all_status_data();
}

static void _exec_event_scheduled_downtime(timed_event* event) {
  logger(dbg_events, basic) << "** Scheduled Downtime Event";

  /* process scheduled downtime info */
  if (event->event_data) {
    handle_scheduled_downtime_by_id(*(unsigned long*)event->event_data);
    delete static_cast<unsigned long*>(event->event_data);
    event->event_data = NULL;
  }
}

static void _exec_event_sfreshness_check(timed_event* event) {
  (void)event;
  logger(dbg_events, basic) << "** Service Result Freshness Check Event";

  /* check service result freshness */
  check_service_result_freshness();
}

static void _exec_event_expire_downtime(timed_event* event) {
  (void)event;
  logger(dbg_events, basic) << "** Expire Downtime Event";

  /* check for expired scheduled downtime entries */
  check_for_expired_downtime();
}

static void _exec_event_host_check(timed_event* event) {
  host* hst = reinterpret_cast<host*>(event->event_data);

  /* get check latency */
  timeval tv;
  gettimeofday(&tv, NULL);
  double latency = (double)((double)(tv.tv_sec - event->run_time)
                            + (double)(tv.tv_usec / 1000) / 1000.0);

  logger(dbg_events, basic)
    << "** Host Check Event ==> Host: '" << hst->name
    << "', Options: " << event->event_options
    << ", Latency: " << latency << " sec";

  /* run the host check */
  perform_scheduled_host_check(hst, event->event_options, latency);
}

static void _exec_event_hfreshness_check(timed_event* event) {
  (void)event;
  logger(dbg_events, basic) << "** Host Result Freshness Check Event";

  /* check host result freshness */
  check_host_result_freshness();
}

static void _exec_event_reschedule_checks(timed_event* event) {
  (void)event;
  logger(dbg_events, basic) << "** Reschedule Checks Event";

  /* adjust scheduling of host and service checks */
  adjust_check_scheduling();
}

static void _exec_event_expire_comment(timed_event* event) {
  logger(dbg_events, basic) << "** Expire Comment Event";

  /* check for expired comment */
  check_for_expired_comment((unsigned long)event->event_data);
}

static void _exec_event_user_function(timed_event* event) {
  logger(dbg_events, basic) << "** User Function Event";

  /* run a user-defined function */
  if (event->event_data != NULL) {
    void (*userfunc)(void*);
    *(void**)(&userfunc) = event->event_data;
    (*userfunc)(event->event_args);
  }
}

/* initialize the event timing loop before we start monitoring */
void init_timing_loop(void) {
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

  if (test_scheduling == TRUE)
    gettimeofday(&tv[0], NULL);

  /* get info on service checks to be scheduled */
  for (temp_service = service_list;
       temp_service != NULL;
       temp_service = temp_service->next) {

    schedule_check = TRUE;

    /* service has no check interval */
    if (temp_service->check_interval == 0)
      schedule_check = FALSE;

    /* active checks are disabled */
    if (temp_service->checks_enabled == FALSE)
      schedule_check = FALSE;

    /* are there any valid times this service can be checked? */
    is_valid_time = check_time_against_period(current_time, temp_service->check_period_ptr);
    if (is_valid_time == ERROR) {
      get_next_valid_time(current_time, &next_valid_time, temp_service->check_period_ptr);
      if (current_time == next_valid_time)
        schedule_check = FALSE;
    }

    if (schedule_check == TRUE) {
      scheduling_info.total_scheduled_services++;

      /* used later in inter-check delay calculations */
      scheduling_info.service_check_interval_total += static_cast<unsigned long>(temp_service->check_interval);

      /* calculate rolling average execution time (available from retained state information) */
      scheduling_info.average_service_execution_time =
        (double)(((scheduling_info.average_service_execution_time
                   * (scheduling_info.total_scheduled_services - 1))
                  + temp_service->execution_time)
		 / (double)scheduling_info.total_scheduled_services);
    }
    else {
      temp_service->should_be_scheduled = FALSE;

      logger(dbg_events, more)
        << "Service '" << temp_service->description
        << "' on host '" << temp_service->host_name
        << "' should not be scheduled.";
    }

    scheduling_info.total_services++;
  }

  if (test_scheduling == TRUE)
    gettimeofday(&tv[1], NULL);

  /* get info on host checks to be scheduled */
  for (temp_host = host_list; temp_host != NULL; temp_host = temp_host->next) {

    schedule_check = TRUE;

    /* host has no check interval */
    if (temp_host->check_interval == 0)
      schedule_check = FALSE;

    /* active checks are disabled */
    if (temp_host->checks_enabled == FALSE)
      schedule_check = FALSE;

    /* are there any valid times this host can be checked? */
    is_valid_time = check_time_against_period(current_time, temp_host->check_period_ptr);
    if (is_valid_time == ERROR) {
      get_next_valid_time(current_time, &next_valid_time, temp_host->check_period_ptr);
      if (current_time == next_valid_time)
        schedule_check = FALSE;
    }

    if (schedule_check == TRUE) {

      scheduling_info.total_scheduled_hosts++;

      /* this is used later in inter-check delay calculations */
      scheduling_info.host_check_interval_total += static_cast<unsigned long>(temp_host->check_interval);
    }
    else {
      temp_host->should_be_scheduled = FALSE;

      logger(dbg_events, more)
        << "Host '" << temp_host->name << "' should not be scheduled.";
    }

    scheduling_info.total_hosts++;
  }

  if (test_scheduling == TRUE)
    gettimeofday(&tv[2], NULL);

  scheduling_info.average_services_per_host =
    (double)((double)scheduling_info.total_services / (double)scheduling_info.total_hosts);
  scheduling_info.average_scheduled_services_per_host =
    (double)((double)scheduling_info.total_scheduled_services / (double)scheduling_info.total_hosts);

  /* adjust the check interval total to correspond to the interval length */
  scheduling_info.service_check_interval_total =
    (scheduling_info.service_check_interval_total * config.get_interval_length());

  /* calculate the average check interval for services */
  scheduling_info.average_service_check_interval =
    (double)((double)scheduling_info.service_check_interval_total
             / (double)scheduling_info.total_scheduled_services);

  /******** DETERMINE SERVICE SCHEDULING PARAMS  ********/

  logger(dbg_events, most) << "Determining service scheduling parameters...";

  /* default max service check spread (in minutes) */
  scheduling_info.max_service_check_spread = config.get_max_service_check_spread();

  /* how should we determine the service inter-check delay to use? */
  switch (config.get_service_inter_check_delay_method()) {

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
    /* be smart and calculate the best delay to use to minimize local load... */
    if (scheduling_info.total_scheduled_services > 0
        && scheduling_info.service_check_interval_total > 0) {

      /* calculate the average inter check delay (in seconds) needed to evenly space the service checks out */
      scheduling_info.average_service_inter_check_delay =
        (double)(scheduling_info.average_service_check_interval
                 / (double)scheduling_info.total_scheduled_services);

      /* set the global inter check delay value */
      scheduling_info.service_inter_check_delay = scheduling_info.average_service_inter_check_delay;

      /* calculate max inter check delay and see if we should use that instead */
      max_inter_check_delay =
        (double)((scheduling_info.max_service_check_spread * 60.0)
                 / (double)scheduling_info.total_scheduled_services);
      if (scheduling_info.service_inter_check_delay > max_inter_check_delay)
        scheduling_info.service_inter_check_delay = max_inter_check_delay;
    }
    else
      scheduling_info.service_inter_check_delay = 0.0;

    logger(dbg_events, more)
      << "Total scheduled service checks:  "
      << scheduling_info.total_scheduled_services;
    logger(dbg_events, more)
      << fixed << setprecision(2)
      << "Average service check interval:  "
      << scheduling_info.average_service_check_interval << " sec";
    logger(dbg_events, more)
      << fixed << setprecision(2)
      << "Service inter-check delay:       "
      << scheduling_info.service_inter_check_delay << " sec";
  }

  /* how should we determine the service interleave factor? */
  switch (config.get_service_interleave_factor_method()) {

  case configuration::state::ilf_user:
    /* the user supplied a value, so don't do any calculation */
    break;

  case configuration::state::ilf_smart:
  default:
    /* protect against a divide by zero problem - shouldn't happen, but just in case... */
    if (scheduling_info.total_hosts == 0)
      scheduling_info.total_hosts = 1;

    scheduling_info.service_interleave_factor =
      (int)(ceil(scheduling_info.average_scheduled_services_per_host));

    logger(dbg_events, more)
      << "Total scheduled service checks: "
      << scheduling_info.total_scheduled_services;
    logger(dbg_events, more)
      << "Total hosts:                    "
      << scheduling_info.total_hosts;
    logger(dbg_events, more)
      << "Service Interleave factor:      "
      << scheduling_info.service_interleave_factor;
  }

  /* calculate number of service interleave blocks */
  if (scheduling_info.service_interleave_factor == 0)
    total_interleave_blocks = scheduling_info.total_scheduled_services;
  else
    total_interleave_blocks =
      (int)ceil((double)scheduling_info.total_scheduled_services
                / (double)scheduling_info.service_interleave_factor);

  scheduling_info.first_service_check = (time_t) 0L;
  scheduling_info.last_service_check = (time_t) 0L;

  logger(dbg_events, more)
    << "Total scheduled services: "
    << scheduling_info.total_scheduled_services;
  logger(dbg_events, more)
    << "Service Interleave factor: "
    << scheduling_info.service_interleave_factor;
  logger(dbg_events, more)
    << "Total service interleave blocks: "
    << total_interleave_blocks;
  logger(dbg_events, more)
    << fixed << setprecision(1)
    << "Service inter-check delay: "
    << scheduling_info.service_inter_check_delay;

  if (test_scheduling == TRUE)
    gettimeofday(&tv[3], NULL);

  /******** SCHEDULE SERVICE CHECKS  ********/

  logger(dbg_events, most) << "Scheduling service checks...";

  /* determine check times for service checks (with interleaving to minimize remote load) */
  current_interleave_block = 0;
  for (temp_service = service_list;
       temp_service != NULL && scheduling_info.service_interleave_factor > 0;) {

    logger(dbg_events, most)
      << "Current Interleave Block: " << current_interleave_block;

    for (interleave_block_index = 0;
         interleave_block_index < scheduling_info.service_interleave_factor && temp_service != NULL;
	 temp_service = temp_service->next) {

      logger(dbg_events, most)
        << "Service '" << temp_service->description
        << "' on host '" << temp_service->host_name << "'";

      /* skip this service if it shouldn't be scheduled */
      if (temp_service->should_be_scheduled == FALSE) {
        logger(dbg_events, most) << "Service check should not be scheduled.";
        continue;
      }

      /* skip services that are already scheduled for the future (from retention data), but reschedule ones that were supposed to happen while we weren't running... */
      if (temp_service->next_check > current_time) {
        logger(dbg_events, most)
          << "Service is already scheduled to be checked in the future: "
          << my_ctime(&temp_service->next_check);
        continue;
      }

      /* interleave block index should only be increased when we find a schedulable service */
      /* moved from for() loop 11/05/05 EG */
      interleave_block_index++;

      mult_factor = current_interleave_block + (interleave_block_index * total_interleave_blocks);

      logger(dbg_events, most)
        << "CIB: " << current_interleave_block
        << ", IBI: " << interleave_block_index
        << ", TIB: " << total_interleave_blocks
        << ", SIF: " << scheduling_info.service_interleave_factor;

      logger(dbg_events, most) << "Mult factor: " << mult_factor;

      /* set the preferred next check time for the service */
      temp_service->next_check =
	(time_t)(current_time + (mult_factor * scheduling_info.service_inter_check_delay));

      logger(dbg_events, most)
        << "Preferred Check Time: " << temp_service->next_check
        << " --> " << my_ctime(&temp_service->next_check);

      /* make sure the service can actually be scheduled when we want */
      is_valid_time = check_time_against_period(temp_service->next_check,
						temp_service->check_period_ptr);
      if (is_valid_time == ERROR) {
        logger(dbg_events, most)
          << "Preferred Time is Invalid In Timeperiod '"
          << temp_service->check_period_ptr->name
          << "': " << temp_service->next_check
          << " --> " << my_ctime(&temp_service->next_check);

        get_next_valid_time(temp_service->next_check, &next_valid_time,
			    temp_service->check_period_ptr);
        temp_service->next_check = next_valid_time;
      }

      logger(dbg_events, most)
        << "Actual Check Time: " << temp_service->next_check
        << " --> " << my_ctime(&temp_service->next_check);

      if (scheduling_info.first_service_check == (time_t)0
          || (temp_service->next_check < scheduling_info.first_service_check))
        scheduling_info.first_service_check = temp_service->next_check;
      if (temp_service->next_check > scheduling_info.last_service_check)
        scheduling_info.last_service_check = temp_service->next_check;
    }

    current_interleave_block++;
  }

  if (test_scheduling == TRUE)
    gettimeofday(&tv[4], NULL);

  /* add scheduled service checks to event queue */
  for (temp_service = service_list;
       temp_service != NULL;
       temp_service = temp_service->next) {

    /* Nagios XI/NDOUtils MOD */
    /* update status of all services (scheduled or not) */
    update_service_status(temp_service, FALSE);

    /* skip most services that shouldn't be scheduled */
    if (temp_service->should_be_scheduled == FALSE) {

      /* passive checks are an exception if a forced check was scheduled before Centreon Engine was restarted */
      if (!(temp_service->checks_enabled == FALSE
	    && temp_service->next_check != (time_t)0L
	    && (temp_service->check_options & CHECK_OPTION_FORCE_EXECUTION)))
        continue;
    }

    /* create a new service check event */
    schedule_new_event(EVENT_SERVICE_CHECK,
		       FALSE,
		       temp_service->next_check,
                       FALSE,
		       0,
		       NULL,
		       TRUE,
		       (void*)temp_service,
		       NULL,
                       temp_service->check_options);
  }

  if (test_scheduling == TRUE)
    gettimeofday(&tv[5], NULL);

  /******** DETERMINE HOST SCHEDULING PARAMS  ********/

  logger(dbg_events, most) << "Determining host scheduling parameters...";

  scheduling_info.first_host_check = (time_t)0L;
  scheduling_info.last_host_check = (time_t)0L;

  /* default max host check spread (in minutes) */
  scheduling_info.max_host_check_spread = config.get_max_host_check_spread();

  /* how should we determine the host inter-check delay to use? */
  switch (config.get_host_inter_check_delay_method()) {
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
    /* be smart and calculate the best delay to use to minimize local load... */
    if (scheduling_info.total_scheduled_hosts > 0
        && scheduling_info.host_check_interval_total > 0) {

      /* adjust the check interval total to correspond to the interval length */
      scheduling_info.host_check_interval_total =
	(scheduling_info.host_check_interval_total * config.get_interval_length());

      /* calculate the average check interval for hosts */
      scheduling_info.average_host_check_interval =
        (double)((double)scheduling_info.host_check_interval_total
                 / (double)scheduling_info.total_scheduled_hosts);

      /* calculate the average inter check delay (in seconds) needed to evenly space the host checks out */
      scheduling_info.average_host_inter_check_delay =
	(double)(scheduling_info.average_host_check_interval
                 / (double)scheduling_info.total_scheduled_hosts);

      /* set the global inter check delay value */
      scheduling_info.host_inter_check_delay = scheduling_info.average_host_inter_check_delay;

      /* calculate max inter check delay and see if we should use that instead */
      max_inter_check_delay = (double)((scheduling_info.max_host_check_spread * 60.0)
				       / (double)scheduling_info.total_scheduled_hosts);
      if (scheduling_info.host_inter_check_delay > max_inter_check_delay)
        scheduling_info.host_inter_check_delay = max_inter_check_delay;
    }
    else
      scheduling_info.host_inter_check_delay = 0.0;

    logger(dbg_events, most)
      << "Total scheduled host checks:  "
      << scheduling_info.total_scheduled_hosts;
    logger(dbg_events, most)
      << "Host check interval total:    "
      << scheduling_info.host_check_interval_total;
    logger(dbg_events, most)
      << fixed << setprecision(2)
      << "Average host check interval:  "
      << scheduling_info.average_host_check_interval << " sec";
    logger(dbg_events, most)
      << fixed << setprecision(2)
      << "Host inter-check delay:       "
      << scheduling_info.host_inter_check_delay << " sec";
  }

  if (test_scheduling == TRUE)
    gettimeofday(&tv[6], NULL);

  /******** SCHEDULE HOST CHECKS  ********/

  logger(dbg_events, most) << "Scheduling host checks...";

  /* determine check times for host checks */
  mult_factor = 0;
  for (temp_host = host_list; temp_host != NULL; temp_host = temp_host->next) {

    logger(dbg_events, most) << "Host '" <<  temp_host->name << "'";

    /* skip hosts that shouldn't be scheduled */
    if (temp_host->should_be_scheduled == FALSE) {
      logger(dbg_events, most) << "Host check should not be scheduled.";
      continue;
    }

    /* skip hosts that are already scheduled for the future (from retention data), but reschedule ones that were supposed to be checked before we started */
    if (temp_host->next_check > current_time) {
      logger(dbg_events, most)
        << "Host is already scheduled to be checked in the future: "
        << my_ctime(&temp_host->next_check);
      continue;
    }

    /* calculate preferred host check time */
    temp_host->next_check = (time_t)(current_time
				     + (mult_factor * scheduling_info.host_inter_check_delay));

    logger(dbg_events, most)
      << "Preferred Check Time: " << temp_host->next_check
      << " --> " << my_ctime(&temp_host->next_check);

    /* make sure the host can actually be scheduled at this time */
    is_valid_time = check_time_against_period(temp_host->next_check,
					      temp_host->check_period_ptr);
    if (is_valid_time == ERROR) {
      get_next_valid_time(temp_host->next_check,
			  &next_valid_time,
                          temp_host->check_period_ptr);
      temp_host->next_check = next_valid_time;
    }

    logger(dbg_events, most)
      << "Actual Check Time: " << temp_host->next_check
      << " --> " << my_ctime(&temp_host->next_check);

    if (scheduling_info.first_host_check == (time_t) 0
        || (temp_host->next_check < scheduling_info.first_host_check))
      scheduling_info.first_host_check = temp_host->next_check;
    if (temp_host->next_check > scheduling_info.last_host_check)
      scheduling_info.last_host_check = temp_host->next_check;

    mult_factor++;
  }

  if (test_scheduling == TRUE)
    gettimeofday(&tv[7], NULL);

  /* add scheduled host checks to event queue */
  for (temp_host = host_list; temp_host != NULL; temp_host = temp_host->next) {

    /* Nagios XI/NDOUtils Mod */
    /* update status of all hosts (scheduled or not) */
    update_host_status(temp_host, FALSE);

    /* skip most hosts that shouldn't be scheduled */
    if (temp_host->should_be_scheduled == FALSE) {

      /* passive checks are an exception if a forced check was scheduled before Centreon Engine was restarted */
      if (!(temp_host->checks_enabled == FALSE
	    && temp_host->next_check != (time_t)0L
	    && (temp_host->check_options & CHECK_OPTION_FORCE_EXECUTION)))
        continue;
    }

    /* schedule a new host check event */
    schedule_new_event(EVENT_HOST_CHECK,
		       FALSE,
		       temp_host->next_check,
		       FALSE,
		       0,
                       NULL,
		       TRUE,
		       (void*)temp_host,
		       NULL,
                       temp_host->check_options);
  }

  if (test_scheduling == TRUE)
    gettimeofday(&tv[8], NULL);

  /******** SCHEDULE MISC EVENTS ********/

  /* add a host and service check rescheduling event */
  if (config.get_auto_reschedule_checks() == true)
    schedule_new_event(EVENT_RESCHEDULE_CHECKS,
		       TRUE,
                       current_time + config.get_auto_rescheduling_interval(),
                       TRUE,
		       config.get_auto_rescheduling_interval(),
		       NULL,
                       TRUE,
		       NULL,
		       NULL,
		       0);

  /* add a check result reaper event */
  schedule_new_event(EVENT_CHECK_REAPER, TRUE,
                     current_time + config.get_check_reaper_interval(),
		     TRUE,
                     config.get_check_reaper_interval(),
		     NULL,
		     TRUE,
		     NULL,
		     NULL,
                     0);

  /* add an orphaned check event */
  if (config.get_check_orphaned_services() == TRUE
      || config.get_check_orphaned_hosts() == true)
    schedule_new_event(EVENT_ORPHAN_CHECK,
		       TRUE,
                       current_time + DEFAULT_ORPHAN_CHECK_INTERVAL,
		       TRUE,
                       DEFAULT_ORPHAN_CHECK_INTERVAL,
		       NULL,
		       TRUE,
		       NULL,
		       NULL,
                       0);

  /* add a service result "freshness" check event */
  if (config.get_check_service_freshness() == true)
    schedule_new_event(EVENT_SFRESHNESS_CHECK,
		       TRUE,
                       current_time + config.get_service_freshness_check_interval(),
		       TRUE,
                       config.get_service_freshness_check_interval(),
		       NULL,
                       TRUE,
		       NULL,
		       NULL,
		       0);

  /* add a host result "freshness" check event */
  if (config.get_check_host_freshness() == true)
    schedule_new_event(EVENT_HFRESHNESS_CHECK,
		       TRUE,
                       current_time + config.get_host_freshness_check_interval(),
		       TRUE,
                       config.get_host_freshness_check_interval(),
		       NULL,
		       TRUE,
                       NULL,
		       NULL,
		       0);

  /* add a status save event */
  schedule_new_event(EVENT_STATUS_SAVE,
		     TRUE,
                     current_time + config.get_status_update_interval(),
		     TRUE,
                     config.get_status_update_interval(),
		     NULL,
		     TRUE,
		     NULL,
                     NULL,
		     0);

  /* add an external command check event if needed */
  if (config.get_check_external_commands() == true) {
    if (config.get_command_check_interval() == -1)
      interval_to_use = (unsigned long)5;
    else
      interval_to_use = (unsigned long)config.get_command_check_interval();
    schedule_new_event(EVENT_COMMAND_CHECK,
		       TRUE,
                       current_time + interval_to_use,
		       TRUE,
		       interval_to_use,
                       NULL,
		       TRUE,
		       NULL,
		       NULL,
		       0);
  }

  /* add a log rotation event if necessary */
  if (config.get_log_rotation_method() != LOG_ROTATION_NONE)
    schedule_new_event(EVENT_LOG_ROTATION,
		       TRUE,
		       get_next_log_rotation_time(),
                       TRUE,
		       0,
		       (void*)get_next_log_rotation_time,
		       TRUE,
		       NULL,
                       NULL,
		       0);

  /* add a retention data save event if needed */
  if (config.get_retain_state_information() == true
      && config.get_retention_update_interval() > 0)
    schedule_new_event(EVENT_RETENTION_SAVE,
		       TRUE,
                       current_time + (config.get_retention_update_interval() * 60),
		       TRUE,
                       (config.get_retention_update_interval() * 60),
		       NULL,
                       TRUE,
		       NULL,
		       NULL,
		       0);

  if (test_scheduling == TRUE) {
    runtime[0] = (double)((double)(tv[1].tv_sec - tv[0].tv_sec)
			  + (double)((tv[1].tv_usec - tv[0].tv_usec) / 1000.0) / 1000.0);
    runtime[1] = (double)((double)(tv[2].tv_sec - tv[1].tv_sec)
			  + (double)((tv[2].tv_usec - tv[1].tv_usec) / 1000.0) / 1000.0);
    runtime[2] = (double)((double)(tv[3].tv_sec - tv[2].tv_sec)
			  + (double)((tv[3].tv_usec - tv[2].tv_usec) / 1000.0) / 1000.0);
    runtime[3] = (double)((double)(tv[4].tv_sec - tv[3].tv_sec)
			  + (double)((tv[4].tv_usec - tv[3].tv_usec) / 1000.0) / 1000.0);
    runtime[4] = (double)((double)(tv[5].tv_sec - tv[4].tv_sec)
			  + (double)((tv[5].tv_usec - tv[4].tv_usec) / 1000.0) / 1000.0);
    runtime[5] = (double)((double)(tv[6].tv_sec - tv[5].tv_sec)
			  + (double)((tv[6].tv_usec - tv[5].tv_usec) / 1000.0) / 1000.0);
    runtime[6] = (double)((double)(tv[7].tv_sec - tv[6].tv_sec)
			  + (double)((tv[7].tv_usec - tv[6].tv_usec) / 1000.0) / 1000.0);
    runtime[7] = (double)((double)(tv[8].tv_sec - tv[7].tv_sec)
			  + (double)((tv[8].tv_usec - tv[7].tv_usec) / 1000.0) / 1000.0);
    runtime[8] = (double)((double)(tv[8].tv_sec - tv[0].tv_sec)
			  + (double)((tv[8].tv_usec - tv[0].tv_usec) / 1000.0) / 1000.0);

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
}

/* displays service check scheduling information */
void display_scheduling_info(void) {
  float minimum_concurrent_checks1 = 0.0;
  float minimum_concurrent_checks2 = 0.0;
  float minimum_concurrent_checks = 0.0;
  float max_reaper_interval = 0.0;
  int suggestions = 0;

  printf("Projected scheduling information for host and service checks\n");
  printf("is listed below.  This information assumes that you are going\n");
  printf("to start running Centreon Engine with your current config files.\n\n");

  printf("HOST SCHEDULING INFORMATION\n");
  printf("---------------------------\n");
  printf("Total hosts:                     %d\n", scheduling_info.total_hosts);
  printf("Total scheduled hosts:           %d\n", scheduling_info.total_scheduled_hosts);

  printf("Host inter-check delay method:   ");
  if (config.get_host_inter_check_delay_method() == configuration::state::icd_none)
    printf("NONE\n");
  else if (config.get_host_inter_check_delay_method() == configuration::state::icd_dumb)
    printf("DUMB\n");
  else if (config.get_host_inter_check_delay_method() == configuration::state::icd_smart) {
    printf("SMART\n");
    printf("Average host check interval:     %.2f sec\n",
	   scheduling_info.average_host_check_interval);
  }
  else
    printf("USER-SUPPLIED VALUE\n");
  printf("Host inter-check delay:          %.2f sec\n", scheduling_info.host_inter_check_delay);
  printf("Max host check spread:           %d min\n", scheduling_info.max_host_check_spread);
  printf("First scheduled check:           %s", (scheduling_info.total_scheduled_hosts == 0)
	 ? "N/A\n" : ctime(&scheduling_info.first_host_check));
  printf("Last scheduled check:            %s", (scheduling_info.total_scheduled_hosts == 0)
	 ? "N/A\n" : ctime(&scheduling_info.last_host_check));
  printf("\n\n");

  printf("SERVICE SCHEDULING INFORMATION\n");
  printf("-------------------------------\n");
  printf("Total services:                     %d\n", scheduling_info.total_services);
  printf("Total scheduled services:           %d\n", scheduling_info.total_scheduled_services);

  printf("Service inter-check delay method:   ");
  if (config.get_service_inter_check_delay_method() == configuration::state::icd_none)
    printf("NONE\n");
  else if (config.get_service_inter_check_delay_method() == configuration::state::icd_dumb)
    printf("DUMB\n");
  else if (config.get_service_inter_check_delay_method() == configuration::state::icd_smart) {
    printf("SMART\n");
    printf("Average service check interval:     %.2f sec\n",
	   scheduling_info.average_service_check_interval);
  }
  else
    printf("USER-SUPPLIED VALUE\n");
  printf("Inter-check delay:                  %.2f sec\n", scheduling_info.service_inter_check_delay);

  printf("Interleave factor method:           %s\n",
         (config.get_service_interleave_factor_method() == configuration::state::ilf_user)
	 ? "USER-SUPPLIED VALUE" : "SMART");
  if (config.get_service_interleave_factor_method() == configuration::state::ilf_smart)
    printf("Average services per host:          %.2f\n", scheduling_info.average_services_per_host);
  printf("Service interleave factor:          %d\n", scheduling_info.service_interleave_factor);

  printf("Max service check spread:           %d min\n", scheduling_info.max_service_check_spread);
  printf("First scheduled check:              %s", ctime(&scheduling_info.first_service_check));
  printf("Last scheduled check:               %s", ctime(&scheduling_info.last_service_check));
  printf("\n\n");

  printf("CHECK PROCESSING INFORMATION\n");
  printf("----------------------------\n");
  printf("Check result reaper interval:       %d sec\n", config.get_check_reaper_interval());
  printf("Max concurrent service checks:      ");
  if (config.get_max_parallel_service_checks() == 0)
    printf("Unlimited\n");
  else
    printf("%d\n", config.get_max_parallel_service_checks());
  printf("\n\n");

  printf("PERFORMANCE SUGGESTIONS\n");
  printf("-----------------------\n");

  /***** MAX REAPER INTERVAL RECOMMENDATION *****/

  /* assume a 100% (2x) check burst for check reaper */
  /* assume we want a max of 2k files in the result queue at any given time */
  max_reaper_interval = floor(2000 * scheduling_info.service_inter_check_delay);
  if (max_reaper_interval < 2.0)
    max_reaper_interval = 2.0;
  if (max_reaper_interval > 30.0)
    max_reaper_interval = 30.0;
  if (max_reaper_interval < config.get_check_reaper_interval()) {
    printf("* Value for 'check_result_reaper_frequency' should be <= %d seconds\n",
	   (int)max_reaper_interval);
    suggestions++;
  }
  if (config.get_check_reaper_interval() < 2) {
    printf("* Value for 'check_result_reaper_frequency' should be >= 2 seconds\n");
    suggestions++;
  }

  /***** MINIMUM CONCURRENT CHECKS RECOMMENDATION *****/

  /* first method (old) - assume a 100% (2x) service check burst for max concurrent checks */
  if (scheduling_info.service_inter_check_delay == 0.0)
    minimum_concurrent_checks1 = ceil(config.get_check_reaper_interval() * 2.0);
  else
    minimum_concurrent_checks1 = ceil((config.get_check_reaper_interval() * 2.0)
				      / scheduling_info.service_inter_check_delay);

  /* second method (new) - assume a 25% (1.25x) service check burst for max concurrent checks */
  minimum_concurrent_checks2 = ceil((((double)scheduling_info.total_scheduled_services)
				     / scheduling_info.average_service_check_interval) * 1.25
				    * config.get_check_reaper_interval()
				    * scheduling_info.average_service_execution_time);

  /* use max of computed values */
  if (minimum_concurrent_checks1 > minimum_concurrent_checks2)
    minimum_concurrent_checks = minimum_concurrent_checks1;
  else
    minimum_concurrent_checks = minimum_concurrent_checks2;

  /* compare with configured value */
  if ((minimum_concurrent_checks > config.get_max_parallel_service_checks())
      && config.get_max_parallel_service_checks() != 0) {
    printf("* Value for 'max_concurrent_checks' option should be >= %d\n",
           (int)minimum_concurrent_checks);
    suggestions++;
  }

  if (suggestions == 0)
    printf("I have no suggestions - things look okay.\n");
  printf("\n");
}

/* schedule a new timed event */
void schedule_new_event(int event_type,
			int high_priority,
			time_t run_time,
                        int recurring,
			unsigned long event_interval,
                        void* timing_func,
			int compensate_for_time_change,
                        void* event_data,
			void* event_args,
			int event_options) {
  timed_event** event_list = NULL;
  timed_event** event_list_tail = NULL;
  timed_event* new_event = NULL;

  logger(dbg_functions, basic) << "schedule_new_event()";

  if (high_priority == TRUE) {
    event_list = &event_list_high;
    event_list_tail = &event_list_high_tail;
  }
  else {
    event_list = &event_list_low;
    event_list_tail = &event_list_low_tail;
  }

  new_event = new timed_event;

  new_event->event_type = event_type;
  new_event->event_data = event_data;
  new_event->event_args = event_args;
  new_event->event_options = event_options;
  new_event->run_time = run_time;
  new_event->recurring = recurring;
  new_event->event_interval = event_interval;
  new_event->timing_func = timing_func;
  new_event->compensate_for_time_change = compensate_for_time_change;

  /* add the event to the event list */
  add_event(new_event, event_list, event_list_tail);
}

/* reschedule an event in order of execution time */
void reschedule_event(timed_event* event,
		      timed_event** event_list,
                      timed_event** event_list_tail) {
  time_t current_time = 0L;
  time_t (*timingfunc)(void);

  logger(dbg_functions, basic) << "reschedule_event()";

  /* reschedule recurring events... */
  if (event->recurring == TRUE) {

    /* use custom timing function */
    if (event->timing_func != NULL) {
      *(void**)(&timingfunc) = event->timing_func;
      event->run_time = (*timingfunc) ();
    }

    /* normal recurring events */
    else {
      event->run_time = event->run_time + event->event_interval;
      time(&current_time);
      if (event->run_time < current_time)
        event->run_time = current_time;
    }
  }

  /* add the event to the event list */
  add_event(event, event_list, event_list_tail);
}

/* add an event to list ordered by execution time */
void add_event(timed_event* event,
	       timed_event** event_list,
               timed_event** event_list_tail) {
  timed_event* temp_event = NULL;
  timed_event* first_event = NULL;

  logger(dbg_functions, basic) << "add_event()";

  event->next = NULL;
  event->prev = NULL;

  if (*event_list == event_list_low)
    quick_timed_event.insert(hash_timed_event::low, event);
  else if (*event_list == event_list_high)
    quick_timed_event.insert(hash_timed_event::high, event);

  first_event = *event_list;

  /* add the event to the head of the list if there are no other events */
  if (*event_list == NULL) {
    *event_list = event;
    *event_list_tail = event;
  }

  /* add event to head of the list if it should be executed first */
  else if (event->run_time < first_event->run_time) {
    event->prev = NULL;
    (*event_list)->prev = event;
    event->next = *event_list;
    *event_list = event;
  }

  /* else place the event according to next execution time */
  else {

    /* start from the end of the list, as new events are likely to be executed in the future, rather than now... */
    for (temp_event = *event_list_tail;
	 temp_event != NULL;
         temp_event = temp_event->prev) {
      if (event->run_time >= temp_event->run_time) {
        event->next = temp_event->next;
        event->prev = temp_event;
        temp_event->next = event;
        if (event->next == NULL)
          *event_list_tail = event;
        else
          event->next->prev = event;
        break;
      }
      else if (temp_event->prev == NULL) {
        temp_event->prev = event;
        event->next = temp_event;
        *event_list = event;
        break;
      }
    }
  }

  /* send event data to broker */
  broker_timed_event(NEBTYPE_TIMEDEVENT_ADD,
		     NEBFLAG_NONE,
		     NEBATTR_NONE,
		     event,
                     NULL);
}

/* remove an event from the queue */
void remove_event(timed_event* event,
		  timed_event** event_list,
                  timed_event** event_list_tail) {
  timed_event* temp_event = NULL;

  logger(dbg_functions, basic) << "remove_event()";

  /* send event data to broker */
  broker_timed_event(NEBTYPE_TIMEDEVENT_REMOVE,
		     NEBFLAG_NONE,
		     NEBATTR_NONE,
                     event,
		     NULL);

  if (*event_list == NULL || event == NULL)
    return;

  if (*event_list == event_list_low)
    quick_timed_event.erase(hash_timed_event::low, event);
  else if (*event_list == event_list_high)
    quick_timed_event.erase(hash_timed_event::high, event);

  if (*event_list == event) {
    event->prev = NULL;
    *event_list = event->next;
    if (*event_list == NULL)
      *event_list_tail = NULL;
  }

  else {

    for (temp_event = *event_list; temp_event != NULL; temp_event = temp_event->next) {
      if (temp_event->next == event) {
        temp_event->next = temp_event->next->next;
        if (temp_event->next == NULL)
          *event_list_tail = temp_event;
        else
          temp_event->next->prev = temp_event;
        event->next = NULL;
        event->prev = NULL;
        break;
      }
    }
  }
}

/* handles a timed event */
int handle_timed_event(timed_event* event) {
  typedef void (*exec_event)(timed_event*);
  static exec_event tab_exec_event[] = {
    &_exec_event_service_check,
    &_exec_event_command_check,
    &_exec_event_log_rotation,
    &_exec_event_program_shutdown,
    &_exec_event_program_restart,
    &_exec_event_check_reaper,
    &_exec_event_orphan_check,
    &_exec_event_retention_save,
    &_exec_event_status_save,
    &_exec_event_scheduled_downtime,
    &_exec_event_sfreshness_check,
    &_exec_event_expire_downtime,
    &_exec_event_host_check,
    &_exec_event_hfreshness_check,
    &_exec_event_reschedule_checks,
    &_exec_event_expire_comment,
    NULL
  };

  logger(dbg_functions, basic) << "handle_timed_event()";

  /* send event data to broker */
  broker_timed_event(NEBTYPE_TIMEDEVENT_EXECUTE,
                     NEBFLAG_NONE,
                     NEBATTR_NONE,
                     event,
                     NULL);

  logger(dbg_events, basic)
    << "** Timed Event ** Type: " << event->event_type
    << ", Run Time: " << my_ctime(&event->run_time);

  /* how should we handle the event? */
  if (event->event_type < sizeof(tab_exec_event) / sizeof(*tab_exec_event))
    (tab_exec_event[event->event_type])(event);
  else if (event->event_type == EVENT_USER_FUNCTION)
    _exec_event_user_function(event);

  return (OK);
}

/* adjusts scheduling of host and service checks */
void adjust_check_scheduling(void) {
  timed_event* temp_event = NULL;
  service* temp_service = NULL;
  host* temp_host = NULL;
  double projected_host_check_overhead = 0.1;
  double projected_service_check_overhead = 0.1;
  time_t current_time = 0L;
  time_t first_window_time = 0L;
  time_t last_window_time = 0L;
  time_t last_check_time = 0L;
  time_t new_run_time = 0L;
  int total_checks = 0;
  int current_check = 0;
  double inter_check_delay = 0.0;
  double current_icd_offset = 0.0;
  double total_check_exec_time = 0.0;
  double last_check_exec_time = 0.0;
  int adjust_scheduling = FALSE;
  double exec_time_factor = 0.0;
  double current_exec_time = 0.0;
  double current_exec_time_offset = 0.0;
  double new_run_time_offset = 0.0;

  logger(dbg_functions, basic) << "adjust_check_scheduling()";

  /* TODO:
     - Track host check overhead on a per-host basis
     - Figure out how to calculate service check overhead
  */

  /* determine our adjustment window */
  time(&current_time);
  first_window_time = current_time;
  last_window_time = first_window_time + config.get_auto_rescheduling_window();

  /* get current scheduling data */
  for (temp_event = event_list_low;
       temp_event != NULL;
       temp_event = temp_event->next) {

    /* skip events outside of our current window */
    if (temp_event->run_time <= first_window_time)
      continue;
    if (temp_event->run_time > last_window_time)
      break;

    if (temp_event->event_type == EVENT_HOST_CHECK) {

      if ((temp_host = (host*)temp_event->event_data) == NULL)
        continue;

      /* ignore forced checks */
      if (temp_host->check_options & CHECK_OPTION_FORCE_EXECUTION)
        continue;

      /* does the last check "bump" into this one? */
      if ((last_check_time + last_check_exec_time) > temp_event->run_time)
        adjust_scheduling = TRUE;

      last_check_time = temp_event->run_time;

      /* calculate time needed to perform check */
      /* NOTE: host check execution time is not taken into account, as scheduled host checks are run in parallel */
      last_check_exec_time = projected_host_check_overhead;
      total_check_exec_time += last_check_exec_time;
    }

    else if (temp_event->event_type == EVENT_SERVICE_CHECK) {

      if ((temp_service = (service*)temp_event->event_data) == NULL)
        continue;

      /* ignore forced checks */
      if (temp_service->check_options & CHECK_OPTION_FORCE_EXECUTION)
        continue;

      /* does the last check "bump" into this one? */
      if ((last_check_time + last_check_exec_time) > temp_event->run_time)
        adjust_scheduling = TRUE;

      last_check_time = temp_event->run_time;

      /* calculate time needed to perform check */
      /* NOTE: service check execution time is not taken into account, as service checks are run in parallel */
      last_check_exec_time = projected_service_check_overhead;
      total_check_exec_time += last_check_exec_time;
    }
    else
      continue;

    total_checks++;
  }

  /* nothing to do... */
  if (total_checks == 0 || adjust_scheduling == FALSE) {
    /*
      printf("\n\n");
      printf("NOTHING TO DO!\n");
      printf("# CHECKS:    %d\n",total_checks);
      printf("WINDOW TIME: %d\n",auto_rescheduling_window);
      printf("EXEC TIME:   %.3f\n",total_check_exec_time);
    */

    return;
  }

  if ((unsigned long)total_check_exec_time > config.get_auto_rescheduling_window()) {
    inter_check_delay = 0.0;
    exec_time_factor = (double)((double)config.get_auto_rescheduling_window()
				/ total_check_exec_time);
  }
  else {
    inter_check_delay = (double)((((double)config.get_auto_rescheduling_window())
				  - total_check_exec_time) / (double)(total_checks * 1.0));
    exec_time_factor = 1.0;
  }

  /*
    printf("\n\n");
    printf("TOTAL CHECKS: %d\n",total_checks);
    printf("WINDOW TIME:  %d\n",auto_rescheduling_window);
    printf("EXEC TIME:    %.3f\n",total_check_exec_time);
    printf("ICD:          %.3f\n",inter_check_delay);
    printf("EXEC FACTOR:  %.3f\n",exec_time_factor);
  */

  /* adjust check scheduling */
  current_icd_offset = (inter_check_delay / 2.0);
  for (temp_event = event_list_low;
       temp_event != NULL;
       temp_event = temp_event->next) {

    /* skip events outside of our current window */
    if (temp_event->run_time <= first_window_time)
      continue;
    if (temp_event->run_time > last_window_time)
      break;

    if (temp_event->event_type == EVENT_HOST_CHECK) {

      if ((temp_host = (host*)temp_event->event_data) == NULL)
        continue;

      /* ignore forced checks */
      if (temp_host->check_options & CHECK_OPTION_FORCE_EXECUTION)
        continue;

      current_exec_time = ((temp_host->execution_time
			    + projected_host_check_overhead)
			   * exec_time_factor);
    }
    else if (temp_event->event_type == EVENT_SERVICE_CHECK) {
      if ((temp_service = (service*)temp_event->event_data) == NULL)
        continue;

      /* ignore forced checks */
      if (temp_service->check_options & CHECK_OPTION_FORCE_EXECUTION)
        continue;

      /* NOTE: service check execution time is not taken into account, as service checks are run in parallel */
      current_exec_time = (projected_service_check_overhead * exec_time_factor);
    }
    else
      continue;

    current_check++;
    new_run_time_offset = current_exec_time_offset + current_icd_offset;
    new_run_time = (time_t)(first_window_time + (unsigned long)new_run_time_offset);

    /*
      printf("  CURRENT CHECK #:      %d\n",current_check);
      printf("  CURRENT ICD OFFSET:   %.3f\n",current_icd_offset);
      printf("  CURRENT EXEC TIME:    %.3f\n",current_exec_time);
      printf("  CURRENT EXEC OFFSET:  %.3f\n",current_exec_time_offset);
      printf("  NEW RUN TIME:         %lu\n",new_run_time);
    */

    if (temp_event->event_type == EVENT_HOST_CHECK) {
      temp_event->run_time = new_run_time;
      temp_host->next_check = new_run_time;
      update_host_status(temp_host, FALSE);
    }
    else {
      temp_event->run_time = new_run_time;
      temp_service->next_check = new_run_time;
      update_service_status(temp_service, FALSE);
    }

    current_icd_offset += inter_check_delay;
    current_exec_time_offset += current_exec_time;
  }

  /* resort event list (some events may be out of order at this point) */
  resort_event_list(&event_list_low, &event_list_low_tail);
}

/* attempts to compensate for a change in the system time */
void compensate_for_system_time_change(unsigned long last_time, unsigned long current_time) {
  unsigned long time_difference = 0L;
  timed_event* temp_event = NULL;
  service* temp_service = NULL;
  host* temp_host = NULL;
  int days = 0;
  int hours = 0;
  int minutes = 0;
  int seconds = 0;
  time_t (*timingfunc)(void);

  logger(dbg_functions, basic) << "compensate_for_system_time_change()";

  /* we moved back in time... */
  if (last_time > current_time) {
    time_difference = last_time - current_time;
    get_time_breakdown(time_difference, &days, &hours, &minutes, &seconds);
    logger(dbg_events, basic)
      << "Detected a backwards time change of " << days << "d "
      << hours << "h " << minutes << "m " << seconds << "s.";
  }
  /* we moved into the future... */
  else {
    time_difference = current_time - last_time;
    get_time_breakdown(time_difference, &days, &hours, &minutes, &seconds);
    logger(dbg_events, basic)
      << "Detected a forwards time change of " << days << "d "
      << hours << "h " << minutes << "m " << seconds << "s.";
  }

  /* log the time change */
  logger(log_process_info | log_runtime_warning, basic)
    << "Warning: A system time change of " << days << "d " << hours << "h "
    << minutes << "m " << seconds << "s ("
    << (last_time > current_time ? "backwards" : "forwards")
    << " in time) has been detected.  Compensating...";

  /* adjust the next run time for all high priority timed events */
  for (temp_event = event_list_high;
       temp_event != NULL;
       temp_event = temp_event->next) {

    /* skip special events that occur at specific times... */
    if (temp_event->compensate_for_time_change == FALSE)
      continue;

    /* use custom timing function */
    if (temp_event->timing_func != NULL) {
      *(void**)(&timingfunc) = temp_event->timing_func;
      temp_event->run_time = (*timingfunc)();
    }

    /* else use standard adjustment */
    else
      adjust_timestamp_for_time_change(last_time,
				       current_time,
				       time_difference,
                                       &temp_event->run_time);
  }

  /* resort event list (some events may be out of order at this point) */
  resort_event_list(&event_list_high, &event_list_high_tail);

  /* adjust the next run time for all low priority timed events */
  for (temp_event = event_list_low; temp_event != NULL; temp_event = temp_event->next) {

    /* skip special events that occur at specific times... */
    if (temp_event->compensate_for_time_change == FALSE)
      continue;

    /* use custom timing function */
    if (temp_event->timing_func != NULL) {
      *(void**)(&timingfunc) = temp_event->timing_func;
      temp_event->run_time = (*timingfunc) ();
    }

    /* else use standard adjustment */
    else
      adjust_timestamp_for_time_change(last_time,
				       current_time,
				       time_difference,
                                       &temp_event->run_time);
  }

  /* resort event list (some events may be out of order at this point) */
  resort_event_list(&event_list_low, &event_list_low_tail);

  /* adjust service timestamps */
  for (temp_service = service_list;
       temp_service != NULL;
       temp_service = temp_service->next) {

    adjust_timestamp_for_time_change(last_time,
				     current_time,
				     time_difference,
                                     &temp_service->last_notification);
    adjust_timestamp_for_time_change(last_time,
				     current_time,
				     time_difference,
                                     &temp_service->last_check);
    adjust_timestamp_for_time_change(last_time,
				     current_time,
				     time_difference,
                                     &temp_service->next_check);
    adjust_timestamp_for_time_change(last_time,
				     current_time,
				     time_difference,
                                     &temp_service->last_state_change);
    adjust_timestamp_for_time_change(last_time,
				     current_time,
				     time_difference,
                                     &temp_service->last_hard_state_change);

    /* recalculate next re-notification time */
    temp_service->next_notification = get_next_service_notification_time(temp_service,
									 temp_service->last_notification);

    /* update the status data */
    update_service_status(temp_service, FALSE);
  }

  /* adjust host timestamps */
  for (temp_host = host_list; temp_host != NULL; temp_host = temp_host->next) {
    adjust_timestamp_for_time_change(last_time,
				     current_time,
				     time_difference,
                                     &temp_host->last_host_notification);
    adjust_timestamp_for_time_change(last_time,
				     current_time,
				     time_difference,
                                     &temp_host->last_check);
    adjust_timestamp_for_time_change(last_time,
				     current_time,
				     time_difference,
                                     &temp_host->next_check);
    adjust_timestamp_for_time_change(last_time,
				     current_time,
				     time_difference,
                                     &temp_host->last_state_change);
    adjust_timestamp_for_time_change(last_time,
				     current_time,
				     time_difference,
                                     &temp_host->last_hard_state_change);
    adjust_timestamp_for_time_change(last_time,
				     current_time,
				     time_difference,
                                     &temp_host->last_state_history_update);

    /* recalculate next re-notification time */
    temp_host->next_host_notification = get_next_host_notification_time(temp_host,
									temp_host->last_host_notification);

    /* update the status data */
    update_host_status(temp_host, FALSE);
  }

  /* adjust program timestamps */
  adjust_timestamp_for_time_change(last_time,
				   current_time,
				   time_difference,
                                   &program_start);
  adjust_timestamp_for_time_change(last_time,
				   current_time,
				   time_difference,
                                   &event_start);
  adjust_timestamp_for_time_change(last_time,
				   current_time,
				   time_difference,
                                   &last_command_check);

  /* update the status data */
  update_program_status(FALSE);
}

/* resorts an event list by event execution time - needed when compensating for system time changes */
void resort_event_list(timed_event** event_list, timed_event** event_list_tail) {
  timed_event* temp_event_list = NULL;
  timed_event* temp_event = NULL;
  timed_event* next_event = NULL;

  logger(dbg_functions, basic) << "resort_event_list()";

  /* move current event list to temp list */
  if (*event_list == event_list_low)
    quick_timed_event.clear(hash_timed_event::low);
  else if (*event_list == event_list_high)
    quick_timed_event.clear(hash_timed_event::high);
  temp_event_list = *event_list;
  *event_list = NULL;

  /* move all events to the new event list */
  for (temp_event = temp_event_list; temp_event != NULL; temp_event = next_event) {
    next_event = temp_event->next;

    /* add the event to the newly created event list so it will be resorted */
    temp_event->next = NULL;
    temp_event->prev = NULL;
    add_event(temp_event, event_list, event_list_tail);
  }
}

/* adjusts a timestamp variable in accordance with a system time change */
void adjust_timestamp_for_time_change(time_t last_time,
				      time_t current_time,
                                      unsigned long time_difference,
                                      time_t * ts) {
  logger(dbg_functions, basic) << "adjust_timestamp_for_time_change()";

  /* we shouldn't do anything with epoch values */
  if (*ts == (time_t)0)
    return;

  /* we moved back in time... */
  if (last_time > current_time) {

    /* we can't precede the UNIX epoch */
    if (time_difference > (unsigned long)*ts)
      *ts = (time_t)0;
    else
      *ts = (time_t)(*ts - time_difference);
  }

  /* we moved into the future... */
  else
    *ts = (time_t)(*ts + time_difference);
}

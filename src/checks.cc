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

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/comments.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/downtime.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/perfdata.hh"
#include "com/centreon/engine/sehandlers.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/utils.hh"

#define MAX_CMD_ARGS 4096

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;

/******************************************************************/
/********************** CHECK REAPER FUNCTIONS ********************/
/******************************************************************/

/* reaps host and service check results */
int reap_check_results() {
  try {
    checks::checker::instance().reap();
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "error: " << e.what();
    return (ERROR);
  }
  return (OK);
}

/******************************************************************/
/****************** SERVICE MONITORING FUNCTIONS ******************/
/******************************************************************/

/* executes a scheduled service check */
int run_scheduled_service_check(
      service* svc,
      int check_options,
      double latency) {
  int result = OK;
  time_t current_time = 0L;
  time_t preferred_time = 0L;
  time_t next_valid_time = 0L;
  int time_is_valid = TRUE;

  if (svc == NULL)
    return (ERROR);

  logger(dbg_functions, basic)
    << "run_scheduled_service_check()";
  logger(dbg_checks, basic)
    << "Attempting to run scheduled check of service '"
    << svc->description << "' on host '" << svc->host_name
    << "': check options=" << check_options << ", latency=" << latency;

  /* attempt to run the check */
  result = run_async_service_check(
             svc,
             check_options,
             latency,
             TRUE,
             TRUE,
             &time_is_valid,
             &preferred_time);

  /* an error occurred, so reschedule the check */
  if (result == ERROR) {
    logger(dbg_checks, more)
      << "Unable to run scheduled service check at this time";

    /* only attempt to (re)schedule checks that should get checked... */
    if (svc->should_be_scheduled == TRUE) {

      /* get current time */
      time(&current_time);

      /* determine next time we should check the service if needed */
      /* if service has no check interval, schedule it again for 5 minutes from now */
      if (current_time >= preferred_time)
        preferred_time = current_time + static_cast<time_t>(svc->check_interval <= 0 ? 300 : svc->check_interval * config->interval_length());

      /* make sure we rescheduled the next service check at a valid time */
      get_next_valid_time(
        preferred_time,
        &next_valid_time,
        svc->check_period_ptr);

      // logit(NSLOG_RUNTIME_WARNING,TRUE,"Warning: Service '%s' on host '%s' timeperiod check failed...\n",svc->description,svc->host_name);
      // logit(NSLOG_RUNTIME_WARNING,TRUE,"Current time: %s",ctime(&current_time));
      // logit(NSLOG_RUNTIME_WARNING,TRUE,"Preferred time: %s",ctime(&preferred_time));
      // logit(NSLOG_RUNTIME_WARNING,TRUE,"Next valid time: %s",ctime(&next_valid_time));

      /* the service could not be rescheduled properly - set the next check time for next week */
      /*if(time_is_valid==FALSE && next_valid_time==preferred_time){ */
      /* UPDATED 08/12/09 EG to reflect proper timeperod check logic */
      if (time_is_valid == FALSE
          && check_time_against_period(
               next_valid_time,
               svc->check_period_ptr) == ERROR) {
        /*
	  svc->next_check=(time_t)(next_valid_time+(60*60*24*365));
	  svc->should_be_scheduled=FALSE;
	*/
        svc->next_check = (time_t)(next_valid_time + (60 * 60 * 24 * 7));

        logger(log_runtime_warning, basic)
          << "Warning: Check of service '" << svc->description
          << "' on host '" << svc->host_name << "' could not be "
          "rescheduled properly.  Scheduling check for next week...";

        logger(dbg_checks, more)
          << "Unable to find any valid times to reschedule the next "
          "service check!";
      }

      /* this service could be rescheduled... */
      else {
        svc->next_check = next_valid_time;
        svc->should_be_scheduled = TRUE;

        logger(dbg_checks, more)
          << "Rescheduled next service check for "
          << my_ctime(&next_valid_time);
      }
    }

    /* reschedule the next service check - unless we couldn't find a valid next check time */
    /* 10/19/07 EG - keep original check options */
    if (svc->should_be_scheduled == TRUE)
      schedule_service_check(svc, svc->next_check, check_options);

    /* update the status log */
    update_service_status(svc, FALSE);
    return (ERROR);
  }
  return (OK);
}

/* forks a child process to run a service check, but does not wait for the service check result */
int run_async_service_check(
      service* svc,
      int check_options,
      double latency,
      int scheduled_check,
      int reschedule_check,
      int* time_is_valid,
      time_t* preferred_time) {
  try {
    checks::checker::instance().run(
                                  svc,
                                  check_options,
                                  latency,
                                  scheduled_check,
                                  reschedule_check,
                                  time_is_valid,
                                  preferred_time);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "error: " << e.what();
    return (ERROR);
  }
  return (OK);
}

/* handles asynchronous service check results */
int handle_async_service_check_result(
      service* temp_service,
      check_result* queued_check_result) {
  host* temp_host = NULL;
  time_t next_service_check = 0L;
  time_t preferred_time = 0L;
  time_t next_valid_time = 0L;
  int reschedule_check = FALSE;
  int state_change = FALSE;
  int hard_state_change = FALSE;
  int first_host_check_initiated = FALSE;
  int route_result = HOST_UP;
  time_t current_time = 0L;
  int state_was_logged = FALSE;
  char* old_plugin_output = NULL;
  char* temp_ptr = NULL;
  objectlist* check_servicelist = NULL;
  objectlist* servicelist_item = NULL;
  service* master_service = NULL;
  int run_async_check = TRUE;
  int state_changes_use_cached_state = TRUE;    /* TODO - 09/23/07 move this to a global variable */
  int flapping_check_done = FALSE;

  logger(dbg_functions, basic)
    << "handle_async_service_check_result()";

  /* make sure we have what we need */
  if (temp_service == NULL || queued_check_result == NULL)
    return (ERROR);

  /* get the current time */
  time(&current_time);

  logger(dbg_checks, basic)
    << "** Handling check result for service '" << temp_service->description
    << "' on host '" << temp_service->host_name << "'...";
  logger(dbg_checks, more)
    << "HOST: " << temp_service->host_name
    << ", SERVICE: " << temp_service->description
    << ", CHECK TYPE: " << (queued_check_result->check_type == SERVICE_CHECK_ACTIVE ? "Active" : "Passive")
    << ", OPTIONS: " << queued_check_result->check_options
    << ", SCHEDULED: " << (queued_check_result->scheduled_check == TRUE ? "Yes" : "No")
    << ", RESCHEDULE: " << (queued_check_result->reschedule_check == TRUE ? "Yes" : "No")
    << ", EXITED OK: " << (queued_check_result->exited_ok == TRUE ? "Yes" : "No")
    << ", return CODE: " << queued_check_result->return_code
    << ", OUTPUT: " << queued_check_result->output;

  /* decrement the number of service checks still out there... */
  if (queued_check_result->check_type == SERVICE_CHECK_ACTIVE
      && currently_running_service_checks > 0)
    currently_running_service_checks--;

  /* skip this service check results if its passive and we aren't accepting passive check results */
  if (queued_check_result->check_type == SERVICE_CHECK_PASSIVE) {
    if (config->accept_passive_service_checks() == false) {
      logger(dbg_checks, basic)
        << "Discarding passive service check result because passive "
        "service checks are disabled globally.";
      return (ERROR);
    }
    if (temp_service->accept_passive_service_checks == FALSE) {
      logger(dbg_checks, basic)
        << "Discarding passive service check result because passive "
        "checks are disabled for this service.";
      return (ERROR);
    }
  }

  /* clear the freshening flag (it would have been set if this service was determined to be stale) */
  if (queued_check_result->check_options & CHECK_OPTION_FRESHNESS_CHECK)
    temp_service->is_being_freshened = FALSE;

  /* clear the execution flag if this was an active check */
  if (queued_check_result->check_type == SERVICE_CHECK_ACTIVE)
    temp_service->is_executing = FALSE;

  /* DISCARD INVALID FRESHNESS CHECK RESULTS */
  /* If a services goes stale, Engine will initiate a forced check in
  ** order to freshen it.  There is a race condition whereby a passive
  ** check could arrive between the 1) initiation of the forced check
  ** and 2) the time when the forced check result is processed here.
  ** This would make the service fresh again, so we do a quick check to
  ** make sure the service is still stale before we accept the check
  ** result.
  */
  if ((queued_check_result->check_options & CHECK_OPTION_FRESHNESS_CHECK)
      && is_service_result_fresh(temp_service, current_time, FALSE) == TRUE) {
    logger(dbg_checks, basic)
      << "Discarding service freshness check result because the service "
      "is currently fresh (race condition avoided).";
    return (OK);
  }

  /* check latency is passed to us */
  temp_service->latency = queued_check_result->latency;

  /* update the execution time for this check (millisecond resolution) */
  temp_service->execution_time
    = (double)((double)(queued_check_result->finish_time.tv_sec
                        - queued_check_result->start_time.tv_sec)
               + (double)((queued_check_result->finish_time.tv_usec
                           - queued_check_result->start_time.tv_usec)
                          / 1000.0) / 1000.0);
  if (temp_service->execution_time < 0.0)
    temp_service->execution_time = 0.0;

  /* get the last check time */
  temp_service->last_check = queued_check_result->start_time.tv_sec;

  /* was this check passive or active? */
  temp_service->check_type
    = (queued_check_result->check_type == SERVICE_CHECK_ACTIVE)
    ? SERVICE_CHECK_ACTIVE
    : SERVICE_CHECK_PASSIVE;

  /* update check statistics for passive checks */
  if (queued_check_result->check_type == SERVICE_CHECK_PASSIVE)
    update_check_stats(
      PASSIVE_SERVICE_CHECK_STATS,
      queued_check_result->start_time.tv_sec);

  /* should we reschedule the next service check? NOTE: This may be overridden later... */
  reschedule_check = queued_check_result->reschedule_check;

  /* save the old service status info */
  temp_service->last_state = temp_service->current_state;

  /* save old plugin output */
  if (temp_service->plugin_output)
    old_plugin_output = temp_service->plugin_output;

  /* clear the old plugin output and perf data buffers */
  delete[] temp_service->long_plugin_output;
  delete[] temp_service->perf_data;

  temp_service->plugin_output = NULL;
  temp_service->long_plugin_output = NULL;
  temp_service->perf_data = NULL;

  /* if there was some error running the command, just skip it (this shouldn't be happening) */
  if (queued_check_result->exited_ok == FALSE) {

    logger(log_runtime_warning, basic)
      << "Warning:  Check of service '" << temp_service->description
      << "' on host '" << temp_service->host_name
      << "' did not exit properly!";

    temp_service->plugin_output
      = my_strdup("(Service check did not exit properly)");
    temp_service->current_state = STATE_CRITICAL;
  }

  /* make sure the return code is within bounds */
  else if (queued_check_result->return_code < 0
	   || queued_check_result->return_code > 3) {

    logger(log_runtime_warning, basic)
      << "Warning: return (code of " << queued_check_result->return_code
      << " for check of service '" << temp_service->description
      << "' on host '" << temp_service->host_name
      << "' was out of bounds."
      << (queued_check_result->return_code == 126
          ? "Make sure the plugin you're trying to run is executable."
          : (queued_check_result->return_code == 127
             ? " Make sure the plugin you're trying to run actually exists."
             : ""));

    std::ostringstream oss;
    oss << "(Return code of " << queued_check_result->return_code
	<< " is out of bounds"
	<< (queued_check_result->return_code == 126
	    ? " - plugin may not be executable"
	    : (queued_check_result->return_code == 127
	       ? " - plugin may be missing" : ""))
	<< ')';

    temp_service->plugin_output = my_strdup(oss.str().c_str());
    temp_service->current_state = STATE_CRITICAL;
  }

  /* else the return code is okay... */
  else {

    /* parse check output to get: (1) short output, (2) long output, (3) perf data */
    parse_check_output(
      queued_check_result->output,
      &temp_service->plugin_output,
      &temp_service->long_plugin_output,
      &temp_service->perf_data,
      TRUE,
      TRUE);

    /* make sure the plugin output isn't null */
    if (temp_service->plugin_output == NULL)
      temp_service->plugin_output
        = my_strdup("(No output returned from plugin)");

    /* replace semicolons in plugin output (but not performance data) with colons */
    else if ((temp_ptr = temp_service->plugin_output)) {
      while ((temp_ptr = strchr(temp_ptr, ';')))
        *temp_ptr = ':';
    }

    logger(dbg_checks, most)
      << "Parsing check output...\n"
      << "Short Output:\n"
      << (temp_service->plugin_output == NULL ? "NULL" : temp_service->plugin_output) << "\n"
      << "Long Output:\n"
      << (temp_service->long_plugin_output ==  NULL ? "NULL" : temp_service->long_plugin_output) << "\n"
      << "Perf Data:\n"
      << (temp_service->perf_data == NULL ? "NULL" : temp_service->perf_data);

    /* grab the return code */
    temp_service->current_state = queued_check_result->return_code;
  }

  /* record the last state time */
  switch (temp_service->current_state) {
  case STATE_OK:
    temp_service->last_time_ok = temp_service->last_check;
    break;

  case STATE_WARNING:
    temp_service->last_time_warning = temp_service->last_check;
    break;

  case STATE_UNKNOWN:
    temp_service->last_time_unknown = temp_service->last_check;
    break;

  case STATE_CRITICAL:
    temp_service->last_time_critical = temp_service->last_check;
    break;

  default:
    break;
  }

  /* log passive checks - we need to do this here, as some my bypass external commands by getting dropped in checkresults dir */
  if (temp_service->check_type == SERVICE_CHECK_PASSIVE) {
    if (config->log_passive_checks() == true)
      logger(log_passive_check, basic)
        << "PASSIVE SERVICE CHECK: " << temp_service->host_name << ";"
        << temp_service->description << ";" << temp_service->current_state
        << ";" << temp_service->plugin_output;
  }

  /* get the host that this service runs on */
  temp_host = (host*)temp_service->host_ptr;

  /* if the service check was okay... */
  if (temp_service->current_state == STATE_OK) {
    /* if the host has never been checked before, verify its status */
    /* only do this if 1) the initial state was set to non-UP or 2) the host is not scheduled to be checked soon (next 5 minutes) */
    if (temp_host->has_been_checked == FALSE
        && (temp_host->initial_state != HOST_UP
            || (unsigned long)temp_host->next_check == 0L
            || (unsigned long)(temp_host->next_check - current_time) >
            300)) {

      /* set a flag to remember that we launched a check */
      first_host_check_initiated = TRUE;

      /* 08/04/07 EG launch an async (parallel) host check unless aggressive host checking is enabled */
      /* previous logic was to simply run a sync (serial) host check */
      /* do NOT allow cached check results to happen here - we need the host to be checked for real... */
      if (config->use_aggressive_host_checking() == true)
        perform_on_demand_host_check(
          temp_host,
          NULL,
          CHECK_OPTION_NONE,
          FALSE,
          0L);
      else
        run_async_host_check_3x(
          temp_host,
          CHECK_OPTION_NONE,
          0.0,
          FALSE,
          FALSE,
          NULL,
          NULL);
    }
  }

  /**** NOTE - THIS WAS MOVED UP FROM LINE 1049 BELOW TO FIX PROBLEMS WHERE CURRENT ATTEMPT VALUE WAS ACTUALLY "LEADING" REAL VALUE ****/
  /* increment the current attempt number if this is a soft state (service was rechecked) */
  if (temp_service->state_type == SOFT_STATE
      && (temp_service->current_attempt < temp_service->max_attempts))
    temp_service->current_attempt = temp_service->current_attempt + 1;

  logger(dbg_checks, most)
    << "ST: " << (temp_service->state_type == SOFT_STATE ? "SOFT" : "HARD")
    << "  CA: " << temp_service->current_attempt
    << "  MA: " << temp_service->max_attempts
    << "  CS: " << temp_service->current_state
    << "  LS: " << temp_service->last_state
    << "  LHS: " << temp_service->last_hard_state;

  /* check for a state change (either soft or hard) */
  if (temp_service->current_state != temp_service->last_state) {
    logger(dbg_checks, most)
      << "Service has changed state since last check!";
    state_change = TRUE;
  }

  /* checks for a hard state change where host was down at last service check */
  /* this occurs in the case where host goes down and service current attempt gets reset to 1 */
  /* if this check is not made, the service recovery looks like a soft recovery instead of a hard one */
  if (temp_service->host_problem_at_last_check == TRUE
      && temp_service->current_state == STATE_OK) {
    logger(dbg_checks, most)
      << "Service had a HARD STATE CHANGE!!";
    hard_state_change = TRUE;
  }

  /* check for a "normal" hard state change where max check attempts is reached */
  if (temp_service->current_attempt >= temp_service->max_attempts
      && temp_service->current_state != temp_service->last_hard_state) {
    logger(dbg_checks, most)
      << "Service had a HARD STATE CHANGE!!";
    hard_state_change = TRUE;
  }

  /* a state change occurred... */
  /* reset last and next notification times and acknowledgement flag if necessary, misc other stuff */
  if (state_change == TRUE || hard_state_change == TRUE) {

    /* reschedule the service check */
    reschedule_check = TRUE;

    /* reset notification times */
    temp_service->last_notification = (time_t)0;
    temp_service->next_notification = (time_t)0;

    /* reset notification suppression option */
    temp_service->no_more_notifications = FALSE;

    if ((ACKNOWLEDGEMENT_NORMAL == temp_service->acknowledgement_type)
	&& ((TRUE == state_change) || (FALSE == hard_state_change))) {

      temp_service->problem_has_been_acknowledged = FALSE;
      temp_service->acknowledgement_type = ACKNOWLEDGEMENT_NONE;

      /* remove any non-persistant comments associated with the ack */
      delete_service_acknowledgement_comments(temp_service);
    }
    else if (temp_service->acknowledgement_type == ACKNOWLEDGEMENT_STICKY
             && temp_service->current_state == STATE_OK) {
      temp_service->problem_has_been_acknowledged = FALSE;
      temp_service->acknowledgement_type = ACKNOWLEDGEMENT_NONE;

      /* remove any non-persistant comments associated with the ack */
      delete_service_acknowledgement_comments(temp_service);
    }

    /* do NOT reset current notification number!!! */
    /* hard changes between non-OK states should continue to be escalated, so don't reset current notification number */
    /*temp_service->current_notification_number=0; */
  }

  /* initialize the last host and service state change times if necessary */
  if (temp_service->last_state_change == (time_t)0)
    temp_service->last_state_change = temp_service->last_check;
  if (temp_service->last_hard_state_change == (time_t)0)
    temp_service->last_hard_state_change = temp_service->last_check;
  if (temp_host->last_state_change == (time_t)0)
    temp_host->last_state_change = temp_service->last_check;
  if (temp_host->last_hard_state_change == (time_t)0)
    temp_host->last_hard_state_change = temp_service->last_check;

  /* update last service state change times */
  if (state_change == TRUE)
    temp_service->last_state_change = temp_service->last_check;
  if (hard_state_change == TRUE)
    temp_service->last_hard_state_change = temp_service->last_check;

  /* update the event and problem ids */
  if (state_change == TRUE) {

    /* always update the event id on a state change */
    temp_service->last_event_id = temp_service->current_event_id;
    temp_service->current_event_id = next_event_id;
    next_event_id++;

    /* update the problem id when transitioning to a problem state */
    if (temp_service->last_state == STATE_OK) {
      /* don't reset last problem id, or it will be zero the next time a problem is encountered */
      /* temp_service->last_problem_id=temp_service->current_problem_id; */
      temp_service->current_problem_id = next_problem_id;
      next_problem_id++;
    }

    /* clear the problem id when transitioning from a problem state to an OK state */
    if (temp_service->current_state == STATE_OK) {
      temp_service->last_problem_id = temp_service->current_problem_id;
      temp_service->current_problem_id = 0L;
    }
  }

  /**************************************/
  /******* SERVICE CHECK OK LOGIC *******/
  /**************************************/

  /* if the service is up and running OK... */
  if (temp_service->current_state == STATE_OK) {

    logger(dbg_checks, more)
      << "Service is OK.";

    /* reset the acknowledgement flag (this should already have been done, but just in case...) */
    temp_service->problem_has_been_acknowledged = FALSE;
    temp_service->acknowledgement_type = ACKNOWLEDGEMENT_NONE;

    /* verify the route to the host and send out host recovery notifications */
    if (temp_host->current_state != HOST_UP) {
      logger(dbg_checks, more)
        << "Host is NOT UP, so we'll check it to see if it recovered...";

      /* 08/04/07 EG launch an async (parallel) host check (possibly cached) unless aggressive host checking is enabled */
      /* previous logic was to simply run a sync (serial) host check */
      if (config->use_aggressive_host_checking() == true)
        perform_on_demand_host_check(
          temp_host,
          NULL,
          CHECK_OPTION_NONE,
          TRUE,
          config->cached_host_check_horizon());
      /* 09/23/07 EG don't launch a new host check if we already did so earlier */
      else if (first_host_check_initiated == TRUE)
        logger(dbg_checks, more)
          << "First host check was already initiated, so we'll skip a "
          "new host check.";
      else {
        /* can we use the last cached host state? */
        /* usually only use cached host state if no service state change has occurred */
        if ((state_change == FALSE
             || state_changes_use_cached_state == TRUE)
            && temp_host->has_been_checked == TRUE
            && (static_cast<unsigned long>(current_time - temp_host->last_check) <= config->cached_host_check_horizon())) {
          logger(dbg_checks, more)
            << "* Using cached host state: " << temp_host->current_state;
          update_check_stats(ACTIVE_ONDEMAND_HOST_CHECK_STATS, current_time);
          update_check_stats(ACTIVE_CACHED_HOST_CHECK_STATS, current_time);
        }

        /* else launch an async (parallel) check of the host */
        else
          run_async_host_check_3x(
            temp_host,
            CHECK_OPTION_NONE,
            0.0,
            FALSE,
            FALSE,
            NULL,
            NULL);
      }
    }

    /* if a hard service recovery has occurred... */
    if (hard_state_change == TRUE) {
      logger(dbg_checks, more)
        << "Service experienced a HARD RECOVERY.";

      /* set the state type macro */
      temp_service->state_type = HARD_STATE;

      /* log the service recovery */
      log_service_event(temp_service);
      state_was_logged = TRUE;

      /* 10/04/07 check to see if the service and/or associate host is flapping */
      /* this should be done before a notification is sent out to ensure the host didn't just start flapping */
      check_for_service_flapping(temp_service, TRUE, TRUE);
      check_for_host_flapping(temp_host, TRUE, FALSE, TRUE);
      flapping_check_done = TRUE;

      /* notify contacts about the service recovery */
      service_notification(
        temp_service,
        NOTIFICATION_NORMAL,
        NULL,
        NULL,
        NOTIFICATION_OPTION_NONE);

      /* run the service event handler to handle the hard state change */
      handle_service_event(temp_service);
    }

    /* else if a soft service recovery has occurred... */
    else if (state_change == TRUE) {
      logger(dbg_checks, more)
        << "Service experienced a SOFT RECOVERY.";

      /* this is a soft recovery */
      temp_service->state_type = SOFT_STATE;

      /* log the soft recovery */
      log_service_event(temp_service);
      state_was_logged = TRUE;

      /* run the service event handler to handle the soft state change */
      handle_service_event(temp_service);
    }

    /* else no service state change has occurred... */
    else
      logger(dbg_checks, more)
        << "Service did not change state.";

    /* should we obsessive over service checks? */
    if (config->obsess_over_services() == true)
      obsessive_compulsive_service_check_processor(temp_service);

    /* reset all service variables because its okay now... */
    temp_service->host_problem_at_last_check = FALSE;
    temp_service->current_attempt = 1;
    temp_service->state_type = HARD_STATE;
    temp_service->last_hard_state = STATE_OK;
    temp_service->last_notification = (time_t)0;
    temp_service->next_notification = (time_t)0;
    temp_service->current_notification_number = 0;
    temp_service->problem_has_been_acknowledged = FALSE;
    temp_service->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
    temp_service->notified_on_unknown = FALSE;
    temp_service->notified_on_warning = FALSE;
    temp_service->notified_on_critical = FALSE;
    temp_service->no_more_notifications = FALSE;
    service_other_props[
      std::make_pair(
             std::string(temp_service->host_ptr->name),
             std::string(temp_service->description))].initial_notif_time
      = 0;

    if (reschedule_check == TRUE)
      next_service_check
        = (time_t)(temp_service->last_check
                   + (temp_service->check_interval
                      * config->interval_length()));
  }

  /*******************************************/
  /******* SERVICE CHECK PROBLEM LOGIC *******/
  /*******************************************/

  /* hey, something's not working quite like it should... */
  else {

    logger(dbg_checks, more)
      << "Service is in a non-OK state!";

    /* check the route to the host if its up right now... */
    if (temp_host->current_state == HOST_UP) {
      logger(dbg_checks, more)
        << "Host is currently UP, so we'll recheck its state to "
        "make sure...";

      /* 08/04/07 EG launch an async (parallel) host check (possibly cached) unless aggressive host checking is enabled */
      /* previous logic was to simply run a sync (serial) host check */
      if (config->use_aggressive_host_checking() == true)
        perform_on_demand_host_check(
          temp_host,
          &route_result,
          CHECK_OPTION_NONE,
          TRUE,
          config->cached_host_check_horizon());
      else {
        /* can we use the last cached host state? */
        /* only use cached host state if no service state change has occurred */
        if ((state_change == FALSE
             || state_changes_use_cached_state == TRUE)
            && temp_host->has_been_checked == TRUE
            && (static_cast<unsigned long>(current_time - temp_host->last_check) <= config->cached_host_check_horizon())) {
          /* use current host state as route result */
          route_result = temp_host->current_state;
          logger(dbg_checks, more)
            << "* Using cached host state: " << temp_host->current_state;
          update_check_stats(ACTIVE_ONDEMAND_HOST_CHECK_STATS, current_time);
          update_check_stats(ACTIVE_CACHED_HOST_CHECK_STATS, current_time);
        }

        /* else launch an async (parallel) check of the host */
        /* CHANGED 02/15/08 only if service changed state since service was last checked */
        else if (state_change == TRUE) {
          /* use current host state as route result */
          route_result = temp_host->current_state;
          run_async_host_check_3x(
            temp_host,
            CHECK_OPTION_NONE,
            0.0,
            FALSE,
            FALSE,
            NULL,
            NULL);
        }

        /* ADDED 02/15/08 */
        /* else assume same host state */
        else {
          route_result = temp_host->current_state;
          logger(dbg_checks, more)
            << "* Using last known host state: "
            << temp_host->current_state;
          update_check_stats(
            ACTIVE_ONDEMAND_HOST_CHECK_STATS,
            current_time);
          update_check_stats(
            ACTIVE_CACHED_HOST_CHECK_STATS,
            current_time);
        }
      }
    }

    /* else the host is either down or unreachable, so recheck it if necessary */
    else {
      logger(dbg_checks, more)
        << "Host is currently DOWN/UNREACHABLE.";

      /* we're using aggressive host checking, so really do recheck the host... */
      if (config->use_aggressive_host_checking() == true) {
        logger(dbg_checks, more)
          << "Agressive host checking is enabled, so we'll recheck the "
          "host state...";
        perform_on_demand_host_check(
          temp_host,
          &route_result,
          CHECK_OPTION_NONE,
          TRUE,
          config->cached_host_check_horizon());
      }

      /* the service wobbled between non-OK states, so check the host... */
      else
        if ((state_change == TRUE
             && state_changes_use_cached_state == FALSE)
            && temp_service->last_hard_state != STATE_OK) {
	  logger(dbg_checks, more)
            << "Service wobbled between non-OK states, so we'll recheck"
            " the host state...";
	  /* 08/04/07 EG launch an async (parallel) host check unless aggressive host checking is enabled */
	  /* previous logic was to simply run a sync (serial) host check */
	  /* use current host state as route result */
	  route_result = temp_host->current_state;
	  run_async_host_check_3x(
            temp_host,
            CHECK_OPTION_NONE,
            0.0,
            FALSE,
            FALSE,
            NULL,
            NULL);
	  /*perform_on_demand_host_check(temp_host,&route_result,CHECK_OPTION_NONE,TRUE,config->cached_host_check_horizon()); */
	}

      /* else fake the host check, but (possibly) resend host notifications to contacts... */
	else {
	  logger(dbg_checks, more)
            << "Assuming host is in same state as before...";

	  /* if the host has never been checked before, set the checked flag and last check time */
	  /* 03/11/06 EG Note: This probably never evaluates to FALSE, present for historical reasons only, can probably be removed in the future */
	  if (temp_host->has_been_checked == FALSE) {
	    temp_host->has_been_checked = TRUE;
	    temp_host->last_check = temp_service->last_check;
	  }

	  /* fake the route check result */
	  route_result = temp_host->current_state;

	  /* possibly re-send host notifications... */
	  host_notification(
            temp_host,
            NOTIFICATION_NORMAL,
            NULL,
            NULL,
            NOTIFICATION_OPTION_NONE);
	}
    }

    /* if the host is down or unreachable ... */
    /* 05/29/2007 NOTE: The host might be in a SOFT problem state due to host check retries/caching.  Not sure if we should take that into account and do something different or not... */
    if (route_result != HOST_UP) {
      logger(dbg_checks, most)
        << "Host is not UP, so we mark state changes if appropriate";

      /* "fake" a hard state change for the service - well, its not really fake, but it didn't get caught earlier... */
      if (temp_service->last_hard_state != temp_service->current_state)
        hard_state_change = TRUE;

      /* update last state change times */
      if (state_change == TRUE || hard_state_change == TRUE)
        temp_service->last_state_change = temp_service->last_check;
      if (hard_state_change == TRUE) {
        temp_service->last_hard_state_change = temp_service->last_check;
        temp_service->state_type = HARD_STATE;
        temp_service->last_hard_state = temp_service->current_state;
      }

      /* put service into a hard state without attempting check retries and don't send out notifications about it */
      temp_service->host_problem_at_last_check = TRUE;
    }

    /* the host is up - it recovered since the last time the service was checked... */
    else if (temp_service->host_problem_at_last_check == TRUE) {
      /* next time the service is checked we shouldn't get into this same case... */
      temp_service->host_problem_at_last_check = FALSE;

      /* reset the current check counter, so we give the service a chance */
      /* this helps prevent the case where service has N max check attempts, N-1 of which have already occurred. */
      /* if we didn't do this, the next check might fail and result in a hard problem - we should really give it more time */
      /* ADDED IF STATEMENT 01-17-05 EG */
      /* 01-17-05: Services in hard problem states before hosts went down would sometimes come back as soft problem states after */
      /* the hosts recovered.  This caused problems, so hopefully this will fix it */
      if (temp_service->state_type == SOFT_STATE)
        temp_service->current_attempt = 1;
    }

    logger(dbg_checks, more)
      << "Current/Max Attempt(s): " << temp_service->current_attempt
      << '/' << temp_service->max_attempts;

    /* if we should retry the service check, do so (except it the host is down or unreachable!) */
    if (temp_service->current_attempt < temp_service->max_attempts) {

      /* the host is down or unreachable, so don't attempt to retry the service check */
      if (route_result != HOST_UP) {

        logger(dbg_checks, more)
          << "Host isn't UP, so we won't retry the service check...";

        /* the host is not up, so reschedule the next service check at regular interval */
        if (reschedule_check == TRUE)
          next_service_check
            = (time_t)(temp_service->last_check
                       + (temp_service->check_interval
                          * config->interval_length()));

        /* log the problem as a hard state if the host just went down */
        if (hard_state_change == TRUE) {
          log_service_event(temp_service);
          state_was_logged = TRUE;

          /* run the service event handler to handle the hard state */
          handle_service_event(temp_service);
        }
      }

      /* the host is up, so continue to retry the service check */
      else {

        logger(dbg_checks, more)
          << "Host is UP, so we'll retry the service check...";

        /* this is a soft state */
        temp_service->state_type = SOFT_STATE;

        /* log the service check retry */
        log_service_event(temp_service);
        state_was_logged = TRUE;

        /* run the service event handler to handle the soft state */
        handle_service_event(temp_service);

        if (reschedule_check == TRUE)
          next_service_check
            = (time_t)(temp_service->last_check
                       + (temp_service->retry_interval
                          * config->interval_length()));
      }

      /* perform dependency checks on the second to last check of the service */
      if (config->enable_predictive_service_dependency_checks() == true
          && temp_service->current_attempt == (temp_service->max_attempts - 1)) {

        logger(dbg_checks, more)
          << "Looking for services to check for predictive "
          "dependency checks...";

        /* check services that THIS ONE depends on for notification AND execution */
        /* we do this because we might be sending out a notification soon and we want the dependency logic to be accurate */
        std::pair<std::string, std::string>
          id(std::make_pair(temp_service->host_name, temp_service->description));
        umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency> > const&
          dependencies(state::instance().servicedependencies());
        for (umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency> >::const_iterator
               it(dependencies.find(id)), end(dependencies.end());
             it != end && it->first == id;
             ++it) {
          servicedependency* temp_dependency(&*it->second);

          if (temp_dependency->dependent_service_ptr == temp_service
              && temp_dependency->master_service_ptr != NULL) {
            master_service = (service*)temp_dependency->master_service_ptr;
            logger(dbg_checks, most)
              << "Predictive check of service '"
              << master_service->description << "' on host '"
              << master_service->host_name << "' queued.";
            add_object_to_objectlist(
              &check_servicelist,
              (void*)master_service);
          }
        }
      }
    }

    /* we've reached the maximum number of service rechecks, so handle the error */
    else {

      logger(dbg_checks, more)
        << "Service has reached max number of rechecks, so we'll "
        "handle the error...";

      /* this is a hard state */
      temp_service->state_type = HARD_STATE;

      /* if we've hard a hard state change... */
      if (hard_state_change == TRUE) {

        /* log the service problem (even if host is not up, which is new in 0.0.5) */
        log_service_event(temp_service);
        state_was_logged = TRUE;
      }

      /* else log the problem (again) if this service is flagged as being volatile */
      else if (temp_service->is_volatile == TRUE) {
        log_service_event(temp_service);
        state_was_logged = TRUE;
      }

      /* check for start of flexible (non-fixed) scheduled downtime if we just had a hard error */
      /* we need to check for both, state_change (SOFT) and hard_state_change (HARD) values */
      if (((TRUE == hard_state_change) || (TRUE == state_change))
          && (temp_service->pending_flex_downtime > 0))
        check_pending_flex_service_downtime(temp_service);

      /* 10/04/07 check to see if the service and/or associate host is flapping */
      /* this should be done before a notification is sent out to ensure the host didn't just start flapping */
      check_for_service_flapping(temp_service, TRUE, TRUE);
      check_for_host_flapping(temp_host, TRUE, FALSE, TRUE);
      flapping_check_done = TRUE;

      /* (re)send notifications out about this service problem if the host is up (and was at last check also) and the dependencies were okay... */
      service_notification(
        temp_service,
        NOTIFICATION_NORMAL,
        NULL,
        NULL,
        NOTIFICATION_OPTION_NONE);

      /* run the service event handler if we changed state from the last hard state or if this service is flagged as being volatile */
      if (hard_state_change == TRUE
          || temp_service->is_volatile == TRUE)
        handle_service_event(temp_service);

      /* save the last hard state */
      temp_service->last_hard_state = temp_service->current_state;

      /* reschedule the next check at the regular interval */
      if (reschedule_check == TRUE)
        next_service_check
          = (time_t)(temp_service->last_check
                     + (temp_service->check_interval
                        * config->interval_length()));
    }


    /* should we obsessive over service checks? */
    if (config->obsess_over_services() == true)
      obsessive_compulsive_service_check_processor(temp_service);
  }

  /* reschedule the next service check ONLY for active, scheduled checks */
  if (reschedule_check == TRUE) {

    logger(dbg_checks, more)
      << "Rescheduling next check of service at "
      << my_ctime(&next_service_check);

    /* default is to reschedule service check unless a test below fails... */
    temp_service->should_be_scheduled = TRUE;

    /* next check time was calculated above */
    temp_service->next_check = next_service_check;

    /* make sure we don't get ourselves into too much trouble... */
    if (current_time > temp_service->next_check)
      temp_service->next_check = current_time;

    /* make sure we rescheduled the next service check at a valid time */
    preferred_time = temp_service->next_check;
    get_next_valid_time(
      preferred_time,
      &next_valid_time,
      temp_service->check_period_ptr);
    temp_service->next_check = next_valid_time;

    /* services with non-recurring intervals do not get rescheduled */
    if (temp_service->check_interval == 0)
      temp_service->should_be_scheduled = FALSE;

    /* services with active checks disabled do not get rescheduled */
    if (temp_service->checks_enabled == FALSE)
      temp_service->should_be_scheduled = FALSE;

    /* schedule a non-forced check if we can */
    if (temp_service->should_be_scheduled == TRUE)
      schedule_service_check(
        temp_service,
        temp_service->next_check,
        CHECK_OPTION_NONE);
  }

  /* if we're stalking this state type and state was not already logged AND the plugin output changed since last check, log it now.. */
  if (temp_service->state_type == HARD_STATE && state_change == FALSE
      && state_was_logged == FALSE
      && compare_strings(
           old_plugin_output,
           temp_service->plugin_output)) {

    if ((temp_service->current_state == STATE_OK
         && temp_service->stalk_on_ok == TRUE))
      log_service_event(temp_service);

    else
      if ((temp_service->current_state == STATE_WARNING
           && temp_service->stalk_on_warning == TRUE))
	log_service_event(temp_service);

      else
	if ((temp_service->current_state == STATE_UNKNOWN
	     && temp_service->stalk_on_unknown == TRUE))
	  log_service_event(temp_service);

	else
	  if ((temp_service->current_state == STATE_CRITICAL
	       && temp_service->stalk_on_critical == TRUE))
	    log_service_event(temp_service);
  }

  /* send data to event broker */
  broker_service_check(
    NEBTYPE_SERVICECHECK_PROCESSED,
    NEBFLAG_NONE,
    NEBATTR_NONE, temp_service,
    temp_service->check_type,
    queued_check_result->start_time,
    queued_check_result->finish_time,
    NULL,
    temp_service->latency,
    temp_service->execution_time,
    config->service_check_timeout(),
    queued_check_result->early_timeout,
    queued_check_result->return_code,
    NULL,
    NULL);

  if (!(reschedule_check == TRUE
	&& temp_service->should_be_scheduled == TRUE
	&& temp_service->has_been_checked == TRUE)
      || temp_service->checks_enabled == FALSE) {
    /* set the checked flag */
    temp_service->has_been_checked = TRUE;
    /* update the current service status log */
    update_service_status(temp_service, FALSE);
  }

  /* check to see if the service and/or associate host is flapping */
  if (flapping_check_done == FALSE) {
    check_for_service_flapping(temp_service, TRUE, TRUE);
    check_for_host_flapping(temp_host, TRUE, FALSE, TRUE);
  }

  /* update service performance info */
  update_service_performance_data(temp_service);

  /* free allocated memory */
  delete[] old_plugin_output;


  /* run async checks of all services we added above */
  /* don't run a check if one is already executing or we can get by with a cached state */
  for (servicelist_item = check_servicelist;
       servicelist_item != NULL;
       servicelist_item = servicelist_item->next) {
    run_async_check = TRUE;
    temp_service = (service*)servicelist_item->object_ptr;

    /* we can get by with a cached state, so don't check the service */
    if (static_cast<unsigned long>(current_time - temp_service->last_check) <= config->cached_service_check_horizon()) {
      run_async_check = FALSE;

      /* update check statistics */
      update_check_stats(
        ACTIVE_CACHED_SERVICE_CHECK_STATS,
        current_time);
    }

    if (temp_service->is_executing == TRUE)
      run_async_check = FALSE;

    if (run_async_check == TRUE)
      run_async_service_check(
        temp_service,
        CHECK_OPTION_NONE,
        0.0,
        FALSE,
        FALSE,
        NULL,
        NULL);
  }
  free_objectlist(&check_servicelist);
  return (OK);
}

/* schedules an immediate or delayed service check */
void schedule_service_check(service* svc, time_t check_time, int options) {
  logger(dbg_functions, basic)
    << "schedule_service_check()";

  if (svc == NULL)
    return;

  logger(dbg_checks, basic)
    << "Scheduling a " << (options & CHECK_OPTION_FORCE_EXECUTION ? "forced" : "non-forced")
    << ", active check of service '" << svc->description
    << "' on host '" << svc->host_name
    << "' @ " << my_ctime(&check_time);

  /* don't schedule a check if active checks of this service are disabled */
  if (svc->checks_enabled == FALSE
      && !(options & CHECK_OPTION_FORCE_EXECUTION)) {
    logger(dbg_checks, basic)
      << "Active checks of this service are disabled.";
    return;
  }
  /* default is to use the new event */
  int use_original_event(FALSE);

  timed_event* temp_event = quick_timed_event.find(
                              hash_timed_event::low,
                              hash_timed_event::service_check,
                              svc);

  /* we found another service check event for this service in the queue - what should we do? */
  if (temp_event != NULL) {
    logger(dbg_checks, most)
      << "Found another service check event for this service @ "
      << my_ctime(&temp_event->run_time);

    /* use the originally scheduled check unless we decide otherwise */
    use_original_event = TRUE;

    /* the original event is a forced check... */
    if ((temp_event->event_options & CHECK_OPTION_FORCE_EXECUTION)) {

      /* the new event is also forced and its execution time is earlier than the original, so use it instead */
      if ((options & CHECK_OPTION_FORCE_EXECUTION)
          && (check_time < temp_event->run_time)) {
        use_original_event = FALSE;
        logger(dbg_checks, most)
          << "New service check event is forced and occurs before the "
          "existing event, so the new event will be used instead.";
      }
    }
    /* the original event is not a forced check... */
    else {
      /* the new event is a forced check, so use it instead */
      if ((options & CHECK_OPTION_FORCE_EXECUTION)) {
        use_original_event = FALSE;
        logger(dbg_checks, most)
          << "New service check event is forced, so it will be used "
          "instead of the existing event.";
      }
      /* the new event is not forced either and its execution time is earlier than the original, so use it instead */
      else if (check_time < temp_event->run_time) {
        use_original_event = FALSE;
        logger(dbg_checks, most)
          << "New service check event occurs before the existing "
          "(older) event, so it will be used instead.";
      }

      /* the new event is older, so override the existing one */
      else {
        logger(dbg_checks, most)
          << "New service check event occurs after the existing event, "
          "so we'll ignore it.";
      }
    }
  }
  /* else we're using the new event, so remove the old one */
  else {
    remove_event(temp_event, &event_list_low, &event_list_low_tail);
    delete temp_event;
    temp_event = NULL;
  }

  /* save check options for retention purposes */
  svc->check_options = options;

  /* schedule a new event */
  if (use_original_event == FALSE) {
    logger(dbg_checks, most)
      << "Scheduling new service check event.";

    /* allocate memory for a new event item */
    try {
      timed_event* new_event(new timed_event);

      /* set the next service check time */
      svc->next_check = check_time;

      /* place the new event in the event queue */
      new_event->event_type = EVENT_SERVICE_CHECK;
      new_event->event_data = (void*)svc;
      new_event->event_args = (void*)NULL;
      new_event->event_options = options;
      new_event->run_time = svc->next_check;
      new_event->recurring = FALSE;
      new_event->event_interval = 0L;
      new_event->timing_func = NULL;
      new_event->compensate_for_time_change = TRUE;
      reschedule_event(new_event, &event_list_low, &event_list_low_tail);
    }
    catch (...) {
      /* update the status log */
      update_service_status(svc, FALSE);
      throw;
    }
  }
  else {
    /* reset the next check time (it may be out of sync) */
    if (temp_event != NULL)
      svc->next_check = temp_event->run_time;

    logger(dbg_checks, most)
      << "Keeping original service check event (ignoring the new one).";
  }

  /* update the status log */
  update_service_status(svc, FALSE);
  return;
}

/* checks viability of performing a service check */
int check_service_check_viability(
      service* svc,
      int check_options,
      int* time_is_valid,
      time_t* new_time) {
  int perform_check = TRUE;
  time_t current_time = 0L;
  time_t preferred_time = 0L;
  int check_interval = 0;

  logger(dbg_functions, basic)
    << "check_service_check_viability()";

  /* make sure we have a service */
  if (svc == NULL)
    return (ERROR);

  /* get the check interval to use if we need to reschedule the check */
  if (svc->state_type == SOFT_STATE && svc->current_state != STATE_OK)
    check_interval = static_cast<int>(svc->retry_interval * config->interval_length());
  else
    check_interval = static_cast<int>(svc->check_interval * config->interval_length());

  /* get the current time */
  time(&current_time);

  /* initialize the next preferred check time */
  preferred_time = current_time;

  /* can we check the host right now? */
  if (!(check_options & CHECK_OPTION_FORCE_EXECUTION)) {

    /* if checks of the service are currently disabled... */
    if (svc->checks_enabled == FALSE) {
      preferred_time = current_time + check_interval;
      perform_check = FALSE;

      logger(dbg_checks, most)
        << "Active checks of the service are currently disabled.";
    }
    /* make sure this is a valid time to check the service */
    if (check_time_against_period(
          (unsigned long)current_time,
          svc->check_period_ptr) == ERROR) {
      preferred_time = current_time;
      if (time_is_valid)
        *time_is_valid = FALSE;
      perform_check = FALSE;

      logger(dbg_checks, most)
        << "This is not a valid time for this service to be actively "
        "checked.";
    }

    /* check service dependencies for execution */
    if (check_service_dependencies(
          svc,
          EXECUTION_DEPENDENCY) == DEPENDENCIES_FAILED) {
      preferred_time = current_time + check_interval;
      perform_check = FALSE;

      logger(dbg_checks, most)
        << "Execution dependencies for this service failed, so it will "
        "not be actively checked.";
    }
  }

  /* pass back the next viable check time */
  if (new_time)
    *new_time = preferred_time;

  return ((perform_check == TRUE) ? OK : ERROR);
}

/* checks service dependencies */
unsigned int check_service_dependencies(
               service* svc,
               int dependency_type) {
  service* temp_service = NULL;
  int state = STATE_OK;
  time_t current_time = 0L;

  logger(dbg_functions, basic)
    << "check_service_dependencies()";

  std::pair<std::string, std::string>
    id(svc->host_name, svc->description);
  umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency> > const&
    dependencies(state::instance().servicedependencies());
  for (umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency> >::const_iterator
         it(dependencies.find(id)), end(dependencies.end());
       it != end && it->first == id;
       ++it) {
    servicedependency* temp_dependency(&*it->second);

    /* only check dependencies of the desired type (notification or execution) */
    if (temp_dependency->dependency_type != dependency_type)
      continue;

    /* find the service we depend on... */
    if ((temp_service = temp_dependency->master_service_ptr) == NULL)
      continue;

    /* skip this dependency if it has a timeperiod and the current time isn't valid */
    time(&current_time);
    if (temp_dependency->dependency_period != NULL
        && check_time_against_period(
             current_time,
             temp_dependency->dependency_period_ptr) == ERROR)
      return (DEPENDENCIES_OK);

    /* get the status to use (use last hard state if its currently in a soft state) */
    if (temp_service->state_type == SOFT_STATE
        && config->soft_state_dependencies() == false)
      state = temp_service->last_hard_state;
    else
      state = temp_service->current_state;

    /* is the service we depend on in state that fails the dependency tests? */
    if (state == STATE_OK && temp_dependency->fail_on_ok == TRUE)
      return (DEPENDENCIES_FAILED);
    if (state == STATE_WARNING
        && temp_dependency->fail_on_warning == TRUE)
      return (DEPENDENCIES_FAILED);
    if (state == STATE_UNKNOWN
        && temp_dependency->fail_on_unknown == TRUE)
      return (DEPENDENCIES_FAILED);
    if (state == STATE_CRITICAL
        && temp_dependency->fail_on_critical == TRUE)
      return (DEPENDENCIES_FAILED);
    if ((state == STATE_OK && temp_service->has_been_checked == FALSE)
        && temp_dependency->fail_on_pending == TRUE)
      return (DEPENDENCIES_FAILED);

    /* immediate dependencies ok at this point - check parent dependencies if necessary */
    if (temp_dependency->inherits_parent == TRUE) {
      if (check_service_dependencies(
            temp_service,
            dependency_type) != DEPENDENCIES_OK)
        return (DEPENDENCIES_FAILED);
    }
  }
  return (DEPENDENCIES_OK);
}

/* check for services that never returned from a check... */
void check_for_orphaned_services() {
  service* temp_service = NULL;
  time_t current_time = 0L;
  time_t expected_time = 0L;

  logger(dbg_functions, basic)
    << "check_for_orphaned_services()";

  /* get the current time */
  time(&current_time);

  /* check all services... */
  for (temp_service = service_list;
       temp_service != NULL;
       temp_service = temp_service->next) {

    /* skip services that are not currently executing */
    if (temp_service->is_executing == FALSE)
      continue;

    /* determine the time at which the check results should have come in (allow 10 minutes slack time) */
    expected_time
      = (time_t)(temp_service->next_check + temp_service->latency
                 + config->service_check_timeout()
                 + config->check_reaper_interval() + 600);

    /* this service was supposed to have executed a while ago, but for some reason the results haven't come back in... */
    if (expected_time < current_time) {

      /* log a warning */
      logger(log_runtime_warning, basic)
        << "Warning: The check of service '"
        << temp_service->description << "' on host '"
        << temp_service->host_name << "' looks like it was orphaned "
        "(results never came back).  I'm scheduling an immediate check "
        "of the service...";

      logger(dbg_checks, more)
        << "Service '" << temp_service->description
        << "' on host '" << temp_service->host_name
        << "' was orphaned, so we're scheduling an immediate check...";

      /* decrement the number of running service checks */
      if (currently_running_service_checks > 0)
        currently_running_service_checks--;

      /* disable the executing flag */
      temp_service->is_executing = FALSE;

      /* schedule an immediate check of the service */
      schedule_service_check(
        temp_service,
        current_time,
        CHECK_OPTION_ORPHAN_CHECK);
    }
  }
  return;
}

/* check freshness of service results */
void check_service_result_freshness() {
  service* temp_service = NULL;
  time_t current_time = 0L;

  logger(dbg_functions, basic)
    << "check_service_result_freshness()";
  logger(dbg_checks, more)
    << "Checking the freshness of service check results...";

  /* bail out if we're not supposed to be checking freshness */
  if (config->check_service_freshness() == false) {
    logger(dbg_checks, more)
      << "Service freshness checking is disabled.";
    return;
  }
  /* get the current time */
  time(&current_time);

  /* check all services... */
  for (temp_service = service_list;
       temp_service != NULL;
       temp_service = temp_service->next) {

    /* skip services we shouldn't be checking for freshness */
    if (temp_service->check_freshness == FALSE)
      continue;

    /* skip services that are currently executing (problems here will be caught by orphaned service check) */
    if (temp_service->is_executing == TRUE)
      continue;

    /* skip services that have both active and passive checks disabled */
    if (temp_service->checks_enabled == FALSE
        && temp_service->accept_passive_service_checks == FALSE)
      continue;

    /* skip services that are already being freshened */
    if (temp_service->is_being_freshened == TRUE)
      continue;

    /* see if the time is right... */
    if (check_time_against_period(
          current_time,
          temp_service->check_period_ptr) == ERROR)
      continue;

    /* EXCEPTION */
    /* don't check freshness of services without regular check intervals if we're using auto-freshness threshold */
    if (temp_service->check_interval == 0
        && temp_service->freshness_threshold == 0)
      continue;

    /* the results for the last check of this service are stale! */
    if (is_service_result_fresh(
          temp_service, current_time,
          TRUE) == FALSE) {

      /* set the freshen flag */
      temp_service->is_being_freshened = TRUE;

      /* schedule an immediate forced check of the service */
      schedule_service_check(
        temp_service,
        current_time,
        CHECK_OPTION_FORCE_EXECUTION | CHECK_OPTION_FRESHNESS_CHECK);
    }
  }
  return;
}

/* tests whether or not a service's check results are fresh */
int is_service_result_fresh(
      service* temp_service,
      time_t current_time,
      int log_this) {
  int freshness_threshold = 0;
  time_t expiration_time = 0L;
  int days = 0;
  int hours = 0;
  int minutes = 0;
  int seconds = 0;
  int tdays = 0;
  int thours = 0;
  int tminutes = 0;
  int tseconds = 0;

  logger(dbg_checks, most)
    << "Checking freshness of service '" << temp_service->description
    << "' on host '" << temp_service->host_name << "'...";

  /* use user-supplied freshness threshold or auto-calculate a freshness threshold to use? */
  if (temp_service->freshness_threshold == 0) {
    if (temp_service->state_type == HARD_STATE
        || temp_service->current_state == STATE_OK)
      freshness_threshold = static_cast<int>((temp_service->check_interval * config->interval_length())
					     + temp_service->latency + config->additional_freshness_latency());
    else
      freshness_threshold = static_cast<int>((temp_service->retry_interval * config->interval_length())
					     + temp_service->latency + config->additional_freshness_latency());
  }
  else
    freshness_threshold = temp_service->freshness_threshold;

  logger(dbg_checks, most)
    << "Freshness thresholds: service="
    << temp_service->freshness_threshold
    << ", use=" << freshness_threshold;

  /* calculate expiration time */
  /* CHANGED 11/10/05 EG - program start is only used in expiration time calculation if > last check AND active checks are enabled, so active checks can become stale immediately upon program startup */
  /* CHANGED 02/25/06 SG - passive checks also become stale, so remove dependence on active check logic */
  if (temp_service->has_been_checked == FALSE)
    expiration_time = (time_t)(event_start + freshness_threshold);
  /* CHANGED 06/19/07 EG - Per Ton's suggestion (and user requests), only use program start time over last check if no specific threshold has been set by user.  Otheriwse use it.  Problems can occur if Engine is restarted more frequently that freshness threshold intervals (services never go stale). */
  /* CHANGED 10/07/07 EG - Only match next condition for services that have active checks enabled... */
  /* CHANGED 10/07/07 EG - Added max_service_check_spread to expiration time as suggested by Altinity */
  else if (temp_service->checks_enabled == TRUE
           && event_start > temp_service->last_check
           && temp_service->freshness_threshold == 0)
    expiration_time
      = (time_t)(event_start + freshness_threshold
                 + (config->max_service_check_spread()
                    * config->interval_length()));
  else
    expiration_time
      = (time_t)(temp_service->last_check + freshness_threshold);

  logger(dbg_checks, most)
    << "HBC: " << temp_service->has_been_checked
    << ", PS: " << program_start
    << ", ES: " << event_start
    << ", LC: " << temp_service->last_check
    << ", CT: " << current_time
    << ", ET: " << expiration_time;

  /* the results for the last check of this service are stale */
  if (expiration_time < current_time) {

    get_time_breakdown(
      (current_time - expiration_time),
      &days,
      &hours,
      &minutes,
      &seconds);
    get_time_breakdown(
      freshness_threshold,
      &tdays,
      &thours,
      &tminutes,
      &tseconds);

    /* log a warning */
    if (log_this == TRUE)
      logger(log_runtime_warning, basic)
        << "Warning: The results of service '" << temp_service->description
        << "' on host '" << temp_service->host_name << "' are stale by "
        << days << "d " << hours << "h " << minutes << "m " << seconds
        << "s (threshold=" << tdays << "d " << thours << "h " << tminutes
        << "m " << tseconds << "s).  I'm forcing an immediate check "
        "of the service.";

    logger(dbg_checks, more)
      << "Check results for service '" << temp_service->description
      << "' on host '" << temp_service->host_name << "' are stale by "
      << days << "d " << hours << "h " << minutes << "m " << seconds
      << "s (threshold=" << tdays << "d " << thours << "h " << tminutes
      << "m " << tseconds << "s).  Forcing an immediate check of "
      "the service...";

    return (FALSE);
  }

  logger(dbg_checks, more)
    << "Check results for service '" << temp_service->description
    << "' on host '" << temp_service->host_name << "' are fresh.";

  return (TRUE);
}

/******************************************************************/
/*************** COMMON ROUTE/HOST CHECK FUNCTIONS ****************/
/******************************************************************/

/* execute an on-demand check  */
int perform_on_demand_host_check(
      host* hst,
      int* check_return_code,
      int check_options,
      int use_cached_result,
      unsigned long check_timestamp_horizon) {
  logger(dbg_functions, basic)
    << "perform_on_demand_host_check()";

  perform_on_demand_host_check_3x(
    hst,
    check_return_code,
    check_options,
    use_cached_result,
    check_timestamp_horizon);
  return (OK);
}

/* execute a scheduled host check using either the 2.x or 3.x logic */
int perform_scheduled_host_check(
     host* hst,
     int check_options,
     double latency) {
  logger(dbg_functions, basic)
    << "perform_scheduled_host_check()";
  run_scheduled_host_check_3x(hst, check_options, latency);
  return (OK);
}

/* schedules an immediate or delayed host check */
void schedule_host_check(host* hst, time_t check_time, int options) {
  timed_event* temp_event = NULL;
  timed_event* new_event = NULL;
  int use_original_event = TRUE;

  logger(dbg_functions, basic)
    << "schedule_host_check()";

  if (hst == NULL)
    return;

  logger(dbg_checks, basic)
    << "Scheduling a "
    << (options & CHECK_OPTION_FORCE_EXECUTION ? "forced" : "non-forced")
    << ", active check of host '" << hst->name << "' @ "
    << my_ctime(&check_time);

  /* don't schedule a check if active checks of this host are disabled */
  if (hst->checks_enabled == FALSE
      && !(options & CHECK_OPTION_FORCE_EXECUTION)) {
    logger(dbg_checks, basic)
      << "Active checks are disabled for this host.";
    return;
  }
  /* allocate memory for a new event item */
  new_event = new timed_event;

  /* default is to use the new event */
  use_original_event = FALSE;

#ifdef PERFORMANCE_INCREASE_BUT_VERY_BAD_IDEA_INDEED
  /* WARNING! 1/19/07 on-demand async host checks will end up causing mutliple scheduled checks of a host to appear in the queue if the code below is skipped */
  /* if(use_large_installation_tweaks==FALSE)... skip code below */
#endif

  /* see if there are any other scheduled checks of this host in the queue */
  temp_event = quick_timed_event.find(
                                   hash_timed_event::low,
                                   hash_timed_event::host_check,
                                   hst);

  /* we found another host check event for this host in the queue - what should we do? */
  if (temp_event != NULL) {

    logger(dbg_checks, most)
      << "Found another host check event for this host @ "
      << my_ctime(&temp_event->run_time);

    /* use the originally scheduled check unless we decide otherwise */
    use_original_event = TRUE;

    /* the original event is a forced check... */
    if ((temp_event->event_options & CHECK_OPTION_FORCE_EXECUTION)) {

      /* the new event is also forced and its execution time is earlier than the original, so use it instead */
      if ((options & CHECK_OPTION_FORCE_EXECUTION)
          && (check_time < temp_event->run_time)) {
        logger(dbg_checks, most)
          << "New host check event is forced and occurs before the "
          "existing event, so the new event be used instead.";
        use_original_event = FALSE;
      }
    }

    /* the original event is not a forced check... */
    else {

      /* the new event is a forced check, so use it instead */
      if ((options & CHECK_OPTION_FORCE_EXECUTION)) {
        use_original_event = FALSE;
        logger(dbg_checks, most)
          << "New host check event is forced, so it will be used "
          "instead of the existing event.";
      }

      /* the new event is not forced either and its execution time is earlier than the original, so use it instead */
      else if (check_time < temp_event->run_time) {
        use_original_event = FALSE;
        logger(dbg_checks, most)
          << "New host check event occurs before the existing (older) "
          "event, so it will be used instead.";
      }

      /* the new event is older, so override the existing one */
      else {
        logger(dbg_checks, most)
          << "New host check event occurs after the existing event, "
          "so we'll ignore it.";
      }
    }

    /* the originally queued event won the battle, so keep it */
    if (use_original_event == TRUE) {
      delete new_event;
    }

    /* else use the new event, so remove the old */
    else {
      remove_event(temp_event, &event_list_low, &event_list_low_tail);
      delete temp_event;
    }
  }

  /* save check options for retention purposes */
  hst->check_options = options;

  /* use the new event */
  if (use_original_event == FALSE) {
    logger(dbg_checks, most)
      << "Scheduling new host check event.";

    /* set the next host check time */
    hst->next_check = check_time;

    /* place the new event in the event queue */
    new_event->event_type = EVENT_HOST_CHECK;
    new_event->event_data = (void*)hst;
    new_event->event_args = (void*)NULL;
    new_event->event_options = options;
    new_event->run_time = hst->next_check;
    new_event->recurring = FALSE;
    new_event->event_interval = 0L;
    new_event->timing_func = NULL;
    new_event->compensate_for_time_change = TRUE;
    reschedule_event(new_event, &event_list_low, &event_list_low_tail);
  }

  else {
    /* reset the next check time (it may be out of sync) */
    if (temp_event != NULL)
      hst->next_check = temp_event->run_time;

    logger(dbg_checks, most)
      << "Keeping original host check event (ignoring the new one).";
  }

  /* update the status log */
  update_host_status(hst, FALSE);
  return;
}

/* checks host dependencies */
unsigned int check_host_dependencies(host* hst, int dependency_type) {
  host* temp_host = NULL;
  int state = HOST_UP;
  time_t current_time = 0L;

  logger(dbg_functions, basic)
    << "check_host_dependencies()";

  std::string id(hst->name);
  umultimap<std::string, shared_ptr<hostdependency> > const&
    dependencies(state::instance().hostdependencies());

  /* check all dependencies... */
  for (umultimap<std::string, shared_ptr<hostdependency> >::const_iterator
         it(dependencies.find(id)), end(dependencies.end());
       it != end && it->first == id;
       ++it) {
         hostdependency* temp_dependency(&*it->second);

    /* only check dependencies of the desired type (notification or execution) */
    if (temp_dependency->dependency_type != dependency_type)
      continue;

    /* find the host we depend on... */
    if ((temp_host = temp_dependency->master_host_ptr) == NULL)
      continue;

    /* skip this dependency if it has a timeperiod and the current time isn't valid */
    time(&current_time);
    if (temp_dependency->dependency_period != NULL
        && check_time_against_period(
             current_time,
             temp_dependency->dependency_period_ptr) == ERROR)
      return (DEPENDENCIES_OK);

    /* get the status to use (use last hard state if its currently in a soft state) */
    if (temp_host->state_type == SOFT_STATE
        && config->soft_state_dependencies() == false)
      state = temp_host->last_hard_state;
    else
      state = temp_host->current_state;

    /* is the host we depend on in state that fails the dependency tests? */
    if (state == HOST_UP && temp_dependency->fail_on_up == TRUE)
      return (DEPENDENCIES_FAILED);
    if (state == HOST_DOWN && temp_dependency->fail_on_down == TRUE)
      return (DEPENDENCIES_FAILED);
    if (state == HOST_UNREACHABLE
        && temp_dependency->fail_on_unreachable == TRUE)
      return (DEPENDENCIES_FAILED);
    if ((state == HOST_UP && temp_host->has_been_checked == FALSE)
        && temp_dependency->fail_on_pending == TRUE)
      return (DEPENDENCIES_FAILED);

    /* immediate dependencies ok at this point - check parent dependencies if necessary */
    if (temp_dependency->inherits_parent == TRUE) {
      if (check_host_dependencies(
            temp_host,
            dependency_type) != DEPENDENCIES_OK)
        return (DEPENDENCIES_FAILED);
    }
  }
  return (DEPENDENCIES_OK);
}

/* check for hosts that never returned from a check... */
void check_for_orphaned_hosts() {
  host* temp_host = NULL;
  time_t current_time = 0L;
  time_t expected_time = 0L;

  logger(dbg_functions, basic)
    << "check_for_orphaned_hosts()";

  /* get the current time */
  time(&current_time);

  /* check all hosts... */
  for (temp_host = host_list;
       temp_host != NULL;
       temp_host = temp_host->next) {

    /* skip hosts that don't have a set check interval (on-demand checks are missed by the orphan logic) */
    if (temp_host->next_check == (time_t)0L)
      continue;

    /* skip hosts that are not currently executing */
    if (temp_host->is_executing == FALSE)
      continue;

    /* determine the time at which the check results should have come in (allow 10 minutes slack time) */
    expected_time
      = (time_t)(temp_host->next_check + temp_host->latency
                 + config->host_check_timeout()
                 + config->check_reaper_interval() + 600);

    /* this host was supposed to have executed a while ago, but for some reason the results haven't come back in... */
    if (expected_time < current_time) {

      /* log a warning */
      logger(log_runtime_warning, basic)
        << "Warning: The check of host '" << temp_host->name
        << "' looks like it was orphaned (results never came back).  "
        "I'm scheduling an immediate check of the host...";

      logger(dbg_checks, more)
        << "Host '" << temp_host->name
        << "' was orphaned, so we're scheduling an immediate check...";

      /* decrement the number of running host checks */
      if (currently_running_host_checks > 0)
        currently_running_host_checks--;

      /* disable the executing flag */
      temp_host->is_executing = FALSE;

      /* schedule an immediate check of the host */
      schedule_host_check(
        temp_host,
        current_time,
        CHECK_OPTION_ORPHAN_CHECK);
    }
  }
  return;
}

/* check freshness of host results */
void check_host_result_freshness() {
  host* temp_host = NULL;
  time_t current_time = 0L;

  logger(dbg_functions, basic)
    << "check_host_result_freshness()";
  logger(dbg_checks, most)
    << "Attempting to check the freshness of host check results...";

  /* bail out if we're not supposed to be checking freshness */
  if (config->check_host_freshness() == false) {
    logger(dbg_checks, most)
      << "Host freshness checking is disabled.";
    return;
  }

  /* get the current time */
  time(&current_time);

  /* check all hosts... */
  for (temp_host = host_list;
       temp_host != NULL;
       temp_host = temp_host->next) {

    /* skip hosts we shouldn't be checking for freshness */
    if (temp_host->check_freshness == FALSE)
      continue;

    /* skip hosts that have both active and passive checks disabled */
    if (temp_host->checks_enabled == FALSE
        && temp_host->accept_passive_host_checks == FALSE)
      continue;

    /* skip hosts that are currently executing (problems here will be caught by orphaned host check) */
    if (temp_host->is_executing == TRUE)
      continue;

    /* skip hosts that are already being freshened */
    if (temp_host->is_being_freshened == TRUE)
      continue;

    /* see if the time is right... */
    if (check_time_against_period(
          current_time,
          temp_host->check_period_ptr) == ERROR)
      continue;

    /* the results for the last check of this host are stale */
    if (is_host_result_fresh(temp_host, current_time, TRUE) == FALSE) {

      /* set the freshen flag */
      temp_host->is_being_freshened = TRUE;

      /* schedule an immediate forced check of the host */
      schedule_host_check(
        temp_host, current_time,
        CHECK_OPTION_FORCE_EXECUTION |
        CHECK_OPTION_FRESHNESS_CHECK);
    }
  }
  return;
}

/* checks to see if a hosts's check results are fresh */
int is_host_result_fresh(
      host* temp_host,
      time_t current_time,
      int log_this) {
  time_t expiration_time = 0L;
  int freshness_threshold = 0;
  int days = 0;
  int hours = 0;
  int minutes = 0;
  int seconds = 0;
  int tdays = 0;
  int thours = 0;
  int tminutes = 0;
  int tseconds = 0;

  logger(dbg_checks, most)
    << "Checking freshness of host '" << temp_host->name << "'...";

  /* use user-supplied freshness threshold or auto-calculate a freshness threshold to use? */
  if (temp_host->freshness_threshold == 0) {
    double interval;
    if ((HARD_STATE == temp_host->state_type)
        || (STATE_OK == temp_host->current_state))
      interval = temp_host->check_interval;
    else
      interval = temp_host->retry_interval;
    freshness_threshold
      = static_cast<int>((interval * config->interval_length())
                         + temp_host->latency
                         + config->additional_freshness_latency());
  }
  else
    freshness_threshold = temp_host->freshness_threshold;

  logger(dbg_checks, most)
    << "Freshness thresholds: host=" << temp_host->freshness_threshold
    << ", use=" << freshness_threshold;

  /* calculate expiration time */
  /* CHANGED 11/10/05 EG - program start is only used in expiration time calculation if > last check AND active checks are enabled, so active checks can become stale immediately upon program startup */
  if (temp_host->has_been_checked == FALSE)
    expiration_time = (time_t)(event_start + freshness_threshold);
  /* CHANGED 06/19/07 EG - Per Ton's suggestion (and user requests), only use program start time over last check if no specific threshold has been set by user.  Otheriwse use it.  Problems can occur if Engine is restarted more frequently that freshness threshold intervals (hosts never go stale). */
  /* CHANGED 10/07/07 EG - Added max_host_check_spread to expiration time as suggested by Altinity */
  else if (temp_host->checks_enabled == TRUE
           && event_start > temp_host->last_check
           && temp_host->freshness_threshold == 0)
    expiration_time
      = (time_t)(event_start + freshness_threshold
                 + (config->max_host_check_spread()
                    * config->interval_length()));
  else
    expiration_time
      = (time_t)(temp_host->last_check + freshness_threshold);

  logger(dbg_checks, most)
    << "HBC: " << temp_host->has_been_checked
    << ", PS: " << program_start
    << ", ES: " << event_start
    << ", LC: " << temp_host->last_check
    << ", CT: " << current_time
    << ", ET: " << expiration_time;

  /* the results for the last check of this host are stale */
  if (expiration_time < current_time) {
    get_time_breakdown(
      (current_time - expiration_time),
      &days,
      &hours,
      &minutes,
      &seconds);
    get_time_breakdown(
      freshness_threshold,
      &tdays,
      &thours,
      &tminutes,
      &tseconds);

    /* log a warning */
    if (log_this == TRUE)
      logger(log_runtime_warning, basic)
        << "Warning: The results of host '" << temp_host->name
        << "' are stale by " << days << "d " << hours << "h "
        << minutes << "m " << seconds << "s (threshold="
        << tdays << "d " << thours << "h " << tminutes << "m "
        << tseconds << "s).  I'm forcing an immediate check of"
        " the host.";

    logger(dbg_checks, more)
      << "Check results for host '" << temp_host->name
      << "' are stale by " << days << "d " << hours << "h " << minutes
      << "m " << seconds << "s (threshold=" << tdays << "d " << thours
      << "h " << tminutes << "m " << tseconds << "s).  "
      "Forcing an immediate check of the host...";

    return (FALSE);
  }
  else
    logger(dbg_checks, more)
      << "Check results for host '" << temp_host->name
      << "' are fresh.";

  return (TRUE);
}

/******************************************************************/
/************* NAGIOS 3.X ROUTE/HOST CHECK FUNCTIONS **************/
/******************************************************************/

/*** ON-DEMAND HOST CHECKS USE THIS FUNCTION ***/
/* check to see if we can reach the host */
int perform_on_demand_host_check_3x(
      host* hst,
      int* check_result_code,
      int check_options,
      int use_cached_result,
      unsigned long check_timestamp_horizon) {
  int result = OK;

  logger(dbg_functions, basic)
    << "perform_on_demand_host_check_3x()";

  /* make sure we have a host */
  if (hst == NULL)
    return (ERROR);

  logger(dbg_checks, basic)
    << "** On-demand check for host '" << hst->name << "'...";

  /* check the status of the host */
  result = run_sync_host_check_3x(
             hst,
             check_result_code,
             check_options,
             use_cached_result,
             check_timestamp_horizon);
  return (result);
}

/* perform a synchronous check of a host *//* on-demand host checks will use this... */
int run_sync_host_check_3x(
      host* hst,
      int* check_result_code,
      int check_options,
      int use_cached_result,
      unsigned long check_timestamp_horizon) {
  logger(dbg_functions, basic)
    << "run_sync_host_check_3x: hst=" << hst
    << ", check_options=" << check_options
    << ", use_cached_result=" << use_cached_result
    << ", check_timestamp_horizon=" << check_timestamp_horizon;

  try {
    checks::checker::instance().run_sync(
                                  hst,
                                  check_result_code,
                                  check_options,
                                  use_cached_result,
                                  check_timestamp_horizon);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "error: " << e.what();
    return (ERROR);
  }
  return (OK);
}

int execute_sync_host_check_3x(host* hst) {
  (void)hst;
  return (ERROR);
}

/* run a scheduled host check asynchronously */
int run_scheduled_host_check_3x(
      host* hst,
      int check_options,
      double latency) {
  int result = OK;
  time_t current_time = 0L;
  time_t preferred_time = 0L;
  time_t next_valid_time = 0L;
  int time_is_valid = TRUE;

  logger(dbg_functions, basic)
    << "run_scheduled_host_check_3x()";

  if (hst == NULL)
    return (ERROR);

  logger(dbg_checks, basic)
    << "Attempting to run scheduled check of host '" << hst->name
    << "': check options=" << check_options << ", latency=" << latency;

  /* attempt to run the check */
  result = run_async_host_check_3x(
             hst,
             check_options,
             latency,
             TRUE,
             TRUE,
             &time_is_valid,
             &preferred_time);

  /* an error occurred, so reschedule the check */
  if (result == ERROR) {
    logger(dbg_checks, more)
      << "Unable to run scheduled host check at this time";

    /* only attempt to (re)schedule checks that should get checked... */
    if (hst->should_be_scheduled == TRUE) {

      /* get current time */
      time(&current_time);

      /* determine next time we should check the host if needed */
      /* if host has no check interval, schedule it again for 5 minutes from now */
      if (current_time >= preferred_time)
        preferred_time
          = current_time + static_cast<time_t>((hst->check_interval <= 0)
                                               ? 300
                                               : (hst->check_interval * config->interval_length()));

      /* make sure we rescheduled the next host check at a valid time */
      get_next_valid_time(
        preferred_time,
        &next_valid_time,
        hst->check_period_ptr);

      /* the host could not be rescheduled properly - set the next check time for next week */
      if (time_is_valid == FALSE && next_valid_time == preferred_time) {
        /*
	  hst->next_check=(time_t)(next_valid_time+(60*60*24*365));
	  hst->should_be_scheduled=FALSE;
	*/

        hst->next_check = (time_t)(next_valid_time + (60 * 60 * 24 * 7));

        logger(log_runtime_warning, basic)
          << "Warning: Check of host '" << hst->name << "' could not be "
          "rescheduled properly.  Scheduling check for next week...";

        logger(dbg_checks, more)
          << "Unable to find any valid times to reschedule the next"
          " host check!";
      }
      /* this service could be rescheduled... */
      else {
        hst->next_check = next_valid_time;
        hst->should_be_scheduled = TRUE;

        logger(dbg_checks, more)
          << "Rescheduled next host check for "
          << my_ctime(&next_valid_time);
      }
    }

    /* update the status log */
    update_host_status(hst, FALSE);

    /* reschedule the next host check - unless we couldn't find a valid next check time */
    /* 10/19/07 EG - keep original check options */
    if (hst->should_be_scheduled == TRUE)
      schedule_host_check(hst, hst->next_check, check_options);

    return (ERROR);
  }
  return (OK);
}

/* perform an asynchronous check of a host */
/* scheduled host checks will use this, as will some checks that result from on-demand checks... */
int run_async_host_check_3x(
      host* hst,
      int check_options,
      double latency,
      int scheduled_check,
      int reschedule_check,
      int* time_is_valid,
      time_t* preferred_time) {
  try {
    checks::checker::instance().run(
                                  hst,
                                  check_options,
                                  latency,
                                  scheduled_check,
                                  reschedule_check,
                                  time_is_valid,
                                  preferred_time);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "error: " << e.what();
    return (ERROR);
  }
  return (OK);
}

/* process results of an asynchronous host check */
int handle_async_host_check_result_3x(
      host* temp_host,
      check_result* queued_check_result) {
  time_t current_time;
  int result = STATE_OK;
  int reschedule_check = FALSE;
  char* old_plugin_output = NULL;
  char* temp_ptr = NULL;
  struct timeval start_time_hires;
  struct timeval end_time_hires;
  double execution_time(0.0);

  logger(dbg_functions, basic)
    << "handle_async_host_check_result_3x()";

  /* make sure we have what we need */
  if (temp_host == NULL || queued_check_result == NULL)
    return (ERROR);

  time(&current_time);

  execution_time
    = (double)((double)(queued_check_result->finish_time.tv_sec
                        - queued_check_result->start_time.tv_sec)
               + (double)((queued_check_result->finish_time.tv_usec
                           - queued_check_result->start_time.tv_usec)
                          / 1000.0) / 1000.0);
  if (execution_time < 0.0)
    execution_time = 0.0;

  logger(dbg_checks, more)
    << "** Handling async check result for host '"
    << temp_host->name << "'...";

  logger(dbg_checks, most)
    << "\tCheck Type:         "
    << (queued_check_result->check_type == HOST_CHECK_ACTIVE ? "Active" : "Passive") << "\n"
    << "\tCheck Options:      "
    << queued_check_result->check_options << "\n"
    << "\tScheduled Check?:   "
    << (queued_check_result->scheduled_check == true ? "Yes" : "No") << "\n"
    << "\tReschedule Check?:  "
    << (queued_check_result->reschedule_check == true ? "Yes" : "No") << "\n"
    << "\tExited OK?:         "
    << (queued_check_result->exited_ok == true ? "Yes" : "No") << "\n"
    << com::centreon::logging::setprecision(3)
    << "\tExec Time:          " << execution_time << "\n"
    << "\tLatency:            " << queued_check_result->latency << "\n"
    << "\treturn Status:      " << queued_check_result->return_code << "\n"
    << "\tOutput:             " << queued_check_result->output;

  /* decrement the number of host checks still out there... */
  if (queued_check_result->check_type == HOST_CHECK_ACTIVE
      && currently_running_host_checks > 0)
    currently_running_host_checks--;

  /* skip this host check results if its passive and we aren't accepting passive check results */
  if (queued_check_result->check_type == HOST_CHECK_PASSIVE) {
    if (config->accept_passive_host_checks() == false) {
      logger(dbg_checks, basic)
        << "Discarding passive host check result because passive host "
        "checks are disabled globally.";
      return (ERROR);
    }
    if (temp_host->accept_passive_host_checks == FALSE) {
      logger(dbg_checks, basic)
        << "Discarding passive host check result because passive checks "
        "are disabled for this host.";
      return (ERROR);
    }
  }

  /* clear the freshening flag (it would have been set if this host was determined to be stale) */
  if (queued_check_result->check_options & CHECK_OPTION_FRESHNESS_CHECK)
    temp_host->is_being_freshened = FALSE;

  /* DISCARD INVALID FRESHNESS CHECK RESULTS */
  /* If a host goes stale, Engine will initiate a forced check in order
  ** to freshen it. There is a race condition whereby a passive check
  ** could arrive between the 1) initiation of the forced check and 2)
  ** the time when the forced check result is processed here. This would
  ** make the host fresh again, so we do a quick check to make sure the
  ** host is still stale before we accept the check result.
  */
  if ((queued_check_result->check_options & CHECK_OPTION_FRESHNESS_CHECK)
      && is_host_result_fresh(temp_host, current_time, FALSE) == TRUE) {
    logger(dbg_checks, basic)
      << "Discarding host freshness check result because the host is "
      "currently fresh (race condition avoided).";
    return (OK);
  }

  /* was this check passive or active? */
  temp_host->check_type = (queued_check_result->check_type == HOST_CHECK_ACTIVE)
    ? HOST_CHECK_ACTIVE : HOST_CHECK_PASSIVE;

  /* update check statistics for passive results */
  if (queued_check_result->check_type == HOST_CHECK_PASSIVE)
    update_check_stats(
      PASSIVE_HOST_CHECK_STATS,
      queued_check_result->start_time.tv_sec);

  /* should we reschedule the next check of the host? NOTE: this might be overridden later... */
  reschedule_check = queued_check_result->reschedule_check;

  /* check latency is passed to us for both active and passive checks */
  temp_host->latency = queued_check_result->latency;

  /* update the execution time for this check (millisecond resolution) */
  temp_host->execution_time = execution_time;

  /* set the checked flag */
  temp_host->has_been_checked = TRUE;

  /* clear the execution flag if this was an active check */
  if (queued_check_result->check_type == HOST_CHECK_ACTIVE)
    temp_host->is_executing = FALSE;

  /* get the last check time */
  temp_host->last_check = queued_check_result->start_time.tv_sec;

  /* was this check passive or active? */
  temp_host->check_type = (queued_check_result->check_type == HOST_CHECK_ACTIVE)
    ? HOST_CHECK_ACTIVE : HOST_CHECK_PASSIVE;

  /* save the old host state */
  temp_host->last_state = temp_host->current_state;
  if (temp_host->state_type == HARD_STATE)
    temp_host->last_hard_state = temp_host->current_state;

  /* save old plugin output */
  if (temp_host->plugin_output)
    old_plugin_output = temp_host->plugin_output;

  /* clear the old plugin output and perf data buffers */
  delete[] temp_host->long_plugin_output;
  delete[] temp_host->perf_data;

  temp_host->plugin_output = NULL;
  temp_host->long_plugin_output = NULL;
  temp_host->perf_data = NULL;

  /* parse check output to get: (1) short output, (2) long output, (3) perf data */
  parse_check_output(
    queued_check_result->output,
    &temp_host->plugin_output,
    &temp_host->long_plugin_output,
    &temp_host->perf_data,
    TRUE,
    TRUE);

  /* make sure we have some data */
  if (temp_host->plugin_output == NULL
      || !strcmp(temp_host->plugin_output, "")) {
    delete[] temp_host->plugin_output;
    temp_host->plugin_output
      = my_strdup("(No output returned from host check)");
  }

  /* replace semicolons in plugin output (but not performance data) with colons */
  if ((temp_ptr = temp_host->plugin_output)) {
    while ((temp_ptr = strchr(temp_ptr, ';')))
      *temp_ptr = ':';
  }

  logger(dbg_checks, most)
    << "Parsing check output...\n"
    << "Short Output:\n"
    << (temp_host->plugin_output == NULL ? "NULL" : temp_host->plugin_output) << "\n"
    << "Long Output:\n"
    << (temp_host->long_plugin_output == NULL ? "NULL" : temp_host->long_plugin_output) << "\n"
    << "Perf Data:\n"
    << (temp_host->perf_data == NULL ? "NULL" : temp_host->perf_data);

  /* get the unprocessed return code */
  /* NOTE: for passive checks, this is the final/processed state */
  result = queued_check_result->return_code;

  /* adjust return code (active checks only) */
  if (queued_check_result->check_type == HOST_CHECK_ACTIVE) {

    /* if there was some error running the command, just skip it (this shouldn't be happening) */
    if (queued_check_result->exited_ok == FALSE) {

      logger(log_runtime_warning, basic)
        << "Warning:  Check of host '" << temp_host->name
        << "' did not exit properly!";

      delete[] temp_host->plugin_output;
      delete[] temp_host->long_plugin_output;
      delete[] temp_host->perf_data;

      temp_host->plugin_output
        = my_strdup("(Host check did not exit properly)");
      temp_host->long_plugin_output = NULL;
      temp_host->perf_data = NULL;

      result = STATE_CRITICAL;
    }

    /* make sure the return code is within bounds */
    else if (queued_check_result->return_code < 0
             || queued_check_result->return_code > 3) {

      logger(log_runtime_warning, basic)
        << "Warning: return (code of " << queued_check_result->return_code
        << " for check of host '" << temp_host->name << "' was out of bounds."
        << ((queued_check_result->return_code == 126
             || queued_check_result->return_code == 127)
            ? " Make sure the plugin you're trying to run actually exists." : "");

      std::ostringstream oss;
      oss << "(Return code of "
	  << queued_check_result->return_code
	  << " is out of bounds"
	  << ((queued_check_result->return_code == 126
	       || queued_check_result->return_code == 127)
	      ? " - plugin may be missing" : "") << ")";

      delete[] temp_host->plugin_output;
      delete[] temp_host->long_plugin_output;
      delete[] temp_host->perf_data;

      temp_host->plugin_output = my_strdup(oss.str().c_str());
      temp_host->long_plugin_output = NULL;
      temp_host->perf_data = NULL;

      result = STATE_CRITICAL;
    }

    /* a NULL host check command means we should assume the host is UP */
    if (temp_host->host_check_command == NULL) {
      delete[] temp_host->plugin_output;
      temp_host->plugin_output = my_strdup("(Host assumed to be UP)");
      result = STATE_OK;
    }
  }

  /* translate return code to basic UP/DOWN state - the DOWN/UNREACHABLE state determination is made later */
  /* NOTE: only do this for active checks - passive check results already have the final state */
  if (queued_check_result->check_type == HOST_CHECK_ACTIVE) {

    /* if we're not doing aggressive host checking, let WARNING states indicate the host is up (fake the result to be STATE_OK) */
    if (config->use_aggressive_host_checking() == false
        && result == STATE_WARNING)
      result = STATE_OK;

    /* OK states means the host is UP */
    if (result == STATE_OK)
      result = HOST_UP;

    /* any problem state indicates the host is not UP */
    else
      result = HOST_DOWN;
  }

  /******************* PROCESS THE CHECK RESULTS ******************/

  /* process the host check result */
  process_host_check_result_3x(
    temp_host,
    result,
    old_plugin_output,
    CHECK_OPTION_NONE,
    reschedule_check,
    TRUE,
    config->cached_host_check_horizon());

  /* free memory */
  delete[] old_plugin_output;

  logger(dbg_checks, more)
    << "** Async check result for host '" << temp_host->name
    << "' handled: new state=" << temp_host->current_state;

  /* high resolution start time for event broker */
  start_time_hires = queued_check_result->start_time;

  /* high resolution end time for event broker */
  gettimeofday(&end_time_hires, NULL);

  /* send data to event broker */
  broker_host_check(
    NEBTYPE_HOSTCHECK_PROCESSED,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    temp_host,
    temp_host->check_type,
    temp_host->current_state,
    temp_host->state_type,
    start_time_hires,
    end_time_hires,
    temp_host->host_check_command,
    temp_host->latency,
    temp_host->execution_time,
    config->host_check_timeout(),
    queued_check_result->early_timeout,
    queued_check_result->return_code,
    NULL,
    temp_host->plugin_output,
    temp_host->long_plugin_output,
    temp_host->perf_data,
    NULL);
  return (OK);
}

/* processes the result of a synchronous or asynchronous host check */
int process_host_check_result_3x(
      host* hst,
      int new_state,
      char* old_plugin_output,
      int check_options,
      int reschedule_check,
      int use_cached_result,
      unsigned long check_timestamp_horizon) {
  hostsmember* temp_hostsmember = NULL;
  host* child_host = NULL;
  host* parent_host = NULL;
  host* master_host = NULL;
  host* temp_host = NULL;
  objectlist* check_hostlist = NULL;
  objectlist* hostlist_item = NULL;
  int parent_state = HOST_UP;
  time_t current_time = 0L;
  time_t next_check = 0L;
  time_t preferred_time = 0L;
  time_t next_valid_time = 0L;
  int run_async_check = TRUE;

  logger(dbg_functions, basic)
    << "process_host_check_result_3x()";

  logger(dbg_checks, more)
    << "HOST: " << hst->name
    << ", ATTEMPT=" << hst->current_attempt << "/" << hst->max_attempts
    << ", CHECK TYPE=" << (hst->check_type == HOST_CHECK_ACTIVE ? "ACTIVE" : "PASSIVE")
    << ", STATE TYPE=" << (hst->state_type == HARD_STATE ? "HARD" : "SOFT")
    << ", OLD STATE=" << hst->current_state
    << ", NEW STATE=" << new_state;

  /* get the current time */
  time(&current_time);

  /* default next check time */
  next_check
    = (unsigned long)(current_time +
                      (hst->check_interval
                       * config->interval_length()));

  /* we have to adjust current attempt # for passive checks, as it isn't done elsewhere */
  if (hst->check_type == HOST_CHECK_PASSIVE
      && config->passive_host_checks_are_soft() == true)
    adjust_host_check_attempt_3x(hst, FALSE);

  /* log passive checks - we need to do this here, as some my bypass external commands by getting dropped in checkresults dir */
  if (hst->check_type == HOST_CHECK_PASSIVE) {
    if (config->log_passive_checks() == true)
      logger(log_passive_check, basic)
        << "PASSIVE HOST CHECK: " << hst->name << ";"
        << new_state << ";" << hst->plugin_output;
  }
  /******* HOST WAS DOWN/UNREACHABLE INITIALLY *******/
  if (hst->current_state != HOST_UP) {
    logger(dbg_checks, more)
      << "Host was DOWN/UNREACHABLE.";

    /***** HOST IS NOW UP *****/
    /* the host just recovered! */
    if (new_state == HOST_UP) {

      /* set the current state */
      hst->current_state = HOST_UP;

      /* set the state type */
      /* set state type to HARD for passive checks and active checks that were previously in a HARD STATE */
      if (hst->state_type == HARD_STATE
          || (hst->check_type == HOST_CHECK_PASSIVE
              && config->passive_host_checks_are_soft() == false))
        hst->state_type = HARD_STATE;
      else
        hst->state_type = SOFT_STATE;

      logger(dbg_checks, more)
        << "Host experienced a "
        << (hst->state_type == HARD_STATE ? "HARD" : "SOFT")
        << " recovery (it's now UP).";

      /* reschedule the next check of the host at the normal interval */
      reschedule_check = TRUE;
      next_check
        = (unsigned long)(current_time
                          + (hst->check_interval
                             * config->interval_length()));

      /* propagate checks to immediate parents if they are not already UP */
      /* we do this because a parent host (or grandparent) may have recovered somewhere and we should catch the recovery as soon as possible */
      logger(dbg_checks, more)
        << "Propagating checks to parent host(s)...";

      for (temp_hostsmember = hst->parent_hosts;
           temp_hostsmember != NULL;
           temp_hostsmember = temp_hostsmember->next) {
        if ((parent_host = temp_hostsmember->host_ptr) == NULL)
          continue;
        if (parent_host->current_state != HOST_UP) {
          logger(dbg_checks, more)
            << "Check of parent host '" << parent_host->name << "' queued.";
          add_object_to_objectlist(&check_hostlist, (void*)parent_host);
        }
      }

      /* propagate checks to immediate children if they are not already UP */
      /* we do this because children may currently be UNREACHABLE, but may (as a result of this recovery) switch to UP or DOWN states */
      logger(dbg_checks, more)
        << "Propagating checks to child host(s)...";

      for (temp_hostsmember = hst->child_hosts;
           temp_hostsmember != NULL;
           temp_hostsmember = temp_hostsmember->next) {
        if ((child_host = temp_hostsmember->host_ptr) == NULL)
          continue;
        if (child_host->current_state != HOST_UP) {
          logger(dbg_checks, more)
            << "Check of child host '" << child_host->name << "' queued.";
          add_object_to_objectlist(&check_hostlist, (void*)child_host);
        }
      }
    }

    /***** HOST IS STILL DOWN/UNREACHABLE *****/
    /* we're still in a problem state... */
    else {

      logger(dbg_checks, more)
        << "Host is still DOWN/UNREACHABLE.";

      /* passive checks are treated as HARD states by default... */
      if (hst->check_type == HOST_CHECK_PASSIVE
          && config->passive_host_checks_are_soft() == false) {

        /* set the state type */
        hst->state_type = HARD_STATE;

        /* reset the current attempt */
        hst->current_attempt = 1;
      }

      /* active checks and passive checks (treated as SOFT states) */
      else {

        /* set the state type */
        /* we've maxed out on the retries */
        if (hst->current_attempt == hst->max_attempts)
          hst->state_type = HARD_STATE;
        /* the host was in a hard problem state before, so it still is now */
        else if (hst->current_attempt == 1)
          hst->state_type = HARD_STATE;
        /* the host is in a soft state and the check will be retried */
        else
          hst->state_type = SOFT_STATE;
      }

      /* make a determination of the host's state */
      /* translate host state between DOWN/UNREACHABLE (only for passive checks if enabled) */
      hst->current_state = new_state;
      if (hst->check_type == HOST_CHECK_ACTIVE
          || config->translate_passive_host_checks() == true)
        hst->current_state = determine_host_reachability(hst);

      /* reschedule the next check if the host state changed */
      if (hst->last_state != hst->current_state
          || hst->last_hard_state != hst->current_state) {

        reschedule_check = TRUE;

        /* schedule a re-check of the host at the retry interval because we can't determine its final state yet... */
        if (hst->state_type == SOFT_STATE)
          next_check
            = (unsigned long)(current_time
                              + (hst->retry_interval
                                 * config->interval_length()));

        /* host has maxed out on retries (or was previously in a hard problem state), so reschedule the next check at the normal interval */
        else
          next_check
            = (unsigned long)(current_time
                              + (hst->check_interval
                               * config->interval_length()));
      }
    }
  }

  /******* HOST WAS UP INITIALLY *******/
  else {
    logger(dbg_checks, more)
      << "Host was UP.";

    /***** HOST IS STILL UP *****/
    /* either the host never went down since last check */
    if (new_state == HOST_UP) {

      logger(dbg_checks, more)
        << "Host is still UP.";

      /* set the current state */
      hst->current_state = HOST_UP;

      /* set the state type */
      hst->state_type = HARD_STATE;

      /* reschedule the next check at the normal interval */
      if (reschedule_check == TRUE)
        next_check
          = (unsigned long)(current_time
                            + (hst->check_interval
                               * config->interval_length()));
    }
    /***** HOST IS NOW DOWN/UNREACHABLE *****/
    else {
      logger(dbg_checks, more)
        << "Host is now DOWN/UNREACHABLE.";

      /***** SPECIAL CASE FOR HOSTS WITH MAX_ATTEMPTS==1 *****/
      if (hst->max_attempts == 1) {

        logger(dbg_checks, more)
          << "Max attempts = 1!.";

        /* set the state type */
        hst->state_type = HARD_STATE;

        /* host has maxed out on retries, so reschedule the next check at the normal interval */
        reschedule_check = TRUE;
        next_check
          = (unsigned long)(current_time
                            + (hst->check_interval
                               * config->interval_length()));

        /* we need to run SYNCHRONOUS checks of all parent hosts to accurately determine the state of this host */
        /* this is extremely inefficient (reminiscent of Nagios 2.x logic), but there's no other good way around it */
        /* check all parent hosts to see if we're DOWN or UNREACHABLE */
        /* only do this for ACTIVE checks, as PASSIVE checks contain a pre-determined state */
        if (hst->check_type == HOST_CHECK_ACTIVE) {

          logger(dbg_checks, more)
            << "** WARNING: Max attempts = 1, so we have to run serial "
            "checks of all parent hosts!";

          for (temp_hostsmember = hst->parent_hosts;
               temp_hostsmember != NULL;
               temp_hostsmember = temp_hostsmember->next) {

            if ((parent_host = temp_hostsmember->host_ptr) == NULL)
              continue;

            logger(dbg_checks, more)
              << "Running serial check parent host '"
              << parent_host->name << "'...";

            /* run an immediate check of the parent host */
            run_sync_host_check_3x(
              parent_host, &parent_state,
              check_options, use_cached_result,
              check_timestamp_horizon);

            /* bail out as soon as we find one parent host that is UP */
            if (parent_state == HOST_UP) {
              logger(dbg_checks, more)
                << "Parent host is UP, so this one is DOWN.";

              /* set the current state */
              hst->current_state = HOST_DOWN;
              break;
            }
          }

          if (temp_hostsmember == NULL) {
            /* host has no parents, so its up */
            if (hst->parent_hosts == NULL) {
              logger(dbg_checks, more)
                << "Host has no parents, so it's DOWN.";
              hst->current_state = HOST_DOWN;
            }
            else {
              /* no parents were up, so this host is UNREACHABLE */
              logger(dbg_checks, more)
                << "No parents were UP, so this host is UNREACHABLE.";
              hst->current_state = HOST_UNREACHABLE;
            }
          }
        }

        /* set the host state for passive checks */
        else {
          /* set the state */
          hst->current_state = new_state;

          /* translate host state between DOWN/UNREACHABLE for passive checks (if enabled) */
          /* make a determination of the host's state */
          if (config->translate_passive_host_checks() == true)
            hst->current_state = determine_host_reachability(hst);

        }

        /* propagate checks to immediate children if they are not UNREACHABLE */
        /* we do this because we may now be blocking the route to child hosts */
        logger(dbg_checks, more)
          << "Propagating check to immediate non-UNREACHABLE child hosts...";

        for (temp_hostsmember = hst->child_hosts;
             temp_hostsmember != NULL;
             temp_hostsmember = temp_hostsmember->next) {
          if ((child_host = temp_hostsmember->host_ptr) == NULL)
            continue;
          if (child_host->current_state != HOST_UNREACHABLE) {
            logger(dbg_checks, more)
              << "Check of child host '"
              << child_host->name << "' queued.";
            add_object_to_objectlist(
              &check_hostlist,
              (void*)child_host);
          }
        }
      }

      /***** MAX ATTEMPTS > 1 *****/
      else {

        /* active and (in some cases) passive check results are treated as SOFT states */
        if (hst->check_type == HOST_CHECK_ACTIVE
            || config->passive_host_checks_are_soft() == true) {

          /* set the state type */
          hst->state_type = SOFT_STATE;
        }

        /* by default, passive check results are treated as HARD states */
        else {

          /* set the state type */
          hst->state_type = HARD_STATE;

          /* reset the current attempt */
          hst->current_attempt = 1;
        }

        /* make a (in some cases) preliminary determination of the host's state */
        /* translate host state between DOWN/UNREACHABLE (for passive checks only if enabled) */
        hst->current_state = new_state;
        if (hst->check_type == HOST_CHECK_ACTIVE
            || config->translate_passive_host_checks() == true)
          hst->current_state = determine_host_reachability(hst);

        /* reschedule a check of the host */
        reschedule_check = TRUE;

        /* schedule a re-check of the host at the retry interval because we can't determine its final state yet... */
        if (hst->check_type == HOST_CHECK_ACTIVE
            || config->passive_host_checks_are_soft() == true)
          next_check
            = (unsigned long)(current_time
                              + (hst->retry_interval
                                 * config->interval_length()));

        /* schedule a re-check of the host at the normal interval */
        else
          next_check
            = (unsigned long)(current_time
                              + (hst->check_interval
                                 * config->interval_length()));

        /* propagate checks to immediate parents if they are UP */
        /* we do this because a parent host (or grandparent) may have gone down and blocked our route */
        /* checking the parents ASAP will allow us to better determine the final state (DOWN/UNREACHABLE) of this host later */
        logger(dbg_checks, more)
          << "Propagating checks to immediate parent hosts that "
          "are UP...";

        for (temp_hostsmember = hst->parent_hosts;
             temp_hostsmember != NULL;
             temp_hostsmember = temp_hostsmember->next) {
          if ((parent_host = temp_hostsmember->host_ptr) == NULL)
            continue;
          if (parent_host->current_state == HOST_UP) {
            add_object_to_objectlist(
              &check_hostlist,
              (void*)parent_host);
            logger(dbg_checks, more)
              << "Check of host '" << parent_host->name << "' queued.";
          }
        }

        /* propagate checks to immediate children if they are not UNREACHABLE */
        /* we do this because we may now be blocking the route to child hosts */
        logger(dbg_checks, more)
          << "Propagating checks to immediate non-UNREACHABLE "
          "child hosts...";

        for (temp_hostsmember = hst->child_hosts;
             temp_hostsmember != NULL;
             temp_hostsmember = temp_hostsmember->next) {
          if ((child_host = temp_hostsmember->host_ptr) == NULL)
            continue;
          if (child_host->current_state != HOST_UNREACHABLE) {
            logger(dbg_checks, more)
              << "Check of child host '"
              << child_host->name << "' queued.";
            add_object_to_objectlist(
              &check_hostlist,
              (void*)child_host);
          }
        }

        /* check dependencies on second to last host check */
        if (config->enable_predictive_host_dependency_checks() == true
	    && hst->current_attempt == (hst->max_attempts - 1)) {

          /* propagate checks to hosts that THIS ONE depends on for notifications AND execution */
          /* we do to help ensure that the dependency checks are accurate before it comes time to notify */
          logger(dbg_checks, more)
            << "Propagating predictive dependency checks to hosts this "
            "one depends on...";

          std::string id(hst->name);
          umultimap<std::string, shared_ptr<hostdependency> > const&
            dependencies(state::instance().hostdependencies());
          for (umultimap<std::string, shared_ptr<hostdependency> >::const_iterator
                 it(dependencies.find(id)), end(dependencies.end());
                 it != end && it->first == id;
                 ++it) {
            hostdependency* temp_dependency(&*it->second);
            if (temp_dependency->dependent_host_ptr == hst
                && temp_dependency->master_host_ptr != NULL) {
              master_host = (host*)temp_dependency->master_host_ptr;
              logger(dbg_checks, more)
                << "Check of host '"
                << master_host->name << "' queued.";
              add_object_to_objectlist(
                &check_hostlist,
                (void*)master_host);
            }
          }
        }
      }
    }
  }

  logger(dbg_checks, more)
    << "Pre-handle_host_state() Host: " << hst->name
    << ", Attempt=" << hst->current_attempt << "/" << hst->max_attempts
    << ", Type=" << (hst->state_type == HARD_STATE ? "HARD" : "SOFT")
    << ", Final State=" << hst->current_state;

  /* handle the host state */
  handle_host_state(hst);

  logger(dbg_checks, more)
    << "Post-handle_host_state() Host: " << hst->name
    << ", Attempt=" << hst->current_attempt << "/" << hst->max_attempts
    << ", Type=" << (hst->state_type == HARD_STATE ? "HARD" : "SOFT")
    << ", Final State=" << hst->current_state;

  /******************** POST-PROCESSING STUFF *********************/

  /* if the plugin output differs from previous check and no state change, log the current state/output if state stalking is enabled */
  if (hst->last_state == hst->current_state
      && compare_strings(old_plugin_output, hst->plugin_output)) {

    if (hst->current_state == HOST_UP && hst->stalk_on_up == TRUE)
      log_host_event(hst);

    else if (hst->current_state == HOST_DOWN
             && hst->stalk_on_down == TRUE)
      log_host_event(hst);

    else if (hst->current_state == HOST_UNREACHABLE
             && hst->stalk_on_unreachable == TRUE)
      log_host_event(hst);
  }

  /* check to see if the associated host is flapping */
  check_for_host_flapping(hst, TRUE, TRUE, TRUE);

  /* reschedule the next check of the host (usually ONLY for scheduled, active checks, unless overridden above) */
  if (reschedule_check == TRUE) {
    logger(dbg_checks, more)
      << "Rescheduling next check of host at " << my_ctime(&next_check);

    /* default is to reschedule host check unless a test below fails... */
    hst->should_be_scheduled = TRUE;

    /* get the new current time */
    time(&current_time);

    /* make sure we don't get ourselves into too much trouble... */
    if (current_time > next_check)
      hst->next_check = current_time;
    else
      hst->next_check = next_check;

    /* make sure we rescheduled the next service check at a valid time */
    preferred_time = hst->next_check;
    get_next_valid_time(
      preferred_time,
      &next_valid_time,
      hst->check_period_ptr);
    hst->next_check = next_valid_time;

    /* hosts with non-recurring intervals do not get rescheduled if we're in a HARD or UP state */
    if (hst->check_interval == 0
        && (hst->state_type == HARD_STATE
            || hst->current_state == HOST_UP))
      hst->should_be_scheduled = FALSE;

    /* host with active checks disabled do not get rescheduled */
    if (hst->checks_enabled == FALSE)
      hst->should_be_scheduled = FALSE;

    /* schedule a non-forced check if we can */
    if (hst->should_be_scheduled == TRUE) {
      schedule_host_check(hst, hst->next_check, CHECK_OPTION_NONE);
    }
  }

  /* update host status - for both active (scheduled) and passive (non-scheduled) hosts */
  update_host_status(hst, FALSE);

  /* run async checks of all hosts we added above */
  /* don't run a check if one is already executing or we can get by with a cached state */
  for (hostlist_item = check_hostlist;
       hostlist_item != NULL;
       hostlist_item = hostlist_item->next) {
    run_async_check = TRUE;
    temp_host = (host*)hostlist_item->object_ptr;

    logger(dbg_checks, most)
      << "ASYNC CHECK OF HOST: " << temp_host->name
      << ", CURRENTTIME: " << current_time
      << ", LASTHOSTCHECK: " << temp_host->last_check
      << ", CACHEDTIMEHORIZON: " << check_timestamp_horizon
      << ", USECACHEDRESULT: " << use_cached_result
      << ", ISEXECUTING: " << temp_host->is_executing;

    if (use_cached_result == TRUE
        && (static_cast<unsigned long>(current_time - temp_host->last_check) <= check_timestamp_horizon))
      run_async_check = FALSE;
    if (temp_host->is_executing == TRUE)
      run_async_check = FALSE;
    if (run_async_check == TRUE)
      run_async_host_check_3x(
        temp_host,
        CHECK_OPTION_NONE,
        0.0,
        FALSE,
        FALSE,
        NULL,
        NULL);
  }
  free_objectlist(&check_hostlist);
  return (OK);
}

/* checks viability of performing a host check */
int check_host_check_viability_3x(
      host* hst,
      int check_options,
      int* time_is_valid,
      time_t* new_time) {
  int result = OK;
  int perform_check = TRUE;
  time_t current_time = 0L;
  time_t preferred_time = 0L;
  int check_interval = 0;

  logger(dbg_functions, basic)
    << "check_host_check_viability_3x()";

  /* make sure we have a host */
  if (hst == NULL)
    return (ERROR);

  /* get the check interval to use if we need to reschedule the check */
  if (hst->state_type == SOFT_STATE && hst->current_state != HOST_UP)
    check_interval
      = static_cast<int>(hst->retry_interval * config->interval_length());
  else
    check_interval
      = static_cast<int>(hst->check_interval * config->interval_length());

  /* make sure check interval is positive - otherwise use 5 minutes out for next check */
  if (check_interval <= 0)
    check_interval = 300;

  /* get the current time */
  time(&current_time);

  /* initialize the next preferred check time */
  preferred_time = current_time;

  /* can we check the host right now? */
  if (!(check_options & CHECK_OPTION_FORCE_EXECUTION)) {

    /* if checks of the host are currently disabled... */
    if (hst->checks_enabled == FALSE) {
      preferred_time = current_time + check_interval;
      perform_check = FALSE;
    }
    /* make sure this is a valid time to check the host */
    if (check_time_against_period(
          static_cast<unsigned long>(current_time),
          hst->check_period_ptr) == ERROR) {
      preferred_time = current_time;
      if (time_is_valid)
	*time_is_valid = FALSE;
      perform_check = FALSE;
    }

    /* check host dependencies for execution */
    if (check_host_dependencies(
          hst,
          EXECUTION_DEPENDENCY) == DEPENDENCIES_FAILED) {
      preferred_time = current_time + check_interval;
      perform_check = FALSE;
    }
  }

  /* pass back the next viable check time */
  if (new_time)
    *new_time = preferred_time;

  result = (perform_check == TRUE) ? OK : ERROR;
  return (result);
}

/* adjusts current host check attempt before a new check is performed */
int adjust_host_check_attempt_3x(host* hst, int is_active) {
  logger(dbg_functions, basic)
    << "adjust_host_check_attempt_3x()";

  if (hst == NULL)
    return (ERROR);

  logger(dbg_checks, most)
    << "Adjusting check attempt number for host '" << hst->name
    << "': current attempt=" << hst->current_attempt << "/"
    << hst->max_attempts << ", state=" << hst->current_state
    << ", state type=" << hst->state_type;

  /* if host is in a hard state, reset current attempt number */
  if (hst->state_type == HARD_STATE)
    hst->current_attempt = 1;

  /* if host is in a soft UP state, reset current attempt number (active checks only) */
  else if (is_active == TRUE && hst->state_type == SOFT_STATE
           && hst->current_state == HOST_UP)
    hst->current_attempt = 1;

  /* increment current attempt number */
  else if (hst->current_attempt < hst->max_attempts)
    hst->current_attempt++;

  logger(dbg_checks, most)
    << "New check attempt number = " << hst->current_attempt;
  return (OK);
}

/* determination of the host's state based on route availability*//* used only to determine difference between DOWN and UNREACHABLE states */
int determine_host_reachability(host* hst) {
  int state = HOST_DOWN;
  host* parent_host = NULL;
  hostsmember* temp_hostsmember = NULL;

  logger(dbg_functions, basic)
    << "determine_host_reachability()";

  if (hst == NULL)
    return (HOST_DOWN);

  logger(dbg_checks, most)
    << "Determining state of host '" << hst->name
    << "': current state=" << hst->current_state;

  /* host is UP - no translation needed */
  if (hst->current_state == HOST_UP) {
    state = HOST_UP;
    logger(dbg_checks, most)
      << "Host is UP, no state translation needed.";
  }

  /* host has no parents, so it is DOWN */
  else if (hst->parent_hosts == NULL) {
    state = HOST_DOWN;
    logger(dbg_checks, most)
      << "Host has no parents, so it is DOWN.";
  }

  /* check all parent hosts to see if we're DOWN or UNREACHABLE */
  else {

    for (temp_hostsmember = hst->parent_hosts;
	 temp_hostsmember != NULL;
         temp_hostsmember = temp_hostsmember->next) {

      if ((parent_host = temp_hostsmember->host_ptr) == NULL)
        continue;

      /* bail out as soon as we find one parent host that is UP */
      if (parent_host->current_state == HOST_UP) {
        /* set the current state */
        state = HOST_DOWN;
        logger(dbg_checks, most)
          << "At least one parent (" << parent_host->name
          << ") is up, so host is DOWN.";
        break;
      }
    }
    /* no parents were up, so this host is UNREACHABLE */
    if (temp_hostsmember == NULL) {
      state = HOST_UNREACHABLE;
      logger(dbg_checks, most)
        << "No parents were up, so host is UNREACHABLE.";
    }
  }

  return (state);
}

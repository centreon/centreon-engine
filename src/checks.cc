/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011-2016 Centreon
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
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/checks/viability_failure.hh"
#include "com/centreon/engine/comment.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/downtimes/downtime.hh"
#include "com/centreon/engine/sehandlers.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/timezone_locker.hh"
#include "com/centreon/engine/utils.hh"

#define MAX_CMD_ARGS 4096

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::downtimes;
using namespace com::centreon::engine::events;
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
      << "Error: " << e.what();
    return ERROR;
  }
  return OK;
}

/******************************************************************/
/****************** SERVICE MONITORING FUNCTIONS ******************/
/******************************************************************/

/* handles asynchronous service check results */
/* checks service dependencies */
unsigned int check_service_dependencies(
               com::centreon::engine::service* svc,
               int dependency_type) {
  com::centreon::engine::service* temp_service = NULL;
  int state = STATE_OK;
  time_t current_time = 0L;

  logger(dbg_functions, basic)
    << "check_service_dependencies()";

  std::pair<std::string, std::string>
    id(svc->get_hostname(), svc->get_description());
  servicedependency_mmap const& dependencies(
    state::instance().servicedependencies());
  for (servicedependency_mmap::const_iterator
         it(dependencies.find(id)), end(dependencies.end());
       it != end && it->first == id;
       ++it) {
    servicedependency* temp_dependency(&*it->second);

    /* only check dependencies of the desired type (notification or execution) */
    if (temp_dependency->get_dependency_type() != dependency_type)
      continue;

    /* find the service we depend on... */
    if ((temp_service = temp_dependency->master_service_ptr) == NULL)
      continue;

    /* skip this dependency if it has a timeperiod and the current time isn't valid */
    time(&current_time);
    if (check_time_against_period(
             current_time,
             temp_dependency->dependency_period_ptr) == ERROR)
      return DEPENDENCIES_OK;

    /* get the status to use (use last hard state if its currently in a soft state) */
    if (temp_service->state_type == SOFT_STATE
        && !config->soft_state_dependencies())
      state = temp_service->last_hard_state;
    else
      state = temp_service->current_state;

    /* is the service we depend on in state that fails the dependency tests? */
    if (state == STATE_OK && temp_dependency->get_fail_on_ok())
      return DEPENDENCIES_FAILED;
    if (state == STATE_WARNING
        && temp_dependency->get_fail_on_warning())
      return DEPENDENCIES_FAILED;
    if (state == STATE_UNKNOWN
        && temp_dependency->get_fail_on_unknown())
      return DEPENDENCIES_FAILED;
    if (state == STATE_CRITICAL
        && temp_dependency->get_fail_on_critical())
      return DEPENDENCIES_FAILED;
    if ((state == STATE_OK && !temp_service->has_been_checked)
        && temp_dependency->get_fail_on_pending())
      return DEPENDENCIES_FAILED;

    /* immediate dependencies ok at this point - check parent dependencies if necessary */
    if (temp_dependency->get_inherits_parent()) {
      if (check_service_dependencies(
            temp_service,
            dependency_type) != DEPENDENCIES_OK)
        return DEPENDENCIES_FAILED;
    }
  }
  return DEPENDENCIES_OK;
}

/* check for services that never returned from a check... */
void check_for_orphaned_services() {
  com::centreon::engine::service* temp_service = NULL;
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
    if (!temp_service->is_executing)
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
        << temp_service->get_description() << "' on host '"
        << temp_service->get_hostname() << "' looks like it was orphaned "
        "(results never came back).  I'm scheduling an immediate check "
        "of the service...";

      logger(dbg_checks, more)
        << "Service '" << temp_service->get_description()
        << "' on host '" << temp_service->get_hostname()
        << "' was orphaned, so we're scheduling an immediate check...";

      /* decrement the number of running service checks */
      if (currently_running_service_checks > 0)
        currently_running_service_checks--;

      /* disable the executing flag */
      temp_service->is_executing = false;

      /* schedule an immediate check of the service */
      temp_service->schedule_check(current_time, CHECK_OPTION_ORPHAN_CHECK);
    }
  }
}

/* check freshness of service results */
void check_service_result_freshness() {
  com::centreon::engine::service* temp_service = NULL;
  time_t current_time = 0L;

  logger(dbg_functions, basic)
    << "check_service_result_freshness()";
  logger(dbg_checks, more)
    << "Checking the freshness of service check results...";

  /* bail out if we're not supposed to be checking freshness */
  if (!config->check_service_freshness()) {
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
    if (!temp_service->get_check_freshness())
      continue;

    /* skip services that are currently executing (problems here will be caught by orphaned service check) */
    if (temp_service->is_executing)
      continue;

    /* skip services that have both active and passive checks disabled */
    if (!temp_service->get_checks_enabled()
        && !temp_service->accept_passive_service_checks)
      continue;

    /* skip services that are already being freshened */
    if (temp_service->is_being_freshened)
      continue;

    // See if the time is right...
    {
      timezone_locker lock(temp_service->get_timezone());
      if (check_time_against_period(
            current_time,
            temp_service->check_period_ptr) == ERROR)
        continue ;
    }

    /* EXCEPTION */
    /* don't check freshness of services without regular check intervals if we're using auto-freshness threshold */
    if (temp_service->get_check_interval() == 0 &&
        temp_service->freshness_threshold == 0)
      continue;

    /* the results for the last check of this service are stale! */
    if (!is_service_result_fresh(
          temp_service, current_time,
          true)) {

      /* set the freshen flag */
      temp_service->is_being_freshened = true;

      /* schedule an immediate forced check of the service */
      temp_service->schedule_check(
          current_time,
          CHECK_OPTION_FORCE_EXECUTION | CHECK_OPTION_FRESHNESS_CHECK);
    }
  }
  return;
}

/* tests whether or not a service's check results are fresh */
int is_service_result_fresh(
      com::centreon::engine::service* temp_service,
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
    << "Checking freshness of service '" << temp_service->get_description()
    << "' on host '" << temp_service->get_hostname() << "'...";

  /* use user-supplied freshness threshold or auto-calculate a freshness threshold to use? */
  if (temp_service->freshness_threshold == 0) {
    if (temp_service->state_type == HARD_STATE ||
        temp_service->current_state == STATE_OK)
      freshness_threshold = static_cast<int>(
          (temp_service->get_check_interval() * config->interval_length()) +
          temp_service->latency + config->additional_freshness_latency());
    else
      freshness_threshold = static_cast<int>(
          temp_service->get_retry_interval() * config->interval_length() +
          temp_service->latency + config->additional_freshness_latency());
  } else
    freshness_threshold = temp_service->freshness_threshold;

  logger(dbg_checks, most)
    << "Freshness thresholds: service="
    << temp_service->freshness_threshold
    << ", use=" << freshness_threshold;

  /* calculate expiration time */
  /* CHANGED 11/10/05 EG - program start is only used in expiration time calculation if > last check AND active checks are enabled, so active checks can become stale immediately upon program startup */
  /* CHANGED 02/25/06 SG - passive checks also become stale, so remove dependence on active check logic */
  if (!temp_service->has_been_checked)
    expiration_time = (time_t)(event_start + freshness_threshold);
  /* CHANGED 06/19/07 EG - Per Ton's suggestion (and user requests), only use program start time over last check if no specific threshold has been set by user.  Otheriwse use it.  Problems can occur if Engine is restarted more frequently that freshness threshold intervals (services never go stale). */
  /* CHANGED 10/07/07 EG - Only match next condition for services that have active checks enabled... */
  /* CHANGED 10/07/07 EG - Added max_service_check_spread to expiration time as suggested by Altinity */
  else if (temp_service->get_checks_enabled()
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
    if (log_this)
      logger(log_runtime_warning, basic)
        << "Warning: The results of service '" << temp_service->get_description()
        << "' on host '" << temp_service->get_hostname() << "' are stale by "
        << days << "d " << hours << "h " << minutes << "m " << seconds
        << "s (threshold=" << tdays << "d " << thours << "h " << tminutes
        << "m " << tseconds << "s).  I'm forcing an immediate check "
        "of the service.";

    logger(dbg_checks, more)
      << "Check results for service '" << temp_service->get_description()
      << "' on host '" << temp_service->get_hostname() << "' are stale by "
      << days << "d " << hours << "h " << minutes << "m " << seconds
      << "s (threshold=" << tdays << "d " << thours << "h " << tminutes
      << "m " << tseconds << "s).  Forcing an immediate check of "
      "the service...";

    return false;
  }

  logger(dbg_checks, more)
    << "Check results for service '" << temp_service->get_description()
    << "' on host '" << temp_service->get_hostname() << "' are fresh.";

  return true;
}

/******************************************************************/
/*************** COMMON ROUTE/HOST CHECK FUNCTIONS ****************/
/******************************************************************/

/* execute an on-demand check  */
int perform_on_demand_host_check(
      com::centreon::engine::host* hst,
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
  return OK;
}

/* execute a scheduled host check using either the 2.x or 3.x logic */
int perform_scheduled_host_check(
     com::centreon::engine::host* hst,
     int check_options,
     double latency) {
  logger(dbg_functions, basic)
    << "perform_scheduled_host_check()";
  hst->run_scheduled_check(check_options, latency);
  return OK;
}

/* checks host dependencies */
unsigned int check_host_dependencies(com::centreon::engine::host* hst, int dependency_type) {
  host* temp_host = NULL;
  int state = HOST_UP;
  time_t current_time = 0L;

  logger(dbg_functions, basic)
    << "check_host_dependencies()";

  std::string id(hst->get_name());
  umultimap<std::string, std::shared_ptr<com::centreon::engine::hostdependency> > const&
    dependencies(state::instance().hostdependencies());

  /* check all dependencies... */
  for (umultimap<std::string, std::shared_ptr<com::centreon::engine::hostdependency> >::const_iterator
         it(dependencies.find(id)), end(dependencies.end());
       it != end && it->first == id;
       ++it) {
         hostdependency* temp_dependency(&*it->second);

    /* only check dependencies of the desired type (notification or execution) */
    if (temp_dependency->get_dependency_type() != dependency_type)
      continue;

    /* find the host we depend on... */
    if ((temp_host = temp_dependency->master_host_ptr) == NULL)
      continue;

    /* skip this dependency if it has a timeperiod and the current time isn't valid */
    time(&current_time);
    if (!temp_dependency->get_dependency_period().empty()
        && check_time_against_period(
             current_time,
             temp_dependency->dependency_period_ptr) == ERROR)
      return DEPENDENCIES_OK;

    /* get the status to use (use last hard state if its currently in a soft state) */
    if (temp_host->get_state_type() == SOFT_STATE
        && !config->soft_state_dependencies())
      state = temp_host->get_last_hard_state();
    else
      state = temp_host->get_current_state();

    /* is the host we depend on in state that fails the dependency tests? */
    if (state == HOST_UP && temp_dependency->get_fail_on_up())
      return DEPENDENCIES_FAILED;
    if (state == HOST_DOWN && temp_dependency->get_fail_on_down())
      return DEPENDENCIES_FAILED;
    if (state == HOST_UNREACHABLE
        && temp_dependency->get_fail_on_unreachable())
      return DEPENDENCIES_FAILED;
    if ((state == HOST_UP && !temp_host->get_has_been_checked())
        && temp_dependency->get_fail_on_pending())
      return DEPENDENCIES_FAILED;

    /* immediate dependencies ok at this point - check parent dependencies if necessary */
    if (temp_dependency->get_inherits_parent()) {
      if (check_host_dependencies(
            temp_host,
            dependency_type) != DEPENDENCIES_OK)
        return DEPENDENCIES_FAILED;
    }
  }
  return DEPENDENCIES_OK;
}

/* check for hosts that never returned from a check... */
void check_for_orphaned_hosts() {
  time_t current_time = 0L;
  time_t expected_time = 0L;

  logger(dbg_functions, basic)
    << "check_for_orphaned_hosts()";

  /* get the current time */
  time(&current_time);

  /* check all hosts... */
  for (host_map::iterator
         it(host::hosts.begin()),
         end(host::hosts.end());
       it != end;
       ++it) {

    /* skip hosts that don't have a set check interval (on-demand checks are missed by the orphan logic) */
    if (it->second->get_next_check() == (time_t)0L)
      continue;

    /* skip hosts that are not currently executing */
    if (!it->second->get_is_executing())
      continue;

    /* determine the time at which the check results should have come in (allow 10 minutes slack time) */
    expected_time
      = (time_t)(it->second->get_next_check() + it->second->get_latency()
                 + config->host_check_timeout()
                 + config->check_reaper_interval() + 600);

    /* this host was supposed to have executed a while ago, but for some reason the results haven't come back in... */
    if (expected_time < current_time) {

      /* log a warning */
      logger(log_runtime_warning, basic)
        << "Warning: The check of host '" << it->second->get_name()
        << "' looks like it was orphaned (results never came back).  "
        "I'm scheduling an immediate check of the host...";

      logger(dbg_checks, more)
        << "Host '" << it->second->get_name()
        << "' was orphaned, so we're scheduling an immediate check...";

      /* decrement the number of running host checks */
      if (currently_running_host_checks > 0)
        currently_running_host_checks--;

      /* disable the executing flag */
      it->second->set_is_executing(false);

      /* schedule an immediate check of the host */
      it->second->schedule_check(current_time, CHECK_OPTION_ORPHAN_CHECK);
    }
  }
  return;
}

/* check freshness of host results */
void check_host_result_freshness() {
  time_t current_time = 0L;

  logger(dbg_functions, basic)
    << "check_host_result_freshness()";
  logger(dbg_checks, most)
    << "Attempting to check the freshness of host check results...";

  /* bail out if we're not supposed to be checking freshness */
  if (!config->check_host_freshness()) {
    logger(dbg_checks, most)
      << "Host freshness checking is disabled.";
    return;
  }

  /* get the current time */
  time(&current_time);

  /* check all hosts... */
  for (host_map::iterator
         it(host::hosts.begin()),
         end(host::hosts.end());
       it != end;
       ++it) {

      /* skip hosts we shouldn't be checking for freshness */
    if (!it->second->get_check_freshness())
      continue;

    /* skip hosts that have both active and passive checks disabled */
    if (!it->second->get_checks_enabled()
        && !it->second->get_accept_passive_host_checks())
      continue;

    /* skip hosts that are currently executing (problems here will be caught by orphaned host check) */
    if (it->second->get_is_executing())
      continue;

    /* skip hosts that are already being freshened */
    if (it->second->get_is_being_freshened())
      continue;

    // See if the time is right...
    {
      timezone_locker lock(it->second->get_timezone());
      if (check_time_against_period(
            current_time,
            it->second->check_period_ptr) == ERROR)
        continue ;
    }

    /* the results for the last check of this host are stale */
    if (!is_host_result_fresh(it->second.get(), current_time, true)) {

      /* set the freshen flag */
      it->second->set_is_being_freshened(true);

      /* schedule an immediate forced check of the host */
      it->second->schedule_check(
          current_time,
          CHECK_OPTION_FORCE_EXECUTION | CHECK_OPTION_FRESHNESS_CHECK);
    }
  }
  return;
}

/* checks to see if a hosts's check results are fresh */
int is_host_result_fresh(
      com::centreon::engine::host* temp_host,
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
    << "Checking freshness of host '" << temp_host->get_name() << "'...";

  /* use user-supplied freshness threshold or auto-calculate a freshness threshold to use? */
  if (temp_host->get_freshness_threshold() == 0) {
    double interval;
    if ((HARD_STATE == temp_host->get_state_type())
        || (STATE_OK == temp_host->get_current_state()))
      interval = temp_host->get_check_interval();
    else
      interval = temp_host->get_retry_interval();
    freshness_threshold
      = static_cast<int>((interval * config->interval_length())
                         + temp_host->get_latency()
                         + config->additional_freshness_latency());
  }
  else
    freshness_threshold = temp_host->get_freshness_threshold();

  logger(dbg_checks, most)
    << "Freshness thresholds: host=" << temp_host->get_freshness_threshold()
    << ", use=" << freshness_threshold;

  /* calculate expiration time */
  /* CHANGED 11/10/05 EG - program start is only used in expiration time calculation if > last check AND active checks are enabled, so active checks can become stale immediately upon program startup */
  if (!temp_host->get_has_been_checked())
    expiration_time = (time_t)(event_start + freshness_threshold);
  /* CHANGED 06/19/07 EG - Per Ton's suggestion (and user requests), only use program start time over last check if no specific threshold has been set by user.  Otheriwse use it.  Problems can occur if Engine is restarted more frequently that freshness threshold intervals (hosts never go stale). */
  /* CHANGED 10/07/07 EG - Added max_host_check_spread to expiration time as suggested by Altinity */
  else if (temp_host->get_checks_enabled()
           && event_start > temp_host->get_last_check()
           && temp_host->get_freshness_threshold() == 0)
    expiration_time
      = (time_t)(event_start + freshness_threshold
                 + (config->max_host_check_spread()
                    * config->interval_length()));
  else
    expiration_time
      = (time_t)(temp_host->get_last_check() + freshness_threshold);

  logger(dbg_checks, most)
    << "HBC: " << temp_host->get_has_been_checked()
    << ", PS: " << program_start
    << ", ES: " << event_start
    << ", LC: " << temp_host->get_last_check()
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
    if (log_this)
      logger(log_runtime_warning, basic)
        << "Warning: The results of host '" << temp_host->get_name()
        << "' are stale by " << days << "d " << hours << "h "
        << minutes << "m " << seconds << "s (threshold="
        << tdays << "d " << thours << "h " << tminutes << "m "
        << tseconds << "s).  I'm forcing an immediate check of"
        " the host.";

    logger(dbg_checks, more)
      << "Check results for host '" << temp_host->get_name()
      << "' are stale by " << days << "d " << hours << "h " << minutes
      << "m " << seconds << "s (threshold=" << tdays << "d " << thours
      << "h " << tminutes << "m " << tseconds << "s).  "
      "Forcing an immediate check of the host...";

    return false;
  }
  else
    logger(dbg_checks, more)
      << "Check results for host '" << temp_host->get_name()
      << "' are fresh.";

  return true;
}

/******************************************************************/
/************* NAGIOS 3.X ROUTE/HOST CHECK FUNCTIONS **************/
/******************************************************************/

/*** ON-DEMAND HOST CHECKS USE THIS FUNCTION ***/
/* check to see if we can reach the host */
int perform_on_demand_host_check_3x(
      com::centreon::engine::host* hst,
      int* check_result_code,
      int check_options,
      int use_cached_result,
      unsigned long check_timestamp_horizon) {
  int result = OK;

  logger(dbg_functions, basic)
    << "perform_on_demand_host_check_3x()";

  /* make sure we have a host */
  if (hst == NULL)
    return ERROR;

  logger(dbg_checks, basic)
    << "** On-demand check for host '" << hst->get_name() << "'...";

  /* check the status of the host */
  result = run_sync_host_check_3x(
             hst,
             check_result_code,
             check_options,
             use_cached_result,
             check_timestamp_horizon);
  return result;
}

/* perform a synchronous check of a host *//* on-demand host checks will use this... */
int run_sync_host_check_3x(
      com::centreon::engine::host* hst,
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
  catch (checks::viability_failure const& e) {
    // Do not log viability failures.
    (void)e;
    return ERROR;
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: " << e.what();
    return ERROR;
  }
  return OK;
}

/* processes the result of a synchronous or asynchronous host check */
int process_host_check_result_3x(com::centreon::engine::host* hst,
                                 int new_state,
                                 char* old_plugin_output,
                                 int check_options,
                                 int reschedule_check,
                                 int use_cached_result,
                                 unsigned long check_timestamp_horizon) {
  com::centreon::engine::host* child_host = NULL;
  com::centreon::engine::host* parent_host = NULL;
  com::centreon::engine::host* master_host = NULL;
  com::centreon::engine::host* temp_host = NULL;
  objectlist* check_hostlist = NULL;
  objectlist* hostlist_item = NULL;
  int parent_state = HOST_UP;
  time_t current_time = 0L;
  time_t next_check = 0L;
  time_t preferred_time = 0L;
  time_t next_valid_time = 0L;
  int run_async_check = true;
  bool has_parent;

  logger(dbg_functions, basic) << "process_host_check_result_3x()";

  logger(dbg_checks, more)
      << "HOST: " << hst->get_name()
      << ", ATTEMPT=" << hst->get_current_attempt() << "/"
      << hst->get_max_attempts() << ", CHECK TYPE="
      << (hst->get_check_type() == check_active ? "ACTIVE" : "PASSIVE")
      << ", STATE TYPE="
      << (hst->get_state_type() == HARD_STATE ? "HARD" : "SOFT")
      << ", OLD STATE=" << hst->get_current_state()
      << ", NEW STATE=" << new_state;

  /* get the current time */
  time(&current_time);

  /* default next check time */
  next_check = (unsigned long)(current_time + (hst->get_check_interval() *
                                               config->interval_length()));

  /* we have to adjust current attempt # for passive checks, as it isn't done
   * elsewhere */
  if (hst->get_check_type() == check_passive &&
      config->passive_host_checks_are_soft())
    adjust_host_check_attempt_3x(hst, false);

  /* log passive checks - we need to do this here, as some my bypass external
   * commands by getting dropped in checkresults dir */
  if (hst->get_check_type() == check_passive) {
    if (config->log_passive_checks())
      logger(log_passive_check, basic)
          << "PASSIVE HOST CHECK: " << hst->get_name() << ";" << new_state
          << ";" << hst->get_plugin_output();
  }
  /******* HOST WAS DOWN/UNREACHABLE INITIALLY *******/
  if (hst->get_current_state() != HOST_UP) {
    logger(dbg_checks, more) << "Host was DOWN/UNREACHABLE.";

    /***** HOST IS NOW UP *****/
    /* the host just recovered! */
    if (new_state == HOST_UP) {
      /* set the current state */
      hst->set_current_state(HOST_UP);

      /* set the state type */
      /* set state type to HARD for passive checks and active checks that were
       * previously in a HARD STATE */
      if (hst->get_state_type() == HARD_STATE ||
          (hst->get_check_type() == check_passive &&
           !config->passive_host_checks_are_soft()))
        hst->set_state_type(HARD_STATE);
      else
        hst->set_state_type(SOFT_STATE);

      logger(dbg_checks, more)
          << "Host experienced a "
          << (hst->get_state_type() == HARD_STATE ? "HARD" : "SOFT")
          << " recovery (it's now UP).";

      /* reschedule the next check of the host at the normal interval */
      reschedule_check = true;
      next_check = (unsigned long)(current_time + (hst->get_check_interval() *
                                                   config->interval_length()));

      /* propagate checks to immediate parents if they are not already UP */
      /* we do this because a parent host (or grandparent) may have recovered
       * somewhere and we should catch the recovery as soon as possible */
      logger(dbg_checks, more) << "Propagating checks to parent host(s)...";

      for (host_map::iterator it(hst->parent_hosts.begin()),
           end(hst->parent_hosts.end());
           it != end; it++) {
        if (it->second == nullptr)
          continue;
        if (it->second->get_current_state() != HOST_UP) {
          logger(dbg_checks, more)
              << "Check of parent host '" << it->first << "' queued.";
          add_object_to_objectlist(&check_hostlist, (void*)it->second.get());
        }
      }

      /* propagate checks to immediate children if they are not already UP */
      /* we do this because children may currently be UNREACHABLE, but may (as a
       * result of this recovery) switch to UP or DOWN states */
      logger(dbg_checks, more) << "Propagating checks to child host(s)...";

      for (host_map::iterator it(hst->child_hosts.begin()),
           end(hst->child_hosts.end());
           it != end; it++) {
        if (it->second == nullptr)
          continue;
        if (it->second->get_current_state() != HOST_UP) {
          logger(dbg_checks, more)
              << "Check of child host '" << it->first << "' queued.";
          add_object_to_objectlist(&check_hostlist, (void*)it->second.get());
        }
      }
    }

    /***** HOST IS STILL DOWN/UNREACHABLE *****/
    /* we're still in a problem state... */
    else {
      logger(dbg_checks, more) << "Host is still DOWN/UNREACHABLE.";

      /* passive checks are treated as HARD states by default... */
      if (hst->get_check_type() == check_passive &&
          !config->passive_host_checks_are_soft()) {
        /* set the state type */
        hst->set_state_type(HARD_STATE);

        /* reset the current attempt */
        hst->set_current_attempt(1);
      }

      /* active checks and passive checks (treated as SOFT states) */
      else {
        /* set the state type */
        /* we've maxed out on the retries */
        if (hst->get_current_attempt() == hst->get_max_attempts())
          hst->set_state_type(HARD_STATE);
        /* the host was in a hard problem state before, so it still is now */
        else if (hst->get_current_attempt() == 1)
          hst->set_state_type(HARD_STATE);
        /* the host is in a soft state and the check will be retried */
        else
          hst->set_state_type(SOFT_STATE);
      }

      /* make a determination of the host's state */
      /* translate host state between DOWN/UNREACHABLE (only for passive checks
       * if enabled) */
      hst->set_current_state(new_state);
      if (hst->get_check_type() == check_active ||
          config->translate_passive_host_checks())
        hst->set_current_state(determine_host_reachability(hst));

      /* reschedule the next check if the host state changed */
      if (hst->get_last_state() != hst->get_current_state() ||
          hst->get_last_hard_state() != hst->get_current_state()) {
        reschedule_check = true;

        /* schedule a re-check of the host at the retry interval because we
         * can't determine its final state yet... */
        if (hst->get_state_type() == SOFT_STATE)
          next_check =
              (unsigned long)(current_time + (hst->get_retry_interval() *
                                              config->interval_length()));

        /* host has maxed out on retries (or was previously in a hard problem
         * state), so reschedule the next check at the normal interval */
        else
          next_check =
              (unsigned long)(current_time + (hst->get_check_interval() *
                                              config->interval_length()));
      }
    }
  }

  /******* HOST WAS UP INITIALLY *******/
  else {
    logger(dbg_checks, more) << "Host was UP.";

    /***** HOST IS STILL UP *****/
    /* either the host never went down since last check */
    if (new_state == HOST_UP) {
      logger(dbg_checks, more) << "Host is still UP.";

      /* set the current state */
      hst->set_current_state(HOST_UP);

      /* set the state type */
      hst->set_state_type(HARD_STATE);

      /* reschedule the next check at the normal interval */
      if (reschedule_check)
        next_check =
            (unsigned long)(current_time + (hst->get_check_interval() *
                                            config->interval_length()));
    }
    /***** HOST IS NOW DOWN/UNREACHABLE *****/
    else {
      logger(dbg_checks, more) << "Host is now DOWN/UNREACHABLE.";

      /***** SPECIAL CASE FOR HOSTS WITH MAX_ATTEMPTS==1 *****/
      if (hst->get_max_attempts() == 1) {
        logger(dbg_checks, more) << "Max attempts = 1!.";

        /* set the state type */
        hst->set_state_type(HARD_STATE);

        /* host has maxed out on retries, so reschedule the next check at the
         * normal interval */
        reschedule_check = true;
        next_check =
            (unsigned long)(current_time + (hst->get_check_interval() *
                                            config->interval_length()));

        /* we need to run SYNCHRONOUS checks of all parent hosts to accurately
         * determine the state of this host */
        /* this is extremely inefficient (reminiscent of Nagios 2.x logic), but
         * there's no other good way around it */
        /* check all parent hosts to see if we're DOWN or UNREACHABLE */
        /* only do this for ACTIVE checks, as PASSIVE checks contain a
         * pre-determined state */
        if (hst->get_check_type() == check_active) {
          has_parent = false;

          logger(dbg_checks, more)
              << "** WARNING: Max attempts = 1, so we have to run serial "
                 "checks of all parent hosts!";

          for (host_map::iterator it(hst->parent_hosts.begin()),
               end(hst->parent_hosts.end());
               it != end; it++) {
            if (it->second == nullptr)
              continue;

            has_parent = true;

            logger(dbg_checks, more)
                << "Running serial check parent host '" << it->first << "'...";

            /* run an immediate check of the parent host */
            run_sync_host_check_3x(it->second.get(), &parent_state,
                                   check_options, use_cached_result,
                                   check_timestamp_horizon);

            /* bail out as soon as we find one parent host that is UP */
            if (parent_state == HOST_UP) {
              logger(dbg_checks, more)
                  << "Parent host is UP, so this one is DOWN.";

              /* set the current state */
              hst->set_current_state(HOST_DOWN);
              break;
            }
          }

          if (!has_parent) {
            /* host has no parents, so its up */
            if (hst->parent_hosts.size() == 0) {
              logger(dbg_checks, more) << "Host has no parents, so it's DOWN.";
              hst->set_current_state(HOST_DOWN);
            } else {
              /* no parents were up, so this host is UNREACHABLE */
              logger(dbg_checks, more)
                  << "No parents were UP, so this host is UNREACHABLE.";
              hst->set_current_state(HOST_UNREACHABLE);
            }
          }
        }

        /* set the host state for passive checks */
        else {
          /* set the state */
          hst->set_current_state(new_state);

          /* translate host state between DOWN/UNREACHABLE for passive checks
           * (if enabled) */
          /* make a determination of the host's state */
          if (config->translate_passive_host_checks())
            hst->set_current_state(determine_host_reachability(hst));
        }

        /* propagate checks to immediate children if they are not UNREACHABLE */
        /* we do this because we may now be blocking the route to child hosts */
        logger(dbg_checks, more)
            << "Propagating check to immediate non-UNREACHABLE child hosts...";

        for (host_map::iterator it(hst->child_hosts.begin()),
             end(hst->child_hosts.end());
             it != end; it++) {
          if (it->second == nullptr)
            continue;
          if (it->second->get_current_state() != HOST_UNREACHABLE) {
            logger(dbg_checks, more)
                << "Check of child host '" << it->first << "' queued.";
            add_object_to_objectlist(&check_hostlist, it->second.get());
          }
        }
      }

      /***** MAX ATTEMPTS > 1 *****/
      else {
        /* active and (in some cases) passive check results are treated as SOFT
         * states */
        if (hst->get_check_type() == check_active ||
            config->passive_host_checks_are_soft()) {
          /* set the state type */
          hst->set_state_type(SOFT_STATE);
        }

        /* by default, passive check results are treated as HARD states */
        else {
          /* set the state type */
          hst->set_state_type(HARD_STATE);

          /* reset the current attempt */
          hst->set_current_attempt(1);
        }

        /* make a (in some cases) preliminary determination of the host's state
         */
        /* translate host state between DOWN/UNREACHABLE (for passive checks
         * only if enabled) */
        hst->set_current_state(new_state);
        if (hst->get_check_type() == check_active ||
            config->translate_passive_host_checks())
          hst->set_current_state(determine_host_reachability(hst));

        /* reschedule a check of the host */
        reschedule_check = true;

        /* schedule a re-check of the host at the retry interval because we
         * can't determine its final state yet... */
        if (hst->get_check_type() == check_active ||
            config->passive_host_checks_are_soft())
          next_check =
              (unsigned long)(current_time + (hst->get_retry_interval() *
                                              config->interval_length()));

        /* schedule a re-check of the host at the normal interval */
        else
          next_check =
              (unsigned long)(current_time + (hst->get_check_interval() *
                                              config->interval_length()));

        /* propagate checks to immediate parents if they are UP */
        /* we do this because a parent host (or grandparent) may have gone down
         * and blocked our route */
        /* checking the parents ASAP will allow us to better determine the final
         * state (DOWN/UNREACHABLE) of this host later */
        logger(dbg_checks, more)
            << "Propagating checks to immediate parent hosts that "
               "are UP...";

        for (host_map::iterator it(hst->parent_hosts.begin()),
             end(hst->parent_hosts.end());
             it != end; it++) {
          if (it->second == nullptr)
            continue;
          if (it->second->get_current_state() == HOST_UP) {
            add_object_to_objectlist(&check_hostlist, it->second.get());
            logger(dbg_checks, more)
                << "Check of host '" << it->first << "' queued.";
          }
        }

        /* propagate checks to immediate children if they are not UNREACHABLE */
        /* we do this because we may now be blocking the route to child hosts */
        logger(dbg_checks, more)
            << "Propagating checks to immediate non-UNREACHABLE "
               "child hosts...";

        for (host_map::iterator it(hst->child_hosts.begin()),
             end(hst->child_hosts.end());
             it != end; it++) {
          if (it->second == nullptr)
            continue;
          if (it->second->get_current_state() != HOST_UNREACHABLE) {
            logger(dbg_checks, more)
                << "Check of child host '" << it->first << "' queued.";
            add_object_to_objectlist(&check_hostlist, it->second.get());
          }
        }

        /* check dependencies on second to last host check */
        if (config->enable_predictive_host_dependency_checks() &&
            hst->get_current_attempt() == (hst->get_max_attempts() - 1)) {
          /* propagate checks to hosts that THIS ONE depends on for
           * notifications AND execution */
          /* we do to help ensure that the dependency checks are accurate before
           * it comes time to notify */
          logger(dbg_checks, more)
              << "Propagating predictive dependency checks to hosts this "
                 "one depends on...";

          std::string id(hst->get_name());
          umultimap<std::string, std::shared_ptr<hostdependency> > const&
              dependencies(state::instance().hostdependencies());
          for (umultimap<std::string,
                         std::shared_ptr<hostdependency> >::const_iterator
                   it(dependencies.find(id)),
               end(dependencies.end());
               it != end && it->first == id; ++it) {
            hostdependency* temp_dependency(&*it->second);
            if (temp_dependency->dependent_host_ptr == hst &&
                temp_dependency->master_host_ptr != NULL) {
              master_host = (com::centreon::engine::host*)
                                temp_dependency->master_host_ptr;
              logger(dbg_checks, more)
                  << "Check of host '" << master_host->get_name()
                  << "' queued.";
              add_object_to_objectlist(&check_hostlist, (void*)master_host);
            }
          }
        }
      }
    }
  }

  logger(dbg_checks, more) << "Pre-handle_host_state() Host: "
                           << hst->get_name()
                           << ", Attempt=" << hst->get_current_attempt() << "/"
                           << hst->get_max_attempts() << ", Type="
                           << (hst->get_state_type() == HARD_STATE ? "HARD"
                                                                   : "SOFT")
                           << ", Final State=" << hst->get_current_state();

  /* handle the host state */
  hst->handle_state();

  logger(dbg_checks, more) << "Post-handle_host_state() Host: "
                           << hst->get_name()
                           << ", Attempt=" << hst->get_current_attempt() << "/"
                           << hst->get_max_attempts() << ", Type="
                           << (hst->get_state_type() == HARD_STATE ? "HARD"
                                                                   : "SOFT")
                           << ", Final State=" << hst->get_current_state();

  /******************** POST-PROCESSING STUFF *********************/

  /* if the plugin output differs from previous check and no state change, log
   * the current state/output if state stalking is enabled */
  if (hst->get_last_state() == hst->get_current_state() &&
      compare_strings(old_plugin_output,
                      const_cast<char*>(hst->get_plugin_output().c_str()))) {
    if (hst->get_current_state() == HOST_UP && hst->get_stalk_on_up())
      hst->log_event();

    else if (hst->get_current_state() == HOST_DOWN && hst->get_stalk_on_down())
      hst->log_event();

    else if (hst->get_current_state() == HOST_UNREACHABLE &&
             hst->get_stalk_on_unreachable())
      hst->log_event();
  }

  /* check to see if the associated host is flapping */
  hst->check_for_flapping(true, true, true);

  /* reschedule the next check of the host (usually ONLY for scheduled, active
   * checks, unless overridden above) */
  if (reschedule_check) {
    logger(dbg_checks, more)
        << "Rescheduling next check of host at " << my_ctime(&next_check);

    /* default is to reschedule host check unless a test below fails... */
    hst->set_should_be_scheduled(true);

    /* get the new current time */
    time(&current_time);

    /* make sure we don't get ourselves into too much trouble... */
    if (current_time > next_check)
      hst->set_next_check(current_time);
    else
      hst->set_next_check(next_check);

    // Make sure we rescheduled the next host check at a valid time.
    {
      timezone_locker lock{hst->get_timezone()};
      preferred_time = hst->get_next_check();
      get_next_valid_time(preferred_time, &next_valid_time,
                          hst->check_period_ptr);
      hst->set_next_check(next_valid_time);
    }

    /* hosts with non-recurring intervals do not get rescheduled if we're in a
     * HARD or UP state */
    if (hst->get_check_interval() == 0 &&
        (hst->get_state_type() == HARD_STATE ||
         hst->get_current_state() == HOST_UP))
      hst->set_should_be_scheduled(false);

    /* host with active checks disabled do not get rescheduled */
    if (!hst->get_checks_enabled())
      hst->set_should_be_scheduled(false);

    /* schedule a non-forced check if we can */
    if (hst->get_should_be_scheduled()) {
      hst->schedule_check(hst->get_next_check(), CHECK_OPTION_NONE);
    }
  }

  /* update host status - for both active (scheduled) and passive
   * (non-scheduled) hosts */
  hst->update_status(false);

  /* run async checks of all hosts we added above */
  /* don't run a check if one is already executing or we can get by with a
   * cached state */
  for (hostlist_item = check_hostlist; hostlist_item != NULL;
       hostlist_item = hostlist_item->next) {
    run_async_check = true;
    temp_host = (com::centreon::engine::host*)hostlist_item->object_ptr;

    logger(dbg_checks, most)
        << "ASYNC CHECK OF HOST: " << temp_host->get_name()
        << ", CURRENTTIME: " << current_time
        << ", LASTHOSTCHECK: " << temp_host->get_last_check()
        << ", CACHEDTIMEHORIZON: " << check_timestamp_horizon
        << ", USECACHEDRESULT: " << use_cached_result
        << ", ISEXECUTING: " << temp_host->get_is_executing();

    if (use_cached_result && (static_cast<unsigned long>(
                                  current_time - temp_host->get_last_check()) <=
                              check_timestamp_horizon))
      run_async_check = false;
    if (temp_host->get_is_executing())
      run_async_check = false;
    if (run_async_check)
      temp_host->run_async_check(CHECK_OPTION_NONE, 0.0, false,
                                 false, NULL, NULL);
  }
  free_objectlist(&check_hostlist);
  return OK;
}

/* adjusts current host check attempt before a new check is performed */
int adjust_host_check_attempt_3x(com::centreon::engine::host* hst,
                                 int is_active) {
  logger(dbg_functions, basic)
    << "adjust_host_check_attempt_3x()";

  if (hst == NULL)
    return ERROR;

  logger(dbg_checks, most)
    << "Adjusting check attempt number for host '" << hst->get_name()
    << "': current attempt=" << hst->get_current_attempt() << "/"
    << hst->get_max_attempts() << ", state=" << hst->get_current_state()
    << ", state type=" << hst->get_state_type();

  /* if host is in a hard state, reset current attempt number */
  if (hst->get_state_type() == HARD_STATE)
    hst->set_current_attempt(1);

  /* if host is in a soft UP state, reset current attempt number (active checks only) */
  else if (is_active && hst->get_state_type() == SOFT_STATE
           && hst->get_current_state() == HOST_UP)
    hst->set_current_attempt(1);

  /* increment current attempt number */
  else if (hst->get_current_attempt() < hst->get_max_attempts())
    hst->set_current_attempt(hst->get_current_attempt() + 1);

  logger(dbg_checks, most)
    << "New check attempt number = " << hst->get_current_attempt();
  return OK;
}

/* determination of the host's state based on route availability*//* used only to determine difference between DOWN and UNREACHABLE states */
int determine_host_reachability(com::centreon::engine::host* hst) {
  int state = HOST_DOWN;
  bool is_host_present = false;

  logger(dbg_functions, basic)
    << "determine_host_reachability()";

  if (hst == NULL)
    return HOST_DOWN;

  logger(dbg_checks, most)
    << "Determining state of host '" << hst->get_name()
    << "': current state=" << hst->get_current_state();

  /* host is UP - no translation needed */
  if (hst->get_current_state() == HOST_UP) {
    state = HOST_UP;
    logger(dbg_checks, most)
      << "Host is UP, no state translation needed.";
  }

  /* host has no parents, so it is DOWN */
  else if (hst->parent_hosts.size() == 0) {
    state = HOST_DOWN;
    logger(dbg_checks, most)
      << "Host has no parents, so it is DOWN.";
  }

  /* check all parent hosts to see if we're DOWN or UNREACHABLE */
  else {

    for (host_map::iterator
           it(hst->parent_hosts.begin()),
           end(hst->parent_hosts.end());
         it != end;
         it++) {

      if (it->second == nullptr)
        continue;

      is_host_present = true;
      /* bail out as soon as we find one parent host that is UP */
      if (it->second->get_current_state() == HOST_UP) {
        /* set the current state */
        state = HOST_DOWN;
        logger(dbg_checks, most)
          << "At least one parent (" << it->first
          << ") is up, so host is DOWN.";
        break;
      }
    }
    /* no parents were up, so this host is UNREACHABLE */
    if (!is_host_present) {
      state = HOST_UNREACHABLE;
      logger(dbg_checks, most)
        << "No parents were up, so host is UNREACHABLE.";
    }
  }

  return state;
}

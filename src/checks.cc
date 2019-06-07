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
#include "com/centreon/engine/notifier.hh"
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
  service* temp_service = NULL;
  int state =  service::state_ok;
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
    if (temp_service->get_state_type() == notifier::soft
        && !config->soft_state_dependencies())
      state = temp_service->get_last_hard_state();
    else
      state = temp_service->get_current_state();

    /* is the service we depend on in state that fails the dependency tests? */
    if (state ==  service::state_ok && temp_dependency->get_fail_on_ok())
      return DEPENDENCIES_FAILED;
    if (state ==  service::state_warning
        && temp_dependency->get_fail_on_warning())
      return DEPENDENCIES_FAILED;
    if (state ==  service::state_unknown
        && temp_dependency->get_fail_on_unknown())
      return DEPENDENCIES_FAILED;
    if (state ==  service::state_critical
        && temp_dependency->get_fail_on_critical())
      return DEPENDENCIES_FAILED;
    if ((state ==  service::state_ok && !temp_service->get_has_been_checked())
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
  time_t current_time{0L};
  time_t expected_time{0L};

  logger(dbg_functions, basic)
    << "check_for_orphaned_services()";

  /* get the current time */
  time(&current_time);

  /* check all services... */
  for (service_map::iterator
         it(service::services.begin()),
         end(service::services.end());
       it != end;
       ++it) {

    /* skip services that are not currently executing */
    if (!it->second->is_executing)
      continue;

    /* determine the time at which the check results should have come in (allow 10 minutes slack time) */
    expected_time
      = (time_t)(it->second->next_check + it->second->get_latency()
                 + config->service_check_timeout()
                 + config->check_reaper_interval() + 600);

    /* this service was supposed to have executed a while ago, but for some reason the results haven't come back in... */
    if (expected_time < current_time) {

      /* log a warning */
      logger(log_runtime_warning, basic)
        << "Warning: The check of service '"
        << it->first.second << "' on host '"
        << it->first.first << "' looks like it was orphaned "
        "(results never came back).  I'm scheduling an immediate check "
        "of the service...";

      logger(dbg_checks, more)
        << "Service '" << it->first.second
        << "' on host '" << it->first.first
        << "' was orphaned, so we're scheduling an immediate check...";

      /* decrement the number of running service checks */
      if (currently_running_service_checks > 0)
        currently_running_service_checks--;

      /* disable the executing flag */
      it->second->is_executing = false;

      /* schedule an immediate check of the service */
      it->second->schedule_check(current_time, CHECK_OPTION_ORPHAN_CHECK);
    }
  }
}

/* check freshness of service results */
void check_service_result_freshness() {
  time_t current_time{0L};

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
  for (service_map::iterator
         it(service::services.begin()),
         end(service::services.end());
       it != end;
       ++it) {

    /* skip services we shouldn't be checking for freshness */
    if (!it->second->get_check_freshness())
      continue;

    /* skip services that are currently executing (problems here will be caught by orphaned service check) */
    if (it->second->is_executing)
      continue;

    /* skip services that have both active and passive checks disabled */
    if (!it->second->get_checks_enabled()
        && !it->second->accept_passive_service_checks)
      continue;

    /* skip services that are already being freshened */
    if (it->second->is_being_freshened)
      continue;

    // See if the time is right...
    {
      timezone_locker lock(it->second->get_timezone());
      if (check_time_against_period(
            current_time,
            it->second->check_period_ptr) == ERROR)
        continue ;
    }

    /* EXCEPTION */
    /* don't check freshness of services without regular check intervals if we're using auto-freshness threshold */
    if (it->second->get_check_interval() == 0 &&
      it->second->get_freshness_threshold() == 0)
      continue;

    /* the results for the last check of this service are stale! */
    if (!it->second->is_result_fresh(current_time, true)) {

      /* set the freshen flag */
      it->second->is_being_freshened = true;

      /* schedule an immediate forced check of the service */
      it->second->schedule_check(
          current_time,
          CHECK_OPTION_FORCE_EXECUTION | CHECK_OPTION_FRESHNESS_CHECK);
    }
  }
  return;
}

/******************************************************************/
/*************** COMMON ROUTE/HOST CHECK FUNCTIONS ****************/
/******************************************************************/



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
  enum host::host_state state = host::state_up;
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
    if (temp_host->get_state_type() == notifier::soft
        && !config->soft_state_dependencies())
      state = temp_host->get_last_hard_state();
    else
      state = temp_host->get_current_state();

    /* is the host we depend on in state that fails the dependency tests? */
    if (state == host::state_up && temp_dependency->get_fail_on_up())
      return DEPENDENCIES_FAILED;
    if (state == host::state_down && temp_dependency->get_fail_on_down())
      return DEPENDENCIES_FAILED;
    if (state == host::state_unreachable
        && temp_dependency->get_fail_on_unreachable())
      return DEPENDENCIES_FAILED;
    if ((state == host::state_up && !temp_host->get_has_been_checked())
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
        && !it->second->get_accept_passive_checks())
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
    if (!it->second->is_result_fresh(current_time, true)) {

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

/******************************************************************/
/************* NAGIOS 3.X ROUTE/HOST CHECK FUNCTIONS **************/
/******************************************************************/

/*** ON-DEMAND HOST CHECKS USE THIS FUNCTION ***/

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
  if (hst->get_state_type() == notifier::hard)
    hst->set_current_attempt(1);

  /* if host is in a soft UP state, reset current attempt number (active checks only) */
  else if (is_active && hst->get_state_type() == notifier::soft
           && hst->get_current_state() ==  host::state_up)
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
  enum host::host_state state = host::state_down;
  bool is_host_present = false;

  logger(dbg_functions, basic)
    << "determine_host_reachability()";

  if (hst == NULL)
    return  host::state_down;

  logger(dbg_checks, most)
    << "Determining state of host '" << hst->get_name()
    << "': current state=" << hst->get_current_state();

  /* host is UP - no translation needed */
  if (hst->get_current_state() ==  host::state_up) {
    state =  host::state_up;
    logger(dbg_checks, most)
      << "Host is UP, no state translation needed.";
  }

  /* host has no parents, so it is DOWN */
  else if (hst->parent_hosts.size() == 0) {
    state =  host::state_down;
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
      if (it->second->get_current_state() ==  host::state_up) {
        /* set the current state */
        state =  host::state_down;
        logger(dbg_checks, most)
          << "At least one parent (" << it->first
          << ") is up, so host is DOWN.";
        break;
      }
    }
    /* no parents were up, so this host is UNREACHABLE */
    if (!is_host_present) {
      state =  host::state_unreachable;
      logger(dbg_checks, most)
        << "No parents were up, so host is UNREACHABLE.";
    }
  }

  return state;
}

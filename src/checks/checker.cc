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

#include <string.h>
#include <sys/time.h>

#include "globals.hh"
#include "neberrors.hh"
#include "error.hh"
#include "checks.hh"
#include "broker.hh"
#include "shared.hh"
#include "logging/logger.hh"
#include "commands/command.hh"
#include "commands/set.hh"
#include "checks/checker.hh"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::checks;

/**
 *  Get instance of the checker singleton.
 *
 *  @return This singleton.
 */
checker& checker::instance() {
  static checker instance;
  return (instance);
}

/**
 *  Cleanup the checker singleton.
 */
void checker::cleanup() {
  checker& instance = checker::instance();
  instance._mut_reap.lock();
  instance._to_reap.clear();
  instance._mut_reap.unlock();

  instance._mut_id.lock();
  instance._list_id.clear();
  instance._mut_id.unlock();
}

/**
 *  Reap and process all result recive by execution process.
 */
void checker::reap() {
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;;
  logger(dbg_checks, basic) << "Starting to reap check results.";

  // time to start reaping.
  time_t reaper_start_time;
  time(&reaper_start_time);

  int reaped_checks = 0;
  while (true) {
    check_result result;
    _mut_reap.lock();
    bool is_empty = _to_reap.isEmpty();
    if (is_empty == false) {
      logger(dbg_checks, basic)
	<< "Found a check result (#" << ++reaped_checks << ") to handle...";

      // get result host or service check.
      result = _to_reap.dequeue();
    }
    _mut_reap.unlock();

    if (is_empty == true) {
      break;
    }

    if (result.object_check_type == SERVICE_CHECK) {
      // check if the service exist.
      service* svc = find_service(result.host_name,
				  result.service_description);
      if (svc == NULL) {
	logger(log_runtime_warning, basic)
	  << "Warning: Check result queue contained results for service '"
	  << result.service_description << "' on host '"
	  << result.host_name << "', but the service could "
	  << "not be found! Perhaps you forgot to define the service in your "
	  << "config files?";

	// cleanup.
        free_check_result(&result);
	continue;
      }

      logger(dbg_checks, more)
	<< "Handling check result for service '"
	<< result.service_description << "' on host '"
	<< result.host_name << "'...";

      // process the check result.
      handle_async_service_check_result(svc, &result);
    }
    else {
      host* hst = find_host(result.host_name);
      if (hst == NULL) {
        // check if the host exist.
	logger(log_runtime_warning, basic)
	  << "Warning: Check result queue contained results for host '"
	  << result.host_name << "', but the host could not be found! "
	  << "Perhaps you forgot to define the host in your config files?";

	// cleanup.
        free_check_result(&result);
        continue;
      }

      logger(dbg_checks, more)
	<< "Handling check result for host '" << result.host_name << "'...";

      // process the check result.
      handle_async_host_check_result_3x(hst, &result);
    }

    // cleanup.
    free_check_result(&result);

    // check if reaping is timeout.
    time_t current_time;
    time(&current_time);
    if ((current_time - reaper_start_time)
	> static_cast<time_t>(config.get_max_check_reaper_time())) {
      logger(dbg_checks, basic)
	<< "Breaking out of check result reaper: max reaper time exceeded";
      break;
    }

    // catch signal, need to break.
    if (sigshutdown == true || sigrestart == true) {
      logger(dbg_checks, basic)
	<< "Breaking out of check result reaper: signal encountered";
      break;
    }
  }

  logger(dbg_checks, basic)
    << "Finished reaping " << reaped_checks << " check results";
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;;
}

/**
 *  Add into the queue a result to reap later.
 *
 *  @param[in] result The check_result to process later.
 */
void checker::push_check_result(check_result const& result) {
  _mut_reap.lock();
  _to_reap.enqueue(result);
  _mut_reap.unlock();
}

/**
 *  Run an host check without waiting check result.
 *
 *  @param[in]  hst              Host to check.
 *  @param[in]  check_options    Event options.
 *  @param[in]  latency          Host latency.
 *  @param[in]  scheduled_check  If the check are schedule.
 *  @param[in]  reschedule_check If the check are reschedule.
 *  @param[out] time_is_valid    Host check viable at this time.
 *  @param[out] preferred_time   The next preferred check time.
 */
void checker::run(host* hst,
		  int check_options,
		  double latency,
		  bool scheduled_check,
		  bool reschedule_check,
		  int* time_is_valid,
		  time_t* preferred_time) {
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;;

  if (hst == NULL) {
    throw (engine_error() << "host pointer is NULL.");
  }

  logger(dbg_checks, basic)
    << "** Running async check of host '" << hst->name << "'...";

  // check if the host is viable now.
  if (check_host_check_viability_3x(hst,
				    check_options,
				    time_is_valid,
				    preferred_time) == ERROR) {
    throw (engine_error() << "check host viability failure.");
  }

  // don't execute a new host check if one is already running.
  if (hst->is_executing == true
      && !(check_options & CHECK_OPTION_FORCE_EXECUTION)) {
    throw (engine_error() << "A check of this host is already being executed, "
	   << "so we'll pass for the moment...");
  }

  timeval start_time = timeval();
  timeval end_time = timeval();

  // send event broker.
  int res = broker_host_check(NEBTYPE_HOSTCHECK_ASYNC_PRECHECK,
			      NEBFLAG_NONE,
			      NEBATTR_NONE,
			      hst,
			      HOST_CHECK_ACTIVE,
			      hst->current_state,
			      hst->state_type,
			      start_time,
			      end_time,
			      hst->host_check_command,
			      hst->latency,
			      0.0,
			      config.get_host_check_timeout(),
			      false,
			      0,
			      NULL,
			      NULL,
			      NULL,
			      NULL,
			      NULL);

  // host check was cancel by neb_module. reschedule check later.
  if (res == NEBERROR_CALLBACKCANCEL) {
    throw (engine_error() << "broker callback cancel.");
  }
  // host check was override by neb_module.
  if (res == NEBERROR_CALLBACKOVERRIDE) {
    return;
  }

  logger(dbg_functions, basic) << "Checking host '" << hst->name << "'...";

  // clear check options.
  if (scheduled_check == true) {
    hst->check_options = CHECK_OPTION_NONE;
  }

  adjust_host_check_attempt_3x(hst, true);

  // update latency for event broker and macros.
  double old_latency = hst->latency;
  hst->latency = latency;

  // get current host macros.
  nagios_macros macros;
  memset(&macros, 0, sizeof(macros));
  grab_host_macros(&macros, hst);
  get_raw_command_line_r(&macros,
			 hst->check_command_ptr,
                         hst->host_check_command,
			 NULL,
			 0);

  // time to start command.
  gettimeofday(&start_time, NULL);

  /*
    set check time for on-demand checks, so they're not incorrectly detected as
    being orphaned - Luke Ross 5/16/08
    NOTE: 06/23/08 EG not sure if there will be side effects to this or not....
  */
  if (scheduled_check == false) {
    hst->next_check = start_time.tv_sec;
  }

  // update the number of running host checks.
  ++currently_running_host_checks;

  // set the execution flag.
  hst->is_executing = true;

  // init check result info.
  check_result check_result_info;
  check_result_info.object_check_type = HOST_CHECK;
  check_result_info.check_type = HOST_CHECK_ACTIVE;
  check_result_info.check_options = check_options;
  check_result_info.scheduled_check = scheduled_check;
  check_result_info.reschedule_check = reschedule_check;
  check_result_info.start_time = start_time;
  check_result_info.finish_time = start_time;
  check_result_info.early_timeout = false;
  check_result_info.exited_ok = true;
  check_result_info.return_code = STATE_OK;
  check_result_info.output = NULL;
  check_result_info.output_file_fd = -1;
  check_result_info.output_file_fp = NULL;
  check_result_info.output_file = NULL;
  check_result_info.host_name = my_strdup(hst->name);
  check_result_info.service_description = NULL;
  check_result_info.latency = latency;
  check_result_info.next = NULL;

  commands::set& cmd_set = commands::set::instance();
  QSharedPointer<commands::command> cmd = cmd_set.get_command(hst->check_command_ptr->name);
  QString processed_cmd = cmd->process_cmd(&macros);
  char* tmp_processed_cmd = my_strdup(qPrintable(processed_cmd));

  // send event broker.
  broker_host_check(NEBTYPE_HOSTCHECK_INITIATE,
		    NEBFLAG_NONE,
                    NEBATTR_NONE,
		    hst,
		    HOST_CHECK_ACTIVE,
                    hst->current_state,
		    hst->state_type,
		    start_time,
                    end_time,
		    hst->host_check_command,
		    hst->latency,
                    0.0,
		    config.get_host_check_timeout(),
		    false,
		    0,
                    tmp_processed_cmd,
		    NULL,
		    NULL,
		    NULL,
		    NULL);

  delete[] tmp_processed_cmd;

  // restore latency.
  hst->latency = old_latency;

  // update statistics.
  update_check_stats(scheduled_check == TRUE
		     ? ACTIVE_SCHEDULED_HOST_CHECK_STATS
		     : ACTIVE_ONDEMAND_HOST_CHECK_STATS,
                     start_time.tv_sec);
  update_check_stats(PARALLEL_HOST_CHECK_STATS, start_time.tv_sec);

  connect(&(*cmd), SIGNAL(command_executed(commands::result const&)),
	  this, SLOT(_command_executed(commands::result const&)),
	  Qt::UniqueConnection);

  // run command.
  unsigned long id = cmd->run(processed_cmd,
			      macros,
			      config.get_host_check_timeout());
  _mut_id.lock();
  _list_id[id] = check_result_info;
  _mut_id.unlock();

  // cleanup.
  clear_volatile_macros(&macros);

  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;;
}

/**
 *  Run an service check without waiting check result.
 *
 *  @param[in] svc              Service to check.
 *  @param[in] check_options    Event options.
 *  @param[in] latency          Service latency.
 *  @param[in] scheduled_check  If the check are schedule.
 *  @param[in] reschedule_check If the check are reschedule.
 *  @param[in] time_is_valid    Service check viable at this time.
 *  @param[in] preferred_time   The next preferred check time.
 */
void checker::run(service* svc,
		  int check_options,
		  double latency,
		  bool scheduled_check,
		  bool reschedule_check,
		  int* time_is_valid,
		  time_t* preferred_time) {
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;

  if (svc == NULL) {
    throw (engine_error() << "service pointer is NULL.");
  }

  if (svc->host_ptr == NULL) {
    throw (engine_error() << "host pointer is NULL.");
  }

  logger(dbg_checks, basic)
    << "** Running async check of service '" << svc->description
    << "' on host '" << svc->host_name << "'...";

  // check if the service is viable now.
  if (check_service_check_viability(svc,
				    check_options,
				    time_is_valid,
				    preferred_time) == ERROR) {
    throw (engine_error() << "check service viability failure.");
  }

  timeval start_time = timeval();
  timeval end_time = timeval();

  // send event broker.
  int res = broker_service_check(NEBTYPE_SERVICECHECK_ASYNC_PRECHECK,
				 NEBFLAG_NONE,
				 NEBATTR_NONE,
				 svc,
				 SERVICE_CHECK_ACTIVE,
				 start_time,
				 end_time,
				 svc->service_check_command,
				 svc->latency,
				 0.0,
				 0,
				 false,
				 0,
				 NULL,
				 NULL);

  // service check was cancel by neb_module. reschedule check later.
  if (res == NEBERROR_CALLBACKCANCEL) {
    if (preferred_time != NULL) {
      *preferred_time += static_cast<time_t>(svc->check_interval * config.get_interval_length());
    }
    throw (engine_error() << "broker callback cancel.");
  }

  // service check was override by neb_module.
  if (res == NEBERROR_CALLBACKOVERRIDE) {
    return;
  }

  logger(dbg_checks, basic)
    << "Checking service '" << svc->description
    << "' on host '" << svc->host_name << "'...";

  // clear check options.
  if (scheduled_check == true) {
    svc->check_options = CHECK_OPTION_NONE;
  }

  // update latency for event broker and macros.
  double old_latency = svc->latency;
  svc->latency = latency;

  // get current host and service macros.
  nagios_macros macros;
  memset(&macros, 0, sizeof(macros));
  grab_host_macros(&macros, svc->host_ptr);
  grab_service_macros(&macros, svc);
  get_raw_command_line_r(&macros,
			 svc->check_command_ptr,
			 svc->service_check_command,
			 NULL,
			 0);

  // time to start command.
  gettimeofday(&start_time, NULL);

  // update the number of running service checks.
  ++currently_running_service_checks;

  // set the execution flag.
  svc->is_executing = true;

  // init check result info.
  check_result check_result_info;
  check_result_info.object_check_type = SERVICE_CHECK;
  check_result_info.check_type = SERVICE_CHECK_ACTIVE;
  check_result_info.check_options = check_options;
  check_result_info.scheduled_check = scheduled_check;
  check_result_info.reschedule_check = reschedule_check;
  check_result_info.start_time = start_time;
  check_result_info.finish_time = start_time;
  check_result_info.early_timeout = false;
  check_result_info.exited_ok = true;
  check_result_info.return_code = STATE_OK;
  check_result_info.output = NULL;
  check_result_info.output_file_fd = -1;
  check_result_info.output_file_fp = NULL;
  check_result_info.output_file = NULL;
  check_result_info.host_name = my_strdup(svc->host_name);
  check_result_info.service_description = my_strdup(svc->description);
  check_result_info.latency = latency;
  check_result_info.next = NULL;

  commands::set& cmd_set = commands::set::instance();
  QSharedPointer<commands::command> cmd = cmd_set.get_command(svc->check_command_ptr->name);
  QString processed_cmd = cmd->process_cmd(&macros);
  char* tmp_processed_cmd = my_strdup(qPrintable(processed_cmd));

  // send event broker.
  res = broker_service_check(NEBTYPE_SERVICECHECK_INITIATE,
			     NEBFLAG_NONE,
			     NEBATTR_NONE,
			     svc,
			     SERVICE_CHECK_ACTIVE,
			     start_time,
			     end_time,
			     svc->service_check_command,
			     svc->latency,
			     0.0,
			     config.get_service_check_timeout(),
			     false,
			     0,
			     tmp_processed_cmd,
			     NULL);

  delete[] tmp_processed_cmd;

  // restore latency.
  svc->latency = old_latency;

  // service check was override by neb_module.
  if (res == NEBERROR_CALLBACKOVERRIDE) {
    clear_volatile_macros(&macros);
    return;
  }

  // update statistics.
  update_check_stats(scheduled_check == TRUE
		     ? ACTIVE_SCHEDULED_SERVICE_CHECK_STATS
		     : ACTIVE_ONDEMAND_SERVICE_CHECK_STATS,
		     start_time.tv_sec);

  connect(&(*cmd), SIGNAL(command_executed(commands::result const&)),
	  this, SLOT(_command_executed(commands::result const&)),
	  Qt::UniqueConnection);

  // run command.
  unsigned long id = cmd->run(processed_cmd,
			      macros,
			      config.get_service_check_timeout());
  _mut_id.lock();
  _list_id[id] = check_result_info;
  _mut_id.unlock();

  // cleanup.
  clear_volatile_macros(&macros);

  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
}

/**
 *  Run an host check with waiting check result.
 *
 *  @param[in]  hst                     Host to check.
 *  @param[out] check_result_code       The return value of the execution.
 *  @param[in]  check_options           Event options.
 *  @param[in]  use_cached_result       Used the last result.
 *  @param[in]  check_timestamp_horizon XXX
 */
void checker::run_sync(host* hst,
		       int* check_result_code,
		       int check_options,
		       int use_cached_result,
		       unsigned long check_timestamp_horizon) {
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;

  if (hst == NULL) {
    throw (engine_error() << "host pointer is NULL.");
  }

  logger(dbg_checks, basic)
    << "** Run sync check of host '" << hst->name << "'...";

  // check if the host is viable now.
  if (check_host_check_viability_3x(hst, check_options, NULL, NULL) == ERROR) {
    if (check_result_code) {
      *check_result_code = hst->current_state;
    }
    logger(dbg_checks, basic) << "Host check is not viable at this time.";
    return;
  }

  // time to start command.
  timeval start_time;
  gettimeofday(&start_time, NULL);

  // can we use the last cached host state?
  if (use_cached_result == true
      && !(check_options & CHECK_OPTION_FORCE_EXECUTION)) {

    // we can used the cached result, so return it and get out of here...
    if (hst->has_been_checked == true
        && (static_cast<unsigned long>(start_time.tv_sec - hst->last_check)
	    <= check_timestamp_horizon)) {
      if (check_result_code) {
        *check_result_code = hst->current_state;
      }

      logger(dbg_checks, more)
	<< "* Using cached host state: " << hst->current_state;

      // update statistics.
      update_check_stats(ACTIVE_ONDEMAND_HOST_CHECK_STATS, start_time.tv_sec);
      update_check_stats(ACTIVE_CACHED_HOST_CHECK_STATS, start_time.tv_sec);
      return;
    }
  }

  logger(dbg_checks, more)
    << "* Running actual host check: old state=" << hst->current_state;

  // update statistics.
  update_check_stats(ACTIVE_ONDEMAND_HOST_CHECK_STATS, start_time.tv_sec);
  update_check_stats(SERIAL_HOST_CHECK_STATS, start_time.tv_sec);

  // reset host check latency, since on-demand checks have none.
  hst->latency = 0.0;

  adjust_host_check_attempt_3x(hst, true);

  // update host state.
  hst->last_state = hst->current_state;
  if (hst->state_type == HARD_STATE) {
    hst->last_hard_state = hst->current_state;
  }

  // save old plugin output for state stalking.
  char* old_plugin_output = NULL;
  if (hst->plugin_output) {
    old_plugin_output = my_strdup(hst->plugin_output);
  }

  // set the checked flag.
  hst->has_been_checked = true;

  // clear the freshness flag.
  hst->is_being_freshened = false;

  // clear check options - we don't want old check options retained.
  hst->check_options = CHECK_OPTION_NONE;

  // set the check type.
  hst->check_type = HOST_CHECK_ACTIVE;

  timeval end_time = timeval();

  // send event broker.
  broker_host_check(NEBTYPE_HOSTCHECK_INITIATE,
		    NEBFLAG_NONE,
                    NEBATTR_NONE,
		    hst,
		    HOST_CHECK_ACTIVE,
                    hst->current_state,
		    hst->state_type,
		    start_time,
                    end_time,
		    hst->host_check_command,
		    hst->latency,
                    0.0,
		    config.get_host_check_timeout(),
		    false,
		    0,
                    NULL,
		    NULL,
		    NULL,
		    NULL,
		    NULL);

  int host_result = _execute_sync(hst);

  process_host_check_result_3x(hst,
  			       host_result,
  			       old_plugin_output,
                               check_options,
  			       false,
  			       use_cached_result,
                               check_timestamp_horizon);

  if (check_result_code) {
    *check_result_code = hst->current_state;
  }

  // cleanup.
  delete[] old_plugin_output;

  logger(dbg_checks, more)
    << "* Sync host check done: new state=" << hst->current_state;

  // get the end time of command.
  gettimeofday(&end_time, NULL);

  // send event broker.
  broker_host_check(NEBTYPE_HOSTCHECK_PROCESSED,
		    NEBFLAG_NONE,
                    NEBATTR_NONE,
		    hst,
		    HOST_CHECK_ACTIVE,
                    hst->current_state,
		    hst->state_type,
		    start_time,
                    end_time,
		    hst->host_check_command,
		    hst->latency,
                    hst->execution_time,
                    config.get_host_check_timeout(),
		    false,
                    hst->current_state,
		    NULL,
		    hst->plugin_output,
                    hst->long_plugin_output,
		    hst->perf_data,
		    NULL);

  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
}

/**
 *  Slot to catch the result of the execution and add to the reap queue.
 *
 *  @param[in] res The result of the execution.
 */
void checker::_command_executed(commands::result const& res) {
  _mut_id.lock();
  QHash<unsigned long, check_result>::iterator it = _list_id.find(res.get_command_id());
  if (it == _list_id.end()) {
    _mut_id.unlock();
    logger(log_runtime_warning, basic)
      << "command id '" << res.get_command_id() << "' not found.";
    return;
  }

  check_result result = it.value();
  _list_id.erase(it);
  _mut_id.unlock();

  result.finish_time = res.get_end_time();
  result.early_timeout = res.get_is_timeout();
  result.return_code = res.get_exit_code();
  result.exited_ok = res.get_is_executed();
  if (res.get_is_executed() == true && res.get_is_timeout() == false) {
    result.output = my_strdup(qPrintable(res.get_stdout()));
  }
  else {
    result.output = my_strdup(qPrintable(res.get_stderr()));
  }

  _mut_reap.lock();
  _to_reap.enqueue(result);
  _mut_reap.unlock();
}

/**
 *  Default constructor.
 */
checker::checker() {

}

/**
 *  Default destructor.
 */
checker::~checker() throw() {

}

/**
 *  Run an host check with waiting check result.
 *
 *  @param[in] hst The host to check.
 *
 *  @result Return if the host is up (HOST_UP) or host down (HOST_DOWN).
 */
int checker::_execute_sync(host* hst) {
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;

  if (hst == NULL) {
    throw (engine_error() << "host pointer is NULL.");
  }

  logger(dbg_checks, basic)
    << "** Executing sync check of host '" << hst->name << "'...";

  timeval start_time = timeval();
  timeval end_time = timeval();

  // send event broker.
  int res = broker_host_check(NEBTYPE_HOSTCHECK_SYNC_PRECHECK,
			      NEBFLAG_NONE,
			      NEBATTR_NONE,
			      hst,
			      HOST_CHECK_ACTIVE,
			      hst->current_state,
			      hst->state_type,
			      start_time,
			      end_time,
			      hst->host_check_command,
			      hst->latency,
			      0.0,
			      config.get_host_check_timeout(),
			      false,
			      0,
			      NULL,
			      NULL,
			      NULL,
			      NULL,
			      NULL);

  // host sync check was cancel by neb_module.
  if (res == NEBERROR_CALLBACKCANCEL) {
    return (hst->current_state);
  }

  // host sync check was override by neb_module.
  if (res == NEBERROR_CALLBACKOVERRIDE) {
    return (hst->current_state);
  }

  // get current host mocros.
  nagios_macros macros;
  memset(&macros, 0, sizeof(macros));
  grab_host_macros(&macros, hst);
  get_raw_command_line_r(&macros,
			 hst->check_command_ptr,
                         hst->host_check_command,
			 NULL,
			 0);

  // time to start command.
  gettimeofday(&start_time, NULL);

  // update last host check.
  hst->last_check = start_time.tv_sec;

  commands::set& cmd_set = commands::set::instance();
  QSharedPointer<commands::command> cmd = cmd_set.get_command(hst->check_command_ptr->name);
  QString processed_cmd = cmd->process_cmd(&macros);
  char* tmp_processed_cmd = my_strdup(qPrintable(processed_cmd));

  // send event broker.
  broker_host_check(NEBTYPE_HOSTCHECK_RAW_START,
		    NEBFLAG_NONE,
                    NEBATTR_NONE,
		    hst,
		    HOST_CHECK_ACTIVE,
		    HOST_UP,
                    hst->state_type,
		    start_time,
		    end_time,
                    hst->host_check_command,
		    0.0,
		    0.0,
                    config.get_host_check_timeout(),
		    false,
                    STATE_OK,
		    tmp_processed_cmd,
		    hst->plugin_output,
                    hst->long_plugin_output,
		    hst->perf_data,
		    NULL);

  logger(dbg_commands, more)
    << "Raw host check command: " << hst->check_command_ptr->command_line;
  logger(dbg_commands, more)
    << "Processed host check ommand: " << processed_cmd;

  // cleanup.
  delete[] hst->plugin_output;
  delete[] hst->long_plugin_output;
  delete[] hst->perf_data;

  hst->plugin_output = NULL;
  hst->long_plugin_output = NULL;
  hst->perf_data = NULL;

  timeval start_cmd;
  gettimeofday(&start_cmd, NULL);

  timeval end_cmd = timeval();

  // send broker event.
  broker_system_command(NEBTYPE_SYSTEM_COMMAND_START,
			NEBFLAG_NONE,
                        NEBATTR_NONE,
			start_cmd,
			end_cmd,
			0,
                        config.get_host_check_timeout(),
			false,
			0,
			tmp_processed_cmd,
			NULL,
                        NULL);

  commands::result cmd_result;
  cmd->run(processed_cmd,
	   macros,
	   config.get_host_check_timeout(),
	   cmd_result);

  char* output = NULL;
  if (cmd_result.get_is_executed() == true) {
    output = my_strdup(qPrintable(cmd_result.get_stdout()));
  }
  else {
    output = my_strdup(qPrintable(cmd_result.get_stderr()));
  }

  // send broker event.
  broker_system_command(NEBTYPE_SYSTEM_COMMAND_END,
			NEBFLAG_NONE,
			NEBATTR_NONE,
			cmd_result.get_start_time(),
			cmd_result.get_end_time(),
			cmd_result.get_execution_time(),
			config.get_host_check_timeout(),
			cmd_result.get_is_timeout(),
			cmd_result.get_exit_code(),
			tmp_processed_cmd,
			output,
			NULL);

  delete[] output;

  clear_volatile_macros(&macros);

  // if the command timeout.
  if (cmd_result.get_is_timeout() == true) {
    QString output("Host check timed out after %1  seconds\n");
    output.arg(config.get_host_check_timeout());

    cmd_result.set_stdout(output);

    logger(log_runtime_warning, basic)
      << "Warning: Host check command '" << processed_cmd
      << "' for host '" << hst->name << "' timed out after "
      << config.get_host_check_timeout() << " seconds";
  }

  // update value.
  hst->execution_time = cmd_result.get_execution_time();
  hst->check_type = HOST_CHECK_ACTIVE;

  char* tmp_plugin_output = NULL;
  if (cmd_result.get_is_executed() == true) {
    tmp_plugin_output = my_strdup(qPrintable(cmd_result.get_stdout()));
  }
  else {
    tmp_plugin_output = my_strdup(qPrintable(cmd_result.get_stderr()));
  }

  // parse the output: short and long output, and perf data.
  parse_check_output(tmp_plugin_output,
  		     &hst->plugin_output,
		     &hst->long_plugin_output,
  		     &hst->perf_data,
  		     true,
		     true);

  delete[] tmp_plugin_output;

  // a NULL host check command means we should assume the host is UP.
  if (hst->host_check_command == NULL) {
    delete[] hst->plugin_output;
    hst->plugin_output = my_strdup("(Host assumed to be UP)");
    cmd_result.set_exit_code(STATE_OK);
  }

  // make sure we have some data.
  if (hst->plugin_output == NULL || !strcmp(hst->plugin_output, "")) {
    delete[] hst->plugin_output;
    hst->plugin_output = my_strdup("(No output returned from host check)");
  }

  // replace semicolons with colons in plugin output (but not performance data).
  if (hst->plugin_output != NULL) {
    for (char* ptr = hst->plugin_output; (ptr = strchr(ptr, ';')); *ptr = ':') {
    }
  }

  /*
    if we're not doing aggressive host checking,
    let WARNING states indicate the host is up (fake the result to be STATE_OK)
  */
  if (config.get_use_aggressive_host_checking() == false
      && cmd_result.get_exit_code() == STATE_WARNING) {
    cmd_result.set_exit_code(STATE_OK);
  }

  int return_result = (cmd_result.get_exit_code() == STATE_OK ? HOST_UP : HOST_DOWN);

  // get the end time of command.
  gettimeofday(&end_time, NULL);

  // send event broker.
  broker_host_check(NEBTYPE_HOSTCHECK_RAW_END,
		    NEBFLAG_NONE,
                    NEBATTR_NONE,
		    hst,
		    HOST_CHECK_ACTIVE,
		    return_result,
                    hst->state_type,
		    start_time,
		    end_time,
                    hst->host_check_command,
		    0.0,
		    cmd_result.get_execution_time(),
                    config.get_host_check_timeout(),
		    cmd_result.get_is_timeout(),
                    cmd_result.get_exit_code(),
		    tmp_processed_cmd,
		    hst->plugin_output,
                    hst->long_plugin_output,
		    hst->perf_data,
		    NULL);

  delete[] tmp_processed_cmd;

  logger(dbg_checks, basic)
    << "** Sync host check done: state=" << return_result;

  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;

  return (return_result);
}

/*
** Copyright 1999-2010 Ethan Galstad
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

#include <assert.h>
#include <QByteArray>
#include <QMetaType>
#include <QMutexLocker>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/commands/command.hh"
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/shared.hh"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::checks;

// Class instance.
std::auto_ptr<checker> checker::_instance;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Default destructor.
 */
checker::~checker() throw () {
  try {
    QMutexLocker lock(&_mut_reap);
    for (QQueue<check_result>::iterator
           it = _to_reap.begin(),
           end = _to_reap.end();
         it != end;
         ++it)
      free_check_result(&*it);
    _to_reap.clear();
  }
  catch (...) {}
}

/**
 *  Get instance of the checker singleton.
 *
 *  @return This singleton.
 */
checker& checker::instance() {
  return (*_instance);
}

/**
 *  Load singleton.
 */
void checker::load() {
  if (!_instance.get())
    _instance.reset(new checker);
  return ;
}

/**
 *  Add into the queue a result to reap later.
 *
 *  @param[in] result The check_result to process later.
 */
void checker::push_check_result(check_result const& result) {
  QMutexLocker lock(&_mut_reap);
  _to_reap.enqueue(result);
  return ;
}

/**
 *  Reap and process all result recive by execution process.
 */
void checker::reap() {
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;
  logger(dbg_checks, basic) << "Starting to reap check results.";

  // Time to start reaping.
  time_t reaper_start_time;
  time(&reaper_start_time);

  // Reap check results.
  unsigned int reaped_checks(0);
  { // Scope to release mutex in all termination cases.
    QMutexLocker lock(&_mut_reap);
    while (!_to_reap.isEmpty()) {
      // Get result host or service check.
      logger(dbg_checks, basic)
        << "Found a check result (#" << ++reaped_checks
        << ") to handle...";
      check_result result(_to_reap.dequeue());
      lock.unlock();

      // Service check result.
      if (SERVICE_CHECK == result.object_check_type) {
        // Check if the service exists.
        service* svc(find_service(
                       result.host_name,
                       result.service_description));
        if (!svc)
          logger(log_runtime_warning, basic)
            << "Warning: Check result queue contained results for "
            << "service '" << result.service_description << "' on "
            << "host '" << result.host_name << "', but the service "
            << "could not be found! Perhaps you forgot to define the "
            << "service in your config files ?";
        else {
          // Process the check result.
          logger(dbg_checks, more)
            << "Handling check result for service '"
            << result.service_description << "' on host '"
            << result.host_name << "'...";
          handle_async_service_check_result(svc, &result);
        }
      }
      // Host check result.
      else {
        host* hst(find_host(result.host_name));
        if (!hst)
          // Check if the host exists.
          logger(log_runtime_warning, basic)
            << "Warning: Check result queue contained results for "
            << "host '" << result.host_name << "', but the host could "
            << "not be found! Perhaps you forgot to define the host in "
            << "your config files ?";
        else {
          // Process the check result.
          logger(dbg_checks, more)
            << "Handling check result for host '"
            << result.host_name << "'...";
          handle_async_host_check_result_3x(hst, &result);
        }
      }

      // Cleanup.
      free_check_result(&result);

      // Check if reaping has timed out.
      time_t current_time;
      time(&current_time);
      if ((current_time - reaper_start_time)
          > static_cast<time_t>(config.get_max_check_reaper_time())) {
        logger(dbg_checks, basic)
          << "Breaking out of check result reaper: "
          << "max reaper time exceeded";
        break ;
      }

      // Caught signal, need to break.
      if (sigshutdown || sigrestart) {
        logger(dbg_checks, basic)
          << "Breaking out of check result reaper: signal encountered";
        break ;
      }

      // Mutex needed to access list.
      lock.relock();
    }
  }

  // Reaping finished.
  logger(dbg_checks, basic)
    << "Finished reaping " << reaped_checks << " check results";
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
  return ;
}

/**
 *  Check if the reaper queue is empty.
 *
 *  @return True if the reper queue is empty, otherwise false.
 */
bool checker::reaper_is_empty() {
  QMutexLocker lock(&_mut_reap);
  return (_to_reap.isEmpty());
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
 *
 *  @return True is the check start correctly.
 */
void checker::run(
                host* hst,
                int check_options,
                double latency,
                bool scheduled_check,
                bool reschedule_check,
                int* time_is_valid,
                time_t* preferred_time) {
  // Preamble.
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;
  if (!hst)
    throw (engine_error() << "attempt to run check on NULL host");
  if (!hst->check_command_ptr)
    throw (engine_error() << "attempt to run active check on host '"
           << hst->name << "' with no check command");
  logger(dbg_checks, basic)
    << "** Running async check of host '" << hst->name << "'...";

  // Check if the host is viable now.
  if (check_host_check_viability_3x(
        hst,
        check_options,
        time_is_valid,
        preferred_time) == ERROR)
    throw (engine_error() << "check host viability failure");

  // Don't execute a new host check if one is already running.
  if (hst->is_executing
      && !(check_options & CHECK_OPTION_FORCE_EXECUTION)) {
    logger(dbg_checks, basic)
      << "A check of this host (" << hst->name
      << ") is already being executed, so we'll pass for the moment...";
    return ;
  }

  // Send broker event.
  timeval start_time;
  timeval end_time;
  memset(&start_time, 0, sizeof(start_time));
  memset(&end_time, 0, sizeof(end_time));
  int res(broker_host_check(
            NEBTYPE_HOSTCHECK_ASYNC_PRECHECK,
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
            NULL));

  // Host check was cancel by NEB module. Reschedule check later.
  if (NEBERROR_CALLBACKCANCEL == res)
    throw (engine_error() << "broker callback cancel");
  // Host check was overriden by NEB module.
  else if (NEBERROR_CALLBACKOVERRIDE == res)
    return ;

  // Checking starts.
  logger(dbg_functions, basic)
    << "Checking host '" << hst->name << "'...";

  // Clear check options.
  if (scheduled_check)
    hst->check_options = CHECK_OPTION_NONE;

  // Adjust check attempts.
  adjust_host_check_attempt_3x(hst, true);

  // Update latency for event broker and macros.
  double old_latency(hst->latency);
  hst->latency = latency;

  // Get current host macros.
  nagios_macros macros;
  memset(&macros, 0, sizeof(macros));
  grab_host_macros_r(&macros, hst);
  get_raw_command_line_r(
    &macros,
    hst->check_command_ptr,
    hst->host_check_command,
    NULL,
    0);

  // Time to start command.
  gettimeofday(&start_time, NULL);

  // Set check time for on-demand checks, so they're
  // not incorrectly detected as being orphaned.
  if (!scheduled_check)
    hst->next_check = start_time.tv_sec;

  // Update the number of running host checks.
  ++currently_running_host_checks;

  // Set the execution flag.
  hst->is_executing = true;

  // Init check result info.
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

  // Get command object.
  commands::set& cmd_set(commands::set::instance());
  QSharedPointer<commands::command>
    cmd(cmd_set.get_command(hst->check_command_ptr->name));
  QString processed_cmd(cmd->process_cmd(&macros));

  // Send event broker.
  broker_host_check(
    NEBTYPE_HOSTCHECK_INITIATE,
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
    QByteArray(qPrintable(processed_cmd)).data(),
    NULL,
    NULL,
    NULL,
    NULL);

  // Restore latency.
  hst->latency = old_latency;

  // Update statistics.
  update_check_stats(
    (scheduled_check == TRUE)
    ? ACTIVE_SCHEDULED_HOST_CHECK_STATS
    : ACTIVE_ONDEMAND_HOST_CHECK_STATS,
    start_time.tv_sec);
  update_check_stats(PARALLEL_HOST_CHECK_STATS, start_time.tv_sec);

  // Change signal connection type.
  disconnect(
    &(*cmd),
    SIGNAL(command_executed(cce_commands_result const&)),
    this,
    SLOT(_command_executed(cce_commands_result const&)));
  connect(
    &(*cmd),
    SIGNAL(command_executed(cce_commands_result const&)),
    this,
    SLOT(_command_executed(cce_commands_result const&)),
    Qt::QueuedConnection);

  // Run command.
  unsigned long id(cmd->run(
                          processed_cmd,
                          macros,
                          config.get_host_check_timeout()));
  if (id != 0) {
    QMutexLocker lock(&_mut_id);
    _list_id[id] = check_result_info;
  }

  // Cleanup.
  clear_volatile_macros_r(&macros);
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
  return ;
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
 *
 *  @return True is the check start correctly.
 */
void checker::run(
                service* svc,
                int check_options,
                double latency,
                bool scheduled_check,
                bool reschedule_check,
                int* time_is_valid,
                time_t* preferred_time) {
  // Preamble.
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;
  if (!svc)
    throw (engine_error() << "attempt to run check on NULL service");
  else if (!svc->host_ptr)
    throw (engine_error()
           << "attempt to run check on service with NULL host");
  else if (!svc->check_command_ptr)
    throw (engine_error() << "attempt to run active check on service '"
           << svc->description << "' on host '" << svc->host_ptr->name
           << "' with no check command");
  logger(dbg_checks, basic)
    << "** Running async check of service '" << svc->description
    << "' on host '" << svc->host_name << "'...";

  // Check if the service is viable now.
  if (check_service_check_viability(
        svc,
        check_options,
        time_is_valid,
        preferred_time) == ERROR)
    throw (engine_error() << "check service viability failure.");

  // Send broker event.
  timeval start_time;
  timeval end_time;
  memset(&start_time, 0, sizeof(start_time));
  memset(&end_time, 0, sizeof(end_time));
  int res(broker_service_check(
            NEBTYPE_SERVICECHECK_ASYNC_PRECHECK,
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
            NULL));

  // Service check was cancel by NEB module. reschedule check later.
  if (NEBERROR_CALLBACKCANCEL == res) {
    if (preferred_time != NULL)
      *preferred_time += static_cast<time_t>(
                           svc->check_interval
                           * config.get_interval_length());
    throw (engine_error() << "broker callback cancel");
  }
  // Service check was override by NEB module.
  else if (NEBERROR_CALLBACKOVERRIDE == res)
    return ;

  // Checking starts.
  logger(dbg_checks, basic)
    << "Checking service '" << svc->description
    << "' on host '" << svc->host_name << "'...";

  // Clear check options.
  if (scheduled_check)
    svc->check_options = CHECK_OPTION_NONE;

  // Update latency for event broker and macros.
  double old_latency(svc->latency);
  svc->latency = latency;

  // Get current host and service macros.
  nagios_macros macros;
  memset(&macros, 0, sizeof(macros));
  grab_host_macros_r(&macros, svc->host_ptr);
  grab_service_macros_r(&macros, svc);
  get_raw_command_line_r(
    &macros,
    svc->check_command_ptr,
    svc->service_check_command,
    NULL,
    0);

  // Time to start command.
  gettimeofday(&start_time, NULL);

  // Update the number of running service checks.
  ++currently_running_service_checks;

  // Set the execution flag.
  svc->is_executing = true;

  // Init check result info.
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

  // Get command object.
  commands::set& cmd_set(commands::set::instance());
  QSharedPointer<commands::command>
    cmd(cmd_set.get_command(svc->check_command_ptr->name));
  QString processed_cmd(cmd->process_cmd(&macros));

  // Send event broker.
  res = broker_service_check(
          NEBTYPE_SERVICECHECK_INITIATE,
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
          QByteArray(qPrintable(processed_cmd)).data(),
          NULL);

  // Restore latency.
  svc->latency = old_latency;

  // Service check was override by neb_module.
  if (NEBERROR_CALLBACKOVERRIDE == res) {
    clear_volatile_macros_r(&macros);
    return ;
  }

  // Update statistics.
  update_check_stats(
    (scheduled_check == TRUE)
    ? ACTIVE_SCHEDULED_SERVICE_CHECK_STATS
    : ACTIVE_ONDEMAND_SERVICE_CHECK_STATS,
    start_time.tv_sec);

  // Change signal connection type.
  disconnect(
    &(*cmd),
    SIGNAL(command_executed(cce_commands_result const&)),
    this,
    SLOT(_command_executed(cce_commands_result const&)));
  connect(
    &(*cmd),
    SIGNAL(command_executed(cce_commands_result const&)),
    this,
    SLOT(_command_executed(cce_commands_result const&)),
    Qt::QueuedConnection);

  // Run command.
  unsigned long id(cmd->run(
                          processed_cmd,
                          macros,
                          config.get_service_check_timeout()));
  if (id != 0) {
    QMutexLocker lock(&_mut_id);
    _list_id[id] = check_result_info;
  }

  // Cleanup.
  clear_volatile_macros_r(&macros);
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
  return ;
}

/**
 *  Run an host check and wait check result.
 *
 *  @param[in]  hst                     Host to check.
 *  @param[out] check_result_code       The return value of the execution.
 *  @param[in]  check_options           Event options.
 *  @param[in]  use_cached_result       Used the last result.
 *  @param[in]  check_timestamp_horizon XXX
 */
void checker::run_sync(
                host* hst,
                int* check_result_code,
                int check_options,
                int use_cached_result,
                unsigned long check_timestamp_horizon) {
  // Preamble.
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;
  if (!hst)
    throw (engine_error() << "host pointer is NULL.");
  logger(dbg_checks, basic)
    << "** Run sync check of host '" << hst->name << "'...";

  // Check if the host is viable now.
  if (check_host_check_viability_3x(hst, check_options, NULL, NULL)
      == ERROR) {
    if (check_result_code)
      *check_result_code = hst->current_state;
    logger(dbg_checks, basic)
      << "Host check is not viable at this time.";
    return ;
  }

  // Time to start command.
  timeval start_time;
  gettimeofday(&start_time, NULL);

  // Can we use the last cached host state?
  if (use_cached_result
      && !(check_options & CHECK_OPTION_FORCE_EXECUTION)) {
    // We can used the cached result, so return it and get out of here.
    if (hst->has_been_checked
        && (static_cast<unsigned long>(
              start_time.tv_sec - hst->last_check)
            <= check_timestamp_horizon)) {
      if (check_result_code)
        *check_result_code = hst->current_state;
      logger(dbg_checks, more)
        << "* Using cached host state: " << hst->current_state;

      // Update statistics.
      update_check_stats(
        ACTIVE_ONDEMAND_HOST_CHECK_STATS,
        start_time.tv_sec);
      update_check_stats(
        ACTIVE_CACHED_HOST_CHECK_STATS,
        start_time.tv_sec);
      return ;
    }
  }

  // Checking starts.
  logger(dbg_checks, more)
    << "* Running actual host check: old state=" << hst->current_state;

  // Update statistics.
  update_check_stats(
    ACTIVE_ONDEMAND_HOST_CHECK_STATS,
    start_time.tv_sec);
  update_check_stats(SERIAL_HOST_CHECK_STATS, start_time.tv_sec);

  // Reset host check latency, since on-demand checks have none.
  hst->latency = 0.0;

  // Adjust check attempts.
  adjust_host_check_attempt_3x(hst, true);

  // Update host state.
  hst->last_state = hst->current_state;
  if (HARD_STATE == hst->state_type)
    hst->last_hard_state = hst->current_state;

  // Save old plugin output for state stalking.
  char* old_plugin_output(NULL);
  if (hst->plugin_output)
    old_plugin_output = my_strdup(hst->plugin_output);

  // Set the checked flag.
  hst->has_been_checked = true;

  // Clear the freshness flag.
  hst->is_being_freshened = false;

  // Clear check options - we don't want old check options retained.
  hst->check_options = CHECK_OPTION_NONE;

  // Set the check type.
  hst->check_type = HOST_CHECK_ACTIVE;

  // Send broker event.
  timeval end_time;
  memset(&end_time, 0, sizeof(end_time));
  broker_host_check(
    NEBTYPE_HOSTCHECK_INITIATE,
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

  // Execute command synchronously.
  int host_result(_execute_sync(hst));

  // Process result.
  process_host_check_result_3x(
    hst,
    host_result,
    old_plugin_output,
    check_options,
    false,
    use_cached_result,
    check_timestamp_horizon);
  if (check_result_code)
    *check_result_code = hst->current_state;

  // Cleanup.
  delete [] old_plugin_output;

  // Synchronous check is done.
  logger(dbg_checks, more)
    << "* Sync host check done: new state=" << hst->current_state;

  // Get the end time of command.
  gettimeofday(&end_time, NULL);

  // Send event broker.
  broker_host_check(
    NEBTYPE_HOSTCHECK_PROCESSED,
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

  // End.
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
  return ;
}

/**
 *  Unload singleton.
 */
void checker::unload() {
  _instance.reset();
  return ;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Slot to catch the result of the execution and add to the reap queue.
 *
 *  @param[in] res The result of the execution.
 */
void checker::_command_executed(cce_commands_result const& res) {
  // Debug message.
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;

  // Find check result.
  check_result result;
  {
    QMutexLocker lock(&_mut_id);
    QHash<unsigned long, check_result>::iterator
      it(_list_id.find(res.get_command_id()));
    if (_list_id.end() == it) {
      lock.unlock();
      logger(log_runtime_warning, basic)
        << "command ID '" << res.get_command_id() << "' not found";
      return ;
    }

    // Check result was found.
    result = it.value();
    _list_id.erase(it);
  }
  logger(dbg_checks, basic)
    << "command ID (" << res.get_command_id() << ") executed";

  // Update check result.
  result.finish_time = res.get_end_time();
  result.early_timeout = res.get_is_timeout();
  result.return_code = res.get_exit_code();
  result.exited_ok = res.get_is_executed();
  if (res.get_is_executed() && !res.get_is_timeout())
    result.output = my_strdup(qPrintable(res.get_stdout()));
  else
    result.output = my_strdup(qPrintable(res.get_stderr()));

  // Queue check result.
  {
    QMutexLocker lock(&_mut_reap);
    _to_reap.enqueue(result);
  }

  // Debug message.
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
  return ;
}

/**
 *  Default constructor.
 */
checker::checker() : _mut_id(QMutex::Recursive) {
  qRegisterMetaType<commands::result>("cce_commands_result");
}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
checker::checker(checker const& right) : QObject() {
  _internal_copy(right);
}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
checker& checker::operator=(checker const& right) {
  _internal_copy(right);
  return (*this);
}

/**
 *  Run an host check with waiting check result.
 *
 *  @param[in] hst The host to check.
 *
 *  @result Return if the host is up (HOST_UP) or host down (HOST_DOWN).
 */
int checker::_execute_sync(host* hst) {
  // Preamble.
  logger(dbg_functions, basic) << "start " << Q_FUNC_INFO;
  if (!hst)
    throw (engine_error() << "host pointer is NULL.");
  logger(dbg_checks, basic)
    << "** Executing sync check of host '" << hst->name << "'...";

  // Send broker event.
  timeval start_time;
  timeval end_time;
  memset(&start_time, 0, sizeof(start_time));
  memset(&end_time, 0, sizeof(end_time));
  int res(broker_host_check(
            NEBTYPE_HOSTCHECK_SYNC_PRECHECK,
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
            NULL));

  // Host sync check was cancelled or overriden by NEB module.
  if ((NEBERROR_CALLBACKCANCEL == res)
      || (NEBERROR_CALLBACKOVERRIDE == res))
    return (hst->current_state);

  // Get current host macros.
  nagios_macros macros;
  memset(&macros, 0, sizeof(macros));
  grab_host_macros_r(&macros, hst);
  get_raw_command_line_r(
    &macros,
    hst->check_command_ptr,
    hst->host_check_command,
    NULL,
    0);

  // Time to start command.
  gettimeofday(&start_time, NULL);

  // Update last host check.
  hst->last_check = start_time.tv_sec;

  // Get command object.
  commands::set& cmd_set(commands::set::instance());
  QSharedPointer<commands::command>
    cmd(cmd_set.get_command(hst->check_command_ptr->name));
  QString processed_cmd(cmd->process_cmd(&macros));
  char* tmp_processed_cmd(my_strdup(qPrintable(processed_cmd)));

  // Send broker event.
  broker_host_check(
    NEBTYPE_HOSTCHECK_RAW_START,
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

  // Debug messages.
  logger(dbg_commands, more)
    << "Raw host check command: " << hst->check_command_ptr->command_line;
  logger(dbg_commands, more)
    << "Processed host check ommand: " << processed_cmd;

  // Cleanup.
  delete [] hst->plugin_output;
  delete [] hst->long_plugin_output;
  delete [] hst->perf_data;
  hst->plugin_output = NULL;
  hst->long_plugin_output = NULL;
  hst->perf_data = NULL;

  // Send broker event.
  timeval start_cmd;
  timeval end_cmd;
  gettimeofday(&start_cmd, NULL);
  memset(&end_cmd, 0, sizeof(end_cmd));
  broker_system_command(
    NEBTYPE_SYSTEM_COMMAND_START,
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

  // Run command.
  commands::result cmd_result;
  cmd->run(
         processed_cmd,
         macros,
         config.get_host_check_timeout(),
         cmd_result);

  // Get output.
  char* output(NULL);
  if (cmd_result.get_is_executed())
    output = my_strdup(qPrintable(cmd_result.get_stdout()));
  else
    output = my_strdup(qPrintable(cmd_result.get_stderr()));

  // Send broker event.
  broker_system_command(
    NEBTYPE_SYSTEM_COMMAND_END,
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

  // Cleanup.
  delete [] output;
  clear_volatile_macros_r(&macros);

  // If the command timed out.
  if (cmd_result.get_is_timeout()) {
    QString output("Host check timed out after %1  seconds");
    output.arg(config.get_host_check_timeout());
    cmd_result.set_stdout(output);
    logger(log_runtime_warning, basic)
      << "Warning: Host check command '" << processed_cmd
      << "' for host '" << hst->name << "' timed out after "
      << config.get_host_check_timeout() << " seconds";
  }

  // Update values.
  hst->execution_time = cmd_result.get_execution_time();
  hst->check_type = HOST_CHECK_ACTIVE;

  // Get plugin output.
  char* tmp_plugin_output(NULL);
  if (cmd_result.get_is_executed())
    tmp_plugin_output = my_strdup(qPrintable(cmd_result.get_stdout()));
  else
    tmp_plugin_output = my_strdup(qPrintable(cmd_result.get_stderr()));

  // Parse the output: short and long output, and perf data.
  parse_check_output(
    tmp_plugin_output,
    &hst->plugin_output,
    &hst->long_plugin_output,
    &hst->perf_data,
    true,
    true);
  delete [] tmp_plugin_output;

  // A NULL host check command means we should assume the host is UP.
  if (!hst->host_check_command) {
    delete [] hst->plugin_output;
    hst->plugin_output = my_strdup("(Host assumed to be UP)");
    cmd_result.set_exit_code(STATE_OK);
  }

  // Make sure we have some data.
  if (!hst->plugin_output || !strcmp(hst->plugin_output, "")) {
    delete [] hst->plugin_output;
    hst->plugin_output
      = my_strdup("(No output returned from host check)");
  }

  // Replace semicolons with colons in plugin output
  // (but not performance data).
  if (hst->plugin_output)
    for (char* ptr = hst->plugin_output; (ptr = strchr(ptr, ';')); *ptr = ':')
      ;

  // If we're not doing aggressive host checking, let WARNING
  // states indicate the host is up (fake the result to be STATE_OK).
  if (!config.get_use_aggressive_host_checking()
      && (cmd_result.get_exit_code() == STATE_WARNING))
    cmd_result.set_exit_code(STATE_OK);

  // Get host state from plugin exit code.
  int return_result(
        (cmd_result.get_exit_code() == STATE_OK)
        ? HOST_UP
        : HOST_DOWN);

  // Get the end time of command.
  gettimeofday(&end_time, NULL);

  // Send broker event.
  broker_host_check(
    NEBTYPE_HOSTCHECK_RAW_END,
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
  delete [] tmp_processed_cmd;

  // Termination.
  logger(dbg_checks, basic)
    << "** Sync host check done: state=" << return_result;
  logger(dbg_functions, basic) << "end " << Q_FUNC_INFO;
  return (return_result);
}

/**
 *  Copy internal data members.
 *
 *  @param[in] right Object to copy.
 */
void checker::_internal_copy(checker const& right) {
  (void)right;
  assert(!"checker is not copyable");
  abort();
  return ;
}

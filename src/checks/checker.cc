/*
** Copyright 1999-2010 Ethan Galstad
** Copyright 2011-2019 Centreon
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

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <sys/time.h>
#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/exceptions/interruption.hh"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/checks/viability_failure.hh"
#include "com/centreon/engine/commands/command.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::checks;

// Class instance.
static checker* _instance = nullptr;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Get instance of the checker singleton.
 *
 *  @return This singleton.
 */
checker& checker::instance() {
  return *_instance;
}

/**
 *  Load singleton.
 */
void checker::load() {
  if (!_instance)
    _instance = new checker;
  return;
}

/**
 *  Add into the queue a result to reap later.
 *
 *  @param[in] result The check_result to process later.
 */
void checker::push_check_result(check_result const& result) {
  concurrency::locker lock(&_mut_reap);
  _to_reap.push(result);
  return;
}

/**
 *  Reap and process all result received by execution process.
 */
void checker::reap() {
  logger(dbg_functions, basic)
    << "checker::reap";
  logger(dbg_checks, basic)
    << "Starting to reap check results.";

  // Time to start reaping.
  time_t reaper_start_time;
  time(&reaper_start_time);

  if (config->use_check_result_path()) {
    std::string const& path(config->check_result_path());
    check_result::process_check_result_queue(path);
  }

  // Keep compatibility with old check result list.
  if (!check_result::results.empty()) {
    concurrency::locker lock(&_mut_reap);
    check_result* cr(nullptr);
    for (check_result_list::iterator
           it(check_result::results.begin()),
           end(check_result::results.end());
         it != end;
         ++it) {
      _to_reap.push(*it);
      delete cr;
    }
  }

  // Reap check results.
  unsigned int reaped_checks(0);
  { // Scope to release mutex in all termination cases.
    concurrency::locker lock(&_mut_reap);

    // Merge partial check results.
    while (!_to_reap_partial.empty()) {
      // Find the two parts.
      umap<unsigned long, check_result>::iterator
        it_partial(_to_reap_partial.begin());
      umap<unsigned long, check_result>::iterator
        it_id(_list_id.find(it_partial->first));
      if (_list_id.end() == it_id) {
        logger(log_runtime_warning, basic)
          << "command ID '" << it_partial->first << "' not found";
      }
      else {
        // Extract base part.
        logger(dbg_checks, basic)
          << "command ID (" << it_partial->first << ") executed";
        check_result result;
        result = it_id->second;
        _list_id.erase(it_id);

        // Merge check result.
        result.set_finish_time(it_partial->second.get_finish_time());
        result.set_early_timeout(it_partial->second.get_early_timeout());
        result.set_return_code(it_partial->second.get_return_code());
        result.set_exited_ok(it_partial->second.get_exited_ok());
        result.set_output(it_partial->second.get_output());

        // Push back in reap list.
        _to_reap.push(result);
      }
      _to_reap_partial.erase(it_partial);
    }

    // Process check results.
    while (!_to_reap.empty()) {
      // Get result host or service check.
      logger(dbg_checks, basic)
        << "Found a check result (#" << ++reaped_checks
        << ") to handle...";
      check_result result(_to_reap.front());
      _to_reap.pop();
      lock.unlock();

      // Service check result.
      if (service_check == result.get_object_check_type()) {
        service_map::iterator
          it = service::services.find({result.get_hostname(),
                                       result.get_service_description()});
        if (it == service::services.end()) {
          logger(log_runtime_error, basic)
            << "Warning: Check result queue contained results for "
            << "service '" << result.get_service_description() << "' on "
            << "host '" << result.get_hostname() << "', but the service "
            << "could not be found! Perhaps you forgot to define the "
            << "service in your config files ?";
        }
        try {
          // Check if the service exists.
          logger(dbg_checks, more)
            << "Handling check result for service '"
            << result.get_service_description() << "' on host '"
            << result.get_hostname() << "'...";
          it->second->handle_async_check_result(&result);
        }
        catch (std::exception const &e) {
          logger(log_runtime_warning, basic)
            << "Check result queue errors for "
            << "host '" << result.get_hostname()
            << "' service '"  << result.get_service_description() << "' ex: "
            << e.what();
        }
      }
      // Host check result.
      else {
        host_map::iterator it = host::hosts.find(result.get_hostname());
        if (it == host::hosts.end())
          logger(log_runtime_warning, basic)
            << "Warning: Check result queue contained results for "
            << "host '" << result.get_hostname() << "', but the host could "
            << "not be found! Perhaps you forgot to define the host in "
            << "your config files ?";
        else {
          try {
            // Process the check result.
            logger(dbg_checks, more)
              << "Handling check result for host '"
              << result.get_hostname() << "'...";
            it->second->handle_async_check_result_3x(&result);
          }
          catch (std::exception const &e) {
            logger(log_runtime_error, basic)
              << "Check result queue errors for "
              << "host '" << result.get_hostname() << "' ex : "
              << e.what();
          }
        }
      }

      // Check if reaping has timed out.
      time_t current_time;
      time(&current_time);
      if ((current_time - reaper_start_time)
          > static_cast<time_t>(config->max_check_reaper_time())) {
        logger(dbg_checks, basic)
          << "Breaking out of check result reaper: "
          << "max reaper time exceeded";
        break;
      }

      // Caught signal, need to break.
      if (sigshutdown) {
        logger(dbg_checks, basic)
          << "Breaking out of check result reaper: signal encountered";
        break;
      }

      // Mutex needed to access list.
      lock.relock();
    }
  }

  // Reaping finished.
  logger(dbg_checks, basic)
    << "Finished reaping " << reaped_checks << " check results";
}

/**
 *  Check if the reaper queue is empty.
 *
 *  @return True if the reper queue is empty, otherwise false.
 */
bool checker::reaper_is_empty() {
  concurrency::locker lock(&_mut_reap);
  return _to_reap.empty();
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
  logger(dbg_functions, basic)
    << "checker::run: hst=" << hst
    << ", check_options=" << check_options
    << ", latency=" << latency
    << ", scheduled_check=" << scheduled_check
    << ", reschedule_check=" << reschedule_check;

  // Preamble.
  if (!hst)
    throw (engine_error() << "Attempt to run check on invalid host");
  if (!hst->get_check_command_ptr())
    throw (engine_error() << "Attempt to run active check on host '"
           << hst->get_name() << "' with no check command");

  logger(dbg_checks, basic)
    << "** Running async check of host '" << hst->get_name() << "'...";

  // Check if the host is viable now.
  if (hst->verify_check_viability(
        check_options,
        time_is_valid,
        preferred_time) == ERROR)
    throw (checks_viability_failure() << "Check of host '" << hst->get_name()
           << "' is not viable");

  // If this check is a rescheduled check, propagate the rescheduled check flag
  // to the host. This solves the problem when a new host check is
  // bound to be rescheduled but would be discarded because a host check
  // is already running.
  if (reschedule_check)
    host::hosts[hst->get_name()]->set_should_reschedule_current_check(true);

  // Don't execute a new host check if one is already running.
  if (hst->get_is_executing()
        && !(check_options & CHECK_OPTION_FORCE_EXECUTION)) {
    logger(dbg_checks, basic)
      << "A check of this host (" << hst->get_name()
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
            checkable::check_active,
            hst->get_current_state(),
            hst->get_state_type(),
            start_time,
            end_time,
            hst->get_check_command().c_str(),
            hst->get_latency(),
            0.0,
            config->host_check_timeout(),
            false,
            0,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr));

  // Host check was cancel by NEB module. Reschedule check later.
  if (NEBERROR_CALLBACKCANCEL == res)
    throw (engine_error()
           << "Some broker module cancelled check of host '"
           << hst->get_name() << "'");
  // Host check was overriden by NEB module.
  else if (NEBERROR_CALLBACKOVERRIDE == res) {
    logger(dbg_functions, basic)
           << "Some broker module overrode check of host '"
           << hst->get_name() << "' so we'll bail out";
    return ;
  }

  // Checking starts.
  logger(dbg_functions, basic)
    << "Checking host '" << hst->get_name() << "'...";

  // Clear check options.
  if (scheduled_check)
    hst->set_check_options(CHECK_OPTION_NONE);

  // Adjust check attempts.
  hst->adjust_check_attempt(true);

  // Update latency for event broker and macros.
  double old_latency(hst->get_latency());
  hst->set_latency(latency);

  // Get current host macros.
  nagios_macros macros;
  grab_host_macros_r(&macros, hst);
  std::string tmp;
  get_raw_command_line_r(
    &macros,
    hst->get_check_command_ptr(),
    hst->get_check_command().c_str(),
    tmp,
    0);

  // Time to start command.
  gettimeofday(&start_time, nullptr);

  // Set check time for on-demand checks, so they're
  // not incorrectly detected as being orphaned.
  if (!scheduled_check)
    hst->set_next_check(start_time.tv_sec);

  // Update the number of running host checks.
  ++currently_running_host_checks;

  // Set the execution flag.
  hst->set_is_executing(true);

  // Init check result info.
  check_result check_result_info(host_check,
                                 hst->get_name(),
                                 "",
                                 checkable::check_active,
                                 check_options,
                                 reschedule_check,
                                 latency,
                                 start_time,
                                 start_time,
                                 false,
                                 true,
                                 notifier::ok,
                                 "");

  // Get command object.
  command_map::iterator found{
    commands::command::commands.find(hst->get_check_command_ptr()->get_name())};
   if(found == commands::command::commands.end() || !found->second)
     throw (engine_error() << "unknow command " << hst->get_check_command_ptr()->get_name());

  std::string processed_cmd(found->second->process_cmd(&macros));
  char* processed_cmd_ptr(string::dup(processed_cmd));

  // Send event broker.
  broker_host_check(
    NEBTYPE_HOSTCHECK_INITIATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    checkable::check_active,
    hst->get_current_state(),
    hst->get_state_type(),
    start_time,
    end_time,
    hst->get_check_command().c_str(),
    hst->get_latency(),
    0.0,
    config->host_check_timeout(),
    false,
    0,
    processed_cmd_ptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr);

  delete[] processed_cmd_ptr;

  // Restore latency.
  hst->set_latency(old_latency);

  // Update statistics.
  update_check_stats(
    (scheduled_check == true)
    ? ACTIVE_SCHEDULED_HOST_CHECK_STATS
    : ACTIVE_ONDEMAND_HOST_CHECK_STATS,
    start_time.tv_sec);
  update_check_stats(PARALLEL_HOST_CHECK_STATS, start_time.tv_sec);

  // Run command.
  bool retry;
  do {
    retry = false;
    try {
      // Run command.
      uint64_t id(found->second->run(
                              processed_cmd,
                              macros,
                              config->host_check_timeout()));
      if (id != 0)
        _list_id[id] = check_result_info;
    }
    catch (com::centreon::exceptions::interruption const& e) {
      (void)e;
      retry = true;
    }
    catch (std::exception const& e) {
      timestamp now(timestamp::now());

      // Update check result.
      struct timeval tv;
      tv.tv_sec = now.to_seconds();
      tv.tv_usec = now.to_useconds() - check_result_info.get_finish_time().tv_sec * 1000000ull;
      check_result_info.set_finish_time(tv);
      check_result_info.set_early_timeout(false);
      check_result_info.set_return_code(notifier::unknown);
      check_result_info.set_exited_ok(true);
      check_result_info.set_output("(Execute command failed)");

      // Queue check result.
      concurrency::locker lock(&_mut_reap);
      _to_reap.push(check_result_info);

      logger(log_runtime_warning, basic)
        << "Error: Host check command execution failed: " << e.what();
    }
  } while (retry);

  // Cleanup.
  clear_volatile_macros_r(&macros);
  return;
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
  logger(dbg_functions, basic)
    << "checker::run: svc=" << svc
    << ", check_options=" << check_options
    << ", latency=" << latency
    << ", scheduled_check=" << scheduled_check
    << ", reschedule_check=" << reschedule_check;

  // Preamble.
  if (!svc)
    throw (engine_error() << "Attempt to run check on invalid service");
  if (!svc->get_host_ptr())
    throw engine_error()
           << "Attempt to run check on service with invalid host";
  if (!svc->get_check_command_ptr())
    throw engine_error() << "Attempt to run active check on service '"
           << svc->get_description() << "' on host '" << svc->get_host_ptr()->get_name()
           << "' with no check command";

  logger(dbg_checks, basic)
    << "** Running async check of service '" << svc->get_description()
    << "' on host '" << svc->get_hostname() << "'...";

  // Check if the service is viable now.
  if (svc->verify_check_viability(
        check_options,
        time_is_valid,
        preferred_time) == ERROR)
    throw (checks_viability_failure() << "Check of service '"
           << svc->get_description() << "' on host '" << svc->get_hostname()
           << "' is not viable");

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
            checkable::check_active,
            start_time,
            end_time,
            svc->get_check_command().c_str(),
            svc->get_latency(),
            0.0,
            0,
            false,
            0,
            nullptr,
            nullptr));

  // Service check was cancel by NEB module. reschedule check later.
  if (NEBERROR_CALLBACKCANCEL == res) {
    if (preferred_time != nullptr)
      *preferred_time += static_cast<time_t>(
                           svc->get_check_interval()
                           * config->interval_length());
    throw (engine_error()
           << "Some broker module cancelled check of service '"
           << svc->get_description() << "' on host '" << svc->get_hostname());
  }
  // Service check was override by NEB module.
  else if (NEBERROR_CALLBACKOVERRIDE == res) {
    logger(dbg_functions, basic)
      << "Some broker module overrode check of service '"
      << svc->get_description() << "' on host '" << svc->get_hostname()
      << "' so we'll bail out";
    return ;
  }

  // Checking starts.
  logger(dbg_checks, basic)
    << "Checking service '" << svc->get_description()
    << "' on host '" << svc->get_hostname() << "'...";

  // Clear check options.
  if (scheduled_check)
    svc->check_options = CHECK_OPTION_NONE;

  // Update latency for event broker and macros.
  double old_latency(svc->get_latency());
  svc->set_latency(latency);

  // Get current host and service macros.
  nagios_macros macros;
  grab_host_macros_r(&macros, svc->get_host_ptr());
  grab_service_macros_r(&macros, svc);
  std::string tmp;
  get_raw_command_line_r(
    &macros,
    svc->get_check_command_ptr(),
    svc->get_check_command().c_str(),
    tmp,
    0);

  // Time to start command.
  gettimeofday(&start_time, nullptr);

  // Update the number of running service checks.
  ++currently_running_service_checks;

  // Set the execution flag.
  svc->set_is_executing(true);

  // Init check result info.
  check_result check_result_info(service_check,
                                 svc->get_hostname(),
                                 svc->get_description(),
                                 checkable::check_active,
                                 check_options,
                                 reschedule_check,
                                 latency,
                                 start_time,
                                 start_time,
                                 false,
                                 true,
                                 notifier::ok,
                                 "");

  // Get command object.
  command_map::iterator found{
    commands::command::commands.find(svc->get_check_command_ptr()->get_name())};
  if(found == commands::command::commands.end() || !found->second)
    throw (engine_error() << "unknow command " << svc->get_check_command_ptr()->get_name());

  std::string processed_cmd(found->second->process_cmd(&macros));
  char* processed_cmd_ptr(string::dup(processed_cmd));

  // Send event broker.
  res = broker_service_check(
          NEBTYPE_SERVICECHECK_INITIATE,
          NEBFLAG_NONE,
          NEBATTR_NONE,
          svc,
          checkable::check_active,
          start_time,
          end_time,
          svc->get_check_command().c_str(),
          svc->get_latency(),
          0.0,
          config->service_check_timeout(),
          false,
          0,
          processed_cmd_ptr,
          nullptr);
  delete[] processed_cmd_ptr;

  // Restore latency.
  svc->set_latency(old_latency);

  // Service check was override by neb_module.
  if (NEBERROR_CALLBACKOVERRIDE == res) {
    clear_volatile_macros_r(&macros);
    return;
  }

  // Update statistics.
  update_check_stats(
    (scheduled_check == true)
    ? ACTIVE_SCHEDULED_SERVICE_CHECK_STATS
    : ACTIVE_ONDEMAND_SERVICE_CHECK_STATS,
    start_time.tv_sec);

  bool retry;
  do {
    retry = false;
    try {
      // Run command.
      uint64_t id(found->second->run(
                              processed_cmd,
                              macros,
                              config->service_check_timeout()));
      if (id != 0)
        _list_id[id] = check_result_info;
    }
    catch (com::centreon::exceptions::interruption const& e) {
      (void)e;
      retry = true;
    }
    catch (std::exception const& e) {
      timestamp now(timestamp::now());

      // Update check result.
      timeval tv;
      tv.tv_sec = now.to_seconds();
      tv.tv_usec = now.to_useconds()
        - check_result_info.get_finish_time().tv_sec * 1000000ull;
      check_result_info.set_finish_time(tv);

      check_result_info.set_early_timeout(false);
      check_result_info.set_return_code(notifier::unknown);
      check_result_info.set_exited_ok(true);
      check_result_info.set_output("(Execute command failed)");

      // Queue check result.
      concurrency::locker lock(&_mut_reap);
      _to_reap.push(check_result_info);

      logger(log_runtime_warning, basic)
        << "Error: Service check command execution failed: " << e.what();
    }
  } while (retry);

  // Cleanup.
  clear_volatile_macros_r(&macros);
  return;
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
                host::host_state * check_result_code,
                int check_options,
                int use_cached_result,
                unsigned long check_timestamp_horizon) {
  logger(dbg_functions, basic)
    << "checker::run: hst=" << hst
    << ", check_options=" << check_options
    << ", use_cached_result=" << use_cached_result
    << ", check_timestamp_horizon=" << check_timestamp_horizon;

  // Preamble.
  if (!hst)
    throw (engine_error()
           << "Attempt to run synchronous check on invalid host");
  if (!hst->get_check_command_ptr())
    throw (engine_error()
           << "Attempt to run synchronous active check on host '"
           << hst->get_name() << "' with no check command");

  logger(dbg_checks, basic)
    << "** Run sync check of host '" << hst->get_name() << "'...";

  // Check if the host is viable now.
  if (hst->verify_check_viability(check_options, nullptr, nullptr)
      == ERROR) {
    if (check_result_code)
      *check_result_code = hst->get_current_state();
    logger(dbg_checks, basic)
      << "Host check is not viable at this time";
    return ;
  }

  // Time to start command.
  timeval start_time;
  gettimeofday(&start_time, nullptr);

  // Can we use the last cached host state?
  if (use_cached_result
      && !(check_options & CHECK_OPTION_FORCE_EXECUTION)) {
    // We can used the cached result, so return it and get out of here.
    if (hst->get_has_been_checked()
        && (static_cast<unsigned long>(
              start_time.tv_sec - hst->get_last_check())
            <= check_timestamp_horizon)) {
      if (check_result_code)
        *check_result_code = hst->get_current_state();
      logger(dbg_checks, more)
        << "* Using cached host state: " << hst->get_current_state();

      // Update statistics.
      update_check_stats(
        ACTIVE_ONDEMAND_HOST_CHECK_STATS,
        start_time.tv_sec);
      update_check_stats(
        ACTIVE_CACHED_HOST_CHECK_STATS,
        start_time.tv_sec);
      return;
    }
  }

  // Checking starts.
  logger(dbg_checks, more)
    << "* Running actual host check: old state=" << hst->get_current_state();

  // Update statistics.
  update_check_stats(
    ACTIVE_ONDEMAND_HOST_CHECK_STATS,
    start_time.tv_sec);
  update_check_stats(SERIAL_HOST_CHECK_STATS, start_time.tv_sec);

  // Reset host check latency, since on-demand checks have none.
  hst->set_latency(0.0);

  // Adjust check attempts.
  hst->adjust_check_attempt(true);

  // Update host state.
  hst->set_last_state(hst->get_current_state());
  if (notifier::hard == hst->get_state_type())
    hst->set_last_hard_state(hst->get_current_state());

  // Save old plugin output for state stalking.
  char* old_plugin_output(string::dup(hst->get_plugin_output().c_str()));

  // Set the checked flag.
  hst->set_has_been_checked(true);

  // Clear the freshness flag.
  hst->set_is_being_freshened(false);

  // Clear check options - we don't want old check options retained.
  hst->set_check_options(CHECK_OPTION_NONE);

  // Set the check type.
  hst->set_check_type(checkable::check_active);

  // Send broker event.
  timeval end_time;
  memset(&end_time, 0, sizeof(end_time));
  broker_host_check(
    NEBTYPE_HOSTCHECK_INITIATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    checkable::check_active,
    hst->get_current_state(),
    hst->get_state_type(),
    start_time,
    end_time,
    hst->get_check_command().c_str(),
    hst->get_latency(),
    0.0,
    config->host_check_timeout(),
    false,
    0,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr);

  // Execute command synchronously.
  host::host_state host_result(_execute_sync(hst));

  // Process result.
  hst->process_check_result_3x(
    host_result,
    old_plugin_output,
    check_options,
    false,
    use_cached_result,
    check_timestamp_horizon);
  if (check_result_code)
    *check_result_code = hst->get_current_state();

  // Cleanup.
  delete[] old_plugin_output;

  // Synchronous check is done.
  logger(dbg_checks, more)
    << "* Sync host check done: new state=" << hst->get_current_state();

  // Get the end time of command.
  gettimeofday(&end_time, nullptr);

  // Send event broker.
  broker_host_check(
    NEBTYPE_HOSTCHECK_PROCESSED,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    checkable::check_active,
    hst->get_current_state(),
    hst->get_state_type(),
    start_time,
    end_time,
    hst->get_check_command().c_str(),
    hst->get_latency(),
    hst->get_execution_time(),
    config->host_check_timeout(),
    false,
    hst->get_current_state(),
    nullptr,
    const_cast<char*>(hst->get_plugin_output().c_str()),
    const_cast<char*>(hst->get_long_plugin_output().c_str()),
    const_cast<char*>(hst->get_perf_data().c_str()),
    nullptr);
}

/**
 *  Unload singleton.
 */
void checker::unload() {
  delete _instance;
  _instance = nullptr;
  return;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
checker::checker()
  : commands::command_listener() {

}

/**
 *  Default destructor.
 */
checker::~checker() throw () {
  try {
    concurrency::locker lock(&_mut_reap);
    while (!_to_reap.empty()) {
      _to_reap.pop();
    }
  }
  catch (...) {}
}

/**
 *  Slot to catch the result of the execution and add to the reap queue.
 *
 *  @param[in] res The result of the execution.
 */
void checker::finished(commands::result const& res) throw () {
  // Debug message.
  logger(dbg_functions, basic)
    << "checker::finished: res=" << &res;

  // Find check result.
  check_result result;

  struct timeval tv;
  // Update check result.
  tv.tv_sec = res.end_time.to_seconds();
  tv.tv_usec = res.end_time.to_useconds()- result.get_finish_time().tv_sec * 1000000ull;
  result.set_finish_time(tv);
  result.set_early_timeout(res.exit_status == process::timeout);
  result.set_return_code(res.exit_code);
  result.set_exited_ok((res.exit_status == process::normal)
                      || (res.exit_status == process::timeout));
  result.set_output(res.output);

  // Queue check result.
  concurrency::locker lock(&_mut_reap);
  _to_reap_partial[res.command_id] = result;
}

/**
 *  Run an host check with waiting check result.
 *
 *  @param[in] hst The host to check.
 *
 *  @result Return if the host is up ( notifier::state_up) or host down ( notifier::state_down).
 */
com::centreon::engine::host::host_state checker::_execute_sync(host* hst) {
  logger(dbg_functions, basic)
    << "checker::_execute_sync: hst=" << hst;

  // Preamble.
  if (!hst)
    throw (engine_error()
           << "Attempt to run synchronous check on invalid host");
  if (!hst->get_check_command_ptr())
    throw (engine_error()
           << "Attempt to run synchronous active check on host '"
           << hst->get_name() << "' with no check command");

  logger(dbg_checks, basic)
    << "** Executing sync check of host '" << hst->get_name() << "'...";

  // Send broker event.
  timeval start_time;
  timeval end_time;
  memset(&start_time, 0, sizeof(start_time));
  memset(&end_time, 0, sizeof(end_time));
  int ret(broker_host_check(
            NEBTYPE_HOSTCHECK_SYNC_PRECHECK,
            NEBFLAG_NONE,
            NEBATTR_NONE,
            hst,
            checkable::check_active,
            hst->get_current_state(),
            hst->get_state_type(),
            start_time,
            end_time,
            hst->get_check_command().c_str(),
            hst->get_latency(),
            0.0,
            config->host_check_timeout(),
            false,
            0,
            nullptr,
            nullptr,
            nullptr,
            nullptr,
            nullptr));

  // Host sync check was cancelled or overriden by NEB module.
  if ((NEBERROR_CALLBACKCANCEL == ret)
      || (NEBERROR_CALLBACKOVERRIDE == ret))
    return hst->get_current_state();

  // Get current host macros.
  nagios_macros macros;
  grab_host_macros_r(&macros, hst);
  std::string tmp;
  get_raw_command_line_r(
    &macros,
    hst->get_check_command_ptr(),
    hst->get_check_command().c_str(),
    tmp,
    0);

  // Time to start command.
  gettimeofday(&start_time, nullptr);

  // Update last host check.
  hst->set_last_check(start_time.tv_sec);

  // Get command object.
  command_map::iterator found{
    commands::command::commands.find(hst->get_check_command_ptr()->get_name())};
  if(found == commands::command::commands.end() || !found->second)
    throw (engine_error() << "unknow command " << hst->get_check_command_ptr()->get_name());

  std::string processed_cmd(found->second->process_cmd(&macros));
  char* tmp_processed_cmd(string::dup(processed_cmd));

  // Send broker event.
  broker_host_check(
    NEBTYPE_HOSTCHECK_RAW_START,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    checkable::check_active,
    host::state_up,
    hst->get_state_type(),
    start_time,
    end_time,
    hst->get_check_command().c_str(),
    0.0,
    0.0,
    config->host_check_timeout(),
    false,
    notifier::ok,
    tmp_processed_cmd,
    const_cast<char*>(hst->get_plugin_output().c_str()),
    const_cast<char*>(hst->get_long_plugin_output().c_str()),
    const_cast<char*>(hst->get_perf_data().c_str()),
    nullptr);

  // Debug messages.
  logger(dbg_commands, more)
    << "Raw host check command: " << hst->get_check_command_ptr()->get_command_line();
  logger(dbg_commands, more)
    << "Processed host check ommand: " << processed_cmd;

  // Cleanup.
  hst->set_plugin_output("");
  hst->set_long_plugin_output("");
  hst->set_perf_data("");

  // Send broker event.
  timeval start_cmd;
  timeval end_cmd;
  gettimeofday(&start_cmd, nullptr);
  memset(&end_cmd, 0, sizeof(end_cmd));
  broker_system_command(
    NEBTYPE_SYSTEM_COMMAND_START,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    start_cmd,
    end_cmd,
    0,
    config->host_check_timeout(),
    false,
    0,
    tmp_processed_cmd,
    nullptr,
    nullptr);

  // Run command.
  commands::result res;
  try {
    found->second->run(
           processed_cmd,
           macros,
           config->host_check_timeout(),
           res);
  }
  catch (std::exception const& e) {
    // Update check result.
    res.command_id = 0;
    res.end_time = timestamp::now();
    res.exit_code = notifier::unknown;
    res.exit_status = process::normal;
    res.output = "(Execute command failed)";
    res.start_time = res.end_time;

    logger(log_runtime_warning, basic)
      << "Error: Synchronous host check command execution failed: "
      << e.what();
  }

  // Get output.
  char* output(string::dup(res.output));

  unsigned int execution_time(0);
  if (res.end_time >= res.start_time)
    execution_time
      = res.end_time.to_seconds() - res.start_time.to_seconds();

  // Send broker event.
  memset(&start_cmd, 0, sizeof(start_time));
  start_cmd.tv_sec = res.start_time.to_seconds();
  start_cmd.tv_usec
    = res.start_time.to_useconds() - start_cmd.tv_sec * 1000000ull;
  memset(&end_cmd, 0, sizeof(end_time));
  end_cmd.tv_sec = res.end_time.to_seconds();
  end_cmd.tv_usec =
    res.end_time.to_useconds() - end_cmd.tv_sec * 1000000ull;
  broker_system_command(
    NEBTYPE_SYSTEM_COMMAND_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    start_cmd,
    end_cmd,
    execution_time,
    config->host_check_timeout(),
    res.exit_status == process::timeout,
    res.exit_code,
    tmp_processed_cmd,
    output,
    nullptr);

  // Cleanup.
  delete[] output;
  clear_volatile_macros_r(&macros);

  // If the command timed out.
  if (res.exit_status == process::timeout) {
    std::ostringstream oss;
    oss << "Host check timed out after "
        << config->host_check_timeout()
        << "  seconds";
    res.output = oss.str();
    logger(log_runtime_warning, basic)
      << "Warning: Host check command '" << processed_cmd
      << "' for host '" << hst->get_name() << "' timed out after "
      << config->host_check_timeout() << " seconds";
  }

  // Update values.
  hst->set_execution_time(execution_time);
  hst->set_check_type(checkable::check_active);

  // Get plugin output.
  std::string pl_output;
  std::string lpl_output;
  std::string perfdata_output;

  // Parse the output: short and long output, and perf data.
  parse_check_output(
    res.output,
    pl_output,
    lpl_output,
    perfdata_output,
    true,
    true);

  hst->set_plugin_output(pl_output);
  hst->set_long_plugin_output(lpl_output);
  hst->set_perf_data(perfdata_output);

  // A nullptr host check command means we should assume the host is UP.
  if (hst->get_check_command().empty()) {
    hst->set_plugin_output("(Host assumed to be UP)");
    res.exit_code = notifier::ok;
  }

  // Make sure we have some data.
  if (hst->get_plugin_output().empty())
    hst->set_plugin_output("(No output returned from host check)");

  std::string ploutput(hst->get_plugin_output());
  std::replace(ploutput.begin(), ploutput.end(), ';', ':');
  hst->set_plugin_output(ploutput);

  // If we're not doing aggressive host checking, let WARNING
  // states indicate the host is up (fake the result to be notifier::ok).
  if (!config->use_aggressive_host_checking()
      && (res.exit_code == notifier::warning))
    res.exit_code = notifier::ok;

  // Get host state from plugin exit code.
  host::host_state return_result(
        (res.exit_code == notifier::ok)
        ?  host::state_up
        :  host::state_down);

  // Get the end time of command.
  gettimeofday(&end_time, nullptr);

  // Send broker event.
  broker_host_check(
    NEBTYPE_HOSTCHECK_RAW_END,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    checkable::check_active,
    return_result,
    hst->get_state_type(),
    start_time,
    end_time,
    hst->get_check_command().c_str(),
    0.0,
    execution_time,
    config->host_check_timeout(),
    res.exit_status == process::timeout,
    res.exit_code,
    tmp_processed_cmd,
    const_cast<char *>(hst->get_plugin_output().c_str()),
    const_cast<char *>(hst->get_long_plugin_output().c_str()),
    const_cast<char *>(hst->get_perf_data().c_str()),
    nullptr);
  delete[] tmp_processed_cmd;

  // Termination.
  logger(dbg_checks, basic)
    << "** Sync host check done: state=" << return_result;
  return return_result;
}

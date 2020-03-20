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

#include "com/centreon/engine/checks/checker.hh"
#include <sys/time.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/check_result.hh"
#include "com/centreon/engine/checks/viability_failure.hh"
#include "com/centreon/engine/commands/command.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/neberrors.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/exceptions/interruption.hh"

using namespace com::centreon;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::checks;

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
  static checker instance;
  return instance;
}

void checker::clear() noexcept {
  try {
    std::lock_guard<std::mutex> lock(_mut_reap);
    while (!_to_reap_partial.empty()) {
      check_result* result = _to_reap_partial.front();
      _to_reap_partial.pop();
      delete result;
    }
    while (!_to_reap.empty()) {
      check_result* result = _to_reap.front();
      _to_reap.pop();
      delete result;
    }
    auto it = _waiting_check_result.begin();
    while (it != _waiting_check_result.end()) {
      delete it->second;
      it = _waiting_check_result.erase(it);
    }
  } catch (...) {
  }
}

/**
 *  Reap and process all results received by execution process.
 */
void checker::reap() {
  logger(dbg_functions, basic) << "checker::reap";
  logger(dbg_checks, basic) << "Starting to reap check results.";

  // Time to start reaping.
  time_t reaper_start_time;
  time(&reaper_start_time);

  // Reap check results.
  unsigned int reaped_checks(0);
  {  // Scope to release mutex in all termination cases.
    {
      std::lock_guard<std::mutex> lock(_mut_reap);
      std::swap(_to_reap, _to_reap_partial);
    }

    // Process check results.
    while (!_to_reap.empty()) {
      // Get result host or service check.
      logger(dbg_checks, basic)
          << "Found a check result (#" << ++reaped_checks << ") to handle...";
      check_result* result = _to_reap.front();
      _to_reap.pop();

      // Service check result->
      if (service_check == result->get_object_check_type()) {
        service_id_map::iterator it = service::services_by_id.find(
            {result->get_host_id(), result->get_service_id()});
        if (it == service::services_by_id.end()) {
          logger(log_runtime_error, basic)
              << "Warning: Check result queue contained results for service "
              << result->get_host_id() << "/" << result->get_service_id()
              << ", but the service could not be found! Perhaps you forgot to "
                 "define the service in your config files ?";
        } else {
          try {
            // Check if the service exists.
            logger(dbg_checks, more)
                << "Handling check result for service " << result->get_host_id()
                << "/" << result->get_service_id() << "...";
            it->second->handle_async_check_result(result);
          } catch (std::exception const& e) {
            logger(log_runtime_warning, basic)
                << "Check result queue errors for service "
                << result->get_host_id() << "/" << result->get_service_id()
                << " : " << e.what();
          }
        }
      }
      // Host check result->
      else {
        host_id_map::iterator it = host::hosts_by_id.find(result->get_host_id());
        if (it == host::hosts_by_id.end())
          logger(log_runtime_warning, basic)
              << "Warning: Check result queue contained results for "
              << "host " << result->get_host_id() << ", but the host could "
              << "not be found! Perhaps you forgot to define the host in "
              << "your config files ?";
        else {
          try {
            // Process the check result->
            logger(dbg_checks, more) << "Handling check result for host "
                                     << result->get_host_id() << "...";
            it->second->handle_async_check_result_3x(result);
          } catch (std::exception const& e) {
            logger(log_runtime_error, basic)
                << "Check result queue errors for "
                << "host " << result->get_host_id() << " : " << e.what();
          }
        }
      }

      delete result;

      // Check if reaping has timed out.
      time_t current_time;
      time(&current_time);
      if (current_time - reaper_start_time >
          static_cast<time_t>(config->max_check_reaper_time())) {
        logger(dbg_checks, basic) << "Breaking out of check result reaper: "
                                  << "max reaper time exceeded";
        break;
      }

      // Caught signal, need to break.
      if (sigshutdown) {
        logger(dbg_checks, basic)
            << "Breaking out of check result reaper: signal encountered";
        break;
      }
    }
  }

  // Reaping finished.
  logger(dbg_checks, basic)
      << "Finished reaping " << reaped_checks << " check results";
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
void checker::run_sync(host* hst,
                       host::host_state* check_result_code,
                       int check_options,
                       int use_cached_result,
                       unsigned long check_timestamp_horizon) {
  logger(dbg_functions, basic)
      << "checker::run: hst=" << hst << ", check_options=" << check_options
      << ", use_cached_result=" << use_cached_result
      << ", check_timestamp_horizon=" << check_timestamp_horizon;

  // Preamble.
  if (!hst)
    throw engine_error() << "Attempt to run synchronous check on invalid host";
  if (!hst->get_check_command_ptr())
    throw engine_error() << "Attempt to run synchronous active check on host '"
                         << hst->get_name() << "' with no check command";

  logger(dbg_checks, basic)
      << "** Run sync check of host '" << hst->get_name() << "'...";

  // Check if the host is viable now.
  if (!hst->verify_check_viability(check_options, nullptr, nullptr)) {
    if (check_result_code)
      *check_result_code = hst->get_current_state();
    logger(dbg_checks, basic) << "Host check is not viable at this time";
    return;
  }

  // Time to start command.
  timeval start_time;
  gettimeofday(&start_time, nullptr);

  // Can we use the last cached host state?
  if (use_cached_result && !(check_options & CHECK_OPTION_FORCE_EXECUTION)) {
    // We can used the cached result, so return it and get out of here.
    if (hst->get_has_been_checked() &&
        (static_cast<unsigned long>(start_time.tv_sec -
                                    hst->get_last_check()) <=
         check_timestamp_horizon)) {
      if (check_result_code)
        *check_result_code = hst->get_current_state();
      logger(dbg_checks, more)
          << "* Using cached host state: " << hst->get_current_state();

      // Update statistics.
      update_check_stats(ACTIVE_ONDEMAND_HOST_CHECK_STATS, start_time.tv_sec);
      update_check_stats(ACTIVE_CACHED_HOST_CHECK_STATS, start_time.tv_sec);
      return;
    }
  }

  // Checking starts.
  logger(dbg_checks, more) << "* Running actual host check: old state="
                           << hst->get_current_state();

  // Update statistics.
  update_check_stats(ACTIVE_ONDEMAND_HOST_CHECK_STATS, start_time.tv_sec);
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
  std::string old_plugin_output{hst->get_plugin_output()};

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
  broker_host_check(NEBTYPE_HOSTCHECK_INITIATE, NEBFLAG_NONE, NEBATTR_NONE, hst,
                    checkable::check_active, hst->get_current_state(),
                    hst->get_state_type(), start_time, end_time,
                    hst->get_check_command().c_str(), hst->get_latency(), 0.0,
                    config->host_check_timeout(), false, 0, nullptr, nullptr,
                    nullptr, nullptr, nullptr);

  // Execute command synchronously.
  host::host_state host_result(_execute_sync(hst));

  // Process result.
  hst->process_check_result_3x(host_result, old_plugin_output, check_options,
                               false, use_cached_result,
                               check_timestamp_horizon);
  if (check_result_code)
    *check_result_code = hst->get_current_state();

  // Synchronous check is done.
  logger(dbg_checks, more) << "* Sync host check done: new state="
                           << hst->get_current_state();

  // Get the end time of command.
  gettimeofday(&end_time, nullptr);

  // Send event broker.
  broker_host_check(NEBTYPE_HOSTCHECK_PROCESSED, NEBFLAG_NONE, NEBATTR_NONE,
                    hst, checkable::check_active, hst->get_current_state(),
                    hst->get_state_type(), start_time, end_time,
                    hst->get_check_command().c_str(), hst->get_latency(),
                    hst->get_execution_time(), config->host_check_timeout(),
                    false, hst->get_current_state(), nullptr,
                    const_cast<char*>(hst->get_plugin_output().c_str()),
                    const_cast<char*>(hst->get_long_plugin_output().c_str()),
                    const_cast<char*>(hst->get_perf_data().c_str()), nullptr);
}

/**************************************
 *                                     *
 *           Private Methods           *
 *                                     *
 **************************************/

/**
 *  Default constructor.
 */
checker::checker() : commands::command_listener() {}

/**
 *  Default destructor.
 */
checker::~checker() noexcept {
  clear();
//  try {
//    std::lock_guard<std::mutex> lock(_mut_reap);
//    while (!_to_reap.empty()) {
//      _to_reap.pop();
//    }
//  } catch (...) {
//  }
}

/**
 *  Slot to catch the result of the execution and add to the reap queue.
 *
 *  @param[in] res The result of the execution.
 */
void checker::finished(commands::result const& res) noexcept {
  // Debug message.
  logger(dbg_functions, basic) << "checker::finished: res=" << &res;

  std::unique_lock<std::mutex> lock(_mut_reap);
  auto it_id = _waiting_check_result.find(res.command_id);
  if (it_id == _waiting_check_result.end()) {
    logger(log_runtime_warning, basic)
      << "command ID '" << res.command_id << "' not found";
    return;
  }

  // Find check result.
  check_result* result = it_id->second;
  _waiting_check_result.erase(it_id);
  lock.unlock();

  // Update check result.
  struct timeval tv = {.tv_sec = res.end_time.to_seconds(),
                       .tv_usec = res.end_time.to_useconds() % 1000000ll};

  result->set_finish_time(tv);
  result->set_early_timeout(res.exit_status == process::timeout);
  result->set_return_code(res.exit_code);
  result->set_exited_ok(res.exit_status == process::normal ||
                        res.exit_status == process::timeout);
  result->set_output(res.output);

  // Queue check result.
  lock.lock();
  _to_reap_partial.push(result);
}

/**
 *  Run an host check with waiting check result.
 *
 *  @param[in] hst The host to check.
 *
 *  @result Return if the host is up ( host::state_up) or host down (
 * host::state_down).
 */
com::centreon::engine::host::host_state checker::_execute_sync(host* hst) {
  logger(dbg_functions, basic) << "checker::_execute_sync: hst=" << hst;

  // Preamble.
  if (!hst)
    throw engine_error() << "Attempt to run synchronous check on invalid host";
  if (!hst->get_check_command_ptr())
    throw engine_error() << "Attempt to run synchronous active check on host '"
                         << hst->get_name() << "' with no check command";

  logger(dbg_checks, basic)
      << "** Executing sync check of host '" << hst->get_name() << "'...";

  // Send broker event.
  timeval start_time;
  timeval end_time;
  memset(&start_time, 0, sizeof(start_time));
  memset(&end_time, 0, sizeof(end_time));
  int ret(broker_host_check(
      NEBTYPE_HOSTCHECK_SYNC_PRECHECK, NEBFLAG_NONE, NEBATTR_NONE, hst,
      checkable::check_active, hst->get_current_state(), hst->get_state_type(),
      start_time, end_time, hst->get_check_command().c_str(),
      hst->get_latency(), 0.0, config->host_check_timeout(), false, 0, nullptr,
      nullptr, nullptr, nullptr, nullptr));

  // Host sync check was cancelled or overriden by NEB module.
  if ((NEBERROR_CALLBACKCANCEL == ret) || (NEBERROR_CALLBACKOVERRIDE == ret))
    return hst->get_current_state();

  // Get current host macros.
  nagios_macros macros;
  grab_host_macros_r(&macros, hst);
  std::string tmp;
  get_raw_command_line_r(&macros, hst->get_check_command_ptr(),
                         hst->get_check_command().c_str(), tmp, 0);

  // Time to start command.
  gettimeofday(&start_time, nullptr);

  // Update last host check.
  hst->set_last_check(start_time.tv_sec);

  // Get command object.
  commands::command* cmd = hst->get_check_command_ptr();
  std::string processed_cmd(cmd->process_cmd(&macros));
  const char* tmp_processed_cmd = processed_cmd.c_str();

  // Send broker event.
  broker_host_check(NEBTYPE_HOSTCHECK_RAW_START, NEBFLAG_NONE, NEBATTR_NONE,
                    hst, checkable::check_active, host::state_up,
                    hst->get_state_type(), start_time, end_time,
                    hst->get_check_command().c_str(), 0.0, 0.0,
                    config->host_check_timeout(), false, service::state_ok,
                    processed_cmd.c_str(),
                    const_cast<char*>(hst->get_plugin_output().c_str()),
                    const_cast<char*>(hst->get_long_plugin_output().c_str()),
                    const_cast<char*>(hst->get_perf_data().c_str()), nullptr);

  // Debug messages.
  logger(dbg_commands, more)
      << "Raw host check command: "
      << hst->get_check_command_ptr()->get_command_line();
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
  broker_system_command(NEBTYPE_SYSTEM_COMMAND_START, NEBFLAG_NONE,
                        NEBATTR_NONE, start_cmd, end_cmd, 0,
                        config->host_check_timeout(), false, 0,
                        tmp_processed_cmd, nullptr, nullptr);

  // Run command.
  commands::result res;
  try {
    cmd->run(processed_cmd, macros, config->host_check_timeout(), res);
  } catch (std::exception const& e) {
    // Update check result.
    res.command_id = 0;
    res.end_time = timestamp::now();
    res.exit_code = service::state_unknown;
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
    execution_time = res.end_time.to_seconds() - res.start_time.to_seconds();

  // Send broker event.
  memset(&start_cmd, 0, sizeof(start_time));
  start_cmd.tv_sec = res.start_time.to_seconds();
  start_cmd.tv_usec =
      res.start_time.to_useconds() - start_cmd.tv_sec * 1000000ull;
  memset(&end_cmd, 0, sizeof(end_time));
  end_cmd.tv_sec = res.end_time.to_seconds();
  end_cmd.tv_usec = res.end_time.to_useconds() - end_cmd.tv_sec * 1000000ull;
  broker_system_command(NEBTYPE_SYSTEM_COMMAND_END, NEBFLAG_NONE, NEBATTR_NONE,
                        start_cmd, end_cmd, execution_time,
                        config->host_check_timeout(),
                        res.exit_status == process::timeout, res.exit_code,
                        tmp_processed_cmd, output, nullptr);

  // Cleanup.
  delete[] output;
  clear_volatile_macros_r(&macros);

  // If the command timed out.
  if (res.exit_status == process::timeout) {
    std::ostringstream oss;
    oss << "Host check timed out after " << config->host_check_timeout()
        << "  seconds";
    res.output = oss.str();
    logger(log_runtime_warning, basic)
        << "Warning: Host check command '" << processed_cmd << "' for host '"
        << hst->get_name() << "' timed out after "
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
  parse_check_output(res.output, pl_output, lpl_output, perfdata_output, true,
                     false);

  hst->set_plugin_output(pl_output);
  hst->set_long_plugin_output(lpl_output);
  hst->set_perf_data(perfdata_output);

  // A nullptr host check command means we should assume the host is UP.
  if (hst->get_check_command().empty()) {
    hst->set_plugin_output("(Host assumed to be UP)");
    res.exit_code = service::state_ok;
  }

  // Make sure we have some data.
  if (hst->get_plugin_output().empty())
    hst->set_plugin_output("(No output returned from host check)");

  std::string ploutput(hst->get_plugin_output());
  std::replace(ploutput.begin(), ploutput.end(), ';', ':');
  hst->set_plugin_output(ploutput);

  // If we're not doing aggressive host checking, let WARNING
  // states indicate the host is up (fake the result to be UP = 0).
  if (!config->use_aggressive_host_checking() &&
      res.exit_code == service::state_warning)
    res.exit_code = service::state_ok;

  // Get host state from plugin exit code.
  host::host_state return_result(
      (res.exit_code == service::state_ok) ? host::state_up : host::state_down);

  // Get the end time of command.
  gettimeofday(&end_time, nullptr);

  // Send broker event.
  broker_host_check(
      NEBTYPE_HOSTCHECK_RAW_END, NEBFLAG_NONE, NEBATTR_NONE, hst,
      checkable::check_active, return_result, hst->get_state_type(), start_time,
      end_time, hst->get_check_command().c_str(), 0.0, execution_time,
      config->host_check_timeout(), res.exit_status == process::timeout,
      res.exit_code, tmp_processed_cmd,
      const_cast<char*>(hst->get_plugin_output().c_str()),
      const_cast<char*>(hst->get_long_plugin_output().c_str()),
      const_cast<char*>(hst->get_perf_data().c_str()), nullptr);

  // Termination.
  logger(dbg_checks, basic)
      << "** Sync host check done: state=" << return_result;
  return return_result;
}

/**
 * @brief This method stores a command id and the check_result at the origin of
 *this command execution. The command is executed asynchronously. When the
 *execution is finished, thanks to this command we will be able to find the
 *check_result.
 *
 * @param id A command id
 * @param check_result A check_result coming from a service or a host.
 */
void checker::add_check_result(uint64_t id,
                               check_result* check_result) noexcept {
  std::lock_guard<std::mutex> lock(_mut_reap);
  _waiting_check_result[id] = check_result;
}

/**
 * @brief This method stores a check_result already finished in the _to_reap
 *list. The goal of this list is to update services and hosts with check_result.
 *
 * @param check_result The check_result already finished.
 */
void checker::add_check_result_to_reap(check_result* check_result) noexcept {
  std::lock_guard<std::mutex> lock(_mut_reap);
  _to_reap_partial.push(check_result);
}

/*
** Copyright 2007-2008      Ethan Galstad
** Copyright 2007,2010      Andreas Ericsson
** Copyright 2010           Max Schubert
** Copyright 2011-2013,2016 Centreon
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

#include <algorithm>
#include <array>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/loop.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/retention/dump.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/engine/command_manager.hh"

using namespace com::centreon::engine::downtimes;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine;

/**
 * Defaut constructor
 */
timed_event::timed_event()
    : event_type{0},
      run_time{0},
      recurring{0},
      event_interval{0},
      compensate_for_time_change{false},
      timing_func{nullptr},
      event_data{nullptr},
      event_args{nullptr},
      event_options{0} {}

/**
 * Constructor with arguments
 *
 * @param event_type
 * @param run_time
 * @param recurring
 * @param event_interval
 * @param compensate_for_time_change
 * @param timing_func
 * @param event_data
 * @param event_args
 * @param event_options
 */
timed_event::timed_event(uint32_t event_type,
                         time_t run_time,
                         bool recurring,
                         unsigned long event_interval,
                         void* timing_func,
                         bool compensate_for_time_change,
                         void* event_data,
                         void* event_args,
                         int32_t event_options)
    : event_type{event_type},
      run_time{run_time},
      recurring{recurring},
      event_interval{event_interval},
      compensate_for_time_change{compensate_for_time_change},
      timing_func{timing_func},
      event_data{event_data},
      event_args{event_args},
      event_options{event_options} {}

timed_event::~timed_event() {
  if (event_type == timed_event::EVENT_SCHEDULED_DOWNTIME && event_data)
    delete static_cast<uint64_t*>(event_data);
}

/**
 *  Execute service check.
 *
 */
void timed_event::_exec_event_service_check() {
  service* svc(reinterpret_cast<service*>(event_data));

  // get check latency.
  timeval tv;
  gettimeofday(&tv, NULL);
  double latency = (double)((double)(tv.tv_sec - run_time) +
                            (double)(tv.tv_usec / 1000) / 1000.0);

  logger(dbg_events, basic)
      << "** Service Check Event ==> Host: '" << svc->get_hostname()
      << "', Service: '" << svc->get_description()
      << "', Options: " << event_options << ", Latency: " << latency
      << " sec";

  // run the service check.
  svc->run_scheduled_check(event_options, latency);
}

/**
 *  Execute command check.
 *
 */
void timed_event::_exec_event_command_check() {
  logger(dbg_events, basic) << "** External Command Check Event";

  // send data to event broker.
  broker_external_command(NEBTYPE_EXTERNALCOMMAND_CHECK, NEBFLAG_NONE,
                          NEBATTR_NONE, CMD_NONE, time(NULL), NULL, NULL, NULL);
}

/**
 * @brief iExecute EngineRPC command check.
 *
 */
void timed_event::_exec_event_enginerpc_check() {
  logger(dbg_events, basic) << "** EngineRPC Command Check Event";

  // send data to event broker.
  command_manager::instance().execute();
}

/**
 *  Execute log rotation.
 *
 */
void timed_event::_exec_event_log_rotation() {}

/**
 *  Execute program shutdown.
 *
 */
void timed_event::_exec_event_program_shutdown() {
  logger(dbg_events, basic) << "** Program Shutdown Event";

  // set the shutdown flag.
  sigshutdown = true;

  // log the shutdown.
  logger(log_process_info, basic)
      << "PROGRAM_SHUTDOWN event encountered, shutting down...";
}

/**
 *  Execute program restart.
 *
 */
void timed_event::_exec_event_program_restart() {
  logger(dbg_events, basic) << "** Program Restart Event";

  // reload configuration.
  sighup = true;

  // log the restart.
  logger(log_process_info, basic)
      << "PROGRAM_RESTART event encountered, restarting...";
}

static int reap_check_results() {
  try {
    checks::checker::instance().reap();
  } catch (std::exception const& e) {
    logger(log_runtime_error, basic) << "Error: " << e.what();
    return ERROR;
  }
  return OK;
}

/**
 *  Execute check reaper.
 *
 */
void timed_event::_exec_event_check_reaper() {
  logger(dbg_events, basic) << "** Check Result Reaper";

  // reap host and service check results.
  reap_check_results();
}

/**
 *  Execute orphan check.
 *
 */
void timed_event::_exec_event_orphan_check() {
  logger(dbg_events, basic) << "** Orphaned Host and Service Check Event";

  // check for orphaned hosts and services.
  if (config->check_orphaned_hosts())
    host::check_for_orphaned();
  if (config->check_orphaned_services())
    service::check_for_orphaned();
}

/**
 *  Execute retention save.
 *
 */
void timed_event::_exec_event_retention_save() {
  logger(dbg_events, basic) << "** Retention Data Save Event";

  // save state retention data.
  retention::dump::save(config->state_retention_file());
}

/**
 *  Execute status save.
 *
 */
void timed_event::_exec_event_status_save() {
  logger(dbg_events, basic) << "** Status Data Save Event";

  // save all status data (program, host, and service).
  update_all_status_data();
}

/**
 *  Execute scheduled downtime.
 *
 */
void timed_event::_exec_event_scheduled_downtime() {
  logger(dbg_events, basic) << "** Scheduled Downtime Event";

  // process scheduled downtime info.
  if (event_data) {
    handle_scheduled_downtime_by_id(*(uint64_t*)event_data);
    delete static_cast<unsigned long*>(event_data);
    event_data = nullptr;
  }
}

/**
 *  Execute sfreshness check.
 *
 */
void timed_event::_exec_event_sfreshness_check() {
  logger(dbg_events, basic) << "** Service Result Freshness Check Event";

  // check service result freshness.
  service::check_result_freshness();
}

/**
 *  Execute expire downtime.
 *
 */
void timed_event::_exec_event_expire_downtime() {
  logger(dbg_events, basic) << "** Expire Downtime Event";

  // check for expired scheduled downtime entries.
  downtime_manager::instance().check_for_expired_downtime();
}

/**
 *  Execute host check.
 *
 */
void timed_event::_exec_event_host_check() {
  host* hst(reinterpret_cast<host*>(event_data));

  // get check latency.
  timeval tv;
  gettimeofday(&tv, NULL);
  double latency = (double)((double)(tv.tv_sec - run_time) +
                            (double)(tv.tv_usec / 1000) / 1000.0);

  logger(dbg_events, basic)
      << "** Host Check Event ==> Host: '" << hst->get_name()
      << "', Options: " << event_options << ", Latency: " << latency
      << " sec";

  // run the host check.
  hst->run_scheduled_check(event_options, latency);
}

/**
 *  Execute hfreshness check.
 *
 */
void timed_event::_exec_event_hfreshness_check() {
  logger(dbg_events, basic) << "** Host Result Freshness Check Event";

  // check host result freshness.
  host::check_result_freshness();
}

/**
 *  Execute rescheduled checks.
 *
 */
void timed_event::_exec_event_reschedule_checks() {
  logger(dbg_events, basic) << "** Reschedule Checks Event";

  // adjust scheduling of host and service checks.
  events::loop::instance().adjust_check_scheduling();
}

/**
 *  Execute expire comment.
 *
 */
void timed_event::_exec_event_expire_comment() {
  logger(dbg_events, basic) << "** Expire Comment Event";

  // check for expired comment.
  comment::remove_if_expired_comment((unsigned long)event_data);
}

/**
 *  Check for expired host acknowledgement.
 *
 */
void timed_event::_exec_event_expire_host_ack() {
  logger(dbg_events, basic) << "** Expire Host Acknowledgement Event";
  static_cast<host*>(event_data)->check_for_expired_acknowledgement();
}

/**
 *  Check for expired service acknowledgement.
 *
 */
void timed_event::_exec_event_expire_service_ack() {
  logger(dbg_events, basic) << "** Expire Service Acknowledgement Event";
  static_cast<service*>(event_data)->check_for_expired_acknowledgement();
}

/**
 *  Execute user function.
 *
 *  @param[in] event The event to execute.
 */
void timed_event::_exec_event_user_function() {
  logger(dbg_events, basic) << "** User Function Event";

  // run a user-defined function.
  if (event_data) {
    union {
      void (*func)(void*);
      void* data;
    } user;
    user.data = event_data;
    (*user.func)(event_args);
  }
}

/**
 *  Adjusts a timestamp variable in accordance with a system
 *  time change.
 *
 *  @param[in]  last_time       The last time.
 *  @param[in]  current_time    The current time.
 *  @param[in]  time_difference The time difference.
 *  @param[in] ts              The time struct to fill.
 *
 *  @return the adjusted time.
 */
time_t adjust_timestamp_for_time_change(int64_t time_difference, time_t ts) {
  logger(dbg_functions, basic) << "adjust_timestamp_for_time_change()";

  // we shouldn't do anything with epoch or invalid values.
  if (ts == (time_t)0 || ts == (time_t)-1)
    return ts;

  time_t retval = ts + time_difference;
  if (retval < 0)
    retval = 0;
  return retval;
}

/**
 *  Handles a timed event.
 *
 *  @param[in] event The event to execute.
 *
 *  @return OK.
 */
int timed_event::handle_timed_event() {
  typedef void (timed_event::*exec_event)();
  static std::array<exec_event, 19> tab_exec_event{
      &timed_event::_exec_event_service_check,
      &timed_event::_exec_event_command_check,
      &timed_event::_exec_event_log_rotation,
      &timed_event::_exec_event_program_shutdown,
      &timed_event::_exec_event_program_restart,
      &timed_event::_exec_event_check_reaper,
      &timed_event::_exec_event_orphan_check,
      &timed_event::_exec_event_retention_save,
      &timed_event::_exec_event_status_save,
      &timed_event::_exec_event_scheduled_downtime,
      &timed_event::_exec_event_sfreshness_check,
      &timed_event::_exec_event_expire_downtime,
      &timed_event::_exec_event_host_check,
      &timed_event::_exec_event_hfreshness_check,
      &timed_event::_exec_event_reschedule_checks,
      &timed_event::_exec_event_expire_comment,
      &timed_event::_exec_event_expire_host_ack,
      &timed_event::_exec_event_expire_service_ack,
      &timed_event::_exec_event_enginerpc_check};

  logger(dbg_functions, basic) << "handle_timed_event()";

  // send event data to broker.
  broker_timed_event(NEBTYPE_TIMEDEVENT_EXECUTE, NEBFLAG_NONE, NEBATTR_NONE,
                     this, nullptr);

  logger(dbg_events, basic) << "** Timed Event ** Type: " << event_type
                            << ", Run Time: " << my_ctime(&run_time);

  // how should we handle the event?
  if (event_type < tab_exec_event.size())
    (this->*(tab_exec_event[event_type]))();
  else if (event_type == timed_event::EVENT_USER_FUNCTION)
    _exec_event_user_function();

  return OK;
}

/**
 *  Get the event name.
 *
 *  @param[in] evt The event to get name.
 *
 *  @return The event name.
 */
std::string const& timed_event::name() const noexcept {
  static std::string const event_unknown("\"unknown\"");
  static std::string const event_sleep("EVENT_SLEEP");
  static std::string const event_user_function("EVENT_USER_FUNCTION");
  static std::string const event_names[] = {
      "EVENT_SERVICE_CHECK",     "EVENT_COMMAND_CHECK",
      "EVENT_LOG_ROTATION",      "EVENT_PROGRAM_SHUTDOWN",
      "EVENT_PROGRAM_RESTART",   "EVENT_CHECK_REAPER",
      "EVENT_ORPHAN_CHECK",      "EVENT_RETENTION_SAVE",
      "EVENT_STATUS_SAVE",       "EVENT_SCHEDULED_DOWNTIME",
      "EVENT_SFRESHNESS_CHECK",  "EVENT_EXPIRE_DOWNTIME",
      "EVENT_HOST_CHECK",        "EVENT_HFRESHNESS_CHECK",
      "EVENT_RESCHEDULE_CHECKS", "EVENT_EXPIRE_COMMENT",
      "EVENT_EXPIRE_HOST_ACK",   "EVENT_EXPIRE_SERVICE_ACK"};

  if (this->event_type < sizeof(event_names) / sizeof(event_names[0]))
    return event_names[this->event_type];
  if (this->event_type == timed_event::EVENT_SLEEP)
    return event_sleep;
  if (this->event_type == timed_event::EVENT_USER_FUNCTION)
    return event_user_function;
  return event_unknown;
}

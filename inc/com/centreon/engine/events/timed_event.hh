/*
** Copyright 2007-2008      Ethan Galstad
** Copyright 2007,2010      Andreas Ericsson
** Copyright 2010           Max Schubert
** Copyright 2011-2013,2019 Centreon
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

#ifndef CCE_EVENTS_TIMED_EVENT_HH
#define CCE_EVENTS_TIMED_EVENT_HH

#include <stdint.h>
#include <time.h>
#include <string>
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()
class timed_event;
CCE_END()

CCE_BEGIN()
class timed_event {
  void _exec_event_service_check();
  void _exec_event_command_check();
  void _exec_event_log_rotation();
  void _exec_event_program_shutdown();
  void _exec_event_program_restart();
  void _exec_event_check_reaper();
  void _exec_event_orphan_check();
  void _exec_event_retention_save();
  void _exec_event_status_save();
  void _exec_event_scheduled_downtime();
  void _exec_event_sfreshness_check();
  void _exec_event_expire_downtime();
  void _exec_event_host_check();
  void _exec_event_hfreshness_check();
  void _exec_event_reschedule_checks();
  void _exec_event_expire_comment();
  void _exec_event_expire_host_ack();
  void _exec_event_expire_service_ack();
  void _exec_event_enginerpc_check();
  void _exec_event_user_function();

 public:
  uint32_t event_type;
  time_t run_time;
  bool recurring;
  unsigned long event_interval;
  bool compensate_for_time_change;
  void* timing_func;
  void* event_data;
  void* event_args;
  int32_t event_options;

  enum event_type {
    EVENT_SERVICE_CHECK,       // active service check
    EVENT_COMMAND_CHECK,       // external command check
    EVENT_LOG_ROTATION,        // log file rotation
    EVENT_PROGRAM_SHUTDOWN,    // program shutdown
    EVENT_PROGRAM_RESTART,     // program restart
    EVENT_CHECK_REAPER,        // reaps results from host and service checks
    EVENT_ORPHAN_CHECK,        // checks for orphaned hosts and services
    EVENT_RETENTION_SAVE,      // save (dump) retention data
    EVENT_STATUS_SAVE,         // save (dump) status data
    EVENT_SCHEDULED_DOWNTIME,  // scheduled host or service downtime
    EVENT_SFRESHNESS_CHECK,    // checks service result "freshness"
    EVENT_EXPIRE_DOWNTIME,     // checks for (and removes) expired scheduled
                               // downtime
    EVENT_HOST_CHECK,          // active host check
    EVENT_HFRESHNESS_CHECK,    // checks host result "freshness"
    EVENT_RESCHEDULE_CHECKS,   // adjust scheduling of host and service checks
    EVENT_EXPIRE_COMMENT,      // removes expired comments
    EVENT_EXPIRE_HOST_ACK,     // remove expired host acknowledgement
    EVENT_EXPIRE_SERVICE_ACK,  // remove expired service acknowledgement
    EVENT_ENGINERPC_CHECK,     // EngineRPC command check
    EVENT_SLEEP = 98,          // asynchronous sleep event that occurs when
                               // event queues are empty
    EVENT_USER_FUNCTION = 99   // USER-defined function (modules)
  };
  timed_event();
  timed_event(uint32_t event_type,
              time_t run_time,
              bool recurring,
              unsigned long event_interval,
              void* timing_func,
              bool compensate_for_time_change,
              void* event_data,
              void* event_args,
              int32_t event_options);
  ~timed_event();
  int handle_timed_event();

  std::string const& name() const noexcept;
};
CCE_END()

#ifdef __cplusplus
extern "C" {
#endif /* C++ */

time_t adjust_timestamp_for_time_change(int64_t time_difference, time_t ts);

#ifdef __cplusplus
}
#endif /* C++ */

#ifdef __cplusplus

#include <ostream>
#include "com/centreon/engine/namespace.hh"

#endif /* C++ */

#endif  // !CCE_EVENTS_TIMED_EVENT_HH

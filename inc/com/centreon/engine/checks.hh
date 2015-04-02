/*
** Copyright 2002-2006      Ethan Galstad
** Copyright 2011-2013,2015 Merethis
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

#ifndef CCE_CHECKS_HH
#  define CCE_CHECKS_HH

#  include <sys/time.h>
#  include "com/centreon/engine/objects/host.hh"
#  include "com/centreon/engine/objects/service.hh"

// Service dependency values
#  define DEPENDENCIES_OK     0
#  define DEPENDENCIES_FAILED 1

// Object check types
#  define SERVICE_CHECK       0
#  define HOST_CHECK          1

// CHECK_RESULT structure
typedef struct                check_result_struct {
  unsigned int                object_check_type;    // is this a service or a host check?
  char*                       host_name;            // host name
  char*                       service_description;  // service description
  int                         check_type;           // was this an active or passive service check?
  int                         check_options;
  int                         scheduled_check;      // was this a scheduled or an on-demand check?
  int                         reschedule_check;     // should we reschedule the next check
  double                      latency;
  struct timeval              start_time;           // time the service check was initiated
  struct timeval              finish_time;          // time the service check was completed
  int                         early_timeout;        // did the service check timeout?
  int                         exited_ok;            // did the plugin check return okay?
  int                         return_code;          // plugin return code
  char*                       output;               // plugin output
  struct check_result_struct* next;
}                             check_result;

#  ifdef __cplusplus
extern "C" {
#  endif // C++

// Common Check Fucntions

int reap_check_results();

// Service Check Functions

int run_scheduled_service_check(
      service* svc,
      int check_options,
      double latency);
int run_async_service_check(
      service* svc,
      int check_options,
      double latency,
      int scheduled_check,
      int reschedule_check,
      int* time_is_valid,
      time_t* preferred_time);
int handle_async_service_check_result(
      service* temp_service,
      check_result* queued_check_result);
int check_service_check_viability(
      service* svc,
      int check_options,
      int* time_is_valid,
      time_t* new_time);

// Internal Command Implementations

// schedules an immediate or delayed service check
void schedule_service_check(
       service* svc,
       time_t check_time,
       int options);
// schedules an immediate or delayed host check
void schedule_host_check(
       host* hst,
       time_t check_time,
       int options);

// Monitoring/Event Handler Functions

// checks service dependencies
unsigned int check_service_dependencies(service* svc);
// checks the "freshness" of service check results
void check_service_result_freshness();
// determines if a service's check results are fresh
int is_service_result_fresh(
      service* temp_service,
      time_t current_time,
      int log_this);
// checks host dependencie
unsigned int check_host_dependencies(host* hst);
// checks the "freshness" of host check results
void check_host_result_freshness();
// determines if a host's check results are fresh
int is_host_result_fresh(
      host* temp_host,
      time_t current_time,
      int log_this);

// Route/Host Check Functions
int perform_on_demand_host_check(
      host* hst,
      int* check_return_code,
      int check_options,
      int use_cached_result,
      unsigned long check_timestamp_horizon);
int perform_scheduled_host_check(
      host* hst,
      int check_options,
      double latency);
int perform_on_demand_host_check_3x(
      host* hst,
      int* check_result_code,
      int check_options,
      int use_cached_result,
      unsigned long check_timestamp_horizon);
int run_sync_host_check_3x(
      host* hst,
      int* check_result_code,
      int check_options,
      int use_cached_result,
      unsigned long check_timestamp_horizon);
int execute_sync_host_check_3x(host* hst);
int run_scheduled_host_check_3x(
      host* hst,
      int check_options,
      double latency);
int run_async_host_check_3x(
      host* hst,
      int check_options,
      double latency,
      int scheduled_check,
      int reschedule_check,
      int* time_is_valid,
      time_t* preferred_time);
int handle_async_host_check_result_3x(
      host* temp_host,
      check_result* queued_check_result);
int process_host_check_result_3x(
      host* hst,
      int new_state,
      int check_options,
      int reschedule_check,
      int use_cached_result,
      unsigned long check_timestamp_horizon);
int check_host_check_viability_3x(
      host* hst,
      int check_options,
      int* time_is_valid,
      time_t* new_time);
int adjust_host_check_attempt_3x(host* hst, int is_active);
int determine_host_reachability(host* hst);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_CHECKS_HH

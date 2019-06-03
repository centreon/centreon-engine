/*
** Copyright 2002-2006 Ethan Galstad
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

#ifndef CCE_CHECKS_HH
#  define CCE_CHECKS_HH

#  include <cstdio>
#  include <sys/time.h>
#  include "com/centreon/engine/namespace.hh"

// Service dependency values
#  define DEPENDENCIES_OK     0
#  define DEPENDENCIES_FAILED 1

// Object check types
#  define SERVICE_CHECK       0
#  define HOST_CHECK          1

CCE_BEGIN()
  class host;
  class service;
CCE_END()

enum check_type {
  check_active =  0,        /* Engine performed the check. */
  check_passive = 1,        /* Check result submitted by an external source. */
};

// CHECK_RESULT structure
typedef struct                check_result_struct {
  unsigned int                object_check_type;    // is this a service or a host check?
  char*                       host_name;            // host name
  char*                       service_description;  // service description
  int                         check_type;           // was this an active or passive service check?
  int                         check_options;
  int                         scheduled_check;      // was this a scheduled or an on-demand check?
  int                         reschedule_check;     // should we reschedule the next check
  char*                       output_file;          // what file is the output stored in?
  FILE*                       output_file_fp;
  int                         output_file_fd;
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

// Monitoring/Event Handler Functions

// checks service dependencies
unsigned int check_service_dependencies(
               com::centreon::engine::service* svc,
               int dependency_type);
// checks for orphaned services
void check_for_orphaned_services();
// checks the "freshness" of service check results
void check_service_result_freshness();
// determines if a service's check results are fresh
int is_service_result_fresh(
      com::centreon::engine::service* temp_service,
      time_t current_time,
      int log_this);
// checks host dependencie
unsigned int check_host_dependencies(
               com::centreon::engine::host* hst,
               int dependency_type);
// checks for orphaned hosts
void check_for_orphaned_hosts();
// checks the "freshness" of host check results
void check_host_result_freshness();
// determines if a host's check results are fresh
int is_host_result_fresh(
      com::centreon::engine::host* temp_host,
      time_t current_time,
      int log_this);

// Route/Host Check Functions
int perform_on_demand_host_check(
      com::centreon::engine::host* hst,
      int* check_return_code,
      int check_options,
      int use_cached_result,
      unsigned long check_timestamp_horizon);
int perform_scheduled_host_check(
      com::centreon::engine::host* hst,
      int check_options,
      double latency);
int perform_on_demand_host_check_3x(
      com::centreon::engine::host* hst,
      int* check_result_code,
      int check_options,
      int use_cached_result,
      unsigned long check_timestamp_horizon);
int run_sync_host_check_3x(
      com::centreon::engine::host* hst,
      int* check_result_code,
      int check_options,
      int use_cached_result,
      unsigned long check_timestamp_horizon);
int process_host_check_result_3x(
      com::centreon::engine::host* hst,
      int new_state,
      char* old_plugin_output,
      int check_options,
      int reschedule_check,
      int use_cached_result,
      unsigned long check_timestamp_horizon);
int adjust_host_check_attempt_3x(com::centreon::engine::host* hst,
                                 int is_active);
int determine_host_reachability(com::centreon::engine::host* hst);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_CHECKS_HH

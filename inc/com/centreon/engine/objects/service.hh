/*
** Copyright 2011-2015 Merethis
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

#ifndef CCE_OBJECTS_SERVICE_HH
#  define CCE_OBJECTS_SERVICE_HH

#  include <time.h>
#  include "com/centreon/engine/common.hh"

/* Forward declaration. */
struct command_struct;
struct customvariablesmember_struct;
struct host_struct;
struct objectlist_struct;
struct timeperiod_struct;

typedef struct                  service_struct {
  char*                         host_name;
  char*                         description;
  char*                         service_check_command;
  char*                         event_handler;
  int                           initial_state;
  double                        check_interval;
  double                        retry_interval;
  int                           max_attempts;
  long                          check_timeout;
  int                           is_volatile;
  char*                         check_period;
  int                           flap_detection_enabled;
  double                        low_flap_threshold;
  double                        high_flap_threshold;
  int                           flap_detection_on_ok;
  int                           flap_detection_on_warning;
  int                           flap_detection_on_unknown;
  int                           flap_detection_on_critical;
  int                           check_freshness;
  int                           freshness_threshold;
  int                           event_handler_enabled;
  int                           checks_enabled;
  int                           obsess_over_service;
  customvariablesmember_struct* custom_variables;
  int                           host_problem_at_last_check;
  int                           check_type;
  int                           current_state;
  int                           last_state;
  int                           last_hard_state;
  char*                         plugin_output;
  char*                         long_plugin_output;
  char*                         perf_data;
  int                           state_type;
  time_t                        next_check;
  int                           should_be_scheduled;
  time_t                        last_check;
  int                           current_attempt;
  unsigned long                 current_event_id;
  unsigned long                 last_event_id;
  unsigned long                 current_problem_id;
  unsigned long                 last_problem_id;
  time_t                        last_state_change;
  time_t                        last_hard_state_change;
  time_t                        last_time_ok;
  time_t                        last_time_warning;
  time_t                        last_time_unknown;
  time_t                        last_time_critical;
  int                           has_been_checked;
  int                           is_being_freshened;
  double                        latency;
  double                        execution_time;
  int                           is_executing;
  int                           check_options;
  int                           state_history[MAX_STATE_HISTORY_ENTRIES];
  unsigned int                  state_history_index;
  int                           is_flapping;
  double                        percent_state_change;
  unsigned long                 modified_attributes;
  char*                         timezone;

  host_struct*                  host_ptr;
  command_struct*               event_handler_ptr;
  char*                         event_handler_args;
  command_struct*               check_command_ptr;
  char*                         check_command_args;
  timeperiod_struct*            check_period_ptr;
  struct service_struct*        next;
  struct service_struct*        nexthash;
}                               service;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

service* add_service(
           char const* host_name,
           char const* description,
           char const* check_period,
           int initial_state,
           int max_attempts,
           unsigned int check_timeout,
           double check_interval,
           double retry_interval,
           int is_volatile,
           char const* event_handler,
           int event_handler_enabled,
           char const* check_command,
           int checks_enabled,
           int flap_detection_enabled,
           double low_flap_threshold,
           double high_flap_threshold,
           int flap_detection_on_ok,
           int flap_detection_on_warning,
           int flap_detection_on_unknown,
           int flap_detection_on_critical,
           int check_freshness,
           int freshness_threshold,
           int obsess_over_service,
           char const* timezone);
int      get_service_count();

#  ifdef __cplusplus
}

#    include <ostream>
#    include <string>
#    include "com/centreon/engine/namespace.hh"

bool          operator==(
                service const& obj1,
                service const& obj2) throw ();
bool          operator!=(
                service_struct const& obj1,
                service_struct const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, service const& obj);

CCE_BEGIN()

service&      find_service(
                std::string const& host_name,
                std::string const& service_description);
bool          is_service_exist(
                std::pair<std::string, std::string> const& id);

CCE_END()

#  endif /* C++ */

#endif // !CCE_OBJECTS_SERVICE_HH

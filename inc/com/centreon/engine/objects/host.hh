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

#ifndef CCE_OBJECTS_HOST_HH
#  define CCE_OBJECTS_HOST_HH

#  include <time.h>
#  include "com/centreon/engine/common.hh"

/* Forward declaration. */
struct command_struct;
struct customvariablesmember_struct;
struct hostsmember_struct;
struct objectlist_struct;
struct servicesmember_struct;
struct timeperiod_struct;

typedef struct                  host_struct {
  char*                         name;
  char*                         alias;
  char*                         address;
  hostsmember_struct*           parent_hosts;
  hostsmember_struct*           child_hosts;
  servicesmember_struct*        services;
  char*                         host_check_command;
  int                           initial_state;
  double                        check_interval;
  double                        retry_interval;
  int                           max_attempts;
  char*                         event_handler;
  char*                         check_period;
  long                          check_timeout;
  int                           flap_detection_enabled;
  double                        low_flap_threshold;
  double                        high_flap_threshold;
  int                           flap_detection_on_up;
  int                           flap_detection_on_down;
  int                           flap_detection_on_unreachable;
  int                           check_freshness;
  int                           freshness_threshold;
  int                           checks_enabled;
  int                           event_handler_enabled;
  int                           obsess_over_host;
  int                           should_be_drawn;
  customvariablesmember_struct* custom_variables;
  int                           check_type;
  int                           current_state;
  int                           last_state;
  int                           last_hard_state;
  char*                         plugin_output;
  char*                         long_plugin_output;
  char*                         perf_data;
  int                           state_type;
  int                           current_attempt;
  unsigned long                 current_event_id;
  unsigned long                 last_event_id;
  unsigned long                 current_problem_id;
  unsigned long                 last_problem_id;
  double                        latency;
  double                        execution_time;
  int                           is_executing;
  int                           check_options;
  time_t                        next_check;
  int                           should_be_scheduled;
  time_t                        last_check;
  time_t                        last_state_change;
  time_t                        last_hard_state_change;
  time_t                        last_time_up;
  time_t                        last_time_down;
  time_t                        last_time_unreachable;
  int                           has_been_checked;
  int                           is_being_freshened;
  int                           state_history[MAX_STATE_HISTORY_ENTRIES];
  unsigned int                  state_history_index;
  time_t                        last_state_history_update;
  int                           is_flapping;
  double                        percent_state_change;
  int                           total_services;
  unsigned long                 total_service_check_interval;
  unsigned long                 modified_attributes;
  int                           circular_path_checked;
  int                           contains_circular_path;
  char*                         timezone;

  command_struct*               event_handler_ptr;
  command_struct*               check_command_ptr;
  timeperiod_struct*            check_period_ptr;
  struct host_struct*           next;
  struct host_struct*           nexthash;
}                               host;

/* Other HOST structure. */
struct                          host_other_properties {
  bool                          should_reschedule_current_check;
};

/* Hash structures. */
typedef struct                  host_cursor_struct {
  int                           host_hashchain_iterator;
  host*                         current_host_pointer;
}                               host_cursor;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

host* add_host(
        char const* name,
        char const* alias,
        char const* address,
        char const* check_period,
        int initial_state,
        double check_interval,
        double retry_interval,
        int max_attempts,
        unsigned int check_timeout,
        char const* check_command,
        int checks_enabled,
        char const* event_handler,
        int event_handler_enabled,
        int flap_detection_enabled,
        double low_flap_threshold,
        double high_flap_threshold,
        int flap_detection_on_up,
        int flap_detection_on_down,
        int flap_detection_on_unreachable,
        int check_freshness,
        int freshness_threshold,
        int should_be_drawn,
        int obsess_over_host,
        char const* timezone);
int   get_host_count();
int   is_host_immediate_child_of_host(host* parent, host* child);
int   is_host_immediate_parent_of_host(host* child, host* parent);
int   number_of_immediate_child_hosts(host* hst);
int   number_of_immediate_parent_hosts(host* hst);
int   number_of_total_child_hosts(host* hst);
int   number_of_total_parent_hosts(host* hst);

#  ifdef __cplusplus
}

#    include <ostream>
#    include <string>
#    include "com/centreon/engine/namespace.hh"

bool          operator==(
                host const& obj1,
                host const& obj2) throw ();
bool          operator!=(
                host const& obj1,
                host const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, host const& obj);

CCE_BEGIN()

host&         find_host(std::string const& name);
bool          is_host_exist(std::string const& name) throw ();

CCE_END()

#  endif /* C++ */

#endif // !CCE_OBJECTS_HOST_HH

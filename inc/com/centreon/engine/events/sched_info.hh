/*
** Copyright 2007-2008 Ethan Galstad
** Copyright 2007,2010 Andreas Ericsson
** Copyright 2010      Max Schubert
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

#ifndef CCE_EVENTS_SCHED_INFO_HH
#  define CCE_EVENTS_SCHED_INFO_HH

#  include "com/centreon/engine/namespace.hh"
#  include <time.h>

// Forward declaration.
CCE_BEGIN()
class service;
CCE_END()

struct timed_event_struct;

typedef struct  sched_info_struct {
  int           total_services;
  int           total_scheduled_services;
  int           total_hosts;
  int           total_scheduled_hosts;
  double        average_services_per_host;
  double        average_scheduled_services_per_host;
  unsigned long service_check_interval_total;
  unsigned long host_check_interval_total;
  double        average_service_execution_time;
  double        average_service_check_interval;
  double        average_host_check_interval;
  double        average_service_inter_check_delay;
  double        average_host_inter_check_delay;
  double        service_inter_check_delay;
  double        host_inter_check_delay;
  int           service_interleave_factor;
  int           max_service_check_spread;
  int           max_host_check_spread;
  time_t        first_service_check;
  time_t        last_service_check;
  time_t        first_host_check;
  time_t        last_host_check;
}               sched_info;

#  ifdef __cplusplus
extern "C" {
#  endif /* C++ */

void adjust_check_scheduling();
void display_scheduling_info();

#  ifdef __cplusplus
}

#    include <ostream>

bool          operator==(
                sched_info const& obj1,
                sched_info const& obj2) throw ();
bool          operator!=(
                sched_info const& obj1,
                sched_info const& obj2) throw ();
std::ostream& operator<<(std::ostream& os, sched_info const& obj);

#  endif /* C++ */

#endif // !CCE_EVENTS_SCHED_INFO_HH

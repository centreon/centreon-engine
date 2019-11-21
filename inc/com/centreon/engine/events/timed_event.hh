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
#include <deque>
#include <string>
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()
class timed_event;
CCE_END()

typedef std::deque<com::centreon::engine::timed_event*> timed_event_list;

CCE_BEGIN()
class timed_event {
 public:
  enum priority { low = 0, high = 1, priority_num };
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

  uint32_t event_type;
  time_t run_time;
  bool recurring;
  unsigned long event_interval;
  bool compensate_for_time_change;
  void* timing_func;
  void* event_data;
  void* event_args;
  int32_t event_options;

  static timed_event_list event_list_high;
  static timed_event_list event_list_low;

  static timed_event* find_event(timed_event::priority,
                                 uint32_t event,
                                 void* data);
  void schedule(bool high_priority);
  std::string const& name() const noexcept;
};
CCE_END()

#ifdef __cplusplus
extern "C" {
#endif /* C++ */

void add_event(com::centreon::engine::timed_event* event,
               com::centreon::engine::timed_event::priority priority);
time_t adjust_timestamp_for_time_change(time_t last_time,
                                        time_t current_time,
                                        uint64_t time_difference,
                                        time_t ts);
void compensate_for_system_time_change(unsigned long last_time,
                                       unsigned long current_time);
int handle_timed_event(com::centreon::engine::timed_event* event);
void remove_event(com::centreon::engine::timed_event* event,
                  com::centreon::engine::timed_event::priority priority);
void reschedule_event(com::centreon::engine::timed_event* event,
                      com::centreon::engine::timed_event::priority priority);
void resort_event_list(com::centreon::engine::timed_event::priority priority);

#ifdef __cplusplus
}
#endif /* C++ */

#ifdef __cplusplus

#include <ostream>
#include "com/centreon/engine/namespace.hh"

bool operator==(com::centreon::engine::timed_event const& obj1,
                com::centreon::engine::timed_event const& obj2) throw();
bool operator!=(com::centreon::engine::timed_event const& obj1,
                com::centreon::engine::timed_event const& obj2) throw();
std::ostream& operator<<(std::ostream& os,
                         com::centreon::engine::timed_event const& obj);

#endif /* C++ */

#endif  // !CCE_EVENTS_TIMED_EVENT_HH

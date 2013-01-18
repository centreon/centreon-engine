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

#ifndef CCE_EVENTS_HH
#  define CCE_EVENTS_HH

#  include <time.h>

// Event Types
# define EVENT_SERVICE_CHECK          0	  // active service check
# define EVENT_COMMAND_CHECK          1	  // external command check
# define EVENT_LOG_ROTATION           2   // log file rotation
# define EVENT_PROGRAM_SHUTDOWN       3   // program shutdown
# define EVENT_PROGRAM_RESTART        4	  // program restart
# define EVENT_CHECK_REAPER           5   // reaps results from host and service checks
# define EVENT_ORPHAN_CHECK           6	  // checks for orphaned hosts and services
# define EVENT_RETENTION_SAVE         7	  // save (dump) retention data
# define EVENT_STATUS_SAVE            8	  // save (dump) status data
# define EVENT_SCHEDULED_DOWNTIME     9	  // scheduled host or service downtime
# define EVENT_SFRESHNESS_CHECK       10  // checks service result "freshness"
# define EVENT_EXPIRE_DOWNTIME        11  // checks for (and removes) expired scheduled downtime
# define EVENT_HOST_CHECK             12  // active host check
# define EVENT_HFRESHNESS_CHECK       13  // checks host result "freshness"
# define EVENT_RESCHEDULE_CHECKS      14  // adjust scheduling of host and service checks
# define EVENT_EXPIRE_COMMENT         15  // removes expired comments
# define EVENT_SLEEP                  98  // asynchronous sleep event that occurs when event queues are empty
# define EVENT_USER_FUNCTION          99  // USER-defined function (modules)

// TIMED_EVENT structure
typedef struct               timed_event_struct {
  unsigned int               event_type;
  time_t                     run_time;
  int                        recurring;
  unsigned long              event_interval;
  int                        compensate_for_time_change;
  void*                      timing_func;
  void*                      event_data;
  void*                      event_args;
  int                        event_options;
  struct timed_event_struct* next;
  struct timed_event_struct* prev;
}                            timed_event;

#  ifdef __cplusplus
extern "C" {
#  endif // C++

// Setup Functions

// setup the initial scheduling queue
void init_timing_loop();
// displays service check scheduling information
void display_scheduling_info();

// Event Queue Functions

// schedules a new timed event
void schedule_new_event(
       int event_type,
       int high_priority,
       time_t run_time,
       int recurring,
       unsigned long event_interval,
       void* timing_func,
       int compensate_for_time_change,
       void* event_data,
       void* event_args,
       int event_options);
// reschedules an event
void reschedule_event(
       timed_event* event,
       timed_event** event_list,
       timed_event** event_list_tail);
// adds an event to the execution queue
void add_event(
       timed_event* event,
       timed_event** event_list,
       timed_event** event_list_tail);
// remove an event from the execution queue
void remove_event(
       timed_event* event,
       timed_event** event_list,
       timed_event** event_list_tail);
// main monitoring/event handler loop
int event_execution_loop();
// top level handler for timed events
int handle_timed_event(timed_event* event);
// auto-adjusts scheduling of host and service checks
void adjust_check_scheduling();
// attempts to compensate for a change in the system time
void compensate_for_system_time_change(
       unsigned long last_time,
       unsigned long current_time);
// resorts event list by event run time for system time changes
void resort_event_list(
       timed_event** event_list,
       timed_event** event_list_tail);
// adjusts a timestamp variable for a system time change
void adjust_timestamp_for_time_change(
       time_t last_time,
       time_t current_time,
       unsigned long time_difference,
       time_t* ts);

#  ifdef __cplusplus
}
#  endif // C++

#endif // !CCE_EVENTS_HH

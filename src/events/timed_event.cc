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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/downtimes/downtime_manager.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/timed_event.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/retention/dump.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine::downtimes;
using namespace com::centreon::engine::events;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine;

timed_event_list timed_event::event_list_high;
timed_event_list timed_event::event_list_low;

timed_event::timed_event() :
  event_type{0},
  run_time{0},
  recurring{0},
  event_interval{0},
  compensate_for_time_change{0},
  timing_func{nullptr},
  event_data{nullptr},
  event_args{nullptr},
  event_options{0}
  {}

/**
 *  Execute service check.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_service_check(timed_event* event) {
  com::centreon::engine::service* svc(reinterpret_cast<com::centreon::engine::service*>(event->event_data));

  // get check latency.
  timeval tv;
  gettimeofday(&tv, NULL);
  double latency = (double)((double)(tv.tv_sec - event->run_time)
                            + (double)(tv.tv_usec / 1000) / 1000.0);

  logger(dbg_events, basic)
    << "** Service Check Event ==> Host: '" << svc->get_hostname()
    << "', Service: '" << svc->get_description() << "', Options: "
    << event->event_options << ", Latency: " << latency << " sec";

  // run the service check.
  svc->run_scheduled_check(event->event_options, latency);
}

/**
 *  Execute command check.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_command_check(timed_event* event) {
  (void)event;
  logger(dbg_events, basic)
    << "** External Command Check Event";

  // send data to event broker.
  broker_external_command(
    NEBTYPE_EXTERNALCOMMAND_CHECK,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    CMD_NONE,
    time(NULL),
    NULL,
    NULL,
    NULL);
}

/**
 *  Execute log rotation.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_log_rotation(timed_event* event) {
  (void)event;
}

/**
 *  Execute program shutdown.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_program_shutdown(timed_event* event) {
  (void)event;
  logger(dbg_events, basic)
    << "** Program Shutdown Event";

  // set the shutdown flag.
  sigshutdown = true;

  // log the shutdown.
  logger(log_process_info, basic)
    << "PROGRAM_SHUTDOWN event encountered, shutting down...";
}

/**
 *  Execute program restart.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_program_restart(timed_event* event) {
  (void)event;
  logger(dbg_events, basic)
    << "** Program Restart Event";

  // reload configuration.
  sighup = true;

  // log the restart.
  logger(log_process_info, basic)
    << "PROGRAM_RESTART event encountered, restarting...";
}

static int reap_check_results() {
  try {
    checks::checker::instance().reap();
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic)
      << "Error: " << e.what();
    return ERROR;
  }
  return OK;
}

/**
 *  Execute check reaper.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_check_reaper(timed_event* event) {
  (void)event;
  logger(dbg_events, basic)
    << "** Check Result Reaper";

  // reap host and service check results.
  reap_check_results();
}

/**
 *  Execute orphan check.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_orphan_check(timed_event* event) {
  (void)event;
  logger(dbg_events, basic)
    << "** Orphaned Host and Service Check Event";

  // check for orphaned hosts and services.
  if (config->check_orphaned_hosts())
    host::check_for_orphaned();
  if (config->check_orphaned_services())
    service::check_for_orphaned();
}

/**
 *  Execute retention save.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_retention_save(timed_event* event) {
  (void)event;
  logger(dbg_events, basic)
    << "** Retention Data Save Event";

  // save state retention data.
  retention::dump::save(config->state_retention_file());
}

/**
 *  Execute status save.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_status_save(timed_event* event) {
  (void)event;
  logger(dbg_events, basic)
    << "** Status Data Save Event";

  // save all status data (program, host, and service).
  update_all_status_data();
}

/**
 *  Execute scheduled downtime.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_scheduled_downtime(timed_event* event) {
  logger(dbg_events, basic)
    << "** Scheduled Downtime Event";

  // process scheduled downtime info.
  if (event->event_data) {
    handle_scheduled_downtime_by_id(*(uint64_t*)event->event_data);
    delete static_cast<unsigned long*>(event->event_data);
    event->event_data = nullptr;
  }
}

/**
 *  Execute sfreshness check.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_sfreshness_check(timed_event* event) {
  (void)event;
  logger(dbg_events, basic)
    << "** Service Result Freshness Check Event";

  // check service result freshness.
  service::check_result_freshness();
}

/**
 *  Execute expire downtime.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_expire_downtime(timed_event* event) {
  (void)event;
  logger(dbg_events, basic)
    << "** Expire Downtime Event";

  // check for expired scheduled downtime entries.
  downtime_manager::instance().check_for_expired_downtime();
}

/**
 *  Execute host check.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_host_check(timed_event* event) {
  host* hst(reinterpret_cast<host*>(event->event_data));

  // get check latency.
  timeval tv;
  gettimeofday(&tv, NULL);
  double latency = (double)((double)(tv.tv_sec - event->run_time)
                            + (double)(tv.tv_usec / 1000) / 1000.0);

  logger(dbg_events, basic)
    << "** Host Check Event ==> Host: '" << hst->get_name()
    << "', Options: " << event->event_options
    << ", Latency: " << latency << " sec";

  // run the host check.
  hst->perform_scheduled_check(event->event_options, latency);
}

/**
 *  Execute hfreshness check.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_hfreshness_check(timed_event* event) {
  (void)event;
  logger(dbg_events, basic)
    << "** Host Result Freshness Check Event";

  // check host result freshness.
  host::check_result_freshness();
}

/**
 *  Execute rescheduled checks.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_reschedule_checks(timed_event* event) {
  (void)event;
  logger(dbg_events, basic)
    << "** Reschedule Checks Event";

  // adjust scheduling of host and service checks.
  adjust_check_scheduling();
}

/**
 *  Execute expire comment.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_expire_comment(timed_event* event) {
  logger(dbg_events, basic)
    << "** Expire Comment Event";

  // check for expired comment.
  comment::remove_if_expired_comment((unsigned long)event->event_data);
}

/**
 *  Check for expired host acknowledgement.
 *
 *  @param[in] event  Event to execute.
 */
static void _exec_event_expire_host_ack(timed_event* event) {
  logger(dbg_events, basic)
    << "** Expire Host Acknowledgement Event";
  static_cast<host*>(event->event_data)->check_for_expired_acknowledgement();
}

/**
 *  Check for expired service acknowledgement.
 *
 *  @param[in] event  Event to execute.
 */
static void _exec_event_expire_service_ack(timed_event* event) {
  logger(dbg_events, basic)
    << "** Expire Service Acknowledgement Event";
  static_cast<service*>(event->event_data)->check_for_expired_acknowledgement();
}

/**
 *  Execute user function.
 *
 *  @param[in] event The event to execute.
 */
static void _exec_event_user_function(timed_event* event) {
  logger(dbg_events, basic)
    << "** User Function Event";

  // run a user-defined function.
  if (event->event_data) {
    union {
      void (*func)(void*);
      void* data;
    } user;
    user.data = event->event_data;
    (*user.func)(event->event_args);
  }
}

/**
 *  Add an event to list ordered by execution time.
 *
 *  @param[in] event           The new event to add.
 *  @param[in] event_list      The head of the event list.
 *  @param[in] event_list_tail The tail of the event list.
 */
void add_event(
       timed_event* event,
       timed_event::priority priority) {
  logger(dbg_functions, basic)
    << "add_event()";

  timed_event_list *list;

  if (priority == timed_event::low) {
    quick_timed_event.insert(timed_event::low, event);
    list = &timed_event::event_list_low;
  } else {
    quick_timed_event.insert(timed_event::high, event);
    list = &timed_event::event_list_high;
  }

  // add the event to the head of the list if there are
  // no other events.
  if (list->empty()) {
    list->push_front(event);
  }

  // add event to head of the list if it should be executed first.
  else if (event->run_time < (*list->begin())->run_time) {
    list->push_front(event);
  }

  // else place the event according to next execution time.
  else {
    // start from the end of the list, as new events are likely to
    // be executed in the future, rather than now...
    for(timed_event_list::reverse_iterator
          it(list->rbegin()),
          end(list->rend());
        it != end;
        ++it) {
      if (event->run_time >= (*it)->run_time) {
        list->insert(it.base(), event);
        break;
      }
    }
  }

  // send event data to broker.
  broker_timed_event(
    NEBTYPE_TIMEDEVENT_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    event,
    nullptr);
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
time_t adjust_timestamp_for_time_change(time_t last_time,
                                        time_t current_time,
                                        uint64_t time_difference,
                                        time_t ts) {
  logger(dbg_functions, basic) << "adjust_timestamp_for_time_change()";

  // we shouldn't do anything with epoch or invalid values.
  if (ts == (time_t)0 || ts == (time_t)-1)
    return ts;

  // we moved back in time...
  if (last_time > current_time) {
    // we can't precede the UNIX epoch.
    if (time_difference > (uint32_t)ts)
      return (time_t)0;
    else
      return (time_t)(ts - time_difference);
  }
  // we moved into the future...
  else
    return (time_t)(ts + time_difference);
}

/**
 *  Attempts to compensate for a change in the system time.
 *
 *  @param[in] last_time    The last time.
 *  @param[in] current_time The current time.
 */
void compensate_for_system_time_change(
       unsigned long last_time,
       unsigned long current_time) {
  int days(0);
  int hours(0);
  int minutes(0);
  int seconds(0);
  unsigned long time_difference(0L);

  logger(dbg_functions, basic)
    << "compensate_for_system_time_change()";

  // we moved back in time...
  if (last_time > current_time) {
    time_difference = last_time - current_time;
    get_time_breakdown(
      time_difference,
      &days,
      &hours,
      &minutes,
      &seconds);
    logger(dbg_events, basic)
      << "Detected a backwards time change of " << days << "d "
      << hours << "h " << minutes << "m " << seconds << "s.";
  }
  // we moved into the future...
  else {
    time_difference = current_time - last_time;
    get_time_breakdown(
      time_difference,
      &days,
      &hours,
      &minutes,
      &seconds);
    logger(dbg_events, basic)
      << "Detected a forwards time change of " << days << "d "
      << hours << "h " << minutes << "m " << seconds << "s.";
  }

  // log the time change.
  logger(log_process_info | log_runtime_warning, basic)
    << "Warning: A system time change of " << days << "d "
    << hours << "h " << minutes << "m " << seconds << "s ("
    << (last_time > current_time ? "backwards" : "forwards")
    << " in time) has been detected.  Compensating...";

  // adjust the next run time for all high priority timed events.
  for (timed_event_list::iterator
         it(timed_event::event_list_high.begin()),
         end(timed_event::event_list_high.end());
       it != end;
       ++it) {

    // skip special events that occur at specific times...
    if (!(*it)->compensate_for_time_change)
      continue;

    // use custom timing function.
    if ((*it)->timing_func) {
      union {
        time_t (*func)(void);
        void* data;
      } timing;
      timing.data = (*it)->timing_func;
      (*it)->run_time = (*timing.func)();
    }

    // else use standard adjustment.
    else
      (*it)->run_time = adjust_timestamp_for_time_change(
        last_time,
        current_time,
        time_difference,
        (*it)->run_time);
  }

  // resort event list (some events may be out of order at this point).
  resort_event_list(timed_event::high);

  // adjust the next run time for all low priority timed events.
  for (timed_event_list::iterator
         it(timed_event::event_list_low.begin()),
         end(timed_event::event_list_low.end());
       it != end;
       ++it) {

    // skip special events that occur at specific times...
    if (!(*it)->compensate_for_time_change)
      continue;

    // use custom timing function.
    if ((*it)->timing_func) {
      union {
        time_t (*func)(void);
        void* data;
      } timing;
      timing.data = (*it)->timing_func;
      (*it)->run_time = (*timing.func)();
    }

    // else use standard adjustment.
    else
      (*it)->run_time = adjust_timestamp_for_time_change(
        last_time,
        current_time,
        time_difference,
        (*it)->run_time);
  }

  // resort event list (some events may be out of order at this point).
  resort_event_list(timed_event::low);

  // adjust service timestamps.
  for (service_map::iterator
         it(service::services.begin()),
         end(service::services.end());
       it != end;
       ++it) {
    it->second->set_last_notification(adjust_timestamp_for_time_change(
      last_time,
      current_time,
      time_difference,
      it->second->get_last_notification()));
    it->second->set_last_check(adjust_timestamp_for_time_change(
      last_time,
      current_time,
      time_difference,
      it->second->get_last_check()));
    it->second->set_next_check(adjust_timestamp_for_time_change(
      last_time,
      current_time,
      time_difference,
      it->second->get_next_check()));
    it->second->set_last_state_change(adjust_timestamp_for_time_change(
      last_time,
      current_time,
      time_difference,
      it->second->get_last_state_change()));
    it->second->set_last_hard_state_change(adjust_timestamp_for_time_change(
      last_time,
      current_time,
      time_difference,
      it->second->get_last_hard_state_change()));

    it->second->set_initial_notif_time(adjust_timestamp_for_time_change(
        last_time, current_time, time_difference,
        it->second->get_initial_notif_time()));
    it->second->set_last_acknowledgement(adjust_timestamp_for_time_change(
        last_time, current_time, time_difference,
        it->second->get_last_acknowledgement()));

    // recalculate next re-notification time.
    it->second->set_next_notification(
      it->second->get_next_notification_time(
        it->second->get_last_notification()));

    // update the status data.
    it->second->update_status(false);
  }

  // adjust host timestamps.
  for (host_map::iterator
         it(com::centreon::engine::host::hosts.begin()),
         end(com::centreon::engine::host::hosts.end());
       it != end;
       ++it) {
    time_t last_host_notif{adjust_timestamp_for_time_change(
      last_time,
      current_time,
      time_difference,
      it->second->get_last_notification())};
    time_t last_check{adjust_timestamp_for_time_change(
      last_time,
      current_time,
      time_difference,
      it->second->get_last_check())};
    time_t next_check{adjust_timestamp_for_time_change(
      last_time,
      current_time,
      time_difference,
      it->second->get_next_check())};
    time_t last_state_change{adjust_timestamp_for_time_change(
      last_time,
      current_time,
      time_difference,
      it->second->get_last_state_change())};
    time_t last_hard_state_change{adjust_timestamp_for_time_change(
      last_time,
      current_time,
      time_difference,
      it->second->get_last_hard_state_change())};
    time_t last_state_history_update{adjust_timestamp_for_time_change(
      last_time,
      current_time,
      time_difference,
      it->second->get_last_state_history_update())};

    it->second->set_last_notification(last_host_notif);
    it->second->set_last_check(last_check);
    it->second->set_next_check(next_check);
    it->second->set_last_state_change(last_state_change);
    it->second->set_last_hard_state_change(last_hard_state_change);
    it->second->set_last_state_history_update(last_state_history_update);
    // recalculate next re-notification time.
    it->second->set_next_notification(
    it->second->get_next_notification_time(
      it->second->get_last_notification()));

    // update the status data.
    it->second->update_status(false);
  }

  // adjust program timestamps.
  program_start = adjust_timestamp_for_time_change(
    last_time,
    current_time,
    time_difference,
    program_start);
  event_start = adjust_timestamp_for_time_change(
    last_time,
    current_time,
    time_difference,
    event_start);
  last_command_check = adjust_timestamp_for_time_change(
    last_time,
    current_time,
    time_difference,
    last_command_check);

  // update the status data.
  update_program_status(false);
}

/**
 *  Handles a timed event.
 *
 *  @param[in] event The event to execute.
 *
 *  @return OK.
 */
int handle_timed_event(timed_event* event) {
  typedef void (*exec_event)(timed_event*);
  static exec_event tab_exec_event[] = {
    &_exec_event_service_check,
    &_exec_event_command_check,
    &_exec_event_log_rotation,
    &_exec_event_program_shutdown,
    &_exec_event_program_restart,
    &_exec_event_check_reaper,
    &_exec_event_orphan_check,
    &_exec_event_retention_save,
    &_exec_event_status_save,
    &_exec_event_scheduled_downtime,
    &_exec_event_sfreshness_check,
    &_exec_event_expire_downtime,
    &_exec_event_host_check,
    &_exec_event_hfreshness_check,
    &_exec_event_reschedule_checks,
    &_exec_event_expire_comment,
    &_exec_event_expire_host_ack,
    &_exec_event_expire_service_ack,
    NULL
  };

  logger(dbg_functions, basic)
    << "handle_timed_event()";

  // send event data to broker.
  broker_timed_event(
    NEBTYPE_TIMEDEVENT_EXECUTE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    event,
    NULL);

  logger(dbg_events, basic)
    << "** Timed Event ** Type: " << event->event_type
    << ", Run Time: " << my_ctime(&event->run_time);

  // how should we handle the event?
  if (event->event_type < sizeof(tab_exec_event) / sizeof(*tab_exec_event))
    (tab_exec_event[event->event_type])(event);
  else if (event->event_type == EVENT_USER_FUNCTION)
    _exec_event_user_function(event);

  return OK;
}

/**
 *  Remove an event from the queue.
 *
 *  @param[in]     event           The event to remove.
 *  @param[in,out] event_list      The head of the event list.
 *  @param[in,out] event_list_tail The tail of the event list.
 */
void remove_event(
       timed_event* event,
       timed_event::priority priority) {
  logger(dbg_functions, basic)
    << "remove_event()";

  // send event data to broker.
  broker_timed_event(
    NEBTYPE_TIMEDEVENT_REMOVE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    event,
    NULL);

  if (!event)
    return;

  if (priority == timed_event::low) {
    quick_timed_event.erase(timed_event::low, event);
    timed_event::event_list_low.remove(event);
  } else {
    quick_timed_event.erase(timed_event::high, event);
    timed_event::event_list_high.remove(event);
  }
}

/**
 *  Reschedule an event in order of execution time.
 *
 *  @param[in]     event           The event to reschedule.
 *  @param[in,out] event_list      The head of the event list.
 *  @param[in,out] event_list_tail The tail of the event list.
 */
void reschedule_event(
       timed_event* event,
       timed_event::priority priority) {
  logger(dbg_functions, basic)
    << "reschedule_event()";

  // reschedule recurring events...
  if (event->recurring) {
    // use custom timing function.
    if (event->timing_func) {
      union {
        time_t (*func)(void);
        void* data;
      } timing;
      timing.data = event->timing_func;
      event->run_time = (*timing.func)();
    }

    // normal recurring events.
    else {
      time_t current_time(0L);
      event->run_time = event->run_time + event->event_interval;
      time(&current_time);
      if (event->run_time < current_time)
        event->run_time = current_time;
    }
  }

  // add the event to the event list.
  add_event(event, priority);
}

static bool compare_event(timed_event* const& first, timed_event* const& second)
{
  if (first->run_time < second->run_time)
    return true;
  return false;
}

/**
 *  Resorts an event list by event execution time - needed when
 *  compensating for system time changes.
 *
 *  @param[in,out] event_list      The head of the event list.
 *  @param[in,out] event_list_tail The tail of the event list.
 */
void resort_event_list(timed_event::priority priority) {
  timed_event_list *list;

  logger(dbg_functions, basic)
    << "resort_event_list()";

  // move current event list to temp list.
  if (priority == timed_event::low) {
    quick_timed_event.clear(timed_event::low);
    list = &timed_event::event_list_low;
  } else {
    quick_timed_event.clear(timed_event::high);
    list = &timed_event::event_list_high;
  }
  list->sort(compare_event);

  // send event data to broker.
  for (timed_event_list::iterator
         it{list->begin()},
         end{list->end()};
       it != end;
       ++it)
    broker_timed_event(
      NEBTYPE_TIMEDEVENT_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      (*it),
      nullptr);
}

/**
 *  Create and chedule a new timed event.
 *
 *  @param[in] event_type                 Event type id.
 *  @param[in] high_priority              Priority list.
 *  @param[in] run_time                   The run time event.
 *  @param[in] recurring                  If the event is recurring.
 *  @param[in] event_interval             The event interval.
 *  @param[in] timing_func                Function to call.
 *  @param[in] compensate_for_time_change If we need to compensate.
 *  @param[in] event_data                 The event data.
 *  @param[in] event_args                 The event args.
 *  @param[in] event_options              The event options.
 */
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
       int event_options) {
  schedule(
    event_type,
    high_priority,
    run_time,
    recurring,
    event_interval,
    timing_func,
    compensate_for_time_change,
    event_data,
    event_args,
    event_options);
}

/**
 *  Create and schedule a new timed event.
 *
 *  @param[in] event_type                 Event type id.
 *  @param[in] high_priority              Priority list.
 *  @param[in] run_time                   The run time event.
 *  @param[in] recurring                  If the event is recurring.
 *  @param[in] event_interval             The event interval.
 *  @param[in] timing_func                Function to call.
 *  @param[in] compensate_for_time_change If we need to compensate.
 *  @param[in] event_data                 The event data.
 *  @param[in] event_args                 The event args.
 *  @param[in] event_options              The event options.
 *
 *  @return The new timed event.
 */
timed_event* events::schedule(
               int event_type,
               int high_priority,
               time_t run_time,
               int recurring,
               unsigned long event_interval,
               void* timing_func,
               int compensate_for_time_change,
               void* event_data,
               void* event_args,
               int event_options) {
  logger(dbg_functions, basic)
    << "schedule_new_event()";

  timed_event* evt(new timed_event);
  evt->event_type = event_type;
  evt->event_data = event_data;
  evt->event_args = event_args;
  evt->event_options = event_options;
  evt->run_time = run_time;
  evt->recurring = recurring;
  evt->event_interval = event_interval;
  evt->timing_func = timing_func;
  evt->compensate_for_time_change = compensate_for_time_change;

  // add the event to the event list.
  if (high_priority)
    add_event(evt, timed_event::high);
  else
    add_event(evt, timed_event::low);
  return evt;
}

/**
 *  Get the event name.
 *
 *  @param[in] evt The event to get name.
 *
 *  @return The event name.
 */
std::string const& events::name(timed_event const& evt) {
  static std::string const event_unknown("\"unknown\"");
  static std::string const event_sleep("EVENT_SLEEP");
  static std::string const event_user_function("EVENT_USER_FUNCTION");
  static std::string const event_names[] = {
    "EVENT_SERVICE_CHECK",
    "EVENT_COMMAND_CHECK",
    "EVENT_LOG_ROTATION",
    "EVENT_PROGRAM_SHUTDOWN",
    "EVENT_PROGRAM_RESTART",
    "EVENT_CHECK_REAPER",
    "EVENT_ORPHAN_CHECK",
    "EVENT_RETENTION_SAVE",
    "EVENT_STATUS_SAVE",
    "EVENT_SCHEDULED_DOWNTIME",
    "EVENT_SFRESHNESS_CHECK",
    "EVENT_EXPIRE_DOWNTIME",
    "EVENT_HOST_CHECK",
    "EVENT_HFRESHNESS_CHECK",
    "EVENT_RESCHEDULE_CHECKS",
    "EVENT_EXPIRE_COMMENT",
    "EVENT_EXPIRE_HOST_ACK",
    "EVENT_EXPIRE_SERVICE_ACK"
  };

  if (evt.event_type < sizeof(event_names) / sizeof(event_names[0]))
    return event_names[evt.event_type];
  if (evt.event_type == EVENT_SLEEP)
    return event_sleep;
  if (evt.event_type == EVENT_USER_FUNCTION)
    return event_user_function;
  return event_unknown;
}

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       timed_event const& obj1,
       timed_event const& obj2) throw () {
  if (obj1.event_type != obj2.event_type)
    return false;

  bool is_not_null(obj1.event_data && obj2.event_data);
  if (is_not_null
      && ((obj1.event_type == EVENT_HOST_CHECK)
          || (obj1.event_type == EVENT_EXPIRE_HOST_ACK))) {
    host& hst1(*(host*)obj1.event_data);
    host& hst2(*(host*)obj2.event_data);
    if (hst1.get_name() != hst2.get_name())
      return false;
  }
  else if (is_not_null
           && ((obj1.event_type == EVENT_SERVICE_CHECK)
               || (obj1.event_type == EVENT_EXPIRE_SERVICE_ACK))) {
    com::centreon::engine::service& svc1(*(com::centreon::engine::service*)obj1.event_data);
    com::centreon::engine::service& svc2(*(com::centreon::engine::service*)obj2.event_data);
    if (svc1.get_hostname() != svc2.get_hostname()
        || svc1.get_description() != svc2.get_description())
      return false;
  }
  else if (is_not_null
           && (obj1.event_type == EVENT_SCHEDULED_DOWNTIME
               || obj1.event_type == EVENT_EXPIRE_COMMENT)) {
    unsigned long id1(*(unsigned long*)obj1.event_data);
    unsigned long id2(*(unsigned long*)obj2.event_data);
    if (id1 != id2)
      return false;
  }
  else if (obj1.event_data != obj2.event_data)
    return false;

  return obj1.run_time == obj2.run_time
          && obj1.recurring == obj2.recurring
          && obj1.event_interval == obj2.event_interval
          && obj1.compensate_for_time_change == obj2.compensate_for_time_change
          && obj1.timing_func == obj2.timing_func
          && obj1.event_args == obj2.event_args
          && obj1.event_options == obj2.event_options;
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool operator!=(
       timed_event const& obj1,
       timed_event const& obj2) throw () {
  return !operator==(obj1, obj2);
}

/**
 *  Dump command content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The command to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, timed_event const& obj) {
  os << "timed_event {\n"
    "  event_type:                 " << events::name(obj) << "\n"
    "  run_time:                   " << string::ctime(obj.run_time) << "\n"
    "  recurring:                  " << obj.recurring << "\n"
    "  event_interval:             " << obj.event_interval << "\n"
    "  compensate_for_time_change: " << obj.compensate_for_time_change << "\n"
    "  timing_func:                " << obj.timing_func << "\n";

  if (!obj.event_data)
    os << "  event_data:                 \"NULL\"\n";
  else if (obj.event_type == EVENT_HOST_CHECK
           || obj.event_type == EVENT_EXPIRE_HOST_ACK) {
    host& hst(*(host*)obj.event_data);
    os << "  event_data:                 "
       << hst.get_name() << "\n";
  }
  else if (obj.event_type == EVENT_SERVICE_CHECK
           || obj.event_type == EVENT_EXPIRE_SERVICE_ACK) {
    com::centreon::engine::service& svc(*(com::centreon::engine::service*)obj.event_data);
    os << "  event_data:                 "
       << svc.get_hostname() << ", " << svc.get_description() << "\n";
  }
  else if (obj.event_type == EVENT_SCHEDULED_DOWNTIME
           || obj.event_type == EVENT_EXPIRE_COMMENT) {
    unsigned long id(*(unsigned long*)obj.event_data);
    os << "  event_data:                 " << id << "\n";
  }
  else
    os << "  event_data:                 " << obj.event_data << "\n";

  os <<
    "  event_args:                 " << obj.event_args << "\n"
    "  event_options:              " << obj.event_options << "\n"
    "}\n";
  return os;
}

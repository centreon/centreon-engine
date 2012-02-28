/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2010 Nagios Core Development Team and Community Contributors
** Copyright 2011      Merethis
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

#include <QTimer>
#include <stdlib.h>
#include <time.h>
#include "globals.hh"
#include "broker.hh"
#include "statusdata.hh"
#include "logging/logger.hh"
#include "events/loop.hh"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::events;

loop* loop::_instance = NULL;

/**
 *  Get instance of the events loop singleton.
 *
 *  @return The singleton.
 */
loop& loop::instance() {
  if (_instance == NULL)
    _instance = new loop();
  return (*_instance);
}

/**
 *  Cleanup the loop singleton.
 */
void loop::cleanup() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
loop::loop()
  : QObject(),
    _app(QCoreApplication::instance()) {

}

/**
 *  Default destructor.
 */
loop::~loop() throw() {

}

/**
 *  Slot to dispatch Centreon Engine events.
 */
void loop::_dispatching() {
  bool quit = false;

  while (!quit) {
    // see if we should exit or restart (a signal was encountered).
    if (sigshutdown == true) {
      emit shutdown();
      return;
    }
    if (sigrestart == true) {
      emit restart();
      return;
    }

    // if we don't have any events to handle, exit.
    if (event_list_high == NULL && event_list_low == NULL) {
      logger(dbg_events, basic)
        << "There aren't any events that need to be handled! Exiting...";
      emit shutdown();
      return;
    }

    QCoreApplication::processEvents();

    // get the current time.
    time_t current_time;
    time(&current_time);

    // hey, wait a second...  we traveled back in time!
    if (current_time < _last_time)
      compensate_for_system_time_change((unsigned long)_last_time,
                                        (unsigned long)current_time);

    // else if the time advanced over the specified threshold, try and compensate...
    else if ((current_time - _last_time)
             >= static_cast<time_t>(config.get_time_change_threshold()))
      compensate_for_system_time_change((unsigned long)_last_time,
                                        (unsigned long)current_time);

    // keep track of the last time.
    _last_time = current_time;

    logger(dbg_events, more) << "** Event Check Loop";
    if (event_list_high != NULL)
      logger(dbg_events, more)
        << "Next High Priority Event Time: "
        << my_ctime(&event_list_high->run_time);
    else
      logger(dbg_events, more)
        << "No high priority events are scheduled...";
    if (event_list_low != NULL)
      logger(dbg_events, more)
        << "Next Low Priority Event Time:  "
        << my_ctime(&event_list_low->run_time);
    else
      logger(dbg_events, more)
        << "No low priority events are scheduled...";
    logger(dbg_events, more)
      << "Current/Max Service Checks: "
      << currently_running_service_checks << '/'
      << config.get_max_parallel_service_checks();

    bool run_event = true;

    // update status information occassionally - NagVis watches the NDOUtils DB to see if Engine is alive.
    if ((unsigned long)(current_time - _last_status_update) > 5) {
      _last_status_update = current_time;
      update_program_status(false);
    }

    // handle high priority events.
    if (event_list_high != NULL && (current_time >= event_list_high->run_time)) {
      // remove the first event from the timing loop.
      timed_event* temp_event = event_list_high;
      event_list_high = event_list_high->next;
      event_list_high->prev = NULL;
      quick_timed_event.erase(hash_timed_event::high, temp_event);

      // handle the event.
      handle_timed_event(temp_event);

      // reschedule the event if necessary.
      if (temp_event->recurring == true)
        reschedule_event(temp_event, &event_list_high, &event_list_high_tail);

      // else free memory associated with the event.
      else
        delete temp_event;
    }

    // handle low priority events.
    else if (event_list_low != NULL && (current_time >= event_list_low->run_time)) {

      // default action is to execute the event.
      run_event = true;
      int nudge_seconds = 0;

      // run a few checks before executing a service check...
      if (event_list_low->event_type == EVENT_SERVICE_CHECK) {
        service* temp_service = static_cast<service*>(event_list_low->event_data);

        // don't run a service check if we're already maxed out on the number of parallel service checks...
        if (config.get_max_parallel_service_checks() != 0
            && (currently_running_service_checks >= config.get_max_parallel_service_checks())) {

          // Move it at least 5 seconds (to overcome the current peak), with a random 10 seconds (to spread the load).
          nudge_seconds = 5 + (rand() % 10);
          logger(dbg_events | dbg_checks, basic)
            << "**WARNING** Max concurrent service checks ("
            << config.get_max_parallel_service_checks()
            << ") has been reached!  Nudging " << temp_service->host_name
            << ":" << temp_service->description << " by "
            << nudge_seconds << " seconds...";

          logger(log_runtime_warning, basic)
            << "\tMax concurrent service checks ("
            << config.get_max_parallel_service_checks()
            << ") has been reached.  Nudging " << temp_service->host_name
            << ":" << temp_service->description << " by " << nudge_seconds
            << " seconds...";
          run_event = false;
        }

        // don't run a service check if active checks are disabled.
        if (config.get_execute_service_checks() == false) {
          logger(dbg_events | dbg_checks, more)
            << "We're not executing service checks right now, so we'll skip this event.";

          run_event = false;
        }

        // forced checks override normal check logic.
        if ((temp_service->check_options & CHECK_OPTION_FORCE_EXECUTION))
          run_event = true;

        // reschedule the check if we can't run it now.
        if (run_event == false) {

          // remove the service check from the event queue and reschedule it for a later time.
          // 12/20/05 since event was not executed, it needs to be remove()'ed to maintain sync with event broker modules.
          timed_event* temp_event = event_list_low;
          remove_event(temp_event, &event_list_low, &event_list_low_tail);

          if (nudge_seconds) {
            // We nudge the next check time when it is due to too many concurrent service checks.
            temp_service->next_check = (time_t)(temp_service->next_check + nudge_seconds);
          }
          else {
            // Otherwise reschedule (TODO: This should be smarter as it doesn't consider its timeperiod).
            if (temp_service->state_type == SOFT_STATE
                && temp_service->current_state != STATE_OK)
              temp_service->next_check = (time_t)(temp_service->next_check
                                                  + (temp_service->retry_interval
                                                     * config.get_interval_length()));
            else
              temp_service->next_check = (time_t)(temp_service->next_check
                                                  + (temp_service->check_interval
                                                     * config.get_interval_length()));
          }

          temp_event->run_time = temp_service->next_check;
          reschedule_event(temp_event, &event_list_low, &event_list_low_tail);
          update_service_status(temp_service, false);

          run_event = false;
        }
      }

      // run a few checks before executing a host check...
      else if (event_list_low->event_type == EVENT_HOST_CHECK) {
        // default action is to execute the event.
        run_event = true;

        host* temp_host = static_cast<host*>(event_list_low->event_data);

        // don't run a host check if active checks are disabled.
        if (config.get_execute_host_checks() == false) {
          logger(dbg_events | dbg_checks, more)
            << "We're not executing host checks right now, so we'll skip this event.";

          run_event = false;
        }

        // forced checks override normal check logic.
        if ((temp_host->check_options & CHECK_OPTION_FORCE_EXECUTION))
          run_event = true;

        // reschedule the host check if we can't run it right now.
        if (run_event == false) {

          // remove the host check from the event queue and reschedule it for a later time.
          // 12/20/05 since event was not executed, it needs to be remove()'ed to maintain sync with event broker modules.
          timed_event* temp_event = event_list_low;
          remove_event(temp_event, &event_list_low, &event_list_low_tail);

          if (temp_host->state_type == SOFT_STATE
              && temp_host->current_state != STATE_OK)
            temp_host->next_check = (time_t)(temp_host->next_check
                                             + (temp_host->retry_interval
                                                * config.get_interval_length()));
          else
            temp_host->next_check = (time_t)(temp_host->next_check
                                             + (temp_host->check_interval
                                                * config.get_interval_length()));

          temp_event->run_time = temp_host->next_check;
          reschedule_event(temp_event, &event_list_low, &event_list_low_tail);
          update_host_status(temp_host, false);

          run_event = false;
        }
      }

      // run the event.
      if (run_event == true) {
        // remove the first event from the timing loop.
        timed_event* temp_event = event_list_low;
        event_list_low = event_list_low->next;
        // we may have just removed the only item from the list.
        if (event_list_low != NULL)
          event_list_low->prev = NULL;
        quick_timed_event.erase(hash_timed_event::low, temp_event);

        logger(dbg_events, more) << "Running event...";

        // handle the event.
        handle_timed_event(temp_event);

        // reschedule the event if necessary.
        if (temp_event->recurring == true)
          reschedule_event(temp_event, &event_list_low, &event_list_low_tail);

        // else free memory associated with the event.
        else
          delete temp_event;
      }

      // wait a while so we don't hog the CPU...
      else {
        logger(dbg_events, most)
          << "Did not execute scheduled event.  Idling for a bit...";
        quit = true;
      }
    }

    // we don't have anything to do at this moment in time...
    else
      if ((event_list_high == NULL
           || (current_time < event_list_high->run_time))
          && (event_list_low == NULL
              || (current_time < event_list_low->run_time))) {

        logger(dbg_events, most)
          << "No events to execute at the moment.  Idling for a bit...";

        // check for external commands if we're supposed to check as often as possible.
        if (config.get_command_check_interval() == -1) {
          // send data to event broker.
          broker_external_command(NEBTYPE_EXTERNALCOMMAND_CHECK,
                                  NEBFLAG_NONE,
                                  NEBATTR_NONE,
                                  CMD_NONE,
                                  time(NULL),
                                  NULL,
                                  NULL,
                                  NULL);
        }

        // set time to sleep so we don't hog the CPU...
        timespec sleep_time;
        sleep_time.tv_sec = (time_t)config.get_sleep_time();
        sleep_time.tv_nsec = (long)((config.get_sleep_time()
                                     - (double)sleep_time.tv_sec) * 1000000000);

        // populate fake "sleep" event.
        _sleep_event.run_time = current_time;
        _sleep_event.event_data = (void*)&sleep_time;

        // send event data to broker.
        broker_timed_event(NEBTYPE_TIMEDEVENT_SLEEP,
                           NEBFLAG_NONE,
                           NEBATTR_NONE,
                           &_sleep_event,
                           NULL);
        quit = true;
      }
  }

  int sec = static_cast<int>(config.get_sleep_time());
  int delay
    = sec * 1000
      + static_cast<int>((config.get_sleep_time() - sec) * 1000);
  QTimer::singleShot(delay, this, SLOT(_dispatching()));
}

/**
 *  Start the events loop thread.
 */
void loop::run() {
  logger(dbg_functions, basic) << "events::loop::run()";

  time(&_last_time);
  _last_status_update = 0L;

  // initialize fake "sleep" event.
  _sleep_event.event_type = EVENT_SLEEP;
  _sleep_event.run_time = _last_time;
  _sleep_event.recurring = false;
  _sleep_event.event_interval = 0L;
  _sleep_event.compensate_for_time_change = false;
  _sleep_event.timing_func = NULL;
  _sleep_event.event_data = NULL;
  _sleep_event.event_args = NULL;
  _sleep_event.event_options = 0;
  _sleep_event.next = NULL;
  _sleep_event.prev = NULL;

  QObject::connect(this, SIGNAL(shutdown()), _app, SLOT(quit()));
  QObject::connect(this, SIGNAL(restart()), _app, SLOT(quit()));
  QTimer::singleShot(0, this, SLOT(_dispatching()));
  _app->exec();
}

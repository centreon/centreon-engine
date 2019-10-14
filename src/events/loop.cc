/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2010 Nagios Core Development Team and Community Contributors
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

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <future>
#include <thread>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/loop.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/logging/engine.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::events;
using namespace com::centreon::engine::logging;

static loop* _instance = nullptr;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Get instance of the events loop singleton.
 *
 *  @return The singleton.
 */
loop& loop::instance() {
  assert(_instance);
  return *_instance;
}

/**
 *  Load singleton.
 */
void loop::load() {
  if (!_instance)
    _instance = new loop;
}

/**
 *  Start the events loop thread.
 */
void loop::run() {
  // Debug message.
  logger(dbg_functions, basic)
    << "events::loop::run()";

  logger(log_info_message, basic)
     << "Configuration loaded, main loop starting.";

  // Initialize some time members.
  time(&_last_time);
  _last_status_update = 0L;

  // Initialize fake "sleep" event.
  _sleep_event.event_type = EVENT_SLEEP;
  _sleep_event.run_time = _last_time;
  _sleep_event.recurring = false;
  _sleep_event.event_interval = 0L;
  _sleep_event.compensate_for_time_change = false;
  _sleep_event.timing_func = nullptr;
  _sleep_event.event_data = nullptr;
  _sleep_event.event_args = nullptr;
  _sleep_event.event_options = 0;

  _dispatching();
}

/**
 *  Unload singleton.
 */
void loop::unload() {
  delete _instance;
  _instance = nullptr;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
loop::loop()
  : _need_reload(0),
    _reload_running(false) {}

/**
 *  Destructor.
 */
loop::~loop() throw () {}

static void apply_conf(std::atomic<bool>* reloading) {
  logger(log_info_message, more)
    << "Starting to reload configuration.";
  try {
    configuration::state config;
    {
      configuration::parser p;
      std::string path(::config->cfg_main());
      p.parse(path, config);
    }
    configuration::applier::state::instance().apply(config);
    logger(log_info_message, basic)
      << "Configuration reloaded, main loop continuing.";
  }
  catch (std::exception const& e) {
    logger(log_config_error, most)
      << "Error: " << e.what();
  }
  *reloading = false;
  logger(log_info_message, more)
    << "Reload configuration finished.";
}

/**
 *  Slot to dispatch Centreon Engine events.
 */
void loop::_dispatching() {
  std::atomic<bool> reloading{false};
  while (true) {
    // See if we should exit or restart (a signal was encountered).
    if (sigshutdown)
      break;

    // If we don't have any events to handle, exit.
    if (timed_event::event_list_high.empty() && timed_event::event_list_low.empty()) {
      logger(log_runtime_error, basic)
        << "There aren't any events that need to be handled! "
        << "Exiting...";
      break;
    }

    if (sighup) {
      com::centreon::logging::engine::instance().reopen();
      ++_need_reload;
      sighup = false;
    }

    // Start reload configuration.
    if (_need_reload) {
      logger(log_info_message, most)
        << "Need reload.";
      if (!reloading) {
        logger(log_info_message, most)
          << "Reloading...";
        reloading = true;
        std::async(std::launch::async, apply_conf, &reloading);
      }
      else
        logger(log_info_message, most)
          << "Already reloading...";

      _need_reload = 0;
    }

    // Get the current time.
    time_t current_time;
    time(&current_time);

    configuration::applier::state::instance().lock();

    // Hey, wait a second...  we traveled back in time!
    if (current_time < _last_time)
      compensate_for_system_time_change(
        static_cast<unsigned long>(_last_time),
        static_cast<unsigned long>(current_time));
    // Else if the time advanced over the specified threshold,
    // try and compensate...
    else if ((current_time - _last_time)
             >= static_cast<time_t>(config->time_change_threshold()))
      compensate_for_system_time_change(
        static_cast<unsigned long>(_last_time),
        static_cast<unsigned long>(current_time));

    // Keep track of the last time.
    _last_time = current_time;

    // Log messages about event lists.
    logger(dbg_events, more)
      << "** Event Check Loop";
    if (!timed_event::event_list_high.empty())
      logger(dbg_events, more)
        << "Next High Priority Event Time: "
        << my_ctime(&(*timed_event::event_list_high.begin())->run_time);
    else
      logger(dbg_events, more)
        << "No high priority events are scheduled...";
    if (!timed_event::event_list_low.empty())
      logger(dbg_events, more)
        << "Next Low Priority Event Time:  "
        << my_ctime(&(*timed_event::event_list_low.begin())->run_time);
    else
      logger(dbg_events, more)
        << "No low priority events are scheduled...";
    logger(dbg_events, more)
      << "Current/Max Service Checks: "
      << currently_running_service_checks << '/'
      << config->max_parallel_service_checks();

    // Update status information occassionally - NagVis watches the
    // NDOUtils DB to see if Engine is alive.
    if ((unsigned long)(current_time - _last_status_update) > 5) {
      _last_status_update = current_time;
      update_program_status(false);
    }

    // Handle high priority events.
    bool run_event(true);
    if (!timed_event::event_list_high.empty()
        && (current_time >= (*timed_event::event_list_high.begin())->run_time)) {
      // Remove the first event from the timing loop.
      timed_event* temp_event(*timed_event::event_list_high.begin());

      timed_event::event_list_high.pop_front();
      // We may have just removed the only item from the list.

      // Handle the event.
      handle_timed_event(temp_event);

      // Reschedule the event if necessary.
      if (temp_event->recurring)
        reschedule_event(temp_event, timed_event::high);
      // Else free memory associated with the event.
      else
        delete temp_event;
    }
    // Handle low priority events.
    else if (!timed_event::event_list_low.empty()
             && (current_time >= (*timed_event::event_list_low.begin())->run_time)) {
      // Default action is to execute the event.
      run_event = true;

      // Run a few checks before executing a service check...
      if ((*timed_event::event_list_low.begin())->event_type == EVENT_SERVICE_CHECK) {
        int nudge_seconds(0);
        service* temp_service(
                   static_cast<service*>((*timed_event::event_list_low.begin())->event_data));

        // Don't run a service check if we're already maxed out on the
        // number of parallel service checks...
        if (config->max_parallel_service_checks() != 0
            && (currently_running_service_checks
                >= config->max_parallel_service_checks())) {
          // Move it at least 5 seconds (to overcome the current peak),
          // with a random 10 seconds (to spread the load).
          nudge_seconds = 5 + (rand() % 10);
          logger(dbg_events | dbg_checks, basic)
            << "**WARNING** Max concurrent service checks ("
            << currently_running_service_checks << "/"
            << config->max_parallel_service_checks()
            << ") has been reached!  Nudging "
            << temp_service->get_hostname() << ":"
            << temp_service->get_description() << " by "
            << nudge_seconds << " seconds...";
          logger(log_runtime_warning, basic)
            << "\tMax concurrent service checks ("
            << currently_running_service_checks << "/"
            << config->max_parallel_service_checks()
            << ") has been reached.  Nudging "
            << temp_service->get_hostname() << ":"
            << temp_service->get_description() << " by "
            << nudge_seconds << " seconds...";
          run_event = false;
        }

        // Don't run a service check if active checks are disabled.
        if (!config->execute_service_checks()) {
          logger(dbg_events | dbg_checks, more)
            << "We're not executing service checks right now, "
            << "so we'll skip this event.";
          run_event = false;
        }

        // Forced checks override normal check logic.
        if (temp_service->get_check_options() & CHECK_OPTION_FORCE_EXECUTION)
          run_event = true;

        // Reschedule the check if we can't run it now.
        if (!run_event) {
          // Remove the service check from the event queue and
          // reschedule it for a later time. Since event was not
          // executed, it needs to be remove()'ed to maintain sync with
          // event broker modules.
          timed_event* temp_event{*timed_event::event_list_low.begin()};
          remove_event(temp_event, timed_event::low);

          // We nudge the next check time when it is
          // due to too many concurrent service checks.
          if (nudge_seconds)
            temp_service->set_next_check(
              (time_t)(temp_service->get_next_check() + nudge_seconds));
          // Otherwise reschedule (TODO: This should be smarter as it
          // doesn't consider its timeperiod).
          else {
            if (notifier::soft == temp_service->get_state_type() &&
                temp_service->get_current_state() != service::state_ok)
              temp_service->set_next_check(
                  (time_t)(temp_service->get_next_check() +
                           temp_service->get_retry_interval() *
                               config->interval_length()));
            else
              temp_service->set_next_check(
                  (time_t)(temp_service->get_next_check() +
                           (temp_service->get_check_interval() *
                            config->interval_length())));
          }
          temp_event->run_time = temp_service->get_next_check();
          reschedule_event(temp_event, timed_event::low);
          temp_service->update_status(false);
          run_event = false;
        }
      }
      // Run a few checks before executing a host check...
      else if (EVENT_HOST_CHECK == (*timed_event::event_list_low.begin())->event_type) {
        // Default action is to execute the event.
        run_event = true;
        host* temp_host(static_cast<host*>((*timed_event::event_list_low.begin())->event_data));

        // Don't run a host check if active checks are disabled.
        if (!config->execute_host_checks()) {
          logger(dbg_events | dbg_checks, more)
            << "We're not executing host checks right now, "
            << "so we'll skip this event.";
          run_event = false;
        }

        // Forced checks override normal check logic.
        if (temp_host->get_check_options() & CHECK_OPTION_FORCE_EXECUTION)
          run_event = true;

        // Reschedule the host check if we can't run it right now.
        if (!run_event) {
          // Remove the host check from the event queue and reschedule
          // it for a later time. Since event was not executed, it needs
          // to be remove()'ed to maintain sync with event broker
          // modules.
          timed_event* temp_event(*timed_event::event_list_low.begin());
          remove_event(temp_event, timed_event::low);

          // Reschedule.
          if ((notifier::soft == temp_host->get_state_type())
              && (temp_host->get_current_state() != host::state_up))
            temp_host->set_next_check(
              (time_t)(temp_host->get_next_check()
                         + (temp_host->get_retry_interval()
                            * config->interval_length())));
          else
            temp_host->set_next_check(
              (time_t)(temp_host->get_next_check()
                         + (temp_host->get_check_interval()
                            * config->interval_length())));
          temp_event->run_time = temp_host->get_next_check();
          reschedule_event(temp_event, timed_event::low);
          temp_host->update_status(false);
          run_event = false;
        }
      }

      // Run the event.
      if (run_event) {
        // Remove the first event from the timing loop.
        timed_event* temp_event(*timed_event::event_list_low.begin());
        timed_event::event_list_low.pop_front();
        // We may have just removed the only item from the list.

        // Handle the event.
        logger(dbg_events, more)
          << "Running event...";
        handle_timed_event(temp_event);

        // Reschedule the event if necessary.
        if (temp_event->recurring)
          reschedule_event(temp_event, timed_event::low);
        // Else free memory associated with the event.
        else
          delete temp_event;
      }
      // Wait a while so we don't hog the CPU...
      else {
        logger(dbg_events, most)
          << "Did not execute scheduled event. Idling for a bit...";
        uint64_t d = static_cast<uint64_t>(config->sleep_time() * 1000000000);
        std::this_thread::sleep_for(std::chrono::nanoseconds(d));
      }
    }
    // We don't have anything to do at this moment in time...
    else if ((timed_event::event_list_high.empty() ||
              current_time <
               (*timed_event::event_list_high.begin())->run_time) &&
             (timed_event::event_list_low.empty() ||
              current_time <
               (*timed_event::event_list_low.begin())->run_time)) {
      logger(dbg_events, most)
          << "No events to execute at the moment. Idling for a bit...";

      // Check for external commands if we're supposed to check as
      // often as possible.
      if (config->command_check_interval() == -1) {
        // Send data to event broker.
        broker_external_command(NEBTYPE_EXTERNALCOMMAND_CHECK, NEBFLAG_NONE,
                                NEBATTR_NONE, CMD_NONE, time(nullptr), nullptr,
                                nullptr, nullptr);
      }

      // Set time to sleep so we don't hog the CPU...
      timespec sleep_time;
      sleep_time.tv_sec = (time_t)config->sleep_time();
      sleep_time.tv_nsec =
          (long)((config->sleep_time() - (double)sleep_time.tv_sec) *
                 1000000000ull);

      // Populate fake "sleep" event.
      _sleep_event.run_time = current_time;
      _sleep_event.event_data = (void*)&sleep_time;

      // Send event data to broker.
      broker_timed_event(NEBTYPE_TIMEDEVENT_SLEEP, NEBFLAG_NONE, NEBATTR_NONE,
                         &_sleep_event, nullptr);

      // Wait a while so we don't hog the CPU...
      uint64_t d = static_cast<uint64_t>(config->sleep_time() * 1000000000);
      std::this_thread::sleep_for(std::chrono::nanoseconds(d));
    }
    configuration::applier::state::instance().unlock();
  }
}

/*
** Copyright 2011-2013,2015-2016 Centreon
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

#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/notifications.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/retention/applier/service.hh"
#include "com/centreon/engine/retention/applier/utils.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::retention;

/**
 *  Update service list.
 *
 *  @param[in] config                The global configuration.
 *  @param[in] lst                   The service list to update.
 *  @param[in] scheduling_info_is_ok True if the retention is not
 *                                   outdated.
 */
void applier::service::apply(
       configuration::state const& config,
       list_service const& lst,
       bool scheduling_info_is_ok) {
  for (list_service::const_iterator it(lst.begin()), end(lst.end());
       it != end;
       ++it) {
    try {
      std::pair<unsigned int, unsigned int> id(get_host_and_service_id(
            (*it)->host_name().c_str(),
            (*it)->service_description().c_str()));
      service_struct& svc(find_service(id.first, id.second));
      _update(config, **it, svc, scheduling_info_is_ok);
    }
    catch (...) {
      // ignore exception for the retention.
    }
  }
}

/**
 *  Update internal service base on service retention.
 *
 *  @param[in]      config                The global configuration.
 *  @param[in]      state                 The service retention state.
 *  @param[in, out] obj                   The service to update.
 *  @param[in]      scheduling_info_is_ok True if the retention is
 *                                        not outdated.
 */
void applier::service::_update(
       configuration::state const& config,
       retention::service const& state,
       service_struct& obj,
       bool scheduling_info_is_ok) {
  if (state.modified_attributes().is_set()) {
    obj.modified_attributes = *state.modified_attributes();
    // mask out attributes we don't want to retain.
    obj.modified_attributes &= ~config.retained_host_attribute_mask();
  }

  if (obj.retain_status_information) {
    if (state.has_been_checked().is_set())
      obj.has_been_checked = *state.has_been_checked();
    if (state.check_execution_time().is_set())
      obj.execution_time = *state.check_execution_time();
    if (state.check_latency().is_set())
      obj.latency = *state.check_latency();
    if (state.check_type().is_set())
      obj.check_type = *state.check_type();
    if (state.current_state().is_set())
      obj.current_state = *state.current_state();
    if (state.last_state().is_set())
      obj.last_state = *state.last_state();
    if (state.last_hard_state().is_set())
      obj.last_hard_state = *state.last_hard_state();
    if (state.current_attempt().is_set())
      obj.current_attempt = *state.current_attempt();
    if (state.current_event_id().is_set())
      obj.current_event_id = *state.current_event_id();
    if (state.last_event_id().is_set())
      obj.last_event_id = *state.last_event_id();
    if (state.current_problem_id().is_set())
      obj.current_problem_id = *state.current_problem_id();
    if (state.last_problem_id().is_set())
      obj.last_problem_id = *state.last_problem_id();
    if (state.state_type().is_set())
      obj.state_type = *state.state_type();
    if (state.last_state_change().is_set())
      obj.last_state_change = *state.last_state_change();
    if (state.last_hard_state_change().is_set())
      obj.last_hard_state_change = *state.last_hard_state_change();
    if (state.last_time_ok().is_set())
      obj.last_time_ok = *state.last_time_ok();
    if (state.last_time_warning().is_set())
      obj.last_time_warning = *state.last_time_warning();
    if (state.last_time_unknown().is_set())
      obj.last_time_unknown = *state.last_time_unknown();
    if (state.last_time_critical().is_set())
      obj.last_time_critical = *state.last_time_critical();
    if (state.plugin_output().is_set())
      string::setstr(obj.plugin_output, *state.plugin_output());
    if (state.long_plugin_output().is_set())
      string::setstr(obj.long_plugin_output, *state.long_plugin_output());
    if (state.performance_data().is_set())
      string::setstr(obj.perf_data, *state.performance_data());
    if (state.last_acknowledgement().is_set())
      service_other_props[std::make_pair(obj.host_ptr->name, obj.description)].last_acknowledgement
        = *state.last_acknowledgement();
    if (state.last_check().is_set())
      obj.last_check = *state.last_check();
    if (state.next_check().is_set()
        && config.use_retained_scheduling_info()
        && scheduling_info_is_ok)
      obj.next_check = *state.next_check();
    if (state.check_options().is_set()
        && config.use_retained_scheduling_info()
        && scheduling_info_is_ok)
      obj.check_options = *state.check_options();
    if (state.notified_on_unknown().is_set())
      obj.notified_on_unknown = *state.notified_on_unknown();
    if (state.notified_on_warning().is_set())
      obj.notified_on_warning = *state.notified_on_warning();
    if (state.notified_on_critical().is_set())
      obj.notified_on_critical = *state.notified_on_critical();
    if (state.current_notification_number().is_set())
      obj.current_notification_number = *state.current_notification_number();
    if (state.current_notification_id().is_set())
      obj.current_notification_id = *state.current_notification_id();
    if (state.last_notification().is_set())
      obj.last_notification = *state.last_notification();
    if (state.percent_state_change().is_set())
      obj.percent_state_change = *state.percent_state_change();
    if (state.check_flapping_recovery_notification().is_set())
      obj.check_flapping_recovery_notification = *state.check_flapping_recovery_notification();
    if (state.state_history().is_set()) {
      utils::set_state_history(
        *state.state_history(),
        obj.state_history);
      obj.state_history_index = 0;
    }
  }

  if (obj.retain_nonstatus_information) {
    if (state.problem_has_been_acknowledged().is_set())
      obj.problem_has_been_acknowledged = *state.problem_has_been_acknowledged();

    if (state.acknowledgement_type().is_set())
      obj.acknowledgement_type = *state.acknowledgement_type();

    if (state.notifications_enabled().is_set()
        && (obj.modified_attributes & MODATTR_NOTIFICATIONS_ENABLED))
      obj.notifications_enabled = *state.notifications_enabled();

    if (state.active_checks_enabled().is_set()
        && (obj.modified_attributes & MODATTR_ACTIVE_CHECKS_ENABLED))
      obj.checks_enabled = *state.active_checks_enabled();

    if (state.passive_checks_enabled().is_set()
        && (obj.modified_attributes & MODATTR_PASSIVE_CHECKS_ENABLED))
      obj.accept_passive_service_checks = *state.passive_checks_enabled();

    if (state.event_handler_enabled().is_set()
        && (obj.modified_attributes & MODATTR_EVENT_HANDLER_ENABLED))
      obj.event_handler_enabled = *state.event_handler_enabled();

    if (state.flap_detection_enabled().is_set()
        && (obj.modified_attributes & MODATTR_FLAP_DETECTION_ENABLED))
      obj.flap_detection_enabled = *state.flap_detection_enabled();

    if (state.process_performance_data().is_set()
        && (obj.modified_attributes & MODATTR_PERFORMANCE_DATA_ENABLED))
      obj.process_performance_data = *state.process_performance_data();

    if (state.obsess_over_service().is_set()
        && (obj.modified_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED))
      obj.obsess_over_service = *state.obsess_over_service();

    if (state.check_command().is_set()
        && (obj.modified_attributes & MODATTR_CHECK_COMMAND)) {
      if (utils::is_command_exist(*state.check_command()))
        string::setstr(obj.service_check_command, *state.check_command());
      else
        obj.modified_attributes -= MODATTR_CHECK_COMMAND;
    }

    if (state.check_period().is_set()
        && (obj.modified_attributes & MODATTR_CHECK_TIMEPERIOD)) {
      if (is_timeperiod_exist(*state.check_period()))
        string::setstr(obj.check_period, *state.check_period());
      else
        obj.modified_attributes -= MODATTR_CHECK_TIMEPERIOD;
    }

    if (state.notification_period().is_set()
        && (obj.modified_attributes & MODATTR_NOTIFICATION_TIMEPERIOD)) {
      if (is_timeperiod_exist(*state.notification_period()))
        string::setstr(obj.notification_period, *state.notification_period());
      else
        obj.modified_attributes -= MODATTR_NOTIFICATION_TIMEPERIOD;
    }

    if (state.event_handler().is_set()
        && (obj.modified_attributes & MODATTR_EVENT_HANDLER_COMMAND)) {
      if (utils::is_command_exist(*state.event_handler()))
        string::setstr(obj.event_handler, *state.event_handler());
      else
        obj.modified_attributes -= MODATTR_EVENT_HANDLER_COMMAND;
    }

    if (state.normal_check_interval().is_set()
        && (obj.modified_attributes & MODATTR_NORMAL_CHECK_INTERVAL))
      obj.check_interval = *state.normal_check_interval();

    if (state.retry_check_interval().is_set()
        && (obj.modified_attributes & MODATTR_RETRY_CHECK_INTERVAL))
      obj.retry_interval = *state.retry_check_interval();

    if (state.max_attempts().is_set()
        && (obj.modified_attributes & MODATTR_MAX_CHECK_ATTEMPTS)) {
      obj.max_attempts = *state.max_attempts();

      // adjust current attempt number if in a hard state.
      if (obj.state_type == HARD_STATE
          && obj.current_state != STATE_OK
          && obj.current_attempt > 1)
        obj.current_attempt = obj.max_attempts;
    }

    if (!state.customvariables().empty()
        && (obj.modified_attributes & MODATTR_CUSTOM_VARIABLE)) {
      for (map_customvar::const_iterator
             it(state.customvariables().begin()),
             end(state.customvariables().end());
           it != end;
           ++it) {
        update_customvariable(
          obj.custom_variables,
          it->first,
          it->second.get_value());
      }
    }
  }
  // Adjust modified attributes if necessary.
  else
    obj.modified_attributes = MODATTR_NONE;

  bool allow_flapstart_notification(true);

  // Adjust modified attributes if no custom variable has been changed.
  if (obj.modified_attributes & MODATTR_CUSTOM_VARIABLE) {
    bool at_least_one_modified(false);
    for (customvariablesmember* member(obj.custom_variables);
         member;
         member = member->next)
      if (member->has_been_modified)
        at_least_one_modified = true;
    if (!at_least_one_modified)
      obj.modified_attributes -= MODATTR_CUSTOM_VARIABLE;
  }

  // calculate next possible notification time.
  if (obj.current_state != STATE_OK && obj.last_notification)
    obj.next_notification
      = get_next_service_notification_time(
          &obj,
          obj.last_notification);

  // fix old vars.
  if (!obj.has_been_checked && obj.state_type == SOFT_STATE)
    obj.state_type = HARD_STATE;

  // ADDED 01/23/2009 adjust current check attempt if service is
  // in hard problem state (max attempts may have changed in config
  // since restart).
  if (obj.current_state != STATE_OK && obj.state_type == HARD_STATE)
    obj.current_attempt = obj.max_attempts;


  // ADDED 02/20/08 assume same flapping state if large
  // install tweaks enabled.
  if (config.use_large_installation_tweaks())
    obj.is_flapping = state.is_flapping();
  // else use normal startup flap detection logic.
  else {
    // service was flapping before program started.
    // 11/10/07 don't allow flapping notifications to go out.
    allow_flapstart_notification = !state.is_flapping();

    // check for flapping.
    check_for_service_flapping(
      &obj,
      false,
      allow_flapstart_notification);

    // service was flapping before and isn't now, so clear
    // recovery check variable if service isn't flapping now.
    if (state.is_flapping() && !obj.is_flapping)
      obj.check_flapping_recovery_notification = false;
  }

  // handle new vars added in 2.x.
  if (obj.last_hard_state_change)
    obj.last_hard_state_change = obj.last_state_change;

  // Handle recovery been sent
  if (state.recovery_been_sent().is_set())
    service_other_props[std::make_pair(obj.description,
                                       obj.host_ptr->name)].recovery_been_sent
      = *state.recovery_been_sent();

  // update service status.
  update_service_status(&obj, false);
}

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

#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/retention/applier/host.hh"
#include "com/centreon/engine/retention/applier/utils.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::retention;

/**
 *  Update host list.
 *
 *  @param[in] config                The global configuration.
 *  @param[in] lst                   The host list to update.
 *  @param[in] scheduling_info_is_ok True if the retention is not
 *                                   outdated.
 */
void applier::host::apply(
       configuration::state const& config,
       list_host const& lst,
       bool scheduling_info_is_ok) {
  (void)config;
  for (list_host::const_iterator it(lst.begin()), end(lst.end());
       it != end;
       ++it) {
    try {
      host_struct& hst(find_host((*it)->host_name()));
      _update(**it, hst, scheduling_info_is_ok);
    }
    catch (...) {
      // Ignore exceptions from the retention.
    }
  }
}

/**
 *  Update internal host base on host retention.
 *
 *  @param[in]      state                 The host retention state.
 *  @param[in, out] obj                   The host to update.
 *  @param[in]      scheduling_info_is_ok True if the retention is
 *                                        not outdated.
 */
void applier::host::_update(
                      retention::host const& state,
                      host_struct& obj,
                      bool scheduling_info_is_ok) {
  if (state.modified_attributes().is_set())
    obj.modified_attributes = *state.modified_attributes();
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
  if (state.plugin_output().is_set())
    string::setstr(obj.plugin_output, *state.plugin_output());
  if (state.long_plugin_output().is_set())
    string::setstr(obj.long_plugin_output, *state.long_plugin_output());
  if (state.performance_data().is_set())
    string::setstr(obj.perf_data, *state.performance_data());
  if (state.last_check().is_set())
    obj.last_check = *state.last_check();
  if (state.next_check().is_set() && scheduling_info_is_ok)
    obj.next_check = *state.next_check();
  if (state.check_options().is_set() && scheduling_info_is_ok)
    obj.check_options = *state.check_options();
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
  if (state.last_time_up().is_set())
    obj.last_time_up = *state.last_time_up();
  if (state.last_time_down().is_set())
    obj.last_time_down = *state.last_time_down();
  if (state.last_time_unreachable().is_set())
    obj.last_time_unreachable = *state.last_time_unreachable();
  if (state.percent_state_change().is_set())
    obj.percent_state_change = *state.percent_state_change();
  if (state.state_history().is_set()) {
    utils::set_state_history(
                             *state.state_history(),
                             obj.state_history);
    obj.state_history_index = 0;
  }
  if (state.active_checks_enabled().is_set()
      && (obj.modified_attributes & MODATTR_ACTIVE_CHECKS_ENABLED))
    obj.checks_enabled = *state.active_checks_enabled();

  if (state.event_handler_enabled().is_set()
      && (obj.modified_attributes & MODATTR_EVENT_HANDLER_ENABLED))
    obj.event_handler_enabled = *state.event_handler_enabled();

  if (state.flap_detection_enabled().is_set()
      && (obj.modified_attributes & MODATTR_FLAP_DETECTION_ENABLED))
    obj.flap_detection_enabled = *state.flap_detection_enabled();

  if (state.obsess_over_host().is_set()
      && (obj.modified_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED))
    obj.obsess_over_host = *state.obsess_over_host();

  if (state.check_command().is_set()
      && (obj.modified_attributes & MODATTR_CHECK_COMMAND)) {
    if (utils::is_command_exist(*state.check_command()))
      string::setstr(obj.host_check_command, *state.check_command());
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

  if (state.event_handler().is_set()
      && (obj.modified_attributes & MODATTR_EVENT_HANDLER_COMMAND)) {
    if (utils::is_command_exist(*state.event_handler()))
      string::setstr(obj.host_check_command, *state.event_handler());
    else
      obj.modified_attributes -= MODATTR_CHECK_COMMAND;
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
        && obj.current_state != HOST_UP
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
                            it->second);
    }
  }

  // adjust modified attributes if no custom variables
  // have been changed.
  if (obj.modified_attributes & MODATTR_CUSTOM_VARIABLE) {
    for (customvariablesmember* member(obj.custom_variables);
         member;
         member = member->next)
      if (member->has_been_modified) {
        obj.modified_attributes -= MODATTR_CUSTOM_VARIABLE;
        break;
      }
  }

  // ADDED 01/23/2009 adjust current check attempts if host in hard
  // problem state (max attempts may have changed in config
  // since restart).
  if (obj.current_state != HOST_UP && obj.state_type == HARD_STATE)
    obj.current_attempt = obj.max_attempts;

  // Assume same flapping state.
  obj.is_flapping = state.is_flapping();

  // handle new vars added in 2.x.
  if (!obj.last_hard_state_change)
    obj.last_hard_state_change = obj.last_state_change;

  // Update host status.
  update_host_status(&obj);
}

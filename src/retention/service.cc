/*
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

#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/flapping.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/retention/service.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine;

/**
 *  Constructor.
 *
 *  @param[in] obj The service to use for retention.
 */
retention::service::service(service_struct* obj)
  : object(object::service),
    _obj(obj),
    _was_flapping(false) {

}

/**
 *  Destructor.
 */
retention::service::~service() throw () {
  _finished();
}

/**
 *  Set new value on specific property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool retention::service::set(
       std::string const& key,
       std::string const& value) {
  if (!_obj) {
    bool ret(false);
    if (key == "host_name") {
      _host_name = value;
      ret = true;
    }
    else if (key == "service_description") {
      _service_description = value;
      ret = true;
    }

    if (ret && !_host_name.empty() && !_service_description.empty()) {
      umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
        it(state::instance().services().find(std::make_pair(_host_name, _service_description)));
      if (it != state::instance().services().end())
        _obj = &*it->second;
    }
    return (ret);
  }
  if (_modified_attributes(key, value))
    return (true);
  if (_retain_status_information(key, value))
    return (true);
  return (_retain_nonstatus_information(key, value));
}

/**
 *  Finish all service update.
 */
void retention::service::_finished() throw () {
  if (!_obj)
    return;

  bool allow_flapstart_notification(true);

  // adjust modified attributes if necessary.
  if (!_obj->retain_nonstatus_information)
    _obj->modified_attributes = MODATTR_NONE;

  // adjust modified attributes if no custom variables
  // have been changed.
  if (_obj->modified_attributes & MODATTR_CUSTOM_VARIABLE) {
    for (customvariablesmember* member(_obj->custom_variables);
         member;
         member = member->next)
      if (member->has_been_modified) {
        _obj->modified_attributes -= MODATTR_CUSTOM_VARIABLE;
        break;
      }
  }

  // calculate next possible notification time.
  if (_obj->current_state != STATE_OK && _obj->last_notification)
    _obj->next_notification
      = get_next_service_notification_time(
          _obj,
          _obj->last_notification);

  // fix old vars.
  if (!_obj->has_been_checked && _obj->state_type == SOFT_STATE)
    _obj->state_type = HARD_STATE;

  // ADDED 01/23/2009 adjust current check attempt if service is
  // in hard problem state (max attempts may have changed in config
  // since restart).
  if (_obj->current_state != STATE_OK && _obj->state_type == HARD_STATE)
    _obj->current_attempt = _obj->max_attempts;


  // ADDED 02/20/08 assume same flapping state if large
  // install tweaks enabled.
  if (config->use_large_installation_tweaks())
    _obj->is_flapping = _was_flapping;
  // else use normal startup flap detection logic.
  else {
    // service was flapping before program started.
    // 11/10/07 don't allow flapping notifications to go out.
    allow_flapstart_notification = (_was_flapping ? false : true);

    // check for flapping.
    check_for_service_flapping(
      _obj,
      false,
      allow_flapstart_notification);

    // service was flapping before and isn't now, so clear
    // recovery check variable if service isn't flapping now.
    if (_was_flapping && !_obj->is_flapping)
      _obj->check_flapping_recovery_notification = false;
  }

  // handle new vars added in 2.x.
  if (_obj->last_hard_state_change)
    _obj->last_hard_state_change = _obj->last_state_change;

  // update service status.
  update_service_status(_obj, false);
}

/**
 *  Set new value on specific modified attrivute property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool retention::service::_modified_attributes(
       std::string const& key,
       std::string const& value) {
  if (key == "modified_attributes") {
    string::to(value, _obj->modified_attributes);
    // mask out attributes we don't want to retain.
    _obj->modified_attributes
      &= ~config->retained_host_attribute_mask();
  }
  return (true);
}

/**
 *  Set new value on specific retain nonstatus information property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool retention::service::_retain_nonstatus_information(
       std::string const& key,
       std::string const& value) {
  if (!_obj->retain_nonstatus_information)
    return (false);

  if (key == "problem_has_been_acknowledged")
    string::to<bool, int>(value, _obj->problem_has_been_acknowledged);
  else if (key == "acknowledgement_type")
    string::to(value, _obj->acknowledgement_type);
  else if (key == "notifications_enabled") {
    if (_obj->modified_attributes & MODATTR_NOTIFICATIONS_ENABLED)
      string::to<bool, int>(value, _obj->notifications_enabled);
  }
  else if (key == "active_checks_enabled") {
    if (_obj->modified_attributes & MODATTR_ACTIVE_CHECKS_ENABLED)
      string::to<bool, int>(value, _obj->checks_enabled);
  }
  else if (key == "passive_checks_enabled") {
    if (_obj->modified_attributes & MODATTR_PASSIVE_CHECKS_ENABLED)
      string::to<bool, int>(value, _obj->accept_passive_service_checks);
  }
  else if (key == "event_handler_enabled") {
    if (_obj->modified_attributes & MODATTR_EVENT_HANDLER_ENABLED)
      string::to<bool, int>(value, _obj->event_handler_enabled);
  }
  else if (key == "flap_detection_enabled") {
    if (_obj->modified_attributes & MODATTR_FLAP_DETECTION_ENABLED)
      string::to<bool, int>(value, _obj->flap_detection_enabled);
  }
  else if (key == "failure_prediction_enabled") {
    if (_obj->modified_attributes & MODATTR_FAILURE_PREDICTION_ENABLED)
      string::to<bool, int>(value, _obj->failure_prediction_enabled);
  }
  else if (key == "process_performance_data") {
    if (_obj->modified_attributes & MODATTR_PERFORMANCE_DATA_ENABLED)
      string::to<bool, int>(value, _obj->process_performance_data);
  }
  else if (key == "obsess_over_service") {
    if (_obj->modified_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED)
      string::to<bool, int>(value, _obj->obsess_over_service);
  }
  else if (key == "check_command") {
    if (_obj->modified_attributes & MODATTR_CHECK_COMMAND) {
      std::size_t pos(value.find('!'));
      if (pos != std::string::npos) {
        std::string command(value.substr(pos + 1));
        if (!find_command(command.c_str()))
          _obj->modified_attributes -= MODATTR_CHECK_COMMAND;
        else
          string::setstr(_obj->service_check_command, value);
      }
    }
  }
  else if (key == "check_period") {
    if (_obj->modified_attributes & MODATTR_CHECK_TIMEPERIOD) {
      if (!find_timeperiod(value.c_str()))
        _obj->modified_attributes -= MODATTR_CHECK_TIMEPERIOD;
      else
        string::setstr(_obj->check_period, value);
    }
  }
  else if (key == "notification_period") {
    if (_obj->modified_attributes & MODATTR_NOTIFICATION_TIMEPERIOD) {
      if (!find_timeperiod(value.c_str()))
        _obj->modified_attributes -= MODATTR_NOTIFICATION_TIMEPERIOD;
      else
        string::setstr(_obj->notification_period, value);
    }
  }
  else if (key == "event_handler") {
    if (_obj->modified_attributes & MODATTR_EVENT_HANDLER_COMMAND) {
      std::size_t pos(value.find('!'));
      if (pos != std::string::npos) {
        std::string command(value.substr(pos + 1));
        if (!find_command(command.c_str()))
          _obj->modified_attributes -= MODATTR_EVENT_HANDLER_COMMAND;
        else
          string::setstr(_obj->event_handler, value);
      }
    }
  }
  else if (key == "normal_check_interval") {
    if (_obj->modified_attributes & MODATTR_NORMAL_CHECK_INTERVAL) {
      double val;
      if (string::to(value, val) && val >= 0)
        _obj->check_interval = val;
    }
  }
  else if (key == "retry_check_interval") {
    if (_obj->modified_attributes & MODATTR_RETRY_CHECK_INTERVAL) {
      double val;
      if (string::to(value, val) && val >= 0)
        _obj->retry_interval = val;
    }
  }
  else if (key == "max_attempts") {
    if (_obj->modified_attributes & MODATTR_MAX_CHECK_ATTEMPTS) {
      int val;
      if (string::to(value, val) && val > 0) {
        _obj->max_attempts = val;

        // adjust current attempt number if in a hard state.
        if (_obj->state_type == HARD_STATE
            && _obj->current_state != STATE_OK
            && _obj->current_attempt > 1)
          _obj->current_attempt = _obj->max_attempts;
      }
    }
  }
  else if (!key.empty() && key[0] == '_') {
    if (_obj->modified_attributes & MODATTR_CUSTOM_VARIABLE
        && value.size() > 3) {
      char const* cvname(key.c_str() + 1);
      char const* cvvalue(value.c_str() + 2);

      for (customvariablesmember* member = _obj->custom_variables;
           member;
           member = member->next) {
        if (!strcmp(cvname, member->variable_name)) {
          if (strcmp(cvvalue, member->variable_value)) {
            string::setstr(member->variable_value, cvvalue);
            member->has_been_modified = true;
          }
          break;
        }
      }
    }
  }
  else
    return (false);
  return (true);
}

/**
 *  Set new value on specific retain nonstatus information property.
 *
 *  @param[in] key   The property to set.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool retention::service::_retain_status_information(
       std::string const& key,
       std::string const& value) {
  if (!_obj->retain_status_information)
    return (false);

  if (key == "has_been_checked")
    string::to<bool, int>(value, _obj->has_been_checked);
  else if (key == "check_execution_time")
    string::to(value, _obj->execution_time);
  else if (key == "check_latency")
    string::to(value, _obj->latency);
  else if (key == "check_type")
    string::to(value, _obj->check_type);
  else if (key == "current_state")
    string::to(value, _obj->current_state);
  else if (key == "last_state")
    string::to(value, _obj->last_state);
  else if (key == "last_hard_state")
    string::to(value, _obj->last_hard_state);
  else if (key == "current_attempt")
    string::to(value, _obj->current_attempt);
  else if (key == "current_event_id")
    string::to(value, _obj->current_event_id);
  else if (key == "last_event_id")
    string::to(value, _obj->last_event_id);
  else if (key == "current_problem_id")
    string::to(value, _obj->current_problem_id);
  else if (key == "last_problem_id")
    string::to(value, _obj->last_problem_id);
  else if (key == "state_type")
    string::to(value, _obj->state_type);
  else if (key == "last_state_change")
    string::to(value, _obj->last_state_change);
  else if (key == "last_hard_state_change")
    string::to(value, _obj->last_hard_state_change);
  else if (key == "last_time_ok")
    string::to(value, _obj->last_time_ok);
  else if (key == "last_time_warning")
    string::to(value, _obj->last_time_warning);
  else if (key == "last_time_unknown")
    string::to(value, _obj->last_time_unknown);
  else if (key == "last_time_critical")
    string::to(value, _obj->last_time_critical);
  else if (key == "plugin_output")
    string::setstr(_obj->plugin_output, value);
  else if (key == "long_plugin_output")
    string::setstr(_obj->long_plugin_output, value);
  else if (key == "performance_data")
    string::setstr(_obj->perf_data, value);
  else if (key == "last_check")
    string::to(value, _obj->last_check);
  else if (key == "next_check") {
    // if (config->use_retained_scheduling_info() && _scheduling_info_is_ok)
      string::to(value, _obj->next_check);
  }
  else if (key == "check_options") {
    // if (config->use_retained_scheduling_info() && _scheduling_info_is_ok)
      string::to(value, _obj->check_options);
  }
  else if (key == "notified_on_unknown")
    string::to<bool, int>(value, _obj->notified_on_unknown);
  else if (key == "notified_on_warning")
    string::to<bool, int>(value, _obj->notified_on_warning);
  else if (key == "notified_on_critical")
    string::to<bool, int>(value, _obj->notified_on_critical);
  else if (key == "current_notification_number")
    string::to(value, _obj->current_notification_number);
  else if (key == "current_notification_id")
    string::to(value, _obj->current_notification_id);
  else if (key == "last_notification")
    string::to(value, _obj->last_notification);
  else if (key == "is_flapping")
    string::to(value, _was_flapping);
  else if (key == "percent_state_change")
    string::to(value, _obj->percent_state_change);
  else if (key == "check_flapping_recovery_notification")
    string::to(value, _obj->check_flapping_recovery_notification);
  else if (key == "state_history") {
    unsigned int x(0);
    std::list<std::string> lst_history;
    string::split(value, lst_history, ',');
    for (std::list<std::string>::const_iterator
           it(lst_history.begin()), end(lst_history.end());
         it != end && x < MAX_STATE_HISTORY_ENTRIES;
         ++it) {
      string::to(*it, _obj->state_history[x++]);
    }
    _obj->state_history_index = 0;
  }
  return (true);
}

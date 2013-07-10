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

#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/retention/program.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;

/**
 *  Constructor.
 */
retention::program::program()
  : object(object::program) {

}

/**
 *  Destructor.
 */
retention::program::~program() throw () {
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
bool retention::program::set(
       std::string const& key,
       std::string const& value) {
  if (key == "modified_host_attributes") {
    string::to(value, modified_host_process_attributes);
    // mask out attributes we don't want to retain.
    modified_host_process_attributes
      &= ~config->retained_process_host_attribute_mask();
  }
  else if (key == "modified_service_attributes") {
    string::to(value, modified_service_process_attributes);
    // mask out attributes we don't want to retain.
    modified_service_process_attributes
      &= ~config->retained_process_host_attribute_mask();
  }
  if (config->use_retained_program_state()) {
    if (key == "enable_notifications") {
      if (modified_host_process_attributes & MODATTR_NOTIFICATIONS_ENABLED) {
        string::to<bool, unsigned int>(value, enable_notifications);
        config->enable_notifications(enable_notifications);
      }
    }
    else if (key == "active_service_checks_enabled") {
      if (modified_service_process_attributes & MODATTR_ACTIVE_CHECKS_ENABLED) {
        string::to<bool, unsigned int>(value, execute_service_checks);
        config->execute_service_checks(execute_service_checks);
      }
    }
    else if (key == "passive_service_checks_enabled") {
      if (modified_service_process_attributes & MODATTR_PASSIVE_CHECKS_ENABLED) {
        string::to<bool, unsigned int>(value, accept_passive_service_checks);
        config->accept_passive_service_checks(accept_passive_service_checks);
      }
    }
    else if (key == "active_host_checks_enabled") {
      if (modified_host_process_attributes & MODATTR_ACTIVE_CHECKS_ENABLED) {
        string::to<bool, unsigned int>(value, execute_host_checks);
        config->execute_host_checks(execute_host_checks);
      }
    }
    else if (key == "passive_host_checks_enabled") {
      if (modified_host_process_attributes & MODATTR_PASSIVE_CHECKS_ENABLED) {
        string::to<bool, unsigned int>(value, accept_passive_host_checks);
        config->accept_passive_host_checks(accept_passive_host_checks);
      }
    }
    else if (key == "enable_event_handlers") {
      if (modified_host_process_attributes & MODATTR_EVENT_HANDLER_ENABLED) {
        string::to<bool, unsigned int>(value, enable_event_handlers);
        config->enable_event_handlers(enable_event_handlers);
      }
    }
    else if (key == "obsess_over_services") {
      if (modified_service_process_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED) {
        string::to<bool, unsigned int>(value, obsess_over_services);
        config->obsess_over_services(obsess_over_services);
      }
    }
    else if (key == "obsess_over_hosts") {
      if (modified_host_process_attributes & MODATTR_OBSESSIVE_HANDLER_ENABLED) {
        string::to<bool, unsigned int>(value, obsess_over_hosts);
        config->obsess_over_hosts(obsess_over_hosts);
      }
    }
    else if (key == "check_service_freshness") {
      if (modified_service_process_attributes & MODATTR_FRESHNESS_CHECKS_ENABLED) {
        string::to<bool, unsigned int>(value, check_service_freshness);
        config->check_service_freshness(check_service_freshness);
      }
    }
    else if (key == "check_host_freshness") {
      if (modified_host_process_attributes & MODATTR_FRESHNESS_CHECKS_ENABLED) {
        string::to<bool, unsigned int>(value, check_host_freshness);
        config->check_host_freshness(check_host_freshness);
      }
    }
    else if (key == "enable_flap_detection") {
      if (modified_host_process_attributes & MODATTR_FLAP_DETECTION_ENABLED) {
        string::to<bool, unsigned int>(value, enable_flap_detection);
        config->enable_flap_detection(enable_flap_detection);
      }
    }
    else if (key == "enable_failure_prediction") {
      if (modified_host_process_attributes & MODATTR_FAILURE_PREDICTION_ENABLED) {
        string::to<bool, unsigned int>(value, enable_failure_prediction);
        config->enable_failure_prediction(enable_failure_prediction);
      }
    }
    else if (key == "process_performance_data") {
      if (modified_host_process_attributes & MODATTR_PERFORMANCE_DATA_ENABLED) {
        string::to<bool, unsigned int>(value, process_performance_data);
        config->process_performance_data(process_performance_data);
      }
    }
    else if (key == "global_host_event_handler") {
      if (modified_host_process_attributes & MODATTR_EVENT_HANDLER_COMMAND) {
        std::size_t pos(value.find('!'));
        if (pos != std::string::npos) {
          std::string command(value.substr(pos + 1));
          if (find_command(command.c_str())) {
            string::setstr(global_host_event_handler, value);
            config->global_host_event_handler(value);
          }
        }
      }
    }
    else if (key == "global_service_event_handler") {
      if (modified_service_process_attributes & MODATTR_EVENT_HANDLER_COMMAND) {
        std::size_t pos(value.find('!'));
        if (pos != std::string::npos) {
          std::string command(value.substr(pos + 1));
          if (find_command(command.c_str())) {
            string::setstr(global_service_event_handler, value);
            config->global_service_event_handler(value);
          }
        }
      }
    }
    else if (key == "next_comment_id")
      string::to(value, next_comment_id);
    else if (key == "next_downtime_id")
      string::to(value, next_downtime_id);
    else if (key == "next_event_id")
      string::to(value, next_event_id);
    else if (key == "next_problem_id")
      string::to(value, next_problem_id);
    else if (key == "next_notification_id")
      string::to(value, next_notification_id);
  }
  else
    return (false);
  return (true);
}

/**
 *  Finish all program update.
 */
void retention::program::_finished() throw () {
  if (!config->use_retained_program_state()) {
    modified_host_process_attributes = MODATTR_NONE;
    modified_service_process_attributes = MODATTR_NONE;
  }
}

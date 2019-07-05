/*
** Copyright 2011-2013,2015,2018 Centreon
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

#include <cassert>
#include "com/centreon/engine/configuration/applier/globals.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

static applier::globals* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::globals::apply(state& config) {
  _set_global(::check_result_path, config.check_result_path());
  _set_global(::debug_file, config.debug_file());
  _set_global(::global_host_event_handler, config.global_host_event_handler());
  _set_global(::global_service_event_handler, config.global_service_event_handler());
  _set_global(::illegal_object_chars, config.illegal_object_chars());
  _set_global(::illegal_output_chars, config.illegal_output_chars());
  _set_global(::log_file, config.log_file());
  _set_global(::ochp_command, config.ochp_command());
  _set_global(::ocsp_command, config.ocsp_command());
  _set_global(::use_timezone, config.use_timezone());

  ::accept_passive_host_checks = config.accept_passive_host_checks();
  ::accept_passive_service_checks = config.accept_passive_service_checks();
  ::additional_freshness_latency = config.additional_freshness_latency();
  ::cached_host_check_horizon = config.cached_host_check_horizon();
  ::check_external_commands = config.check_external_commands();
  ::check_host_freshness = config.check_host_freshness();
  ::check_reaper_interval = config.check_reaper_interval();
  ::check_service_freshness = config.check_service_freshness();
  ::enable_event_handlers = config.enable_event_handlers();
  ::enable_flap_detection = config.enable_flap_detection();
  ::enable_notifications = config.enable_notifications();
  ::execute_host_checks = config.execute_host_checks();
  ::execute_service_checks = config.execute_service_checks();
  ::interval_length = config.interval_length();
  ::log_notifications = config.log_notifications();
  ::log_passive_checks = config.log_passive_checks();
  ::max_host_check_spread = config.max_host_check_spread();
  ::max_service_check_spread = config.max_service_check_spread();
  ::notification_timeout = config.notification_timeout();
  ::obsess_over_hosts = config.obsess_over_hosts();
  ::obsess_over_services = config.obsess_over_services();
  ::process_performance_data = config.process_performance_data();
  ::soft_state_dependencies = config.soft_state_dependencies();
  ::use_aggressive_host_checking = config.use_aggressive_host_checking();
  ::use_large_installation_tweaks = config.use_large_installation_tweaks();
}

/**
 *  Get the singleton instance of globals applier.
 *
 *  @return Singleton instance.
 */
applier::globals& applier::globals::instance() {
  assert(_instance);
  return (*_instance);
}

/**
 *  Load globals applier singleton.
 */
void applier::globals::load() {
  if (!_instance)
    _instance = new applier::globals;
}

/**
 *  Unload globals applier singleton.
 */
void applier::globals::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::globals::globals() {

}

/**
 *  Destructor.
 */
applier::globals::~globals() throw() {
  delete[] ::check_result_path;
  delete[] ::debug_file;
  delete[] ::global_host_event_handler;
  delete[] ::global_service_event_handler;
  delete[] ::illegal_object_chars;
  delete[] ::illegal_output_chars;
  delete[] ::log_file;
  delete[] ::ochp_command;
  delete[] ::ocsp_command;
  delete[] ::use_timezone;

  ::check_result_path = NULL;
  ::debug_file = NULL;
  ::global_host_event_handler = NULL;
  ::global_service_event_handler = NULL;
  ::illegal_object_chars = NULL;
  ::illegal_output_chars = NULL;
  ::log_file = NULL;
  ::ochp_command = NULL;
  ::ocsp_command = NULL;
  ::use_timezone = NULL;
}

void applier::globals::_set_global(
       char*& property,
       std::string const& value) {
  if (!property || strcmp(property, value.c_str()))
    property = string::dup(value);
}

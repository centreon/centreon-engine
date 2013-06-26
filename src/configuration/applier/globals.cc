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

#include "com/centreon/engine/configuration/applier/globals.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/misc/string.hh"

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
  _set_global(::command_file, config.command_file());
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
  ::allow_empty_hostgroup_assignment = config.allow_empty_hostgroup_assignment();
  ::auto_reschedule_checks = config.auto_reschedule_checks();
  ::auto_rescheduling_interval = config.auto_rescheduling_interval();
  ::auto_rescheduling_window = config.auto_rescheduling_window();
  ::cached_host_check_horizon = config.cached_host_check_horizon();
  ::cached_service_check_horizon = config.cached_service_check_horizon();
  ::check_external_commands = config.check_external_commands();
  ::check_host_freshness = config.check_host_freshness();
  ::check_orphaned_hosts = config.check_orphaned_hosts();
  ::check_orphaned_services = config.check_orphaned_services();
  ::check_reaper_interval = config.check_reaper_interval();
  ::check_service_freshness = config.check_service_freshness();
  ::command_check_interval = config.command_check_interval();
  ::date_format = config.date_format();
  ::debug_level = config.debug_level();
  ::debug_verbosity = config.debug_verbosity();
  ::enable_environment_macros = config.enable_environment_macros();
  ::enable_event_handlers = config.enable_event_handlers();
  ::enable_failure_prediction = config.enable_failure_prediction();
  ::enable_flap_detection = config.enable_flap_detection();
  ::enable_notifications = config.enable_notifications();
  ::enable_predictive_host_dependency_checks = config.enable_predictive_host_dependency_checks();
  ::enable_predictive_service_dependency_checks = config.enable_predictive_service_dependency_checks();
  ::event_broker_options = config.event_broker_options();
  ::event_handler_timeout = config.event_handler_timeout();
  ::execute_host_checks = config.execute_host_checks();
  ::execute_service_checks = config.execute_service_checks();
  ::external_command_buffer_slots = config.external_command_buffer_slots();
  ::high_host_flap_threshold = config.high_host_flap_threshold();
  ::high_service_flap_threshold = config.high_service_flap_threshold();
  ::host_check_timeout = config.host_check_timeout();
  ::host_freshness_check_interval = config.host_freshness_check_interval();
  ::host_inter_check_delay_method = config.host_inter_check_delay_method();
  ::interval_length = config.interval_length();
  ::log_event_handlers = config.log_event_handlers();
  ::log_external_commands = config.log_external_commands();
  ::log_host_retries = config.log_host_retries();
  ::log_initial_states = config.log_initial_states();
  ::log_notifications = config.log_notifications();
  ::log_passive_checks = config.log_passive_checks();
  ::log_service_retries = config.log_service_retries();
  ::low_host_flap_threshold = config.low_host_flap_threshold();
  ::low_service_flap_threshold = config.low_service_flap_threshold();
  ::max_check_reaper_time = config.max_check_reaper_time();
  ::max_check_result_file_age = config.max_check_result_file_age();
  ::max_debug_file_size = config.max_debug_file_size();
  ::max_host_check_spread = config.max_host_check_spread();
  ::max_parallel_service_checks = config.max_parallel_service_checks();
  ::max_service_check_spread = config.max_service_check_spread();
  ::notification_timeout = config.notification_timeout();
  ::obsess_over_hosts = config.obsess_over_hosts();
  ::obsess_over_services = config.obsess_over_services();
  ::ochp_timeout = config.ochp_timeout();
  ::ocsp_timeout = config.ocsp_timeout();
  ::passive_host_checks_are_soft = config.passive_host_checks_are_soft();
  ::process_performance_data = config.process_performance_data();
  ::retained_contact_host_attribute_mask = config.retained_contact_host_attribute_mask();
  ::retained_contact_service_attribute_mask = config.retained_contact_service_attribute_mask();
  ::retained_host_attribute_mask = config.retained_host_attribute_mask();
  ::retained_process_host_attribute_mask = config.retained_process_host_attribute_mask();
  ::retain_state_information = config.retain_state_information();
  ::retention_scheduling_horizon = config.retention_scheduling_horizon();
  ::retention_update_interval = config.retention_update_interval();
  ::service_check_timeout = config.service_check_timeout();
  ::service_freshness_check_interval = config.service_freshness_check_interval();
  ::service_inter_check_delay_method = config.service_inter_check_delay_method();
  ::service_interleave_factor_method = config.service_interleave_factor_method();
  ::sleep_time = config.sleep_time();
  ::soft_state_dependencies = config.soft_state_dependencies();
  ::status_update_interval = config.status_update_interval();
  ::time_change_threshold = config.time_change_threshold();
  ::translate_passive_host_checks = config.translate_passive_host_checks();
  ::use_aggressive_host_checking = config.use_aggressive_host_checking();
  ::use_large_installation_tweaks = config.use_large_installation_tweaks();
  ::use_regexp_matches = config.use_regexp_matches();
  ::use_retained_program_state = config.use_retained_program_state();
  ::use_retained_scheduling_info = config.use_retained_scheduling_info();
  ::use_syslog = config.use_syslog();
  ::use_true_regexp_matching = config.use_true_regexp_matching();

  std::vector<std::string> const& users(config.user());
  for (unsigned int i(0), end(users.size()); i != end; ++i)
    misc::setstr(macro_user[i], users[i]);
}

/**
 *  Get the singleton instance of globals applier.
 *
 *  @return Singleton instance.
 */
applier::globals& applier::globals::instance() {
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
  delete[] ::command_file;
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
  ::command_file = NULL;
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
  if (property && strcmp(property, value.c_str())) {
    property = misc::strdup(value);
  }
}

/*
** Copyright 2012 Merethis
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

#include <fstream>
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/modules/webservice/configuration/save/state.hh"

using namespace com::centreon::engine::modules::webservice::configuration::save;

/**
 *  Default constructor.
 */
state::state() {

}

/**
 *  Copy constructor.
 *
 *  @param[in] right  The object to copy.
 */
state::state(state const& right) {
  _internal_copy(right);
}

/**
 *  Destructor.
 */
state::~state() throw () {

}

/**
 *  Assignment operator.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
state& state::operator=(state const& right) {
  return (_internal_copy(right));
}

/**
 *  Translate a configuration state to the engine configuration format.
 *
 *  @param[in] obj  The configuration to save.
 */
state& state::operator<<(engine::configuration::state const& obj) {
  std::list<std::string> const& lst_dir(obj.get_cfg_dir());
  for (std::list<std::string>::const_iterator
         it(lst_dir.begin()), end(lst_dir.end());
       it != end;
       ++it)
    _stream << "cfg_dir=" << *it << std::endl;

  std::list<std::string> const& lst_file(obj.get_cfg_file());
  for (std::list<std::string>::const_iterator
         it(lst_file.begin()), end(lst_file.end());
       it != end;
       ++it)
    _stream << "cfg_file=" << *it << std::endl;

  std::list<std::string> const& lst_resource(obj.get_resource_file());
  for (std::list<std::string>::const_iterator
         it(lst_resource.begin()), end(lst_resource.end());
       it != end;
       ++it)
    _stream << "resource_file=" << *it << std::endl;

  std::list<std::string> const& lst_module(obj.get_broker_module());
  for (std::list<std::string>::const_iterator
         it(lst_module.begin()), end(lst_module.end());
       it != end;
       ++it)
    _stream << "broker_module=" << *it << std::endl;

  int command_check_interval
    = obj.get_command_check_interval() / obj.get_interval_length();

  static char const* date_format[] = {
    "us",
    "euro",
    "iso8601",
    "strict-iso8601"
  };

  static char const* delay_method[] = {
    "n",
    "d",
    "s",
    "u"
  };

  _stream
    << "accept_passive_host_checks="
    << obj.get_accept_passive_host_checks() << std::endl
    << "accept_passive_service_checks="
    << obj.get_accept_passive_service_checks() << std::endl
    << "additional_freshness_latency="
    << obj.get_additional_freshness_latency() << std::endl
    << "admin_email="
    << obj.get_admin_email() << std::endl
    << "admin_pager="
    << obj.get_admin_pager() << std::endl
    << "allow_empty_hostgroup_assignment="
    << obj.get_allow_empty_hostgroup_assignment() << std::endl
    << "auto_reschedule_checks="
    << obj.get_auto_reschedule_checks() << std::endl
    << "auto_rescheduling_interval="
    << obj.get_auto_rescheduling_interval() << std::endl
    << "auto_rescheduling_window="
    << obj.get_auto_rescheduling_window() << std::endl
    << "broker_module_directory="
    << obj.get_broker_module_directory() << std::endl
    << "cached_host_check_horizon="
    << obj.get_cached_host_check_horizon() << std::endl
    << "cached_service_check_horizon="
    << obj.get_cached_service_check_horizon() << std::endl
    << "check_external_commands="
    << obj.get_check_external_commands() << std::endl
    << "check_for_orphaned_hosts="
    << obj.get_check_orphaned_hosts() << std::endl
    << "check_for_orphaned_services="
    << obj.get_check_orphaned_services() << std::endl
    << "check_host_freshness="
    << obj.get_check_host_freshness() << std::endl
    << "check_result_reaper_frequency="
    << obj.get_check_reaper_interval() << std::endl
    << "check_service_freshness="
    << obj.get_check_service_freshness() << std::endl
    << "command_check_interval="
    << command_check_interval << std::endl
    << "command_file="
    << obj.get_command_file() << std::endl
    << "date_format="
    << date_format[obj.get_date_format()] << std::endl
    << "debug_file="
    << obj.get_debug_file() << std::endl
    << "debug_level="
    << obj.get_debug_level() << std::endl
    << "debug_verbosity="
    << obj.get_debug_verbosity() << std::endl
    << "enable_environment_macros="
    << obj.get_enable_environment_macros() << std::endl
    << "enable_event_handlers="
    << obj.get_enable_event_handlers() << std::endl
    << "enable_failure_prediction="
    << obj.get_enable_failure_prediction() << std::endl
    << "enable_flap_detection="
    << obj.get_enable_flap_detection() << std::endl
    << "enable_notifications="
    << obj.get_enable_notifications() << std::endl
    << "enable_predictive_host_dependency_checks="
    << obj.get_enable_predictive_host_dependency_checks() << std::endl
    << "enable_predictive_service_dependency_checks="
    << obj.get_enable_predictive_service_dependency_checks() << std::endl;

  _stream << "event_broker_options=";
  if (obj.get_event_broker_options()
      == static_cast<unsigned long>(BROKER_EVERYTHING))
    _stream << "-1";
  else
    _stream << obj.get_event_broker_options();
  _stream << std::endl;

  _stream
    << "event_handler_timeout="
    << obj.get_event_handler_timeout() << std::endl
    << "execute_host_checks="
    << obj.get_execute_host_checks() << std::endl
    << "execute_service_checks="
    << obj.get_execute_service_checks() << std::endl
    << "external_command_buffer_slots="
    << obj.get_external_command_buffer_slots() << std::endl
    << "free_child_process_memory="
    << obj.get_free_child_process_memory() << std::endl
    << "global_host_event_handler="
    << obj.get_global_host_event_handler() << std::endl
    << "global_service_event_handler="
    << obj.get_global_service_event_handler() << std::endl
    << "high_host_flap_threshold="
    << obj.get_high_host_flap_threshold() << std::endl
    << "high_service_flap_threshold="
    << obj.get_high_service_flap_threshold() << std::endl
    << "host_check_timeout="
    << obj.get_host_check_timeout() << std::endl
    << "host_freshness_check_interval="
    << obj.get_host_freshness_check_interval() << std::endl;

  _stream << "host_inter_check_delay_method=";
  if (obj.get_host_inter_check_delay_method()
      == com::centreon::engine::configuration::state::icd_user)
    _stream << scheduling_info.host_inter_check_delay;
  else
    _stream << delay_method[obj.get_host_inter_check_delay_method()];
  _stream << std::endl;

  _stream
    << "illegal_macro_output_chars="
    << obj.get_illegal_output_chars() << std::endl
    << "illegal_object_name_chars="
    << obj.get_illegal_object_chars() << std::endl
    << "interval_length="
    << obj.get_interval_length() << std::endl
    << "log_event_handlers="
    << obj.get_log_event_handlers() << std::endl
    << "log_external_commands="
    << obj.get_log_external_commands() << std::endl
    << "log_file="
    << obj.get_log_file() << std::endl
    << "log_host_retries="
    << obj.get_log_host_retries() << std::endl
    << "log_initial_states="
    << obj.get_log_initial_state() << std::endl
    << "log_notifications="
    << obj.get_log_notifications() << std::endl
    << "log_passive_checks="
    << obj.get_log_passive_checks() << std::endl
    << "log_service_retries="
    << obj.get_log_service_retries() << std::endl
    << "low_host_flap_threshold="
    << obj.get_low_host_flap_threshold() << std::endl
    << "low_service_flap_threshold="
    << obj.get_low_service_flap_threshold() << std::endl
    << "max_check_result_reaper_time="
    << obj.get_max_check_reaper_time() << std::endl
    << "max_concurrent_checks="
    << obj.get_max_parallel_service_checks() << std::endl
    << "max_debug_file_size="
    << obj.get_max_debug_file_size() << std::endl
    << "max_host_check_spread="
    << obj.get_max_host_check_spread() << std::endl
    << "max_log_file_size="
    << obj.get_max_log_file_size() << std::endl
    << "max_service_check_spread="
    << obj.get_max_service_check_spread() << std::endl
    << "notification_timeout="
    << obj.get_notification_timeout() << std::endl
    << "object_cache_file="
    << obj.get_object_cache_file() << std::endl
    << "obsess_over_hosts="
    << obj.get_obsess_over_hosts() << std::endl
    << "obsess_over_services="
    << obj.get_obsess_over_services() << std::endl
    << "ochp_command="
    << obj.get_ochp_command() << std::endl
    << "ochp_timeout="
    << obj.get_ochp_timeout() << std::endl
    << "ocsp_command="
    << obj.get_ocsp_command() << std::endl
    << "ocsp_timeout="
    << obj.get_ocsp_timeout() << std::endl
    << "passive_host_checks_are_soft="
    << obj.get_passive_host_checks_are_soft() << std::endl
    << "perfdata_timeout="
    << obj.get_perfdata_timeout() << std::endl
    << "precached_object_file="
    << obj.get_precached_object_file() << std::endl
    << "process_performance_data="
    << obj.get_process_performance_data() << std::endl
    << "retained_contact_host_attribute_mask="
    << obj.get_retained_contact_host_attribute_mask() << std::endl
    << "retained_contact_service_attribute_mask="
    << obj.get_retained_contact_service_attribute_mask() << std::endl
    << "retained_host_attribute_mask="
    << obj.get_retained_host_attribute_mask() << std::endl
    << "retained_process_host_attribute_mask="
    << obj.get_retained_process_host_attribute_mask() << std::endl
    << "retain_state_information="
    << obj.get_retain_state_information() << std::endl
    << "retention_scheduling_horizon="
    << obj.get_retention_scheduling_horizon() << std::endl
    << "retention_update_interval="
    << obj.get_retention_update_interval() << std::endl
    << "service_check_timeout="
    << obj.get_service_check_timeout() << std::endl
    << "service_freshness_check_interval="
    << obj.get_service_freshness_check_interval() << std::endl;

  _stream  << "service_inter_check_delay_method=";
  if (obj.get_service_inter_check_delay_method()
      == com::centreon::engine::configuration::state::icd_user)
    _stream << scheduling_info.service_inter_check_delay;
  else
    _stream << delay_method[obj.get_service_inter_check_delay_method()];
  _stream << std::endl;

  _stream << "service_interleave_factor=";
  if (obj.get_service_interleave_factor_method()
      == com::centreon::engine::configuration::state::ilf_user)
    _stream << scheduling_info.service_interleave_factor;
  else
    _stream << "s";
  _stream << std::endl;

  _stream
    << "service_reaper_frequency="
    << obj.get_check_reaper_interval() << std::endl
    << "sleep_time="
    << obj.get_sleep_time() << std::endl
    << "soft_state_dependencies="
    << obj.get_soft_state_dependencies() << std::endl
    << "state_retention_file="
    << obj.get_state_retention_file() << std::endl
    << "status_file="
    << obj.get_status_file() << std::endl
    << "status_update_interval="
    << obj.get_status_update_interval() << std::endl
    << "time_change_threshold="
    << obj.get_time_change_threshold() << std::endl
    << "translate_passive_host_checks="
    << obj.get_translate_passive_host_checks() << std::endl
    << "use_aggressive_host_checking="
    << obj.get_use_aggressive_host_checking() << std::endl
    << "use_large_installation_tweaks="
    << obj.get_use_large_installation_tweaks() << std::endl
    << "use_regexp_matching="
    << obj.get_use_regexp_matches() << std::endl
    << "use_retained_program_state="
    << obj.get_use_retained_program_state() << std::endl
    << "use_retained_scheduling_info="
    << obj.get_use_retained_scheduling_info() << std::endl
    << "use_syslog="
    << obj.get_use_syslog() << std::endl
    << "use_timezone="
    << obj.get_use_timezone() << std::endl
    << "use_true_regexp_matching="
    << obj.get_use_true_regexp_matching() << std::endl;
  return (*this);
}

/**
 *  Save into the file the current configuration.
 *
 *  @param[in] filename  The file path to save buffer.
 */
void state::backup(std::string const& filename) const {
  std::ofstream output(filename.c_str());
  if (!output.is_open())
    throw (engine_error() << "save configuration: open file '"
           << filename << "' failed");
  output << _stream.str();
}

/**
 *  Clear buffer.
 */
void state::clear() {
  _stream.str("");
}

/**
 *  Get the save string.
 *
 *  @return The save string.
 */
std::string state::to_string() const {
  return (_stream.str());
}

/**
 *  Internal copy.
 *
 *  @param[in] right  The object to copy.
 *
 *  @return This object.
 */
state& state::_internal_copy(state const& right) {
  if (&right != this) {
    _stream.str(right._stream.str());
  }
  return (*this);
}

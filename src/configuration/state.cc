/*
** Copyright 2011-2013,2015-2017 Centreon
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

#include "com/centreon/engine/configuration/state.hh"

#include <limits>

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/io/file_entry.hh"
#include "compatibility/locations.h"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::logging;

#define SETTER(type, method) &state::setter<type, &state::method>::generic

std::unordered_map<std::string, state::setter_func> const state::_setters{
    {"accept_passive_host_checks", SETTER(bool, accept_passive_host_checks)},
    {"accept_passive_service_checks",
     SETTER(bool, accept_passive_service_checks)},
    {"additional_freshness_latency", SETTER(int, additional_freshness_latency)},
    {"admin_email", SETTER(std::string const&, admin_email)},
    {"admin_pager", SETTER(std::string const&, admin_pager)},
    {"aggregate_status_updates",
     SETTER(std::string const&, _set_aggregate_status_updates)},
    {"allow_empty_hostgroup_assignment",
     SETTER(bool, allow_empty_hostgroup_assignment)},
    {"auth_file", SETTER(std::string const&, _set_auth_file)},
    {"auto_reschedule_checks", SETTER(bool, auto_reschedule_checks)},
    {"auto_rescheduling_interval",
     SETTER(unsigned int, auto_rescheduling_interval)},
    {"auto_rescheduling_window",
     SETTER(unsigned int, auto_rescheduling_window)},
    {"bare_update_check", SETTER(std::string const&, _set_bare_update_check)},
    {"broker_module_directory",
     SETTER(std::string const&, broker_module_directory)},
    {"broker_module", SETTER(std::string const&, _set_broker_module)},
    {"cached_host_check_horizon",
     SETTER(unsigned long, cached_host_check_horizon)},
    {"cached_service_check_horizon",
     SETTER(unsigned long, cached_service_check_horizon)},
    {"cfg_dir", SETTER(std::string const&, _set_cfg_dir)},
    {"cfg_file", SETTER(std::string const&, _set_cfg_file)},
    {"check_external_commands", SETTER(bool, check_external_commands)},
    {"check_for_orphaned_hosts", SETTER(bool, check_orphaned_hosts)},
    {"check_for_orphaned_services", SETTER(bool, check_orphaned_services)},
    {"check_for_updates", SETTER(std::string const&, _set_check_for_updates)},
    {"check_host_freshness", SETTER(bool, check_host_freshness)},
    {"check_result_reaper_frequency",
     SETTER(unsigned int, check_reaper_interval)},
    {"check_service_freshness", SETTER(bool, check_service_freshness)},
    {"child_processes_fork_twice",
     SETTER(std::string const&, _set_child_processes_fork_twice)},
    {"command_check_interval",
     SETTER(std::string const&, _set_command_check_interval)},
    {"command_file", SETTER(std::string const&, command_file)},
    {"comment_file", SETTER(std::string const&, _set_comment_file)},
    {"daemon_dumps_core", SETTER(std::string const&, _set_daemon_dumps_core)},
    {"date_format", SETTER(std::string const&, _set_date_format)},
    {"debug_file", SETTER(std::string const&, debug_file)},
    {"debug_level", SETTER(unsigned long long, debug_level)},
    {"debug_verbosity", SETTER(unsigned int, debug_verbosity)},
    {"downtime_file", SETTER(std::string const&, _set_downtime_file)},
    {"enable_embedded_perl",
     SETTER(std::string const&, _set_enable_embedded_perl)},
    {"enable_environment_macros", SETTER(bool, enable_environment_macros)},
    {"enable_event_handlers", SETTER(bool, enable_event_handlers)},
    {"enable_failure_prediction",
     SETTER(std::string const&, _set_enable_failure_prediction)},
    {"enable_flap_detection", SETTER(bool, enable_flap_detection)},
    {"enable_macros_filter", SETTER(bool, enable_macros_filter)},
    {"enable_notifications", SETTER(bool, enable_notifications)},
    {"enable_predictive_host_dependency_checks",
     SETTER(bool, enable_predictive_host_dependency_checks)},
    {"enable_predictive_service_dependency_checks",
     SETTER(bool, enable_predictive_service_dependency_checks)},
    {"event_broker_options",
     SETTER(std::string const&, _set_event_broker_options)},
    {"event_handler_timeout", SETTER(unsigned int, event_handler_timeout)},
    {"execute_host_checks", SETTER(bool, execute_host_checks)},
    {"execute_service_checks", SETTER(bool, execute_service_checks)},
    {"external_command_buffer_slots",
     SETTER(int, external_command_buffer_slots)},
    {"free_child_process_memory",
     SETTER(std::string const&, _set_free_child_process_memory)},
    {"global_host_event_handler",
     SETTER(std::string const&, global_host_event_handler)},
    {"global_service_event_handler",
     SETTER(std::string const&, global_service_event_handler)},
    {"high_host_flap_threshold", SETTER(float, high_host_flap_threshold)},
    {"high_service_flap_threshold", SETTER(float, high_service_flap_threshold)},
    {"host_check_timeout", SETTER(unsigned int, host_check_timeout)},
    {"host_freshness_check_interval",
     SETTER(unsigned int, host_freshness_check_interval)},
    {"host_inter_check_delay_method",
     SETTER(std::string const&, _set_host_inter_check_delay_method)},
    {"host_perfdata_command",
     SETTER(std::string const&, host_perfdata_command)},
    {"host_perfdata_file", SETTER(std::string const&, host_perfdata_file)},
    {"host_perfdata_file_mode",
     SETTER(std::string const&, _set_host_perfdata_file_mode)},
    {"host_perfdata_file_processing_command",
     SETTER(std::string const&, host_perfdata_file_processing_command)},
    {"host_perfdata_file_processing_interval",
     SETTER(unsigned int, host_perfdata_file_processing_interval)},
    {"host_perfdata_file_template",
     SETTER(std::string const&, host_perfdata_file_template)},
    {"illegal_macro_output_chars",
     SETTER(std::string const&, illegal_output_chars)},
    {"illegal_object_name_chars",
     SETTER(std::string const&, illegal_object_chars)},
    {"interval_length", SETTER(unsigned int, interval_length)},
    {"lock_file", SETTER(std::string const&, _set_lock_file)},
    {"log_archive_path", SETTER(std::string const&, _set_log_archive_path)},
    {"log_event_handlers", SETTER(bool, log_event_handlers)},
    {"log_external_commands", SETTER(bool, log_external_commands)},
    {"log_file", SETTER(std::string const&, log_file)},
    {"log_host_retries", SETTER(bool, log_host_retries)},
    {"log_initial_states", SETTER(std::string const&, _set_log_initial_states)},
    {"log_notifications", SETTER(bool, log_notifications)},
    {"log_passive_checks", SETTER(bool, log_passive_checks)},
    {"log_pid", SETTER(bool, log_pid)},
    {"log_rotation_method",
     SETTER(std::string const&, _set_log_rotation_method)},
    {"log_service_retries", SETTER(bool, log_service_retries)},
    {"low_host_flap_threshold", SETTER(float, low_host_flap_threshold)},
    {"low_service_flap_threshold", SETTER(float, low_service_flap_threshold)},
    {"macros_filter", SETTER(std::string const&, macros_filter)},
    {"max_check_result_reaper_time",
     SETTER(unsigned int, max_check_reaper_time)},
    {"max_concurrent_checks",
     SETTER(unsigned int, max_parallel_service_checks)},
    {"max_debug_file_size", SETTER(unsigned long, max_debug_file_size)},
    {"max_host_check_spread", SETTER(unsigned int, max_host_check_spread)},
    {"max_log_file_size", SETTER(unsigned long, max_log_file_size)},
    {"max_service_check_spread",
     SETTER(unsigned int, max_service_check_spread)},
    {"nagios_group", SETTER(std::string const&, _set_nagios_group)},
    {"nagios_user", SETTER(std::string const&, _set_nagios_user)},
    {"notification_timeout", SETTER(unsigned int, notification_timeout)},
    {"object_cache_file", SETTER(std::string const&, _set_object_cache_file)},
    {"obsess_over_hosts", SETTER(bool, obsess_over_hosts)},
    {"obsess_over_services", SETTER(bool, obsess_over_services)},
    {"ochp_command", SETTER(std::string const&, ochp_command)},
    {"ochp_timeout", SETTER(unsigned int, ochp_timeout)},
    {"ocsp_command", SETTER(std::string const&, ocsp_command)},
    {"ocsp_timeout", SETTER(unsigned int, ocsp_timeout)},
    {"p1_file", SETTER(std::string const&, _set_p1_file)},
    {"passive_host_checks_are_soft",
     SETTER(bool, passive_host_checks_are_soft)},
    {"perfdata_timeout", SETTER(int, perfdata_timeout)},
    {"poller_name", SETTER(std::string const&, poller_name)},
    {"poller_id", SETTER(uint32_t, poller_id)},
    {"rpc_port", SETTER(uint16_t, rpc_port)},
    {"precached_object_file",
     SETTER(std::string const&, _set_precached_object_file)},
    {"process_performance_data", SETTER(bool, process_performance_data)},
    {"resource_file", SETTER(std::string const&, _set_resource_file)},
    {"retained_contact_host_attribute_mask",
     SETTER(unsigned long, retained_contact_host_attribute_mask)},
    {"retained_contact_service_attribute_mask",
     SETTER(unsigned long, retained_contact_service_attribute_mask)},
    {"retained_host_attribute_mask",
     SETTER(unsigned long, retained_host_attribute_mask)},
    {"retained_process_host_attribute_mask",
     SETTER(unsigned long, retained_process_host_attribute_mask)},
    {"retained_process_service_attribute_mask",
     SETTER(std::string const&, _set_retained_process_service_attribute_mask)},
    {"retained_service_attribute_mask",
     SETTER(std::string const&, _set_retained_service_attribute_mask)},
    {"retain_state_information", SETTER(bool, retain_state_information)},
    {"retention_scheduling_horizon",
     SETTER(unsigned int, retention_scheduling_horizon)},
    {"retention_update_interval",
     SETTER(unsigned int, retention_update_interval)},
    {"service_check_timeout", SETTER(unsigned int, service_check_timeout)},
    {"service_freshness_check_interval",
     SETTER(unsigned int, service_freshness_check_interval)},
    {"service_inter_check_delay_method",
     SETTER(std::string const&, _set_service_inter_check_delay_method)},
    {"service_interleave_factor",
     SETTER(std::string const&, _set_service_interleave_factor_method)},
    {"service_perfdata_command",
     SETTER(std::string const&, service_perfdata_command)},
    {"service_perfdata_file",
     SETTER(std::string const&, service_perfdata_file)},
    {"service_perfdata_file_mode",
     SETTER(std::string const&, _set_service_perfdata_file_mode)},
    {"service_perfdata_file_processing_command",
     SETTER(std::string const&, service_perfdata_file_processing_command)},
    {"service_perfdata_file_processing_interval",
     SETTER(unsigned int, service_perfdata_file_processing_interval)},
    {"service_perfdata_file_template",
     SETTER(std::string const&, service_perfdata_file_template)},
    {"service_reaper_frequency", SETTER(unsigned int, check_reaper_interval)},
    {"sleep_time", SETTER(float, sleep_time)},
    {"soft_state_dependencies", SETTER(bool, soft_state_dependencies)},
    {"state_retention_file", SETTER(std::string const&, state_retention_file)},
    {"status_file", SETTER(std::string const&, status_file)},
    {"status_update_interval", SETTER(unsigned int, status_update_interval)},
    {"temp_file", SETTER(std::string const&, _set_temp_file)},
    {"temp_path", SETTER(std::string const&, _set_temp_path)},
    {"time_change_threshold", SETTER(unsigned int, time_change_threshold)},
    {"translate_passive_host_checks",
     SETTER(bool, translate_passive_host_checks)},
    {"use_aggressive_host_checking",
     SETTER(bool, use_aggressive_host_checking)},
    {"use_agressive_host_checking", SETTER(bool, use_aggressive_host_checking)},
    {"use_embedded_perl_implicitly",
     SETTER(std::string const&, _set_use_embedded_perl_implicitly)},
    {"use_large_installation_tweaks",
     SETTER(bool, use_large_installation_tweaks)},
    {"instance_heartbeat_interval",
     SETTER(uint32_t, instance_heartbeat_interval)},
    {"use_regexp_matching", SETTER(bool, use_regexp_matches)},
    {"use_retained_program_state", SETTER(bool, use_retained_program_state)},
    {"use_retained_scheduling_info",
     SETTER(bool, use_retained_scheduling_info)},
    {"use_setpgid", SETTER(bool, use_setpgid)},
    {"use_syslog", SETTER(bool, use_syslog)},
    {"use_timezone", SETTER(std::string const&, use_timezone)},
    {"use_true_regexp_matching", SETTER(bool, use_true_regexp_matching)},
    {"xcddefault_comment_file", SETTER(std::string const&, _set_comment_file)},
    {"xdddefault_downtime_file",
     SETTER(std::string const&, _set_downtime_file)}};

// Default values.
static bool const default_accept_passive_host_checks(true);
static bool const default_accept_passive_service_checks(true);
static int const default_additional_freshness_latency(15);
static std::string const default_admin_email("");
static std::string const default_admin_pager("");
static bool const default_allow_empty_hostgroup_assignment(false);
static bool const default_auto_reschedule_checks(false);
static unsigned int const default_auto_rescheduling_interval(30);
static unsigned int const default_auto_rescheduling_window(180);
static std::string const default_broker_module_directory("");
static unsigned long const default_cached_host_check_horizon(15);
static unsigned long const default_cached_service_check_horizon(15);
static bool const default_check_external_commands(true);
static bool const default_check_host_freshness(false);
static bool const default_check_orphaned_hosts(true);
static bool const default_check_orphaned_services(true);
static unsigned int const default_check_reaper_interval(10);
static bool const default_check_service_freshness(true);
static int const default_command_check_interval(-1);
static std::string const default_command_file(DEFAULT_COMMAND_FILE);
static state::date_type const default_date_format(state::us);
static std::string const default_debug_file(DEFAULT_DEBUG_FILE);
static unsigned long long const default_debug_level(0);
static unsigned int const default_debug_verbosity(1);
static bool const default_enable_environment_macros(false);
static bool const default_enable_event_handlers(true);
static bool const default_enable_flap_detection(false);
static bool const default_enable_macros_filter(false);
static bool const default_enable_notifications(true);
static bool const default_enable_predictive_host_dependency_checks(true);
static bool const default_enable_predictive_service_dependency_checks(true);
static unsigned long const default_event_broker_options(
    std::numeric_limits<unsigned long>::max());
static unsigned int const default_event_handler_timeout(30);
static bool const default_execute_host_checks(true);
static bool const default_execute_service_checks(true);
static int const default_external_command_buffer_slots(4096);
static std::string const default_global_host_event_handler("");
static std::string const default_global_service_event_handler("");
static float const default_high_host_flap_threshold(30.0);
static float const default_high_service_flap_threshold(30.0);
static unsigned int const default_host_check_timeout(30);
static unsigned int const default_host_freshness_check_interval(60);
static state::inter_check_delay const default_host_inter_check_delay_method(
    state::icd_smart);
static state::perfdata_file_mode const default_host_perfdata_file_mode(
    state::mode_pipe);
static unsigned int const default_host_perfdata_file_processing_interval(0);
static std::string const default_host_perfdata_file_template(
    "[HOSTPERFDATA]\t$TIMET$\t$HOSTNAME$\t$HOSTEXECUTIONTIME$\t$HOSTOUTPUT$\t$"
    "HOSTPERFDATA$");
static std::string const default_illegal_object_chars("");
static std::string const default_illegal_output_chars("`~$&|'\"<>");
static unsigned int const default_interval_length(60);
static bool const default_log_event_handlers(true);
static bool const default_log_external_commands(true);
static std::string const default_log_file(DEFAULT_LOG_FILE);
static bool const default_log_host_retries(false);
static bool const default_log_notifications(true);
static bool const default_log_passive_checks(true);
static bool const default_log_pid(true);
static bool const default_log_service_retries(false);
static float const default_low_host_flap_threshold(20.0);
static float const default_low_service_flap_threshold(20.0);
static unsigned int const default_max_check_reaper_time(30);
static unsigned long const default_max_debug_file_size(1000000);
static unsigned int const default_max_host_check_spread(5);
static unsigned long const default_max_log_file_size(0);
static unsigned int const default_max_parallel_service_checks(0);
static unsigned int const default_max_service_check_spread(5);
static unsigned int const default_notification_timeout(30);
static bool const default_obsess_over_hosts(false);
static bool const default_obsess_over_services(false);
static std::string const default_ochp_command("");
static unsigned int const default_ochp_timeout(15);
static std::string const default_ocsp_command("");
static unsigned int const default_ocsp_timeout(15);
static bool const default_passive_host_checks_are_soft(false);
static int const default_perfdata_timeout(5);
static bool const default_process_performance_data(false);
static unsigned long const default_retained_contact_host_attribute_mask(0L);
static unsigned long const default_retained_contact_service_attribute_mask(0L);
static unsigned long const default_retained_host_attribute_mask(0L);
static unsigned long const default_retained_process_host_attribute_mask(0L);
static bool const default_retain_state_information(true);
static unsigned int const default_retention_scheduling_horizon(900);
static unsigned int const default_retention_update_interval(60);
static unsigned int const default_service_check_timeout(60);
static unsigned int const default_service_freshness_check_interval(60);
static state::inter_check_delay const default_service_inter_check_delay_method(
    state::icd_smart);
static state::interleave_factor const default_service_interleave_factor_method(
    state::ilf_smart);
static state::perfdata_file_mode const default_service_perfdata_file_mode(
    state::mode_pipe);
static unsigned int const default_service_perfdata_file_processing_interval(0);
static std::string const default_service_perfdata_file_template(
    "[SERVICEPERFDATA]\t$TIMET$\t$HOSTNAME$\t$SERVICEDESC$\t$"
    "SERVICEEXECUTIONTIME$\t$SERVICELATENCY$\t$SERVICEOUTPUT$\t$"
    "SERVICEPERFDATA$");
static float const default_sleep_time(0.5);
static bool const default_soft_state_dependencies(false);
static std::string const default_state_retention_file(DEFAULT_RETENTION_FILE);
static std::string const default_status_file(DEFAULT_STATUS_FILE);
static unsigned int const default_status_update_interval(60);
static unsigned int const default_time_change_threshold(900);
static bool const default_translate_passive_host_checks(false);
static bool const default_use_aggressive_host_checking(false);
static bool const default_use_large_installation_tweaks(false);
static uint32_t const default_instance_heartbeat_interval(30);
static bool const default_use_regexp_matches(false);
static bool const default_use_retained_program_state(true);
static bool const default_use_retained_scheduling_info(false);
static bool const default_use_setpgid(true);
static bool const default_use_syslog(true);
static std::string const default_use_timezone("");
static bool const default_use_true_regexp_matching(false);

/**
 *  Default constructor.
 */
state::state()
    : _accept_passive_host_checks(default_accept_passive_host_checks),
      _accept_passive_service_checks(default_accept_passive_service_checks),
      _additional_freshness_latency(default_additional_freshness_latency),
      _admin_email(default_admin_email),
      _admin_pager(default_admin_pager),
      _allow_empty_hostgroup_assignment(
          default_allow_empty_hostgroup_assignment),
      _auto_reschedule_checks(default_auto_reschedule_checks),
      _auto_rescheduling_interval(default_auto_rescheduling_interval),
      _auto_rescheduling_window(default_auto_rescheduling_window),
      _cached_host_check_horizon(default_cached_host_check_horizon),
      _cached_service_check_horizon(default_cached_service_check_horizon),
      _check_external_commands(default_check_external_commands),
      _check_host_freshness(default_check_host_freshness),
      _check_orphaned_hosts(default_check_orphaned_hosts),
      _check_orphaned_services(default_check_orphaned_services),
      _check_reaper_interval(default_check_reaper_interval),
      _check_service_freshness(default_check_service_freshness),
      _command_check_interval(default_command_check_interval),
      _command_check_interval_is_seconds(false),
      _command_file(default_command_file),
      _date_format(default_date_format),
      _debug_file(default_debug_file),
      _debug_level(default_debug_level),
      _debug_verbosity(default_debug_verbosity),
      _enable_environment_macros(default_enable_environment_macros),
      _enable_event_handlers(default_enable_event_handlers),
      _enable_flap_detection(default_enable_flap_detection),
      _enable_macros_filter(default_enable_macros_filter),
      _enable_notifications(default_enable_notifications),
      _enable_predictive_host_dependency_checks(
          default_enable_predictive_host_dependency_checks),
      _enable_predictive_service_dependency_checks(
          default_enable_predictive_service_dependency_checks),
      _event_broker_options(default_event_broker_options),
      _event_handler_timeout(default_event_handler_timeout),
      _execute_host_checks(default_execute_host_checks),
      _execute_service_checks(default_execute_service_checks),
      _external_command_buffer_slots(default_external_command_buffer_slots),
      _global_host_event_handler(default_global_host_event_handler),
      _global_service_event_handler(default_global_service_event_handler),
      _high_host_flap_threshold(default_high_host_flap_threshold),
      _high_service_flap_threshold(default_high_service_flap_threshold),
      _host_check_timeout(default_host_check_timeout),
      _host_freshness_check_interval(default_host_freshness_check_interval),
      _host_inter_check_delay_method(default_host_inter_check_delay_method),
      _host_perfdata_file_mode(default_host_perfdata_file_mode),
      _host_perfdata_file_processing_interval(
          default_host_perfdata_file_processing_interval),
      _host_perfdata_file_template(default_host_perfdata_file_template),
      _illegal_object_chars(default_illegal_object_chars),
      _illegal_output_chars(default_illegal_output_chars),
      _interval_length(default_interval_length),
      _log_event_handlers(default_log_event_handlers),
      _log_external_commands(default_log_external_commands),
      _log_file(default_log_file),
      _log_host_retries(default_log_host_retries),
      _log_notifications(default_log_notifications),
      _log_passive_checks(default_log_passive_checks),
      _log_pid(default_log_pid),
      _log_service_retries(default_log_service_retries),
      _low_host_flap_threshold(default_low_host_flap_threshold),
      _low_service_flap_threshold(default_low_service_flap_threshold),
      _max_check_reaper_time(default_max_check_reaper_time),
      _max_debug_file_size(default_max_debug_file_size),
      _max_host_check_spread(default_max_host_check_spread),
      _max_log_file_size(default_max_log_file_size),
      _max_parallel_service_checks(default_max_parallel_service_checks),
      _max_service_check_spread(default_max_service_check_spread),
      _notification_timeout(default_notification_timeout),
      _obsess_over_hosts(default_obsess_over_hosts),
      _obsess_over_services(default_obsess_over_services),
      _ochp_command(default_ochp_command),
      _ochp_timeout(default_ochp_timeout),
      _ocsp_command(default_ocsp_command),
      _ocsp_timeout(default_ocsp_timeout),
      _passive_host_checks_are_soft(default_passive_host_checks_are_soft),
      _perfdata_timeout(default_perfdata_timeout),
      _poller_name{"unknown"},
      _poller_id{0},
      _rpc_port{0},
      _process_performance_data(default_process_performance_data),
      _retained_contact_host_attribute_mask(
          default_retained_contact_host_attribute_mask),
      _retained_contact_service_attribute_mask(
          default_retained_contact_service_attribute_mask),
      _retained_host_attribute_mask(default_retained_host_attribute_mask),
      _retained_process_host_attribute_mask(
          default_retained_process_host_attribute_mask),
      _retain_state_information(default_retain_state_information),
      _retention_scheduling_horizon(default_retention_scheduling_horizon),
      _retention_update_interval(default_retention_update_interval),
      _service_check_timeout(default_service_check_timeout),
      _service_freshness_check_interval(
          default_service_freshness_check_interval),
      _service_inter_check_delay_method(
          default_service_inter_check_delay_method),
      _service_interleave_factor_method(
          default_service_interleave_factor_method),
      _service_perfdata_file_mode(default_service_perfdata_file_mode),
      _service_perfdata_file_processing_interval(
          default_service_perfdata_file_processing_interval),
      _service_perfdata_file_template(default_service_perfdata_file_template),
      _sleep_time(default_sleep_time),
      _soft_state_dependencies(default_soft_state_dependencies),
      _state_retention_file(default_state_retention_file),
      _status_file(default_status_file),
      _status_update_interval(default_status_update_interval),
      _time_change_threshold(default_time_change_threshold),
      _translate_passive_host_checks(default_translate_passive_host_checks),
      _use_aggressive_host_checking(default_use_aggressive_host_checking),
      _use_large_installation_tweaks(default_use_large_installation_tweaks),
      _instance_heartbeat_interval(default_instance_heartbeat_interval),
      _use_regexp_matches(default_use_regexp_matches),
      _use_retained_program_state(default_use_retained_program_state),
      _use_retained_scheduling_info(default_use_retained_scheduling_info),
      _use_setpgid(default_use_setpgid),
      _use_syslog(default_use_syslog),
      _use_timezone(default_use_timezone),
      _use_true_regexp_matching(default_use_true_regexp_matching) {}

/**
 *  Copy constructor.
 *
 *  @param[in] right The object to copy.
 */
state::state(state const& right) {
  operator=(right);
}

/**
 *  Destructor.
 */
state::~state() noexcept {}

/**
 *  Copy operator.
 *
 *  @param[in] right The object to copy.
 *
 *  @return This object.
 */
state& state::operator=(state const& right) {
  if (this != &right) {
    _accept_passive_host_checks = right._accept_passive_host_checks;
    _accept_passive_service_checks = right._accept_passive_service_checks;
    _additional_freshness_latency = right._additional_freshness_latency;
    _admin_email = right._admin_email;
    _admin_pager = right._admin_pager;
    _allow_empty_hostgroup_assignment = right._allow_empty_hostgroup_assignment;
    _auto_reschedule_checks = right._auto_reschedule_checks;
    _auto_rescheduling_interval = right._auto_rescheduling_interval;
    _auto_rescheduling_window = right._auto_rescheduling_window;
    _broker_module_directory = right._broker_module_directory;
    _cached_host_check_horizon = right._cached_host_check_horizon;
    _cached_service_check_horizon = right._cached_service_check_horizon;
    _check_external_commands = right._check_external_commands;
    _check_host_freshness = right._check_host_freshness;
    _check_orphaned_hosts = right._check_orphaned_hosts;
    _check_orphaned_services = right._check_orphaned_services;
    _check_reaper_interval = right._check_reaper_interval;
    _check_service_freshness = right._check_service_freshness;
    _commands = right._commands;
    _command_check_interval = right._command_check_interval;
    _command_check_interval_is_seconds =
        right._command_check_interval_is_seconds;
    _command_file = right._command_file;
    _connectors = right._connectors;
    _contactgroups = right._contactgroups;
    _contacts = right._contacts;
    _date_format = right._date_format;
    _debug_file = right._debug_file;
    _debug_level = right._debug_level;
    _debug_verbosity = right._debug_verbosity;
    _enable_environment_macros = right._enable_environment_macros;
    _enable_event_handlers = right._enable_event_handlers;
    _enable_flap_detection = right._enable_flap_detection;
    _enable_macros_filter = right._enable_macros_filter;
    _enable_notifications = right._enable_notifications;
    _enable_predictive_host_dependency_checks =
        right._enable_predictive_host_dependency_checks;
    _enable_predictive_service_dependency_checks =
        right._enable_predictive_service_dependency_checks;
    _event_broker_options = right._event_broker_options;
    _event_handler_timeout = right._event_handler_timeout;
    _execute_host_checks = right._execute_host_checks;
    _execute_service_checks = right._execute_service_checks;
    _external_command_buffer_slots = right._external_command_buffer_slots;
    _global_host_event_handler = right._global_host_event_handler;
    _global_service_event_handler = right._global_service_event_handler;
    _high_host_flap_threshold = right._high_host_flap_threshold;
    _high_service_flap_threshold = right._high_service_flap_threshold;
    _hostdependencies = right._hostdependencies;
    _hostescalations = right._hostescalations;
    _hostgroups = right._hostgroups;
    _hosts = right._hosts;
    _host_check_timeout = right._host_check_timeout;
    _host_freshness_check_interval = right._host_freshness_check_interval;
    _host_inter_check_delay_method = right._host_inter_check_delay_method;
    _host_perfdata_command = right._host_perfdata_command;
    _host_perfdata_file = right._host_perfdata_file;
    _host_perfdata_file_mode = right._host_perfdata_file_mode;
    _host_perfdata_file_processing_command =
        right._host_perfdata_file_processing_command;
    _host_perfdata_file_processing_interval =
        right._host_perfdata_file_processing_interval;
    _host_perfdata_file_template = right._host_perfdata_file_template;
    _illegal_object_chars = right._illegal_object_chars;
    _illegal_output_chars = right._illegal_output_chars;
    _interval_length = right._interval_length;
    _log_event_handlers = right._log_event_handlers;
    _log_external_commands = right._log_external_commands;
    _log_file = right._log_file;
    _log_host_retries = right._log_host_retries;
    _log_notifications = right._log_notifications;
    _log_passive_checks = right._log_passive_checks;
    _log_pid = right._log_pid;
    _log_service_retries = right._log_service_retries;
    _low_host_flap_threshold = right._low_host_flap_threshold;
    _low_service_flap_threshold = right._low_service_flap_threshold;
    _macros_filter = right._macros_filter;
    _max_check_reaper_time = right._max_check_reaper_time;
    _max_debug_file_size = right._max_debug_file_size;
    _max_host_check_spread = right._max_host_check_spread;
    _max_log_file_size = right._max_log_file_size;
    _max_parallel_service_checks = right._max_parallel_service_checks;
    _max_service_check_spread = right._max_service_check_spread;
    _notification_timeout = right._notification_timeout;
    _obsess_over_hosts = right._obsess_over_hosts;
    _obsess_over_services = right._obsess_over_services;
    _ochp_command = right._ochp_command;
    _ochp_timeout = right._ochp_timeout;
    _ocsp_command = right._ocsp_command;
    _ocsp_timeout = right._ocsp_timeout;
    _passive_host_checks_are_soft = right._passive_host_checks_are_soft;
    _perfdata_timeout = right._perfdata_timeout;
    _poller_name = right._poller_name;
    _poller_id = right._poller_id;
    _rpc_port = right._rpc_port;
    _process_performance_data = right._process_performance_data;
    _retained_contact_host_attribute_mask =
        right._retained_contact_host_attribute_mask;
    _retained_contact_service_attribute_mask =
        right._retained_contact_service_attribute_mask;
    _retained_host_attribute_mask = right._retained_host_attribute_mask;
    _retained_process_host_attribute_mask =
        right._retained_process_host_attribute_mask;
    _retain_state_information = right._retain_state_information;
    _retention_scheduling_horizon = right._retention_scheduling_horizon;
    _retention_update_interval = right._retention_update_interval;
    _servicedependencies = right._servicedependencies;
    _serviceescalations = right._serviceescalations;
    _servicegroups = right._servicegroups;
    _services = right._services;
    _service_check_timeout = right._service_check_timeout;
    _service_freshness_check_interval = right._service_freshness_check_interval;
    _service_inter_check_delay_method = right._service_inter_check_delay_method;
    _service_interleave_factor_method = right._service_interleave_factor_method;
    _service_perfdata_command = right._service_perfdata_command;
    _service_perfdata_file = right._service_perfdata_file;
    _service_perfdata_file_mode = right._service_perfdata_file_mode;
    _service_perfdata_file_processing_command =
        right._service_perfdata_file_processing_command;
    _service_perfdata_file_processing_interval =
        right._service_perfdata_file_processing_interval;
    _service_perfdata_file_template = right._service_perfdata_file_template;
    _sleep_time = right._sleep_time;
    _soft_state_dependencies = right._soft_state_dependencies;
    _state_retention_file = right._state_retention_file;
    _status_file = right._status_file;
    _status_update_interval = right._status_update_interval;
    _timeperiods = right._timeperiods;
    _time_change_threshold = right._time_change_threshold;
    _translate_passive_host_checks = right._translate_passive_host_checks;
    _users = right._users;
    _use_aggressive_host_checking = right._use_aggressive_host_checking;
    _use_large_installation_tweaks = right._use_large_installation_tweaks;
    _use_regexp_matches = right._use_regexp_matches;
    _use_retained_program_state = right._use_retained_program_state;
    _use_retained_scheduling_info = right._use_retained_scheduling_info;
    _use_setpgid = right._use_setpgid;
    _use_syslog = right._use_syslog;
    _use_timezone = right._use_timezone;
    _use_true_regexp_matching = right._use_true_regexp_matching;
  }
  return *this;
}

/**
 *  Equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object is the same object, otherwise false.
 */
bool state::operator==(state const& right) const noexcept {
  return (
      _accept_passive_host_checks == right._accept_passive_host_checks &&
      _accept_passive_service_checks == right._accept_passive_service_checks &&
      _additional_freshness_latency == right._additional_freshness_latency &&
      _admin_email == right._admin_email &&
      _admin_pager == right._admin_pager &&
      _allow_empty_hostgroup_assignment ==
          right._allow_empty_hostgroup_assignment &&
      _auto_reschedule_checks == right._auto_reschedule_checks &&
      _auto_rescheduling_interval == right._auto_rescheduling_interval &&
      _auto_rescheduling_window == right._auto_rescheduling_window &&
      _broker_module_directory == right._broker_module_directory &&
      _cached_host_check_horizon == right._cached_host_check_horizon &&
      _cached_service_check_horizon == right._cached_service_check_horizon &&
      _check_external_commands == right._check_external_commands &&
      _check_host_freshness == right._check_host_freshness &&
      _check_orphaned_hosts == right._check_orphaned_hosts &&
      _check_orphaned_services == right._check_orphaned_services &&
      _check_reaper_interval == right._check_reaper_interval &&
      _check_service_freshness == right._check_service_freshness &&
      _commands == right._commands &&
      _command_check_interval == right._command_check_interval &&
      _command_check_interval_is_seconds ==
          right._command_check_interval_is_seconds &&
      _command_file == right._command_file &&
      _connectors == right._connectors &&
      _contactgroups == right._contactgroups && _contacts == right._contacts &&
      _date_format == right._date_format && _debug_file == right._debug_file &&
      _debug_level == right._debug_level &&
      _debug_verbosity == right._debug_verbosity &&
      _enable_environment_macros == right._enable_environment_macros &&
      _enable_event_handlers == right._enable_event_handlers &&
      _enable_flap_detection == right._enable_flap_detection &&
      _enable_macros_filter == right._enable_macros_filter &&
      _enable_notifications == right._enable_notifications &&
      _enable_predictive_host_dependency_checks ==
          right._enable_predictive_host_dependency_checks &&
      _enable_predictive_service_dependency_checks ==
          right._enable_predictive_service_dependency_checks &&
      _event_broker_options == right._event_broker_options &&
      _event_handler_timeout == right._event_handler_timeout &&
      _execute_host_checks == right._execute_host_checks &&
      _execute_service_checks == right._execute_service_checks &&
      _external_command_buffer_slots == right._external_command_buffer_slots &&
      _global_host_event_handler == right._global_host_event_handler &&
      _global_service_event_handler == right._global_service_event_handler &&
      _high_host_flap_threshold == right._high_host_flap_threshold &&
      _high_service_flap_threshold == right._high_service_flap_threshold &&
      _hostdependencies == right._hostdependencies &&
      _hostescalations == right._hostescalations &&
      _hostgroups == right._hostgroups && _hosts == right._hosts &&
      _host_check_timeout == right._host_check_timeout &&
      _host_freshness_check_interval == right._host_freshness_check_interval &&
      _host_inter_check_delay_method == right._host_inter_check_delay_method &&
      _host_perfdata_command == right._host_perfdata_command &&
      _host_perfdata_file == right._host_perfdata_file &&
      _host_perfdata_file_mode == right._host_perfdata_file_mode &&
      _host_perfdata_file_processing_command ==
          right._host_perfdata_file_processing_command &&
      _host_perfdata_file_processing_interval ==
          right._host_perfdata_file_processing_interval &&
      _host_perfdata_file_template == right._host_perfdata_file_template &&
      _illegal_object_chars == right._illegal_object_chars &&
      _illegal_output_chars == right._illegal_output_chars &&
      _interval_length == right._interval_length &&
      _log_event_handlers == right._log_event_handlers &&
      _log_external_commands == right._log_external_commands &&
      _log_file == right._log_file &&
      _log_host_retries == right._log_host_retries &&
      _log_notifications == right._log_notifications &&
      _log_passive_checks == right._log_passive_checks &&
      _log_pid == right._log_pid &&
      _log_service_retries == right._log_service_retries &&
      _low_host_flap_threshold == right._low_host_flap_threshold &&
      _low_service_flap_threshold == right._low_service_flap_threshold &&
      _macros_filter == right._macros_filter &&
      _max_check_reaper_time == right._max_check_reaper_time &&
      _max_debug_file_size == right._max_debug_file_size &&
      _max_host_check_spread == right._max_host_check_spread &&
      _max_log_file_size == right._max_log_file_size &&
      _max_parallel_service_checks == right._max_parallel_service_checks &&
      _max_service_check_spread == right._max_service_check_spread &&
      _notification_timeout == right._notification_timeout &&
      _obsess_over_hosts == right._obsess_over_hosts &&
      _obsess_over_services == right._obsess_over_services &&
      _ochp_command == right._ochp_command &&
      _ochp_timeout == right._ochp_timeout &&
      _ocsp_command == right._ocsp_command &&
      _ocsp_timeout == right._ocsp_timeout &&
      _passive_host_checks_are_soft == right._passive_host_checks_are_soft &&
      _perfdata_timeout == right._perfdata_timeout &&
      _poller_name == right._poller_name && _poller_id == right._poller_id &&
      _rpc_port == right._rpc_port &&
      _process_performance_data == right._process_performance_data &&
      _retained_contact_host_attribute_mask ==
          right._retained_contact_host_attribute_mask &&
      _retained_contact_service_attribute_mask ==
          right._retained_contact_service_attribute_mask &&
      _retained_host_attribute_mask == right._retained_host_attribute_mask &&
      _retained_process_host_attribute_mask ==
          right._retained_process_host_attribute_mask &&
      _retain_state_information == right._retain_state_information &&
      _retention_scheduling_horizon == right._retention_scheduling_horizon &&
      _retention_update_interval == right._retention_update_interval &&
      _servicedependencies == right._servicedependencies &&
      _serviceescalations == right._serviceescalations &&
      _servicegroups == right._servicegroups && _services == right._services &&
      _service_check_timeout == right._service_check_timeout &&
      _service_freshness_check_interval ==
          right._service_freshness_check_interval &&
      _service_inter_check_delay_method ==
          right._service_inter_check_delay_method &&
      _service_interleave_factor_method ==
          right._service_interleave_factor_method &&
      _service_perfdata_command == right._service_perfdata_command &&
      _service_perfdata_file == right._service_perfdata_file &&
      _service_perfdata_file_mode == right._service_perfdata_file_mode &&
      _service_perfdata_file_processing_command ==
          right._service_perfdata_file_processing_command &&
      _service_perfdata_file_processing_interval ==
          right._service_perfdata_file_processing_interval &&
      _service_perfdata_file_template ==
          right._service_perfdata_file_template &&
      _sleep_time == right._sleep_time &&
      _soft_state_dependencies == right._soft_state_dependencies &&
      _state_retention_file == right._state_retention_file &&
      _status_file == right._status_file &&
      _status_update_interval == right._status_update_interval &&
      _timeperiods == right._timeperiods &&
      _time_change_threshold == right._time_change_threshold &&
      _translate_passive_host_checks == right._translate_passive_host_checks &&
      _users == right._users &&
      _use_aggressive_host_checking == right._use_aggressive_host_checking &&
      _use_large_installation_tweaks == right._use_large_installation_tweaks &&
      _use_regexp_matches == right._use_regexp_matches &&
      _use_retained_program_state == right._use_retained_program_state &&
      _use_retained_scheduling_info == right._use_retained_scheduling_info &&
      _use_setpgid == right._use_setpgid && _use_syslog == right._use_syslog &&
      _use_timezone == right._use_timezone &&
      _use_true_regexp_matching == right._use_true_regexp_matching);
}

/**
 *  Not equal operator.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object is not the same object, otherwise false.
 */
bool state::operator!=(state const& right) const noexcept {
  return !operator==(right);
}

/**
 *  Get accept_passive_host_checks value.
 *
 *  @return The accept_passive_host_checks value.
 */
bool state::accept_passive_host_checks() const noexcept {
  return _accept_passive_host_checks;
}

/**
 *  Set accept_passive_host_checks value.
 *
 *  @param[in] value The new accept_passive_host_checks value.
 */
void state::accept_passive_host_checks(bool value) {
  _accept_passive_host_checks = value;
}

/**
 *  Get accept_passive_service_checks value.
 *
 *  @return The accept_passive_service_checks value.
 */
bool state::accept_passive_service_checks() const noexcept {
  return _accept_passive_service_checks;
}

/**
 *  Set accept_passive_service_checks value.
 *
 *  @param[in] value The new accept_passive_service_checks value.
 */
void state::accept_passive_service_checks(bool value) {
  _accept_passive_service_checks = value;
}

/**
 *  Get additional_freshness_latency value.
 *
 *  @return The additional_freshness_latency value.
 */
int state::additional_freshness_latency() const noexcept {
  return _additional_freshness_latency;
}

/**
 *  Set additional_freshness_latency value.
 *
 *  @param[in] value The new additional_freshness_latency value.
 */
void state::additional_freshness_latency(int value) {
  _additional_freshness_latency = value;
}

/**
 *  Get admin_email value.
 *
 *  @return The admin_email value.
 */
std::string const& state::admin_email() const noexcept {
  return _admin_email;
}

/**
 *  Set admin_email value.
 *
 *  @param[in] value The new admin_email value.
 */
void state::admin_email(std::string const& value) {
  _admin_email = value;
}

/**
 *  Get admin_pager value.
 *
 *  @return The admin_pager value.
 */
std::string const& state::admin_pager() const noexcept {
  return _admin_pager;
}

/**
 *  Set admin_pager value.
 *
 *  @param[in] value The new admin_pager value.
 */
void state::admin_pager(std::string const& value) {
  _admin_pager = value;
}

/**
 *  Get allow_empty_hostgroup_assignment value.
 *
 *  @return The allow_empty_hostgroup_assignment value.
 */
bool state::allow_empty_hostgroup_assignment() const noexcept {
  return _allow_empty_hostgroup_assignment;
}

/**
 *  Set allow_empty_hostgroup_assignment value.
 *
 *  @param[in] value The new allow_empty_hostgroup_assignment value.
 */
void state::allow_empty_hostgroup_assignment(bool value) {
  _allow_empty_hostgroup_assignment = value;
}

/**
 *  Get auto_reschedule_checks value.
 *
 *  @return The auto_reschedule_checks value.
 */
bool state::auto_reschedule_checks() const noexcept {
  return _auto_reschedule_checks;
}

/**
 *  Set auto_reschedule_checks value.
 *
 *  @param[in] value The new auto_reschedule_checks value.
 */
void state::auto_reschedule_checks(bool value) {
  _auto_reschedule_checks = value;
}

/**
 *  Get auto_rescheduling_interval value.
 *
 *  @return The auto_rescheduling_interval value.
 */
unsigned int state::auto_rescheduling_interval() const noexcept {
  return _auto_rescheduling_interval;
}

/**
 *  Set auto_rescheduling_interval value.
 *
 *  @param[in] value The new auto_rescheduling_interval value.
 */
void state::auto_rescheduling_interval(unsigned int value) {
  if (!value)
    throw(engine_error() << "auto_rescheduling_interval cannot be 0");
  _auto_rescheduling_interval = value;
}

/**
 *  Get auto_rescheduling_window value.
 *
 *  @return The auto_rescheduling_window value.
 */
unsigned int state::auto_rescheduling_window() const noexcept {
  return _auto_rescheduling_window;
}

/**
 *  Set auto_rescheduling_window value.
 *
 *  @param[in] value The new auto_rescheduling_window value.
 */
void state::auto_rescheduling_window(unsigned int value) {
  if (!value)
    throw(engine_error() << "auto_rescheduling_window cannot be 0");
  _auto_rescheduling_window = value;
}

/**
 *  Get broker_module value.
 *
 *  @return The auto_rescheduling_window value.
 */
std::list<std::string> const& state::broker_module() const noexcept {
  return _broker_module;
}

/**
 *  Set broker_module value.
 *
 *  @param[in] value The new broker_module_directory value.
 */
void state::broker_module(std::list<std::string> const& value) {
  _broker_module = value;
}

/**
 *  Get broker_module_directory value.
 *
 *  @return The broker_module_directory value.
 */
std::string const& state::broker_module_directory() const noexcept {
  return _broker_module_directory;
}

/**
 *  Set broker_module_directory value.
 *
 *  @param[in] value The new broker_module_directory value.
 */
void state::broker_module_directory(std::string const& value) {
  if (value.empty() || value[0] == '/')
    _broker_module_directory = value;
  else {
    io::file_entry fe(_cfg_main);
    std::string base_name(fe.directory_name());
    _broker_module_directory = base_name + "/" + value;
  }
}

/**
 *  Get cached_host_check_horizon value.
 *
 *  @return The cached_host_check_horizon value.
 */
unsigned long state::cached_host_check_horizon() const noexcept {
  return _cached_host_check_horizon;
}

/**
 *  Set cached_host_check_horizon value.
 *
 *  @param[in] value The new cached_host_check_horizon value.
 */
void state::cached_host_check_horizon(unsigned long value) {
  _cached_host_check_horizon = value;
}

/**
 *  Get cached_service_check_horizon value.
 *
 *  @return The cached_service_check_horizon value.
 */
unsigned long state::cached_service_check_horizon() const noexcept {
  return _cached_service_check_horizon;
}

/**
 *  Set cached_service_check_horizon value.
 *
 *  @param[in] value The new cached_service_check_horizon value.
 */
void state::cached_service_check_horizon(unsigned long value) {
  _cached_service_check_horizon = value;
}

/**
 *  Get cfg_dir value.
 *
 *  @return The cfg_dir value.
 */
std::list<std::string> const& state::cfg_dir() const noexcept {
  return _cfg_dir;
}

/**
 *  Get cfg_file value.
 *
 *  @return The cfg_file value.
 */
std::list<std::string> const& state::cfg_file() const noexcept {
  return _cfg_file;
}

/**
 *  Get check_external_commands value.
 *
 *  @return The check_external_commands value.
 */
bool state::check_external_commands() const noexcept {
  return _check_external_commands;
}

/**
 *  Set cfg_main value.
 *
 *  @param[in] value The new cfg_main value.
 */
void state::cfg_main(std::string const& value) {
  _cfg_main = value;
}

/**
 *  Get cfg_main value.
 *
 *  @return The cfg_main value.
 */
std::string const& state::cfg_main() const noexcept {
  return _cfg_main;
}

/**
 *  Set check_external_commands value.
 *
 *  @param[in] value The new check_external_commands value.
 */
void state::check_external_commands(bool value) {
  _check_external_commands = value;
}

/**
 *  Get check_host_freshness value.
 *
 *  @return The check_host_freshness value.
 */
bool state::check_host_freshness() const noexcept {
  return _check_host_freshness;
}

/**
 *  Set check_host_freshness value.
 *
 *  @param[in] value The new check_host_freshness value.
 */
void state::check_host_freshness(bool value) {
  _check_host_freshness = value;
}

/**
 *  Get check_orphaned_hosts value.
 *
 *  @return The check_orphaned_hosts value.
 */
bool state::check_orphaned_hosts() const noexcept {
  return _check_orphaned_hosts;
}

/**
 *  Set check_orphaned_hosts value.
 *
 *  @param[in] value The new check_orphaned_hosts value.
 */
void state::check_orphaned_hosts(bool value) {
  _check_orphaned_hosts = value;
}

/**
 *  Set check_orphaned_services value.
 *
 *  @param[in] value The new check_orphaned_services value.
 */
void state::check_orphaned_services(bool value) {
  _check_orphaned_services = value;
}

/**
 *  Get check_orphaned_services value.
 *
 *  @return The check_orphaned_services value.
 */
bool state::check_orphaned_services() const noexcept {
  return _check_orphaned_services;
}

/**
 *  Get check_reaper_interval value.
 *
 *  @return The check_reaper_interval value.
 */
unsigned int state::check_reaper_interval() const noexcept {
  return _check_reaper_interval;
}

/**
 *  Set check_reaper_interval value.
 *
 *  @param[in] value The new check_reaper_interval value.
 */
void state::check_reaper_interval(unsigned int value) {
  if (!value)
    throw(engine_error() << "check_reaper_interval cannot be 0");
  _check_reaper_interval = value;
}

/**
 *  Get check_service_freshness value.
 *
 *  @return The check_service_freshness value.
 */
bool state::check_service_freshness() const noexcept {
  return _check_service_freshness;
}

/**
 *  Set check_service_freshness value.
 *
 *  @param[in] value The new check_service_freshness value.
 */
void state::check_service_freshness(bool value) {
  _check_service_freshness = value;
}

/**
 *  Get all engine commands.
 *
 *  @return All engine commands.
 */
set_command const& state::commands() const noexcept {
  return _commands;
}

/**
 *  Get all engine commands.
 *
 *  @return All engine commands.
 */
set_command& state::commands() noexcept {
  return _commands;
}

/**
 *  Find a command by its key.
 *
 *  @param[in] k Command name.
 *
 *  @return Iterator to the element if found, commands().end()
 *          otherwise.
 */
set_command::const_iterator state::commands_find(
    command::key_type const& k) const {
  configuration::command below_searched(k);
  set_command::const_iterator it(_commands.upper_bound(below_searched));
  if ((it != _commands.end()) && (it->command_name() == k))
    return it;
  else if ((it != _commands.begin()) && ((--it)->command_name() == k))
    return it;
  return _commands.end();
}

/**
 *  Find a command by its key.
 *
 *  @param[in] k Command name.
 *
 *  @return Iterator to the element if found, commands().end()
 *          otherwise.
 */
set_command::iterator state::commands_find(command::key_type const& k) {
  configuration::command below_searched(k);
  set_command::iterator it(_commands.upper_bound(below_searched));
  if ((it != _commands.end()) && (it->command_name() == k))
    return it;
  else if ((it != _commands.begin()) && ((--it)->command_name() == k))
    return it;
  return _commands.end();
}

/**
 *  Get command_check_interval value.
 *
 *  @return The command_check_interval value.
 */
int state::command_check_interval() const noexcept {
  return _command_check_interval;
}

/**
 *  Set command_check_interval value.
 *
 *  @param[in] value The new command_check_interval value.
 */
void state::command_check_interval(int value) {
  if (value < -1 || !value)
    throw(engine_error()
          << "command_check_interval must be either positive or -1 (" << value
          << " provided)");

  _command_check_interval = value;

  // adjust command check interval
  if (!_command_check_interval_is_seconds && _command_check_interval != -1)
    _command_check_interval *= _interval_length;
}

/**
 *  Set command_check_interval value.
 *
 *  @param[in] value     The new command_check_interval value.
 *  @param[in] is_second True if the value is in second.
 */
void state::command_check_interval(int value, bool is_second) {
  _command_check_interval_is_seconds = is_second;
  command_check_interval(value);
}

/**
 *  Return true is check interval is in seconds.
 *
 *  @return true if the check interval is in seconds.
 */
bool state::command_check_interval_is_seconds() const noexcept {
  return _command_check_interval_is_seconds;
}

/**
 *  Get command_file value.
 *
 *  @return The command_file value.
 */
std::string const& state::command_file() const noexcept {
  return _command_file;
}

/**
 *  Set command_file value.
 *
 *  @param[in] value The new command_file value.
 */
void state::command_file(std::string const& value) {
  _command_file = value;
}

/**
 *  Get all engine connectors.
 *
 *  @return All engine connectors.
 */
set_connector const& state::connectors() const noexcept {
  return _connectors;
}

/**
 *  Get all engine connectors.
 *
 *  @return All engine connectors.
 */
set_connector& state::connectors() noexcept {
  return _connectors;
}

/**
 *  Find a connector by its key.
 *
 *  @param[in] k Connector name.
 *
 *  @return Iterator to the element if found, connectors().end()
 *          otherwise.
 */
set_connector::const_iterator state::connectors_find(
    connector::key_type const& k) const {
  configuration::connector below_searched(k);
  set_connector::const_iterator it(_connectors.upper_bound(below_searched));
  if ((it != _connectors.end()) && (it->connector_name() == k))
    return it;
  else if ((it != _connectors.begin()) && ((--it)->connector_name() == k))
    return it;
  return _connectors.end();
}

/**
 *  Find a connector by its key.
 *
 *  @param[in] k Connector name.
 *
 *  @return Iterator to the element if found, connectors().end()
 *          otherwise.
 */
set_connector::iterator state::connectors_find(connector::key_type const& k) {
  configuration::connector below_searched(k);
  set_connector::iterator it(_connectors.upper_bound(below_searched));
  if ((it != _connectors.end()) && (it->connector_name() == k))
    return it;
  else if ((it != _connectors.begin()) && ((--it)->connector_name() == k))
    return it;
  return _connectors.end();
}

/**
 *  Get all engine contacts.
 *
 *  @return All engine contacts.
 */
set_contact const& state::contacts() const noexcept {
  return _contacts;
}

/**
 *  Get all engine contacts.
 *
 *  @return All engine contacts.
 */
set_contact& state::contacts() noexcept {
  return _contacts;
}

/**
 *  Find a contact by its key.
 *
 *  @param[in] k Contact name.
 *
 *  @return Iterator to the element if found, contacts().end()
 *          otherwise.
 */
set_contact::const_iterator state::contacts_find(
    contact::key_type const& k) const {
  configuration::contact below_searched(k);
  set_contact::const_iterator it(_contacts.upper_bound(below_searched));
  if ((it != _contacts.end()) && (it->contact_name() == k))
    return it;
  else if ((it != _contacts.begin()) && ((--it)->contact_name() == k))
    return it;
  return _contacts.end();
}

/**
 *  Find a contact by its key.
 *
 *  @param[in] k Contact name.
 *
 *  @return Iterator to the element if found, contacts().end()
 *          otherwise.
 */
set_contact::iterator state::contacts_find(contact::key_type const& k) {
  configuration::contact below_searched(k);
  set_contact::iterator it(_contacts.upper_bound(below_searched));
  if ((it != _contacts.end()) && (it->contact_name() == k))
    return it;
  else if ((it != _contacts.begin()) && ((--it)->contact_name() == k))
    return it;
  return _contacts.end();
}

/**
 *  Find a contact group by its key.
 *
 *  @param[in] k Contact group key.
 *
 *  @return Iterator to the element if found, contactgroups().end()
 *          otherwise.
 */
set_contactgroup::const_iterator state::contactgroups_find(
    contactgroup::key_type const& k) const {
  configuration::contactgroup below_searched(k);
  set_contactgroup::const_iterator it(
      _contactgroups.upper_bound(below_searched));
  if ((it != _contactgroups.end()) && (it->contactgroup_name() == k))
    return it;
  else if ((it != _contactgroups.begin()) && ((--it)->contactgroup_name() == k))
    return it;
  return _contactgroups.end();
}

/**
 *  Find a contact group by its key.
 *
 *  @param[in] k Contact group key.
 *
 *  @return Iterator to the element if found, contactgroups().end()
 *          otherwise.
 */
set_contactgroup::iterator state::contactgroups_find(
    contactgroup::key_type const& k) {
  configuration::contactgroup below_searched(k);
  set_contactgroup::iterator it(_contactgroups.upper_bound(below_searched));
  if ((it != _contactgroups.end()) && (it->contactgroup_name() == k))
    return it;
  else if ((it != _contactgroups.begin()) && ((--it)->contactgroup_name() == k))
    return it;
  return _contactgroups.end();
}

/**
 *  Get all engine contactgroups.
 *
 *  @return All engine contactgroups.
 */
set_contactgroup& state::contactgroups() noexcept {
  return _contactgroups;
}

/**
 *  Get all engine contactgroups.
 *
 *  @return All engine contactgroups.
 */
set_contactgroup const& state::contactgroups() const noexcept {
  return _contactgroups;
}

/**
 *  Get date_format value.
 *
 *  @return The date_format value.
 */
state::date_type state::date_format() const noexcept {
  return _date_format;
}

/**
 *  Set date_format value.
 *
 *  @param[in] value The new date_format value.
 */
void state::date_format(date_type value) {
  _date_format = value;
}

/**
 *  Get debug_file value.
 *
 *  @return The debug_file value.
 */
std::string const& state::debug_file() const noexcept {
  return _debug_file;
}

/**
 *  Set debug_file value.
 *
 *  @param[in] value The new debug_file value.
 */
void state::debug_file(std::string const& value) {
  _debug_file = value;
}

/**
 *  Get debug_level value.
 *
 *  @return The debug_level value.
 */
unsigned long long state::debug_level() const noexcept {
  return _debug_level;
}

/**
 *  Set debug_level value.
 *
 *  @param[in] value The new debug_level value.
 */
void state::debug_level(unsigned long long value) {
  if (value == std::numeric_limits<unsigned long>::max())
    _debug_level = static_cast<unsigned long long>(all);
  else
    _debug_level = value;
}

/**
 *  Get debug_verbosity value.
 *
 *  @return The debug_verbosity value.
 */
unsigned int state::debug_verbosity() const noexcept {
  return _debug_verbosity;
}

/**
 *  Set debug_verbosity value.
 *
 *  @param[in] value The new debug_verbosity value.
 */
void state::debug_verbosity(unsigned int value) {
  if (value > most)
    _debug_verbosity = static_cast<unsigned int>(most);
  else
    _debug_verbosity = value;
}

/**
 *  Get enable_environment_macros value.
 *
 *  @return The enable_environment_macros value.
 */
bool state::enable_environment_macros() const noexcept {
  return _enable_environment_macros;
}

/**
 *  Set enable_environment_macros value.
 *
 *  @param[in] value The new enable_environment_macros value.
 */
void state::enable_environment_macros(bool value) {
  _enable_environment_macros = value;
}

/**
 *  Get enable_event_handlers value.
 *
 *  @return The enable_event_handlers value.
 */
bool state::enable_event_handlers() const noexcept {
  return _enable_event_handlers;
}

/**
 *  Set enable_event_handlers value.
 *
 *  @param[in] value The new enable_event_handlers value.
 */
void state::enable_event_handlers(bool value) {
  _enable_event_handlers = value;
}

/**
 *  Get enable_flap_detection value.
 *
 *  @return The enable_flap_detection value.
 */
bool state::enable_flap_detection() const noexcept {
  return _enable_flap_detection;
}

/**
 *  Set enable_flap_detection value.
 *
 *  @param[in] value The new enable_flap_detection value.
 */
void state::enable_flap_detection(bool value) {
  _enable_flap_detection = value;
}

/**
 *  Get enable_notifications value.
 *
 *  @return The enable_notifications value.
 */
bool state::enable_notifications() const noexcept {
  return _enable_notifications;
}

/**
 *  Set enable_notifications value.
 *
 *  @param[in] value The new enable_notifications value.
 */
void state::enable_notifications(bool value) {
  _enable_notifications = value;
}

/**
 *  Get enable_predictive_host_dependency_checks value.
 *
 *  @return The enable_predictive_host_dependency_checks value.
 */
bool state::enable_predictive_host_dependency_checks() const noexcept {
  return _enable_predictive_host_dependency_checks;
}

/**
 *  Set enable_predictive_host_dependency_checks value.
 *
 *  @param[in] value The new enable_predictive_host_dependency_checks value.
 */
void state::enable_predictive_host_dependency_checks(bool value) {
  _enable_predictive_host_dependency_checks = value;
}

/**
 *  Get enable_predictive_service_dependency_checks value.
 *
 *  @return The enable_predictive_service_dependency_checks value.
 */
bool state::enable_predictive_service_dependency_checks() const noexcept {
  return _enable_predictive_service_dependency_checks;
}

/**
 *  Set enable_predictive_service_dependency_checks value.
 *
 *  @param[in] value The new enable_predictive_service_dependency_checks value.
 */
void state::enable_predictive_service_dependency_checks(bool value) {
  _enable_predictive_service_dependency_checks = value;
}

/**
 *  Get event_broker_options value.
 *
 *  @return The event_broker_options value.
 */
unsigned long state::event_broker_options() const noexcept {
  return _event_broker_options;
}

/**
 *  Set event_broker_options value.
 *
 *  @param[in] value The new event_broker_options value.
 */
void state::event_broker_options(unsigned long value) {
  _event_broker_options = value;
}

/**
 *  Get event_handler_timeout value.
 *
 *  @return The event_handler_timeout value.
 */
unsigned int state::event_handler_timeout() const noexcept {
  return _event_handler_timeout;
}

/**
 *  Set event_handler_timeout value.
 *
 *  @param[in] value The new event_handler_timeout value.
 */
void state::event_handler_timeout(unsigned int value) {
  if (!value)
    throw(engine_error() << "event_handler_timeout cannot be 0");
  _event_handler_timeout = value;
}

/**
 *  Get execute_host_checks value.
 *
 *  @return The execute_host_checks value.
 */
bool state::execute_host_checks() const noexcept {
  return _execute_host_checks;
}

/**
 *  Set execute_host_checks value.
 *
 *  @param[in] value The new execute_host_checks value.
 */
void state::execute_host_checks(bool value) {
  _execute_host_checks = value;
}

/**
 *  Get execute_service_checks value.
 *
 *  @return The execute_service_checks value.
 */
bool state::execute_service_checks() const noexcept {
  return _execute_service_checks;
}

/**
 *  Set execute_service_checks value.
 *
 *  @param[in] value The new execute_service_checks value.
 */
void state::execute_service_checks(bool value) {
  _execute_service_checks = value;
}

/**
 *  Get external_command_buffer_slots value.
 *
 *  @return The external_command_buffer_slots value.
 */
int state::external_command_buffer_slots() const noexcept {
  return _external_command_buffer_slots;
}

/**
 *  Set external_command_buffer_slots value.
 *
 *  @param[in] value The new external_command_buffer_slots value.
 */
void state::external_command_buffer_slots(int value) {
  _external_command_buffer_slots = value;
}

/**
 *  Get global_host_event_handler value.
 *
 *  @return The global_host_event_handler value.
 */
std::string const& state::global_host_event_handler() const noexcept {
  return _global_host_event_handler;
}

/**
 *  Set global_host_event_handler value.
 *
 *  @param[in] value The new global_host_event_handler value.
 */
void state::global_host_event_handler(std::string const& value) {
  _global_host_event_handler = value;
}

/**
 *  Get global_service_event_handler value.
 *
 *  @return The global_service_event_handler value.
 */
std::string const& state::global_service_event_handler() const noexcept {
  return _global_service_event_handler;
}

/**
 *  Set global_service_event_handler value.
 *
 *  @param[in] value The new global_service_event_handler value.
 */
void state::global_service_event_handler(std::string const& value) {
  _global_service_event_handler = value;
}

/**
 *  Get high_host_flap_threshold value.
 *
 *  @return The high_host_flap_threshold value.
 */
float state::high_host_flap_threshold() const noexcept {
  return _high_host_flap_threshold;
}

/**
 *  Set high_host_flap_threshold value.
 *
 *  @param[in] value The new high_host_flap_threshold value.
 */
void state::high_host_flap_threshold(float value) {
  if (value <= 0.0 || value >= 100.0)
    throw(engine_error() << "high_host_flap_threshold must "
                         << "be between 0.0 and 100.0, both excluded");
  _high_host_flap_threshold = value;
}

/**
 *  Get high_service_flap_threshold value.
 *
 *  @return The high_service_flap_threshold value.
 */
float state::high_service_flap_threshold() const noexcept {
  return _high_service_flap_threshold;
}

/**
 *  Set high_service_flap_threshold value.
 *
 *  @param[in] value The new high_service_flap_threshold value.
 */
void state::high_service_flap_threshold(float value) {
  if (value <= 0.0 || value >= 100.0)
    throw(engine_error() << "high_service_flap_threshold "
                         << "must be between 0.0 and 100.0, both excluded");
  _high_service_flap_threshold = value;
}

/**
 *  Get all engine hostdependencies.
 *
 *  @return All engine hostdependencies.
 */
set_hostdependency const& state::hostdependencies() const noexcept {
  return _hostdependencies;
}

/**
 *  Get all engine hostdependencies.
 *
 *  @return All engine hostdependencies.
 */
set_hostdependency& state::hostdependencies() noexcept {
  return _hostdependencies;
}

/**
 *  Get all engine hostescalations.
 *
 *  @return All engine hostescalations.
 */
set_hostescalation const& state::hostescalations() const noexcept {
  return _hostescalations;
}

/**
 *  Get all engine hostescalations.
 *
 *  @return All engine hostescalations.
 */
set_hostescalation& state::hostescalations() noexcept {
  return _hostescalations;
}

/**
 *  Get all engine hostgroups.
 *
 *  @return All engine hostgroups.
 */
set_hostgroup const& state::hostgroups() const noexcept {
  return _hostgroups;
}

/**
 *  Get all engine hostgroups.
 *
 *  @return All engine hostgroups.
 */
set_hostgroup& state::hostgroups() noexcept {
  return _hostgroups;
}

/**
 *  Find a host group by its key.
 *
 *  @param[in] k Host group key.
 *
 *  @return Iterator to the element if found, hostgroups().end()
 *          otherwise.
 */
set_hostgroup::const_iterator state::hostgroups_find(
    hostgroup::key_type const& k) const {
  configuration::hostgroup below_searched(k);
  set_hostgroup::const_iterator it(_hostgroups.upper_bound(below_searched));
  if (it != _hostgroups.end() && it->hostgroup_name() == k)
    return it;
  else if (it != _hostgroups.begin() && (--it)->hostgroup_name() == k)
    return it;
  return _hostgroups.end();
}

/**
 *  Find a host group by its key.
 *
 *  @param[in] k Host group key.
 *
 *  @return Iterator to the element if found, hostgroups().end()
 *          otherwise.
 */
set_hostgroup::iterator state::hostgroups_find(hostgroup::key_type const& k) {
  configuration::hostgroup below_searched(k);
  set_hostgroup::iterator it(_hostgroups.upper_bound(below_searched));
  if ((it != _hostgroups.end()) && (it->hostgroup_name() == k))
    return it;
  else if ((it != _hostgroups.begin()) && ((--it)->hostgroup_name() == k))
    return it;
  return _hostgroups.end();
}

/**
 *  Get all engine hosts.
 *
 *  @return All engine hosts.
 */
set_host const& state::hosts() const noexcept {
  return _hosts;
}

/**
 *  Get all engine hosts.
 *
 *  @return All engine hosts.
 */
set_host& state::hosts() noexcept {
  return _hosts;
}

/**
 *  Find a host from its key.
 *
 *  @param[in] k Host key (host name).
 *
 *  @return Iterator to the host if found, hosts().end() if it was not.
 */
set_host::const_iterator state::hosts_find(host::key_type const& k) const {
  configuration::host below_searched(k);
  set_host::const_iterator it(_hosts.upper_bound(below_searched));
  if ((it != _hosts.end()) && (it->host_id() == k))
    return it;
  else if ((it != _hosts.begin()) && ((--it)->host_id() == k))
    return it;
  return _hosts.end();
}

set_host::const_iterator state::hosts_find(std::string const& name) const {
  for (set_host::const_iterator it(_hosts.begin()), end(_hosts.end());
       it != end; ++it) {
    if (it->host_name() == name)
      return it;
  }
  return _hosts.end();
}

/**
 *  Find a host from its key.
 *
 *  @param[in] k Host key (host name).
 *
 *  @return Iterator to the host if found, hosts().end() if it was not.
 */
set_host::iterator state::hosts_find(host::key_type const& k) {
  configuration::host below_searched(k);
  set_host::iterator it(_hosts.upper_bound(below_searched));
  if (it != _hosts.end() && it->host_id() == k)
    return it;
  else if (it != _hosts.begin() && (--it)->host_id() == k)
    return it;
  return _hosts.end();
}

/**
 *  Get host_check_timeout value.
 *
 *  @return The host_check_timeout value.
 */
unsigned int state::host_check_timeout() const noexcept {
  return _host_check_timeout;
}

/**
 *  Set host_check_timeout value.
 *
 *  @param[in] value The new host_check_timeout value.
 */
void state::host_check_timeout(unsigned int value) {
  if (!value)
    throw(engine_error() << "host_check_timeout cannot be 0");
  _host_check_timeout = value;
}

/**
 *  Get host_freshness_check_interval value.
 *
 *  @return The host_freshness_check_interval value.
 */
unsigned int state::host_freshness_check_interval() const noexcept {
  return _host_freshness_check_interval;
}

/**
 *  Set host_freshness_check_interval value.
 *
 *  @param[in] value The new host_freshness_check_interval value.
 */
void state::host_freshness_check_interval(unsigned int value) {
  _host_freshness_check_interval = value;
}

/**
 *  Get host_inter_check_delay_method value.
 *
 *  @return The host_inter_check_delay_method value.
 */
state::inter_check_delay state::host_inter_check_delay_method() const noexcept {
  return _host_inter_check_delay_method;
}

/**
 *  Set host_inter_check_delay_method value.
 *
 *  @param[in] value The new host_inter_check_delay_method value.
 */
void state::host_inter_check_delay_method(inter_check_delay value) {
  _host_inter_check_delay_method = value;
}

/**
 *  Get host_perfdata_command value.
 *
 *  @return The host_perfdata_command value.
 */
std::string const& state::host_perfdata_command() const noexcept {
  return _host_perfdata_command;
}

/**
 *  Set host_perfdata_command value.
 *
 *  @param[in] value The new host_perfdata_command value.
 */
void state::host_perfdata_command(std::string const& value) {
  _host_perfdata_command = value;
}

/**
 *  Get host_perfdata_file value.
 *
 *  @return The host_perfdata_file value.
 */
std::string const& state::host_perfdata_file() const noexcept {
  return _host_perfdata_file;
}

/**
 *  Set host_perfdata_file value.
 *
 *  @param[in] value The new host_perfdata_file value.
 */
void state::host_perfdata_file(std::string const& value) {
  _host_perfdata_file = value;
}

/**
 *  Get host_perfdata_file_mode value.
 *
 *  @return The host_perfdata_file_mode value.
 */
state::perfdata_file_mode state::host_perfdata_file_mode() const noexcept {
  return _host_perfdata_file_mode;
}

/**
 *  Set host_perfdata_file_mode value.
 *
 *  @param[in] value The new host_perfdata_file_mode value.
 */
void state::host_perfdata_file_mode(perfdata_file_mode value) {
  _host_perfdata_file_mode = value;
}

/**
 *  Get host_perfdata_file_processing_command value.
 *
 *  @return The host_perfdata_file_processing_command value.
 */
std::string const& state::host_perfdata_file_processing_command() const
    noexcept {
  return _host_perfdata_file_processing_command;
}

/**
 *  Set host_perfdata_file_processing_command value.
 *
 *  @param[in] value The new host_perfdata_file_processing_command value.
 */
void state::host_perfdata_file_processing_command(std::string const& value) {
  _host_perfdata_file_processing_command = value;
}

/**
 *  Get host_perfdata_file_processing_interval value.
 *
 *  @return The host_perfdata_file_processing_interval value.
 */
unsigned int state::host_perfdata_file_processing_interval() const noexcept {
  return _host_perfdata_file_processing_interval;
}

/**
 *  Set host_perfdata_file_processing_interval value.
 *
 *  @param[in] value The new host_perfdata_file_processing_interval value.
 */
void state::host_perfdata_file_processing_interval(unsigned int value) {
  _host_perfdata_file_processing_interval = value;
}

/**
 *  Get host_perfdata_file_template value.
 *
 *  @return The host_perfdata_file_template value.
 */
std::string const& state::host_perfdata_file_template() const noexcept {
  return _host_perfdata_file_template;
}

/**
 *  Set host_perfdata_file_template value.
 *
 *  @param[in] value The new host_perfdata_file_template value.
 */
void state::host_perfdata_file_template(std::string const& value) {
  _host_perfdata_file_template = value;
}

/**
 *  Get illegal_object_chars value.
 *
 *  @return The illegal_object_chars value.
 */
std::string const& state::illegal_object_chars() const noexcept {
  return _illegal_object_chars;
}

/**
 *  Set illegal_object_chars value.
 *
 *  @param[in] value The new illegal_object_chars value.
 */
void state::illegal_object_chars(std::string const& value) {
  _illegal_object_chars = value;
}

/**
 *  Get illegal_output_chars value.
 *
 *  @return The illegal_output_chars value.
 */
std::string const& state::illegal_output_chars() const noexcept {
  return _illegal_output_chars;
}

/**
 *  Set illegal_output_chars value.
 *
 *  @param[in] value The new illegal_output_chars value.
 */
void state::illegal_output_chars(std::string const& value) {
  _illegal_output_chars = value;
}

/**
 *  Get interval_length value.
 *
 *  @return The interval_length value.
 */
unsigned int state::interval_length() const noexcept {
  return _interval_length;
}

/**
 *  Set interval_length value.
 *
 *  @param[in] value The new interval_length value.
 */
void state::interval_length(unsigned int value) {
  if (!value)
    throw(engine_error() << "interval_length cannot be 0");

  if (!_command_check_interval_is_seconds && _command_check_interval != -1) {
    _command_check_interval /= _interval_length;
    _interval_length = value;
    _command_check_interval *= _interval_length;
  } else
    _interval_length = value;
}

/**
 *  Get log_event_handlers value.
 *
 *  @return The log_event_handlers value.
 */
bool state::log_event_handlers() const noexcept {
  return _log_event_handlers;
}

/**
 *  Set log_event_handlers value.
 *
 *  @param[in] value The new log_event_handlers value.
 */
void state::log_event_handlers(bool value) {
  _log_event_handlers = value;
}

/**
 *  Get log_external_commands value.
 *
 *  @return The log_external_commands value.
 */
bool state::log_external_commands() const noexcept {
  return _log_external_commands;
}

/**
 *  Set log_external_commands value.
 *
 *  @param[in] value The new log_external_commands value.
 */
void state::log_external_commands(bool value) {
  _log_external_commands = value;
}

/**
 *  Get log_file value.
 *
 *  @return The log_file value.
 */
std::string const& state::log_file() const noexcept {
  return _log_file;
}

/**
 *  Set log_file value.
 *
 *  @param[in] value The new log_file value.
 */
void state::log_file(std::string const& value) {
  _log_file = value;
}

/**
 *  Get log_host_retries value.
 *
 *  @return The log_host_retries value.
 */
bool state::log_host_retries() const noexcept {
  return _log_host_retries;
}

/**
 *  Set log_host_retries value.
 *
 *  @param[in] value The new log_host_retries value.
 */
void state::log_host_retries(bool value) {
  _log_host_retries = value;
}

/**
 *  Get log_notifications value.
 *
 *  @return The log_notifications value.
 */
bool state::log_notifications() const noexcept {
  return _log_notifications;
}

/**
 *  Set log_notifications value.
 *
 *  @param[in] value The new log_notifications value.
 */
void state::log_notifications(bool value) {
  _log_notifications = value;
}

/**
 *  Get log_passive_checks value.
 *
 *  @return The log_passive_checks value.
 */
bool state::log_passive_checks() const noexcept {
  return _log_passive_checks;
}

/**
 *  Set log_passive_checks value.
 *
 *  @param[in] value The new log_passive_checks value.
 */
void state::log_passive_checks(bool value) {
  _log_passive_checks = value;
}

/**
 *  Get log pid value.
 *
 *  @return  Log pid value.
 */
bool state::log_pid() const noexcept {
  return _log_pid;
}

/**
 *  Set the log pid value.
 *
 *  @param[in] value  The new log pid value.
 */
void state::log_pid(bool value) {
  _log_pid = value;
}

/**
 *  Get log_service_retries value.
 *
 *  @return The log_service_retries value.
 */
bool state::log_service_retries() const noexcept {
  return _log_service_retries;
}

/**
 *  Set log_service_retries value.
 *
 *  @param[in] value The new log_service_retries value.
 */
void state::log_service_retries(bool value) {
  _log_service_retries = value;
}

/**
 *  Get low_host_flap_threshold value.
 *
 *  @return The low_host_flap_threshold value.
 */
float state::low_host_flap_threshold() const noexcept {
  return _low_host_flap_threshold;
}

/**
 *  Set low_host_flap_threshold value.
 *
 *  @param[in] value The new low_host_flap_threshold value.
 */
void state::low_host_flap_threshold(float value) {
  if (value <= 0.0 || value >= 100.0)
    throw(engine_error() << "low_host_flap_threshold "
                         << "must be between 0.0 and 100.0, both excluded");
  _low_host_flap_threshold = value;
}

/**
 *  Get low_service_flap_threshold value.
 *
 *  @return The low_service_flap_threshold value.
 */
float state::low_service_flap_threshold() const noexcept {
  return _low_service_flap_threshold;
}

/**
 *  Set low_service_flap_threshold value.
 *
 *  @param[in] value The new low_service_flap_threshold value.
 */
void state::low_service_flap_threshold(float value) {
  if (value <= 0.0 || value >= 100.0)
    throw(engine_error() << "low_service_flap_threshold "
                         << "must be between 0.0 and 100.0, both excluded");
  _low_service_flap_threshold = value;
}

/**
 *  Get max_check_reaper_time value.
 *
 *  @return The max_check_reaper_time value.
 */
unsigned int state::max_check_reaper_time() const noexcept {
  return _max_check_reaper_time;
}

/**
 *  Set max_check_reaper_time value.
 *
 *  @param[in] value The new max_check_reaper_time value.
 */
void state::max_check_reaper_time(unsigned int value) {
  if (!value)
    throw(engine_error() << "max_check_reaper_time cannot be 0");
  _max_check_reaper_time = value;
}

/**
 *  Get max_debug_file_size value.
 *
 *  @return The max_debug_file_size value.
 */
unsigned long state::max_debug_file_size() const noexcept {
  return _max_debug_file_size;
}

/**
 *  Set max_debug_file_size value.
 *
 *  @param[in] value The new max_debug_file_size value.
 */
void state::max_debug_file_size(unsigned long value) {
  _max_debug_file_size = value;
}

/**
 *  Get max_host_check_spread value.
 *
 *  @return The max_host_check_spread value.
 */
unsigned int state::max_host_check_spread() const noexcept {
  return _max_host_check_spread;
}

/**
 *  Set max_host_check_spread value.
 *
 *  @param[in] value The new max_host_check_spread value.
 */
void state::max_host_check_spread(unsigned int value) {
  if (!value)
    throw(engine_error() << "max_host_check_spread cannot be 0");
  _max_host_check_spread = value;
}

/**
 *  Get max_log_file_size value.
 *
 *  @return The max_log_file_size value.
 */
unsigned long state::max_log_file_size() const noexcept {
  return _max_log_file_size;
}

/**
 *  Set max_log_file_size value.
 *
 *  @param[in] value The new max_log_file_size value.
 */
void state::max_log_file_size(unsigned long value) {
  _max_log_file_size = value;
}

/**
 *  Get max_parallel_service_checks value.
 *
 *  @return The max_parallel_service_checks value.
 */
unsigned int state::max_parallel_service_checks() const noexcept {
  return _max_parallel_service_checks;
}

/**
 *  Set max_parallel_service_checks value.
 *
 *  @param[in] value The new max_parallel_service_checks value.
 */
void state::max_parallel_service_checks(unsigned int value) {
  _max_parallel_service_checks = value;
}

/**
 *  Get max_service_check_spread value.
 *
 *  @return The max_service_check_spread value.
 */
unsigned int state::max_service_check_spread() const noexcept {
  return _max_service_check_spread;
}

/**
 *  Set max_service_check_spread value.
 *
 *  @param[in] value The new max_service_check_spread value.
 */
void state::max_service_check_spread(unsigned int value) {
  if (!value)
    throw(engine_error() << "max_service_check_spread cannot be 0");
  _max_service_check_spread = value;
}

/**
 *  Get notification_timeout value.
 *
 *  @return The notification_timeout value.
 */
unsigned int state::notification_timeout() const noexcept {
  return _notification_timeout;
}

/**
 *  Set notification_timeout value.
 *
 *  @param[in] value The new notification_timeout value.
 */
void state::notification_timeout(unsigned int value) {
  if (!value)
    throw(engine_error() << "notification_timeout cannot be 0");
  _notification_timeout = value;
}

/**
 *  Get obsess_over_hosts value.
 *
 *  @return The obsess_over_hosts value.
 */
bool state::obsess_over_hosts() const noexcept {
  return _obsess_over_hosts;
}

/**
 *  Set obsess_over_hosts value.
 *
 *  @param[in] value The new obsess_over_hosts value.
 */
void state::obsess_over_hosts(bool value) {
  _obsess_over_hosts = value;
}

/**
 *  Get obsess_over_services value.
 *
 *  @return The obsess_over_services value.
 */
bool state::obsess_over_services() const noexcept {
  return _obsess_over_services;
}

/**
 *  Set obsess_over_services value.
 *
 *  @param[in] value The new obsess_over_services value.
 */
void state::obsess_over_services(bool value) {
  _obsess_over_services = value;
}

/**
 *  Get ochp_command value.
 *
 *  @return The ochp_command value.
 */
std::string const& state::ochp_command() const noexcept {
  return _ochp_command;
}

/**
 *  Set ochp_command value.
 *
 *  @param[in] value The new ochp_command value.
 */
void state::ochp_command(std::string const& value) {
  _ochp_command = value;
}

/**
 *  Get ochp_timeout value.
 *
 *  @return The ochp_timeout value.
 */
unsigned int state::ochp_timeout() const noexcept {
  return _ochp_timeout;
}

/**
 *  Set ochp_timeout value.
 *
 *  @param[in] value The new ochp_timeout value.
 */
void state::ochp_timeout(unsigned int value) {
  if (!value)
    throw(engine_error() << "ochp_timeout cannot be 0");
  _ochp_timeout = value;
}

/**
 *  Get ocsp_command value.
 *
 *  @return The ocsp_command value.
 */
std::string const& state::ocsp_command() const noexcept {
  return _ocsp_command;
}

/**
 *  Set ocsp_command value.
 *
 *  @param[in] value The new ocsp_command value.
 */
void state::ocsp_command(std::string const& value) {
  _ocsp_command = value;
}

/**
 *  Get ocsp_timeout value.
 *
 *  @return The ocsp_timeout value.
 */
unsigned int state::ocsp_timeout() const noexcept {
  return _ocsp_timeout;
}

/**
 *  Set ocsp_timeout value.
 *
 *  @param[in] value The new ocsp_timeout value.
 */
void state::ocsp_timeout(unsigned int value) {
  if (!value)
    throw(engine_error() << "ocsp_timeout cannot be 0");
  _ocsp_timeout = value;
}

/**
 *  Get passive_host_checks_are_soft value.
 *
 *  @return The passive_host_checks_are_soft value.
 */
bool state::passive_host_checks_are_soft() const noexcept {
  return _passive_host_checks_are_soft;
}

/**
 *  Set passive_host_checks_are_soft value.
 *
 *  @param[in] value The new passive_host_checks_are_soft value.
 */
void state::passive_host_checks_are_soft(bool value) {
  _passive_host_checks_are_soft = value;
}

/**
 *  Get perfdata_timeout value.
 *
 *  @return The perfdata_timeout value.
 */
int state::perfdata_timeout() const noexcept {
  return _perfdata_timeout;
}

/**
 *  Set perfdata_timeout value.
 *
 *  @param[in] value The new perfdata_timeout value.
 */
void state::perfdata_timeout(int value) {
  _perfdata_timeout = value;
}

/**
 *  Get poller_name value.
 *
 *  @return The poller_name value.
 */
std::string const& state::poller_name() const noexcept {
  return _poller_name;
}

/**
 *  Set poller_name value.
 *
 *  @param[in] value The new poller_name value.
 */
void state::poller_name(std::string const& value) noexcept {
  _poller_name = value;
}

/**
 *  Get poller_id value.
 *
 *  @return The poller_id value.
 */
uint32_t state::poller_id() const noexcept {
  return _poller_id;
}

/**
 *  Set poller_id value.
 *
 *  @param[in] value The new poller_id value.
 */
void state::poller_id(uint32_t value) noexcept {
  _poller_id = value;
}

/**
 *  Get rpc_port value.
 *
 *  @return The poller_id value.
 */
uint16_t state::rpc_port() const noexcept {
  return _poller_id;
}

/**
 *  Set poller_id value.
 *
 *  @param[in] value The new poller_id value.
 */
void state::rpc_port(uint16_t value) noexcept {
  _poller_id = value;
}

/**
 *  Get process_performance_data value.
 *
 *  @return The process_performance_data value.
 */
bool state::process_performance_data() const noexcept {
  return _process_performance_data;
}

/**
 *  Set process_performance_data value.
 *
 *  @param[in] value The new process_performance_data value.
 */
void state::process_performance_data(bool value) {
  _process_performance_data = value;
}

/**
 *  Get resource_file value.
 *
 *  @return The resource_file value.
 */
std::list<std::string> const& state::resource_file() const noexcept {
  return _resource_file;
}

/**
 *  Set resource_file value.
 *
 *  @param[in] value The new resource_file value.
 */
void state::resource_file(std::list<std::string> const& value) {
  _resource_file = value;
}

/**
 *  Get retained_contact_host_attribute_mask value.
 *
 *  @return The retained_contact_host_attribute_mask value.
 */
unsigned long state::retained_contact_host_attribute_mask() const noexcept {
  return _retained_contact_host_attribute_mask;
}

/**
 *  Set retained_contact_host_attribute_mask value.
 *
 *  @param[in] value The new retained_contact_host_attribute_mask value.
 */
void state::retained_contact_host_attribute_mask(unsigned long value) {
  _retained_contact_host_attribute_mask = value;
}

/**
 *  Get retained_contact_service_attribute_mask value.
 *
 *  @return The retained_contact_service_attribute_mask value.
 */
unsigned long state::retained_contact_service_attribute_mask() const noexcept {
  return _retained_contact_service_attribute_mask;
}

/**
 *  Set retained_contact_service_attribute_mask value.
 *
 *  @param[in] value The new retained_contact_service_attribute_mask value.
 */
void state::retained_contact_service_attribute_mask(unsigned long value) {
  _retained_contact_service_attribute_mask = value;
}

/**
 *  Get retained_host_attribute_mask value.
 *
 *  @return The retained_host_attribute_mask value.
 */
unsigned long state::retained_host_attribute_mask() const noexcept {
  return _retained_host_attribute_mask;
}

/**
 *  Set retained_host_attribute_mask value.
 *
 *  @param[in] value The new retained_host_attribute_mask value.
 */
void state::retained_host_attribute_mask(unsigned long value) {
  _retained_host_attribute_mask = value;
}

/**
 *  Get retained_process_host_attribute_mask value.
 *
 *  @return The retained_process_host_attribute_mask value.
 */
unsigned long state::retained_process_host_attribute_mask() const noexcept {
  return _retained_process_host_attribute_mask;
}

/**
 *  Set retained_process_host_attribute_mask value.
 *
 *  @param[in] value The new retained_process_host_attribute_mask value.
 */
void state::retained_process_host_attribute_mask(unsigned long value) {
  _retained_process_host_attribute_mask = value;
}

/**
 *  Get retain_state_information value.
 *
 *  @return The retain_state_information value.
 */
bool state::retain_state_information() const noexcept {
  return _retain_state_information;
}

/**
 *  Set retain_state_information value.
 *
 *  @param[in] value The new retain_state_information value.
 */
void state::retain_state_information(bool value) {
  _retain_state_information = value;
}

/**
 *  Get retention_scheduling_horizon value.
 *
 *  @return The retention_scheduling_horizon value.
 */
unsigned int state::retention_scheduling_horizon() const noexcept {
  return _retention_scheduling_horizon;
}

/**
 *  Set retention_scheduling_horizon value.
 *
 *  @param[in] value The new retention_scheduling_horizon value.
 */
void state::retention_scheduling_horizon(unsigned int value) {
  if (!value)
    throw(engine_error() << "retention_scheduling_horizon cannot be 0");
  _retention_scheduling_horizon = value;
}

/**
 *  Get retention_update_interval value.
 *
 *  @return The retention_update_interval value.
 */
unsigned int state::retention_update_interval() const noexcept {
  return _retention_update_interval;
}

/**
 *  Set retention_update_interval value.
 *
 *  @param[in] value The new retention_update_interval value.
 */
void state::retention_update_interval(unsigned int value) {
  if (!value)
    throw(engine_error() << "retention_update_interval cannot be 0");
  _retention_update_interval = value;
}

/**
 *  Get all engine servicedependencies.
 *
 *  @return All engine servicedependencies.
 */
set_servicedependency const& state::servicedependencies() const noexcept {
  return _servicedependencies;
}

/**
 *  Get all engine servicedependencies.
 *
 *  @return All engine servicedependencies.
 */
set_servicedependency& state::servicedependencies() noexcept {
  return _servicedependencies;
}

/**
 *  Get all engine serviceescalations.
 *
 *  @return All engine serviceescalations.
 */
set_serviceescalation const& state::serviceescalations() const noexcept {
  return _serviceescalations;
}

/**
 *  Get all engine serviceescalations.
 *
 *  @return All engine serviceescalations.
 */
set_serviceescalation& state::serviceescalations() noexcept {
  return _serviceescalations;
}

/**
 *  Get all engine servicegroups.
 *
 *  @return All engine servicegroups.
 */
set_servicegroup const& state::servicegroups() const noexcept {
  return _servicegroups;
}

/**
 *  Get all engine servicegroups.
 *
 *  @return All engine servicegroups.
 */
set_servicegroup& state::servicegroups() noexcept {
  return _servicegroups;
}

/**
 *  Get service group by its key.
 *
 *  @param[in] k Service group name.
 *
 *  @return Iterator to the element if found, servicegroups().end()
 *          otherwise.
 */
set_servicegroup::const_iterator state::servicegroups_find(
    servicegroup::key_type const& k) const {
  configuration::servicegroup below_searched(k);
  set_servicegroup::const_iterator it(
      _servicegroups.upper_bound(below_searched));
  if ((it != _servicegroups.end()) && (it->servicegroup_name() == k))
    return it;
  else if ((it != _servicegroups.begin()) && ((--it)->servicegroup_name() == k))
    return it;
  return _servicegroups.end();
}

/**
 *  Get service group by its key.
 *
 *  @param[in] k Service group name.
 *
 *  @return Iterator to the element if found, servicegroups().end()
 *          otherwise.
 */
set_servicegroup::iterator state::servicegroups_find(
    servicegroup::key_type const& k) {
  configuration::servicegroup below_searched(k);
  set_servicegroup::iterator it(_servicegroups.upper_bound(below_searched));
  if ((it != _servicegroups.end()) && (it->servicegroup_name() == k))
    return it;
  else if ((it != _servicegroups.begin()) && ((--it)->servicegroup_name() == k))
    return it;
  return _servicegroups.end();
}
//
///**
// *  Get all engine anomalydetections.
// *
// *  @return All engine anomalydetections.
// */
// set_anomalydetection const& state::anomalydetections() const noexcept {
//  return _anomalydetections;
//}

/**
 *  Get all engine anomalydetections.
 *
 *  @return All engine anomalydetections.
 */
set_anomalydetection& state::anomalydetections() noexcept {
  return _anomalydetections;
}

/**
 *  Get all engine services.
 *
 *  @return All engine services.
 */
set_service const& state::services() const noexcept {
  return _services;
}

/**
 *  Get all engine services.
 *
 *  @return All engine services.
 */
set_service& state::services() noexcept {
  return _services;
}

/**
 *  Get anomaly detection service by its key.
 *
 *  @param[in] k anomaly detection service name.
 *
 *  @return Iterator to the element if found, anomalydetections().end()
 *          otherwise.
 */
set_anomalydetection::iterator state::anomalydetections_find(
    anomalydetection::key_type const& k) {
  configuration::anomalydetection below_searched;
  below_searched.set_host_id(k.first);
  below_searched.set_service_id(k.second);
  set_anomalydetection::const_iterator it{
      _anomalydetections.upper_bound(below_searched)};
  if (it != _anomalydetections.end() && it->host_id() == k.first &&
      it->service_id() == k.second)
    return it;
  else if (it != _anomalydetections.begin() && (--it)->host_id() == k.first &&
           it->service_id() == k.second)
    return it;
  return _anomalydetections.end();
}
/**
 *  Get service by its key.
 *
 *  @param[in] k Service name.
 *
 *  @return Iterator to the element if found, services().end()
 *          otherwise.
 */
set_service::iterator state::services_find(service::key_type const& k) {
  configuration::service below_searched;
  below_searched.set_host_id(k.first);
  below_searched.set_service_id(k.second);
  set_service::const_iterator it{_services.upper_bound(below_searched)};
  if (it != _services.end() && it->host_id() == k.first &&
      it->service_id() == k.second)
    return it;
  else if (it != _services.begin() && (--it)->host_id() == k.first &&
           it->service_id() == k.second)
    return it;
  return _services.end();
}

set_service::const_iterator state::services_find(
    std::string const& host_name,
    std::string const& service_desc) const {
  for (auto it = _services.begin(), end = _services.end(); it != end; ++it) {
    if (it->service_description() == service_desc &&
        *it->hosts().begin() == host_name)
      return it;
  }
  return _services.end();
}

/**
 *  Get service_check_timeout value.
 *
 *  @return The service_check_timeout value.
 */
unsigned int state::service_check_timeout() const noexcept {
  return _service_check_timeout;
}

/**
 *  Set service_check_timeout value.
 *
 *  @param[in] value The new service_check_timeout value.
 */
void state::service_check_timeout(unsigned int value) {
  if (!value)
    throw(engine_error() << "service_check_timeout cannot be 0");
  _service_check_timeout = value;
}

/**
 *  Get service_freshness_check_interval value.
 *
 *  @return The service_freshness_check_interval value.
 */
unsigned int state::service_freshness_check_interval() const noexcept {
  return _service_freshness_check_interval;
}

/**
 *  Set service_freshness_check_interval value.
 *
 *  @param[in] value The new service_freshness_check_interval value.
 */
void state::service_freshness_check_interval(unsigned int value) {
  if (!value)
    throw(engine_error() << "service_freshness_check_interval cannot be 0");
  _service_freshness_check_interval = value;
}

/**
 *  Get service_inter_check_delay_method value.
 *
 *  @return The service_inter_check_delay_method value.
 */
state::inter_check_delay state::service_inter_check_delay_method() const
    noexcept {
  return _service_inter_check_delay_method;
}

/**
 *  Set service_inter_check_delay_method value.
 *
 *  @param[in] value The new service_inter_check_delay_method value.
 */
void state::service_inter_check_delay_method(inter_check_delay value) {
  _service_inter_check_delay_method = value;
}

/**
 *  Get service_interleave_factor_method value.
 *
 *  @return The service_interleave_factor_method value.
 */
state::interleave_factor state::service_interleave_factor_method() const
    noexcept {
  return _service_interleave_factor_method;
}

/**
 *  Set service_interleave_factor_method value.
 *
 *  @param[in] value The new service_interleave_factor_method value.
 */
void state::service_interleave_factor_method(interleave_factor value) {
  _service_interleave_factor_method = value;
}

/**
 *  Get service_perfdata_command value.
 *
 *  @return The service_perfdata_command value.
 */
std::string const& state::service_perfdata_command() const noexcept {
  return _service_perfdata_command;
}

/**
 *  Set service_perfdata_command value.
 *
 *  @param[in] value The new service_perfdata_command value.
 */
void state::service_perfdata_command(std::string const& value) {
  _service_perfdata_command = value;
}

/**
 *  Get service_perfdata_file value.
 *
 *  @return The service_perfdata_file value.
 */
std::string const& state::service_perfdata_file() const noexcept {
  return _service_perfdata_file;
}

/**
 *  Set service_perfdata_file value.
 *
 *  @param[in] value The new service_perfdata_file value.
 */
void state::service_perfdata_file(std::string const& value) {
  _service_perfdata_file = value;
}

/**
 *  Get service_perfdata_file_mode value.
 *
 *  @return The service_perfdata_file_mode value.
 */
state::perfdata_file_mode state::service_perfdata_file_mode() const noexcept {
  return _service_perfdata_file_mode;
}

/**
 *  Set service_perfdata_file_mode value.
 *
 *  @param[in] value The new service_perfdata_file_mode value.
 */
void state::service_perfdata_file_mode(perfdata_file_mode value) {
  _service_perfdata_file_mode = value;
}

/**
 *  Get service_perfdata_file_processing_command value.
 *
 *  @return The service_perfdata_file_processing_command value.
 */
std::string const& state::service_perfdata_file_processing_command() const
    noexcept {
  return _service_perfdata_file_processing_command;
}

/**
 *  Set service_perfdata_file_processing_command value.
 *
 *  @param[in] value The new service_perfdata_file_processing_command value.
 */
void state::service_perfdata_file_processing_command(std::string const& value) {
  _service_perfdata_file_processing_command = value;
}

/**
 *  Get service_perfdata_file_processing_interval value.
 *
 *  @return The service_perfdata_file_processing_interval value.
 */
unsigned int state::service_perfdata_file_processing_interval() const noexcept {
  return _service_perfdata_file_processing_interval;
}

/**
 *  Set service_perfdata_file_processing_interval value.
 *
 *  @param[in] value The new service_perfdata_file_processing_interval value.
 */
void state::service_perfdata_file_processing_interval(unsigned int value) {
  _service_perfdata_file_processing_interval = value;
}

/**
 *  Get service_perfdata_file_template value.
 *
 *  @return The service_perfdata_file_template value.
 */
std::string const& state::service_perfdata_file_template() const noexcept {
  return _service_perfdata_file_template;
}

/**
 *  Set service_perfdata_file_template value.
 *
 *  @param[in] value The new service_perfdata_file_template value.
 */
void state::service_perfdata_file_template(std::string const& value) {
  _service_perfdata_file_template = value;
}

/**
 *  Get sleep_time value.
 *
 *  @return The sleep_time value.
 */
float state::sleep_time() const noexcept {
  return _sleep_time;
}

/**
 *  Set sleep_time value.
 *
 *  @param[in] value The new sleep_time value.
 */
void state::sleep_time(float value) {
  if (value <= 0.0)
    throw(engine_error() << "sleep_time cannot be less or equal to 0 (" << value
                         << " provided)");
  _sleep_time = value;
}

/**
 *  Get soft_state_dependencies value.
 *
 *  @return The soft_state_dependencies value.
 */
bool state::soft_state_dependencies() const noexcept {
  return _soft_state_dependencies;
}

/**
 *  Set soft_state_dependencies value.
 *
 *  @param[in] value The new soft_state_dependencies value.
 */
void state::soft_state_dependencies(bool value) {
  _soft_state_dependencies = value;
}

/**
 *  Get state_retention_file value.
 *
 *  @return The state_retention_file value.
 */
std::string const& state::state_retention_file() const noexcept {
  return _state_retention_file;
}

/**
 *  Set state_retention_file value.
 *
 *  @param[in] value The new state_retention_file value.
 */
void state::state_retention_file(std::string const& value) {
  if (value.empty() || value[0] == '/')
    _state_retention_file = value;
  else {
    io::file_entry fe(_cfg_main);
    std::string base_name(fe.directory_name());
    _state_retention_file = base_name + "/" + value;
  }
}

/**
 *  Get status_file value.
 *
 *  @return The status_file value.
 */
std::string const& state::status_file() const noexcept {
  return _status_file;
}

/**
 *  Set status_file value.
 *
 *  @param[in] value The new status_file value.
 */
void state::status_file(std::string const& value) {
  _status_file = value;
}

/**
 *  Get status_update_interval value.
 *
 *  @return The status_update_interval value.
 */
unsigned int state::status_update_interval() const noexcept {
  return _status_update_interval;
}

/**
 *  Set status_update_interval value.
 *
 *  @param[in] value The new status_update_interval value.
 */
void state::status_update_interval(unsigned int value) {
  if (value < 2)
    throw(engine_error() << "status_update_interval cannot be less than 2 ("
                         << value << " provided)");
  _status_update_interval = value;
}

/**
 *  Set a property with new value.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool state::set(char const* key, char const* value) {
  try {
    std::unordered_map<std::string, state::setter_func>::const_iterator it{
        _setters.find(key)};
    if (it != _setters.end())
      return (it->second)(*this, value);
  } catch (std::exception const& e) {
    logger(log_config_error, basic) << e.what();
    return false;
  }
  return true;
}

/**
 *  Get all engine timeperiods.
 *
 *  @return All engine timeperiods.
 */
set_timeperiod const& state::timeperiods() const noexcept {
  return _timeperiods;
}

/**
 *  Get all engine timeperiods.
 *
 *  @return All engine timeperiods.
 */
set_timeperiod& state::timeperiods() noexcept {
  return _timeperiods;
}

/**
 *  Find a time period by its key.
 *
 *  @param[in] k Time period name.
 *
 *  @return Iterator to the element if found, timeperiods().end()
 *          otherwise.
 */
set_timeperiod::const_iterator state::timeperiods_find(
    timeperiod::key_type const& k) const {
  configuration::timeperiod below_searched(k);
  set_timeperiod::const_iterator it(_timeperiods.upper_bound(below_searched));
  if ((it != _timeperiods.end()) && (it->timeperiod_name() == k))
    return it;
  else if ((it != _timeperiods.begin()) && ((--it)->timeperiod_name() == k))
    return it;
  return _timeperiods.end();
}

/**
 *  Find a time period by its key.
 *
 *  @param[in] k Time period name.
 *
 *  @return Iterator to the element if found, timeperiods().end()
 *          otherwise.
 */
set_timeperiod::iterator state::timeperiods_find(
    timeperiod::key_type const& k) {
  configuration::timeperiod below_searched(k);
  set_timeperiod::iterator it(_timeperiods.upper_bound(below_searched));
  if ((it != _timeperiods.end()) && (it->timeperiod_name() == k))
    return it;
  else if ((it != _timeperiods.begin()) && ((--it)->timeperiod_name() == k))
    return it;
  return _timeperiods.end();
}

/**
 *  Get time_change_threshold value.
 *
 *  @return The time_change_threshold value.
 */
unsigned int state::time_change_threshold() const noexcept {
  return _time_change_threshold;
}

/**
 *  Set time_change_threshold value.
 *
 *  @param[in] value The new time_change_threshold value.
 */
void state::time_change_threshold(unsigned int value) {
  if (value < 6)
    throw(engine_error() << "time_change_threshold cannot be less than 6 ("
                         << value << " provided)");
  _time_change_threshold = value;
}

/**
 *  Get translate_passive_host_checks value.
 *
 *  @return The translate_passive_host_checks value.
 */
bool state::translate_passive_host_checks() const noexcept {
  return _translate_passive_host_checks;
}

/**
 *  Set translate_passive_host_checks value.
 *
 *  @param[in] value The new translate_passive_host_checks value.
 */
void state::translate_passive_host_checks(bool value) {
  _translate_passive_host_checks = value;
}

/**
 *  Get user resources.
 *
 *  @return The users resources list.
 */
std::unordered_map<std::string, std::string> const& state::user() const
    noexcept {
  return _users;
}

/**
 *  Set the user resources.
 *
 *  @param[in] value The new users list.
 */
void state::user(std::unordered_map<std::string, std::string> const& value) {
  _users = value;
}

/**
 *  Set the user resources.
 *
 *  @param[in] key   The user key.
 *  @param[in] value The user value.
 */
void state::user(std::string const& key, std::string const& value) {
  if (key.size() < 3 || key[0] != '$' || key[key.size() - 1] != '$')
    throw(engine_error() << "Invalid user key '" << key << "'");
  std::string new_key = key;
  new_key.erase(new_key.begin(), new_key.begin() + 1);
  new_key.erase(new_key.end() - 1, new_key.end());
  _users[new_key] = value;
}

/**
 *  Set the user resources.
 *
 *  @param[in] key   The user key.
 *  @param[in] value The user value.
 */
void state::user(unsigned int key, std::string const& value) {
  _users[string::from(key)] = value;
}

/**
 *  Get use_aggressive_host_checking value.
 *
 *  @return The use_aggressive_host_checking value.
 */
bool state::use_aggressive_host_checking() const noexcept {
  return _use_aggressive_host_checking;
}

/**
 *  Set use_aggressive_host_checking value.
 *
 *  @param[in] value The new use_aggressive_host_checking value.
 */
void state::use_aggressive_host_checking(bool value) {
  _use_aggressive_host_checking = value;
}

/**
 *  Get use_large_installation_tweaks value.
 *
 *  @return The use_large_installation_tweaks value.
 */
bool state::use_large_installation_tweaks() const noexcept {
  return _use_large_installation_tweaks;
}

/**
 * @brief Get instance_heartbeat_interval value. This is the minimal delay in
 * seconds between two instance status sent to broker.
 *
 * @return this value in seconds.
 */
uint32_t state::instance_heartbeat_interval() const noexcept {
  return _instance_heartbeat_interval;
}

/**
 *  Set use_large_installation_tweaks value.
 *
 *  @param[in] value The new use_large_installation_tweaks value.
 */
void state::use_large_installation_tweaks(bool value) {
  _use_large_installation_tweaks = value;
}

/**
 *  Set instance_heartbeat_interval value.
 *
 *  @param[in] value The new instance_heartbeat_interval value.
 */
void state::instance_heartbeat_interval(uint32_t value) {
  _instance_heartbeat_interval = value;
}

/**
 *  Get use_regexp_matches value.
 *
 *  @return The use_regexp_matches value.
 */
bool state::use_regexp_matches() const noexcept {
  return _use_regexp_matches;
}

/**
 *  Set use_regexp_matches value.
 *
 *  @param[in] value The new use_regexp_matches value.
 */
void state::use_regexp_matches(bool value) {
  _use_regexp_matches = value;
}

/**
 *  Get use_retained_program_state value.
 *
 *  @return The use_retained_program_state value.
 */
bool state::use_retained_program_state() const noexcept {
  return _use_retained_program_state;
}

/**
 *  Set use_retained_program_state value.
 *
 *  @param[in] value The new use_retained_program_state value.
 */
void state::use_retained_program_state(bool value) {
  _use_retained_program_state = value;
}

/**
 *  Get use_retained_scheduling_info value.
 *
 *  @return The use_retained_scheduling_info value.
 */
bool state::use_retained_scheduling_info() const noexcept {
  return _use_retained_scheduling_info;
}

/**
 *  Set use_retained_scheduling_info value.
 *
 *  @param[in] value The new use_retained_scheduling_info value.
 */
void state::use_retained_scheduling_info(bool value) {
  _use_retained_scheduling_info = value;
}

/**
 *  Get use_setpgid value.
 *
 *  @return The use_setpgid value.
 */
bool state::use_setpgid() const noexcept {
  return _use_setpgid;
}

/**
 *  Set use_setpgid value.
 *
 *  @param[in] value The new use_setpgid value.
 */
void state::use_setpgid(bool value) {
  _use_setpgid = value;
}

/**
 *  Get use_syslog value.
 *
 *  @return The use_syslog value.
 */
bool state::use_syslog() const noexcept {
  return _use_syslog;
}

/**
 *  Set use_syslog value.
 *
 *  @param[in] value The new use_syslog value.
 */
void state::use_syslog(bool value) {
  _use_syslog = value;
}

/**
 *  Get use_timezone value.
 *
 *  @return The use_timezone value.
 */
std::string const& state::use_timezone() const noexcept {
  return _use_timezone;
}

/**
 *  Set use_timezone value.
 *
 *  @param[in] value The new use_timezone value.
 */
void state::use_timezone(std::string const& value) {
  _use_timezone = value;
}

/**
 *  Get use_true_regexp_matching value.
 *
 *  @return The use_true_regexp_matching value.
 */
bool state::use_true_regexp_matching() const noexcept {
  return _use_true_regexp_matching;
}

/**
 *  Set use_true_regexp_matching value.
 *
 *  @param[in] value The new use_true_regexp_matching value.
 */
void state::use_true_regexp_matching(bool value) {
  _use_true_regexp_matching = value;
}

/**
 *  Unused variable aggregate_status_updates.
 *
 *  @param[in] value Unused.
 */
void state::_set_aggregate_status_updates(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: aggregate_status_updates variable ignored";
  ++config_warnings;
}

/**
 *  Unused variable auth_file.
 *
 *  @param[in] value Unused.
 */
void state::_set_auth_file(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic) << "Warning: auth_file variable ignored";
  ++config_warnings;
}

/**
 *  Unused variable bare_update_check.
 *
 *  @param[in] value Unused.
 */
void state::_set_bare_update_check(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: bare_update_check variable ignored";
  ++config_warnings;
}

/**
 *  Add broker module.
 *
 *  @param[in] value The new broker module.
 */
void state::_set_broker_module(std::string const& value) {
  _broker_module.push_back(value);
}

/**
 *  Add configuration directory.
 *
 *  @param[in] value The new configuration directory.
 */
void state::_set_cfg_dir(std::string const& value) {
  if (value.empty() || value[0] == '/')
    _cfg_dir.push_back(value);
  else {
    io::file_entry fe(_cfg_main);
    std::string base_name(fe.directory_name());
    _cfg_dir.push_back(base_name + "/" + value);
  }
}

/**
 *  Add configuration file.
 *
 *  @param[in] value The new configuration file.
 */
void state::_set_cfg_file(std::string const& value) {
  if (value.empty() || value[0] == '/')
    _cfg_file.push_back(value);
  else {
    io::file_entry fe(_cfg_main);
    std::string base_name(fe.directory_name());
    _cfg_file.push_back(base_name + "/" + value);
  }
}

/**
 *  Unused variable check_for_updates.
 *
 *  @param[in] value Unused.
 */
void state::_set_check_for_updates(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: check_for_updates variable ignored";
  ++config_warnings;
}

/**
 *  Unused variable child_processes_fork_twice.
 *
 *  @param[in] value Unused.
 */
void state::_set_child_processes_fork_twice(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: child_processes_fork_twice variable ignored";
  ++config_warnings;
}

/**
 *  Set command check interval.
 *
 *  @param[in] value The new command check interval.
 */
void state::_set_command_check_interval(std::string const& value) {
  std::string val(value);
  size_t pos(val.find('s'));
  if (pos == std::string::npos)
    _command_check_interval_is_seconds = false;
  else if (pos == val.size() - 1) {
    _command_check_interval_is_seconds = true;
    val.erase(val.begin() + pos);
  }
  setter<int, &state::command_check_interval>::generic(*this, val.c_str());
}

/**
 *  Unused variable comment_file.
 *
 *  @param[in] value Unused.
 */
void state::_set_comment_file(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic) << "Warning: comment_file variable ignored";
  ++config_warnings;
}

/**
 *  Unused variable daemon_dumps_core.
 *
 *  @param[in] value Unused.
 */
void state::_set_daemon_dumps_core(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: daemon_dumps_core variable ignored";
  ++config_warnings;
}

/**
 *  Set date format.
 *
 *  @param[in] value The new date format.
 */
void state::_set_date_format(std::string const& value) {
  if (value == "euro")
    _date_format = euro;
  else if (value == "iso8601")
    _date_format = iso8601;
  else if (value == "strict-iso8601")
    _date_format = strict_iso8601;
  else
    _date_format = us;
}

/**
 *  Unused variable downtime_file.
 *
 *  @param[in] value Unused.
 */
void state::_set_downtime_file(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: downtime_file variable ignored";
  ++config_warnings;
}

/**
 *  Unused variable enable_embedded_perl.
 *
 *  @param[in] value Unused.
 */
void state::_set_enable_embedded_perl(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: enable_embedded_perl variable ignored";
  ++config_warnings;
}

/**
 *  Unused variable enable_failure_prediction.
 *
 *  @param[in] value  Unused.
 */
void state::_set_enable_failure_prediction(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: enable_failure_prediction variable ignored";
  ++config_warnings;
  return;
}

/**
 *  Set event_broker_options.
 *
 *  @param[in] value The new event_broker_options value.
 */
void state::_set_event_broker_options(std::string const& value) {
  if (value != "-1")
    setter<unsigned long, &state::event_broker_options>::generic(*this,
                                                                 value.c_str());
  else {
    _event_broker_options = BROKER_EVERYTHING;
  }
}

/**
 *  Unused variable free_child_process_memory.
 *
 *  @param[in] value Unused.
 */
void state::_set_free_child_process_memory(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: free_child_process_memory variable ignored";
  ++config_warnings;
}

/**
 *  Set host_inter_check_delay_method.
 *
 *  @param[in] value The new host_inter_check_delay_method value.
 */
void state::_set_host_inter_check_delay_method(std::string const& value) {
  if (value == "n")
    _host_inter_check_delay_method = icd_none;
  else if (value == "d")
    _host_inter_check_delay_method = icd_dumb;
  else if (value == "s")
    _host_inter_check_delay_method = icd_smart;
  else {
    _host_inter_check_delay_method = icd_user;
    if (!string::to(value.c_str(), scheduling_info.host_inter_check_delay) ||
        scheduling_info.host_inter_check_delay <= 0.0)
      throw(engine_error()
            << "Invalid value for host_inter_check_delay_method, must "
            << "be one of 'n' (none), 'd' (dumb), 's' (smart) or a "
            << "stricly positive value (" << value << " provided)");
  }
}

/**
 *  Set host_perfdata_file_mode.
 *
 *  @param[in] value The new host_inter_check_delay_method value.
 */
void state::_set_host_perfdata_file_mode(std::string const& value) {
  if (value == "p")
    _host_perfdata_file_mode = mode_pipe;
  else if (value == "w")
    _host_perfdata_file_mode = mode_file;
  else
    _host_perfdata_file_mode = mode_file_append;
}

/**
 *  Unused variable lock_file.
 *
 *  @param[in] value Unused.
 */
void state::_set_lock_file(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic) << "Warning: lock_file variable ignored";
  ++config_warnings;
}

/**
 *  Unused variable log_archive_path.
 *
 *  @param[in] value Unused.
 */
void state::_set_log_archive_path(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: log_archive_path variable ignored";
  ++config_warnings;
}

/**
 *  Unused variable log_initial_states.
 *
 *  @param[in] value  Unused.
 */
void state::_set_log_initial_states(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: log_initial_states variable ignored";
  ++config_warnings;
  return;
}

/**
 *  Unused variable log_rotation_method.
 *
 *  @param[in] value Unused.
 */
void state::_set_log_rotation_method(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: log_rotation_method variable ignored";
  ++config_warnings;
}

/**
 *  Unused variable nagios_group.
 *
 *  @param[in] value Unused.
 */
void state::_set_nagios_group(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic) << "Warning: nagios_group variable ignored";
  ++config_warnings;
}

/**
 *  Unused variable nagios_user.
 *
 *  @param[in] value Unused.
 */
void state::_set_nagios_user(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic) << "Warning: nagios_user variable ignored";
  ++config_warnings;
}

/**
 *  Set object_cache_file value.
 *
 *  @param[in] value Unused.
 */
void state::_set_object_cache_file(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: object_cache_file variable ignored";
  ++config_warnings;
}

/**
 *  Unused variable p1_file.
 *
 *  @param[in] value Unused.
 */
void state::_set_p1_file(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic) << "Warning: p1_file variable ignored";
  ++config_warnings;
}

/**
 *  Set precached_object_file value.
 *
 *  @param[in] value Unused.
 */
void state::_set_precached_object_file(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: precached_object_file variable ignored";
  ++config_warnings;
}

/**
 *  Set resource_file.
 *
 *  @param[in] value The new resource_file.
 */
void state::_set_resource_file(std::string const& value) {
  if (value.empty() || value[0] == '/')
    _resource_file.push_back(value);
  else {
    io::file_entry fe(_cfg_main);
    std::string base_name(fe.directory_name());
    _resource_file.push_back(base_name + "/" + value);
  }
}

/**
 *  Unused variable retained_process_service_attribute_mask.
 *
 *  @param[in] value Unused.
 */
void state::_set_retained_process_service_attribute_mask(
    std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: retained_process_service_attribute_mask variable ignored";
  ++config_warnings;
}

/**
 *  Unused variable retained_service_attribute_mask.
 *
 *  @param[in] value Unused.
 */
void state::_set_retained_service_attribute_mask(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: retained_service_attribute_mask variable ignored";
  ++config_warnings;
}

/**
 *  Set service_inter_check_delay_method
 *
 *  @param[in] value The new service_inter_check_delay_method.
 */
void state::_set_service_inter_check_delay_method(std::string const& value) {
  if (value == "n")
    _service_inter_check_delay_method = icd_none;
  else if (value == "d")
    _service_inter_check_delay_method = icd_dumb;
  else if (value == "s")
    _service_inter_check_delay_method = icd_smart;
  else {
    _service_inter_check_delay_method = icd_user;
    if (!string::to(value.c_str(), scheduling_info.service_inter_check_delay) ||
        scheduling_info.service_inter_check_delay <= 0.0)
      throw(engine_error()
            << "Invalid value for service_inter_check_delay_method, "
            << "must be one of 'n' (none), 'd' (dumb), 's' (smart) or "
            << "a strictly positive value (" << value << " provided)");
  }
}

/**
 *  Set service_interleave_factor_method
 *
 *  @param[in] value The new service_interleave_factor_method.
 */
void state::_set_service_interleave_factor_method(std::string const& value) {
  if (value == "s")
    _service_interleave_factor_method = ilf_smart;
  else {
    _service_interleave_factor_method = ilf_user;
    if (!string::to(value.c_str(), scheduling_info.service_interleave_factor) ||
        scheduling_info.service_interleave_factor < 1)
      scheduling_info.service_interleave_factor = 1;
  }
}

/**
 *  Set service_perfdata_file_mode.
 *
 *  @param[in] value The new service_inter_check_delay_method value.
 */
void state::_set_service_perfdata_file_mode(std::string const& value) {
  if (value == "p")
    _service_perfdata_file_mode = mode_pipe;
  else if (value == "w")
    _service_perfdata_file_mode = mode_file;
  else
    _service_perfdata_file_mode = mode_file_append;
}

/**
 *  Unused variable temp_file.
 *
 *  @param[in] value Unused.
 */
void state::_set_temp_file(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic) << "Warning: temp_file variable ignored";
  ++config_warnings;
}

/**
 *  Unused variable temp_path.
 *
 *  @param[in] value Unused.
 */
void state::_set_temp_path(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic) << "Warning: temp_path variable ignored";
  ++config_warnings;
}

/**
 *  Unused variable use_embedded_perl_implicitly.
 *
 *  @param[in] value Unused.
 */
void state::_set_use_embedded_perl_implicitly(std::string const& value) {
  (void)value;
  logger(log_config_warning, basic)
      << "Warning: use_embedded_perl_implicitly variable ignored";
  ++config_warnings;
}

void state::macros_filter(std::string const& value) {
  size_t previous(0), first, last;
  size_t current(value.find(','));
  while (current != std::string::npos) {
    std::string item(value.substr(previous, current - previous));
    first = item.find_first_not_of(' ');
    last = item.find_last_not_of(' ');
    _macros_filter.insert(item.substr(first, last - first + 1));
    previous = current + 1;
    current = value.find(',', previous);
  }
  std::string item(value.substr(previous, current - previous));
  first = item.find_first_not_of(' ');
  last = item.find_last_not_of(' ');
  _macros_filter.insert(item.substr(first, last - first + 1));
}

std::set<std::string> const& state::macros_filter() const {
  return _macros_filter;
}

/**
 *  Get enable_macros_filter value.
 *
 *  @return The enable_macros_filter value.
 */
bool state::enable_macros_filter() const noexcept {
  return _enable_macros_filter;
}

/**
 *  Set enable_macros_filter value.
 *
 *  @param[in] value The new enable_macros_filter value.
 */
void state::enable_macros_filter(bool value) {
  _enable_macros_filter = value;
}

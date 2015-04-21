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

#include <limits>
#include "compatibility/locations.h"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/deprecated.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/io/file_entry.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::logging;

#define SETTER(type, method) \
  &state::setter<type, &state::method>::generic

state::setters const state::_setters[] = {
  { "additional_freshness_latency",                SETTER(duration const&, additional_freshness_latency) },
  { "broker_module_directory",                     SETTER(std::string const&, broker_module_directory) },
  { "broker_module",                               SETTER(std::string const&, _set_broker_module) },
  { "cached_host_check_horizon",                   SETTER(duration const&, cached_host_check_horizon) },
  { "cached_service_check_horizon",                SETTER(duration const&, cached_service_check_horizon) },
  { "cfg_dir",                                     SETTER(std::string const&, _set_cfg_dir) },
  { "cfg_file",                                    SETTER(std::string const&, _set_cfg_file) },
  { "check_host_freshness",                        SETTER(bool, check_host_freshness) },
  { "check_result_reaper_frequency",               SETTER(duration const&, check_reaper_interval) },
  { "check_service_freshness",                     SETTER(bool, check_service_freshness) },
  { "command_check_interval",                      SETTER(duration const&, command_check_interval) },
  { "command_file",                                SETTER(std::string const&, command_file) },
  { "debug_file",                                  SETTER(std::string const&, debug_file) },
  { "debug_level",                                 SETTER(unsigned long long, debug_level) },
  { "debug_verbosity",                             SETTER(unsigned int, debug_verbosity) },
  { "enable_event_handlers",                       SETTER(bool, enable_event_handlers) },
  { "enable_flap_detection",                       SETTER(bool, enable_flap_detection) },
  { "enable_predictive_host_dependency_checks",    SETTER(bool, enable_predictive_host_dependency_checks) },
  { "enable_predictive_service_dependency_checks", SETTER(bool, enable_predictive_service_dependency_checks) },
  { "event_broker_options",                        SETTER(std::string const&, _set_event_broker_options) },
  { "event_handler_timeout",                       SETTER(duration const&, event_handler_timeout) },
  { "external_command_buffer_slots",               SETTER(int, external_command_buffer_slots) },
  { "global_host_event_handler",                   SETTER(std::string const&, global_host_event_handler) },
  { "global_service_event_handler",                SETTER(std::string const&, global_service_event_handler) },
  { "high_host_flap_threshold",                    SETTER(float, high_host_flap_threshold) },
  { "high_service_flap_threshold",                 SETTER(float, high_service_flap_threshold) },
  { "host_check_timeout",                          SETTER(duration const&, host_check_timeout) },
  { "host_freshness_check_interval",               SETTER(duration const&, host_freshness_check_interval) },
  { "illegal_macro_output_chars",                  SETTER(std::string const&, illegal_output_chars) },
  { "illegal_object_name_chars",                   SETTER(std::string const&, illegal_object_chars) },
  { "log_event_handlers",                          SETTER(bool, log_event_handlers) },
  { "log_external_commands",                       SETTER(bool, log_external_commands) },
  { "log_file",                                    SETTER(std::string const&, log_file) },
  { "log_host_retries",                            SETTER(bool, log_host_retries) },
  { "log_initial_states",                          SETTER(bool, log_initial_states) },
  { "log_passive_checks",                          SETTER(bool, log_passive_checks) },
  { "log_pid",                                     SETTER(bool, log_pid) },
  { "log_service_retries",                         SETTER(bool, log_service_retries) },
  { "low_host_flap_threshold",                     SETTER(float, low_host_flap_threshold) },
  { "low_service_flap_threshold",                  SETTER(float, low_service_flap_threshold) },
  { "max_concurrent_checks",                       SETTER(unsigned int, max_parallel_service_checks) },
  { "max_debug_file_size",                         SETTER(unsigned long, max_debug_file_size) },
  { "max_log_file_size",                           SETTER(unsigned long, max_log_file_size) },
  { "obsess_over_hosts",                           SETTER(bool, obsess_over_hosts) },
  { "obsess_over_services",                        SETTER(bool, obsess_over_services) },
  { "ochp_command",                                SETTER(std::string const&, ochp_command) },
  { "ochp_timeout",                                SETTER(duration const&, ochp_timeout) },
  { "ocsp_command",                                SETTER(std::string const&, ocsp_command) },
  { "ocsp_timeout",                                SETTER(duration const&, ocsp_timeout) },
  { "resource_file",                               SETTER(std::string const&, _set_resource_file) },
  { "retention_update_interval",                   SETTER(duration const&, retention_update_interval) },
  { "service_check_timeout",                       SETTER(duration const&, service_check_timeout) },
  { "service_freshness_check_interval",            SETTER(duration const&, service_freshness_check_interval) },
  { "service_reaper_frequency",                    SETTER(duration const&, check_reaper_interval) },
  { "sleep_time",                                  SETTER(float, sleep_time) },
  { "soft_state_dependencies",                     SETTER(bool, soft_state_dependencies) },
  { "state_retention_file",                        SETTER(std::string const&, state_retention_file) },
  { "status_file",                                 SETTER(std::string const&, status_file) },
  { "time_change_threshold",                       SETTER(duration const&, time_change_threshold) },
  { "timezone",                                    SETTER(std::string const&, use_timezone) },
  { "use_setpgid",                                 SETTER(bool, use_setpgid) },
  { "use_syslog",                                  SETTER(bool, use_syslog) },
  { "use_timezone",                                SETTER(std::string const&, use_timezone) }
};

char const* const state::_deprecated[][2] = {
  { "accept_passive_host_checks",                  deprecated::check_acceptance_msg },
  { "accept_passive_service_checks",               deprecated::check_acceptance_msg },
  { "admin_email",                                 deprecated::notification_msg },
  { "admin_pager",                                 deprecated::notification_msg },
  { "aggregate_status_updates",                    deprecated::status_file_usage_msg },
  { "allow_empty_hostgroup_assignment",            deprecated::groups_msg },
  { "auth_file",                                   deprecated::startup_script_msg },
  { "auto_reschedule_checks",                      deprecated::auto_reschedule_msg },
  { "auto_rescheduling_interval",                  deprecated::auto_reschedule_msg },
  { "auto_rescheduling_window",                    deprecated::auto_reschedule_msg },
  { "bare_update_check",                           deprecated::default_msg },
  { "check_external_commands",                     deprecated::external_command_msg },
  { "check_for_orphaned_hosts",                    deprecated::default_msg },
  { "check_for_orphaned_services",                 deprecated::default_msg },
  { "check_for_updates",                           deprecated::default_msg },
  { "check_result_path",                           deprecated::check_result_path_msg },
  { "child_processes_fork_twice",                  deprecated::engine_performance_msg },
  { "comment_file",                                deprecated::status_file_usage_msg },
  { "daemon_dumps_core",                           deprecated::startup_script_msg },
  { "date_format",                                 deprecated::default_msg },
  { "downtime_file",                               deprecated::status_file_usage_msg },
  { "enable_embedded_perl",                        deprecated::embedded_perl_msg },
  { "enable_environment_macros",                   deprecated::default_msg },
  { "enable_failure_prediction",                   deprecated::default_msg },
  { "enable_notifications",                        deprecated::notification_msg },
  { "execute_host_checks",                         deprecated::check_acceptance_msg },
  { "execute_service_checks",                      deprecated::check_acceptance_msg },
  { "free_child_process_memory",                   deprecated::engine_performance_msg },
  { "host_inter_check_delay_method",               deprecated::auto_reschedule_msg },
  { "host_perfdata_command",                       deprecated::perfdata_msg },
  { "host_perfdata_file",                          deprecated::perfdata_msg },
  { "host_perfdata_file_mode",                     deprecated::perfdata_msg },
  { "host_perfdata_file_processing_command",       deprecated::perfdata_msg },
  { "host_perfdata_file_processing_interval",      deprecated::perfdata_msg },
  { "host_perfdata_file_template",                 deprecated::perfdata_msg },
  { "interval_length",                             deprecated::interval_length_msg },
  { "lock_file",                                   deprecated::startup_script_msg },
  { "log_archive_path",                            deprecated::log_msg },
  { "log_notifications",                           deprecated::notification_msg },
  { "log_rotation_method",                         deprecated::log_msg },
  { "max_check_result_file_age",                   deprecated::check_result_path_msg },
  { "max_check_result_reaper_time",                deprecated::check_result_path_msg },
  { "max_host_check_spread",                       deprecated::auto_reschedule_msg },
  { "max_service_check_spread",                    deprecated::auto_reschedule_msg },
  { "nagios_group",                                deprecated::startup_script_msg },
  { "nagios_user",                                 deprecated::startup_script_msg },
  { "notification_timeout",                        deprecated::notification_msg },
  { "object_cache_file",                           deprecated::engine_performance_msg },
  { "p1_file",                                     deprecated::embedded_perl_msg },
  { "passive_host_checks_are_soft",                deprecated::passive_host_checks_msg },
  { "perfdata_timeout",                            deprecated::perfdata_msg },
  { "precached_object_file",                       deprecated::engine_performance_msg },
  { "process_performance_data",                    deprecated::perfdata_msg },
  { "retained_contact_host_attribute_mask",        deprecated::retention_usage_msg },
  { "retained_contact_service_attribute_mask",     deprecated::retention_usage_msg },
  { "retained_host_attribute_mask",                deprecated::retention_usage_msg },
  { "retained_process_host_attribute_mask",        deprecated::retention_usage_msg },
  { "retained_process_service_attribute_mask",     deprecated::retention_usage_msg },
  { "retained_service_attribute_mask",             deprecated::retention_usage_msg },
  { "retain_state_information",                    deprecated::retention_usage_msg },
  { "retention_scheduling_horizon",                deprecated::auto_reschedule_msg },
  { "service_inter_check_delay_method",            deprecated::auto_reschedule_msg },
  { "service_interleave_factor",                   deprecated::auto_reschedule_msg },
  { "service_perfdata_command",                    deprecated::perfdata_msg },
  { "service_perfdata_file",                       deprecated::perfdata_msg },
  { "service_perfdata_file_mode",                  deprecated::perfdata_msg },
  { "service_perfdata_file_processing_command",    deprecated::perfdata_msg },
  { "service_perfdata_file_processing_interval",   deprecated::perfdata_msg },
  { "service_perfdata_file_template",              deprecated::perfdata_msg },
  { "status_update_interval",                      deprecated::status_file_usage_msg },
  { "temp_file",                                   deprecated::default_msg },
  { "temp_path",                                   deprecated::default_msg },
  { "translate_passive_host_checks",               deprecated::default_msg },
  { "use_aggressive_host_checking",                deprecated::default_msg },
  { "use_agressive_host_checking",                 deprecated::default_msg },
  { "use_check_result_path",                       deprecated::check_result_path_msg },
  { "use_embedded_perl_implicitly",                deprecated::embedded_perl_msg },
  { "use_large_installation_tweaks",               deprecated::engine_performance_msg },
  { "use_regexp_matching",                         deprecated::default_msg },
  { "use_retained_program_state",                  deprecated::retention_usage_msg },
  { "use_retained_scheduling_info",                deprecated::retention_usage_msg },
  { "use_true_regexp_matching",                    deprecated::default_msg },
  { "xcddefault_comment_file",                     deprecated::status_file_usage_msg },
  { "xdddefault_downtime_file",                    deprecated::status_file_usage_msg }
};

// Default values.
static long const                      default_additional_freshness_latency(15);
static std::string const               default_broker_module_directory("");
static long const                      default_cached_host_check_horizon(15);
static long const                      default_cached_service_check_horizon(15);
static bool const                      default_check_host_freshness(false);
static long const                      default_check_reaper_interval(10);
static bool const                      default_check_service_freshness(true);
static long const                      default_command_check_interval(-1);
static std::string const               default_command_file(DEFAULT_COMMAND_FILE);
static std::string const               default_debug_file(DEFAULT_DEBUG_FILE);
static unsigned long long const        default_debug_level(0);
static unsigned int const              default_debug_verbosity(1);
static bool const                      default_enable_event_handlers(true);
static bool const                      default_enable_flap_detection(false);
static bool const                      default_enable_predictive_host_dependency_checks(true);
static bool const                      default_enable_predictive_service_dependency_checks(true);
static unsigned long const             default_event_broker_options(std::numeric_limits<unsigned long>::max());
static long const                      default_event_handler_timeout(30);
static int const                       default_external_command_buffer_slots(4096);
static std::string const               default_global_host_event_handler("");
static std::string const               default_global_service_event_handler("");
static float const                     default_high_host_flap_threshold(30.0);
static float const                     default_high_service_flap_threshold(30.0);
static long const                      default_host_check_timeout(30);
static long const                      default_host_freshness_check_interval(60);
static std::string const               default_illegal_object_chars("");
static std::string const               default_illegal_output_chars("`~$&|'\"<>");
static bool const                      default_log_event_handlers(true);
static bool const                      default_log_external_commands(true);
static std::string const               default_log_file(DEFAULT_LOG_FILE);
static bool const                      default_log_host_retries(false);
static bool const                      default_log_initial_states(false);
static bool const                      default_log_passive_checks(true);
static bool const                      default_log_pid(true);
static bool const                      default_log_service_retries(false);
static float const                     default_low_host_flap_threshold(20.0);
static float const                     default_low_service_flap_threshold(20.0);
static unsigned long const             default_max_debug_file_size(1000000);
static unsigned long const             default_max_log_file_size(0);
static unsigned int const              default_max_parallel_service_checks(0);
static bool const                      default_obsess_over_hosts(false);
static bool const                      default_obsess_over_services(false);
static std::string const               default_ochp_command("");
static long const                      default_ochp_timeout(15);
static std::string const               default_ocsp_command("");
static long const                      default_ocsp_timeout(15);
static long const                      default_retention_update_interval(3600);
static long const                      default_service_check_timeout(60);
static long const                      default_service_freshness_check_interval(60);
static float const                     default_sleep_time(0.1);
static bool const                      default_soft_state_dependencies(false);
static std::string const               default_state_retention_file(DEFAULT_RETENTION_FILE);
static std::string const               default_status_file(DEFAULT_STATUS_FILE);
static long const                      default_time_change_threshold(900);
static bool const                      default_use_setpgid(true);
static bool const                      default_use_syslog(true);
static std::string const               default_use_timezone("");

/**
 *  Compare sets with the pointer content.
 *
 *  @param[in] s1 The first set.
 *  @param[in] s2 The second set.
 *
 *  @return True on success, otherwise false.
 */
template <typename T>
static bool cmp_set_ptr(
              std::set<shared_ptr<T> > const& s1,
              std::set<shared_ptr<T> > const& s2) {
  if (s1.size() != s2.size())
    return (false);
  typename std::set<shared_ptr<T> >::const_iterator
    it1(s1.begin()),
    end1(s1.end()),
    it2(s2.begin());
  while (it1 != end1)
    if (**it1++ != **it2++)
      return (false);
  return (true);
}

/**
 *  Default constructor.
 */
state::state()
  : _additional_freshness_latency(default_additional_freshness_latency),
    _cached_host_check_horizon(default_cached_host_check_horizon),
    _cached_service_check_horizon(default_cached_service_check_horizon),
    _check_host_freshness(default_check_host_freshness),
    _check_reaper_interval(default_check_reaper_interval),
    _check_service_freshness(default_check_service_freshness),
    _command_check_interval(default_command_check_interval),
    _command_file(default_command_file),
    _debug_file(default_debug_file),
    _debug_level(default_debug_level),
    _debug_verbosity(default_debug_verbosity),
    _enable_event_handlers(default_enable_event_handlers),
    _enable_flap_detection(default_enable_flap_detection),
    _enable_predictive_host_dependency_checks(default_enable_predictive_host_dependency_checks),
    _enable_predictive_service_dependency_checks(default_enable_predictive_service_dependency_checks),
    _event_broker_options(default_event_broker_options),
    _event_handler_timeout(default_event_handler_timeout),
    _external_command_buffer_slots(default_external_command_buffer_slots),
    _global_host_event_handler(default_global_host_event_handler),
    _global_service_event_handler(default_global_service_event_handler),
    _high_host_flap_threshold(default_high_host_flap_threshold),
    _high_service_flap_threshold(default_high_service_flap_threshold),
    _host_check_timeout(default_host_check_timeout),
    _host_freshness_check_interval(default_host_freshness_check_interval),
    _illegal_object_chars(default_illegal_object_chars),
    _illegal_output_chars(default_illegal_output_chars),
    _log_event_handlers(default_log_event_handlers),
    _log_external_commands(default_log_external_commands),
    _log_file(default_log_file),
    _log_host_retries(default_log_host_retries),
    _log_initial_states(default_log_initial_states),
    _log_passive_checks(default_log_passive_checks),
    _log_pid(default_log_pid),
    _log_service_retries(default_log_service_retries),
    _low_host_flap_threshold(default_low_host_flap_threshold),
    _low_service_flap_threshold(default_low_service_flap_threshold),
    _max_debug_file_size(default_max_debug_file_size),
    _max_log_file_size(default_max_log_file_size),
    _max_parallel_service_checks(default_max_parallel_service_checks),
    _obsess_over_hosts(default_obsess_over_hosts),
    _obsess_over_services(default_obsess_over_services),
    _ochp_command(default_ochp_command),
    _ochp_timeout(default_ochp_timeout),
    _ocsp_command(default_ocsp_command),
    _ocsp_timeout(default_ocsp_timeout),
    _retention_update_interval(default_retention_update_interval),
    _service_check_timeout(default_service_check_timeout),
    _service_freshness_check_interval(default_service_freshness_check_interval),
    _sleep_time(default_sleep_time),
    _soft_state_dependencies(default_soft_state_dependencies),
    _state_retention_file(default_state_retention_file),
    _status_file(default_status_file),
    _time_change_threshold(default_time_change_threshold),
    _use_setpgid(default_use_setpgid),
    _use_syslog(default_use_syslog),
    _use_timezone(default_use_timezone) {
  _users.resize(10);
}

/**
 *  Copy constructor.
 *
 *  @param[in] other  The object to copy.
 */
state::state(state const& other) {
  operator=(other);
}

/**
 *  Destructor.
 */
state::~state() throw () {}

/**
 *  Copy operator.
 *
 *  @param[in] other  The object to copy.
 *
 *  @return This object.
 */
state& state::operator=(state const& other) {
  if (this != &other) {
    _additional_freshness_latency = other._additional_freshness_latency;
    _broker_module_directory = other._broker_module_directory;
    _cached_host_check_horizon = other._cached_host_check_horizon;
    _cached_service_check_horizon = other._cached_service_check_horizon;
    _check_host_freshness = other._check_host_freshness;
    _check_reaper_interval = other._check_reaper_interval;
    _check_service_freshness = other._check_service_freshness;
    _commands = other._commands;
    _command_check_interval = other._command_check_interval;
    _command_file = other._command_file;
    _connectors = other._connectors;
    _debug_file = other._debug_file;
    _debug_level = other._debug_level;
    _debug_verbosity = other._debug_verbosity;
    _enable_event_handlers = other._enable_event_handlers;
    _enable_flap_detection = other._enable_flap_detection;
    _enable_predictive_host_dependency_checks = other._enable_predictive_host_dependency_checks;
    _enable_predictive_service_dependency_checks = other._enable_predictive_service_dependency_checks;
    _event_broker_options = other._event_broker_options;
    _event_handler_timeout = other._event_handler_timeout;
    _external_command_buffer_slots = other._external_command_buffer_slots;
    _global_host_event_handler = other._global_host_event_handler;
    _global_service_event_handler = other._global_service_event_handler;
    _high_host_flap_threshold = other._high_host_flap_threshold;
    _high_service_flap_threshold = other._high_service_flap_threshold;
    _hostdependencies = other._hostdependencies;
    _hosts = other._hosts;
    _host_check_timeout = other._host_check_timeout;
    _host_freshness_check_interval = other._host_freshness_check_interval;
    _illegal_object_chars = other._illegal_object_chars;
    _illegal_output_chars = other._illegal_output_chars;
    _log_event_handlers = other._log_event_handlers;
    _log_external_commands = other._log_external_commands;
    _log_file = other._log_file;
    _log_host_retries = other._log_host_retries;
    _log_initial_states = other._log_initial_states;
    _log_passive_checks = other._log_passive_checks;
    _log_pid = other._log_pid;
    _log_service_retries = other._log_service_retries;
    _low_host_flap_threshold = other._low_host_flap_threshold;
    _low_service_flap_threshold = other._low_service_flap_threshold;
    _max_debug_file_size = other._max_debug_file_size;
    _max_log_file_size = other._max_log_file_size;
    _max_parallel_service_checks = other._max_parallel_service_checks;
    _obsess_over_hosts = other._obsess_over_hosts;
    _obsess_over_services = other._obsess_over_services;
    _ochp_command = other._ochp_command;
    _ochp_timeout = other._ochp_timeout;
    _ocsp_command = other._ocsp_command;
    _ocsp_timeout = other._ocsp_timeout;
    _retention_update_interval = other._retention_update_interval;
    _servicedependencies = other._servicedependencies;
    _services = other._services;
    _service_check_timeout = other._service_check_timeout;
    _service_freshness_check_interval = other._service_freshness_check_interval;
    _sleep_time = other._sleep_time;
    _soft_state_dependencies = other._soft_state_dependencies;
    _state_retention_file = other._state_retention_file;
    _status_file = other._status_file;
    _timeperiods = other._timeperiods;
    _time_change_threshold = other._time_change_threshold;
    _users = other._users;
    _use_setpgid = other._use_setpgid;
    _use_syslog = other._use_syslog;
    _use_timezone = other._use_timezone;
  }
  return (*this);
}

/**
 *  Equal operator.
 *
 *  @param[in] other  The object to compare.
 *
 *  @return True if object is the same object, otherwise false.
 */
bool state::operator==(state const& other) const {
  return (_additional_freshness_latency == other._additional_freshness_latency
          && _broker_module_directory == other._broker_module_directory
          && _cached_host_check_horizon == other._cached_host_check_horizon
          && _cached_service_check_horizon == other._cached_service_check_horizon
          && _check_host_freshness == other._check_host_freshness
          && _check_reaper_interval == other._check_reaper_interval
          && _check_service_freshness == other._check_service_freshness
          && cmp_set_ptr(_commands, other._commands)
          && _command_check_interval == other._command_check_interval
          && _command_file == other._command_file
          && cmp_set_ptr(_connectors, other._connectors)
          && _debug_file == other._debug_file
          && _debug_level == other._debug_level
          && _debug_verbosity == other._debug_verbosity
          && _enable_event_handlers == other._enable_event_handlers
          && _enable_flap_detection == other._enable_flap_detection
          && _enable_predictive_host_dependency_checks == other._enable_predictive_host_dependency_checks
          && _enable_predictive_service_dependency_checks == other._enable_predictive_service_dependency_checks
          && _event_broker_options == other._event_broker_options
          && _event_handler_timeout == other._event_handler_timeout
          && _external_command_buffer_slots == other._external_command_buffer_slots
          && _global_host_event_handler == other._global_host_event_handler
          && _global_service_event_handler == other._global_service_event_handler
          && _high_host_flap_threshold == other._high_host_flap_threshold
          && _high_service_flap_threshold == other._high_service_flap_threshold
          && cmp_set_ptr(_hostdependencies, other._hostdependencies)
          && cmp_set_ptr(_hosts, other._hosts)
          && _host_check_timeout == other._host_check_timeout
          && _host_freshness_check_interval == other._host_freshness_check_interval
          && _illegal_object_chars == other._illegal_object_chars
          && _illegal_output_chars == other._illegal_output_chars
          && _log_event_handlers == other._log_event_handlers
          && _log_external_commands == other._log_external_commands
          && _log_file == other._log_file
          && _log_host_retries == other._log_host_retries
          && _log_initial_states == other._log_initial_states
          && _log_passive_checks == other._log_passive_checks
          && _log_pid == other._log_pid
          && _log_service_retries == other._log_service_retries
          && _low_host_flap_threshold == other._low_host_flap_threshold
          && _low_service_flap_threshold == other._low_service_flap_threshold
          && _max_debug_file_size == other._max_debug_file_size
          && _max_log_file_size == other._max_log_file_size
          && _max_parallel_service_checks == other._max_parallel_service_checks
          && _obsess_over_hosts == other._obsess_over_hosts
          && _obsess_over_services == other._obsess_over_services
          && _ochp_command == other._ochp_command
          && _ochp_timeout == other._ochp_timeout
          && _ocsp_command == other._ocsp_command
          && _ocsp_timeout == other._ocsp_timeout
          && _retention_update_interval == other._retention_update_interval
          && cmp_set_ptr(_servicedependencies, other._servicedependencies)
          && cmp_set_ptr(_services, other._services)
          && _service_check_timeout == other._service_check_timeout
          && _service_freshness_check_interval == other._service_freshness_check_interval
          && _sleep_time == other._sleep_time
          && _soft_state_dependencies == other._soft_state_dependencies
          && _state_retention_file == other._state_retention_file
          && _status_file == other._status_file
          && cmp_set_ptr(_timeperiods, other._timeperiods)
          && _time_change_threshold == other._time_change_threshold
          && _users == other._users
          && _use_setpgid == other._use_setpgid
          && _use_syslog == other._use_syslog
          && _use_timezone == other._use_timezone);
}

/**
 *  Not equal operator.
 *
 *  @param[in] other  The object to compare.
 *
 *  @return True if object is not the same object, otherwise false.
 */
bool state::operator!=(state const& other) const {
  return (!operator==(other));
}

/**
 *  Get additional_freshness_latency value.
 *
 *  @return The additional_freshness_latency value.
 */
duration const& state::additional_freshness_latency() const throw () {
  return (_additional_freshness_latency);
}

/**
 *  Set additional_freshness_latency value.
 *
 *  @param[in] value  The new additional_freshness_latency value.
 */
void state::additional_freshness_latency(duration const& value) {
  _additional_freshness_latency = value;
  return ;
}

/**
 *  Get broker_module value.
 *
 *  @return The list of broker modules.
 */
std::list<std::string> const& state::broker_module() const throw () {
  return (_broker_module);
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
std::string const& state::broker_module_directory() const throw () {
  return (_broker_module_directory);
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
duration const& state::cached_host_check_horizon() const throw () {
  return (_cached_host_check_horizon);
}

/**
 *  Set cached_host_check_horizon value.
 *
 *  @param[in] value  The new cached_host_check_horizon value.
 */
void state::cached_host_check_horizon(duration const& value) {
  _cached_host_check_horizon = value;
  return ;
}

/**
 *  Get cached_service_check_horizon value.
 *
 *  @return The cached_service_check_horizon value.
 */
duration const& state::cached_service_check_horizon() const throw () {
  return (_cached_service_check_horizon);
}

/**
 *  Set cached_service_check_horizon value.
 *
 *  @param[in] value  The new cached_service_check_horizon value.
 */
void state::cached_service_check_horizon(duration const& value) {
  _cached_service_check_horizon = value;
  return ;
}

/**
 *  Get cfg_dir value.
 *
 *  @return The cfg_dir value.
 */
std::list<std::string> const& state::cfg_dir() const throw () {
  return (_cfg_dir);
}

/**
 *  Get cfg_file value.
 *
 *  @return The cfg_file value.
 */
std::list<std::string> const& state::cfg_file() const throw () {

  return (_cfg_file);
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
std::string const& state::cfg_main() const throw () {
  return (_cfg_main);
}

/**
 *  Get check_host_freshness value.
 *
 *  @return The check_host_freshness value.
 */
bool state::check_host_freshness() const throw () {
  return (_check_host_freshness);
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
 *  Get check_reaper_interval value.
 *
 *  @return The check_reaper_interval value.
 */
duration const& state::check_reaper_interval() const throw () {
  return (_check_reaper_interval);
}

/**
 *  Set check_reaper_interval value.
 *
 *  @param[in] value  The new check_reaper_interval value.
 */
void state::check_reaper_interval(duration const& value) {
  if (value <= 0)
    throw (engine_error()
           << "check_reaper_interval cannot be equal to or less than 0");
  _check_reaper_interval = value;
  return ;
}

/**
 *  Get check_service_freshness value.
 *
 *  @return The check_service_freshness value.
 */
bool state::check_service_freshness() const throw () {
  return (_check_service_freshness);
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
set_command const& state::commands() const throw () {
  return (_commands);
}

/**
 *  Get all engine commands.
 *
 *  @return All engine commands.
 */
set_command& state::commands() throw () {
  return (_commands);
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
  shared_ptr<configuration::command>
    below_searched(new configuration::command(k));
  set_command::const_iterator
    it(_commands.upper_bound(below_searched));
  if ((it != _commands.end()) && ((*it)->command_name() == k))
    return (it);
  else if ((it != _commands.begin())
           && ((*--it)->command_name() == k))
    return (it);
  return (_commands.end());
}

/**
 *  Find a command by its key.
 *
 *  @param[in] k Command name.
 *
 *  @return Iterator to the element if found, commands().end()
 *          otherwise.
 */
set_command::iterator state::commands_find(
                               command::key_type const& k) {
  shared_ptr<configuration::command>
    below_searched(new configuration::command(k));
  set_command::iterator
    it(_commands.upper_bound(below_searched));
  if ((it != _commands.end()) && ((*it)->command_name() == k))
    return (it);
  else if ((it != _commands.begin())
           && ((*--it)->command_name() == k))
    return (it);
  return (_commands.end());
}

/**
 *  Get command_check_interval value.
 *
 *  @return The command_check_interval value.
 */
duration const& state::command_check_interval() const throw () {
  return (_command_check_interval);
}

/**
 *  Set command_check_interval value.
 *
 *  @param[in] value  The new command_check_interval value.
 */
void state::command_check_interval(duration const& value) {
  if (value < -1 || !value)
    throw (engine_error()
           << "command_check_interval must be either positive or -1 ("
           << value << " provided)");
  _command_check_interval = value;
  return ;
}

/**
 *  Get command_file value.
 *
 *  @return The command_file value.
 */
std::string const& state::command_file() const throw () {
  return (_command_file);
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
set_connector const& state::connectors() const throw () {
  return (_connectors);
}

/**
 *  Get all engine connectors.
 *
 *  @return All engine connectors.
 */
set_connector& state::connectors() throw () {
  return (_connectors);
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
  shared_ptr<configuration::connector>
    below_searched(new configuration::connector(k));
  set_connector::const_iterator
    it(_connectors.upper_bound(below_searched));
  if ((it != _connectors.end()) && ((*it)->connector_name() == k))
    return (it);
  else if ((it != _connectors.begin())
           && ((*--it)->connector_name() == k))
    return (it);
  return (_connectors.end());
}

/**
 *  Find a connector by its key.
 *
 *  @param[in] k Connector name.
 *
 *  @return Iterator to the element if found, connectors().end()
 *          otherwise.
 */
set_connector::iterator state::connectors_find(
                                 connector::key_type const& k) {
  shared_ptr<configuration::connector>
    below_searched(new configuration::connector(k));
  set_connector::iterator
    it(_connectors.upper_bound(below_searched));
  if ((it != _connectors.end()) && ((*it)->connector_name() == k))
    return (it);
  else if ((it != _connectors.begin())
           && ((*--it)->connector_name() == k))
    return (it);
  return (_connectors.end());
}

/**
 *  Get debug_file value.
 *
 *  @return The debug_file value.
 */
std::string const& state::debug_file() const throw () {
  return (_debug_file);
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
unsigned long long state::debug_level() const throw () {
  return (_debug_level);
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
unsigned int state::debug_verbosity() const throw () {
  return (_debug_verbosity);
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
 *  Get enable_event_handlers value.
 *
 *  @return The enable_event_handlers value.
 */
bool state::enable_event_handlers() const throw () {
  return (_enable_event_handlers);
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
bool state::enable_flap_detection() const throw () {
  return (_enable_flap_detection);
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
 *  Get enable_predictive_host_dependency_checks value.
 *
 *  @return The enable_predictive_host_dependency_checks value.
 */
bool state::enable_predictive_host_dependency_checks() const throw () {
  return (_enable_predictive_host_dependency_checks);
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
bool state::enable_predictive_service_dependency_checks() const throw () {
  return (_enable_predictive_service_dependency_checks);
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
unsigned long state::event_broker_options() const throw () {
  return (_event_broker_options);
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
duration const& state::event_handler_timeout() const throw () {
  return (_event_handler_timeout);
}

/**
 *  Set event_handler_timeout value.
 *
 *  @param[in] value  The new event_handler_timeout value.
 */
void state::event_handler_timeout(duration const& value) {
  if (value <= 0)
    throw (engine_error()
           << "event_handler_timeout cannot be equal to or less than 0");
  _event_handler_timeout = value;
  return ;
}

/**
 *  Get external_command_buffer_slots value.
 *
 *  @return The external_command_buffer_slots value.
 */
int state::external_command_buffer_slots() const throw () {
  return (_external_command_buffer_slots);
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
std::string const& state::global_host_event_handler() const throw () {
  return (_global_host_event_handler);
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
std::string const& state::global_service_event_handler() const throw () {
  return (_global_service_event_handler);
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
float state::high_host_flap_threshold() const throw () {
  return (_high_host_flap_threshold);
}

/**
 *  Set high_host_flap_threshold value.
 *
 *  @param[in] value The new high_host_flap_threshold value.
 */
void state::high_host_flap_threshold(float value) {
  if (value <= 0.0 || value >= 100.0)
    throw (engine_error() << "high_host_flap_threshold must "
           << "be between 0.0 and 100.0, both excluded");
  _high_host_flap_threshold = value;
}

/**
 *  Get high_service_flap_threshold value.
 *
 *  @return The high_service_flap_threshold value.
 */
float state::high_service_flap_threshold() const throw () {
  return (_high_service_flap_threshold);
}

/**
 *  Set high_service_flap_threshold value.
 *
 *  @param[in] value The new high_service_flap_threshold value.
 */
void state::high_service_flap_threshold(float value) {
  if (value <= 0.0 || value >= 100.0)
    throw (engine_error() << "high_service_flap_threshold "
           << "must be between 0.0 and 100.0, both excluded");
  _high_service_flap_threshold = value;
}

/**
 *  Get all engine hostdependencies.
 *
 *  @return All engine hostdependencies.
 */
set_hostdependency const& state::hostdependencies() const throw () {
  return (_hostdependencies);
}

/**
 *  Get all engine hostdependencies.
 *
 *  @return All engine hostdependencies.
 */
set_hostdependency& state::hostdependencies() throw () {
  return (_hostdependencies);
}

/**
 *  Get all engine hosts.
 *
 *  @return All engine hosts.
 */
set_host const& state::hosts() const throw () {
  return (_hosts);
}

/**
 *  Get all engine hosts.
 *
 *  @return All engine hosts.
 */
set_host& state::hosts() throw () {
  return (_hosts);
}

/**
 *  Find a host from its key.
 *
 *  @param[in] k Host key (host name).
 *
 *  @return Iterator to the host if found, hosts().end() if it was not.
 */
set_host::const_iterator state::hosts_find(
                                  host::key_type const& k) const {
  shared_ptr<configuration::host>
    below_searched(new configuration::host(k));
  set_host::const_iterator it(_hosts.upper_bound(below_searched));
  if ((it != _hosts.end()) && ((*it)->host_name() == k))
    return (it);
  else if ((it != _hosts.begin()) && ((*--it)->host_name() == k))
    return (it);
  return (_hosts.end());
}

/**
 *  Find a host from its key.
 *
 *  @param[in] k Host key (host name).
 *
 *  @return Iterator to the host if found, hosts().end() if it was not.
 */
set_host::iterator state::hosts_find(
                            host::key_type const& k) {
  shared_ptr<configuration::host>
    below_searched(new configuration::host(k));
  set_host::iterator it(_hosts.upper_bound(below_searched));
  if ((it != _hosts.end()) && ((*it)->host_name() == k))
    return (it);
  else if ((it != _hosts.begin()) && ((*--it)->host_name() == k))
    return (it);
  return (_hosts.end());
}

/**
 *  Get host_check_timeout value.
 *
 *  @return The host_check_timeout value.
 */
duration const& state::host_check_timeout() const throw () {
  return (_host_check_timeout);
}

/**
 *  Set host_check_timeout value.
 *
 *  @param[in] value  The new host_check_timeout value.
 */
void state::host_check_timeout(duration const& value) {
  if (value <= 0)
    throw (engine_error()
           << "host_check_timeout cannot be equal to or less than 0");
  return ;
  _host_check_timeout = value;
}

/**
 *  Get host_freshness_check_interval value.
 *
 *  @return The host_freshness_check_interval value.
 */
duration const& state::host_freshness_check_interval() const throw () {
  return (_host_freshness_check_interval);
}

/**
 *  Set host_freshness_check_interval value.
 *
 *  @param[in] value  The new host_freshness_check_interval value.
 */
void state::host_freshness_check_interval(duration const& value) {
  if (value <= 0)
    throw (engine_error()
           << "host_freshness_check_interval cannot be equal to or less than 0");
  _host_freshness_check_interval = value;
  return ;
}

/**
 *  Get illegal_object_chars value.
 *
 *  @return The illegal_object_chars value.
 */
std::string const& state::illegal_object_chars() const throw () {
  return (_illegal_object_chars);
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
std::string const& state::illegal_output_chars() const throw () {
  return (_illegal_output_chars);
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
 *  Get log_event_handlers value.
 *
 *  @return The log_event_handlers value.
 */
bool state::log_event_handlers() const throw () {
  return (_log_event_handlers);
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
bool state::log_external_commands() const throw () {
  return (_log_external_commands);
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
std::string const& state::log_file() const throw () {
  return (_log_file);
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
bool state::log_host_retries() const throw () {
  return (_log_host_retries);
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
 *  Get log_initial_states value.
 *
 *  @return The log_initial_states value.
 */
bool state::log_initial_states() const throw () {
  return (_log_initial_states);
}

/**
 *  Set log_initial_states value.
 *
 *  @param[in] value The new log_initial_states value.
 */
void state::log_initial_states(bool value) {
  _log_initial_states = value;
}

/**
 *  Get log_passive_checks value.
 *
 *  @return The log_passive_checks value.
 */
bool state::log_passive_checks() const throw () {
  return (_log_passive_checks);
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
bool state::log_pid() const throw() {
  return (_log_pid);
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
bool state::log_service_retries() const throw () {
  return (_log_service_retries);
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
float state::low_host_flap_threshold() const throw () {
  return (_low_host_flap_threshold);
}

/**
 *  Set low_host_flap_threshold value.
 *
 *  @param[in] value The new low_host_flap_threshold value.
 */
void state::low_host_flap_threshold(float value) {
  if (value <= 0.0 || value >= 100.0)
    throw (engine_error() << "low_host_flap_threshold "
           << "must be between 0.0 and 100.0, both excluded");
  _low_host_flap_threshold = value;
}

/**
 *  Get low_service_flap_threshold value.
 *
 *  @return The low_service_flap_threshold value.
 */
float state::low_service_flap_threshold() const throw () {
  return (_low_service_flap_threshold);
}

/**
 *  Set low_service_flap_threshold value.
 *
 *  @param[in] value The new low_service_flap_threshold value.
 */
void state::low_service_flap_threshold(float value) {
  if (value <= 0.0 || value >= 100.0)
    throw (engine_error() << "low_service_flap_threshold "
           << "must be between 0.0 and 100.0, both excluded");
  _low_service_flap_threshold = value;
}

/**
 *  Get max_debug_file_size value.
 *
 *  @return The max_debug_file_size value.
 */
unsigned long state::max_debug_file_size() const throw () {
  return (_max_debug_file_size);
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
 *  Get max_log_file_size value.
 *
 *  @return The max_log_file_size value.
 */
unsigned long state::max_log_file_size() const throw () {
  return (_max_log_file_size);
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
unsigned int state::max_parallel_service_checks() const throw () {
  return (_max_parallel_service_checks);
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
 *  Get obsess_over_hosts value.
 *
 *  @return The obsess_over_hosts value.
 */
bool state::obsess_over_hosts() const throw () {
  return (_obsess_over_hosts);
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
bool state::obsess_over_services() const throw () {
  return (_obsess_over_services);
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
std::string const& state::ochp_command() const throw () {
  return (_ochp_command);
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
duration const& state::ochp_timeout() const throw () {
  return (_ochp_timeout);
}

/**
 *  Set ochp_timeout value.
 *
 *  @param[in] value  The new ochp_timeout value.
 */
void state::ochp_timeout(duration const& value) {
  if (value <= 0)
    throw (engine_error()
           << "ochp_timeout cannot be equal to or less than 0");
  _ochp_timeout = value;
  return ;
}

/**
 *  Get ocsp_command value.
 *
 *  @return The ocsp_command value.
 */
std::string const& state::ocsp_command() const throw () {
  return (_ocsp_command);
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
duration const& state::ocsp_timeout() const throw () {
  return (_ocsp_timeout);
}

/**
 *  Set ocsp_timeout value.
 *
 *  @param[in] value  The new ocsp_timeout value.
 */
void state::ocsp_timeout(duration const& value) {
  if (value <= 0)
    throw (engine_error()
           << "ocsp_timeout cannot be equal to or less than 0");
  _ocsp_timeout = value;
  return ;
}

/**
 *  Get resource_file value.
 *
 *  @return The resource_file value.
 */
std::list<std::string> const& state::resource_file() const throw () {
  return (_resource_file);
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
 *  Get retention_update_interval value.
 *
 *  @return The retention_update_interval value.
 */
duration const& state::retention_update_interval() const throw () {
  return (_retention_update_interval);
}

/**
 *  Set retention_update_interval value.
 *
 *  @param[in] value  The new retention_update_interval value.
 */
void state::retention_update_interval(duration const& value) {
  if (value <= 0)
    throw (engine_error()
           << "retention_update_interval cannot be equal to or less than 0");
  _retention_update_interval = value;
  return ;
}

/**
 *  Get all engine servicedependencies.
 *
 *  @return All engine servicedependencies.
 */
set_servicedependency const& state::servicedependencies() const throw () {
  return (_servicedependencies);
}

/**
 *  Get all engine servicedependencies.
 *
 *  @return All engine servicedependencies.
 */
set_servicedependency& state::servicedependencies() throw () {
  return (_servicedependencies);
}

/**
 *  Get all engine services.
 *
 *  @return All engine services.
 */
set_service const& state::services() const throw () {
  return (_services);
}

/**
 *  Get all engine services.
 *
 *  @return All engine services.
 */
set_service& state::services() throw () {
  return (_services);
}

/**
 *  Get service by its key.
 *
 *  @param[in] k Service name.
 *
 *  @return Iterator to the element if found, services().end()
 *          otherwise.
 */
set_service::const_iterator state::services_find(
                                     service::key_type const& k) const {
  shared_ptr<configuration::service>
    below_searched(new configuration::service);
  below_searched->hosts().push_back(k.first);
  below_searched->service_description() = k.second;
  set_service::const_iterator
    it(_services.upper_bound(below_searched));
  if ((it != _services.end())
      && ((*it)->hosts().front() == k.first)
      && ((*it)->service_description() == k.second))
    return (it);
  else if ((it != _services.begin())
           && ((*--it)->hosts().front() == k.first)
           && ((*it)->service_description() == k.second))
    return (it);
  return (_services.end());
}

/**
 *  Get service by its key.
 *
 *  @param[in] k Service name.
 *
 *  @return Iterator to the element if found, services().end()
 *          otherwise.
 */
set_service::iterator state::services_find(
                               service::key_type const& k) {
  shared_ptr<configuration::service>
    below_searched(new configuration::service);
  below_searched->hosts().push_back(k.first);
  below_searched->service_description() = k.second;
  set_service::iterator
    it(_services.upper_bound(below_searched));
  if ((it != _services.end())
      && ((*it)->hosts().front() == k.first)
      && ((*it)->service_description() == k.second))
    return (it);
  else if ((it != _services.begin())
           && ((*--it)->hosts().front() == k.first)
           && ((*it)->service_description() == k.second))
    return (it);
  return (_services.end());
}

/**
 *  Get service_check_timeout value.
 *
 *  @return The service_check_timeout value.
 */
duration const& state::service_check_timeout() const throw () {
  return (_service_check_timeout);
}

/**
 *  Set service_check_timeout value.
 *
 *  @param[in] value  The new service_check_timeout value.
 */
void state::service_check_timeout(duration const& value) {
  if (value <= 0)
    throw (engine_error()
           << "service_check_timeout cannot be equal to or less than 0");
  _service_check_timeout = value;
  return ;
}

/**
 *  Get service_freshness_check_interval value.
 *
 *  @return The service_freshness_check_interval value.
 */
duration const& state::service_freshness_check_interval() const throw () {
  return (_service_freshness_check_interval);
}

/**
 *  Set service_freshness_check_interval value.
 *
 *  @param[in] value  The new service_freshness_check_interval value.
 */
void state::service_freshness_check_interval(duration const& value) {
  if (value <= 0)
    throw (engine_error()
           << "service_freshness_check_interval cannot be equal to or less than 0");
  _service_freshness_check_interval = value;
  return ;
}

/**
 *  Get sleep_time value.
 *
 *  @return The sleep_time value.
 */
float state::sleep_time() const throw () {
  return (_sleep_time);
}

/**
 *  Set sleep_time value.
 *
 *  @param[in] value The new sleep_time value.
 */
void state::sleep_time(float value) {
  if (value <= 0.0)
    throw (engine_error() << "sleep_time cannot be less or equal to 0 ("
           << value << " provided)");
  _sleep_time = value;
}

/**
 *  Get soft_state_dependencies value.
 *
 *  @return The soft_state_dependencies value.
 */
bool state::soft_state_dependencies() const throw () {
  return (_soft_state_dependencies);
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
std::string const& state::state_retention_file() const throw () {
  return (_state_retention_file);
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
std::string const& state::status_file() const throw () {
  return (_status_file);
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
 *  Set a property with new value.
 *
 *  @param[in] key   The property name.
 *  @param[in] value The new value.
 *
 *  @return True on success, otherwise false.
 */
bool state::set(char const* key, char const* value) {
  try {
    for (unsigned int i(0);
         i < sizeof(_setters) / sizeof(*_setters);
         ++i)
      if (!strcmp(_setters[i].name, key))
        return ((_setters[i].func)(*this, key, value));
    for (unsigned int i(0);
         i < sizeof(_deprecated) / sizeof(*_deprecated);
         ++i)
      if (!strcmp(_deprecated[i][0], key)) {
        logger(log_config_warning, basic)
          << "Warning: variable '" << key << "' is no longer supported\n"
          << _deprecated[i][1];
        ++config_warnings;
        return (true);
      }
  }
  catch (std::exception const& e) {
    logger(log_config_error, basic)
      << e.what();
    return (false);
  }
  return (true);
}

/**
 *  Get all engine timeperiods.
 *
 *  @return All engine timeperiods.
 */
set_timeperiod const& state::timeperiods() const throw () {
  return (_timeperiods);
}

/**
 *  Get all engine timeperiods.
 *
 *  @return All engine timeperiods.
 */
set_timeperiod& state::timeperiods() throw () {
  return (_timeperiods);
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
  shared_ptr<configuration::timeperiod>
    below_searched(new configuration::timeperiod(k));
  set_timeperiod::const_iterator
    it(_timeperiods.upper_bound(below_searched));
  if ((it != _timeperiods.end()) && ((*it)->timeperiod_name() == k))
    return (it);
  else if ((it != _timeperiods.begin())
           && ((*--it)->timeperiod_name() == k))
    return (it);
  return (_timeperiods.end());
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
  shared_ptr<configuration::timeperiod>
    below_searched(new configuration::timeperiod(k));
  set_timeperiod::iterator
    it(_timeperiods.upper_bound(below_searched));
  if ((it != _timeperiods.end()) && ((*it)->timeperiod_name() == k))
    return (it);
  else if ((it != _timeperiods.begin())
           && ((*--it)->timeperiod_name() == k))
    return (it);
  return (_timeperiods.end());
}

/**
 *  Get time_change_threshold value.
 *
 *  @return The time_change_threshold value.
 */
duration const& state::time_change_threshold() const throw () {
  return (_time_change_threshold);
}

/**
 *  Set time_change_threshold value.
 *
 *  @param[in] value  The new time_change_threshold value.
 */
void state::time_change_threshold(duration const& value) {
  if (value < 6)
    throw (engine_error()
           << "time_change_threshold cannot be less than 6 ("
           << value << " provided)");
  _time_change_threshold = value;
  return ;
}

/**
 *  Get user resources.
 *
 *  @return The users resources list.
 */
std::vector<std::string> const& state::user() const throw () {
  return (_users);
}

/**
 *  Set the user resources.
 *
 *  @param[in] value The new users list.
 */
void state::user(std::vector<std::string> const& value) {
  _users = value;
}

/**
 *  Set the user resources.
 *
 *  @param[in] key   The user key.
 *  @param[in] value The user value.
 */
void state::user(std::string const& key, std::string const& value) {
  std::size_t pos(key.find("$USER"));
  if (pos != 0)
    throw (engine_error() << "Invalid user key '" << key << "'");
  std::string tmp(key.substr(5));

  pos = tmp.size();
  if (!pos || tmp[pos - 1] != '$')
    throw (engine_error() << "Invalid user key '" << key << "'");
  tmp.erase(pos - 1);

  unsigned int idx;
  if (!string::to(tmp.c_str(), idx) || !idx || idx > MAX_USER_MACROS)
    throw (engine_error() << "Invalid user key '" << key << "'");

  user(idx - 1, value);
}

/**
 *  Set the user resources.
 *
 *  @param[in] key   The user key.
 *  @param[in] value The user value.
 */
void state::user(unsigned int key, std::string const& value) {
  if (key >= _users.size())
    _users.resize(key + 1);
  _users[key] = value;
}

/**
 *  Get use_setpgid value.
 *
 *  @return The use_setpgid value.
 */
bool state::use_setpgid() const throw () {
  return (_use_setpgid);
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
bool state::use_syslog() const throw () {
  return (_use_syslog);
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
std::string const& state::use_timezone() const throw () {
  return (_use_timezone);
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
 *  Set event_broker_options.
 *
 *  @param[in] value The new event_broker_options value.
 */
void state::_set_event_broker_options(std::string const& value) {
  if (value != "-1")
    _event_broker_options = std::strtoul(value.c_str(), NULL, 0);
  else
    _event_broker_options = BROKER_EVERYTHING;
  return ;
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

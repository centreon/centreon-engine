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

#ifndef CCE_CONFIGURATION_STATE_HH
#  define CCE_CONFIGURATION_STATE_HH

#  include <list>
#  include <set>
#  include <string>
#  include <vector>
#  include "com/centreon/engine/configuration/command.hh"
#  include "com/centreon/engine/configuration/connector.hh"
#  include "com/centreon/engine/configuration/host.hh"
#  include "com/centreon/engine/configuration/hostdependency.hh"
#  include "com/centreon/engine/configuration/hostgroup.hh"
#  include "com/centreon/engine/configuration/service.hh"
#  include "com/centreon/engine/configuration/servicedependency.hh"
#  include "com/centreon/engine/configuration/servicegroup.hh"
#  include "com/centreon/engine/configuration/timeperiod.hh"
#  include "com/centreon/engine/logging/logger.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/string.hh"
#  include "com/centreon/shared_ptr.hh"

CCE_BEGIN()

namespace               configuration {
  /**
   *  @class state state.hh
   *  @brief Simple configuration state class.
   *
   *  Simple configuration state class used by Centreon Engine
   *  to manage configuration data.
   */
  class                 state {
  public:
                        state();
                        state(state const& right);
                        ~state() throw ();
    state&              operator=(state const& right);
    bool                operator==(state const& right) const throw ();
    bool                operator!=(state const& right) const throw ();
    int                 additional_freshness_latency() const throw ();
    void                additional_freshness_latency(int value);
    std::list<std::string> const&
                        broker_module() const throw ();
    void                broker_module(std::list<std::string> const& value);
    std::string const&  broker_module_directory() const throw ();
    void                broker_module_directory(std::string const& value);
    unsigned long       cached_host_check_horizon() const throw ();
    void                cached_host_check_horizon(unsigned long value);
    unsigned long       cached_service_check_horizon() const throw ();
    void                cached_service_check_horizon(unsigned long value);
    std::list<std::string> const&
                        cfg_dir() const throw ();
    std::list<std::string> const&
                        cfg_file() const throw ();
    std::string const&  cfg_main() const throw ();
    void                cfg_main(std::string const& value);
    bool                check_host_freshness() const throw ();
    void                check_host_freshness(bool value);
    unsigned int        check_reaper_interval() const throw ();
    void                check_reaper_interval(unsigned int value);
    bool                check_service_freshness() const throw ();
    void                check_service_freshness(bool value);
    set_command const&  commands() const throw ();
    set_command&        commands() throw ();
    set_command::const_iterator
                        commands_find(command::key_type const& k) const;
    set_command::iterator
                        commands_find(command::key_type const& k);
    int                 command_check_interval() const throw ();
    void                command_check_interval(int value);
    void                command_check_interval(int value, bool is_second);
    bool                command_check_interval_is_seconds() const throw();
    std::string const&  command_file() const throw ();
    void                command_file(std::string const& value);
    set_connector const&
                        connectors() const throw ();
    set_connector&      connectors() throw ();
    set_connector::const_iterator
                        connectors_find(connector::key_type const& k) const;
    set_connector::iterator
                        connectors_find(connector::key_type const& k);
    std::string const&  debug_file() const throw ();
    void                debug_file(std::string const& value);
    unsigned long       debug_level() const throw ();
    void                debug_level(unsigned long value);
    unsigned int        debug_verbosity() const throw ();
    void                debug_verbosity(unsigned int value);
    bool                enable_event_handlers() const throw ();
    void                enable_event_handlers(bool value);
    bool                enable_flap_detection() const throw ();
    void                enable_flap_detection(bool value);
    bool                enable_predictive_host_dependency_checks() const throw ();
    void                enable_predictive_host_dependency_checks(bool value);
    bool                enable_predictive_service_dependency_checks() const throw ();
    void                enable_predictive_service_dependency_checks(bool value);
    unsigned long       event_broker_options() const throw ();
    void                event_broker_options(unsigned long value);
    unsigned int        event_handler_timeout() const throw ();
    void                event_handler_timeout(unsigned int value);
    int                 external_command_buffer_slots() const throw ();
    void                external_command_buffer_slots(int value);
    std::string const&  global_host_event_handler() const throw ();
    void                global_host_event_handler(std::string const& value);
    std::string const&  global_service_event_handler() const throw ();
    void                global_service_event_handler(std::string const& value);
    float               high_host_flap_threshold() const throw ();
    void                high_host_flap_threshold(float value);
    float               high_service_flap_threshold() const throw ();
    void                high_service_flap_threshold(float value);
    set_hostdependency const&
                        hostdependencies() const throw ();
    set_hostdependency& hostdependencies() throw ();
    set_hostgroup const&
                        hostgroups() const throw ();
    set_hostgroup&      hostgroups() throw ();
    set_hostgroup::const_iterator
                        hostgroups_find(hostgroup::key_type const& k) const;
    set_hostgroup::iterator
                        hostgroups_find(hostgroup::key_type const& k);
    set_host const&     hosts() const throw ();
    set_host&           hosts() throw ();
    set_host::const_iterator
                        hosts_find(host::key_type const& k) const;
    set_host::iterator  hosts_find(host::key_type const& k);
    unsigned int        host_check_timeout() const throw ();
    void                host_check_timeout(unsigned int value);
    unsigned int        host_freshness_check_interval() const throw ();
    void                host_freshness_check_interval(unsigned int value);
    std::string const&  illegal_object_chars() const throw ();
    void                illegal_object_chars(std::string const& value);
    std::string const&  illegal_output_chars() const throw ();
    void                illegal_output_chars(std::string const& value);
    unsigned int        interval_length() const throw ();
    void                interval_length(unsigned int value);
    bool                log_event_handlers() const throw ();
    void                log_event_handlers(bool value);
    bool                log_external_commands() const throw ();
    void                log_external_commands(bool value);
    std::string const&  log_file() const throw ();
    void                log_file(std::string const& value);
    bool                log_host_retries() const throw ();
    void                log_host_retries(bool value);
    bool                log_initial_states() const throw ();
    void                log_initial_states(bool value);
    bool                log_passive_checks() const throw ();
    void                log_passive_checks(bool value);
    bool                log_service_retries() const throw ();
    void                log_service_retries(bool value);
    float               low_host_flap_threshold() const throw ();
    void                low_host_flap_threshold(float value);
    float               low_service_flap_threshold() const throw ();
    void                low_service_flap_threshold(float value);
    unsigned long       max_debug_file_size() const throw ();
    void                max_debug_file_size(unsigned long value);
    unsigned long       max_log_file_size() const throw ();
    void                max_log_file_size(unsigned long value);
    unsigned int        max_parallel_service_checks() const throw ();
    void                max_parallel_service_checks(unsigned int value);
    bool                obsess_over_hosts() const throw ();
    void                obsess_over_hosts(bool value);
    bool                obsess_over_services() const throw ();
    void                obsess_over_services(bool value);
    std::string const&  ochp_command() const throw ();
    void                ochp_command(std::string const& value);
    unsigned int        ochp_timeout() const throw ();
    void                ochp_timeout(unsigned int value);
    std::string const&  ocsp_command() const throw ();
    void                ocsp_command(std::string const& value);
    unsigned int        ocsp_timeout() const throw ();
    void                ocsp_timeout(unsigned int value);
    bool                passive_host_checks_are_soft() const throw ();
    void                passive_host_checks_are_soft(bool value);
    std::list<std::string> const&
                        resource_file() const throw ();
    void                resource_file(std::list<std::string> const& value);
    unsigned int        retention_update_interval() const throw ();
    void                retention_update_interval(unsigned int value);
    set_servicedependency const&
                        servicedependencies() const throw ();
    set_servicedependency&
                        servicedependencies() throw ();
    set_servicegroup const&
                        servicegroups() const throw ();
    set_servicegroup&   servicegroups() throw ();
    set_servicegroup::const_iterator
                        servicegroups_find(servicegroup::key_type const& k) const;
    set_servicegroup::iterator
                        servicegroups_find(servicegroup::key_type const& k);
    set_service const&  services() const throw ();
    set_service&        services() throw ();
    set_service::const_iterator
                        services_find(service::key_type const& k) const;
    set_service::iterator
                        services_find(service::key_type const& k);
    unsigned int        service_check_timeout() const throw ();
    void                service_check_timeout(unsigned int value);
    unsigned int        service_freshness_check_interval() const throw ();
    void                service_freshness_check_interval(unsigned int value);
    float               sleep_time() const throw ();
    void                sleep_time(float value);
    bool                soft_state_dependencies() const throw ();
    void                soft_state_dependencies(bool value);
    std::string const&  state_retention_file() const throw ();
    void                state_retention_file(std::string const& value);
    std::string const&  status_file() const throw ();
    void                status_file(std::string const& value);
    bool                set(char const* key, char const* value);
    set_timeperiod const&
                        timeperiods() const throw ();
    set_timeperiod&     timeperiods() throw ();
    set_timeperiod::const_iterator
                        timeperiods_find(timeperiod::key_type const& k) const;
    set_timeperiod::iterator
                        timeperiods_find(timeperiod::key_type const& k);
    unsigned int        time_change_threshold() const throw ();
    void                time_change_threshold(unsigned int value);
    std::vector<std::string> const&
                        user() const throw ();
    void                user(std::vector<std::string> const& value);
    void                user(std::string const& key, std::string const& value);
    void                user(unsigned int key, std::string const& value);
    bool                use_setpgid() const throw ();
    void                use_setpgid(bool value);
    bool                use_syslog() const throw ();
    void                use_syslog(bool value);
    std::string const&  use_timezone() const throw ();
    void                use_timezone(std::string const& value);

  private:
    struct              setters {
      char const*       name;
      bool              (*func)(state&, char const*);
    };

    void                _set_accept_passive_host_checks(bool value);
    void                _set_accept_passive_service_checks(bool value);
    void                _set_admin_email(std::string const& value);
    void                _set_admin_pager(std::string const& value);
    void                _set_aggregate_status_updates(std::string const& value);
    void                _set_allow_empty_hostgroup_assignment(bool value);
    void                _set_auth_file(std::string const& value);
    void                _set_auto_reschedule_checks(bool value);
    void                _set_auto_rescheduling_interval(unsigned int value);
    void                _set_auto_rescheduling_window(unsigned int value);
    void                _set_bare_update_check(std::string const& value);
    void                _set_broker_module(std::string const& value);
    void                _set_cfg_dir(std::string const& value);
    void                _set_cfg_file(std::string const& value);
    void                _set_check_external_commands(bool value);
    void                _set_check_for_orphaned_hosts(bool value);
    void                _set_check_for_orphaned_services(bool value);
    void                _set_check_for_updates(std::string const& value);
    void                _set_check_result_path(std::string const& value);
    void                _set_child_processes_fork_twice(std::string const& value);
    void                _set_command_check_interval(std::string const& value);
    void                _set_comment_file(std::string const& value);
    void                _set_daemon_dumps_core(std::string const& value);
    void                _set_date_format(std::string const& value);
    void                _set_downtime_file(std::string const& value);
    void                _set_enable_embedded_perl(std::string const& value);
    void                _set_enable_environment_macros(bool value);
    void                _set_enable_failure_prediction(bool value);
    void                _set_enable_notifications(bool value);
    void                _set_event_broker_options(std::string const& value);
    void                _set_execute_host_checks(bool value);
    void                _set_execute_service_checks(bool value);
    void                _set_free_child_process_memory(std::string const& value);
    void                _set_host_inter_check_delay_method(std::string const& value);
    void                _set_host_perfdata_command(std::string const& value);
    void                _set_host_perfdata_file(std::string const& value);
    void                _set_host_perfdata_file_mode(std::string const& value);
    void                _set_host_perfdata_file_processing_command(std::string const& value);
    void                _set_host_perfdata_file_processing_interval(unsigned int value);
    void                _set_host_perfdata_file_template(std::string const& value);
    void                _set_lock_file(std::string const& value);
    void                _set_log_archive_path(std::string const& value);
    void                _set_log_notifications(bool value);
    void                _set_log_rotation_method(std::string const& value);
    void                _set_max_check_result_file_age(unsigned long value);
    void                _set_max_check_result_reaper_time(unsigned int value);
    void                _set_max_host_check_spread(unsigned int value);
    void                _set_max_service_check_spread(unsigned int value);
    void                _set_nagios_group(std::string const& value);
    void                _set_nagios_user(std::string const& value);
    void                _set_notification_timeout(unsigned int value);
    void                _set_object_cache_file(std::string const& value);
    void                _set_p1_file(std::string const& value);
    void                _set_perfdata_timeout(int value);
    void                _set_precached_object_file(std::string const& value);
    void                _set_process_performance_data(bool value);
    void                _set_resource_file(std::string const& value);
    void                _set_retained_contact_host_attribute_mask(unsigned long value);
    void                _set_retained_contact_service_attribute_mask(unsigned long value);
    void                _set_retained_host_attribute_mask(unsigned long value);
    void                _set_retained_process_host_attribute_mask(unsigned long value);
    void                _set_retained_process_service_attribute_mask(std::string const& value);
    void                _set_retained_service_attribute_mask(std::string const& value);
    void                _set_retain_state_information(bool value);
    void                _set_retention_scheduling_horizon(unsigned int value);
    void                _set_service_inter_check_delay_method(std::string const& value);
    void                _set_service_interleave_factor_method(std::string const& value);
    void                _set_service_perfdata_command(std::string const& value);
    void                _set_service_perfdata_file(std::string const& value);
    void                _set_service_perfdata_file_mode(std::string const& value);
    void                _set_service_perfdata_file_processing_command(std::string const& value);
    void                _set_service_perfdata_file_processing_interval(unsigned int value);
    void                _set_service_perfdata_file_template(std::string const& value);
    void                _set_status_update_interval(unsigned int value);
    void                _set_temp_file(std::string const& value);
    void                _set_temp_path(std::string const& value);
    void                _set_translate_passive_host_checks(bool value);
    void                _set_use_aggressive_host_checking(bool value);
    void                _set_use_check_result_path(bool value);
    void                _set_use_embedded_perl_implicitly(std::string const& value);
    void                _set_use_large_installation_tweaks(bool value);
    void                _set_use_regexp_matching(bool value);
    void                _set_use_retained_program_state(bool value);
    void                _set_use_retained_scheduling_info(bool value);
    void                _set_use_true_regexp_matching(bool value);

    template<typename U, void (state::*ptr)(U)>
    struct              setter {
      static bool       generic(state& obj, char const* value) {
        try {
          U val(0);
          if (!string::to(value, val))
            return (false);
          (obj.*ptr)(val);
        }
        catch (std::exception const& e) {
          logger(logging::log_config_error, logging::basic)
            << e.what();
          return (false);
        }
        return (true);
      }
    };

    template<void (state::*ptr)(std::string const&)>
    struct              setter<std::string const&, ptr> {
      static bool       generic(state& obj, char const* value) {
        try {
          (obj.*ptr)(value);
        }
        catch (std::exception const& e) {
          logger(logging::log_config_error, logging::basic)
            << e.what();
          return (false);
        }
        return (true);
      }
    };

    int                 _additional_freshness_latency;
    std::list<std::string>
                        _broker_module;
    std::string         _broker_module_directory;
    unsigned long       _cached_host_check_horizon;
    unsigned long       _cached_service_check_horizon;
    std::list<std::string>
                        _cfg_dir;
    std::list<std::string>
                        _cfg_file;
    std::string         _cfg_main;
    bool                _check_host_freshness;
    unsigned int        _check_reaper_interval;
    bool                _check_service_freshness;
    set_command         _commands;
    int                 _command_check_interval;
    bool                _command_check_interval_is_seconds;
    std::string         _command_file;
    set_connector       _connectors;
    std::string         _debug_file;
    unsigned long       _debug_level;
    unsigned int        _debug_verbosity;
    bool                _enable_event_handlers;
    bool                _enable_flap_detection;
    bool                _enable_predictive_host_dependency_checks;
    bool                _enable_predictive_service_dependency_checks;
    unsigned long       _event_broker_options;
    unsigned int        _event_handler_timeout;
    int                 _external_command_buffer_slots;
    std::string         _global_host_event_handler;
    std::string         _global_service_event_handler;
    float               _high_host_flap_threshold;
    float               _high_service_flap_threshold;
    set_hostdependency  _hostdependencies;
    set_hostgroup       _hostgroups;
    set_host            _hosts;
    unsigned int        _host_check_timeout;
    unsigned int        _host_freshness_check_interval;
    std::string         _illegal_object_chars;
    std::string         _illegal_output_chars;
    unsigned int        _interval_length;
    bool                _log_event_handlers;
    bool                _log_external_commands;
    std::string         _log_file;
    bool                _log_host_retries;
    bool                _log_initial_states;
    bool                _log_passive_checks;
    bool                _log_service_retries;
    float               _low_host_flap_threshold;
    float               _low_service_flap_threshold;
    unsigned long       _max_debug_file_size;
    unsigned long       _max_log_file_size;
    unsigned int        _max_parallel_service_checks;
    bool                _obsess_over_hosts;
    bool                _obsess_over_services;
    std::string         _ochp_command;
    unsigned int        _ochp_timeout;
    std::string         _ocsp_command;
    unsigned int        _ocsp_timeout;
    bool                _passive_host_checks_are_soft;
    std::list<std::string>
                        _resource_file;
    unsigned int        _retention_update_interval;
    set_servicedependency
                        _servicedependencies;
    set_servicegroup    _servicegroups;
    set_service         _services;
    unsigned int        _service_check_timeout;
    unsigned int        _service_freshness_check_interval;
    static setters const
                        _setters[];
    float               _sleep_time;
    bool                _soft_state_dependencies;
    std::string         _state_retention_file;
    std::string         _status_file;
    set_timeperiod      _timeperiods;
    unsigned int        _time_change_threshold;
    std::vector<std::string>
                        _users;
    bool                _use_setpgid;
    bool                _use_syslog;
    std::string         _use_timezone;
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_STATE_HH

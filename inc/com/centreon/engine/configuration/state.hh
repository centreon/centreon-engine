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
#  include "com/centreon/engine/configuration/duration.hh"
#  include "com/centreon/engine/configuration/host.hh"
#  include "com/centreon/engine/configuration/hostdependency.hh"
#  include "com/centreon/engine/configuration/service.hh"
#  include "com/centreon/engine/configuration/servicedependency.hh"
#  include "com/centreon/engine/configuration/timeperiod.hh"
#  include "com/centreon/engine/logging/logger.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/string.hh"
#  include "com/centreon/shared_ptr.hh"

CCE_BEGIN()

namespace                           configuration {
  /**
   *  @class state state.hh "com/centreon/engine/configuration/state.hh"
   *  @brief Global state class.
   *
   *  Configuration state class used by Centreon Engine to manage the
   *  entire configuration data.
   */
  class                             state {
  public:
                                    state();
                                    state(state const& other);
                                    ~state() throw ();
    state&                          operator=(state const& other);
    bool                            operator==(state const& other) const;
    bool                            operator!=(state const& other) const;
    duration const&                 additional_freshness_latency() const throw ();
    void                            additional_freshness_latency(duration const& value);
    std::list<std::string> const&   broker_module() const throw ();
    void                            broker_module(std::list<std::string> const& value);
    std::string const&              broker_module_directory() const throw ();
    void                            broker_module_directory(std::string const& value);
    duration const&                 cached_host_check_horizon() const throw ();
    void                            cached_host_check_horizon(duration const& value);
    duration const&                 cached_service_check_horizon() const throw ();
    void                            cached_service_check_horizon(duration const& value);
    std::list<std::string> const&   cfg_dir() const throw ();
    std::list<std::string> const&   cfg_file() const throw ();
    std::list<std::string>&         cfg_include() throw ();
    std::list<std::string> const&   cfg_include() const throw ();
    std::string const&              cfg_main() const throw ();
    void                            cfg_main(std::string const& value);
    bool                            check_host_freshness() const throw ();
    void                            check_host_freshness(bool value);
    duration const&                 check_reaper_interval() const throw ();
    void                            check_reaper_interval(duration const& value);
    bool                            check_service_freshness() const throw ();
    void                            check_service_freshness(bool value);
    set_command const&              commands() const throw ();
    set_command&                    commands() throw ();
    set_command::const_iterator     commands_find(command::key_type const& k) const;
    set_command::iterator           commands_find(command::key_type const& k);
    duration const&                 command_check_interval() const throw ();
    void                            command_check_interval(duration const& value);
    std::string const&              command_file() const throw ();
    void                            command_file(std::string const& value);
    set_connector const&            connectors() const throw ();
    set_connector&                  connectors() throw ();
    set_connector::const_iterator   connectors_find(connector::key_type const& k) const;
    set_connector::iterator         connectors_find(connector::key_type const& k);
    std::string const&              debug_file() const throw ();
    void                            debug_file(std::string const& value);
    unsigned long long              debug_level() const throw ();
    void                            debug_level(unsigned long long value);
    unsigned int                    debug_verbosity() const throw ();
    void                            debug_verbosity(unsigned int value);
    bool                            enable_event_handlers() const throw ();
    void                            enable_event_handlers(bool value);
    bool                            enable_flap_detection() const throw ();
    void                            enable_flap_detection(bool value);
    bool                            enable_predictive_host_dependency_checks() const throw ();
    void                            enable_predictive_host_dependency_checks(bool value);
    bool                            enable_predictive_service_dependency_checks() const throw ();
    void                            enable_predictive_service_dependency_checks(bool value);
    unsigned long                   event_broker_options() const throw ();
    void                            event_broker_options(unsigned long value);
    duration const&                 event_handler_timeout() const throw ();
    void                            event_handler_timeout(duration const& value);
    int                             external_command_buffer_slots() const throw ();
    void                            external_command_buffer_slots(int value);
    std::string const&              global_host_event_handler() const throw ();
    void                            global_host_event_handler(std::string const& value);
    std::string const&              global_service_event_handler() const throw ();
    void                            global_service_event_handler(std::string const& value);
    float                           high_host_flap_threshold() const throw ();
    void                            high_host_flap_threshold(float value);
    float                           high_service_flap_threshold() const throw ();
    void                            high_service_flap_threshold(float value);
    set_hostdependency const&       hostdependencies() const throw ();
    set_hostdependency&             hostdependencies() throw ();
    set_host const&                 hosts() const throw ();
    set_host&                       hosts() throw ();
    set_host::const_iterator        hosts_find(host::key_type const& k) const;
    set_host::iterator              hosts_find(host::key_type const& k);
    duration const&                 host_check_timeout() const throw ();
    void                            host_check_timeout(duration const& value);
    duration const&                 host_freshness_check_interval() const throw ();
    void                            host_freshness_check_interval(duration const& value);
    std::string const&              illegal_object_chars() const throw ();
    void                            illegal_object_chars(std::string const& value);
    std::string const&              illegal_output_chars() const throw ();
    void                            illegal_output_chars(std::string const& value);
    bool                            log_event_handlers() const throw ();
    void                            log_event_handlers(bool value);
    bool                            log_external_commands() const throw ();
    void                            log_external_commands(bool value);
    std::string const&              log_file() const throw ();
    void                            log_file(std::string const& value);
    bool                            log_host_retries() const throw ();
    void                            log_host_retries(bool value);
    bool                            log_initial_states() const throw ();
    void                            log_initial_states(bool value);
    bool                            log_passive_checks() const throw ();
    void                            log_passive_checks(bool value);
    bool                            log_pid() const throw();
    void                            log_pid(bool value);
    bool                            log_service_retries() const throw ();
    void                            log_service_retries(bool value);
    float                           low_host_flap_threshold() const throw ();
    void                            low_host_flap_threshold(float value);
    float                           low_service_flap_threshold() const throw ();
    void                            low_service_flap_threshold(float value);
    unsigned long                   max_debug_file_size() const throw ();
    void                            max_debug_file_size(unsigned long value);
    unsigned long                   max_log_file_size() const throw ();
    void                            max_log_file_size(unsigned long value);
    unsigned int                    max_parallel_service_checks() const throw ();
    void                            max_parallel_service_checks(unsigned int value);
    bool                            obsess_over_hosts() const throw ();
    void                            obsess_over_hosts(bool value);
    bool                            obsess_over_services() const throw ();
    void                            obsess_over_services(bool value);
    std::string const&              ochp_command() const throw ();
    void                            ochp_command(std::string const& value);
    duration const&                 ochp_timeout() const throw ();
    void                            ochp_timeout(duration const& value);
    std::string const&              ocsp_command() const throw ();
    void                            ocsp_command(std::string const& value);
    duration const&                 ocsp_timeout() const throw ();
    void                            ocsp_timeout(duration const& value);
    duration const&                 retention_update_interval() const throw ();
    void                            retention_update_interval(duration const& value);
    set_servicedependency const&    servicedependencies() const throw ();
    set_servicedependency&          servicedependencies() throw ();
    set_service const&              services() const throw ();
    set_service&                    services() throw ();
    set_service::const_iterator     services_find(service::key_type const& k) const;
    set_service::iterator           services_find(service::key_type const& k);
    duration const&                 service_check_timeout() const throw ();
    void                            service_check_timeout(duration const& value);
    duration const&                 service_freshness_check_interval() const throw ();
    void                            service_freshness_check_interval(duration const& value);
    float                           sleep_time() const throw ();
    void                            sleep_time(float value);
    bool                            soft_state_dependencies() const throw ();
    void                            soft_state_dependencies(bool value);
    std::string const&              state_retention_file() const throw ();
    void                            state_retention_file(std::string const& value);
    std::string const&              status_file() const throw ();
    void                            status_file(std::string const& value);
    bool                            set(char const* key, char const* value);
    set_timeperiod const&           timeperiods() const throw ();
    set_timeperiod&                 timeperiods() throw ();
    set_timeperiod::const_iterator  timeperiods_find(timeperiod::key_type const& k) const;
    set_timeperiod::iterator        timeperiods_find(timeperiod::key_type const& k);
    duration const&                 time_change_threshold() const throw ();
    void                            time_change_threshold(duration const& value);
    std::vector<std::string> const& user() const throw ();
    void                            user(std::vector<std::string> const& value);
    void                            user(std::string const& key, std::string const& value);
    void                            user(unsigned int key, std::string const& value);
    bool                            use_setpgid() const throw ();
    void                            use_setpgid(bool value);
    bool                            use_syslog() const throw ();
    void                            use_syslog(bool value);
    std::string const&              use_timezone() const throw ();
    void                            use_timezone(std::string const& value);

  private:
    struct                          setters {
      char const*                   name;
      bool(*                        func)(state&, char const*, char const*);
    };

    template <typename U, void (state::* ptr)(U)>
    struct                          setter {
      static bool                   generic(state& obj, char const* variable, char const* value) {
        try {
          U val(0);
          if (!string::to(value, val))
            return (false);
          (obj.*ptr)(val);
        }
        catch (std::exception const& e) {
          logger(logging::log_config_error, logging::basic)
            << "Error while processing variable '"
            << variable << "': " << e.what();
          return (false);
        }
        return (true);
      }
    };

    template <void (state::* ptr)(std::string const&)>
    struct                          setter<std::string const&, ptr> {
      static bool                   generic(state& obj, char const* variable, char const* value) {
        try {
          (obj.*ptr)(value);
        }
        catch (std::exception const& e) {
          logger(logging::log_config_error, logging::basic)
            << "Error while processing variable '"
            << variable << "': " << e.what();
          return (false);
        }
        return (true);
      }
    };

    template <void (state::* ptr)(duration const&)>
    struct                          setter<duration const&, ptr> {
      static bool                   generic(state& obj, char const* variable, char const* value) {
        try {
          duration d;
          d.set(value);
          (obj.*ptr)(d);
        }
        catch (std::exception const& e) {
          logger(logging::log_config_error, logging::basic)
            << "Error while processing variable '"
            << variable << "': " << e.what();
          return (false);
        }
        return (true);
      }
    };

    void                            _set_broker_module(std::string const& value);
    void                            _set_cfg_dir(std::string const& value);
    void                            _set_cfg_file(std::string const& value);
    void                            _set_cfg_include(std::string const& value);
    void                            _set_event_broker_options(std::string const& value);

    duration                        _additional_freshness_latency;
    std::list<std::string>          _broker_module;
    std::string                     _broker_module_directory;
    duration                        _cached_host_check_horizon;
    duration                        _cached_service_check_horizon;
    std::list<std::string>          _cfg_dir;
    std::list<std::string>          _cfg_file;
    std::list<std::string>          _cfg_include;
    std::string                     _cfg_main;
    bool                            _check_host_freshness;
    duration                        _check_reaper_interval;
    bool                            _check_service_freshness;
    set_command                     _commands;
    duration                        _command_check_interval;
    std::string                     _command_file;
    set_connector                   _connectors;
    std::string                     _debug_file;
    unsigned long long              _debug_level;
    unsigned int                    _debug_verbosity;
    bool                            _enable_event_handlers;
    bool                            _enable_flap_detection;
    bool                            _enable_predictive_host_dependency_checks;
    bool                            _enable_predictive_service_dependency_checks;
    unsigned long                   _event_broker_options;
    duration                        _event_handler_timeout;
    int                             _external_command_buffer_slots;
    std::string                     _global_host_event_handler;
    std::string                     _global_service_event_handler;
    float                           _high_host_flap_threshold;
    float                           _high_service_flap_threshold;
    set_hostdependency              _hostdependencies;
    set_host                        _hosts;
    duration                        _host_check_timeout;
    duration                        _host_freshness_check_interval;
    std::string                     _illegal_object_chars;
    std::string                     _illegal_output_chars;
    bool                            _log_event_handlers;
    bool                            _log_external_commands;
    std::string                     _log_file;
    bool                            _log_host_retries;
    bool                            _log_initial_states;
    bool                            _log_passive_checks;
    bool                            _log_pid;
    bool                            _log_service_retries;
    float                           _low_host_flap_threshold;
    float                           _low_service_flap_threshold;
    unsigned long                   _max_debug_file_size;
    unsigned long                   _max_log_file_size;
    unsigned int                    _max_parallel_service_checks;
    bool                            _obsess_over_hosts;
    bool                            _obsess_over_services;
    std::string                     _ochp_command;
    duration                        _ochp_timeout;
    std::string                     _ocsp_command;
    duration                        _ocsp_timeout;
    duration                        _retention_update_interval;
    set_servicedependency           _servicedependencies;
    set_service                     _services;
    duration                        _service_check_timeout;
    duration                        _service_freshness_check_interval;
    float                           _sleep_time;
    bool                            _soft_state_dependencies;
    std::string                     _state_retention_file;
    std::string                     _status_file;
    set_timeperiod                  _timeperiods;
    duration                        _time_change_threshold;
    std::vector<std::string>        _users;
    bool                            _use_setpgid;
    bool                            _use_syslog;
    std::string                     _use_timezone;

    static setters const            _setters[];
    static char const* const        _deprecated[][2];
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_STATE_HH

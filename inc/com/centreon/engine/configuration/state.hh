/*
** Copyright 2011-2012 Merethis
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

#  include <map>
#  include <QString>
#  include <sstream>
#  include <string>
#  include "com/centreon/engine/error.hh"

// Forward declaration.
struct nagios_macros;

namespace                   com {
  namespace                 centreon {
    namespace               engine {
      namespace             configuration {
	/**
	 *  @class state state.hh
	 *  @brief Simple configuration state class.
	 *
	 *  Simple configuration state class used by Centreon Engine
	 *  to manage configuration data.
	 */
	class                 state {
	public:
	  /**
	   *  @enum state::e_date_format
	   *  Date format types
	   */
	  enum                e_date_format {
	    us = 0,             // U.S. (MM-DD-YYYY HH:MM:SS)
	    euro,               // European (DD-MM-YYYY HH:MM:SS)
	    iso8601,            // ISO8601 (YYYY-MM-DD HH:MM:SS)
	    strict_iso8601      // ISO8601 (YYYY-MM-DDTHH:MM:SS)
	  };

	  /**
	   *  @enum state::e_inter_check_delay
	   *  Inter-check delay calculation types
	   */
	  enum                e_inter_check_delay {
	    icd_none = 0,       // no inter-check delay
	    icd_dumb,           // dumb delay of 1 second
	    icd_smart,          // smart delay
	    icd_user            // user-specified delay
	  };

	  /**
	   *  @enum state::e_interleave_factor
	   *  Interleave factor calculation types
	   */
	  enum                e_interleave_factor {
	    ilf_user = 0,       // user-specified interleave factor
	    ilf_smart           // smart interleave
	  };

	                      state();
	                      state(state const& right);
	                      ~state() throw();

	  state&              operator=(state const& right);

	  void                reset();
	  void                parse(QString const& filename);

	  QString const&      get_log_file() const throw();
	  QString const&      get_broker_module_directory() const throw();
	  QString const&      get_debug_file() const throw();
	  QString const&      get_command_file() const throw();
	  QString const&      get_global_host_event_handler() const throw();
	  QString const&      get_global_service_event_handler() const throw();
	  QString const&      get_ocsp_command() const throw();
	  QString const&      get_ochp_command() const throw();
	  QString const&      get_illegal_object_chars() const throw();
	  QString const&      get_illegal_output_chars() const throw();
	  QString const&      get_use_timezone() const throw();
	  int                 get_additional_freshness_latency() const throw();
	  unsigned long       get_debug_level() const throw();
	  unsigned int        get_debug_verbosity() const throw();
	  int                 get_command_check_interval() const throw();
	  int                 get_external_command_buffer_slots() const throw();
	  unsigned int        get_max_service_check_spread() const throw();
	  unsigned int        get_max_host_check_spread() const throw();
	  unsigned int        get_max_parallel_service_checks() const throw();
	  unsigned int        get_check_reaper_interval() const throw();
	  unsigned int        get_max_check_reaper_time() const throw();
	  unsigned int        get_interval_length() const throw();
	  unsigned int        get_service_freshness_check_interval() const throw();
	  unsigned int        get_host_freshness_check_interval() const throw();
	  unsigned int        get_auto_rescheduling_interval() const throw();
	  unsigned int        get_auto_rescheduling_window() const throw();
	  unsigned int        get_status_update_interval() const throw();
	  unsigned int        get_time_change_threshold() const throw();
	  unsigned int        get_retention_update_interval() const throw();
	  unsigned int        get_retention_scheduling_horizon() const throw();
	  unsigned int        get_service_check_timeout() const throw();
	  unsigned int        get_host_check_timeout() const throw();
	  unsigned int        get_event_handler_timeout() const throw();
	  unsigned int        get_notification_timeout() const throw();
	  unsigned int        get_ocsp_timeout() const throw();
	  unsigned int        get_ochp_timeout() const throw();
	  unsigned long       get_max_debug_file_size() const throw();
	  unsigned long       get_max_log_file_size() const throw();
	  unsigned long       get_retained_host_attribute_mask() const throw();
	  unsigned long       get_retained_process_host_attribute_mask() const throw();
	  unsigned long       get_retained_contact_host_attribute_mask() const throw();
	  unsigned long       get_retained_contact_service_attribute_mask() const throw();
	  unsigned long       get_cached_host_check_horizon() const throw();
	  unsigned long       get_cached_service_check_horizon() const throw();
	  unsigned long       get_event_broker_options() const throw();
	  bool                get_use_syslog() const throw();
	  bool                get_log_notifications() const throw();
	  bool                get_log_service_retries() const throw();
	  bool                get_log_host_retries() const throw();
	  bool                get_log_event_handlers() const throw();
	  bool                get_log_external_commands() const throw();
	  bool                get_log_passive_checks() const throw();
	  bool                get_log_initial_state() const throw();
	  bool                get_retain_state_information() const throw();
	  bool                get_use_retained_program_state() const throw();
	  bool                get_use_retained_scheduling_info() const throw();
	  bool                get_obsess_over_services() const throw();
	  bool                get_obsess_over_hosts() const throw();
	  bool                get_translate_passive_host_checks() const throw();
	  bool                get_passive_host_checks_are_soft() const throw();
	  bool                get_use_aggressive_host_checking() const throw();
	  bool                get_enable_predictive_host_dependency_checks() const throw();
	  bool                get_enable_predictive_service_dependency_checks() const throw();
	  bool                get_soft_state_dependencies() const throw();
	  bool                get_enable_event_handlers() const throw();
	  bool                get_enable_notifications() const throw();
	  bool                get_execute_service_checks() const throw();
	  bool                get_accept_passive_service_checks() const throw();
	  bool                get_execute_host_checks() const throw();
	  bool                get_accept_passive_host_checks() const throw();
	  bool                get_check_external_commands() const throw();
	  bool                get_check_orphaned_services() const throw();
	  bool                get_check_orphaned_hosts() const throw();
	  bool                get_check_service_freshness() const throw();
	  bool                get_check_host_freshness() const throw();
	  bool                get_auto_reschedule_checks() const throw();
	  bool                get_process_performance_data() const throw();
	  bool                get_enable_flap_detection() const throw();
	  bool                get_enable_failure_prediction() const throw();
	  bool                get_use_regexp_matches() const throw();
	  bool                get_use_true_regexp_matching() const throw();
	  bool                get_use_large_installation_tweaks() const throw();
	  bool                get_enable_environment_macros() const throw();
	  bool                get_free_child_process_memory() const throw();
	  bool                get_child_processes_fork_twice() const throw();
	  bool                get_allow_empty_hostgroup_assignment() const throw();
	  float               get_sleep_time() const throw();
	  float               get_low_service_flap_threshold() const throw();
	  float               get_high_service_flap_threshold() const throw();
	  float               get_low_host_flap_threshold() const throw();
	  float               get_high_host_flap_threshold() const throw();
	  e_date_format       get_date_format() const throw();
	  e_inter_check_delay get_service_inter_check_delay_method() const throw();
	  e_inter_check_delay get_host_inter_check_delay_method() const throw();
	  e_interleave_factor get_service_interleave_factor_method() const throw();

	  void                set_log_file(QString const& value);
	  void                set_broker_module_directory(QString const& value);
	  void                set_debug_file(QString const& value);
	  void                set_command_file(QString const& value);
	  void                set_temp_file(QString const& value);
	  void                set_temp_path(QString const& value);
	  void                set_check_result_path(QString const& value);
	  void                set_global_host_event_handler(QString const& value);
	  void                set_global_service_event_handler(QString const& value);
	  void                set_ocsp_command(QString const& value);
	  void                set_ochp_command(QString const& value);
	  void                set_log_archive_path(QString const& value);
	  void                set_p1_file(QString const& value);
	  void                set_illegal_object_chars(QString const& value);
	  void                set_illegal_output_chars(QString const& value);
	  void                set_use_timezone(QString const& value);
	  void                set_additional_freshness_latency(int value);
	  void                set_debug_level(unsigned long value);
	  void                set_debug_verbosity(unsigned int value);
	  void                set_command_check_interval(int value);
	  void                set_command_check_interval(int value, bool is_second);
	  void                set_command_check_interval(QString const& value);
	  void                set_external_command_buffer_slots(int value);
	  void                set_max_service_check_spread(unsigned int value);
	  void                set_max_host_check_spread(unsigned int value);
	  void                set_max_parallel_service_checks(unsigned int value);
	  void                set_check_reaper_interval(unsigned int value);
	  void                set_max_check_reaper_time(unsigned int value);
	  void                set_interval_length(unsigned int value);
	  void                set_service_freshness_check_interval(unsigned int value);
	  void                set_host_freshness_check_interval(unsigned int value);
	  void                set_auto_rescheduling_interval(unsigned int value);
	  void                set_auto_rescheduling_window(unsigned int value);
	  void                set_status_update_interval(unsigned int value);
	  void                set_time_change_threshold(unsigned int value);
	  void                set_retention_update_interval(unsigned int value);
	  void                set_retention_scheduling_horizon(unsigned int value);
	  void                set_service_check_timeout(unsigned int value);
	  void                set_host_check_timeout(unsigned int value);
	  void                set_event_handler_timeout(unsigned int value);
	  void                set_notification_timeout(unsigned int value);
	  void                set_ocsp_timeout(unsigned int value);
	  void                set_ochp_timeout(unsigned int value);
	  void                set_max_debug_file_size(unsigned long value);
	  void                set_max_log_file_size(unsigned long value);
	  void                set_max_check_result_file_age(unsigned long value);
	  void                set_retained_host_attribute_mask(unsigned long value);
	  void                set_retained_process_host_attribute_mask(unsigned long value);
	  void                set_retained_contact_host_attribute_mask(unsigned long value);
	  void                set_retained_contact_service_attribute_mask(unsigned long value);
	  void                set_cached_host_check_horizon(unsigned long value);
	  void                set_cached_service_check_horizon(unsigned long value);
	  void                set_event_broker_options(unsigned long value);
	  void                set_event_broker_options(QString const& value);
	  void                set_use_syslog(bool value);
	  void                set_log_notifications(bool value);
	  void                set_log_service_retries(bool value);
	  void                set_log_host_retries(bool value);
	  void                set_log_event_handlers(bool value);
	  void                set_log_external_commands(bool value);
	  void                set_log_passive_checks(bool value);
	  void                set_log_initial_state(bool value);
	  void                set_retain_state_information(bool value);
	  void                set_use_retained_program_state(bool value);
	  void                set_use_retained_scheduling_info(bool value);
	  void                set_obsess_over_services(bool value);
	  void                set_obsess_over_hosts(bool value);
	  void                set_translate_passive_host_checks(bool value);
	  void                set_passive_host_checks_are_soft(bool value);
	  void                set_use_aggressive_host_checking(bool value);
	  void                set_enable_predictive_host_dependency_checks(bool value);
	  void                set_enable_predictive_service_dependency_checks(bool value);
	  void                set_soft_state_dependencies(bool value);
	  void                set_enable_event_handlers(bool value);
	  void                set_enable_notifications(bool value);
	  void                set_execute_service_checks(bool value);
	  void                set_accept_passive_service_checks(bool value);
	  void                set_execute_host_checks(bool value);
	  void                set_accept_passive_host_checks(bool value);
	  void                set_check_external_commands(bool value);
	  void                set_check_orphaned_services(bool value);
	  void                set_check_orphaned_hosts(bool value);
	  void                set_check_service_freshness(bool value);
	  void                set_check_host_freshness(bool value);
	  void                set_auto_reschedule_checks(bool value);
	  void                set_process_performance_data(bool value);
	  void                set_enable_flap_detection(bool value);
	  void                set_enable_failure_prediction(bool value);
	  void                set_use_regexp_matches(bool value);
	  void                set_use_true_regexp_matching(bool value);
	  void                set_use_large_installation_tweaks(bool value);
	  void                set_enable_environment_macros(bool value);
	  void                set_free_child_process_memory(bool value);
	  void                set_child_processes_fork_twice(bool value);
	  void                set_enable_embedded_perl(bool value);
	  void                set_use_embedded_perl_implicitly(bool value);
	  void                set_allow_empty_hostgroup_assignment(bool value);
	  void                set_sleep_time(float value);
	  void                set_low_service_flap_threshold(float value);
	  void                set_high_service_flap_threshold(float value);
	  void                set_low_host_flap_threshold(float value);
	  void                set_high_host_flap_threshold(float value);
	  void                set_date_format(e_date_format value);
	  void                set_date_format(QString const& value);
	  void                set_log_rotation_method(QString const& value);
	  void                set_service_inter_check_delay_method(e_inter_check_delay value);
	  void                set_service_inter_check_delay_method(QString const& value);
	  void                set_host_inter_check_delay_method(e_inter_check_delay value);
	  void                set_host_inter_check_delay_method(QString const& value);
	  void                set_service_interleave_factor_method(e_interleave_factor value);
	  void                set_service_interleave_factor_method(QString const& value);

	private:
	  /**
	   *  @enum state::e_var_string
	   *  List all string variable
	   */
	  enum                e_var_string {
	    log_file = 0,
	    broker_module_directory,
	    debug_file,
	    command_file,
	    global_host_event_handler,
	    global_service_event_handler,
	    ocsp_command,
	    ochp_command,
	    illegal_object_chars,
	    illegal_output_chars,
	    use_timezone,
	    max_string
	  };

	  /**
	   *  @enum state::e_var_string
	   *  List all unsigned long variable
	   */
	  enum                e_var_ulong {
	    debug_level = 0,
	    max_debug_file_size,
	    max_log_file_size,
	    retained_host_attribute_mask,
	    retained_process_host_attribute_mask,
	    retained_contact_host_attribute_mask,
	    retained_contact_service_attribute_mask,
	    cached_host_check_horizon,
	    cached_service_check_horizon,
	    event_broker_options,
	    max_ulong
	  };

	  /**
	   *  @enum state::e_var_string
	   *  List all float variable
	   */
	  enum                e_var_float {
	    sleep_time = 0,
	    low_service_flap_threshold,
	    high_service_flap_threshold,
	    low_host_flap_threshold,
	    high_host_flap_threshold,
	    max_float
	  };

	  /**
	   *  @enum state::e_var_string
	   *  List all int variable
	   */
	  enum                e_var_int {
	    additional_freshness_latency = 0,
	    command_check_interval,
	    external_command_buffer_slots,
	    free_child_process_memory,
	    child_processes_fork_twice,
	    max_int
	  };

	  /**
	   *  @enum state::e_var_string
	   *  List all unsigned int variable
	   */
	  enum                e_var_uint {
	    debug_verbosity = 0,
	    max_service_check_spread,
	    max_host_check_spread,
	    max_parallel_service_checks,
	    check_reaper_interval,
	    max_check_reaper_time,
	    interval_length,
	    service_freshness_check_interval,
	    host_freshness_check_interval,
	    auto_rescheduling_interval,
	    auto_rescheduling_window,
	    status_update_interval,
	    time_change_threshold,
	    retention_update_interval,
	    retention_scheduling_horizon,
	    service_check_timeout,
	    host_check_timeout,
	    event_handler_timeout,
	    notification_timeout,
	    ocsp_timeout,
	    ochp_timeout,
	    date_format,
	    service_inter_check_delay_method,
	    host_inter_check_delay_method,
	    service_interleave_factor_method,
	    max_uint
	  };

	  /**
	   *  @enum state::e_var_string
	   *  List all bool variable
	   */
	  enum                e_var_bool {
	    use_syslog = 0,
	    log_notifications,
	    log_service_retries,
	    log_host_retries,
	    log_event_handlers,
	    log_external_commands,
	    log_passive_checks,
	    log_initial_state,
	    retain_state_information,
	    use_retained_program_state,
	    use_retained_scheduling_info,
	    obsess_over_services,
	    obsess_over_hosts,
	    translate_passive_host_checks,
	    passive_host_checks_are_soft,
	    use_aggressive_host_checking,
	    enable_predictive_host_dependency_checks,
	    enable_predictive_service_dependency_checks,
	    soft_state_dependencies,
	    enable_event_handlers,
	    enable_notifications,
	    execute_service_checks,
	    accept_passive_service_checks,
	    execute_host_checks,
	    accept_passive_host_checks,
	    check_external_commands,
	    check_orphaned_services,
	    check_orphaned_hosts,
	    check_service_freshness,
	    check_host_freshness,
	    auto_reschedule_checks,
	    process_performance_data,
	    enable_flap_detection,
	    enable_failure_prediction,
	    use_regexp_matches,
	    use_true_regexp_matching,
	    use_large_installation_tweaks,
	    enable_environment_macros,
	    allow_empty_hostgroup_assignment,
	    max_bool
	  };

	  void                _reset();
	  void                _parse_resource_file(QString const& value);

	  void                _set_auth_file(QString const& value);
	  void                _set_admin_email(QString const& value);
	  void                _set_admin_pager(QString const& value);
	  void                _set_retained_service_attribute_mask(QString const& value);
	  void                _set_retained_process_service_attribute_mask(QString const& value);
	  void                _set_max_service_check_spread(QString const& value);
	  void                _set_aggregate_status_updates(QString const& value);
	  void                _set_broker_module(QString const& value);
	  void                _set_bare_update_check(QString const& value);
	  void                _set_check_for_updates(QString const& value);
	  void                _set_comment_file(QString const& value);
	  void                _set_daemon_dumps_core(QString const& value);
	  void                _set_downtime_file(QString const& value);
	  void                _set_lock_file(QString const& value);
	  void                _set_user(QString const& value);
	  void                _set_group(QString const& value);

	  static std::string  _getline(std::ifstream& ifs) throw();
	  static std::string& _trim(std::string& str) throw();

	  template<class T>
	  static bool         _str2obj(QString const& value, T* ret) {
	    std::istringstream iss(value.toStdString());
	    if (!(iss >> *ret) || !iss.eof()) {
	      return (false);
	    }
	    return (true);
	  }

	  template<class T, void (state::*ptr)(T)>
	  struct              cpp_suck {
	    static bool       set_generic(QString const& value, state& config) {
	      T ret;
	      if (_str2obj(value, &ret) == false) {
		return (false);
	      }
	      (config.*ptr)(ret);
	      return (true);
	    }
	  };

	  template<void (state::*ptr)(QString const&)>
	  struct              cpp_suck<QString const&, ptr> {
	    static bool       set_generic(QString const& value, state& config) {
	      (config.*ptr)(value);
	      return (true);
	    }
	  };

	  template<class T, class U, void (state::*ptr)(T)>
	  struct              cpp_suck_cast {
	    static bool       set_generic_cast(QString const& value, state& config) {
	      U ret;
	      if (_str2obj(value, &ret) == false) {
		return (false);
	      }
	      (config.*ptr)(static_cast<T>(ret));
	      return (true);
	    }
	  };

	  typedef std::map<QString, bool (*)(QString const&, state& config)> methods;

	  QString             _tab_string[max_string];
	  unsigned long       _tab_ulong[max_ulong];
	  float               _tab_float[max_float];
	  int                 _tab_int[max_int];
	  unsigned int        _tab_uint[max_uint];
	  bool                _tab_bool[max_bool];

	  QString             _filename;
	  methods             _lst_method;
	  unsigned int        _cur_line;
	  bool                _command_check_interval_is_seconds;
	  nagios_macros       *_mac;

	private:
	  static const int                 DEFAULT_ADDITIONAL_FRESHNESS_LATENCY                = 15;
	  static const unsigned long       DEFAULT_DEBUG_LEVEL                                 = 0;
	  static const int                 DEFAULT_DEBUG_VERBOSITY                             = 1;
	  static const int                 DEFAULT_COMMAND_CHECK_INTERVAL                      = -1;
	  static const int                 DEFAULT_EXTERNAL_COMMAND_BUFFER_SLOTS               = 4096;
	  static const int                 DEFAULT_FREE_CHILD_PROCESS_MEMORY                   = -1;
	  static const int                 DEFAULT_CHILD_PROCESSES_FORK_TWICE                  = -1;
	  static const unsigned int        DEFAULT_SERVICE_CHECK_SPREAD                        = 30;
	  static const unsigned int        DEFAULT_HOST_CHECK_SPREAD                           = 30;
	  static const unsigned int        DEFAULT_MAX_PARALLEL_SERVICE_CHECKS                 = 0;
	  static const unsigned int        DEFAULT_CHECK_REAPER_INTERVAL                       = 10;
	  static const unsigned int        DEFAULT_MAX_REAPER_TIME                             = 30;
	  static const unsigned int        DEFAULT_INTERVAL_LENGTH                             = 60;
	  static const unsigned int        DEFAULT_FRESHNESS_CHECK_INTERVAL                    = 60;
	  static const unsigned int        DEFAULT_AUTO_RESCHEDULING_INTERVAL                  = 30;
	  static const unsigned int        DEFAULT_AUTO_RESCHEDULING_WINDOW                    = 180;
	  static const unsigned int        DEFAULT_STATUS_UPDATE_INTERVAL                      = 60;
	  static const unsigned int        DEFAULT_TIME_CHANGE_THRESHOLD                       = 900;
	  static const unsigned int        DEFAULT_RETENTION_UPDATE_INTERVAL                   = 60;
	  static const unsigned int        DEFAULT_RETENTION_SCHEDULING_HORIZON                = 900;
	  static const unsigned int        DEFAULT_SERVICE_CHECK_TIMEOUT                       = 60;
	  static const unsigned int        DEFAULT_HOST_CHECK_TIMEOUT                          = 30;
	  static const unsigned int        DEFAULT_EVENT_HANDLER_TIMEOUT                       = 30;
	  static const unsigned int        DEFAULT_NOTIFICATION_TIMEOUT                        = 30;
	  static const unsigned int        DEFAULT_OCSP_TIMEOUT                                = 15;
	  static const unsigned int        DEFAULT_OCHP_TIMEOUT                                = 15;
	  static const unsigned int        DEFAULT_LOG_ROTATION                                = 0;
	  static const e_date_format       DEFAULT_DATE_FORMAT                                 = us;
	  static const e_inter_check_delay DEFAULT_SERVICE_INTER_CHECK_DELAY_METHOD            = icd_smart;
	  static const e_inter_check_delay DEFAULT_HOST_INTER_CHECK_DELAY_METHOD               = icd_smart;
	  static const e_interleave_factor DEFAULT_SERVICE_INTERLEAVE_FACTOR_METHOD            = ilf_smart;
	  static const unsigned long       DEFAULT_MAX_DEBUG_FILE_SIZE                         = 1000000;
	  static const unsigned long       DEFAULT_MAX_LOG_FILE_SIZE                           = 0;
	  static const unsigned long       DEFAULT_RETAINED_HOST_ATTRIBUTE_MASK                = 0L;
	  static const unsigned long       DEFAULT_RETAINED_PROCESS_HOST_ATTRIBUTE_MASK        = 0L;
	  static const unsigned long       DEFAULT_RETAINED_CONTACT_HOST_ATTRIBUTE_MASK        = 0L;
	  static const unsigned long       DEFAULT_RETAINED_CONTACT_SERVICE_ATTRIBUTE_MASK     = 0L;
	  static const unsigned long       DEFAULT_CACHED_HOST_CHECK_HORIZON                   = 15;
	  static const unsigned long       DEFAULT_CACHED_SERVICE_CHECK_HORIZON                = 15;
	  static const unsigned long       DEFAULT_EVENT_BROKER_OPTIONS                        = 0;
	  static const bool                DEFAULT_USE_SYSLOG                                  = true;
	  static const bool                DEFAULT_NOTIFICATION_LOGGING                        = true;
	  static const bool                DEFAULT_LOG_SERVICE_RETRIES                         = false;
	  static const bool                DEFAULT_LOG_HOST_RETRIES                            = false;
	  static const bool                DEFAULT_LOG_EVENT_HANDLERS                          = true;
	  static const bool                DEFAULT_LOG_EXTERNAL_COMMANDS                       = true;
	  static const bool                DEFAULT_LOG_PASSIVE_CHECKS                          = true;
	  static const bool                DEFAULT_LOG_INITIAL_STATE                          = false;
	  static const bool                DEFAULT_RETAIN_STATE_INFORMATION                    = false;
	  static const bool                DEFAULT_USE_RETAINED_PROGRAM_STATE                  = true;
	  static const bool                DEFAULT_USE_RETAINED_SCHEDULING_INFO                = false;
	  static const bool                DEFAULT_OBSESS_OVER_SERVICES                        = false;
	  static const bool                DEFAULT_OBSESS_OVER_HOSTS                           = false;
	  static const bool                DEFAULT_TRANSLATE_PASSIVE_HOST_CHECKS               = false;
	  static const bool                DEFAULT_PASSIVE_HOST_CHECKS_SOFT                    = false;
	  static const bool                DEFAULT_AGGRESSIVE_HOST_CHECKING                    = false;
	  static const bool                DEFAULT_ENABLE_PREDICTIVE_HOST_DEPENDENCY_CHECKS    = true;
	  static const bool                DEFAULT_ENABLE_PREDICTIVE_SERVICE_DEPENDENCY_CHECKS = true;
	  static const bool                DEFAULT_SOFT_STATE_DEPENDENCIES                     = false;
	  static const bool                DEFAULT_ENABLE_EVENT_HANDLERS                       = true;
	  static const bool                DEFAULT_ENABLE_NOTIFICATIONS                        = true;
	  static const bool                DEFAULT_EXECUTE_SERVICE_CHECKS                      = true;
	  static const bool                DEFAULT_ACCEPT_PASSIVE_SERVICE_CHECKS               = true;
	  static const bool                DEFAULT_EXECUTE_HOST_CHECKS                         = true;
	  static const bool                DEFAULT_ACCEPT_PASSIVE_HOST_CHECKS                  = true;
	  static const bool                DEFAULT_CHECK_EXTERNAL_COMMANDS                     = true;
	  static const bool                DEFAULT_CHECK_ORPHANED_SERVICES                     = true;
	  static const bool                DEFAULT_CHECK_ORPHANED_HOSTS                        = true;
	  static const bool                DEFAULT_CHECK_SERVICE_FRESHNESS                     = true;
	  static const bool                DEFAULT_CHECK_HOST_FRESHNESS                        = false;
	  static const bool                DEFAULT_AUTO_RESCHEDULE_CHECKS                      = false;
	  static const bool                DEFAULT_PROCESS_PERFORMANCE_DATA                    = false;
	  static const bool                DEFAULT_ENABLE_FLAP_DETECTION                       = false;
	  static const bool                DEFAULT_ENABLE_FAILURE_PREDICTION                   = true;
	  static const bool                DEFAULT_USE_REGEXP_MATCHES                          = false;
	  static const bool                DEFAULT_USE_TRUE_REGEXP_MATCHING                    = false;
	  static const bool                DEFAULT_USE_LARGE_INSTALLATION_TWEAKS               = false;
	  static const bool                DEFAULT_ENABLE_ENVIRONMENT_MACROS                   = false;
	  static const bool                DEFAULT_ALLOW_EMPTY_HOSTGROUP_ASSIGNMENT            = false;
	  static const float               DEFAULT_SLEEP_TIME;
	  static const float               DEFAULT_LOW_SERVICE_FLAP_THRESHOLD;
	  static const float               DEFAULT_HIGH_SERVICE_FLAP_THRESHOLD;
	  static const float               DEFAULT_LOW_HOST_FLAP_THRESHOLD;
	  static const float               DEFAULT_HIGH_HOST_FLAP_THRESHOLD;
          static const char * const        DEFAULT_ILLEGAL_OUTPUT_CHARS;
	};
      }
    }
  }
}

#endif // !CCE_CONFIGURATION_STATE_HH

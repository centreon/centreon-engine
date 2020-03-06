/*
** Copyright 2011-2020 Centreon
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

#include <unistd.h>

#include <array>
#include <cassert>
#include <unordered_map>

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/commands/connector.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/connector.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/globals.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/hostdependency.hh"
#include "com/centreon/engine/configuration/applier/hostescalation.hh"
#include "com/centreon/engine/configuration/applier/hostgroup.hh"
#include "com/centreon/engine/configuration/applier/logging.hh"
#include "com/centreon/engine/configuration/applier/macros.hh"
#include "com/centreon/engine/configuration/applier/scheduler.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/servicedependency.hh"
#include "com/centreon/engine/configuration/applier/serviceescalation.hh"
#include "com/centreon/engine/configuration/applier/servicegroup.hh"
#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/command.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/retention/applier/state.hh"
#include "com/centreon/engine/retention/state.hh"
#include "com/centreon/engine/version.hh"
#include "com/centreon/engine/xpddefault.hh"
#include "com/centreon/engine/xsddefault.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::logging;

static bool has_already_been_loaded(false);

/**
 *  Apply new configuration.
 *
 *  @param[in] new_cfg        The new configuration.
 *  @param[in] waiting_thread True to wait thread after calulate differencies.
 */
void applier::state::apply(configuration::state& new_cfg) {
  configuration::state save(*config);
  try {
    _processing_state = state_ready;
    _processing(new_cfg);
  } catch (std::exception const& e) {
    // If is the first time to load configuration, we don't
    // have a valid configuration to restore.
    if (!has_already_been_loaded)
      throw;

    // If is not the first time, we can restore the old one.
    logger(log_config_error, basic)
        << "Error: Could not apply new configuration: " << e.what();

    // Check if we need to restore old configuration.
    if (_processing_state == state_error) {
      logger(dbg_config, more)
          << "configuration: try to restore old configuration";
      _processing(save);
    }
  }
}

/**
 *  Apply new configuration.
 *
 *  @param[in] new_cfg        The new configuration.
 *  @param[in] state          The retention to use.
 */
void applier::state::apply(configuration::state& new_cfg,
                           retention::state& state) {
  configuration::state save(*config);
  try {
    _processing_state = state_ready;
    _processing(new_cfg, &state);
  } catch (std::exception const& e) {
    // If is the first time to load configuration, we don't
    // have a valid configuration to restore.
    if (!has_already_been_loaded)
      throw;

    // If is not the first time, we can restore the old one.
    logger(log_config_error, basic)
        << "Cannot apply new configuration: " << e.what();

    // Check if we need to restore old configuration.
    if (_processing_state == state_error) {
      logger(dbg_config, more)
          << "configuration: try to restore old configuration";
      _processing(save, &state);
    }
  }
}

/**
 *  Get the singleton instance of state applier.
 *
 *  @return Singleton instance.
 */
applier::state& applier::state::instance() {
  static applier::state instance;
  return instance;
}

void applier::state::clear() {
  engine::contact::contacts.clear();
  engine::contactgroup::contactgroups.clear();
  engine::servicegroup::servicegroups.clear();
  engine::hostgroup::hostgroups.clear();
  engine::commands::command::commands.clear();
  engine::commands::connector::connectors.clear();
  engine::service::services.clear();
  engine::service::services_by_id.clear();
  engine::servicedependency::servicedependencies.clear();
  engine::serviceescalation::serviceescalations.clear();
  engine::host::hosts.clear();
  engine::host::hosts_by_id.clear();
  engine::hostdependency::hostdependencies.clear();
  engine::hostescalation::hostescalations.clear();
  engine::timeperiod::timeperiods.clear();
  engine::comment::comments.clear();
  engine::comment::set_next_comment_id(1llu);

  xpddefault_cleanup_performance_data();

  applier::scheduler::instance().clear();
  applier::macros::instance().clear();
  applier::globals::instance().clear();
  applier::logging::instance().clear();

  _processing_state = state_ready;
  _config = nullptr;
}

/**
 *  Default constructor.
 */
applier::state::state() : _config(nullptr), _processing_state(state_ready) {}

/**
 *  Destructor.
 */
applier::state::~state() throw() {
  engine::contact::contacts.clear();
  engine::contactgroup::contactgroups.clear();
  engine::servicegroup::servicegroups.clear();
  engine::hostgroup::hostgroups.clear();
  engine::commands::command::commands.clear();
  engine::commands::connector::connectors.clear();
  engine::service::services.clear();
  engine::service::services_by_id.clear();
  engine::servicedependency::servicedependencies.clear();
  engine::serviceescalation::serviceescalations.clear();
  engine::host::hosts.clear();
  engine::host::hosts_by_id.clear();
  engine::hostdependency::hostdependencies.clear();
  engine::hostescalation::hostescalations.clear();
  engine::timeperiod::timeperiods.clear();
  engine::comment::comments.clear();
  engine::comment::set_next_comment_id(1llu);

  xpddefault_cleanup_performance_data();
}

/**
 *  Return the user macros.
 *
 *  @return  The user macros.
 */
std::unordered_map<std::string, std::string>& applier::state::user_macros() {
  return _user_macros;
}

/**
 *  Find a user macro.
 *
 *  @param[in] key  The key.
 *
 *  @return  Iterator to user macros.
 */
std::unordered_map<std::string, std::string>::const_iterator
applier::state::user_macros_find(std::string const& key) const {
  return _user_macros.find(key);
}

/**
 *  Lock state
 *
 */
void applier::state::lock() {
  _apply_lock.lock();
}

/**
 *  Unlock state
 *
 */
void applier::state::unlock() {
  _apply_lock.unlock();
}

/*
 *  Update all new globals.
 *
 *  @param[in]  new_cfg The new configuration state.
 */
void applier::state::_apply(configuration::state const& new_cfg) {
  // Check variables should not be change after the first execution.
  if (has_already_been_loaded) {
    if (config->broker_module() != new_cfg.broker_module()) {
      logger(log_config_warning, basic)
          << "Warning: Broker modules cannot be changed nor reloaded";
      ++config_warnings;
    }
    if (config->broker_module_directory() !=
        new_cfg.broker_module_directory()) {
      logger(log_config_warning, basic)
          << "Warning: Broker module directory cannot be changed";
      ++config_warnings;
    }
    if (config->command_file() != new_cfg.command_file()) {
      logger(log_config_warning, basic)
          << "Warning: Command file cannot be changed";
      ++config_warnings;
    }
    if (config->external_command_buffer_slots() !=
        new_cfg.external_command_buffer_slots()) {
      logger(log_config_warning, basic)
          << "Warning: External command buffer slots cannot be changed";
      ++config_warnings;
    }
    if (config->use_timezone() != new_cfg.use_timezone()) {
      logger(log_config_warning, basic)
          << "Warning: Timezone can not be changed";
      ++config_warnings;
    }
  }

  // Initialize perfdata if necessary.
  bool modify_perfdata(false);
  if (!has_already_been_loaded ||
      config->host_perfdata_command() != new_cfg.host_perfdata_command() ||
      config->host_perfdata_file() != new_cfg.host_perfdata_file() ||
      config->host_perfdata_file_mode() != new_cfg.host_perfdata_file_mode() ||
      config->host_perfdata_file_processing_command() !=
          new_cfg.host_perfdata_file_processing_command() ||
      config->host_perfdata_file_processing_interval() !=
          new_cfg.host_perfdata_file_processing_interval() ||
      config->host_perfdata_file_template() !=
          new_cfg.host_perfdata_file_template() ||
      config->service_perfdata_command() !=
          new_cfg.service_perfdata_command() ||
      config->service_perfdata_file() != new_cfg.service_perfdata_file() ||
      config->service_perfdata_file_mode() !=
          new_cfg.service_perfdata_file_mode() ||
      config->service_perfdata_file_processing_command() !=
          new_cfg.service_perfdata_file_processing_command() ||
      config->service_perfdata_file_processing_interval() !=
          new_cfg.service_perfdata_file_processing_interval() ||
      config->service_perfdata_file_template() !=
          new_cfg.service_perfdata_file_template())
    modify_perfdata = true;

  // Initialize status file.
  bool modify_status(false);
  if (!has_already_been_loaded ||
      config->status_file() != new_cfg.status_file())
    modify_status = true;

  // Cleanup.
  if (modify_perfdata)
    xpddefault_cleanup_performance_data();
  if (modify_status)
    xsddefault_cleanup_status_data(true);

  // Set new values.
  config->accept_passive_host_checks(new_cfg.accept_passive_host_checks());
  config->accept_passive_service_checks(
      new_cfg.accept_passive_service_checks());
  config->additional_freshness_latency(new_cfg.additional_freshness_latency());
  config->admin_email(new_cfg.admin_email());
  config->admin_pager(new_cfg.admin_pager());
  config->allow_empty_hostgroup_assignment(
      new_cfg.allow_empty_hostgroup_assignment());
  config->auto_reschedule_checks(new_cfg.auto_reschedule_checks());
  config->auto_rescheduling_interval(new_cfg.auto_rescheduling_interval());
  config->auto_rescheduling_window(new_cfg.auto_rescheduling_window());
  config->cached_host_check_horizon(new_cfg.cached_host_check_horizon());
  config->cached_service_check_horizon(new_cfg.cached_service_check_horizon());
  config->cfg_main(new_cfg.cfg_main());
  config->check_external_commands(new_cfg.check_external_commands());
  config->check_host_freshness(new_cfg.check_host_freshness());
  config->check_orphaned_hosts(new_cfg.check_orphaned_hosts());
  config->check_orphaned_services(new_cfg.check_orphaned_services());
  config->check_reaper_interval(new_cfg.check_reaper_interval());
  config->check_service_freshness(new_cfg.check_service_freshness());
  config->command_check_interval(new_cfg.command_check_interval(),
                                 new_cfg.command_check_interval_is_seconds());
  config->date_format(new_cfg.date_format());
  config->debug_file(new_cfg.debug_file());
  config->debug_level(new_cfg.debug_level());
  config->debug_verbosity(new_cfg.debug_verbosity());
  config->enable_environment_macros(new_cfg.enable_environment_macros());
  config->enable_event_handlers(new_cfg.enable_event_handlers());
  config->enable_flap_detection(new_cfg.enable_flap_detection());
  config->enable_notifications(new_cfg.enable_notifications());
  config->enable_predictive_host_dependency_checks(
      new_cfg.enable_predictive_host_dependency_checks());
  config->enable_predictive_service_dependency_checks(
      new_cfg.enable_predictive_service_dependency_checks());
  config->event_broker_options(new_cfg.event_broker_options());
  config->event_handler_timeout(new_cfg.event_handler_timeout());
  config->execute_host_checks(new_cfg.execute_host_checks());
  config->execute_service_checks(new_cfg.execute_service_checks());
  config->global_host_event_handler(new_cfg.global_host_event_handler());
  config->global_service_event_handler(new_cfg.global_service_event_handler());
  config->high_host_flap_threshold(new_cfg.high_host_flap_threshold());
  config->high_service_flap_threshold(new_cfg.high_service_flap_threshold());
  config->host_check_timeout(new_cfg.host_check_timeout());
  config->host_freshness_check_interval(
      new_cfg.host_freshness_check_interval());
  config->host_inter_check_delay_method(
      new_cfg.host_inter_check_delay_method());
  config->host_perfdata_command(new_cfg.host_perfdata_command());
  config->host_perfdata_file(new_cfg.host_perfdata_file());
  config->host_perfdata_file_mode(new_cfg.host_perfdata_file_mode());
  config->host_perfdata_file_processing_command(
      new_cfg.host_perfdata_file_processing_command());
  config->host_perfdata_file_processing_interval(
      new_cfg.host_perfdata_file_processing_interval());
  config->host_perfdata_file_template(new_cfg.host_perfdata_file_template());
  config->illegal_object_chars(new_cfg.illegal_object_chars());
  config->illegal_output_chars(new_cfg.illegal_output_chars());
  config->interval_length(new_cfg.interval_length());
  config->log_event_handlers(new_cfg.log_event_handlers());
  config->log_external_commands(new_cfg.log_external_commands());
  config->log_file(new_cfg.log_file());
  config->log_host_retries(new_cfg.log_host_retries());
  config->log_notifications(new_cfg.log_notifications());
  config->log_passive_checks(new_cfg.log_passive_checks());
  config->log_service_retries(new_cfg.log_service_retries());
  config->low_host_flap_threshold(new_cfg.low_host_flap_threshold());
  config->low_service_flap_threshold(new_cfg.low_service_flap_threshold());
  config->max_check_reaper_time(new_cfg.max_check_reaper_time());
  config->max_debug_file_size(new_cfg.max_debug_file_size());
  config->max_host_check_spread(new_cfg.max_host_check_spread());
  config->max_log_file_size(new_cfg.max_log_file_size());
  config->max_parallel_service_checks(new_cfg.max_parallel_service_checks());
  config->max_service_check_spread(new_cfg.max_service_check_spread());
  config->notification_timeout(new_cfg.notification_timeout());
  config->obsess_over_hosts(new_cfg.obsess_over_hosts());
  config->obsess_over_services(new_cfg.obsess_over_services());
  config->ochp_command(new_cfg.ochp_command());
  config->ochp_timeout(new_cfg.ochp_timeout());
  config->ocsp_command(new_cfg.ocsp_command());
  config->ocsp_timeout(new_cfg.ocsp_timeout());
  config->passive_host_checks_are_soft(new_cfg.passive_host_checks_are_soft());
  config->perfdata_timeout(new_cfg.perfdata_timeout());
  config->process_performance_data(new_cfg.process_performance_data());
  config->resource_file(new_cfg.resource_file());
  config->retain_state_information(new_cfg.retain_state_information());
  config->retained_contact_host_attribute_mask(
      new_cfg.retained_contact_host_attribute_mask());
  config->retained_contact_service_attribute_mask(
      new_cfg.retained_contact_service_attribute_mask());
  config->retained_host_attribute_mask(new_cfg.retained_host_attribute_mask());
  config->retained_process_host_attribute_mask(
      new_cfg.retained_process_host_attribute_mask());
  config->retention_scheduling_horizon(new_cfg.retention_scheduling_horizon());
  config->retention_update_interval(new_cfg.retention_update_interval());
  config->service_check_timeout(new_cfg.service_check_timeout());
  config->service_freshness_check_interval(
      new_cfg.service_freshness_check_interval());
  config->service_inter_check_delay_method(
      new_cfg.service_inter_check_delay_method());
  config->service_interleave_factor_method(
      new_cfg.service_interleave_factor_method());
  config->service_perfdata_command(new_cfg.service_perfdata_command());
  config->service_perfdata_file(new_cfg.service_perfdata_file());
  config->service_perfdata_file_mode(new_cfg.service_perfdata_file_mode());
  config->service_perfdata_file_processing_command(
      new_cfg.service_perfdata_file_processing_command());
  config->service_perfdata_file_processing_interval(
      new_cfg.service_perfdata_file_processing_interval());
  config->service_perfdata_file_template(
      new_cfg.service_perfdata_file_template());
  config->sleep_time(new_cfg.sleep_time());
  config->soft_state_dependencies(new_cfg.soft_state_dependencies());
  config->state_retention_file(new_cfg.state_retention_file());
  config->status_file(new_cfg.status_file());
  config->status_update_interval(new_cfg.status_update_interval());
  config->time_change_threshold(new_cfg.time_change_threshold());
  config->translate_passive_host_checks(
      new_cfg.translate_passive_host_checks());
  config->use_aggressive_host_checking(new_cfg.use_aggressive_host_checking());
  config->use_large_installation_tweaks(
      new_cfg.use_large_installation_tweaks());
  config->use_regexp_matches(new_cfg.use_regexp_matches());
  config->use_retained_program_state(new_cfg.use_retained_program_state());
  config->use_retained_scheduling_info(new_cfg.use_retained_scheduling_info());
  config->use_setpgid(new_cfg.use_setpgid());
  config->use_syslog(new_cfg.use_syslog());
  config->use_true_regexp_matching(new_cfg.use_true_regexp_matching());
  config->user(new_cfg.user());

  // Set this variable just the first time.
  if (!has_already_been_loaded) {
    config->broker_module(new_cfg.broker_module());
    config->broker_module_directory(new_cfg.broker_module_directory());
    config->command_file(new_cfg.command_file());
    config->external_command_buffer_slots(
        new_cfg.external_command_buffer_slots());
    config->use_timezone(new_cfg.use_timezone());
  }

  // Initialize.
  if (modify_status)
    xsddefault_initialize_status_data();
  if (modify_perfdata)
    xpddefault_initialize_performance_data();

  // Check global event handler commands...
  if (verify_config)
    logger(log_info_message, basic) << "Checking global event handlers...";
  if (!config->global_host_event_handler().empty()) {
    // Check the event handler command.
    std::string temp_command_name(config->global_host_event_handler().substr(
        0, config->global_host_event_handler().find_first_of('!')));
    command_map::iterator found{
        commands::command::commands.find(temp_command_name)};
    if (found == commands::command::commands.end() || !found->second) {
      logger(log_verification_error, basic)
          << "Error: Global host event handler command '" << temp_command_name
          << "' is not defined anywhere!";
      ++config_errors;
      global_host_event_handler_ptr = nullptr;
    } else
      global_host_event_handler_ptr = found->second.get();
  }
  if (!config->global_service_event_handler().empty()) {
    // Check the event handler command.
    std::string temp_command_name(config->global_service_event_handler().substr(
        0, config->global_service_event_handler().find_first_of('!')));
    command_map::iterator found{
        commands::command::commands.find(temp_command_name)};
    if (found == commands::command::commands.end() || !found->second) {
      logger(log_verification_error, basic)
          << "Error: Global service event handler command '"
          << temp_command_name << "' is not defined anywhere!";
      ++config_errors;
      global_service_event_handler_ptr = nullptr;
    } else
      global_service_event_handler_ptr = found->second.get();
  }

  // Check obsessive processor commands...
  if (verify_config)
    logger(log_info_message, basic)
        << "Checking obsessive compulsive processor commands...";
  if (!config->ocsp_command().empty()) {
    std::string temp_command_name(config->ocsp_command().substr(
        0, config->ocsp_command().find_first_of('!')));
    command_map::iterator found{
        commands::command::commands.find(temp_command_name)};
    if (found == commands::command::commands.end() || !found->second) {
      logger(log_verification_error, basic)
          << "Error: Obsessive compulsive service processor command '"
          << temp_command_name << "' is not defined anywhere!";
      ++config_errors;
      ocsp_command_ptr = nullptr;
    } else
      ocsp_command_ptr = found->second.get();
  }
  if (!config->ochp_command().empty()) {
    std::string temp_command_name(config->ochp_command().substr(
        0, config->ochp_command().find_first_of('!')));
    command_map::iterator found{
        commands::command::commands.find(temp_command_name)};
    if (found == commands::command::commands.end() || !found->second) {
      logger(log_verification_error, basic)
          << "Error: Obsessive compulsive host processor command '"
          << temp_command_name << "' is not defined anywhere!";
      ++config_errors;
      ochp_command_ptr = nullptr;
    } else
      ochp_command_ptr = found->second.get();
  }
}

/**
 *  @brief Apply configuration of a specific object type.
 *
 *  This method will perform a diff on cur_cfg and new_cfg to create the
 *  three element sets : added, modified and removed. The type applier
 *  will then be called to 1) remove old objects 2) create new objects
 *  3) modify existing objects, in this order.
 *
 *  @param[in] cur_cfg Current configuration set.
 *  @param[in] new_cfg New configuration set.
 */
template <typename ConfigurationType, typename ApplierType>
void applier::state::_apply(
    difference<std::set<ConfigurationType> > const& diff) {
  // Type alias.
  typedef std::set<ConfigurationType> cfg_set;

  /*
  ** Configuration application.
  */

  // Applier.
  ApplierType aplyr;

  // Erase objects.
  for (typename cfg_set::const_iterator it_delete(diff.deleted().begin()),
       end_delete(diff.deleted().end());
       it_delete != end_delete; ++it_delete) {
    if (!verify_config)
      aplyr.remove_object(*it_delete);
    else {
      try {
        aplyr.remove_object(*it_delete);
      } catch (std::exception const& e) {
        ++config_errors;
        logger(log_info_message, basic) << e.what();
      }
    }
  }

  // Add objects.
  for (typename cfg_set::const_iterator it_create(diff.added().begin()),
       end_create(diff.added().end());
       it_create != end_create; ++it_create) {
    if (!verify_config)
      aplyr.add_object(*it_create);
    else {
      try {
        aplyr.add_object(*it_create);
      } catch (std::exception const& e) {
        ++config_errors;
        logger(log_info_message, basic) << e.what();
      }
    }
  }

  // Modify objects.
  for (typename cfg_set::const_iterator it_modify(diff.modified().begin()),
       end_modify(diff.modified().end());
       it_modify != end_modify; ++it_modify) {
    if (!verify_config)
      aplyr.modify_object(*it_modify);
    else {
      try {
        aplyr.modify_object(*it_modify);
      } catch (std::exception const& e) {
        ++config_errors;
        logger(log_info_message, basic) << e.what();
      }
    }
  }
}

#ifdef DEBUG_CONFIG
/**
 *  A method to check service escalations pointers of each service are well
 *  defined.
 *
 *  If something wrong is found, an exception is thrown.
 */
void applier::state::_check_serviceescalations() const {
  for (auto const& p : engine::service::services) {
    engine::service const* srv{p.second.get()};

    std::unordered_set<uint64_t> s;
    for (auto const& escalation : srv->get_escalations()) {
      s.insert((uint64_t) static_cast<void*>(escalation));
      bool found = false;
      for (auto const& e : engine::serviceescalation::serviceescalations) {
        if (e.second.get() == escalation) {
          found = true;
          break;
        }
      }
      if (!found) {
        logger(log_config_error, basic)
            << "Error on serviceescalation !!! The service "
            << srv->get_hostname() << "/" << srv->get_description()
            << " contains a non existing service escalation";
        throw engine_error() << "This is a bug";
      }
    }
    if (s.size() != srv->get_escalations().size()) {
      logger(log_config_error, basic)
          << "Error on serviceescalation !!! Some escalations are stored "
             "several times in service "
          << srv->get_hostname() << "/" << srv->get_description()
          << "set size: " << s.size()
          << " ; list size: " << srv->get_escalations().size();
      throw engine_error() << "This is a bug";
    }
  }

  for (auto const& e : engine::serviceescalation::serviceescalations) {
    engine::serviceescalation const* se{e.second.get()};
    bool found = false;

    for (auto const& p : engine::service::services) {
      if (p.second.get() == se->notifier_ptr) {
        found = true;
        if (se->get_hostname() != p.second->get_hostname()) {
          logger(log_config_error, basic)
              << "Error on serviceescalation !!! The notifier seen by the "
                 "escalation is wrong. "
              << "Host name given by the escalation is " << se->get_hostname()
              << " whereas the hostname from the notifier is "
              << p.second->get_hostname() << ".";
          throw engine_error() << "This is a bug";
        }
        if (se->get_description() != p.second->get_description()) {
          logger(log_config_error, basic)
              << "Error on serviceescalation !!! The notifier seen by the "
                 "escalation is wrong. "
              << "Service description given by the escalation is "
              << se->get_description()
              << " whereas the service description from the notifier is "
              << p.second->get_description() << ".";
          throw engine_error() << "This is a bug";
        }
        break;
      }
    }
    if (!found) {
      logger(log_config_error, basic)
          << "Error on serviceescalation !!! The notifier seen by the "
             "escalation is wrong "
          << "The bug is detected on escalation concerning host "
          << se->get_hostname() << " and service " << se->get_description();
      throw engine_error() << "This is a bug";
    }
  }
}

/**
 *  A method to check host escalations pointers of each host are well
 *  defined.
 *
 *  If something wrong is found, an exception is thrown.
 */
void applier::state::_check_hostescalations() const {
  for (auto const& p : engine::host::hosts) {
    engine::host const* hst{p.second.get()};

    for (auto const& escalation : hst->get_escalations()) {
      bool found = false;
      for (auto const& e : engine::hostescalation::hostescalations) {
        if (e.second.get() == escalation) {
          found = true;
          break;
        }
      }
      if (!found) {
        logger(log_config_error, basic)
            << "Error on hostescalation !!! The host " << hst->get_name()
            << " contains a non existing host escalation";
        throw engine_error() << "This is a bug";
      }
    }
  }

  for (auto const& e : engine::hostescalation::hostescalations) {
    engine::hostescalation const* he{e.second.get()};
    bool found = false;

    for (auto const& p : engine::host::hosts) {
      if (p.second.get() == he->notifier_ptr) {
        found = true;
        if (he->get_hostname() != p.second->get_name()) {
          logger(log_config_error, basic)
              << "Error on hostescalation !!! The notifier seen by the "
                 "escalation is wrong. "
              << "Host name given by the escalation is " << he->get_hostname()
              << " whereas the hostname from the notifier is "
              << p.second->get_name() << ".";
          throw engine_error() << "This is a bug";
        }
        break;
      }
    }
    if (!found) {
      logger(log_config_error, basic)
          << "Error on hostescalation !!! The notifier seen by the escalation "
             "is wrong "
          << "The bug is detected on escalation concerning host "
          << he->get_hostname();
      throw engine_error() << "This is a bug";
    }
  }
}

/**
 *  A method to check contacts pointers of each possible container are well
 *  defined.
 *
 *  If something wrong is found, an exception is thrown.
 */
void applier::state::_check_contacts() const {
  for (auto const& p : engine::contactgroup::contactgroups) {
    engine::contactgroup const* cg{p.second.get()};
    for (auto const& pp : cg->get_members()) {
      contact_map::iterator found{engine::contact::contacts.find(pp.first)};
      if (found == engine::contact::contacts.end() ||
          found->second.get() != pp.second) {
        logger(log_config_error, basic)
            << "Error on contact !!! The contact " << pp.first
            << " used in contactgroup " << p.first
            << " is not or badly defined";
        throw engine_error() << "This is a bug";
      }
    }
  }

  for (auto const& p : engine::service::services) {
    for (auto const& pp : p.second->get_contacts()) {
      contact_map::iterator found{engine::contact::contacts.find(pp.first)};
      if (found == engine::contact::contacts.end() ||
          found->second.get() != pp.second) {
        logger(log_config_error, basic)
            << "Error on contact !!! The contact " << pp.first
            << " used in service " << p.second->get_hostname() << '/'
            << p.second->get_description() << " is not or badly defined";
        throw engine_error() << "This is a bug";
      }
    }
  }

  for (auto const& p : engine::host::hosts) {
    for (auto const& pp : p.second->get_contacts()) {
      contact_map::iterator found{engine::contact::contacts.find(pp.first)};
      if (found == engine::contact::contacts.end() ||
          found->second.get() != pp.second) {
        logger(log_config_error, basic)
            << "Error on contact !!! The contact " << pp.first
            << " used in service " << p.second->get_name()
            << " is not or badly defined";
        throw engine_error() << "This is a bug";
      }
    }
  }
}

/**
 *  A method to check contactgroups pointers of each possible container are well
 *  defined.
 *
 *  If something wrong is found, an exception is thrown.
 */
void applier::state::_check_contactgroups() const {
  for (auto const& p : engine::service::services) {
    engine::service const* svc{p.second.get()};
    for (auto const& pp : svc->get_contactgroups()) {
      contactgroup_map::iterator found{
          engine::contactgroup::contactgroups.find(pp.first)};
      if (found == engine::contactgroup::contactgroups.end() ||
          found->second.get() != pp.second) {
        logger(log_config_error, basic)
            << "Error on contactgroup !!! The contactgroup " << pp.first
            << " used in service " << p.first.first << '/' << p.first.second
            << " is not or badly defined";
        throw engine_error() << "This is a bug";
      }
    }
  }

  for (auto const& p : engine::host::hosts) {
    for (auto const& pp : p.second->get_contactgroups()) {
      contactgroup_map::iterator found{
          engine::contactgroup::contactgroups.find(pp.first)};
      if (found == engine::contactgroup::contactgroups.end() ||
          found->second.get() != pp.second) {
        logger(log_config_error, basic)
            << "Error on contactgroup !!! The contactgroup " << pp.first
            << " used in host " << p.first << " is not or badly defined";
        throw engine_error() << "This is a bug";
      }
    }
  }

  for (auto const& p : engine::serviceescalation::serviceescalations) {
    for (auto const& pp : p.second->get_contactgroups()) {
      contactgroup_map::iterator found{
          engine::contactgroup::contactgroups.find(pp.first)};
      if (found == engine::contactgroup::contactgroups.end() ||
          found->second.get() != pp.second) {
        logger(log_config_error, basic)
            << "Error on contactgroup !!! The contactgroup " << pp.first
            << " used in serviceescalation " << p.second->get_uuid().to_string()
            << " is not or badly defined";
        throw engine_error() << "This is a bug";
      }
    }
  }

  for (auto const& p : engine::hostescalation::hostescalations) {
    for (auto const& pp : p.second->get_contactgroups()) {
      contactgroup_map::iterator found{
          engine::contactgroup::contactgroups.find(pp.first)};
      if (found == engine::contactgroup::contactgroups.end() ||
          found->second.get() != pp.second) {
        logger(log_config_error, basic)
            << "Error on contactgroup !!! The contactgroup " << pp.first
            << " used in hostescalation " << p.second->get_uuid().to_string()
            << " is not or badly defined";
        throw engine_error() << "This is a bug";
      }
    }
  }
}

/**
 *  A method to check services pointers of each possible container are well
 *  defined.
 *
 *  If something wrong is found, an exception is thrown.
 */
void applier::state::_check_services() const {
  for (auto const& p : engine::servicedependency::servicedependencies) {
    engine::servicedependency const* sd{p.second.get()};
    std::list<engine::service*> svcs{sd->master_service_ptr,
                                     sd->dependent_service_ptr};
    for (engine::service const* svc : svcs) {
      service_id_map::const_iterator found{engine::service::services_by_id.find(
          {svc->get_host_id(), svc->get_service_id()})};
      if (found == engine::service::services_by_id.end() ||
          found->second.get() != svc) {
        logger(log_config_error, basic)
            << "Error on service !!! The service " << p.first.first << '/'
            << p.first.second << " used in service dependency " << p.first.first
            << '/' << p.first.second << " is not or badly defined";
        throw engine_error() << "This is a bug";
      }
    }
  }

  for (auto const& p : engine::service::services_by_id) {
    service_map::const_iterator found{engine::service::services.find(
        {p.second->get_hostname(), p.second->get_description()})};
    if (found == engine::service::services.end() ||
        found->second.get() != p.second.get()) {
      logger(log_config_error, basic)
          << "Error on service !!! The service " << p.first.first << '/'
          << p.first.second
          << " defined in services is not defined in services_by_id";
      throw engine_error() << "This is a bug";
    }
  }

  for (auto const& p : engine::service::services) {
    std::array<commands::command*, 2> arr{
        p.second->get_check_command_ptr(),
        p.second->get_event_handler_ptr(),
    };
    for (auto cc : arr) {
      if (cc) {
        bool found = false;
        for (auto& c : engine::commands::command::commands) {
          if (c.second.get() == cc) {
            found = true;
            break;
          }
        }
        if (!found) {
          logger(log_config_error, basic)
              << "Error on service !!! The service " << p.first.first << '/'
              << p.first.second
              << " defined in services has a wrong check command";
          throw engine_error() << "This is a bug";
        }
      }
    }
  }

  if (engine::service::services_by_id.size() !=
      engine::service::services.size()) {
    logger(log_config_error, basic)
        << "Error on service !!! services_by_id contains ices that are not in "
           "services. The first one size is "
        << engine::service::services.size() << "  the second size is "
        << engine::service::services.size();
    throw engine_error() << "This is a bug";
  }
}

/**
 *  A method to check hosts pointers of each possible container are well
 *  defined.
 *
 *  If something wrong is found, an exception is thrown.
 */
void applier::state::_check_hosts() const {
  auto find_host_by_name = [](engine::host const* hst,
                              std::string const& where) {
    host_map::const_iterator found{engine::host::hosts.find(hst->get_name())};
    if (found == engine::host::hosts.end() || found->second.get() != hst) {
      logger(log_config_error, basic)
          << "Error on host !!! The host " << hst->get_name() << " used in "
          << where << " is not defined or badly defined in hosts";
      throw engine_error() << "This is a bug";
    }
  };

  for (auto const& p : engine::hostdependency::hostdependencies) {
    engine::hostdependency const* sd{p.second.get()};
    std::list<engine::host*> hsts{sd->master_host_ptr, sd->dependent_host_ptr};
    for (engine::host const* hst : hsts)
      find_host_by_name(hst, "hostdependency");
  }

  for (auto const& p : engine::host::hosts_by_id) {
    find_host_by_name(p.second.get(), "hosts_by_id");
  }

  for (auto const& p : engine::host::hosts) {
    std::array<commands::command*, 2> arr{
        p.second->get_check_command_ptr(),
        p.second->get_event_handler_ptr(),
    };
    for (auto cc : arr) {
      if (cc) {
        bool found = false;
        for (auto& c : engine::commands::command::commands) {
          if (c.second.get() == cc) {
            found = true;
            break;
          }
        }
        if (!found) {
          logger(log_config_error, basic)
              << "Error on host !!! The host " << p.first
              << " defined in hosts has a wrong check command";
          throw engine_error() << "This is a bug";
        }
      }
    }
  }

  if (engine::host::hosts_by_id.size() != engine::host::hosts.size()) {
    logger(log_config_error, basic)
        << "Error on host !!! hosts_by_id contains hosts that are not in "
           "hosts. The first one size is "
        << engine::service::services.size() << " whereas the second size is "
        << engine::service::services.size();
    throw engine_error() << "This is a bug";
  }

  for (auto const& p : engine::service::services)
    find_host_by_name(p.second->get_host_ptr(), "service");
}

#endif

/**
 *  Apply retention.
 *
 *  @param[in] new_cfg New configuration set.
 *  @param[in] state   The retention state to use.
 */
void applier::state::_apply(configuration::state& new_cfg,
                            retention::state& state) {
  retention::applier::state app_state;
  if (!verify_config)
    app_state.apply(new_cfg, state);
  else {
    try {
      app_state.apply(new_cfg, state);
    } catch (std::exception const& e) {
      ++config_errors;
      logger(log_info_message, basic) << e.what();
    }
  }
}

/**
 *  Expand objects.
 *
 *  @param[in,out] new_state New configuration state.
 *  @param[in,out] cfg       Configuration objects.
 */
template <typename ConfigurationType, typename ApplierType>
void applier::state::_expand(configuration::state& new_state) {
  ApplierType aplyr;
  try {
    aplyr.expand_objects(new_state);
  } catch (std::exception const& e) {
    if (verify_config) {
      ++config_errors;
      logger(log_info_message, basic) << e.what();
    } else
      throw;
  }
}

/**
 *  Process new configuration and apply it.
 *
 *  @param[in] new_cfg        The new configuration.
 *  @param[in] state          The retention to use.
 */
void applier::state::_processing(configuration::state& new_cfg,
                                 retention::state* state) {
  // Timing.
  struct timeval tv[5];

  // Call prelauch broker event the first time to run applier state.
  if (!has_already_been_loaded)
    broker_program_state(NEBTYPE_PROCESS_PRELAUNCH, NEBFLAG_NONE, NEBATTR_NONE,
                         nullptr);

  //
  // Expand all objects.
  //
  gettimeofday(tv, nullptr);

  // Expand timeperiods.
  _expand<configuration::timeperiod, applier::timeperiod>(new_cfg);

  // Expand connectors.
  _expand<configuration::connector, applier::connector>(new_cfg);

  // Expand commands.
  _expand<configuration::command, applier::command>(new_cfg);

  // Expand contacts.
  _expand<configuration::contact, applier::contact>(new_cfg);

  // Expand contactgroups.
  _expand<configuration::contactgroup, applier::contactgroup>(new_cfg);

  // Expand hosts.
  _expand<configuration::host, applier::host>(new_cfg);

  // Expand hostgroups.
  _expand<configuration::hostgroup, applier::hostgroup>(new_cfg);

  // Expand services.
  _expand<configuration::service, applier::service>(new_cfg);

  // Expand servicegroups.
  _expand<configuration::servicegroup, applier::servicegroup>(new_cfg);

  // Expand hostdependencies.
  _expand<configuration::hostdependency, applier::hostdependency>(new_cfg);

  // Expand servicedependencies.
  _expand<configuration::servicedependency, applier::servicedependency>(
      new_cfg);

  // Expand hostescalations.
  _expand<configuration::hostescalation, applier::hostescalation>(new_cfg);

  // Expand serviceescalations.
  _expand<configuration::serviceescalation, applier::serviceescalation>(
      new_cfg);

  //
  //  Build difference for all objects.
  //

  // Build difference for timeperiods.
  difference<set_timeperiod> diff_timeperiods;
  diff_timeperiods.parse(config->timeperiods(), new_cfg.timeperiods());

  // Build difference for connectors.
  difference<set_connector> diff_connectors;
  diff_connectors.parse(config->connectors(), new_cfg.connectors());

  // Build difference for commands.
  difference<set_command> diff_commands;
  diff_commands.parse(config->commands(), new_cfg.commands());

  // Build difference for contacts.
  difference<set_contact> diff_contacts;
  diff_contacts.parse(config->contacts(), new_cfg.contacts());

  // Build difference for contactgroups.
  difference<set_contactgroup> diff_contactgroups;
  diff_contactgroups.parse(config->contactgroups(), new_cfg.contactgroups());

  // Build difference for hosts.
  difference<set_host> diff_hosts;
  diff_hosts.parse(config->hosts(), new_cfg.hosts());

  // Build difference for hostgroups.
  difference<set_hostgroup> diff_hostgroups;
  diff_hostgroups.parse(config->hostgroups(), new_cfg.hostgroups());

  // Build difference for services.
  difference<set_service> diff_services;
  diff_services.parse(config->services(), new_cfg.services());

  // Build difference for servicegroups.
  difference<set_servicegroup> diff_servicegroups;
  diff_servicegroups.parse(config->servicegroups(), new_cfg.servicegroups());

  // Build difference for hostdependencies.
  difference<set_hostdependency> diff_hostdependencies;
  diff_hostdependencies.parse(config->hostdependencies(),
                              new_cfg.hostdependencies());

  // Build difference for servicedependencies.
  difference<set_servicedependency> diff_servicedependencies;
  diff_servicedependencies.parse(config->servicedependencies(),
                                 new_cfg.servicedependencies());

  // Build difference for hostescalations.
  difference<set_hostescalation> diff_hostescalations;
  diff_hostescalations.parse(config->hostescalations(),
                             new_cfg.hostescalations());

  // Build difference for serviceescalations.
  difference<set_serviceescalation> diff_serviceescalations;
  diff_serviceescalations.parse(config->serviceescalations(),
                                new_cfg.serviceescalations());

  // Timing.
  gettimeofday(tv + 1, nullptr);

  try {
    std::lock_guard<std::mutex> locker(_apply_lock);

    // Apply logging configurations.
    applier::logging::instance().apply(new_cfg);

    // Apply globals configurations.
    applier::globals::instance().apply(new_cfg);

    // Apply macros configurations.
    applier::macros::instance().apply(new_cfg);

    // Timing.
    gettimeofday(tv + 2, nullptr);

    if (!has_already_been_loaded && !verify_config && !test_scheduling) {
      // This must be logged after we read config data,
      // as user may have changed location of main log file.
      logger(log_process_info, basic)
          << "Centreon Engine " << CENTREON_ENGINE_VERSION_STRING
          << " starting ... (PID=" << getpid() << ")";

      // Log the local time - may be different than clock
      // time due to timezone offset.
      logger(log_process_info, basic)
          << "Local time is " << string::ctime(program_start) << "\n"
          << "LOG VERSION: " << LOG_VERSION_2;
    }

    //
    //  Apply and resolve all objects.
    //

    // Apply timeperiods.
    _apply<configuration::timeperiod, applier::timeperiod>(diff_timeperiods);
    _resolve<configuration::timeperiod, applier::timeperiod>(
        config->timeperiods());

    // Apply connectors.
    _apply<configuration::connector, applier::connector>(diff_connectors);
    _resolve<configuration::connector, applier::connector>(
        config->connectors());

    // Apply commands.
    _apply<configuration::command, applier::command>(diff_commands);
    _resolve<configuration::command, applier::command>(config->commands());

    // Apply contacts and contactgroups.
    _apply<configuration::contact, applier::contact>(diff_contacts);
    _apply<configuration::contactgroup, applier::contactgroup>(
        diff_contactgroups);
    _resolve<configuration::contactgroup, applier::contactgroup>(
        config->contactgroups());
    _resolve<configuration::contact, applier::contact>(config->contacts());

    // Apply hosts and hostgroups.
    _apply<configuration::host, applier::host>(diff_hosts);
    _apply<configuration::hostgroup, applier::hostgroup>(diff_hostgroups);

    // Apply services and servicegroups.
    _apply<configuration::service, applier::service>(diff_services);
    _apply<configuration::servicegroup, applier::servicegroup>(
        diff_servicegroups);

    // Resolve hosts, services, host groups and service groups.
    _resolve<configuration::host, applier::host>(config->hosts());
    _resolve<configuration::hostgroup, applier::hostgroup>(
        config->hostgroups());
    _resolve<configuration::service, applier::service>(config->services());
    _resolve<configuration::servicegroup, applier::servicegroup>(
        config->servicegroups());

    // Apply host dependencies.
    _apply<configuration::hostdependency, applier::hostdependency>(
        diff_hostdependencies);
    _resolve<configuration::hostdependency, applier::hostdependency>(
        config->hostdependencies());

    // Apply service dependencies.
    _apply<configuration::servicedependency, applier::servicedependency>(
        diff_servicedependencies);
    _resolve<configuration::servicedependency, applier::servicedependency>(
        config->servicedependencies());

    // Apply host escalations.
    _apply<configuration::hostescalation, applier::hostescalation>(
        diff_hostescalations);
    _resolve<configuration::hostescalation, applier::hostescalation>(
        config->hostescalations());

    // Apply service escalations.
    _apply<configuration::serviceescalation, applier::serviceescalation>(
        diff_serviceescalations);
    _resolve<configuration::serviceescalation, applier::serviceescalation>(
        config->serviceescalations());

#ifdef DEBUG_CONFIG
    logger(log_config_error, basic) << "WARNING!! You are using a version of "
                                       "centreon engine for developers!!!"
                                       " This is not a production version.";
    // Checks on configuration
    _check_serviceescalations();
    _check_hostescalations();
    _check_contacts();
    _check_contactgroups();
    _check_services();
    _check_hosts();
#endif

    // Load retention.
    if (state)
      _apply(new_cfg, *state);

    // Apply scheduler.
    if (!verify_config)
      applier::scheduler::instance().apply(new_cfg, diff_hosts, diff_services);

    // Apply new global on the current state.
    if (!verify_config)
      _apply(new_cfg);
    else {
      try {
        _apply(new_cfg);
      } catch (std::exception const& e) {
        ++config_errors;
        logger(log_info_message, basic) << e.what();
      }
    }

    // Timing.
    gettimeofday(tv + 3, nullptr);

    // Check for circular paths between hosts.
    pre_flight_circular_check(&config_warnings, &config_errors);

    // Call start broker event the first time to run applier state.
    if (!has_already_been_loaded) {
      neb_load_all_modules();

      broker_program_state(NEBTYPE_PROCESS_START, NEBFLAG_NONE, NEBATTR_NONE,
                           nullptr);
    } else
      neb_reload_all_modules();

    // Print initial states of new hosts and services.
    if (!verify_config && !test_scheduling) {
      for (set_host::iterator it(diff_hosts.added().begin()),
           end(diff_hosts.added().end());
           it != end; ++it) {
        host_id_map::const_iterator hst(
            engine::host::hosts_by_id.find(it->host_id()));
        if (hst != engine::host::hosts_by_id.end())
          log_host_state(INITIAL_STATES, hst->second.get());
      }
      for (set_service::iterator it(diff_services.added().begin()),
           end(diff_services.added().end());
           it != end; ++it) {
        service_id_map::const_iterator svc(engine::service::services_by_id.find(
            {it->host_id(), it->service_id()}));
        if (svc != engine::service::services_by_id.end())
          log_service_state(INITIAL_STATES, svc->second.get());
      }
    }

    // Timing.
    gettimeofday(tv + 4, nullptr);
    if (test_scheduling) {
      double runtimes[5];
      runtimes[4] = 0.0;
      for (unsigned int i(0); i < (sizeof(runtimes) / sizeof(*runtimes) - 1);
           ++i) {
        runtimes[i] = tv[i + 1].tv_sec - tv[i].tv_sec +
                      (tv[i + 1].tv_usec - tv[i].tv_usec) / 1000000.0;
        runtimes[4] += runtimes[i];
      }
      logger(log_info_message, basic)
          << "\nTiming information on configuration verification is listed "
             "below.\n\n"
          << "CONFIG VERIFICATION TIMES          (* = Potential for speedup "
             "with -x option)\n"
          << "----------------------------------\n"
          << "Template Resolutions: " << runtimes[0] << " sec\n"
          << "Object Relationships: " << runtimes[2] << " sec\n"
          << "Circular Paths:       " << runtimes[3] << " sec  *\n"
          << "Misc:                 " << runtimes[1] << " sec\n"
          << "                      ============\n"
          << "TOTAL:                " << runtimes[4]
          << " sec  * = " << runtimes[3] << " sec ("
          << (runtimes[3] * 100.0 / runtimes[4]) << "%) estimated savings\n";
    }
  } catch (...) {
    _processing_state = state_error;
    throw;
  }

  has_already_been_loaded = true;
  _processing_state = state_ready;
}

/**
 *  Resolve objects.
 *
 *  @param[in] cfg Configuration objects.
 */
template <typename ConfigurationType, typename ApplierType>
void applier::state::_resolve(std::set<ConfigurationType>& cfg) {
  ApplierType aplyr;
  for (typename std::set<ConfigurationType>::const_iterator it(cfg.begin()),
       end(cfg.end());
       it != end; ++it) {
    try {
      aplyr.resolve_object(*it);
    } catch (std::exception const& e) {
      if (verify_config) {
        ++config_errors;
        logger(log_info_message, basic) << e.what();
      } else
        throw;
    }
  }
}

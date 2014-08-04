/*
** Copyright 2011-2014 Merethis
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

#include <cmath>
#include <cstddef>
#include <cstring>
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/scheduler.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/timedevent.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/events/defines.hh"
#include "com/centreon/engine/events/hash_timed_event.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/logging/logger.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::logging;
using namespace com::centreon::logging;

static applier::scheduler* _instance(NULL);

/**
 *  Apply new configuration.
 *
 *  @param[in] config        The new configuration.
 *  @param[in] diff_hosts    The difference between old and the
 *                           new host configuration.
 *  @param[in] diff_services The difference between old and the
 *                           new service configuration.
 */
void applier::scheduler::apply(
       configuration::state& config,
       difference<set_host> const& diff_hosts,
       difference<set_service> const& diff_services) {
  // Remove and create misc event.
  _apply_misc_event();

  // Objects set.
  set_host hst_to_unschedule;
  set_service svc_to_unschedule;
  set_host hst_to_schedule;
  set_service svc_to_schedule;
  hst_to_unschedule = diff_hosts.deleted();
  svc_to_unschedule = diff_services.deleted();
  hst_to_schedule = diff_hosts.added();
  svc_to_schedule = diff_services.added();
  for (set_host::iterator
         it(diff_hosts.modified().begin()),
         end(diff_hosts.modified().end());
       it != end;
       ++it) {
    umap<std::string, shared_ptr<host_struct> > const&
      hosts(applier::state::instance().hosts());
    umap<std::string, shared_ptr<host_struct> >::const_iterator
      hst(hosts.find((*it)->host_name()));
    if (hst != hosts.end()) {
      bool has_event(quick_timed_event.find(
                                         events::hash_timed_event::low,
                                         events::hash_timed_event::host_check,
                                         hst->second.get()));
      bool should_schedule((*it)->checks_active()
                           && ((*it)->check_interval() > 0));
      if (has_event && should_schedule) {
        hst_to_unschedule.insert(*it);
        hst_to_schedule.insert(*it);
      }
      else if (!has_event && should_schedule)
        hst_to_schedule.insert(*it);
      else if (has_event && !should_schedule)
        hst_to_unschedule.insert(*it);
      // Else it has no event and should not be scheduled, so do nothing.
    }
  }
  for (set_service::iterator
         it(diff_services.modified().begin()),
         end(diff_services.modified().end());
       it != end;
       ++it) {
    umap<std::pair<std::string, std::string>, shared_ptr<service_struct> > const&
      services(applier::state::instance().services());
    umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
      svc(services.find(std::make_pair(
                               (*it)->hosts().front(),
                               (*it)->service_description())));
    if (svc != services.end()) {
      bool has_event(quick_timed_event.find(
                                         events::hash_timed_event::low,
                                         events::hash_timed_event::service_check,
                                         svc->second.get()));
      bool should_schedule((*it)->checks_active()
                           && ((*it)->check_interval() > 0));
      if (has_event && should_schedule) {
        svc_to_unschedule.insert(*it);
        svc_to_schedule.insert(*it);
      }
      else if (!has_event && should_schedule)
        svc_to_schedule.insert(*it);
      else if (has_event && !should_schedule)
        svc_to_unschedule.insert(*it);
      // Else it has no event and should not be scheduled, so do nothing.
    }
  }

  // Remove deleted host check from the scheduler.
  {
    std::vector<host_struct*> old_hosts;
    _get_hosts(hst_to_unschedule, old_hosts, false);
    _unschedule_host_checks(old_hosts);
  }

  // Remove deleted service check from the scheduler.
  {
    std::vector<service_struct*> old_services;
    _get_services(svc_to_unschedule, old_services, false);
    _unschedule_service_checks(old_services);
  }

  // Check if we need to add or modify objects into the scheduler.
  if (!hst_to_schedule.empty() || !svc_to_schedule.empty()) {
    // Reset scheduling info.
    memset(&scheduling_info, 0, sizeof(scheduling_info));

    // Calculate scheduling parameters.
    _calculate_host_scheduling_params(config);
    _calculate_service_scheduling_params(config);

    // Get and schedule new hosts.
    {
      std::vector<host_struct*> new_hosts;
      _get_hosts(hst_to_schedule, new_hosts, true);
      _schedule_host_checks(new_hosts);
    }

    // Get and schedule new services.
    {
      std::vector<service_struct*> new_services;
      _get_services(svc_to_schedule, new_services, true);
      _schedule_service_checks(new_services);
    }
  }
}

/**
 *  Get the singleton instance of scheduler applier.
 *
 *  @return Singleton instance.
 */
applier::scheduler& applier::scheduler::instance() {
  return (*_instance);
}

/**
 *  Load scheduler applier singleton.
 */
void applier::scheduler::load() {
  if (!_instance)
    _instance = new applier::scheduler;
}

/**
 *  Remove some host from scheduling.
 *
 *  @param[in] h  Host configuration.
 */
void applier::scheduler::remove_host(configuration::host const& h) {
  umap<std::string, shared_ptr<host_struct> > const&
    hosts(applier::state::instance().hosts());
  umap<std::string, shared_ptr<host_struct> >::const_iterator
    hst(hosts.find(h.host_name()));
  if (hst != hosts.end()) {
    std::vector<host_struct*> hvec;
    hvec.push_back(hst->second.get());
    _unschedule_host_checks(hvec);
  }
  return ;
}

/**
 *  Remove some service from scheduling.
 *
 *  @param[in] s  Service configuration.
 */
void applier::scheduler::remove_service(
                           configuration::service const& s) {
  umap<std::pair<std::string, std::string>, shared_ptr<service_struct> > const&
    services(applier::state::instance().services());
  umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
    svc(services.find(std::make_pair(
                             s.hosts().front(),
                             s.service_description())));
  if (svc != services.end()) {
    std::vector<service_struct*> svec;
    svec.push_back(svc->second.get());
    _unschedule_service_checks(svec);
  }
  return ;
}

/**
 *  Unload scheduler applier singleton.
 */
void applier::scheduler::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::scheduler::scheduler()
  : _evt_check_reaper(NULL),
    _evt_command_check(NULL),
    _evt_hfreshness_check(NULL),
    _evt_orphan_check(NULL),
    _evt_reschedule_checks(NULL),
    _evt_retention_save(NULL),
    _evt_sfreshness_check(NULL),
    _evt_status_save(NULL),
    _old_auto_rescheduling_interval(0),
    _old_check_reaper_interval(0),
    _old_command_check_interval(0),
    _old_host_freshness_check_interval(0),
    _old_retention_update_interval(0),
    _old_service_freshness_check_interval(0),
    _old_status_update_interval(0) {
  memset(&scheduling_info, 0, sizeof(scheduling_info));
}

/**
 *  Default destructor.
 */
applier::scheduler::~scheduler() throw () {
  deleter::listmember(event_list_low, &deleter::timedevent);
  deleter::listmember(event_list_high, &deleter::timedevent);
}

/**
 *  Remove and create misc event if necessary.
 */
void applier::scheduler::_apply_misc_event() {
  // Get current time.
  time_t const now(time(NULL));

  // Remove and add check result reaper event.
  if (!_evt_check_reaper
      || (_old_check_reaper_interval
          != config->check_reaper_interval())) {
    _remove_misc_event(_evt_check_reaper);
    _evt_check_reaper = _create_misc_event(
                          EVENT_CHECK_REAPER,
                          now + config->check_reaper_interval(),
                          config->check_reaper_interval());
    _old_check_reaper_interval = config->check_reaper_interval();
  }

  // Remove and add an external command check event.
  if ((!_evt_command_check && config->check_external_commands())
      || (_evt_command_check && !config->check_external_commands())
      || (_old_command_check_interval
          != config->command_check_interval())) {
    _remove_misc_event(_evt_command_check);
    if (config->check_external_commands()) {
      unsigned long interval(5);
      if (config->command_check_interval() != -1)
        interval = (unsigned long)config->command_check_interval();
      _evt_command_check = _create_misc_event(
                             EVENT_COMMAND_CHECK,
                             now + interval,
                             interval);
    }
    _old_command_check_interval = config->command_check_interval();
  }

  // Remove and add a host result "freshness" check event.
  if ((!_evt_hfreshness_check && config->check_host_freshness())
      || (_evt_hfreshness_check && !config->check_host_freshness())
      || (_old_host_freshness_check_interval
          != config->host_freshness_check_interval())) {
    _remove_misc_event(_evt_hfreshness_check);
    if (config->check_host_freshness())
      _evt_hfreshness_check
        = _create_misc_event(
            EVENT_HFRESHNESS_CHECK,
            now + config->host_freshness_check_interval(),
            config->host_freshness_check_interval());
    _old_host_freshness_check_interval
      = config->host_freshness_check_interval();
  }

  // Remove and add an orphaned check event.
  if ((!_evt_orphan_check && config->check_orphaned_services())
      || (!_evt_orphan_check && config->check_orphaned_hosts())
      || (_evt_orphan_check
          && !config->check_orphaned_services()
          && !config->check_orphaned_hosts())) {
    _remove_misc_event(_evt_orphan_check);
    if (config->check_orphaned_services()
        || config->check_orphaned_hosts())
      _evt_orphan_check = _create_misc_event(
                            EVENT_ORPHAN_CHECK,
                            now + DEFAULT_ORPHAN_CHECK_INTERVAL,
                            DEFAULT_ORPHAN_CHECK_INTERVAL);
  }

  // Remove and add a host and service check rescheduling event.
  if ((!_evt_reschedule_checks && config->auto_reschedule_checks())
      || (_evt_reschedule_checks && !config->auto_reschedule_checks())
      || (_old_auto_rescheduling_interval
          != ::config->auto_rescheduling_interval())) {
    _remove_misc_event(_evt_reschedule_checks);
    if (config->auto_reschedule_checks())
      _evt_reschedule_checks
        = _create_misc_event(
            EVENT_RESCHEDULE_CHECKS,
            now + config->auto_rescheduling_interval(),
            config->auto_rescheduling_interval());
    _old_auto_rescheduling_interval
      = config->auto_rescheduling_interval();
  }

  // Remove and add a retention data save event if needed.
  if ((!_evt_retention_save && config->retain_state_information())
      || (_evt_retention_save && !config->retain_state_information())
      || (_old_retention_update_interval
          != config->retention_update_interval())) {
    _remove_misc_event(_evt_retention_save);
    if (config->retain_state_information()
        && config->retention_update_interval() > 0) {
      unsigned long interval(config->retention_update_interval() * 60);
      _evt_retention_save = _create_misc_event(
                              EVENT_RETENTION_SAVE,
                              now + interval,
                              interval);
    }
    _old_retention_update_interval
      = config->retention_update_interval();
  }

  // Remove add a service result "freshness" check event.
  if ((!_evt_sfreshness_check && config->check_service_freshness())
      || (!_evt_sfreshness_check && !config->check_service_freshness())
      || (_old_service_freshness_check_interval
          != config->service_freshness_check_interval())) {
    _remove_misc_event(_evt_sfreshness_check);
    if (config->check_service_freshness())
      _evt_sfreshness_check
        = _create_misc_event(
            EVENT_SFRESHNESS_CHECK,
            now + config->service_freshness_check_interval(),
            config->service_freshness_check_interval());
    _old_service_freshness_check_interval
      = config->service_freshness_check_interval();
  }

  // Remove and add a status save event.
  if (!_evt_status_save
      || (_old_status_update_interval
          != config->status_update_interval())) {
    _remove_misc_event(_evt_status_save);
    _evt_status_save = _create_misc_event(
                         EVENT_STATUS_SAVE,
                         now + config->status_update_interval(),
                         config->status_update_interval());
    _old_status_update_interval = config->status_update_interval();
  }

  return ;
}

/**
 *  How should we determine the host inter-check delay to use.
 *
 *  @param[in] method The method to use to calculate inter check delay.
 */
void applier::scheduler::_calculate_host_inter_check_delay(
       configuration::state::inter_check_delay method) {
  switch (method) {
  case configuration::state::icd_none:
    scheduling_info.host_inter_check_delay = 0.0;
    break;

  case configuration::state::icd_dumb:
    scheduling_info.host_inter_check_delay = 1.0;
    break;

  case configuration::state::icd_user:
    // the user specified a delay, so don't try to calculate one.
    break;

  case configuration::state::icd_smart:
  default:
    // be smart and calculate the best delay to use
    // to minimize local load...
    if (scheduling_info.total_scheduled_hosts > 0
        && scheduling_info.host_check_interval_total > 0) {

      // calculate the average check interval for hosts.
      scheduling_info.average_host_check_interval
        = scheduling_info.host_check_interval_total
        / (double)scheduling_info.total_scheduled_hosts;

      // calculate the average inter check delay (in seconds)
      // needed to evenly space the host checks out.
      scheduling_info.average_host_inter_check_delay
        = scheduling_info.average_host_check_interval
        / (double)scheduling_info.total_scheduled_hosts;

      // set the global inter check delay value.
      scheduling_info.host_inter_check_delay
        = scheduling_info.average_host_inter_check_delay;

      // calculate max inter check delay and see if we should use that instead.
      double const max_inter_check_delay(
               (scheduling_info.max_host_check_spread * 60)
               / (double)scheduling_info.total_scheduled_hosts);
      if (scheduling_info.host_inter_check_delay > max_inter_check_delay)
        scheduling_info.host_inter_check_delay = max_inter_check_delay;
    }
    else
      scheduling_info.host_inter_check_delay = 0.0;

    logger(dbg_events, most)
      << "Total scheduled host checks:  "
      << scheduling_info.total_scheduled_hosts;
    logger(dbg_events, most)
      << "Host check interval total:    "
      << scheduling_info.host_check_interval_total;
    logger(dbg_events, most)
      << setprecision(2) << "Average host check interval:  "
      << scheduling_info.average_host_check_interval << " sec";
    logger(dbg_events, most)
      << setprecision(2) << "Host inter-check delay:       "
      << scheduling_info.host_inter_check_delay << " sec";
  }
}

/**
 *
 *
 *  @param[in] config The configuration to use.
 */
void applier::scheduler::_calculate_host_scheduling_params(
       configuration::state const& config) {
  logger(dbg_events, most)
    << "Determining host scheduling parameters...";

  // get current time.
  time_t const now(time(NULL));

  // get total hosts and total scheduled hosts.
  for (umap<std::string, shared_ptr<host_struct> >::const_iterator
         it(applier::state::instance().hosts().begin()),
         end(applier::state::instance().hosts().end());
         it != end;
         ++it) {
    host_struct& hst(*it->second);

    bool schedule_check(true);
    if (!hst.check_interval || !hst.checks_enabled)
      schedule_check = false;
    else {
      if (check_time_against_period(
            now,
            hst.check_period_ptr) == ERROR) {
        time_t next_valid_time(0);
        get_next_valid_time(
          now,
          &next_valid_time,
          hst.check_period_ptr);
        if (now == next_valid_time)
          schedule_check = false;
      }
    }

    if (schedule_check) {
      ++scheduling_info.total_scheduled_hosts;
      scheduling_info.host_check_interval_total
        += static_cast<unsigned long>(hst.check_interval);
    }
    else {
      hst.should_be_scheduled = false;
      logger(dbg_events, more)
        << "Host " << hst.name << " should not be scheduled.";
    }

    ++scheduling_info.total_hosts;
  }

  // default max host check spread (in minutes).
  scheduling_info.max_host_check_spread
    = config.max_host_check_spread();

  // adjust the check interval total to correspond to
  // the interval length.
  scheduling_info.host_check_interval_total
    = scheduling_info.host_check_interval_total * config.interval_length();

  _calculate_host_inter_check_delay(
    config.host_inter_check_delay_method());
}

/**
 *  How should we determine the service inter-check delay
 *  to use (in seconds).
 *
 *  @param[in] method The method to use to calculate inter check delay.
 */
void applier::scheduler::_calculate_service_inter_check_delay(
       configuration::state::inter_check_delay method) {
  switch (method) {
  case configuration::state::icd_none:
    scheduling_info.service_inter_check_delay = 0.0;
    break;

  case configuration::state::icd_dumb:
    scheduling_info.service_inter_check_delay = 1.0;
    break;

  case configuration::state::icd_user:
    // the user specified a delay, so don't try to calculate one.
    break;

  case configuration::state::icd_smart:
  default:
    // be smart and calculate the best delay to use to
    // minimize local load...
    if (scheduling_info.total_scheduled_services > 0
        && scheduling_info.service_check_interval_total > 0) {

      // calculate the average inter check delay (in seconds) needed
      // to evenly space the service checks out.
      scheduling_info.average_service_inter_check_delay
        = scheduling_info.average_service_check_interval
        / (double)scheduling_info.total_scheduled_services;

      // set the global inter check delay value.
      scheduling_info.service_inter_check_delay
        = scheduling_info.average_service_inter_check_delay;

      // calculate max inter check delay and see if we should use that instead.
      double const max_inter_check_delay(
               (scheduling_info.max_service_check_spread * 60)
               / (double)scheduling_info.total_scheduled_services);
      if (scheduling_info.service_inter_check_delay > max_inter_check_delay)
        scheduling_info.service_inter_check_delay = max_inter_check_delay;
    }
    else
      scheduling_info.service_inter_check_delay = 0.0;

    logger(dbg_events, more)
      << "Total scheduled service checks:  "
      << scheduling_info.total_scheduled_services;
    logger(dbg_events, more)
      << setprecision(2) << "Average service check interval:  "
      << scheduling_info.average_service_check_interval << " sec";
    logger(dbg_events, more)
      << setprecision(2) << "Service inter-check delay:       "
      << scheduling_info.service_inter_check_delay << " sec";
  }
}

/**
 *  How should we determine the service interleave factor.
 *
 *  @param[in] method The method to use to calculate interleave factor.
 */
void applier::scheduler::_calculate_service_interleave_factor(
       configuration::state::interleave_factor method) {
  switch (method) {
  case configuration::state::ilf_user:
    // the user supplied a value, so don't do any calculation.
    break;

  case configuration::state::ilf_smart:
  default:
    scheduling_info.service_interleave_factor
      = (int)(ceil(scheduling_info.average_scheduled_services_per_host));

    logger(dbg_events, more)
      << "Total scheduled service checks: "
      << scheduling_info.total_scheduled_services;
    logger(dbg_events, more)
      << "Total hosts:                    "
      << scheduling_info.total_hosts;
    logger(dbg_events, more)
      << "Service Interleave factor:      "
      << scheduling_info.service_interleave_factor;
  }
}

/**
 *
 *
 *  @param[in] config The configuration to use.
 */
void applier::scheduler::_calculate_service_scheduling_params(
       configuration::state const& config) {
  logger(dbg_events, most)
    << "Determining service scheduling parameters...";

  // get current time.
  time_t const now(time(NULL));

  // get total services and total scheduled services.
  for (umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
         it(applier::state::instance().services().begin()),
         end(applier::state::instance().services().end());
       it != end;
       ++it) {
    service_struct& svc(*it->second);

    bool schedule_check(true);
    if (!svc.check_interval || !svc.checks_enabled)
      schedule_check = false;

    if (check_time_against_period(now, svc.check_period_ptr) == ERROR) {
      time_t next_valid_time(0);
      get_next_valid_time(now, &next_valid_time, svc.check_period_ptr);
      if (now == next_valid_time)
        schedule_check = false;
    }

    if (schedule_check) {
      ++scheduling_info.total_scheduled_services;
      scheduling_info.service_check_interval_total
        += static_cast<unsigned long>(svc.check_interval);
    }
    else {
      svc.should_be_scheduled = false;
      logger(dbg_events, more)
        << "Service " << svc.description << " on host " << svc.host_name
        << " should not be scheduled.";
    }
    ++scheduling_info.total_services;
  }

  // default max service check spread (in minutes).
  scheduling_info.max_service_check_spread
    = config.max_service_check_spread();

  // used later in inter-check delay calculations.
  scheduling_info.service_check_interval_total
    = scheduling_info.service_check_interval_total * config.interval_length();

  if (scheduling_info.total_hosts) {
    scheduling_info.average_services_per_host
      = scheduling_info.total_services
      / (double)scheduling_info.total_hosts;
    scheduling_info.average_scheduled_services_per_host
      = scheduling_info.total_scheduled_services
      / (double)scheduling_info.total_hosts;
  }

  // calculate rolling average execution time (available
  // from retained state information).
  if (scheduling_info.total_scheduled_services)
    scheduling_info.average_service_check_interval
      = scheduling_info.service_check_interval_total
      / (double)scheduling_info.total_scheduled_services;

  _calculate_service_inter_check_delay(
    config.service_inter_check_delay_method());
  _calculate_service_interleave_factor(
    config.service_interleave_factor_method());
}

/**
 *  Create and register new misc event.
 *
 *  @param[in] type     The event type.
 *  @param[in] start    The date time to start event.
 *  @param[in] interval The rescheduling interval.
 *  @param[in] data     The timed event data.
 *
 *  @return The new event.
 */
timed_event* applier::scheduler::_create_misc_event(
               int type,
               time_t start,
               unsigned long interval,
               void* data) {
  return (events::schedule(
            type,
            true,
            start,
            true,
            interval,
            NULL,
            true,
            data,
            NULL,
            0));
}

/**
 *  Get engine hosts struct with configuration hosts objects.
 *
 *  @param[in]  hst_cfg             The list of configuration hosts objects.
 *  @param[out] hst_obj             The list of engine hosts to fill.
 *  @param[in]  throw_if_not_found  Flag to throw if an host is not
 *                                  found.
 */
void applier::scheduler::_get_hosts(
       set_host const& hst_cfg,
       std::vector<host_struct*>& hst_obj,
       bool throw_if_not_found) {
  umap<std::string, shared_ptr<host_struct> > const&
    hosts(applier::state::instance().hosts());
  for (set_host::const_reverse_iterator
         it(hst_cfg.rbegin()), end(hst_cfg.rend());
       it != end;
       ++it) {
    std::string const& host_name((*it)->host_name());
    umap<std::string, shared_ptr<host_struct> >::const_iterator
      hst(hosts.find(host_name));
    if (hst == hosts.end()) {
      if (throw_if_not_found)
        throw (engine_error() << "Could not schedule non-existing host '"
               << host_name << "'");
    }
    else
      hst_obj.push_back(&*hst->second);
  }
  return ;
}

/**
 *  Get engine services struct with configuration services objects.
 *
 *  @param[in]  svc_cfg             The list of configuration services objects.
 *  @param[out] svc_obj             The list of engine services to fill.
 *  @param[in]  throw_if_not_found  Flag to throw if an host is not
 *                                  found.
 */
void applier::scheduler::_get_services(
       set_service const& svc_cfg,
       std::vector<service_struct*>& svc_obj,
       bool throw_if_not_found) {
  umap<std::pair<std::string, std::string>, shared_ptr<service_struct> > const&
    services(applier::state::instance().services());
  for (set_service::const_reverse_iterator
         it(svc_cfg.rbegin()), end(svc_cfg.rend());
       it != end;
       ++it) {
    std::string const& host_name((*it)->hosts().front());
    std::string const& service_description((*it)->service_description());
    umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
      svc(services.find(std::make_pair(host_name, service_description)));
    if (svc == services.end()) {
      if (throw_if_not_found)
        throw (engine_error() << "Cannot schedule non-existing service '"
               << service_description << "' on host '"
               << host_name << "'");
    }
    else
      svc_obj.push_back(&*svc->second);
  }
  return ;
}

/**
 *  Remove misc event.
 *
 *  @param[int,out] evt The event to remove.
 */
void applier::scheduler::_remove_misc_event(timed_event*& evt) {
  if (evt) {
    remove_event(evt, &event_list_high, &event_list_high_tail);
    delete evt;
    evt = NULL;
  }
}

/**
 *  Schedule host checks.
 *
 *  @param[in] hosts The list of hosts to schedule.
 */
void applier::scheduler::_schedule_host_checks(
       std::vector<host_struct*> const& hosts) {
  logger(dbg_events, most)
    << "Scheduling host checks...";

  // get current time.
  time_t const now(time(NULL));

  unsigned int const end(hosts.size());

  // determine check times for host checks.
  int mult_factor(0);
  for (unsigned int i(0); i < end; ++i) {
    host_struct& hst(*hosts[i]);

    logger(dbg_events, most)
      << "Host '" <<  hst.name << "'";

    // skip hosts that shouldn't be scheduled.
    if (!hst.should_be_scheduled) {
      logger(dbg_events, most)
        << "Host check should not be scheduled.";
      continue;
    }

    // calculate preferred host check time.
    hst.next_check
      = (time_t)(now
           + (mult_factor * scheduling_info.host_inter_check_delay));

    logger(dbg_events, most)
      << "Preferred Check Time: " << hst.next_check
      << " --> " << my_ctime(&hst.next_check);

    // make sure the host can actually be scheduled at this time.
    if (check_time_against_period(
          hst.next_check,
          hst.check_period_ptr) == ERROR) {
      time_t next_valid_time(0);
      get_next_valid_time(
        hst.next_check,
        &next_valid_time,
        hst.check_period_ptr);
      hst.next_check = next_valid_time;
    }

    logger(dbg_events, most)
      << "Actual Check Time: " << hst.next_check
      << " --> " << my_ctime(&hst.next_check);

    if (!scheduling_info.first_host_check
        || (hst.next_check < scheduling_info.first_host_check))
      scheduling_info.first_host_check = hst.next_check;
    if (hst.next_check > scheduling_info.last_host_check)
      scheduling_info.last_host_check = hst.next_check;

    ++mult_factor;
  }

  // Need to optimize add_event insert.
  std::multimap<time_t, host_struct*> hosts_to_schedule;

  // add scheduled host checks to event queue.
  for (unsigned int i(0); i < end; ++i) {
    host_struct& hst(*hosts[i]);

    // Update status of all hosts (scheduled or not).
    update_host_status(&hst);

    // skip most hosts that shouldn't be scheduled.
    if (!hst.should_be_scheduled) {
      // passive checks are an exception if a forced check was
      // scheduled before Centreon Engine was restarted.
      if (!(hst.checks_enabled == false
            && hst.next_check
            && (hst.check_options & CHECK_OPTION_FORCE_EXECUTION)))
        continue;
    }
    hosts_to_schedule.insert(std::make_pair(hst.next_check, &hst));
  }

  // Schedule events list.
  for (std::multimap<time_t, host_struct*>::const_iterator
         it(hosts_to_schedule.begin()), end(hosts_to_schedule.end());
       it != end;
       ++it) {
    host_struct& hst(*it->second);

    // Schedule a new host check event.
    events::schedule(
              EVENT_HOST_CHECK,
              false,
              hst.next_check,
              false,
              0,
              NULL,
              true,
              (void*)&hst,
              NULL,
              hst.check_options);
  }

  return ;
}

/**
 *  Schedule service checks.
 *
 *  @param[in] services The list of services to schedule.
 */
void applier::scheduler::_schedule_service_checks(
       std::vector<service_struct*> const& services) {
  logger(dbg_events, most)
    << "Scheduling service checks...";

  // get current time.
  time_t const now(time(NULL));

  int total_interleave_blocks(scheduling_info.total_scheduled_services);
  // calculate number of service interleave blocks.
  if (scheduling_info.service_interleave_factor)
    total_interleave_blocks
      = (int)ceil(scheduling_info.total_scheduled_services
                  / (double)scheduling_info.service_interleave_factor);

  // determine check times for service checks (with
  // interleaving to minimize remote load).

  int current_interleave_block(0);
  unsigned int const end(services.size());

  if (scheduling_info.service_interleave_factor > 0) {
    int interleave_block_index(0);
    for (unsigned int i(0); i < end; ++i) {
      service_struct& svc(*services[i]);
      if (interleave_block_index >= scheduling_info.service_interleave_factor) {
        ++current_interleave_block;
        interleave_block_index = 0;
      }

      // skip this service if it shouldn't be scheduled.
      if (!svc.should_be_scheduled)
        continue;

      int const mult_factor(
            current_interleave_block
            + ++interleave_block_index * total_interleave_blocks);

      // set the preferred next check time for the service.
      svc.next_check
        = (time_t)(now
             + mult_factor * scheduling_info.service_inter_check_delay);

      // make sure the service can actually be scheduled when we want.
      if (check_time_against_period(
            svc.next_check,
            svc.check_period_ptr) == ERROR) {
        time_t next_valid_time(0);
        get_next_valid_time(
          svc.next_check,
          &next_valid_time,
          svc.check_period_ptr);
        svc.next_check = next_valid_time;
      }

      if (!scheduling_info.first_service_check
          || svc.next_check < scheduling_info.first_service_check)
        scheduling_info.first_service_check = svc.next_check;
      if (svc.next_check > scheduling_info.last_service_check)
        scheduling_info.last_service_check = svc.next_check;
    }
  }

  // Need to optimize add_event insert.
  std::multimap<time_t, service_struct*> services_to_schedule;

  // add scheduled service checks to event queue.
  for (unsigned int i(0); i < end; ++i) {
    service_struct& svc(*services[i]);

    // Update status of all services (scheduled or not).
    update_service_status(&svc);

    // skip most services that shouldn't be scheduled.
    if (!svc.should_be_scheduled) {
      // passive checks are an exception if a forced check was
      // scheduled before Centreon Engine was restarted.
      if (!(svc.checks_enabled == false
            && svc.next_check
            && (svc.check_options & CHECK_OPTION_FORCE_EXECUTION)))
        continue;
    }
    services_to_schedule.insert(std::make_pair(svc.next_check, &svc));
  }

  // Schedule events list.
  for (std::multimap<time_t, service_struct*>::const_iterator
         it(services_to_schedule.begin()),
         end(services_to_schedule.end());
       it != end;
       ++it) {
    service_struct& svc(*it->second);
    // Create a new service check event.
    events::schedule(
              EVENT_SERVICE_CHECK,
              false,
              svc.next_check,
              false,
              0,
              NULL,
              true,
              (void*)&svc,
              NULL,
              svc.check_options);
  }

  return ;
}

/**
 *  Unschedule host checks.
 *
 *  @param[in] hosts The list of hosts to unschedule.
 */
void applier::scheduler::_unschedule_host_checks(
                           std::vector<host_struct*> const& hosts) {
  for (std::vector<host_struct*>::const_iterator
         it(hosts.begin()),
         end(hosts.end());
       it != end;
       ++it) {
    timed_event* evt(quick_timed_event.find(
                                         events::hash_timed_event::low,
                                         events::hash_timed_event::host_check,
                                         *it));
    while (evt) {
      remove_event(evt, &event_list_low, &event_list_low_tail);
      delete evt;
      evt = quick_timed_event.find(
                                events::hash_timed_event::low,
                                events::hash_timed_event::host_check,
                                *it);
    }
  }
  return ;
}

/**
 *  Schedule service checks.
 *
 *  @param[in] services The list of services to schedule.
 */
void applier::scheduler::_unschedule_service_checks(
                           std::vector<service_struct*> const& services) {
  for (std::vector<service_struct*>::const_iterator
         it(services.begin()),
         end(services.end());
       it != end;
       ++it) {
    timed_event* evt(quick_timed_event.find(
                                         events::hash_timed_event::low,
                                         events::hash_timed_event::service_check,
                                         *it));
    while (evt) {
      remove_event(evt, &event_list_low, &event_list_low_tail);
      delete evt;
      evt = quick_timed_event.find(
                                events::hash_timed_event::low,
                                events::hash_timed_event::service_check,
                                *it);
    }
  }
  return ;
}

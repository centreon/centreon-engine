/*
** Copyright 2011-2017 Centreon
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
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/statusdata.hh"
#include "com/centreon/engine/timezone_locker.hh"
#include "com/centreon/engine/xpddefault.hh"
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
  // Internal pointer will be used in private methods.
  _config = &config;

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
    host_map const& hosts{engine::host::hosts};
    host_map::const_iterator hst(hosts.find(it->host_name().c_str()));
    if (hst != hosts.end()) {
      bool has_event(timed_event::find_event(timed_event::low,
                                             EVENT_HOST_CHECK,
                                             hst->second.get()));
      bool should_schedule(it->checks_active()
                           && (it->check_interval() > 0));
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
    service_id_map const& services(engine::service::services_by_id);
    service_id_map::const_iterator
      svc(engine::service::services_by_id.find({
        it->host_id(), it->service_id()}));
    if (svc != services.end()) {
      bool has_event(timed_event::find_event(timed_event::low,
                                             EVENT_SERVICE_CHECK,
                                             svc->second.get()));
      bool should_schedule(it->checks_active()
                           && (it->check_interval() > 0));
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
    std::vector<com::centreon::engine::host*> old_hosts;
    _get_hosts(hst_to_unschedule, old_hosts, false);
    _unschedule_host_events(old_hosts);
  }

  // Remove deleted service check from the scheduler.
  {
    std::vector<engine::service*> old_services;
    _get_services(svc_to_unschedule, old_services, false);
    _unschedule_service_events(old_services);
  }

  // Check if we need to add or modify objects into the scheduler.
  if (!hst_to_schedule.empty() || !svc_to_schedule.empty()) {
    // Reset scheduling info.
    // Keep data that has been set manually by the user
    // (service interleave and intercheck delays).
    int old_service_leave_factor = scheduling_info.service_interleave_factor;
    double old_service_inter_check_delay = scheduling_info.service_inter_check_delay;
    double old_host_inter_check_delay_method = scheduling_info.host_inter_check_delay;
    memset(&scheduling_info, 0, sizeof(scheduling_info));
    if (config.service_interleave_factor_method()
          == configuration::state::ilf_user)
      scheduling_info.service_interleave_factor = old_service_leave_factor;
    if (config.service_inter_check_delay_method()
        == configuration::state::icd_user)
      scheduling_info.service_inter_check_delay
        = old_service_inter_check_delay;
    if (config.host_inter_check_delay_method()
        == configuration::state::icd_user)
      scheduling_info.host_inter_check_delay
        = old_host_inter_check_delay_method;

    // Calculate scheduling parameters.
    _calculate_host_scheduling_params();
    _calculate_service_scheduling_params();

    // Get and schedule new hosts.
    {
      std::vector<com::centreon::engine::host*> new_hosts;
      _get_hosts(hst_to_schedule, new_hosts, true);
      _schedule_host_events(new_hosts);
    }

    // Get and schedule new services.
    {
      std::vector<engine::service*> new_services;
      _get_services(svc_to_schedule, new_services, true);
      _schedule_service_events(new_services);
    }
  }
}

/**
 *  Get the singleton instance of scheduler applier.
 *
 *  @return Singleton instance.
 */
applier::scheduler& applier::scheduler::instance() {
  assert(_instance);
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
  host_map const& hosts(engine::host::hosts);
  host_map::const_iterator hst(hosts.find(h.host_name()));
  if (hst != hosts.end()) {
    std::vector<com::centreon::engine::host*> hvec;
    hvec.push_back(hst->second.get());
    _unschedule_host_events(hvec);
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
  service_id_map const& services(engine::service::services_by_id);
  service_id_map::const_iterator svc(services.find({s.host_id(), s.service_id()}));
  if (svc != services.end()) {
    std::vector<engine::service*> svec;
    svec.push_back(svc->second.get());
    _unschedule_service_events(svec);
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
  : _config(NULL),
    _evt_check_reaper(NULL),
    _evt_command_check(NULL),
    _evt_hfreshness_check(NULL),
    _evt_host_perfdata(NULL),
    _evt_orphan_check(NULL),
    _evt_reschedule_checks(NULL),
    _evt_retention_save(NULL),
    _evt_sfreshness_check(NULL),
    _evt_service_perfdata(NULL),
    _evt_status_save(NULL),
    _old_auto_rescheduling_interval(0),
    _old_check_reaper_interval(0),
    _old_command_check_interval(0),
    _old_host_freshness_check_interval(0),
    _old_host_perfdata_file_processing_interval(0),
    _old_retention_update_interval(0),
    _old_service_freshness_check_interval(0),
    _old_service_perfdata_file_processing_interval(0),
    _old_status_update_interval(0) {
  memset(&scheduling_info, 0, sizeof(scheduling_info));
}

/**
 *  Default destructor.
 */
applier::scheduler::~scheduler() throw () {
  timed_event *evt;

  for (timed_event_list::iterator
         it{timed_event::event_list_low.begin()},
         end{timed_event::event_list_low.end()};
       it != end;) {
    if((*it)->event_type == EVENT_SCHEDULED_DOWNTIME) {
      delete static_cast<unsigned long *>((*it)->event_data);
      (*it)->event_data = nullptr;
    }
    evt = (*it);
    it = timed_event::event_list_low.erase(it);
    delete evt;
  }

  for (timed_event_list::iterator
         it{timed_event::event_list_high.begin()},
         end{timed_event::event_list_high.end()};
       it != end;
       ++it) {
    if((*it)->event_type == EVENT_SCHEDULED_DOWNTIME) {
      delete static_cast<unsigned long *>((*it)->event_data);
      (*it)->event_data = nullptr;
    }
    evt = (*it);
    it = timed_event::event_list_high.erase(it);
    delete evt;
  }
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
          != _config->check_reaper_interval())) {
    _remove_misc_event(_evt_check_reaper);
    _evt_check_reaper = _create_misc_event(
                          EVENT_CHECK_REAPER,
                          now + _config->check_reaper_interval(),
                          _config->check_reaper_interval());
    _old_check_reaper_interval = _config->check_reaper_interval();
  }

  // Remove and add an external command check event.
  if ((!_evt_command_check && _config->check_external_commands())
      || (_evt_command_check && !_config->check_external_commands())
      || (_old_command_check_interval
          != _config->command_check_interval())) {
    _remove_misc_event(_evt_command_check);
    if (_config->check_external_commands()) {
      unsigned long interval(5);
      if (_config->command_check_interval() != -1)
        interval = (unsigned long)_config->command_check_interval();
      _evt_command_check = _create_misc_event(
                             EVENT_COMMAND_CHECK,
                             now + interval,
                             interval);
    }
    _old_command_check_interval = _config->command_check_interval();
  }

  // Remove and add a host result "freshness" check event.
  if ((!_evt_hfreshness_check && _config->check_host_freshness())
      || (_evt_hfreshness_check && !_config->check_host_freshness())
      || (_old_host_freshness_check_interval
          != _config->host_freshness_check_interval())) {
    _remove_misc_event(_evt_hfreshness_check);
    if (_config->check_host_freshness())
      _evt_hfreshness_check
        = _create_misc_event(
            EVENT_HFRESHNESS_CHECK,
            now + _config->host_freshness_check_interval(),
            _config->host_freshness_check_interval());
    _old_host_freshness_check_interval
      = _config->host_freshness_check_interval();
  }

  // Remove and add an orphaned check event.
  if ((!_evt_orphan_check && _config->check_orphaned_services())
      || (!_evt_orphan_check && _config->check_orphaned_hosts())
      || (_evt_orphan_check
          && !_config->check_orphaned_services()
          && !_config->check_orphaned_hosts())) {
    _remove_misc_event(_evt_orphan_check);
    if (_config->check_orphaned_services()
        || _config->check_orphaned_hosts())
      _evt_orphan_check = _create_misc_event(
                            EVENT_ORPHAN_CHECK,
                            now + DEFAULT_ORPHAN_CHECK_INTERVAL,
                            DEFAULT_ORPHAN_CHECK_INTERVAL);
  }

  // Remove and add a host and service check rescheduling event.
  if ((!_evt_reschedule_checks && _config->auto_reschedule_checks())
      || (_evt_reschedule_checks && !_config->auto_reschedule_checks())
      || (_old_auto_rescheduling_interval
          != _config->auto_rescheduling_interval())) {
    _remove_misc_event(_evt_reschedule_checks);
    if (_config->auto_reschedule_checks())
      _evt_reschedule_checks
        = _create_misc_event(
            EVENT_RESCHEDULE_CHECKS,
            now + _config->auto_rescheduling_interval(),
            _config->auto_rescheduling_interval());
    _old_auto_rescheduling_interval
      = _config->auto_rescheduling_interval();
  }

  // Remove and add a retention data save event if needed.
  if ((!_evt_retention_save && _config->retain_state_information())
      || (_evt_retention_save && !_config->retain_state_information())
      || (_old_retention_update_interval
          != _config->retention_update_interval())) {
    _remove_misc_event(_evt_retention_save);
    if (_config->retain_state_information()
        && _config->retention_update_interval() > 0) {
      unsigned long interval(_config->retention_update_interval() * 60);
      _evt_retention_save = _create_misc_event(
                              EVENT_RETENTION_SAVE,
                              now + interval,
                              interval);
    }
    _old_retention_update_interval
      = _config->retention_update_interval();
  }

  // Remove add a service result "freshness" check event.
  if ((!_evt_sfreshness_check && _config->check_service_freshness())
      || (!_evt_sfreshness_check && !_config->check_service_freshness())
      || (_old_service_freshness_check_interval
          != _config->service_freshness_check_interval())) {
    _remove_misc_event(_evt_sfreshness_check);
    if (_config->check_service_freshness())
      _evt_sfreshness_check
        = _create_misc_event(
            EVENT_SFRESHNESS_CHECK,
            now + _config->service_freshness_check_interval(),
            _config->service_freshness_check_interval());
    _old_service_freshness_check_interval
      = _config->service_freshness_check_interval();
  }

  // Remove and add a status save event.
  if (!_evt_status_save
      || (_old_status_update_interval
          != _config->status_update_interval())) {
    _remove_misc_event(_evt_status_save);
    _evt_status_save = _create_misc_event(
                         EVENT_STATUS_SAVE,
                         now + _config->status_update_interval(),
                         _config->status_update_interval());
    _old_status_update_interval = _config->status_update_interval();
  }

  union {
    int (*func)();
    void* data;
  } type;

  // Remove and add process host perfdata file.
  if (!_evt_host_perfdata
      || (_old_host_perfdata_file_processing_interval
          != _config->host_perfdata_file_processing_interval())
      || (_old_host_perfdata_file_processing_command
          != _config->host_perfdata_file_processing_command())) {
    _remove_misc_event(_evt_host_perfdata);
    if (_config->host_perfdata_file_processing_interval() > 0
        && !_config->host_perfdata_file_processing_command().empty()) {
      type.func = &xpddefault_process_host_perfdata_file;
      _evt_host_perfdata
        = _create_misc_event(
            EVENT_USER_FUNCTION,
            now + _config->host_perfdata_file_processing_interval(),
            _config->host_perfdata_file_processing_interval(),
            type.data);
    }
    _old_host_perfdata_file_processing_interval
      = _config->host_perfdata_file_processing_interval();
    _old_host_perfdata_file_processing_command
      = _config->host_perfdata_file_processing_command();
  }

  // Remove and add process service perfdata file.
  if (!_evt_service_perfdata
      || (_old_service_perfdata_file_processing_interval
          != _config->service_perfdata_file_processing_interval())
      || (_old_service_perfdata_file_processing_command
          != _config->service_perfdata_file_processing_command())) {
    _remove_misc_event(_evt_service_perfdata);
    if (_config->service_perfdata_file_processing_interval() > 0
        && !_config->service_perfdata_file_processing_command().empty()) {
      type.func = &xpddefault_process_service_perfdata_file;
      _evt_service_perfdata
        = _create_misc_event(
            EVENT_USER_FUNCTION,
            now + _config->service_perfdata_file_processing_interval(),
            _config->service_perfdata_file_processing_interval(),
            type.data);
    }
    _old_service_perfdata_file_processing_interval
      = _config->service_perfdata_file_processing_interval();
    _old_service_perfdata_file_processing_command
      = _config->service_perfdata_file_processing_command();
  }
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
 *  Calculate host scheduling params.
 */
void applier::scheduler::_calculate_host_scheduling_params() {
  logger(dbg_events, most)
    << "Determining host scheduling parameters...";

  // get current time.
  time_t const now(time(NULL));

  // get total hosts and total scheduled hosts.
  for (host_map::const_iterator
         it(engine::host::hosts.begin()),
         end(engine::host::hosts.end());
         it != end;
         ++it) {
    com::centreon::engine::host& hst(*it->second);

    bool schedule_check(true);
    if (!hst.get_check_interval() || !hst.get_checks_enabled())
      schedule_check = false;
    else {
      timezone_locker lock(hst.get_timezone());
      if (!check_time_against_period(
            now,
            hst.check_period_ptr)) {
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
      hst.set_should_be_scheduled(true);
      ++scheduling_info.total_scheduled_hosts;
      scheduling_info.host_check_interval_total
        += static_cast<unsigned long>(hst.get_check_interval());
    }
    else {
      hst.set_should_be_scheduled(false);
      logger(dbg_events, more)
        << "Host " << hst.get_name() << " should not be scheduled.";
    }

    ++scheduling_info.total_hosts;
  }

  // Default max host check spread (in minutes).
  scheduling_info.max_host_check_spread
    = _config->max_host_check_spread();

  // Adjust the check interval total to correspond to
  // the interval length.
  scheduling_info.host_check_interval_total
    = scheduling_info.host_check_interval_total
      * _config->interval_length();

  _calculate_host_inter_check_delay(
    _config->host_inter_check_delay_method());

  return ;
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
 *  Calculate service scheduling params.
 */
void applier::scheduler::_calculate_service_scheduling_params() {
  logger(dbg_events, most)
    << "Determining service scheduling parameters...";

  // get current time.
  time_t const now(time(NULL));

  // get total services and total scheduled services.
  for (service_id_map::const_iterator
         it(engine::service::services_by_id.begin()),
         end(engine::service::services_by_id.end());
       it != end;
       ++it) {
    engine::service& svc(*it->second);

    bool schedule_check(true);
    if (!svc.get_check_interval() || !svc.get_checks_enabled())
      schedule_check = false;

    {
      timezone_locker lock(svc.get_timezone());
      if (!check_time_against_period(now, svc.check_period_ptr)) {
        time_t next_valid_time(0);
        get_next_valid_time(
          now,
          &next_valid_time,
          svc.check_period_ptr);
        if (now == next_valid_time)
          schedule_check = false;
      }
    }

    if (schedule_check) {
      svc.set_should_be_scheduled(true);
      ++scheduling_info.total_scheduled_services;
      scheduling_info.service_check_interval_total
        += static_cast<unsigned long>(svc.get_check_interval());
    }
    else {
      svc.set_should_be_scheduled(false);
      logger(dbg_events, more)
        << "Service " << svc.get_description() << " on host " << svc.get_hostname()
        << " should not be scheduled.";
    }
    ++scheduling_info.total_services;
  }

  // default max service check spread (in minutes).
  scheduling_info.max_service_check_spread
    = _config->max_service_check_spread();

  // used later in inter-check delay calculations.
  scheduling_info.service_check_interval_total
    = scheduling_info.service_check_interval_total
      * _config->interval_length();

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
    _config->service_inter_check_delay_method());
  _calculate_service_interleave_factor(
    _config->service_interleave_factor_method());
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
       std::vector<com::centreon::engine::host*>& hst_obj,
       bool throw_if_not_found) {
  host_id_map const& hosts{engine::host::hosts_by_id};
  for (set_host::const_reverse_iterator
         it(hst_cfg.rbegin()),
         end(hst_cfg.rend());
       it != end;
       ++it) {
    uint64_t host_id(it->host_id());
    std::string const& host_name(it->host_name());
    host_id_map::const_iterator hst(hosts.find(host_id));
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
       std::vector<engine::service*>& svc_obj,
       bool throw_if_not_found) {
  service_id_map const& services(engine::service::services_by_id);
  for (set_service::const_reverse_iterator
         it(svc_cfg.rbegin()), end(svc_cfg.rend());
       it != end;
       ++it) {
    uint64_t host_id(it->host_id());
    uint64_t service_id(it->service_id());
    std::string const& host_name(*it->hosts().begin());
    std::string const& service_description(it->service_description());
    service_id_map::const_iterator svc(services.find({host_id, service_id}));
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
    remove_event(evt, timed_event::high);
    delete evt;
    evt = NULL;
  }
}

/**
 *  Schedule host events (checks notably).
 *
 *  @param[in] hosts  The list of hosts to schedule.
 */
void applier::scheduler::_schedule_host_events(
       std::vector<com::centreon::engine::host*> const& hosts) {
  logger(dbg_events, most)
    << "Scheduling host checks...";

  // get current time.
  time_t const now(time(NULL));

  unsigned int const end(hosts.size());

  // determine check times for host checks.
  int mult_factor(0);
  for (unsigned int i(0); i < end; ++i) {
    com::centreon::engine::host& hst(*hosts[i]);

    logger(dbg_events, most)
      << "Host '" <<  hst.get_name() << "'";

    // skip hosts that shouldn't be scheduled.
    if (!hst.get_should_be_scheduled()) {
      logger(dbg_events, most)
        << "Host check should not be scheduled.";
      continue;
    }

    // calculate preferred host check time.
    hst.set_next_check(
      (time_t)(now
           + (mult_factor * scheduling_info.host_inter_check_delay)));

    time_t time = hst.get_next_check();
    logger(dbg_events, most)
      << "Preferred Check Time: " << hst.get_next_check()
      << " --> " << my_ctime(&time);

    // Make sure the host can actually be scheduled at this time.
    {
      timezone_locker lock(hst.get_timezone());
      if (!check_time_against_period(
            hst.get_next_check(),
            hst.check_period_ptr)) {
        time_t next_valid_time(0);
        get_next_valid_time(
          hst.get_next_check(),
          &next_valid_time,
          hst.check_period_ptr);
        hst.set_next_check(next_valid_time);
      }
    }

    time = hst.get_next_check();
    logger(dbg_events, most)
      << "Actual Check Time: " << hst.get_next_check()
      << " --> " << my_ctime(&time);

    if (!scheduling_info.first_host_check
        || (hst.get_next_check() < scheduling_info.first_host_check))
      scheduling_info.first_host_check = hst.get_next_check();
    if (hst.get_next_check() > scheduling_info.last_host_check)
      scheduling_info.last_host_check = hst.get_next_check();

    ++mult_factor;
  }

  // Need to optimize add_event insert.
  std::multimap<time_t, com::centreon::engine::host*> hosts_to_schedule;

  // add scheduled host checks to event queue.
  for (unsigned int i(0); i < end; ++i) {
    com::centreon::engine::host& hst(*hosts[i]);

    // update status of all hosts (scheduled or not).
    hst.update_status(false);

    // skip most hosts that shouldn't be scheduled.
    if (!hst.get_should_be_scheduled()) {
      // passive checks are an exception if a forced check was
      // scheduled before Centreon Engine was restarted.
      if (!(!hst.get_checks_enabled()
            && hst.get_next_check()
            && (hst.get_check_options() & CHECK_OPTION_FORCE_EXECUTION)))
        continue;
    }
    hosts_to_schedule.insert(std::make_pair(hst.get_next_check(), &hst));
  }

  // Schedule events list.
  for (std::multimap<time_t, com::centreon::engine::host*>::const_iterator
         it(hosts_to_schedule.begin()), end(hosts_to_schedule.end());
       it != end;
       ++it) {
    com::centreon::engine::host& hst(*it->second);

    // Schedule a new host check event.
    events::schedule(
              EVENT_HOST_CHECK,
              false,
              hst.get_next_check(),
              false,
              0,
              NULL,
              true,
              (void*)&hst,
              NULL,
              hst.get_check_options());
  }

  // Schedule acknowledgement expirations.
  logger(dbg_events, most)
    << "Scheduling host acknowledgement expirations...";
  for (int i(0), end(hosts.size()); i < end; ++i)
    if (hosts[i]->get_problem_has_been_acknowledged())
      hosts[i]->schedule_acknowledgement_expiration();
}

/**
 *  Schedule service events (checks notably).
 *
 *  @param[in] services  The list of services to schedule.
 */
void applier::scheduler::_schedule_service_events(
       std::vector<engine::service*> const& services) {
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
      engine::service& svc(*services[i]);
      if (interleave_block_index >= scheduling_info.service_interleave_factor) {
        ++current_interleave_block;
        interleave_block_index = 0;
      }

      // skip this service if it shouldn't be scheduled.
      if (!svc.get_should_be_scheduled())
        continue;

      int const mult_factor(
            current_interleave_block
            + ++interleave_block_index * total_interleave_blocks);

      // set the preferred next check time for the service.
      svc.set_next_check(
        (time_t)(now
             + mult_factor * scheduling_info.service_inter_check_delay));

      // Make sure the service can actually be scheduled when we want.
      {
        timezone_locker lock(svc.get_timezone());
        if (!check_time_against_period(
              svc.get_next_check(),
              svc.check_period_ptr)) {
          time_t next_valid_time(0);
          get_next_valid_time(
            svc.get_next_check(),
            &next_valid_time,
            svc.check_period_ptr);
          svc.set_next_check(next_valid_time);
        }
      }

      if (!scheduling_info.first_service_check
          || svc.get_next_check() < scheduling_info.first_service_check)
        scheduling_info.first_service_check = svc.get_next_check();
      if (svc.get_next_check() > scheduling_info.last_service_check)
        scheduling_info.last_service_check = svc.get_next_check();
    }
  }

  // Need to optimize add_event insert.
  std::multimap<time_t, engine::service*> services_to_schedule;

  // add scheduled service checks to event queue.
  for (unsigned int i(0); i < end; ++i) {
    engine::service& svc(*services[i]);

    // update status of all services (scheduled or not).
    svc.update_status(false);

    // skip most services that shouldn't be scheduled.
    if (!svc.get_should_be_scheduled()) {
      // passive checks are an exception if a forced check was
      // scheduled before Centreon Engine was restarted.
      if (!(!svc.get_checks_enabled()
            && svc.get_next_check()
            && (svc.get_check_options() & CHECK_OPTION_FORCE_EXECUTION)))
        continue;
    }
    services_to_schedule.insert(std::make_pair(svc.get_next_check(), &svc));
  }

  // Schedule events list.
  for (std::multimap<time_t, engine::service*>::const_iterator
         it(services_to_schedule.begin()),
         end(services_to_schedule.end());
       it != end;
       ++it) {
    engine::service& svc(*it->second);
    // Create a new service check event.
    events::schedule(
              EVENT_SERVICE_CHECK,
              false,
              svc.get_next_check(),
              false,
              0,
              NULL,
              true,
              (void*)&svc,
              NULL,
              svc.get_check_options());
  }

  // Schedule acknowledgement expirations.
  logger(dbg_events, most)
    << "Scheduling service acknowledgement expirations...";
  for (int i(0), end(services.size()); i < end; ++i)
    if (services[i]->get_problem_has_been_acknowledged())
      services[i]->schedule_acknowledgement_expiration();

  return ;
}

/**
 *  Unschedule host events.
 *
 *  @param[in] hosts  The list of hosts to unschedule.
 */
void applier::scheduler::_unschedule_host_events(
                           std::vector<com::centreon::engine::host*> const& hosts) {
  for (std::vector<com::centreon::engine::host*>::const_iterator
         it(hosts.begin()),
         end(hosts.end());
       it != end;
       ++it) {
    timed_event* evt(NULL);
    while ((evt = timed_event::find_event(timed_event::low,
                                          EVENT_HOST_CHECK,
                                          *it))) {
      remove_event(evt, timed_event::low);
      delete evt;
    }
    while ((evt = timed_event::find_event(timed_event::low,
                                          EVENT_EXPIRE_HOST_ACK,
                                          *it))) {
      remove_event(evt, timed_event::low);
      delete evt;
    }
  }
  return ;
}

/**
 *  Unschedule service events.
 *
 *  @param[in] services  The list of services to unschedule.
 */
void applier::scheduler::_unschedule_service_events(
                           std::vector<engine::service*> const& services) {
  for (std::vector<engine::service*>::const_iterator
         it(services.begin()),
         end(services.end());
       it != end;
       ++it) {
    timed_event* evt(NULL);
    while ((evt = timed_event::find_event(timed_event::low,
                                          EVENT_SERVICE_CHECK,
                                          *it))) {
      remove_event(evt, timed_event::low);
      delete evt;
    }
    while ((evt = timed_event::find_event(timed_event::low,
                                          EVENT_EXPIRE_SERVICE_ACK,
                                          *it))) {
      remove_event(evt, timed_event::low);
      delete evt;
    }
  }
  return ;
}

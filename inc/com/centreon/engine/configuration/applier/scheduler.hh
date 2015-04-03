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

#ifndef CCE_CONFIGURATION_APPLIER_SCHEDULER_HH
#  define CCE_CONFIGURATION_APPLIER_SCHEDULER_HH

#  include <vector>
#  include "com/centreon/engine/configuration/applier/difference.hh"
#  include "com/centreon/engine/configuration/state.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/unordered_hash.hh"

// Forward declaration.
struct host_struct;
struct service_struct;
struct timed_event_struct;

CCE_BEGIN()

namespace                 configuration {
  namespace               applier {
    /**
     *  @class scheduler scheduler.hh
     *  @brief Simple configuration applier for scheduler class.
     *
     *  Simple configuration applier for scheduler class.
     */
    class                 scheduler {
    public:
      static int const    auto_rescheduling_interval = 5 * 60;

      void                apply(
                            state& config,
                            difference<set_host> const& diff_hosts,
                            difference<set_service> const& diff_services);
      static scheduler&   instance();
      static void         load();
      void                remove_host(configuration::host const& h);
      void                remove_service(configuration::service const& s);
      static void         unload();

    private:
                          scheduler();
                          scheduler(scheduler const&);
                          ~scheduler() throw ();
      scheduler&          operator=(scheduler const&);
      void                _apply_misc_event();
      void                _calculate_host_inter_check_delay();
      void                _calculate_host_scheduling_params(
                            configuration::state const& config);
      void                _calculate_service_inter_check_delay();
      void                _calculate_service_interleave_factor();
      void                _calculate_service_scheduling_params(
                            configuration::state const& config);
      timed_event_struct* _create_misc_event(
                            int type,
                            time_t start,
                            unsigned long interval,
                            void* data = NULL);
      void                _get_hosts(
                            set_host const& hst_added,
                            std::vector<host_struct*>& new_hosts,
                            bool throw_if_not_found = true);
      void                _get_services(
                            set_service const& svc_added,
                            std::vector<service_struct*>& new_services,
                            bool throw_if_not_found = true);
      void                _remove_misc_event(timed_event_struct*& evt);
      void                _schedule_host_checks(
                            std::vector<host_struct*> const& hosts);
      void                _schedule_service_checks(
                            std::vector<service_struct*> const& services);
      void                _unschedule_host_checks(
                            std::vector<host_struct*> const& hosts);
      void                _unschedule_service_checks(
                            std::vector<service_struct*> const& services);

      timed_event_struct* _evt_check_reaper;
      timed_event_struct* _evt_command_check;
      timed_event_struct* _evt_hfreshness_check;
      timed_event_struct* _evt_reschedule_checks;
      timed_event_struct* _evt_retention_save;
      timed_event_struct* _evt_sfreshness_check;
      timed_event_struct* _evt_status_save;
      unsigned int        _old_check_reaper_interval;
      int                 _old_command_check_interval;
      unsigned int        _old_host_freshness_check_interval;
      unsigned int        _old_retention_update_interval;
      unsigned int        _old_service_freshness_check_interval;
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_SCHEDULER_HH

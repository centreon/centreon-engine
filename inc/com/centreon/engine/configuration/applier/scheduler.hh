/*
** Copyright 2011-2013 Merethis
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
      void                apply(
                            state& config,
                            difference<set_host> const& diff_hosts,
                            difference<set_service> const& diff_services);
      static scheduler&   instance();
      static void         load();
      static void         unload();

    private:
                          scheduler();
                          scheduler(scheduler const&);
                          ~scheduler() throw ();
      scheduler&          operator=(scheduler const&);
      void                _apply_misc_event();
      void                _calculate_host_inter_check_delay(
                            configuration::state::inter_check_delay method);
      void                _calculate_host_scheduling_params(
                            configuration::state const& config);
      void                _calculate_service_inter_check_delay(
                            configuration::state::inter_check_delay method);
      void                _calculate_service_interleave_factor(
                            configuration::state::interleave_factor method);
      void                _calculate_service_scheduling_params(
                            configuration::state const& config);
      timed_event_struct* _create_misc_event(
                            int type,
                            time_t start,
                            unsigned long interval,
                            void* data = NULL);
      void                _get_new_hosts(
                            set_host const& hst_added,
                            std::vector<host_struct*>& new_hosts);
      void                _get_new_services(
                            set_service const& svc_added,
                            std::vector<service_struct*>& new_services);
      void                _remove_misc_event(timed_event_struct*& evt);
      void                _scheduling_host_checks(
                            std::vector<host_struct*> const& hosts);
      void                _scheduling_service_checks(
                            std::vector<service_struct*> const& services);
      void                _unscheduling_host_checks(
                            set_host const& hosts);
      void                _unscheduling_service_checks(
                            set_service const& services);

      timed_event_struct* _evt_check_reaper;
      timed_event_struct* _evt_command_check;
      timed_event_struct* _evt_hfreshness_check;
      umap<std::string, timed_event_struct*>
                          _evt_host_check;
      timed_event_struct* _evt_host_perfdata;
      timed_event_struct* _evt_orphan_check;
      timed_event_struct* _evt_reschedule_checks;
      timed_event_struct* _evt_retention_save;
      timed_event_struct* _evt_sfreshness_check;
      umap<std::pair<std::string, std::string>, timed_event_struct*>
                          _evt_service_check;
      timed_event_struct* _evt_service_perfdata;
      timed_event_struct* _evt_status_save;
      unsigned int        _old_auto_rescheduling_interval;
      unsigned int        _old_check_reaper_interval;
      int                 _old_command_check_interval;
      unsigned int        _old_host_freshness_check_interval;
      std::string         _old_host_perfdata_file_processing_command;
      unsigned int        _old_host_perfdata_file_processing_interval;
      unsigned int        _old_retention_update_interval;
      unsigned int        _old_service_freshness_check_interval;
      std::string         _old_service_perfdata_file_processing_command;
      unsigned int        _old_service_perfdata_file_processing_interval;
      unsigned int        _old_status_update_interval;
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_SCHEDULER_HH

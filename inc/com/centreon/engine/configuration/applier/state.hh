/*
** Copyright 2011-2019 Centreon
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

#ifndef CCE_CONFIGURATION_APPLIER_STATE_HH
#  define CCE_CONFIGURATION_APPLIER_STATE_HH

#  include <memory>
#  include <string>
#  include <utility>
#  include "com/centreon/concurrency/condvar.hh"
#  include "com/centreon/concurrency/mutex.hh"
#  include "com/centreon/engine/configuration/applier/difference.hh"
#  include "com/centreon/engine/configuration/state.hh"
#  include "com/centreon/engine/hostdependency.hh"
#  include "com/centreon/engine/hostescalation.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/engine/servicedependency.hh"
#  include "com/centreon/engine/timeperiod.hh"

CCE_BEGIN()
class serviceescalation;

// Forward declaration.
namespace commands {
  class   command;
  class   connector;
  class   service;
}

namespace retention {
  class   state;
}

namespace           configuration {
  namespace         applier {
    /**
     *  @class state state.hh
     *  @brief Simple configuration applier for state class.
     *
     *  Simple configuration applier for state class.
     */
    class           state {
    public:
      void          apply(
                      configuration::state& new_cfg,
                      bool waiting_thread = false);
      void          apply(
                      configuration::state& new_cfg,
                      retention::state& state,
                      bool waiting_thread = false);
      static state& instance();
      static void   load();
      static void   unload();

      engine::contact*
                    find_contact(configuration::contact::key_type const& k);
      engine::contactgroup*
                    find_contactgroup(configuration::contactgroup::key_type const& k);
      contactgroup_map const&
                    contactgroups() const throw ();
      contactgroup_map&
                    contactgroups() throw ();
      contactgroup_map::iterator
                    contactgroups_find(configuration::contactgroup::key_type const& k);
      hostdependency_mmap const&
                    hostdependencies() const throw ();
      hostdependency_mmap&
                    hostdependencies() throw ();
      hostdependency_mmap::iterator
                    hostdependencies_find(configuration::hostdependency::key_type const& k);
      servicedependency_mmap const&
                    servicedependencies() const throw ();
      servicedependency_mmap&
                    servicedependencies() throw ();
      servicedependency_mmap::iterator
                    servicedependencies_find(configuration::servicedependency::key_type const& k);
      timeperiod_map const&
                    timeperiods() const throw ();
      timeperiod_map &
                    timeperiods() throw ();
      timeperiod_map::iterator
                    timeperiods_find(configuration::timeperiod::key_type const& k);
      std::unordered_map<std::string, std::string>&
                    user_macros();
      std::unordered_map<std::string, std::string>::const_iterator
                    user_macros_find(std::string const& key) const;
      void          try_lock();

    private:
      enum          processing_state {
        state_waiting,
        state_apply,
        state_error,
        state_ready
      };

                    state();
                    state(state const&);
                    ~state() throw ();
      state&        operator=(state const&);
      void          _apply(configuration::state const& new_cfg);
      template      <typename ConfigurationType, typename ApplierType>
      void          _apply(
                      difference<std::set<ConfigurationType>> const& diff);
      void          _apply(
                      configuration::state& new_cfg,
                      retention::state& state);
      template      <typename ConfigurationType, typename ApplierType>
      void          _expand(configuration::state& new_state);
      void          _processing(
                      configuration::state& new_cfg,
                      bool waiting_thread,
                      retention::state* state = NULL);
      template      <typename ConfigurationType,
                     typename ApplierType>
      void          _resolve(
                      std::set<ConfigurationType>& cfg);

      state*        _config;

      std::unordered_map<std::string, std::shared_ptr<com::centreon::engine::contactgroup>>
                    _contactgroups;
      concurrency::condvar
                    _cv_lock;
      hostdependency_mmap
                    _hostdependencies;
      hostgroup_map _hostgroups;
      concurrency::mutex
                    _lock;
      processing_state
                    _processing_state;

      servicedependency_mmap
                    _servicedependencies;
      servicegroup_map
                    _servicegroups;
      timeperiod_map
                    _timeperiods;
      std::unordered_map<std::string, std::string>
                    _user_macros;
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_STATE_HH

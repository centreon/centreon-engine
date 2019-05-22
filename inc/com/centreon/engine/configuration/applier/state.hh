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

// Forward declaration.
struct service_struct;
struct servicedependency_struct;
struct serviceescalation_struct;
struct servicegroup_struct;
struct timeperiod_struct;

CCE_BEGIN()

// Forward declaration.
namespace commands {
  class   command;
  class   connector;
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

      std::unordered_map<std::string, std::shared_ptr<commands::command>> const&
                    commands() const throw ();
      std::unordered_map<std::string, std::shared_ptr<commands::command>>&
                    commands() throw ();
      commands::command const*
                    find_command(configuration::command::key_type const& k) const;
      commands::command*
                    find_command(configuration::command::key_type const& k);
      engine::contact const*
                    find_contact(configuration::contact::key_type const& k) const;
      engine::contact*
                    find_contact(configuration::contact::key_type const& k);
      engine::contactgroup const*
                    find_contactgroup(configuration::contactgroup::key_type const& k) const;
      engine::contactgroup*
                    find_contactgroup(configuration::contactgroup::key_type const& k);
      std::unordered_map<std::string, std::shared_ptr<commands::connector>> const&
                    connectors() const throw ();
      std::unordered_map<std::string, std::shared_ptr<commands::connector>>&
                    connectors() throw ();
      commands::connector const*
                    find_connector(configuration::connector::key_type const& k) const;
      commands::connector*
                    find_connector(configuration::connector::key_type const& k);
      contact_map const&
                    contacts() const throw ();
      contact_map&
                    contacts() throw ();
      contact_map::const_iterator
                    contacts_find(configuration::contact::key_type const& k) const;
      contact_map::iterator
                    contacts_find(configuration::contact::key_type const& k);
      contactgroup_map const&
                    contactgroups() const throw ();
      contactgroup_map&
                    contactgroups() throw ();
      contactgroup_map::const_iterator
                    contactgroups_find(configuration::contactgroup::key_type const& k) const;
      contactgroup_map::iterator
                    contactgroups_find(configuration::contactgroup::key_type const& k);
      std::unordered_map<unsigned int, std::shared_ptr<com::centreon::engine::host>> const&
                    hosts() const throw ();
      std::unordered_map<unsigned int, std::shared_ptr<com::centreon::engine::host>>&
                    hosts() throw ();
      std::unordered_map<unsigned int, std::shared_ptr<com::centreon::engine::host>>::const_iterator
                    hosts_find(configuration::host::key_type const& k) const;
      std::unordered_map<unsigned int, std::shared_ptr<com::centreon::engine::host>>::iterator
                    hosts_find(configuration::host::key_type const& k);
      hostdependency_mmap const&
                    hostdependencies() const throw ();
      hostdependency_mmap&
                    hostdependencies() throw ();
      hostdependency_mmap::const_iterator
                    hostdependencies_find(configuration::hostdependency::key_type const& k) const;
      hostdependency_mmap::iterator
                    hostdependencies_find(configuration::hostdependency::key_type const& k);
      hostescalation_mmap const&
                    hostescalations() const throw ();
      hostescalation_mmap&
                    hostescalations() throw ();
      hostescalation_mmap::const_iterator
                    hostescalations_find(configuration::hostescalation::key_type const& k) const;
      hostescalation_mmap::iterator
                    hostescalations_find(configuration::hostescalation::key_type const& k);
      hostgroup_map const&
                    hostgroups() const throw ();
      hostgroup_map&
                    hostgroups() throw ();
      hostgroup_map::const_iterator
                    hostgroups_find(configuration::hostgroup::key_type const& k) const;
      hostgroup_map::iterator
                    hostgroups_find(configuration::hostgroup::key_type const& k);
      std::unordered_map<std::pair<unsigned int, unsigned int>, std::shared_ptr<service_struct>> const&
                    services() const throw ();
      std::unordered_map<std::pair<unsigned int, unsigned int>, std::shared_ptr<service_struct>>&
                    services() throw ();
      std::unordered_map<std::pair<unsigned int, unsigned int>, std::shared_ptr<service_struct>>::const_iterator
                    services_find(configuration::service::key_type const& k) const;
      std::unordered_map<std::pair<unsigned int, unsigned int>, std::shared_ptr<service_struct>>::iterator
                    services_find(configuration::service::key_type const& k);
      umultimap<std::pair<std::string, std::string>, std::shared_ptr<servicedependency_struct>> const&
                    servicedependencies() const throw ();
      umultimap<std::pair<std::string, std::string>, std::shared_ptr<servicedependency_struct>>&
                    servicedependencies() throw ();
      umultimap<std::pair<std::string, std::string>, std::shared_ptr<servicedependency_struct>>::const_iterator
                    servicedependencies_find(configuration::servicedependency::key_type const& k) const;
      umultimap<std::pair<std::string, std::string>, std::shared_ptr<servicedependency_struct>>::iterator
                    servicedependencies_find(configuration::servicedependency::key_type const& k);
      umultimap<std::pair<std::string, std::string>, std::shared_ptr<serviceescalation_struct>> const&
                    serviceescalations() const throw ();
      umultimap<std::pair<std::string, std::string>, std::shared_ptr<serviceescalation_struct>>&
                    serviceescalations() throw ();
      umultimap<std::pair<std::string, std::string>, std::shared_ptr<serviceescalation_struct>>::const_iterator
                    serviceescalations_find(configuration::serviceescalation::key_type const& k) const;
      umultimap<std::pair<std::string, std::string>, std::shared_ptr<serviceescalation_struct>>::iterator
                    serviceescalations_find(configuration::serviceescalation::key_type const& k);
      std::unordered_map<std::string, std::shared_ptr<servicegroup_struct>> const&
                    servicegroups() const throw ();
      std::unordered_map<std::string, std::shared_ptr<servicegroup_struct>>&
                    servicegroups() throw ();
      std::unordered_map<std::string, std::shared_ptr<servicegroup_struct>>::const_iterator
                    servicegroups_find(configuration::servicegroup::key_type const& k) const;
      std::unordered_map<std::string, std::shared_ptr<servicegroup_struct>>::iterator
                    servicegroups_find(configuration::servicegroup::key_type const& k);
      std::unordered_map<std::string, std::shared_ptr<timeperiod_struct>> const&
                    timeperiods() const throw ();
      std::unordered_map<std::string, std::shared_ptr<timeperiod_struct>>&
                    timeperiods() throw ();
      std::unordered_map<std::string, std::shared_ptr<timeperiod_struct>>::const_iterator
                    timeperiods_find(configuration::timeperiod::key_type const& k) const;
      std::unordered_map<std::string, std::shared_ptr<timeperiod_struct>>::iterator
                    timeperiods_find(configuration::timeperiod::key_type const& k);
      std::unordered_map<std::string, std::string>&
                    user_macros();
      std::unordered_map<std::string, std::string> const&
                    user_macros() const;
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

      std::unordered_map<std::string, std::shared_ptr<commands::command>>
                    _commands;
      std::unordered_map<std::string, std::shared_ptr<commands::connector>>
                    _connectors;
      std::unordered_map<std::string, std::shared_ptr<com::centreon::engine::contact>>
                    _contacts;
      std::unordered_map<std::string, std::shared_ptr<com::centreon::engine::contactgroup>>
                    _contactgroups;
      concurrency::condvar
                    _cv_lock;
      std::unordered_map<unsigned int, std::shared_ptr<com::centreon::engine::host>>
                    _hosts;
      hostdependency_mmap
                    _hostdependencies;
      hostescalation_mmap
                    _hostescalations;
      hostgroup_map _hostgroups;
      concurrency::mutex
                    _lock;
      processing_state
                    _processing_state;
      std::unordered_map<std::pair<unsigned int, unsigned int>,
                         std::shared_ptr<service_struct>>
                    _services;
      umultimap<std::pair<std::string, std::string>, std::shared_ptr<servicedependency_struct>>
                    _servicedependencies;
      umultimap<std::pair<std::string, std::string>, std::shared_ptr<serviceescalation_struct>>
                    _serviceescalations;
      std::unordered_map<std::string, std::shared_ptr<servicegroup_struct>>
                    _servicegroups;
      std::unordered_map<std::string, std::shared_ptr<timeperiod_struct>>
                    _timeperiods;
      std::unordered_map<std::string, std::string>
                    _user_macros;
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_STATE_HH

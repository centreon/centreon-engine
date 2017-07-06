/*
** Copyright 2011-2013,2017 Centreon
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

#  include <string>
#  include <utility>
#  include "com/centreon/concurrency/condvar.hh"
#  include "com/centreon/concurrency/mutex.hh"
#  include "com/centreon/engine/configuration/applier/difference.hh"
#  include "com/centreon/engine/configuration/state.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/shared_ptr.hh"

// Forward declaration.
struct command_struct;
struct contact_struct;
struct contactgroup_struct;
struct host_struct;
struct hostdependency_struct;
struct hostescalation_struct;
struct hostgroup_struct;
struct service_struct;
struct servicedependency_struct;
struct serviceescalation_struct;
struct servicegroup_struct;
struct timeperiod_struct;

CCE_BEGIN()

// Forward declaration.
namespace commands {
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

      umap<std::string, shared_ptr<command_struct> > const&
                    commands() const throw ();
      umap<std::string, shared_ptr<command_struct> >&
                    commands() throw ();
      umap<std::string, shared_ptr<command_struct> >::const_iterator
                    commands_find(configuration::command::key_type const& k) const;
      umap<std::string, shared_ptr<command_struct> >::iterator
                    commands_find(configuration::command::key_type const& k);
      umap<std::string, shared_ptr<commands::connector> > const&
                    connectors() const throw ();
      umap<std::string, shared_ptr<commands::connector> >&
                    connectors() throw ();
      umap<std::string, shared_ptr<commands::connector> >::const_iterator
                    connectors_find(configuration::connector::key_type const& k) const;
      umap<std::string, shared_ptr<commands::connector> >::iterator
                    connectors_find(configuration::connector::key_type const& k);
      umap<std::string, shared_ptr<contact_struct> > const&
                    contacts() const throw ();
      umap<std::string, shared_ptr<contact_struct> >&
                    contacts() throw ();
      umap<std::string, shared_ptr<contact_struct> >::const_iterator
                    contacts_find(configuration::contact::key_type const& k) const;
      umap<std::string, shared_ptr<contact_struct> >::iterator
                    contacts_find(configuration::contact::key_type const& k);
      umap<std::string, shared_ptr<contactgroup_struct> > const&
                    contactgroups() const throw ();
      umap<std::string, shared_ptr<contactgroup_struct> >&
                    contactgroups() throw ();
      umap<std::string, shared_ptr<contactgroup_struct> >::const_iterator
                    contactgroups_find(configuration::contactgroup::key_type const& k) const;
      umap<std::string, shared_ptr<contactgroup_struct> >::iterator
                    contactgroups_find(configuration::contactgroup::key_type const& k);
      umap<std::string, shared_ptr<host_struct> > const&
                    hosts() const throw ();
      umap<std::string, shared_ptr<host_struct> >&
                    hosts() throw ();
      umap<std::string, shared_ptr<host_struct> >::const_iterator
                    hosts_find(configuration::host::key_type const& k) const;
      umap<std::string, shared_ptr<host_struct> >::iterator
                    hosts_find(configuration::host::key_type const& k);
      umultimap<std::string, shared_ptr<hostdependency_struct> > const&
                    hostdependencies() const throw ();
      umultimap<std::string, shared_ptr<hostdependency_struct> >&
                    hostdependencies() throw ();
      umultimap<std::string, shared_ptr<hostdependency_struct> >::const_iterator
                    hostdependencies_find(configuration::hostdependency::key_type const& k) const;
      umultimap<std::string, shared_ptr<hostdependency_struct> >::iterator
                    hostdependencies_find(configuration::hostdependency::key_type const& k);
      umultimap<std::string, shared_ptr<hostescalation_struct> > const&
                    hostescalations() const throw ();
      umultimap<std::string, shared_ptr<hostescalation_struct> >&
                    hostescalations() throw ();
      umultimap<std::string, shared_ptr<hostescalation_struct> >::const_iterator
                    hostescalations_find(configuration::hostescalation::key_type const& k) const;
      umultimap<std::string, shared_ptr<hostescalation_struct> >::iterator
                    hostescalations_find(configuration::hostescalation::key_type const& k);
      umap<std::string, shared_ptr<hostgroup_struct> > const&
                    hostgroups() const throw ();
      umap<std::string, shared_ptr<hostgroup_struct> >&
                    hostgroups() throw ();
      umap<std::string, shared_ptr<hostgroup_struct> >::const_iterator
                    hostgroups_find(configuration::hostgroup::key_type const& k) const;
      umap<std::string, shared_ptr<hostgroup_struct> >::iterator
                    hostgroups_find(configuration::hostgroup::key_type const& k);
      umap<std::pair<std::string, std::string>, shared_ptr<service_struct> > const&
                    services() const throw ();
      umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >&
                    services() throw ();
      umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
                    services_find(configuration::service::key_type const& k) const;
      umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::iterator
                    services_find(configuration::service::key_type const& k);
      umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> > const&
                    servicedependencies() const throw ();
      umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >&
                    servicedependencies() throw ();
      umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >::const_iterator
                    servicedependencies_find(configuration::servicedependency::key_type const& k) const;
      umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >::iterator
                    servicedependencies_find(configuration::servicedependency::key_type const& k);
      umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> > const&
                    serviceescalations() const throw ();
      umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> >&
                    serviceescalations() throw ();
      umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> >::const_iterator
                    serviceescalations_find(configuration::serviceescalation::key_type const& k) const;
      umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> >::iterator
                    serviceescalations_find(configuration::serviceescalation::key_type const& k);
      umap<std::string, shared_ptr<servicegroup_struct> > const&
                    servicegroups() const throw ();
      umap<std::string, shared_ptr<servicegroup_struct> >&
                    servicegroups() throw ();
      umap<std::string, shared_ptr<servicegroup_struct> >::const_iterator
                    servicegroups_find(configuration::servicegroup::key_type const& k) const;
      umap<std::string, shared_ptr<servicegroup_struct> >::iterator
                    servicegroups_find(configuration::servicegroup::key_type const& k);
      umap<std::string, shared_ptr<timeperiod_struct> > const&
                    timeperiods() const throw ();
      umap<std::string, shared_ptr<timeperiod_struct> >&
                    timeperiods() throw ();
      umap<std::string, shared_ptr<timeperiod_struct> >::const_iterator
                    timeperiods_find(configuration::timeperiod::key_type const& k) const;
      umap<std::string, shared_ptr<timeperiod_struct> >::iterator
                    timeperiods_find(configuration::timeperiod::key_type const& k);
      umap<std::string, std::string>&
                    user_macros();
      umap<std::string, std::string> const&
                    user_macros() const;
      umap<std::string, std::string>::const_iterator
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
                      difference<std::set<ConfigurationType> > const& diff);
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

      umap<std::string, shared_ptr<command_struct> >
                    _commands;
      umap<std::string, shared_ptr<commands::connector> >
                    _connectors;
      umap<std::string, shared_ptr<contact_struct> >
                    _contacts;
      umap<std::string, shared_ptr<contactgroup_struct> >
                    _contactgroups;
      concurrency::condvar
                    _cv_lock;
      umap<std::string, shared_ptr<host_struct> >
                    _hosts;
      umultimap<std::string, shared_ptr<hostdependency_struct> >
                    _hostdependencies;
      umultimap<std::string, shared_ptr<hostescalation_struct> >
                    _hostescalations;
      umap<std::string, shared_ptr<hostgroup_struct> >
                    _hostgroups;
      concurrency::mutex
                    _lock;
      processing_state
                    _processing_state;
      umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >
                    _services;
      umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >
                    _servicedependencies;
      umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> >
                    _serviceescalations;
      umap<std::string, shared_ptr<servicegroup_struct> >
                    _servicegroups;
      umap<std::string, shared_ptr<timeperiod_struct> >
                    _timeperiods;
      umap<std::string, std::string>
                    _user_macros;
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_STATE_HH

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
struct host_struct;
struct hostdependency_struct;
struct service_struct;
struct servicedependency_struct;
struct timeperiod_struct;
struct scheduled_downtime_struct;

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
      umap<std::string, shared_ptr<timeperiod_struct> > const&
                    timeperiods() const throw ();
      umap<std::string, shared_ptr<timeperiod_struct> >&
                    timeperiods() throw ();
      umap<std::string, shared_ptr<timeperiod_struct> >::const_iterator
                    timeperiods_find(configuration::timeperiod::key_type const& k) const;
      umap<std::string, shared_ptr<timeperiod_struct> >::iterator
                    timeperiods_find(configuration::timeperiod::key_type const& k);
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
                      difference<std::set<shared_ptr<ConfigurationType> > > const& diff);
      void          _apply(
                      configuration::state& new_cfg,
                      retention::state& state);
      void          _expand(configuration::state& new_state);
      template      <typename ConfigurationType,
                     typename ApplierType>
      void          _expand(
                      configuration::state& new_state,
                      std::set<shared_ptr<ConfigurationType> >& cfg);
      void          _processing(
                      configuration::state& new_cfg,
                      bool waiting_thread,
                      retention::state* state = NULL);
      template      <typename ConfigurationType,
                     typename ApplierType>
      void          _resolve(
                      std::set<shared_ptr<ConfigurationType> >& cfg);

      state*        _config;

      umap<std::string, shared_ptr<command_struct> >
                    _commands;
      umap<std::string, shared_ptr<commands::connector> >
                    _connectors;
      concurrency::condvar
                    _cv_lock;
      umap<std::string, shared_ptr<host_struct> >
                    _hosts;
      umultimap<std::string, shared_ptr<hostdependency_struct> >
                    _hostdependencies;
      concurrency::mutex
                    _lock;
      processing_state
                    _processing_state;
      umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >
                    _services;
      umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >
                    _servicedependencies;
      umap<std::string, shared_ptr<timeperiod_struct> >
                    _timeperiods;
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_STATE_HH

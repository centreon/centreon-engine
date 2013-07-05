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

#ifndef CCE_CONFIGURATION_APPLIER_STATE_HH
#  define CCE_CONFIGURATION_APPLIER_STATE_HH

#  include <string>
#  include <utility>
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
      void          apply(configuration::state& new_cfg);
      static state& instance();
      static void   load();
      static void   unload();

      umap<std::string, shared_ptr<command_struct> > const&
                    commands() const throw ();
      umap<std::string, shared_ptr<command_struct> >&
                    commands() throw ();
      umap<std::string, shared_ptr<commands::connector> > const&
                    connectors() const throw ();
      umap<std::string, shared_ptr<commands::connector> >&
                    connectors() throw ();
      umap<std::string, shared_ptr<contact_struct> > const&
                    contacts() const throw ();
      umap<std::string, shared_ptr<contact_struct> >&
                    contacts() throw ();
      umap<std::string, shared_ptr<contactgroup_struct> > const&
                    contactgroups() const throw ();
      umap<std::string, shared_ptr<contactgroup_struct> >&
                    contactgroups() throw ();
      umap<std::string, shared_ptr<host_struct> > const&
                    hosts() const throw ();
      umap<std::string, shared_ptr<host_struct> >&
                    hosts() throw ();
      umap<std::string, shared_ptr<host_struct> >::iterator
                    hosts_find(configuration::host::key_type const& k);
      umap<std::string, shared_ptr<host_struct> >::const_iterator
                    hosts_find(configuration::host::key_type const& k) const;
      umultimap<std::string, shared_ptr<hostdependency_struct> > const&
                    hostdependencies() const throw ();
      umultimap<std::string, shared_ptr<hostdependency_struct> >&
                    hostdependencies() throw ();
      umultimap<std::string, shared_ptr<hostescalation_struct> > const&
                    hostescalations() const throw ();
      umultimap<std::string, shared_ptr<hostescalation_struct> >&
                    hostescalations() throw ();
      umap<std::string, shared_ptr<hostgroup_struct> > const&
                    hostgroups() const throw ();
      umap<std::string, shared_ptr<hostgroup_struct> >&
                    hostgroups() throw ();
      umap<std::pair<std::string, std::string>, shared_ptr<service_struct> > const&
                    services() const throw ();
      umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >&
                    services() throw ();
      umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> > const&
                    servicedependencies() const throw ();
      umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >&
                    servicedependencies() throw ();
      umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> > const&
                    serviceescalations() const throw ();
      umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> >&
                    serviceescalations() throw ();
      umap<std::string, shared_ptr<servicegroup_struct> > const&
                    servicegroups() const throw ();
      umap<std::string, shared_ptr<servicegroup_struct> >&
                    servicegroups() throw ();
      umap<std::string, shared_ptr<timeperiod_struct> > const&
                    timeperiods() const throw ();
      umap<std::string, shared_ptr<timeperiod_struct> >&
                    timeperiods() throw ();

    private:
                    state();
                    state(state const&);
                    ~state() throw ();
      state&        operator=(state const&);
      template      <typename ConfigurationType, typename ApplierType>
      void          _apply(
                      difference<std::set<shared_ptr<ConfigurationType> > > const& diff);
      void          _expand(configuration::state& new_state);
      template      <typename ConfigurationType,
                     typename ApplierType>
      void          _expand(
                      configuration::state& new_state,
                      std::set<shared_ptr<ConfigurationType> >& cfg);
      template      <typename ConfigurationType,
                     typename ApplierType>
      void          _resolve(
                      std::set<shared_ptr<ConfigurationType> >& cfg);

      state*        _config;

      umap<std::string, shared_ptr<command_struct> >
                    _commands;
      umap<std::string, shared_ptr<commands::connector> >
                    _connectors;
      umap<std::string, shared_ptr<contact_struct> >
                    _contacts;
      umap<std::string, shared_ptr<contactgroup_struct> >
                    _contactgroups;
      umap<std::string, shared_ptr<host_struct> >
                    _hosts;
      umultimap<std::string, shared_ptr<hostdependency_struct> >
                    _hostdependencies;
      umultimap<std::string, shared_ptr<hostescalation_struct> >
                    _hostescalations;
      umap<std::string, shared_ptr<hostgroup_struct> >
                    _hostgroups;
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
    };
  }
}

CCE_END()

#endif // !CCE_CONFIGURATION_APPLIER_STATE_HH


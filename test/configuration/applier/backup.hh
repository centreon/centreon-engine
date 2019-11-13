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

#ifndef CCE_TEST_CONFIGURATION_APPLIER_BACKUP_HH
#define CCE_TEST_CONFIGURATION_APPLIER_BACKUP_HH

#include "chkdiff.hh"
#include "com/centreon/engine/configuration/state.hh"

namespace cc = com::centreon;
namespace cce = cc::engine;

class backup {
 public:
  backup() : _config(NULL) {}
  backup(cce::configuration::state const& config,
         cce::configuration::applier::state const& state)
      : _config(new cce::configuration::state(config)) {
    _commands = state.commands();
    _connectors = state.connectors();
    _contacts = state.contacts();
    _contactgroups = state.contactgroups();
    _hosts = state.hosts();
    _hostdependencies = state.hostdependencies();
    _hostescalations = state.hostescalations();
    _hostgroups = state.hostgroups();
    _services = state.services();
    _servicedependencies = state.servicedependencies();
    _serviceescalations = state.serviceescalations();
    _servicegroups = state.servicegroups();
    _timeperiods = state.timeperiods();
  }
  backup(backup const& right) { operator=(right); }
  ~backup() { delete _config; }
  backup& operator=(backup const& right) {
    if (this != &right) {
      delete _config;
      _config = new cce::configuration::state(*right._config);
      _commands = right._commands;
      _connectors = right._connectors;
      _contacts = right._contacts;
      _contactgroups = right._contactgroups;
      _hosts = right._hosts;
      _hostdependencies = right._hostdependencies;
      _hostescalations = right._hostescalations;
      _hostgroups = right._hostgroups;
      _services = right._services;
      _servicedependencies = right._servicedependencies;
      _serviceescalations = right._serviceescalations;
      _servicegroups = right._servicegroups;
      _timeperiods = right._timeperiods;
    }
    return (*this);
  }
  void is_equal(cce::configuration::state const& config) {
    if (*_config != config)
      throw(engine_error() << "state are not equal");
  }
  void is_equal(cce::configuration::applier::state const& state) {
    reset_next_check(_hosts);
    reset_next_check(_services);
    reset_next_check(state.hosts());
    reset_next_check(state.services());

    if (!compare_with_true_contents(_commands, state.commands()))
      throw(engine_error() << "commands are not equal");
    if (!compare_with_true_contents(_connectors, state.connectors()))
      throw(engine_error() << "connectors are not equal");
    if (!compare_with_true_contents(_contacts, state.contacts()))
      throw(engine_error() << "contacts are not equal");
    if (!compare_with_true_contents(_contactgroups, state.contactgroups()))
      throw(engine_error() << "contactgroups are not equal");
    if (!compare_with_true_contents(_hosts, state.hosts()))
      throw(engine_error() << "hosts are not equal");
    if (!compare_with_true_contents(_hostdependencies,
                                    state.hostdependencies()))
      throw(engine_error() << "hostdependencies are not equal");
    if (!compare_with_true_contents(_hostescalations, state.hostescalations()))
      throw(engine_error() << "hostescalations are not equal");
    if (!compare_with_true_contents(_hostgroups, state.hostgroups()))
      throw(engine_error() << "hostgroups are not equal");
    if (!compare_with_true_contents(_services, state.services()))
      throw(engine_error() << "services are not equal");
    if (!compare_with_true_contents(_servicedependencies,
                                    state.servicedependencies()))
      throw(engine_error() << "servicedependencies are not equal");
    if (!compare_with_true_contents(_serviceescalations,
                                    state.serviceescalations()))
      throw(engine_error() << "serviceescalations are not equal");
    if (!compare_with_true_contents(_servicegroups, state.servicegroups()))
      throw(engine_error() << "servicegroups are not equal");
    if (!compare_with_true_contents(_timeperiods, state.timeperiods()))
      throw(engine_error() << "timeperiods are not equal");
  }
  static void set_to_null() {
    ::command_list = NULL;
    ::contact_list = NULL;
    ::contactgroup_list = NULL;
    ::host_list = NULL;
    ::hostdependency_list = NULL;
    ::hostescalation_list = NULL;
    ::hostgroup_list = NULL;
    ::service_list = NULL;
    ::servicedependency_list = NULL;
    ::serviceescalation_list = NULL;
    ::servicegroup_list = NULL;
    ::timeperiod_list = NULL;
  }

 private:
  umap<std::string, std::shared_ptr<command_struct> > _commands;
  cce::configuration::state* _config;
  umap<std::string, std::shared_ptr<cce::commands::connector> > _connectors;
  umap<std::string, std::shared_ptr<contact_struct> > _contacts;
  umap<std::string, std::shared_ptr<contactgroup_struct> > _contactgroups;
  umap<std::string, std::shared_ptr<host_struct> > _hosts;
  umultimap<std::string, std::shared_ptr<hostdependency_struct> >
      _hostdependencies;
  umultimap<std::string, std::shared_ptr<hostescalation_struct> >
      _hostescalations;
  umap<std::string, std::shared_ptr<hostgroup_struct> > _hostgroups;
  umap<std::pair<std::string, std::string>, std::shared_ptr<engine::service> >
      _services;
  umultimap<std::pair<std::string, std::string>,
            std::shared_ptr<servicedependency_struct> >
      _servicedependencies;
  umultimap<std::pair<std::string, std::string>,
            std::shared_ptr<serviceescalation_struct> >
      _serviceescalations;
  umap<std::string, std::shared_ptr<servicegroup_struct> > _servicegroups;
  umap<std::string, std::shared_ptr<timeperiod_struct> > _timeperiods;
};

#endif  // !CCE_TEST_CONFIGURATION_APPLIER_BACKUP_HH

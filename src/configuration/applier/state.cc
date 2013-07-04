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

#include "com/centreon/engine/commands/connector.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/connector.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/hostdependency.hh"
#include "com/centreon/engine/configuration/applier/hostescalation.hh"
#include "com/centreon/engine/configuration/applier/hostgroup.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/servicedependency.hh"
#include "com/centreon/engine/configuration/applier/serviceescalation.hh"
#include "com/centreon/engine/configuration/applier/servicegroup.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/command.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

static applier::state* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] new_cfg The new configuration.
 */
void applier::state::apply(configuration::state& new_cfg) {
  // Apply timeperiods.
  _expand<configuration::timeperiod, applier::timeperiod>(
    new_cfg,
    new_cfg.timeperiods());
  _apply<configuration::timeperiod, applier::timeperiod>(
    config->timeperiods(),
    new_cfg.timeperiods());
  _resolve<configuration::timeperiod, applier::timeperiod>(
    config->timeperiods());

  // Apply connectors.
  _expand<configuration::connector, applier::connector>(
    new_cfg,
    new_cfg.connectors());
  _apply<configuration::connector, applier::connector>(
    config->connectors(),
    new_cfg.connectors());
  _resolve<configuration::connector, applier::connector>(
    config->connectors());

  // Apply commands.
  _expand<configuration::command, applier::command>(
    new_cfg,
    new_cfg.commands());
  _apply<configuration::command, applier::command>(
    config->commands(),
    new_cfg.commands());
  _resolve<configuration::command, applier::command>(
    config->commands());

  // Apply contacts and contactgroups.
  _expand<configuration::contact, applier::contact>(
    new_cfg,
    new_cfg.contacts());
  _apply<configuration::contact, applier::contact>(
    config->contacts(),
    new_cfg.contacts());
  _expand<configuration::contactgroup, applier::contactgroup>(
    new_cfg,
    new_cfg.contactgroups());
  _apply<configuration::contactgroup, applier::contactgroup>(
    config->contactgroups(),
    new_cfg.contactgroups());
  _resolve<configuration::contactgroup, applier::contactgroup>(
    config->contactgroups());
  _resolve<configuration::contact, applier::contact>(
    config->contacts());

  // Apply hosts and hostgroups.
  _expand<configuration::host, applier::host>(
    new_cfg,
    new_cfg.hosts());
  _apply<configuration::host, applier::host>(
    config->hosts(),
    new_cfg.hosts());
  _expand<configuration::hostgroup, applier::hostgroup>(
    new_cfg,
    new_cfg.hostgroups());
  _apply<configuration::hostgroup, applier::hostgroup>(
    config->hostgroups(),
    new_cfg.hostgroups());
  _resolve<configuration::hostgroup, applier::hostgroup>(
    config->hostgroups());
  _resolve<configuration::host, applier::host>(
    config->hosts());

  // Apply services and servicegroups.
  _expand<configuration::service, applier::service>(
    new_cfg,
    new_cfg.services());
  _apply<configuration::service, applier::service>(
    config->services(),
    new_cfg.services());
  _expand<configuration::servicegroup, applier::servicegroup>(
    new_cfg,
    new_cfg.servicegroups());
  _apply<configuration::servicegroup, applier::servicegroup>(
    config->servicegroups(),
    new_cfg.servicegroups());
  _resolve<configuration::servicegroup, applier::servicegroup>(
    config->servicegroups());
  _resolve<configuration::service, applier::service>(
    config->services());

  // // Apply host dependencies.
  // _expand<configuration::hostdependency, applier::hostdependency>(
  //   new_cfg,
  //   new_cfg.hostdependencies());
  // _apply<configuration::hostdependency, applier::hostdependency>(
  //   config->hostdependencies(),
  //   new_cfg.hostdependencies());
  // _resolve<configuration::hostdependency, applier::hostdependency>(
  //   config->hostdependencies());

  // // Apply service dependencies.
  // _expand<configuration::servicedependency, applier::servicedependency>(
  //   new_cfg,
  //   new_cfg.servicedependencies());
  // _apply<configuration::servicedependency, applier::servicedependency>(
  //   config->servicedependencies(),
  //   new_cfg.servicedependencies());
  // _resolve<configuration::servicedependency, applier::servicedependency>(
  //   config->servicedependencies());

  // // Apply host escalations.
  // _expand<configuration::hostescalation, applier::hostescalation>(
  //   new_cfg,
  //   new_cfg.hostescalations());
  // _apply<configuration::hostescalation, applier::hostescalation>(
  //   config->hostescalations(),
  //   new_cfg.hostescalations());
  // _resolve<configuration::hostescalation, applier::hostescalation>(
  //   config->hostescalations());

  // // Apply service escalations.
  // _expand<configuration::serviceescalation, applier::serviceescalation>(
  //   new_cfg,
  //   new_cfg.serviceescalations());
  // _apply<configuration::serviceescalation, applier::serviceescalation>(
  //   config->serviceescalations(),
  //   new_cfg.serviceescalations());
  // _resolve<configuration::serviceescalation, applier::serviceescalation>(
  //   config->serviceescalations());

  // Pre-flight check.
  {
    bool old_verify_config(verify_config);
    verify_config = true;
    pre_flight_check();
    verify_config = old_verify_config;
  }

  return ;
}

/**
 *  Get the singleton instance of state applier.
 *
 *  @return Singleton instance.
 */
applier::state& applier::state::instance() {
  return (*_instance);
}

/**
 *  Load state applier singleton.
 */
void applier::state::load() {
  if (!_instance) {
    _instance = new applier::state;
    config = new configuration::state;
  }
  return ;
}

/**
 *  Unload state applier singleton.
 */
void applier::state::unload() {
  delete _instance;
  _instance = NULL;
  delete config;
  config = NULL;
  return ;
}

/**
 *  Default constructor.
 */
applier::state::state() : _config(NULL) {}

/**
 *  Destructor.
 */
applier::state::~state() throw() {

}

/**
 *  Get the current commands.
 *
 *  @return The current commands.
 */
umap<std::string, shared_ptr<command_struct> > const& applier::state::commands() const throw () {
  return (_commands);
}

/**
 *  Get the current commands.
 *
 *  @return The current commands.
 */
umap<std::string, shared_ptr<command_struct> >& applier::state::commands() throw () {
  return (_commands);
}

/**
 *  Get the current connectors.
 *
 *  @return The current connectors.
 */
umap<std::string, shared_ptr<commands::connector> > const& applier::state::connectors() const throw () {
  return (_connectors);
}

/**
 *  Get the current connectors.
 *
 *  @return The current connectors.
 */
umap<std::string, shared_ptr<commands::connector> >& applier::state::connectors() throw () {
  return (_connectors);
}

/**
 *  Get the current contacts.
 *
 *  @return The current contacts.
 */
umap<std::string, shared_ptr<contact_struct> > const& applier::state::contacts() const throw () {
  return (_contacts);
}

/**
 *  Get the current contacts.
 *
 *  @return The current contacts.
 */
umap<std::string, shared_ptr<contact_struct> >& applier::state::contacts() throw () {
  return (_contacts);
}

/**
 *  Get the current contactgroups.
 *
 *  @return The current contactgroups.
 */
umap<std::string, shared_ptr<contactgroup_struct> > const& applier::state::contactgroups() const throw () {
  return (_contactgroups);
}

/**
 *  Get the current contactgroups.
 *
 *  @return The current contactgroups.
 */
umap<std::string, shared_ptr<contactgroup_struct> >& applier::state::contactgroups() throw () {
  return (_contactgroups);
}

/**
 *  Get the current hosts.
 *
 *  @return The current hosts.
 */
umap<std::string, shared_ptr<host_struct> > const& applier::state::hosts() const throw () {
  return (_hosts);
}

/**
 *  Get the current hosts.
 *
 *  @return The current hosts.
 */
umap<std::string, shared_ptr<host_struct> >& applier::state::hosts() throw () {
  return (_hosts);
}

/**
 *  Get the current hostdependencies.
 *
 *  @return The current hostdependencies.
 */
umultimap<std::string, shared_ptr<hostdependency_struct> > const& applier::state::hostdependencies() const throw () {
  return (_hostdependencies);
}

/**
 *  Get the current hostdependencies.
 *
 *  @return The current hostdependencies.
 */
umultimap<std::string, shared_ptr<hostdependency_struct> >& applier::state::hostdependencies() throw () {
  return (_hostdependencies);
}

/**
 *  Get the current hostescalations.
 *
 *  @return The current hostescalations.
 */
umultimap<std::string, shared_ptr<hostescalation_struct> > const& applier::state::hostescalations() const throw () {
  return (_hostescalations);
}

/**
 *  Get the current hostescalations.
 *
 *  @return The current hostescalations.
 */
umultimap<std::string, shared_ptr<hostescalation_struct> >& applier::state::hostescalations() throw () {
  return (_hostescalations);
}

/**
 *  Get the current hostgroups.
 *
 *  @return The current hostgroups.
 */
umap<std::string, shared_ptr<hostgroup_struct> > const& applier::state::hostgroups() const throw () {
  return (_hostgroups);
}

/**
 *  Get the current hostgroups.
 *
 *  @return The current hostgroups.
 */
umap<std::string, shared_ptr<hostgroup_struct> >& applier::state::hostgroups() throw () {
  return (_hostgroups);
}

/**
 *  Get the current services.
 *
 *  @return The current services.
 */
umap<std::pair<std::string, std::string>, shared_ptr<service_struct> > const& applier::state::services() const throw () {
  return (_services);
}

/**
 *  Get the current services.
 *
 *  @return The current services.
 */
umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >& applier::state::services() throw () {
  return (_services);
}

/**
 *  Get the current servicedependencies.
 *
 *  @return The current servicedependencies.
 */
umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> > const& applier::state::servicedependencies() const throw () {
  return (_servicedependencies);
}

/**
 *  Get the current servicedependencies.
 *
 *  @return The current servicedependencies.
 */
umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >& applier::state::servicedependencies() throw () {
  return (_servicedependencies);
}

/**
 *  Get the current serviceescalations.
 *
 *  @return The current serviceescalations.
 */
umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> > const& applier::state::serviceescalations() const throw () {
  return (_serviceescalations);
}

/**
 *  Get the current serviceescalations.
 *
 *  @return The current serviceescalations.
 */
umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> >& applier::state::serviceescalations() throw () {
  return (_serviceescalations);
}

/**
 *  Get the current servicegroups.
 *
 *  @return The current servicegroups.
 */
umap<std::string, shared_ptr<servicegroup_struct> > const& applier::state::servicegroups() const throw () {
  return (_servicegroups);
}

/**
 *  Get the current servicegroups.
 *
 *  @return The current servicegroups.
 */
umap<std::string, shared_ptr<servicegroup_struct> >& applier::state::servicegroups() throw () {
  return (_servicegroups);
}

/**
 *  Get the current timeperiods.
 *
 *  @return The current timeperiods.
 */
umap<std::string, shared_ptr<timeperiod_struct> > const& applier::state::timeperiods() const throw () {
  return (_timeperiods);
}

/**
 *  Get the current timeperiods.
 *
 *  @return The current timeperiods.
 */
umap<std::string, shared_ptr<timeperiod_struct> >& applier::state::timeperiods() throw () {
  return (_timeperiods);
}

/**
 *  @brief Apply configuration of a specific object type.
 *
 *  This method will perform a diff on cur_cfg and new_cfg to create the
 *  three element sets : added, modified and removed. The type applier
 *  will then be called to 1) remove old objects 2) create new objects
 *  3) modify existing objects, in this order.
 *
 *  @param[in] cur_cfg Current configuration set.
 *  @param[in] new_cfg New configuration set.
 */
template <typename ConfigurationType, typename ApplierType>
void applier::state::_apply(
                       std::set<shared_ptr<ConfigurationType> > const& cur_cfg,
                       std::set<shared_ptr<ConfigurationType> > const& new_cfg) {
  // Type alias.
  typedef std::set<shared_ptr<ConfigurationType> > cfg_set;

  /*
  ** Configuration diff.
  */

  difference<std::set<shared_ptr<ConfigurationType> > > diff;
  diff.parse(cur_cfg, new_cfg);

  /*
  ** Configuration application.
  */

  // Applier.
  ApplierType aplyr;

  // Erase objects.
  for (typename cfg_set::const_iterator
         it_delete(diff.deleted().begin()),
         end_delete(diff.deleted().end());
       it_delete != end_delete;
       ++it_delete)
    aplyr.remove_object(*it_delete);

  // Add objects.
  for (typename cfg_set::const_iterator
         it_create(diff.added().begin()),
         end_create(diff.added().end());
       it_create != end_create;
       ++it_create)
    aplyr.add_object(*it_create);

  // Modify objects.
  for (typename cfg_set::const_iterator
         it_modify(diff.modified().begin()),
         end_modify(diff.modified().end());
       it_modify != end_modify;
       ++it_modify)
    aplyr.modify_object(*it_modify);

  return ;
}

/**
 *  Expand objects.
 *
 *  @param[in,out] new_state New configuration state.
 *  @param[in,out] cfg       Configuration objects.
 */
template <typename ConfigurationType, typename ApplierType>
void applier::state::_expand(
                       configuration::state& new_state,
                       std::set<shared_ptr<ConfigurationType> >& cfg) {
  ApplierType aplyr;
  for (typename std::set<shared_ptr<ConfigurationType> >::iterator
         it(cfg.begin()),
         end(cfg.end());
       it != end;) {
    typename std::set<shared_ptr<ConfigurationType> >::iterator
      to_expand(it++);
    aplyr.expand_object(*to_expand, new_state);
  }
  return ;
}

/**
 *  Resolve objects.
 *
 *  @param[in] cfg Configuration objects.
 */
template <typename ConfigurationType, typename ApplierType>
void applier::state::_resolve(
       std::set<shared_ptr<ConfigurationType> >& cfg) {
  ApplierType aplyr;
  for (typename std::set<shared_ptr<ConfigurationType> >::const_iterator
         it(cfg.begin()),
         end(cfg.end());
       it != end;
       ++it)
    aplyr.resolve_object(*it);
  return ;
}

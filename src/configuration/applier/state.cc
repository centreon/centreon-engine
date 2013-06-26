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
#include "com/centreon/engine/configuration/applier/globals.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/hostgroup.hh"
#include "com/centreon/engine/configuration/applier/macros.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
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
 *  Get the key of a command.
 *
 *  @return Command name.
 */
std::string command_key(configuration::command const& c) {
  return (c.command_name());
}

/**
 *  Get the key of a connector.
 *
 *  @return Connector name.
 */
std::string connector_key(configuration::connector const& c) {
  return (c.connector_name());
}

/**
 *  Get the key of a contact.
 *
 *  @return Contact name.
 */
std::string contact_key(configuration::contact const& c) {
  return (c.contact_name());
}

/**
 *  Get the key of a contactgroup.
 *
 *  @return Contactgroup name.
 */
std::string contactgroup_key(configuration::contactgroup const& cg) {
  return (cg.contactgroup_name());
}

/**
 *  Get the key of a host.
 *
 *  @return Host name.
 */
std::string host_key(configuration::host const& h) {
  return (h.host_name());
}

/**
 *  Get the key of a hostgroup.
 *
 *  @return Hostgroup name.
 */
std::string hostgroup_key(configuration::hostgroup const& hg) {
  return (hg.hostgroup_name());
}

/**
 *  Get the key of a service.
 *
 *  @return Pair of host name / service description.
 */
std::pair<std::string, std::string> service_key(
                                      configuration::service const& s) {
  if ((s.hosts().size() > 1)
      || !s.hostgroups().empty())
    throw (engine_error() << "Error: Cannot apply unexpanded service '"
           << s.service_description() << "'.");
  return (std::make_pair(
                 s.hosts().front(),
                 s.service_description()));
}

/**
 *  Get the key of a servicegroup.
 *
 *  @return Servicegroup name.
 */
std::string servicegroup_key(configuration::servicegroup const& sg) {
  return (sg.servicegroup_name());
}

/**
 *  Get the key of a timeperiod.
 *
 *  @return Timeperiod name.
 */
std::string timeperiod_key(configuration::timeperiod const& t) {
  return (t.timeperiod_name());
}

/**
 *  Apply new configuration.
 *
 *  @param[in] new_cfg The new configuration.
 */
void applier::state::apply(configuration::state& new_cfg) {
  // Apply globals
  applier::globals::instance().apply(new_cfg);

  // Apply macros
  applier::macros::instance().apply(new_cfg);

  // Apply timeperiods.
  _apply<configuration::timeperiod,
         timeperiod_struct,
         applier::timeperiod,
         std::string,
         &timeperiod_key>(
    config->timeperiods(),
    _timeperiods,
    new_cfg,
    new_cfg.timeperiods());
  _resolve<configuration::timeperiod, applier::timeperiod>(
    config->timeperiods());

  // Apply connectors and commands.
  _apply<configuration::connector,
         commands::connector,
         applier::connector,
         std::string,
         &connector_key>(
    config->connectors(),
    _connectors,
    new_cfg,
    new_cfg.connectors());
  _apply<configuration::command,
         command_struct,
         applier::command,
         std::string,
         &command_key>(
    config->commands(),
    _commands,
    new_cfg,
    new_cfg.commands());
  _resolve<configuration::connector, applier::connector>(
    config->connectors());
  _resolve<configuration::command, applier::command>(
    config->commands());

  // Apply contacts and contactgroups.
  _expand<configuration::contact, applier::contact>(
    new_cfg,
    new_cfg.contacts());
  _apply<configuration::contact,
         contact_struct,
         applier::contact,
         std::string,
         &contact_key>(
    config->contacts(),
    _contacts,
    new_cfg,
    new_cfg.contacts());
  _expand<configuration::contactgroup, applier::contactgroup>(
    new_cfg,
    new_cfg.contactgroups());
  _apply<configuration::contactgroup,
         contactgroup_struct,
         applier::contactgroup,
         std::string,
         &contactgroup_key>(
    config->contactgroups(),
    _contactgroups,
    new_cfg,
    new_cfg.contactgroups());
  _resolve<configuration::contactgroup, applier::contactgroup>(
    config->contactgroups());
  _resolve<configuration::contact, applier::contact>(
    config->contacts());

  // Apply hosts and hostgroups.
  _expand<configuration::host, applier::host>(
    new_cfg,
    new_cfg.hosts());
  _apply<configuration::host,
         host_struct,
         applier::host,
         std::string,
         &host_key>(
    config->hosts(),
    _hosts,
    new_cfg,
    new_cfg.hosts());
  _expand<configuration::hostgroup, applier::hostgroup>(
    new_cfg,
    new_cfg.hostgroups());
  _apply<configuration::hostgroup,
         hostgroup_struct,
         applier::hostgroup,
         std::string,
         &hostgroup_key>(
    config->hostgroups(),
    _hostgroups,
    new_cfg,
    new_cfg.hostgroups());
  _resolve<configuration::hostgroup, applier::hostgroup>(
    config->hostgroups());
  _resolve<configuration::host, applier::host>(
    config->hosts());

  // Apply servicegroups and services.
  _apply<configuration::servicegroup,
         servicegroup_struct,
         applier::servicegroup,
         std::string,
         &servicegroup_key>(
    config->servicegroups(),
    _servicegroups,
    new_cfg,
    new_cfg.servicegroups());
  {
    std::set<shared_ptr<configuration::service> > expanded_services;
    {
      applier::service aplyr;
      for (std::set<shared_ptr<configuration::service> >::const_iterator
             it(new_cfg.services().begin()),
             end(new_cfg.services().end());
           it != end;
           ++it)
        aplyr.expand_object(**it, new_cfg, expanded_services);
    }
    _apply<configuration::service,
           service_struct,
           applier::service,
           std::pair<std::string, std::string>,
           &service_key>(
      config->services(),
      _services,
      new_cfg,
      expanded_services);
  }
  _resolve<configuration::servicegroup, applier::servicegroup>(
    config->servicegroups());
  _resolve<configuration::service, applier::service>(
    config->services());

  // Pre-flight check.
  {
    bool old_verify_config(verify_config);
    verify_config = true;
    int w(0);
    int e(0);
    pre_flight_object_check(&w, &e);
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
 *  Apply configuration of a specific object type.
 *
 *  XXX
 */
template <typename ConfigurationType,
          typename ObjectType,
          typename ApplierType,
          typename KeyType,
          KeyType (* config_key)(ConfigurationType const&)>
void applier::state::_apply(
                       std::set<shared_ptr<ConfigurationType> >& cur_cfg,
                       umap<KeyType, shared_ptr<ObjectType> >& cur_obj,
                       configuration::state const& new_state,
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
  {
    typename cfg_set::iterator
      it_current(cur_cfg.begin()),
      end_current(cur_cfg.end());
    for (typename cfg_set::const_iterator
           it_delete(diff.deleted().begin()),
           end_delete(diff.deleted().end());
         it_delete != end_delete;
         ++it_delete) {
      typename umap<KeyType, shared_ptr<ObjectType> >::iterator
        it(cur_obj.find((*config_key)(**it_delete)));
      if (it != cur_obj.end()) {
        aplyr.remove_object(**it_delete, new_state);
        while ((it_current != end_current)
               && ((*config_key)(**it_current)
                   < (*config_key)(**it_delete)))
          ++it_current;
        if ((it_current != end_current)
            && ((*config_key)(**it_current)
                == (*config_key)(**it_delete))) {
          typename cfg_set::iterator will_be_erased(it_current);
          ++it_current;
          cur_cfg.erase(will_be_erased);
        }
      }
    }
  }

  // Add objects.
  for (typename cfg_set::const_iterator
         it_create(diff.added().begin()),
         end_create(diff.added().end());
       it_create != end_create;
       ++it_create) {
    aplyr.add_object(**it_create, new_state);
    cur_cfg.insert(*it_create);
  }

  // Modify objects.
  // XXX

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
       std::set<shared_ptr<ConfigurationType> > const& cfg) {
  ApplierType aplyr;
  for (typename std::set<shared_ptr<ConfigurationType> >::const_iterator
         it(cfg.begin()),
         end(cfg.end());
       it != end;
       ++it)
    aplyr.resolve_object(**it, *config);
  return ;
}

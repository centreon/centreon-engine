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
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/connector.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/hostgroup.hh"
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
void applier::state::apply(configuration::state const& new_cfg) {
  // // Apply timeperiods.
  // _apply<configuration::timeperiod,
  //        timeperiod_struct,
  //        applier::timeperiod,
  //        std::string,
  //        &configuration::timeperiod::timeperiod_name> (
  //   _timeperiods,
  //   config,
  //   config.timeperiods());

  // Apply connectors.
  _apply<configuration::connector,
         commands::connector,
         applier::connector,
         std::string,
         &configuration::connector::connector_name>(
    config->connectors(),
    _connectors,
    new_cfg,
    new_cfg.connectors());

  // // Apply commands.
  // _apply<configuration::command,
  //        command_struct,
  //        applier::command,
  //        std::string,
  //        &configuration::command::command_name>(
  //   _commands,
  //   config,
  //   config.commands());

  // // Apply contactgroups.
  // _apply<configuration::contactgroup,
  //        contactgroup_struct,
  //        applier::contactgroup,
  //        std::string,
  //        &configuration::contactgroup::contactgroup_name>(
  //   _contactgroups,
  //   config,
  //   config.contactgroups());

  // // Apply contacts.
  // _apply<configuration::contact,
  //        contact_struct,
  //        applier::contact,
  //        std::string,
  //        &configuration::contact::contact_name>(
  //   _contacts,
  //   config,
  //   config.contacts());

  // // Apply hostgroups.
  // _apply<configuration::hostgroup,
  //        hostgroup_struct,
  //        applier::hostgroup,
  //        std::string,
  //        &configuration::hostgroup::hostgroup_name>(
  //   _hostgroups,
  //   config,
  //   config.hostgroups());

  // // Apply hosts.
  // _apply<configuration::host,
  //        host_struct,
  //        applier::host,
  //        std::string,
  //        &configuration::host::host_name>(
  //   _hosts,
  //   config,
  //   config.hosts());

  // // Apply servicegroups.
  // _apply<configuration::servicegroup,
  //        servicegroup_struct,
  //        applier::servicegroup,
  //        std::string,
  //        &configuration::servicegroup::servicegroup_name>(
  //   _servicegroups,
  //   config,
  //   config.servicegroups());

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
  if (!_instance)
    _instance = new applier::state;
}

/**
 *  Unload state applier singleton.
 */
void applier::state::unload() {
  delete _instance;
  _instance = NULL;
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
          KeyType const& (ConfigurationType::* config_key)() const throw () >
void applier::state::_apply(
                       std::list<shared_ptr<ConfigurationType> >& cur_cfg,
                       umap<KeyType, shared_ptr<ObjectType> >& cur_obj,
                       configuration::state const& new_state,
                       std::list<shared_ptr<ConfigurationType> > const& new_cfg) {
  // Type alias.
  typedef std::list<shared_ptr<ConfigurationType> > cfg_set;

  /*
  ** Configuration diff.
  */

  difference<std::list<shared_ptr<ConfigurationType> > > diff;
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
        it(cur_obj.find(((**it_delete).*config_key)()));
      if (it != cur_obj.end()) {
        aplyr.remove_object(**it_delete, new_state);
        while ((it_current != end_current)
               && (((**it_current).*config_key)()
                   < ((**it_delete).*config_key)()))
          ++it_current;
        if ((it_current != end_current)
            && (((**it_current).*config_key)()
                == ((**it_delete).*config_key)()))
          it_current = cur_cfg.erase(it_current);
      }
    }
  }

  // Add objects.
  for (typename cfg_set::const_iterator
         it_create(diff.added().begin()),
         end_create(diff.added().end());
       it_create != end_create;
       ++it_create)
    aplyr.add_object(**it_create, new_state);
  // XXX : add in cur_cfg (sorted)

  // Modify objects.
  // XXX

  return ;
}

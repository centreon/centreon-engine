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
 *  Local structure identifying an host dependency.
 */
struct hostdependency_id {
  std::string dependent_host;
  std::string host;
  bool        is_notification;

  bool        operator==(hostdependency_id const& right) const {
    return ((dependent_host == right.dependent_host)
            && (host == right.host)
            && (is_notification == right.is_notification));
  }

  bool        operator<(hostdependency_id const& right) const {
    if (dependent_host != right.dependent_host)
      return (dependent_host < right.dependent_host);
    else if (host != right.host)
      return (host < right.host);
    return (is_notification < right.is_notification);
  }
};

/**
 *  Find key in collection with direct access to find().
 *
 *  @param[in] obj Collection in which to search for.
 *  @param[in] key Key to search for.
 *
 *  @return Iterator to the element matching key, obj.end() if it was
 *          not found.
 */
template <typename ObjectCollectionType, typename KeyType>
typename ObjectCollectionType::iterator find_key_with_collection_find(
                                         ObjectCollectionType& obj,
                                         KeyType const& key) {
  return (obj.find(key));
}

/**
 *  Find a host dependency in the umultimap.
 *
 *  @param[in] obj Collection in which to search for.
 *  @param[in] key Key to search for.
 *
 *  @return Iterator to the element matching key, obj.end() if it was
 *          not found.
 */
umultimap<std::string, shared_ptr<hostdependency_struct> >::iterator find_hostdependency_key(
  umultimap<std::string, shared_ptr<hostdependency_struct> >& obj,
  hostdependency_id const& key) {
  typedef umultimap<std::string, shared_ptr<hostdependency_struct> > CollectionType;
  std::pair<CollectionType::iterator, CollectionType::iterator>
    p(obj.equal_range(key.dependent_host));
  while (p.first != p.second) {
    if ((p.first->second->host_name == key.host)
        && ((p.first->second->dependency_type
             == NOTIFICATION_DEPENDENCY) == key.is_notification))
      break ;
    ++p.first;
  }
  return ((p.first != p.second) ? p.first : obj.end());
}

/**
 *  Find a host escalation in the umultimap.
 *
 *  @param[in] obj Collection in which to search for.
 *  @param[in] key Key to search for.
 *
 *  @return Iterator to the element matching key, obj.end() if it was
 *          not found.
 */
umultimap<std::string, shared_ptr<hostescalation_struct> >::iterator find_hostescalation_key(
  umultimap<std::string, shared_ptr<hostescalation_struct> >& obj,
  configuration::hostescalation const& key) {
  typedef umultimap<std::string, shared_ptr<hostescalation_struct> > CollectionType;
  std::pair<CollectionType::iterator, CollectionType::iterator>
    p(obj.equal_range(key.hosts().front()));
  while (p.first != p.second) {
    // XXX
    ++p.first;
  }
  return ((p.first != p.second) ? p.first : obj.end());
}

/**
 *  Find a service dependency in the umultimap.
 *
 *  @param[in] obj Collection in which to search for.
 *  @param[in] key Key to search for.
 *
 *  @return Iterator to the element matching key, obj.end() if it was
 *          not found.
 */
umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >::iterator find_servicedependency_key(
  umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >& obj,
  configuration::servicedependency const& key) {
  typedef umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> > CollectionType;
  std::pair<CollectionType::iterator, CollectionType::iterator>
    p(obj.equal_range(std::make_pair(
                        key.dependent_hosts().front(),
                        key.dependent_service_description().front())));
  while (p.first != p.second) {
    servicedependency_struct& sd(*p.first->second);
    unsigned int options(
                   (sd.fail_on_ok
                    ? configuration::servicedependency::ok
                    : 0)
                   | (sd.fail_on_warning
                      ? configuration::servicedependency::warning
                      : 0)
                   | (sd.fail_on_unknown
                      ? configuration::servicedependency::unknown
                      : 0)
                   | (sd.fail_on_critical
                      ? configuration::servicedependency::critical
                      : 0)
                   | (sd.fail_on_pending
                      ? configuration::servicedependency::pending
                      : 0));
    std::string dhn(sd.dependent_host_name
                    ? sd.dependent_host_name
                    : "");
    std::string dsd(sd.dependent_service_description
                    ? sd.dependent_service_description
                    : "");
    std::string hn(sd.host_name
                   ? sd.host_name
                   : "");
    std::string sde(sd.service_description
                    ? sd.service_description
                    : "");
    std::string dp(sd.dependency_period
                   ? sd.dependency_period
                   : NULL);
    if ((sd.dependency_type == key.dependency_type())
        && (dhn == key.dependent_hosts().front())
        && (dsd == key.dependent_service_description().front())
        && (hn == key.hosts().front())
        && (sde == key.service_description().front())
        && (dp == key.dependency_period())
        && (sd.inherits_parent == key.inherits_parent())
        && (options
            == ((sd.dependency_type == EXECUTION_DEPENDENCY)
                ? key.execution_failure_options()
                : key.notification_failure_options())))
      break ;
    ++p.first;
  }
  return ((p.first != p.second) ? p.first : obj.end());
}

/**
 *  Find a service escalation in the umultimap.
 *
 *  @param[in] obj Collection in which to search for.
 *  @param[in] key Key to search for.
 *
 *  @return Iterator to the element matchin key, obj.end() if it was
 *          not found.
 */
umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> >::iterator find_serviceescalation_key(
  umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> >& obj,
  configuration::serviceescalation const& key) {
  typedef umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> > CollectionType;
  std::pair<CollectionType::iterator, CollectionType::iterator>
    p(obj.equal_range(std::make_pair(
                             key.hosts().front(),
                             key.service_description().front())));
  while (p.first != p.second) {
    // XXX
    ++p.first;
  }
  return ((p.first != p.second) ? p.first : obj.end());
}

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
 *  Get the key of a hostdependency.
 *
 *  @return hostdependency_id with dependent_host, host and false for
 *          an execution dependency and true for a notification
 *          dependency.
 */
hostdependency_id hostdependency_key(
                    configuration::hostdependency const& hd) {
  hostdependency_id id;
  id.dependent_host = hd.dependent_hosts().front();
  id.host = hd.hosts().front();
  id.is_notification = hd.notification_failure_options();
  return (id);
}

/**
 *  Get the key of a host escalation.
 *
 *  @return A copy of the host escalation configuration object.
 */
configuration::hostescalation hostescalation_key(
                                configuration::hostescalation const& he) {
  return (he);
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
  return (std::make_pair(
                 s.hosts().front(),
                 s.service_description()));
}

/**
 *  Get the key of a service dependency.
 *
 *  @param[in] sd Service dependency.
 */
configuration::servicedependency servicedependency_key(
                 configuration::servicedependency const& sd) {
  return (sd);
}

/**
 *  Get the key of a service escalation.
 *
 *  @param[in] se Service escalation.
 *
 *  @return A copy of the service escalation configuration object.
 */
configuration::serviceescalation serviceescalation_key(
                                   configuration::serviceescalation const& se) {
  return (se);
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
  // Apply timeperiods.
  _apply<configuration::timeperiod,
         umap<std::string, shared_ptr<timeperiod_struct> >,
         applier::timeperiod,
         std::string,
         &timeperiod_key,
         &find_key_with_collection_find<
            umap<std::string, shared_ptr<timeperiod_struct> >,
            std::string> >(
    config->timeperiods(),
    _timeperiods,
    new_cfg,
    new_cfg.timeperiods());
  _resolve<configuration::timeperiod, applier::timeperiod>(
    config->timeperiods());

  // Apply connectors and commands.
  _apply<configuration::connector,
         umap<std::string, shared_ptr<commands::connector> >,
         applier::connector,
         std::string,
         &connector_key,
         &find_key_with_collection_find<
            umap<std::string, shared_ptr<commands::connector> >,
            std::string> >(
    config->connectors(),
    _connectors,
    new_cfg,
    new_cfg.connectors());
  _apply<configuration::command,
         umap<std::string, shared_ptr<command_struct> >,
         applier::command,
         std::string,
         &command_key,
         &find_key_with_collection_find<
            umap<std::string, shared_ptr<command_struct> >,
            std::string> >(
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
         umap<std::string, shared_ptr<contact_struct> >,
         applier::contact,
         std::string,
         &contact_key,
         &find_key_with_collection_find<
            umap<std::string, shared_ptr<contact_struct> >,
            std::string> >(
    config->contacts(),
    _contacts,
    new_cfg,
    new_cfg.contacts());
  _expand<configuration::contactgroup, applier::contactgroup>(
    new_cfg,
    new_cfg.contactgroups());
  _apply<configuration::contactgroup,
         umap<std::string, shared_ptr<contactgroup_struct> >,
         applier::contactgroup,
         std::string,
         &contactgroup_key,
         &find_key_with_collection_find<
            umap<std::string, shared_ptr<contactgroup_struct> >,
            std::string> >(
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
         umap<std::string, shared_ptr<host_struct> >,
         applier::host,
         std::string,
         &host_key,
         &find_key_with_collection_find<
            umap<std::string, shared_ptr<host_struct> >,
            std::string> >(
    config->hosts(),
    _hosts,
    new_cfg,
    new_cfg.hosts());
  _expand<configuration::hostgroup, applier::hostgroup>(
    new_cfg,
    new_cfg.hostgroups());
  _apply<configuration::hostgroup,
         umap<std::string, shared_ptr<hostgroup_struct> >,
         applier::hostgroup,
         std::string,
         &hostgroup_key,
         &find_key_with_collection_find<
            umap<std::string, shared_ptr<hostgroup_struct> >,
            std::string> >(
    config->hostgroups(),
    _hostgroups,
    new_cfg,
    new_cfg.hostgroups());
  _resolve<configuration::hostgroup, applier::hostgroup>(
    config->hostgroups());
  _resolve<configuration::host, applier::host>(
    config->hosts());

  // Apply services and servicegroups.
  _expand<configuration::service, applier::service>(
    new_cfg,
    new_cfg.services());
  _apply<configuration::service,
         umap<std::pair<std::string, std::string>,
              shared_ptr<service_struct> >,
         applier::service,
         std::pair<std::string, std::string>,
         &service_key,
         &find_key_with_collection_find<
            umap<std::pair<std::string, std::string>,
                 shared_ptr<service_struct> >,
            std::pair<std::string, std::string> > >(
    config->services(),
    _services,
    new_cfg,
    new_cfg.services());
  _expand<configuration::servicegroup, applier::servicegroup>(
    new_cfg,
    new_cfg.servicegroups());
  _apply<configuration::servicegroup,
         umap<std::string, shared_ptr<servicegroup_struct> >,
         applier::servicegroup,
         std::string,
         &servicegroup_key,
         &find_key_with_collection_find<
            umap<std::string, shared_ptr<servicegroup_struct> >,
            std::string> >(
    config->servicegroups(),
    _servicegroups,
    new_cfg,
    new_cfg.servicegroups());
  _resolve<configuration::servicegroup, applier::servicegroup>(
    config->servicegroups());
  _resolve<configuration::service, applier::service>(
    config->services());

  // Apply host dependencies.
  _expand<configuration::hostdependency, applier::hostdependency>(
    new_cfg,
    new_cfg.hostdependencies());
  _apply<configuration::hostdependency,
         umultimap<std::string, shared_ptr<hostdependency_struct> >,
         applier::hostdependency,
         hostdependency_id,
         &hostdependency_key,
         &find_hostdependency_key>(
    config->hostdependencies(),
    _hostdependencies,
    new_cfg,
    new_cfg.hostdependencies());
  _resolve<configuration::hostdependency, applier::hostdependency>(
    config->hostdependencies());

  // Apply service dependencies.
  _expand<configuration::servicedependency, applier::servicedependency>(
    new_cfg,
    new_cfg.servicedependencies());
  _apply<configuration::servicedependency,
         umultimap<std::pair<std::string, std::string>,
                   shared_ptr<servicedependency_struct> >,
         applier::servicedependency,
         configuration::servicedependency,
         &servicedependency_key,
         &find_servicedependency_key>(
    config->servicedependencies(),
    _servicedependencies,
    new_cfg,
    new_cfg.servicedependencies());
  _resolve<configuration::servicedependency, applier::servicedependency>(
    config->servicedependencies());

  // Apply host escalations.
  _expand<configuration::hostescalation, applier::hostescalation>(
    new_cfg,
    new_cfg.hostescalations());
  _apply<configuration::hostescalation,
         umultimap<std::string,
                   shared_ptr<hostescalation_struct> >,
         applier::hostescalation,
         configuration::hostescalation,
         &hostescalation_key,
         &find_hostescalation_key>(
    config->hostescalations(),
    _hostescalations,
    new_cfg,
    new_cfg.hostescalations());
  _resolve<configuration::hostescalation, applier::hostescalation>(
    config->hostescalations());

  // Apply service escalations.
  _expand<configuration::serviceescalation, applier::serviceescalation>(
    new_cfg,
    new_cfg.serviceescalations());
  _apply<configuration::serviceescalation,
         umultimap<std::pair<std::string, std::string>,
                   shared_ptr<serviceescalation_struct> >,
         applier::serviceescalation,
         configuration::serviceescalation,
         &serviceescalation_key,
         &find_serviceescalation_key>(
    config->serviceescalations(),
    _serviceescalations,
    new_cfg,
    new_cfg.serviceescalations());
  _resolve<configuration::serviceescalation, applier::serviceescalation>(
    config->serviceescalations());

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
 *  Apply configuration of a specific object type.
 *
 *  XXX
 */
template <typename ConfigurationType,
          typename ObjectCollectionType,
          typename ApplierType,
          typename KeyType,
          KeyType (* config_key)(ConfigurationType const&),
          typename ObjectCollectionType::iterator (* find_obj_from_key)(ObjectCollectionType&, KeyType const&)>
void applier::state::_apply(
                       std::set<shared_ptr<ConfigurationType> >& cur_cfg,
                       ObjectCollectionType& cur_obj,
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
      typename ObjectCollectionType::iterator
        it(find_obj_from_key(cur_obj, (*config_key)(**it_delete)));
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

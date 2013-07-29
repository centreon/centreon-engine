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

#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/commands/connector.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/connector.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/globals.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/hostdependency.hh"
#include "com/centreon/engine/configuration/applier/hostescalation.hh"
#include "com/centreon/engine/configuration/applier/hostgroup.hh"
#include "com/centreon/engine/configuration/applier/logging.hh"
#include "com/centreon/engine/configuration/applier/macros.hh"
#include "com/centreon/engine/configuration/applier/scheduler.hh"
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
#include "com/centreon/engine/retention/applier/state.hh"
#include "com/centreon/engine/retention/state.hh"
#include "com/centreon/engine/xpddefault.hh"
#include "com/centreon/engine/xsddefault.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::logging;

static bool            has_already_been_loaded(false);
static applier::state* _instance(NULL);

/**
 *  Apply new configuration.
 *
 *  @param[in] new_cfg The new configuration.
 */
void applier::state::apply(configuration::state& new_cfg) {
  configuration::state save(*config);
  try {
    _processing(new_cfg);
  }
  catch (std::exception const& e) {
    // If is the first time to load configuration, we don't
    // have a valid configuration to restore.
    if (!has_already_been_loaded)
      throw;

    // If is not the first time, we can restore the old one.
    logger(log_config_error, basic)
      << "configuration: apply new configuration failed: "
      << e.what();
    logger(dbg_config, more)
      << "configuration: try to restore old configuration";
    _processing(save);
  }
  return ;
}

/**
 *  Apply new configuration.
 *
 *  @param[in] new_cfg The new configuration.
 *  @param[in] state   The retention to use.
 */
void applier::state::apply(
       configuration::state& new_cfg,
       retention::state& state) {
  configuration::state save(*config);
  try {
    _processing(new_cfg, &state);
  }
  catch (std::exception const& e) {
    // If is the first time to load configuration, we don't
    // have a valid configuration to restore.
    if (!has_already_been_loaded)
      throw;

    // If is not the first time, we can restore the old one.
    logger(log_config_error, basic)
      << "configuration: apply new configuration failed: "
      << e.what();
    logger(dbg_config, more)
      << "configuration: try to restore old configuration";
    _processing(save, &state);
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
applier::state::state()
  : _config(NULL),
    _waiting(false) {
  applier::logging::load();
  applier::globals::load();
  applier::macros::load();
  applier::scheduler::load();
}

/**
 *  Destructor.
 */
applier::state::~state() throw() {
  applier::scheduler::unload();
  applier::macros::unload();
  applier::globals::unload();
  applier::logging::unload();
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
 *  Find a command from its key.
 *
 *  @param[in] k Command name.
 *
 *  @return Iterator to the element if found, commands().end()
 *          otherwise.
 */
umap<std::string, shared_ptr<command_struct> >::const_iterator applier::state::commands_find(configuration::command::key_type const& k) const {
  return (_commands.find(k));
}

/**
 *  Find a command from its key.
 *
 *  @param[in] k Command name.
 *
 *  @return Iterator to the element if found, commands().end()
 *          otherwise.
 */
umap<std::string, shared_ptr<command_struct> >::iterator applier::state::commands_find(configuration::command::key_type const& k) {
  return (_commands.find(k));
}

/**
 *  Find a connector from its key.
 *
 *  @param[in] k Connector name.
 *
 *  @return Iterator to the element if found, connectors().end()
 *          otherwise.
 */
umap<std::string, shared_ptr<commands::connector> >::const_iterator applier::state::connectors_find(configuration::connector::key_type const& k) const {
  return (_connectors.find(k));
}

/**
 *  Find a connector from its key.
 *
 *  @param[in] k Connector name.
 *
 *  @return Iterator to the element if found, connectors().end()
 *          otherwise.
 */
umap<std::string, shared_ptr<commands::connector> >::iterator applier::state::connectors_find(configuration::connector::key_type const& k) {
  return (_connectors.find(k));
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
 *  Find a contact from its key.
 *
 *  @param[in] k Contact name.
 *
 *  @return Iterator to the element if found, contacts().end()
 *          otherwise.
 */
umap<std::string, shared_ptr<contact_struct> >::const_iterator applier::state::contacts_find(configuration::contact::key_type const& k) const {
  return (_contacts.find(k));
}

/**
 *  Find a contact from its key.
 *
 *  @param[in] k Contact name.
 *
 *  @return Iterator to the element if found, contacts().end()
 *          otherwise.
 */
umap<std::string, shared_ptr<contact_struct> >::iterator applier::state::contacts_find(configuration::contact::key_type const& k) {
  return (_contacts.find(k));
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
 *  Find a contact group from its key.
 *
 *  @param[in] k Contact group key.
 *
 *  @return Iterator to the element if found, contactgroups().end()
 *          otherwise.
 */
umap<std::string, shared_ptr<contactgroup_struct> >::const_iterator applier::state::contactgroups_find(configuration::contactgroup::key_type const& k) const {
  return (_contactgroups.find(k));
}

/**
 *  Find a contact group from its key.
 *
 *  @param[in] k Contact group key.
 *
 *  @return Iterator to the element if found, contactgroups().end()
 *          otherwise.
 */
umap<std::string, shared_ptr<contactgroup_struct> >::iterator applier::state::contactgroups_find(configuration::contactgroup::key_type const& k) {
  return (_contactgroups.find(k));
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
 *  Find a host from its key.
 *
 *  @param[in] k Host key (host name).
 *
 *  @return Iterator to the host object if found, hosts().end() if it
 *          was not.
 */
umap<std::string, shared_ptr<host_struct> >::const_iterator applier::state::hosts_find(configuration::host::key_type const& k) const {
  return (_hosts.find(k));
}

/**
 *  Find a host from its key.
 *
 *  @param[in] k Host key (host name).
 *
 *  @return Iterator to the host object if found, hosts().end() if it
 *          was not.
 */
umap<std::string, shared_ptr<host_struct> >::iterator applier::state::hosts_find(configuration::host::key_type const& k) {
  return (_hosts.find(k));
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
 *  Get a host dependency from its key.
 *
 *  @param[in] k Host dependency key.
 *
 *  @return Iterator to the element if found, hostdependencies().end()
 *          otherwise.
 */
umultimap<std::string, shared_ptr<hostdependency_struct> >::const_iterator applier::state::hostdependencies_find(configuration::hostdependency::key_type const& k) const {
  return (const_cast<state*>(this)->hostdependencies_find(k));
}

/**
 *  Get a host dependency from its key.
 *
 *  @param[in] k Host dependency key.
 *
 *  @return Iterator to the element if found, hostdependencies().end()
 *          otherwise.
 */
umultimap<std::string, shared_ptr<hostdependency_struct> >::iterator applier::state::hostdependencies_find(configuration::hostdependency::key_type const& k) {
  typedef umultimap<std::string, shared_ptr<hostdependency_struct> > collection;
  std::pair<collection::iterator, collection::iterator> p;
  p = _hostdependencies.equal_range(k.dependent_hosts().front());
  while (p.first != p.second) {
    configuration::hostdependency current;
    current.configuration::object::operator=(k);
    current.dependent_hosts().push_back(
                                p.first->second->dependent_host_name);
    current.hosts().push_back(p.first->second->host_name);
    current.dependency_period(p.first->second->dependency_period
                              ? p.first->second->dependency_period
                              : "");
    current.inherits_parent(p.first->second->inherits_parent);
    unsigned int options(
      (p.first->second->fail_on_up
       ? configuration::hostdependency::up
       : 0)
      | (p.first->second->fail_on_down
         ? configuration::hostdependency::down
         : 0)
      | (p.first->second->fail_on_unreachable
         ? configuration::hostdependency::unreachable
         : 0)
      | (p.first->second->fail_on_pending
         ? configuration::hostdependency::pending
         : 0));
    if (p.first->second->dependency_type == NOTIFICATION_DEPENDENCY) {
      current.dependency_type(
                configuration::hostdependency::notification_dependency);
      current.notification_failure_options(options);
    }
    else {
      current.dependency_type(
                configuration::hostdependency::execution_dependency);
      current.execution_failure_options(options);
    }
    if (current == k)
      break ;
    ++p.first;
  }
  return ((p.first == p.second) ? _hostdependencies.end() : p.first);
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
 *  Find a host escalation by its key.
 *
 *  @param[in] k Host escalation configuration.
 *
 *  @return Iterator to the element if found, hostescalations().end()
 *          otherwise.
 */
umultimap<std::string, shared_ptr<hostescalation_struct> >::const_iterator applier::state::hostescalations_find(configuration::hostescalation::key_type const& k) const {
  return (const_cast<state*>(this)->hostescalations_find(k));
}

/**
 *  Find a host escalation by its key.
 *
 *  @param[in] k Host escalation configuration.
 *
 *  @return Iterator to the element if found, hostescalations().end()
 *          otherwise.
 */
umultimap<std::string, shared_ptr<hostescalation_struct> >::iterator applier::state::hostescalations_find(configuration::hostescalation::key_type const& k) {
  typedef umultimap<std::string, shared_ptr<hostescalation_struct> > collection;
  std::pair<collection::iterator, collection::iterator> p;
  p = _hostescalations.equal_range(k.hosts().front());
  while (p.first != p.second) {
    configuration::hostescalation current;
    current.configuration::object::operator=(k);
    current.hosts().push_back(p.first->second->host_name);
    current.first_notification(p.first->second->first_notification);
    current.last_notification(p.first->second->last_notification);
    current.notification_interval(
              static_cast<unsigned int>(p.first->second->notification_interval));
    current.escalation_period(p.first->second->escalation_period
                              ? p.first->second->escalation_period
                              : "");
    unsigned int options(
                   (p.first->second->escalate_on_recovery
                    ? configuration::hostescalation::recovery
                    : 0)
                   | (p.first->second->escalate_on_down
                      ? configuration::hostescalation::down
                      : 0)
                   | (p.first->second->escalate_on_unreachable
                      ? configuration::hostescalation::unreachable
                      : 0));
    current.escalation_options(options);
    for (contactsmember_struct* m(p.first->second->contacts);
         m;
         m = m->next)
      current.contacts().push_front(m->contact_ptr
                                    ? m->contact_ptr->name
                                    : m->contact_name);
    for (contactgroupsmember_struct* m(p.first->second->contact_groups);
         m;
         m = m->next)
      current.contactgroups().push_front(m->group_ptr
                                         ? m->group_ptr->group_name
                                         : m->group_name);
    if (current == k)
      break ;
    ++p.first;
  }
  return ((p.first == p.second) ? _hostescalations.end() : p.first);
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
 *  Find a host group from its key.
 *
 *  @param[in] k Host group name.
 *
 *  @return Iterator to the element if found, hostgroups().end()
 *          otherwise.
 */
umap<std::string, shared_ptr<hostgroup_struct> >::const_iterator applier::state::hostgroups_find(configuration::hostgroup::key_type const& k) const {
  return (_hostgroups.find(k));
}

/**
 *  Find a host group from its key.
 *
 *  @param[in] k Host group name.
 *
 *  @return Iterator to the element if found, hostgroups().end()
 *          otherwise.
 */
umap<std::string, shared_ptr<hostgroup_struct> >::iterator applier::state::hostgroups_find(configuration::hostgroup::key_type const& k) {
  return (_hostgroups.find(k));
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
 *  Find a service by its key.
 *
 *  @param[in] k Pair of host name / service description.
 *
 *  @return Iterator to the element if found, services().end()
 *          otherwise.
 */
umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator applier::state::services_find(configuration::service::key_type const& k) const {
  return (_services.find(k));
}

/**
 *  Find a service by its key.
 *
 *  @param[in] k Pair of host name / service description.
 *
 *  @return Iterator to the element if found, services().end()
 *          otherwise.
 */
umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::iterator applier::state::services_find(configuration::service::key_type const& k) {
  return (_services.find(k));
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
 *  Find a service dependency from its key.
 *
 *  @param[in] k The service dependency configuration.
 *
 *  @return Iterator to the element if found,
 *          servicedependencies().end() otherwise.
 */
umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >::const_iterator applier::state::servicedependencies_find(configuration::servicedependency::key_type const& k) const {
  return (const_cast<state*>(this)->servicedependencies_find(k));
}

/**
 *  Find a service dependency from its key.
 *
 *  @param[in] k The service dependency configuration.
 *
 *  @return Iterator to the element if found,
 *          servicedependencies().end() otherwise.
 */
umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >::iterator applier::state::servicedependencies_find(configuration::servicedependency::key_type const& k) {
  typedef umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> > collection;
  std::pair<collection::iterator, collection::iterator> p;
  p = _servicedependencies.equal_range(std::make_pair(k.dependent_hosts().front(), k.dependent_service_description().front()));
  while (p.first != p.second) {
    configuration::servicedependency current;
    current.configuration::object::operator=(k);
    current.dependent_hosts().push_back(
                                p.first->second->dependent_host_name);
    current.dependent_service_description().push_back(
              p.first->second->dependent_service_description);
    current.hosts().push_back(p.first->second->host_name);
    current.service_description().push_back(
              p.first->second->service_description);
    current.dependency_period(p.first->second->dependency_period
                              ? p.first->second->dependency_period
                              : "");
    current.inherits_parent(p.first->second->inherits_parent);
    unsigned int options(
                   (p.first->second->fail_on_ok
                    ? configuration::servicedependency::ok
                    : 0)
                   | (p.first->second->fail_on_warning
                      ? configuration::servicedependency::warning
                      : 0)
                   | (p.first->second->fail_on_unknown
                      ? configuration::servicedependency::unknown
                      : 0)
                   | (p.first->second->fail_on_critical
                      ? configuration::servicedependency::critical
                      : 0)
                   | (p.first->second->fail_on_pending
                      ? configuration::servicedependency::pending
                      : 0));
    if (p.first->second->dependency_type == NOTIFICATION_DEPENDENCY) {
      current.dependency_type(
        configuration::servicedependency::notification_dependency);
      current.notification_failure_options(options);
    }
    else {
      current.dependency_type(
        configuration::servicedependency::execution_dependency);
      current.execution_failure_options(options);
    }
    if (current == k)
      break ;
    ++p.first;
  }
  return ((p.first == p.second) ? _servicedependencies.end() : p.first);
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
 *  Find a service escalation by its key.
 *
 *  @param[in] k Service escalation configuration object.
 *
 *  @return Iterator to the element if found, serviceescalations().end()
 *          otherwise.
 */
umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> >::const_iterator applier::state::serviceescalations_find(configuration::serviceescalation::key_type const& k) const {
  return (const_cast<state*>(this)->serviceescalations_find(k));
}

/**
 *  Find a service escalation by its key.
 *
 *  @param[in] k Service escalation configuration object.
 *
 *  @return Iterator to the element if found, serviceescalations().end()
 *          otherwise.
 */
umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> >::iterator applier::state::serviceescalations_find(configuration::serviceescalation::key_type const& k) {
  typedef umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> > collection;
  std::pair<collection::iterator, collection::iterator> p;
  p = _serviceescalations.equal_range(std::make_pair(k.hosts().front(), k.service_description().front()));
  while (p.first != p.second) {
    configuration::serviceescalation current;
    current.configuration::object::operator=(k);
    current.hosts().push_back(p.first->second->host_name);
    current.service_description().push_back(
                                    p.first->second->description);
    current.first_notification(p.first->second->first_notification);
    current.last_notification(p.first->second->last_notification);
    current.notification_interval(
              static_cast<unsigned int>(p.first->second->notification_interval));
    current.escalation_period(p.first->second->escalation_period
                              ? p.first->second->escalation_period
                              : "");
    unsigned int options((p.first->second->escalate_on_recovery
                          ? configuration::serviceescalation::recovery
                          : 0)
                         | (p.first->second->escalate_on_warning
                            ? configuration::serviceescalation::warning
                            : 0)
                         | (p.first->second->escalate_on_unknown
                            ? configuration::serviceescalation::unknown
                            : 0)
                         | (p.first->second->escalate_on_critical
                            ? configuration::serviceescalation::critical
                            : 0));
    current.escalation_options(options);
    for (contactsmember_struct* m(p.first->second->contacts);
         m;
         m = m->next)
      current.contacts().push_front(m->contact_ptr
                                    ? m->contact_ptr->name
                                    : m->contact_name);
    for (contactgroupsmember_struct* m(p.first->second->contact_groups);
         m;
         m = m->next)
      current.contactgroups().push_front(m->group_ptr
                                         ? m->group_ptr->group_name
                                         : m->group_name);
    if (current == k)
      break ;
    ++p.first;
  }
  return ((p.first == p.second) ? _serviceescalations.end() : p.first);
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
 *  Find a service group from its key.
 *
 *  @param[in] k Service group name.
 *
 *  @return Iterator to the element if found, servicegroups().end()
 *          otherwise.
 */
umap<std::string, shared_ptr<servicegroup_struct> >::const_iterator applier::state::servicegroups_find(configuration::servicegroup::key_type const& k) const {
  return (_servicegroups.find(k));
}

/**
 *  Find a service group from its key.
 *
 *  @param[in] k Service group name.
 *
 *  @return Iterator to the element if found, servicegroups().end()
 *          otherwise.
 */
umap<std::string, shared_ptr<servicegroup_struct> >::iterator applier::state::servicegroups_find(configuration::servicegroup::key_type const& k) {
  return (_servicegroups.find(k));
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
 *  Find a time period from its key.
 *
 *  @param[in] k Time period name.
 *
 *  @return Iterator to the element if found, timeperiods().end()
 *          otherwise.
 */
umap<std::string, shared_ptr<timeperiod_struct> >::const_iterator applier::state::timeperiods_find(configuration::timeperiod::key_type const& k) const {
  return (_timeperiods.find(k));
}

/**
 *  Find a time period from its key.
 *
 *  @param[in] k Time period name.
 *
 *  @return Iterator to the element if found, timeperiods().end()
 *          otherwise.
 */
umap<std::string, shared_ptr<timeperiod_struct> >::iterator applier::state::timeperiods_find(configuration::timeperiod::key_type const& k) {
  return (_timeperiods.find(k));
}

/**
 *  Try to lock.
 */
void applier::state::try_lock() {
  if (has_already_been_loaded) {
    concurrency::locker lock(&_lock);
    if (_waiting) {
      _cv_lock.wake_one();
      _cv_lock.wait(&_lock);
    }
  }
}

/*
 *  Update all new globals.
 *
 *  @param[in] new_cfg The new configuration state.
 */
void applier::state::_apply(configuration::state const& new_cfg) {
  // Check variables should not be change after the first execution.
  if (has_already_been_loaded) {
    if (config->broker_module() != new_cfg.broker_module())
      logger(log_config_warning, basic)
        << "configuration: warning: broker module can not be change";

    if (config->broker_module_directory() != new_cfg.broker_module_directory())
      logger(log_config_warning, basic)
        << "configuration: warning: broker module directory "
        "can not be change";

    if (config->command_file() != new_cfg.command_file())
      logger(log_config_warning, basic)
        << "configuration: warning: command file can not be change";

    if (config->external_command_buffer_slots() != new_cfg.external_command_buffer_slots())
      logger(log_config_warning, basic)
        << "configuration: warning: external command buffer slots "
        "can not be change";

    if (config->use_timezone() != new_cfg.use_timezone())
      logger(log_config_warning, basic)
        << "configuration: warning: timezone can not be change";
  }

  // Initialize perfdata if necessary.
  if (!has_already_been_loaded
      || config->host_perfdata_command() != new_cfg.host_perfdata_command()
      || config->host_perfdata_file() != new_cfg.host_perfdata_file()
      || config->host_perfdata_file_mode() != new_cfg.host_perfdata_file_mode()
      || config->host_perfdata_file_processing_command() != new_cfg.host_perfdata_file_processing_command()
      || config->host_perfdata_file_processing_interval() != new_cfg.host_perfdata_file_processing_interval()
      || config->host_perfdata_file_template() != new_cfg.host_perfdata_file_template()
      || config->service_perfdata_command() != new_cfg.service_perfdata_command()
      || config->service_perfdata_file() != new_cfg.service_perfdata_file()
      || config->service_perfdata_file_mode() != new_cfg.service_perfdata_file_mode()
      || config->service_perfdata_file_processing_command() != new_cfg.service_perfdata_file_processing_command()
      || config->service_perfdata_file_processing_interval() != new_cfg.service_perfdata_file_processing_interval()
      || config->service_perfdata_file_template() != new_cfg.service_perfdata_file_template()) {
    xpddefault_cleanup_performance_data();
    xpddefault_initialize_performance_data();
  }

  // Initialize status file.
  if (!has_already_been_loaded
      || config->status_file() != new_cfg.status_file()) {
    xsddefault_cleanup_status_data(true);
    xsddefault_initialize_status_data();
  }

  // Set new values.
  config->accept_passive_host_checks(new_cfg.accept_passive_host_checks());
  config->accept_passive_service_checks(new_cfg.accept_passive_service_checks());
  config->additional_freshness_latency(new_cfg.additional_freshness_latency());
  config->admin_email(new_cfg.admin_email());
  config->admin_pager(new_cfg.admin_pager());
  config->allow_empty_hostgroup_assignment(new_cfg.allow_empty_hostgroup_assignment());
  config->auto_reschedule_checks(new_cfg.auto_reschedule_checks());
  config->auto_rescheduling_interval(new_cfg.auto_rescheduling_interval());
  config->auto_rescheduling_window(new_cfg.auto_rescheduling_window());
  config->cached_host_check_horizon(new_cfg.cached_host_check_horizon());
  config->cached_service_check_horizon(new_cfg.cached_service_check_horizon());
  config->cfg_main(new_cfg.cfg_main());
  config->check_external_commands(new_cfg.check_external_commands());
  config->check_host_freshness(new_cfg.check_host_freshness());
  config->check_orphaned_hosts(new_cfg.check_orphaned_hosts());
  config->check_orphaned_services(new_cfg.check_orphaned_services());
  config->check_reaper_interval(new_cfg.check_reaper_interval());
  config->check_result_path(new_cfg.check_result_path());
  config->check_service_freshness(new_cfg.check_service_freshness());
  config->command_check_interval(new_cfg.command_check_interval());
  config->date_format(new_cfg.date_format());
  config->debug_file(new_cfg.debug_file());
  config->debug_level(new_cfg.debug_level());
  config->debug_verbosity(new_cfg.debug_verbosity());
  config->enable_environment_macros(new_cfg.enable_environment_macros());
  config->enable_event_handlers(new_cfg.enable_event_handlers());
  config->enable_failure_prediction(new_cfg.enable_failure_prediction());
  config->enable_flap_detection(new_cfg.enable_flap_detection());
  config->enable_notifications(new_cfg.enable_notifications());
  config->enable_predictive_host_dependency_checks(new_cfg.enable_predictive_host_dependency_checks());
  config->enable_predictive_service_dependency_checks(new_cfg.enable_predictive_service_dependency_checks());
  config->event_broker_options(new_cfg.event_broker_options());
  config->event_handler_timeout(new_cfg.event_handler_timeout());
  config->execute_host_checks(new_cfg.execute_host_checks());
  config->execute_service_checks(new_cfg.execute_service_checks());
  config->global_host_event_handler(new_cfg.global_host_event_handler());
  config->global_service_event_handler(new_cfg.global_service_event_handler());
  config->high_host_flap_threshold(new_cfg.high_host_flap_threshold());
  config->high_service_flap_threshold(new_cfg.high_service_flap_threshold());
  config->host_check_timeout(new_cfg.host_check_timeout());
  config->host_freshness_check_interval(new_cfg.host_freshness_check_interval());
  config->host_inter_check_delay_method(new_cfg.host_inter_check_delay_method());
  config->host_perfdata_command(new_cfg.host_perfdata_command());
  config->host_perfdata_file(new_cfg.host_perfdata_file());
  config->host_perfdata_file_mode(new_cfg.host_perfdata_file_mode());
  config->host_perfdata_file_processing_command(new_cfg.host_perfdata_file_processing_command());
  config->host_perfdata_file_processing_interval(new_cfg.host_perfdata_file_processing_interval());
  config->host_perfdata_file_template(new_cfg.host_perfdata_file_template());
  config->illegal_object_chars(new_cfg.illegal_object_chars());
  config->illegal_output_chars(new_cfg.illegal_output_chars());
  config->interval_length(new_cfg.interval_length());
  config->log_event_handlers(new_cfg.log_event_handlers());
  config->log_external_commands(new_cfg.log_external_commands());
  config->log_file(new_cfg.log_file());
  config->log_host_retries(new_cfg.log_host_retries());
  config->log_initial_states(new_cfg.log_initial_states());
  config->log_notifications(new_cfg.log_notifications());
  config->log_passive_checks(new_cfg.log_passive_checks());
  config->log_service_retries(new_cfg.log_service_retries());
  config->low_host_flap_threshold(new_cfg.low_host_flap_threshold());
  config->low_service_flap_threshold(new_cfg.low_service_flap_threshold());
  config->max_check_reaper_time(new_cfg.max_check_reaper_time());
  config->max_check_result_file_age(new_cfg.max_check_result_file_age());
  config->max_debug_file_size(new_cfg.max_debug_file_size());
  config->max_host_check_spread(new_cfg.max_host_check_spread());
  config->max_log_file_size(new_cfg.max_log_file_size());
  config->max_parallel_service_checks(new_cfg.max_parallel_service_checks());
  config->max_service_check_spread(new_cfg.max_service_check_spread());
  config->notification_timeout(new_cfg.notification_timeout());
  config->object_cache_file(new_cfg.object_cache_file());
  config->obsess_over_hosts(new_cfg.obsess_over_hosts());
  config->obsess_over_services(new_cfg.obsess_over_services());
  config->ochp_command(new_cfg.ochp_command());
  config->ochp_timeout(new_cfg.ochp_timeout());
  config->ocsp_command(new_cfg.ocsp_command());
  config->ocsp_timeout(new_cfg.ocsp_timeout());
  config->passive_host_checks_are_soft(new_cfg.passive_host_checks_are_soft());
  config->perfdata_timeout(new_cfg.perfdata_timeout());
  config->precached_object_file(new_cfg.precached_object_file());
  config->process_performance_data(new_cfg.process_performance_data());
  config->resource_file(new_cfg.resource_file());
  config->retain_state_information(new_cfg.retain_state_information());
  config->retained_contact_host_attribute_mask(new_cfg.retained_contact_host_attribute_mask());
  config->retained_contact_service_attribute_mask(new_cfg.retained_contact_service_attribute_mask());
  config->retained_host_attribute_mask(new_cfg.retained_host_attribute_mask());
  config->retained_process_host_attribute_mask(new_cfg.retained_process_host_attribute_mask());
  config->retention_scheduling_horizon(new_cfg.retention_scheduling_horizon());
  config->retention_update_interval(new_cfg.retention_update_interval());
  config->service_check_timeout(new_cfg.service_check_timeout());
  config->service_freshness_check_interval(new_cfg.service_freshness_check_interval());
  config->service_inter_check_delay_method(new_cfg.service_inter_check_delay_method());
  config->service_interleave_factor_method(new_cfg.service_interleave_factor_method());
  config->service_perfdata_command(new_cfg.service_perfdata_command());
  config->service_perfdata_file(new_cfg.service_perfdata_file());
  config->service_perfdata_file_mode(new_cfg.service_perfdata_file_mode());
  config->service_perfdata_file_processing_command(new_cfg.service_perfdata_file_processing_command());
  config->service_perfdata_file_processing_interval(new_cfg.service_perfdata_file_processing_interval());
  config->service_perfdata_file_template(new_cfg.service_perfdata_file_template());
  config->sleep_time(new_cfg.sleep_time());
  config->soft_state_dependencies(new_cfg.soft_state_dependencies());
  config->state_retention_file(new_cfg.state_retention_file());
  config->status_file(new_cfg.status_file());
  config->status_update_interval(new_cfg.status_update_interval());
  config->time_change_threshold(new_cfg.time_change_threshold());
  config->translate_passive_host_checks(new_cfg.translate_passive_host_checks());
  config->use_aggressive_host_checking(new_cfg.use_aggressive_host_checking());
  config->use_check_result_path(new_cfg.use_check_result_path());
  config->use_large_installation_tweaks(new_cfg.use_large_installation_tweaks());
  config->use_regexp_matches(new_cfg.use_regexp_matches());
  config->use_retained_program_state(new_cfg.use_retained_program_state());
  config->use_retained_scheduling_info(new_cfg.use_retained_scheduling_info());
  config->use_setpgid(new_cfg.use_setpgid());
  config->use_syslog(new_cfg.use_syslog());
  config->use_true_regexp_matching(new_cfg.use_true_regexp_matching());
  config->user(new_cfg.user());

  // Set this variable just the first time.
  if (!has_already_been_loaded) {
    config->broker_module(new_cfg.broker_module());
    config->broker_module_directory(new_cfg.broker_module_directory());
    config->command_file(new_cfg.command_file());
    config->external_command_buffer_slots(new_cfg.external_command_buffer_slots());
    config->use_timezone(new_cfg.use_timezone());
  }
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
       difference<std::set<shared_ptr<ConfigurationType> > > const& diff) {
  // Type alias.
  typedef std::set<shared_ptr<ConfigurationType> > cfg_set;

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
 *  Process new configuration and apply it.
 *
 *  @param[in] new_cfg The new configuration.
 *  @param[in] state   The retention to use.
 */
void applier::state::_processing(
       configuration::state& new_cfg,
       retention::state* state) {
  // Call prelauch broker event the first time to run applier state.
  if (!has_already_been_loaded)
    broker_program_state(
      NEBTYPE_PROCESS_PRELAUNCH,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      NULL);

  // Apply logging configurations.
  applier::logging::instance().apply(new_cfg);

  // Apply globals configurations.
  applier::globals::instance().apply(new_cfg);

  // Apply macros configurations.
  applier::macros::instance().apply(new_cfg);

  //
  // Expand all objects.
  //

  // Expand timeperiods.
  _expand<configuration::timeperiod, applier::timeperiod>(
    new_cfg,
    new_cfg.timeperiods());

  // Expand connectors.
  _expand<configuration::connector, applier::connector>(
    new_cfg,
    new_cfg.connectors());

  // Expand commands.
  _expand<configuration::command, applier::command>(
    new_cfg,
    new_cfg.commands());

  // Expand contacts.
  _expand<configuration::contact, applier::contact>(
    new_cfg,
    new_cfg.contacts());

  // Expand contactgroups.
  _expand<configuration::contactgroup, applier::contactgroup>(
    new_cfg,
    new_cfg.contactgroups());

  // Expand hosts.
  _expand<configuration::host, applier::host>(
    new_cfg,
    new_cfg.hosts());

  // Expand hostgroups.
  _expand<configuration::hostgroup, applier::hostgroup>(
    new_cfg,
    new_cfg.hostgroups());

  // Expand services.
  _expand<configuration::service, applier::service>(
    new_cfg,
    new_cfg.services());

  // Expand servicegroups.
  _expand<configuration::servicegroup, applier::servicegroup>(
    new_cfg,
    new_cfg.servicegroups());

  // Expand hostdependencies.
  _expand<configuration::hostdependency, applier::hostdependency>(
    new_cfg,
    new_cfg.hostdependencies());

  // Expand servicedependencies.
  _expand<configuration::servicedependency, applier::servicedependency>(
    new_cfg,
    new_cfg.servicedependencies());

  // Expand hostescalations.
  _expand<configuration::hostescalation, applier::hostescalation>(
    new_cfg,
    new_cfg.hostescalations());

  // Expand serviceescalations.
  _expand<configuration::serviceescalation, applier::serviceescalation>(
    new_cfg,
    new_cfg.serviceescalations());

  //
  //  Build difference for all objects.
  //

  // Build difference for timeperiods.
  difference<set_timeperiod> diff_timeperiods;
  diff_timeperiods.parse(
    config->timeperiods(),
    new_cfg.timeperiods());

  // Build difference for connectors.
  difference<set_connector> diff_connectors;
  diff_connectors.parse(
    config->connectors(),
    new_cfg.connectors());

  // Build difference for commands.
  difference<set_command> diff_commands;
  diff_commands.parse(
    config->commands(),
    new_cfg.commands());

  // Build difference for contacts.
  difference<set_contact> diff_contacts;
  diff_contacts.parse(
    config->contacts(),
    new_cfg.contacts());

  // Build difference for contactgroups.
  difference<set_contactgroup> diff_contactgroups;
  diff_contactgroups.parse(
    config->contactgroups(),
    new_cfg.contactgroups());

  // Build difference for hosts.
  difference<set_host> diff_hosts;
  diff_hosts.parse(
    config->hosts(),
    new_cfg.hosts());

  // Build difference for hostgroups.
  difference<set_hostgroup> diff_hostgroups;
  diff_hostgroups.parse(
    config->hostgroups(),
    new_cfg.hostgroups());

  // Build difference for services.
  difference<set_service> diff_services;
  diff_services.parse(
    config->services(),
    new_cfg.services());

  // Build difference for servicegroups.
  difference<set_servicegroup> diff_servicegroups;
  diff_servicegroups.parse(
    config->servicegroups(),
    new_cfg.servicegroups());

  // Build difference for hostdependencies.
  difference<set_hostdependency> diff_hostdependencies;
  diff_hostdependencies.parse(
    config->hostdependencies(),
    new_cfg.hostdependencies());

  // Build difference for servicedependencies.
  difference<set_servicedependency> diff_servicedependencies;
  diff_servicedependencies.parse(
    config->servicedependencies(),
    new_cfg.servicedependencies());

  // Build difference for hostescalations.
  difference<set_hostescalation> diff_hostescalations;
  diff_hostescalations.parse(
    config->hostescalations(),
    new_cfg.hostescalations());

  // Build difference for serviceescalations.
  difference<set_serviceescalation> diff_serviceescalations;
  diff_serviceescalations.parse(
    config->serviceescalations(),
    new_cfg.serviceescalations());

  concurrency::locker lock(&_lock);
  // Check if the engine is running.
  if (has_already_been_loaded) {
    _waiting = true;
    // Wait to stop engine before apply configuration.
    _cv_lock.wake_one();
    _cv_lock.wait(&_lock);
    _waiting = false;
  }

  //
  //  Apply and resolve all objects.
  //

  // Apply timeperiods.
  _apply<configuration::timeperiod, applier::timeperiod>(
    diff_timeperiods);
  _resolve<configuration::timeperiod, applier::timeperiod>(
    config->timeperiods());

  // Apply connectors.
  _apply<configuration::connector, applier::connector>(
    diff_connectors);
  _resolve<configuration::connector, applier::connector>(
    config->connectors());

  // Apply commands.
  _apply<configuration::command, applier::command>(
    diff_commands);
  _resolve<configuration::command, applier::command>(
    config->commands());

  // Apply contacts and contactgroups.
  _apply<configuration::contact, applier::contact>(
    diff_contacts);
  _apply<configuration::contactgroup, applier::contactgroup>(
    diff_contactgroups);
  _resolve<configuration::contactgroup, applier::contactgroup>(
    config->contactgroups());
  _resolve<configuration::contact, applier::contact>(
    config->contacts());

  // Apply hosts and hostgroups.
  _apply<configuration::host, applier::host>(
    diff_hosts);
  _apply<configuration::hostgroup, applier::hostgroup>(
    diff_hostgroups);
  _resolve<configuration::hostgroup, applier::hostgroup>(
    config->hostgroups());
  _resolve<configuration::host, applier::host>(
    config->hosts());

  // Apply services and servicegroups.
  _apply<configuration::service, applier::service>(
    diff_services);
  _apply<configuration::servicegroup, applier::servicegroup>(
    diff_servicegroups);
  _resolve<configuration::servicegroup, applier::servicegroup>(
    config->servicegroups());
  _resolve<configuration::service, applier::service>(
    config->services());

  // Apply host dependencies.
  _apply<configuration::hostdependency, applier::hostdependency>(
    diff_hostdependencies);
  _resolve<configuration::hostdependency, applier::hostdependency>(
    config->hostdependencies());

  // Apply service dependencies.
  _apply<configuration::servicedependency, applier::servicedependency>(
    diff_servicedependencies);
  _resolve<configuration::servicedependency, applier::servicedependency>(
    config->servicedependencies());

  // Apply host escalations.
  _apply<configuration::hostescalation, applier::hostescalation>(
    diff_hostescalations);
  _resolve<configuration::hostescalation, applier::hostescalation>(
    config->hostescalations());

  // Apply service escalations.
  _apply<configuration::serviceescalation, applier::serviceescalation>(
    diff_serviceescalations);
  _resolve<configuration::serviceescalation, applier::serviceescalation>(
    config->serviceescalations());

  // Call start broker event the first time to run applier state.
  if (!has_already_been_loaded)
    broker_program_state(
      NEBTYPE_PROCESS_START,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      NULL);

  // Load retention.
  if (state) {
    retention::applier::state app_state;
    app_state.apply(new_cfg, *state);
  }

  // Schedule host and service checks.
  applier::scheduler::instance().apply(
    new_cfg,
    diff_hosts,
    diff_services);

  // Apply new global on the current state.
  _apply(new_cfg);

  if (has_already_been_loaded) {
    // wake up waiting thread.
    _cv_lock.wake_one();
  }

  has_already_been_loaded = true;
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

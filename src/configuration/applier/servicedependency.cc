/*
** Copyright 2011-2013,2015 Merethis
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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/servicedependency.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/object.hh"
#include "com/centreon/engine/configuration/servicedependency.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::servicedependency::servicedependency() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::servicedependency::servicedependency(
                              applier::servicedependency const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::servicedependency::~servicedependency() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::servicedependency& applier::servicedependency::operator=(
                              applier::servicedependency const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new service dependency.
 *
 *  @param[in] obj The new servicedependency to add into the monitoring
 *                 engine.
 */
void applier::servicedependency::add_object(
                                   shared_ptr<configuration::servicedependency> obj) {
  // Check service dependency.
  if ((obj->hosts().size() != 1)
      || (obj->service_description().size() != 1)
      || (obj->dependent_hosts().size() != 1)
      || (obj->dependent_service_description().size() != 1))
    throw (engine_error() << "Could not create service "
           << "dependency with multiple (dependent) hosts / services");

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new service dependency of service '"
    << obj->dependent_service_description().front() << "' of host '"
    << obj->dependent_hosts().front() << "' on service '"
    << obj->service_description().front() << "' of host '"
    << obj->hosts().front() << "'.";

  // Add dependency to the global configuration set.
  config->servicedependencies().insert(obj);

  // Create execution dependency.
  if (!add_service_dependency(
         obj->dependent_hosts().front().c_str(),
         obj->dependent_service_description().front().c_str(),
         obj->hosts().front().c_str(),
         obj->service_description().front().c_str(),
         obj->inherits_parent(),
         static_cast<bool>(
           obj->failure_options()
           & configuration::servicedependency::ok),
         static_cast<bool>(
           obj->failure_options()
           & configuration::servicedependency::warning),
         static_cast<bool>(
           obj->failure_options()
           & configuration::servicedependency::unknown),
         static_cast<bool>(
           obj->failure_options()
           & configuration::servicedependency::critical),
         static_cast<bool>(
           obj->failure_options()
           & configuration::servicedependency::pending),
         NULL_IF_EMPTY(obj->dependency_period())))
    throw (engine_error() << "Could not create service "
           << "execution dependency of service '"
           << obj->dependent_service_description().front()
           << "' of host '" << obj->dependent_hosts().front()
           << "' on service '" << obj->service_description().front()
           << "' of host '" << obj->hosts().front() << "'");

  return ;
}

/**
 *  Expand service dependency.
 *
 *  @param[in,out] obj Service dependency object to expand.
 *  @param[in,out] s   Configuration being applied.
 */
void applier::servicedependency::expand_object(
                                   shared_ptr<configuration::servicedependency> obj,
                                   configuration::state& s) {
  // Check service dependency.
  if ((obj->hosts().size() != 1)
      || (obj->service_description().size() != 1)
      || (obj->dependent_hosts().size() != 1)
      || (obj->dependent_service_description().size() != 1)) {
    // Expand depended services.
    std::set<std::pair<std::string, std::string> >
      depended_services;
    _expand_services(
      obj->hosts(),
      obj->service_description(),
      depended_services);

    // Expand dependent services.
    std::set<std::pair<std::string, std::string> >
      dependent_services;
    _expand_services(
      obj->dependent_hosts(),
      obj->dependent_service_description(),
      dependent_services);

    // Remove current service dependency.
    s.servicedependencies().erase(obj);

    // Browse all depended and dependent services.
    for (std::set<std::pair<std::string, std::string> >::const_iterator
           it1(depended_services.begin()),
           end1(depended_services.end());
         it1 != end1;
         ++it1)
      for (std::set<std::pair<std::string, std::string> >::const_iterator
             it2(dependent_services.begin()),
             end2(dependent_services.end());
           it2 != end2;
           ++it2)
        for (unsigned int i(0); i < 2; ++i) {
          // Create service dependency instance.
          shared_ptr<configuration::servicedependency>
            sdep(new configuration::servicedependency(*obj));
          sdep->hosts().clear();
          sdep->hosts().push_back(it1->first);
          sdep->service_description().clear();
          sdep->service_description().push_back(it1->second);
          sdep->dependent_hosts().clear();
          sdep->dependent_hosts().push_back(it2->first);
          sdep->dependent_service_description().clear();
          sdep->dependent_service_description().push_back(it2->second);

          // Insert new service dependency. We do not need to expand it
          // because no expansion is made on 1->1 dependency.
          s.servicedependencies().insert(sdep);
        }
  }

  return ;
}

/**
 *  @brief Modify service dependency.
 *
 *  Service dependencies cannot be defined with anything else than their
 *  full content. Therefore no modification can occur.
 *
 *  @param[in] obj Unused.
 */
void applier::servicedependency::modify_object(
                                   shared_ptr<configuration::servicedependency> obj) {
  (void)obj;
  throw (engine_error() << "Could not modify a service "
         << "dependency: service dependency objects can only be added "
         << "or removed, this is likely a software bug that you should "
         << "report to Centreon Engine developers");
  return ;
}

/**
 *  Remove old service dependency.
 *
 *  @param[in] obj The service dependency to remove from the monitoring
 *                 engine.
 */
void applier::servicedependency::remove_object(
                                   shared_ptr<configuration::servicedependency> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing a service dependency.";

  // Find service dependency.
  umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >::iterator
    it(applier::state::instance().servicedependencies_find(obj->key()));
  if (it != applier::state::instance().servicedependencies().end()) {
    servicedependency_struct* dependency(it->second.get());

    // Remove service dependency from its list.
    unregister_object<servicedependency_struct>(
      &servicedependency_list,
      dependency);

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_dependency_data(
      NEBTYPE_SERVICEDEPENDENCY_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      dependency,
      &tv);

    // Erase service dependency (will effectively delete the object).
    applier::state::instance().servicedependencies().erase(it);
  }

  // Remove dependency from the global configuration set.
  config->servicedependencies().erase(obj);

  return ;
}

/**
 *  Resolve a servicedependency.
 *
 *  @param[in] obj Servicedependency object.
 */
void applier::servicedependency::resolve_object(
                shared_ptr<configuration::servicedependency> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving a service dependency.";

  // Find service dependency.
  umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >::iterator
    it(applier::state::instance().servicedependencies_find(obj->key()));
  if (applier::state::instance().servicedependencies().end() == it)
    throw (engine_error() << "Cannot resolve non-existing "
           << "service dependency");

  // Resolve service dependency.
  if (!check_servicedependency(it->second.get(), NULL, NULL))
    throw (engine_error() << "Cannot resolve service dependency");

  return ;
}

/**
 *  Expand services.
 *
 *  @param[in]  hst       Hosts.
 *  @param[in]  svc       Service descriptions.
 *  @param[out] expanded  Expanded services.
 */
void applier::servicedependency::_expand_services(
       std::list<std::string> const& hst,
       std::list<std::string> const& svc,
       std::set<std::pair<std::string, std::string> >& expanded) {
  // Expanded hosts.
  std::set<std::string> all_hosts;

  // Base hosts.
  for (std::list<std::string>::const_iterator
         it(hst.begin()),
         end(hst.end());
       it != end;
       ++it)
    all_hosts.insert(*it);

  // Hosts * services.
  for (std::set<std::string>::const_iterator
         it_host(all_hosts.begin()),
         end_host(all_hosts.end());
       it_host != end_host;
       ++it_host)
    for (std::list<std::string>::const_iterator
           it_service(svc.begin()),
           end_service(svc.end());
         it_service != end_service;
         ++it_service)
      expanded.insert(std::make_pair(*it_host, *it_service));

  return ;
}

/*
** Copyright 2011-2019 Centreon
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

#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/contactgroup.hh"
#include "com/centreon/engine/commands/command.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/hostgroup.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/engine/objects/servicegroup.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "find.hh"

// forward declaration.
struct hostdependency_struct;
struct hostescalation_struct;
struct servicedependency_struct;
struct serviceescalation_struct;

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;

/**
 *  Given a host/service name, find the service in the list in memory.
 *
 *  @param[in] host_name Host name.
 *  @param[in] svc_desc  Service description.
 *
 *  @return Service object if found, nullptr otherwise.
 */
com::centreon::engine::service* find_service(char const* host_name, char const* svc_desc) {
  if (!host_name || !svc_desc)
    return nullptr;

  std::pair<uint64_t, uint64_t> id(get_host_and_service_id(host_name, svc_desc));
  umap<std::pair<uint64_t, uint64_t>, std::shared_ptr<com::centreon::engine::service> >::const_iterator
    it(state::instance().services().find(id));
  if (it != state::instance().services().end())
    return &(*it->second);
  return nullptr;
}

/**
 *  Find a servicegroup from the list in memory.
 *
 *  @param[in] name Service group name.
 *
 *  @return Service group object if found, nullptr otherwise.
 */
servicegroup* find_servicegroup(std::string const& name) {
  umap<std::string, std::shared_ptr<servicegroup_struct> >::const_iterator
    it(state::instance().servicegroups().find(name));
  if (it != state::instance().servicegroups().end())
    return it->second.get();
  return nullptr;
}

/**
 *  Given a timeperiod name, find the timeperiod from the list in memory.
 *
 *  @param[in] name Timeperiod name.
 *
 *  @return Timeperiod object if found, nullptr otherwise.
 */
timeperiod* find_timeperiod(std::string const& name) {
  if (name.empty())
    return nullptr;

  umap<std::string, std::shared_ptr<timeperiod_struct> >::const_iterator
    it(state::instance().timeperiods().find(name));
  if (it != state::instance().timeperiods().end())
    return it->second.get();
  return nullptr;
}

/**
 *  Deprecated function.
 *
 *  @param[in]  host_name Unused.
 *  @param[out] ptr       Unused.
 *
 *  @return nullptr.
 */
hostdependency_struct* get_first_host_dependency_by_dependent_host(
                         char const* host_name,
                         void** ptr) {
  (void)host_name;
  (void)ptr;
  return nullptr;
}

/**
 *  Deprecated function.
 *
 *  @param[in]  host_name Unused.
 *  @param[out] ptr       Unused.
 *
 *  @return nullptr.
 */
hostescalation_struct* get_first_host_escalation_by_host(
                         char const* host_name,
                         void** ptr) {
  (void)host_name;
  (void)ptr;
  return nullptr;
}

/**
 *  Deprecated function.
 *
 *  @param[in]  host_name       Unused.
 *  @param[in]  svc_description Unused.
 *  @param[out] ptr             Unused.
 *
 *  @return nullptr.
 */
servicedependency_struct* get_first_service_dependency_by_dependent_service(
                            char const* host_name,
                            char const* svc_description,
                            void** ptr) {
  (void)host_name;
  (void)svc_description;
  (void)ptr;
  return nullptr;
}

/**
 *  Deprecated function.
 *
 *  @param[in]  host_name       Unused.
 *  @param[in]  svc_description Unused.
 *  @param[out] ptr             Unused.
 *
 *  @return nullptr.
 */
serviceescalation_struct* get_first_service_escalation_by_service(
                            char const* host_name,
                            char const* svc_description,
                            void** ptr) {
  (void)host_name;
  (void)svc_description;
  (void)ptr;
  return nullptr;
}

/**
 *  Deprecated function.
 *
 *  @param[in]     host_name Unused.
 *  @param[in,out] ptr       Unused.
 *
 *  @return nullptr.
 */
hostdependency_struct* get_next_host_dependency_by_dependent_host(
                         char const* host_name,
                         void** ptr) {
  (void)host_name;
  (void)ptr;
  return nullptr;
}

/**
 *  Deprecated function.
 *
 *  @param[in]     host_name Unused.
 *  @param[in,out] ptr       Unused.
 *
 *  @return nullptr.
 */
hostescalation_struct* get_next_host_escalation_by_host(
                         char const* host_name,
                         void** ptr) {
  (void)host_name;
  (void)ptr;
  return nullptr;
}

/**
 *  Deprecated function.
 *
 *  @param[in]     host_name       Unused.
 *  @param[in]     svc_description Unused.
 *  @param[in,out] ptr             Unused.
 *
 *  @return nullptr.
 */
servicedependency_struct* get_next_service_dependency_by_dependent_service(
                            char const* host_name,
                            char const* svc_description,
                            void** ptr) {
  (void)host_name;
  (void)svc_description;
  (void)ptr;
  return nullptr;
}

/**
 *  Deprecated function.
 *
 *  @param[in]     host_name       Unused.
 *  @param[in]     svc_description Unused.
 *  @param[in,out] ptr             Unused.
 *
 *  @return nullptr
 */
serviceescalation_struct* get_next_service_escalation_by_service(
                            char const* host_name,
                            char const* svc_description,
                            void** ptr) {
  (void)host_name;
  (void)svc_description;
  (void)ptr;
  return nullptr;
}

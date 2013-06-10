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

#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine::configuration::applier;

/**
 *  Given a command name, find a command from the list in memory.
 *
 *  @param[in] name Command name.
 *
 *  @return Command object if found, NULL otherwise.
 */
command* find_command(char const* name) {
  if (!name)
    return (NULL);

  umap<std::string, shared_ptr<command_struct> >::const_iterator
    it(state::instance().commands().find(name));
  if (it != state::instance().commands().end())
    return (&(*it->second));
  return (NULL);
}

/**
 *  Find a contact from the list in memory.
 *
 *  @param[in] name Contact name.
 *
 *  @return Contact object if found, NULL otherwise.
 */
contact* find_contact(char const* name) {
  if (!name)
    return (NULL);

  umap<std::string, shared_ptr<contact_struct> >::const_iterator
    it(state::instance().contacts().find(name));
  if (it != state::instance().contacts().end())
    return (&(*it->second));
  return (NULL);
}

/**
 *  Find a contact group from the list in memory.
 *
 *  @param[in] name Contact group name.
 *
 *  @return Contact group object if found, NULL otherwise.
 */
contactgroup* find_contactgroup(char const* name) {
  if (!name)
    return (NULL);

  umap<std::string, shared_ptr<contactgroup_struct> >::const_iterator
    it(state::instance().contactgroups().find(name));
  if (it != state::instance().contactgroups().end())
    return (&(*it->second));
  return (NULL);
}

/**
 *  Given a host name, find it in the list in memory.
 *
 *  @param[in] name Host name.
 *
 *  @return Host object if found, NULL otherwise.
 */
host* find_host(char const* name) {
  if (!name)
    return (NULL);

  umap<std::string, shared_ptr<host_struct> >::const_iterator
    it(state::instance().hosts().find(name));
  if (it != state::instance().hosts().end())
    return (&(*it->second));
  return (NULL);
}

/**
 *  Find a hostgroup from the list in memory.
 *
 *  @param[in] name Host group name.
 *
 *  @return Host group object if found, NULL otherwise.
 */
hostgroup* find_hostgroup(char const* name) {
  if (!name)
    return (NULL);

  umap<std::string, shared_ptr<hostgroup_struct> >::const_iterator
    it(state::instance().hostgroups().find(name));
  if (it != state::instance().hostgroups().end())
    return (&(*it->second));
  return (NULL);
}

/**
 *  Given a host/service name, find the service in the list in memory.
 *
 *  @param[in] host_name Host name.
 *  @param[in] svc_desc  Service description.
 *
 *  @return Service object if found, NULL otherwise.
 */
service* find_service(char const* host_name, char const* svc_desc) {
  if (!host_name || !svc_desc)
    return (NULL);

  umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
    it(state::instance().services().find(std::make_pair(host_name, svc_desc)));
  if (it != state::instance().services().end())
    return (&(*it->second));
  return (NULL);
}

/**
 *  Find a servicegroup from the list in memory.
 *
 *  @param[in] name Service group name.
 *
 *  @return Service group object if found, NULL otherwise.
 */
servicegroup* find_servicegroup(char const* name) {
  if (!name)
    return (NULL);

  umap<std::string, shared_ptr<servicegroup_struct> >::const_iterator
    it(state::instance().servicegroups().find(name));
  if (it != state::instance().servicegroups().end())
    return (&(*it->second));
  return (NULL);
}

/**
 *  Given a timeperiod name, find the timeperiod from the list in memory.
 *
 *  @param[in] name Timeperiod name.
 *
 *  @return Timeperiod object if found, NULL otherwise.
 */
timeperiod* find_timeperiod(char const* name) {
  if (!name)
    return (NULL);

  umap<std::string, shared_ptr<timeperiod_struct> >::const_iterator
    it(state::instance().timeperiods().find(name));
  if (it != state::instance().timeperiods().end())
    return (&(*it->second));
  return (NULL);
}

/*
** Copyright 2011-2012 Merethis
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
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/host.hh"
#include "com/centreon/engine/objects/hostgroup.hh"
#include "com/centreon/engine/objects/hostsmember.hh"
#include "com/centreon/engine/objects/utils.hh"
#include "com/centreon/engine/skiplist.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::objects::utils;

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::link
 */
bool link_hostgroup(
       hostgroup* obj,
       host** members,
       hostgroup** groups) {
  try {
    objects::link(
               obj,
               tab2vec(members),
               tab2vec(groups));
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << "error: " << e.what();
    return (false);
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << "error: link_hostgroup: unknow exception";
    return (false);
  }
  return (true);
}

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::release
 */
void release_hostgroup(hostgroup const* obj) {
  try {
    objects::release(obj);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << "error: " << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << "error: release_hostgroup: unknow exception";
  }
  return;
}

/**
 *  Link an hostgroup with hosts and groups into the engine.
 *
 *  @param[in,out] obj     Object to link with correct group_name.
 *  @param[in]     members The table with hosts member name.
 *  @param[in]     groups  The table with host groups member name.
 */
void objects::link(
                hostgroup* obj,
                std::vector<host*> const& members,
                std::vector<hostgroup*> const& groups) {
  // Check object contents.
  if (!obj)
    throw (engine_error() << "hostgroup is a NULL pointer");
  if (!obj->group_name)
    throw (engine_error() << "hostgroup invalid group name");

  // Add all hosts into the hostgroup.
  if (!add_hosts_to_object(members, &obj->members))
    throw (engine_error() << "hostgroup '" << obj->group_name
           << "' invalid member");

  // Broker timestamp.
  timeval tv(get_broker_timestamp(NULL));

  // Browse hosts.
  for (std::vector<host*>::const_iterator
         it(members.begin()), end(members.end());
       it != end;
       ++it) {
    // Link host group to host.
    add_object_to_objectlist(&(*it)->hostgroups_ptr, obj);

    // Notify event broker of new member.
    broker_group_member(
      NEBTYPE_HOSTGROUPMEMBER_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj,
      *it,
      &tv);
  }

  // Add the content of other hostgroups into this hostgroup.
  std::vector<host*> other_members;
  for (std::vector<hostgroup*>::const_iterator
         it(groups.begin()), end(groups.end());
       it != end;
       ++it) {
    if (!*it)
      throw (engine_error() << "hostgroup '" << obj->group_name
             << "' invalid group member");
    // Browse members.
    for (hostsmember* mbr((*it)->members);
         mbr;
         mbr = mbr->next)
      other_members.push_back(mbr->host_ptr);
  }

  // Recursive call.
  if (!other_members.empty())
    objects::link(obj, other_members, std::vector<hostgroup*>());
  return;
}

/**
 *  Cleanup memory of hostgroup.
 *
 *  @param[in] obj The hostgroup to cleanup memory.
 */
void objects::release(hostgroup const* obj) {
  if (obj == NULL)
    return;

  hostsmember const* member = obj->members;
  while ((member = release(member)));

  skiplist_delete(object_skiplists[HOSTGROUP_SKIPLIST], obj);
  remove_object_list(obj, &hostgroup_list, &hostgroup_list_tail);

  delete[] obj->group_name;
  delete[] obj->alias;
  delete[] obj->notes;
  delete[] obj->notes_url;
  delete[] obj->action_url;
  delete obj;
  return;
}

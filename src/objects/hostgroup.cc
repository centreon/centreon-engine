/*
** Copyright 2011 Merethis
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

#include "error.hh"
#include "globals.hh"
#include "skiplist.hh"
#include "logging/logger.hh"
#include "objects/utils.hh"
#include "objects/host.hh"
#include "objects/hostsmember.hh"
#include "objects/hostgroup.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::objects::utils;

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::link
 */
bool link_hostgroup(hostgroup* obj,
                    host** members,
                    hostgroup** groups) {
  try {
    objects::link(obj,
                  tab2qvec(members),
                  tab2qvec(groups));
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << e.what();
    return (false);
  }
  catch (...) {
    logger(log_runtime_error, basic) << Q_FUNC_INFO << " unknow exception.";
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
    logger(log_runtime_error, basic) << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic) << Q_FUNC_INFO << " unknow exception.";
  }
}

/**
 *  Link an hostgroup with hosts and groups into the engine.
 *
 *  @param[in,out] obj     Object to link with correct group_name.
 *  @param[in]     members The table with hosts member name.
 *  @param[in]     groups  The table with host groups member name.
 */
void objects::link(hostgroup* obj,
                   QVector<host*> const& members,
                   QVector<hostgroup*> const& groups) {
  // check object contents.
  if (obj == NULL)
    throw (engine_error() << "hostgroup is a NULL pointer.");
  if (obj->group_name == NULL)
    throw (engine_error() << "hostgroup invalid group name.");

  // add all host into the hostgroup.
  if (members.empty() || add_hosts_to_object(members, &obj->members) == false)
    throw (engine_error() << "hostgroup '" << obj->group_name << "' invalid member.");

  // add the content of other hostgroups into this hostgroup.
  for (QVector<hostgroup*>::const_iterator it = groups.begin(),
	 end = groups.end();
       it != end;
       ++it) {
    if (*it == NULL)
      throw (engine_error() << "hostgroup '" << obj->group_name << "' invalid group member.");

    hostsmember* hstmembers = (*it)->members;
    while (hstmembers) {
      hostsmember* member = add_host_to_hostgroup(obj, hstmembers->host_name);
      member->host_ptr = hstmembers->host_ptr;
      hstmembers = hstmembers->next;
    }
  }

  for (hostsmember const* hm = obj->members; hm != NULL; hm = hm->next)
    add_object_to_objectlist(&hm->host_ptr->hostgroups_ptr, obj);
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
}

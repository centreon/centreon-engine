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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/servicegroup.hh"
#include "com/centreon/engine/objects/servicesmember.hh"
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
bool link_servicegroup(
       servicegroup* obj,
       service** members,
       servicegroup** groups) {
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
      << "error: link_servicegroup: unknow exception";
    return (false);
  }
  return (true);
}

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::release
 */
void release_servicegroup(servicegroup const* obj) {
  try {
    objects::release(obj);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << "error: " << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << "error: release_servicegroup: unknow exception";
  }
  return;
}

/**
 *  Link a servicegroup with services and groups into the engine.
 *
 *  @param[in,out] obj     Object to link with correct group_name.
 *  @param[in]     members The table with services member name.
 *  @param[in]     groups  The table with service groups member name.
 */
void objects::link(
                servicegroup* obj,
                std::vector<service*> const& members,
                std::vector<servicegroup*> const& groups) {
  // Check object contents.
  if (!obj)
    throw (engine_error() << "servicegroup is a NULL pointer");
  if (!obj->group_name)
    throw (engine_error() << "servicegroup invalid group name");

  // Broker timestamp.
  timeval tv(get_broker_timestamp(NULL));

  // Add all services into the servicegroup.
  for (std::vector<service*>::const_iterator
         it(members.begin()), end(members.end());
       it != end;
       ++it) {
    if (!*it)
      throw (engine_error() << "servicegroup '" << obj->group_name
             << "' invalid member");

    // Create a new servicegroupsmember and add it
    // into the servicegroup list.
    servicesmember* member(add_service_to_servicegroup(
                             obj,
                             (*it)->host_name,
                             (*it)->description));

    // Add service to the servicesmember.
    member->service_ptr = *it;

    // Link service group to service.
    add_object_to_objectlist(&(*it)->servicegroups_ptr, obj);

    // Notify event broker of new member.
    broker_group_member(
      NEBTYPE_SERVICEGROUPMEMBER_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj,
      *it,
      &tv);
  }

  // Add the content of other servicegroups into this servicegroup.
  std::vector<service*> other_members;
  for (std::vector<servicegroup*>::const_iterator
         it(groups.begin()), end(groups.end());
       it != end;
       ++it) {
    if (!*it)
      throw (engine_error() << "servicegroup '" << obj->group_name
             << "' invalid group member");
    // Browse members.
    for (servicesmember* mbr((*it)->members);
         mbr;
         mbr = mbr->next)
      other_members.push_back(mbr->service_ptr);
  }

  // Recursive call.
  if (!other_members.empty())
    objects::link(obj, other_members, std::vector<servicegroup*>());
  return;
}

/**
 *  Cleanup memory of servicegroup.
 *
 *  @param[in] obj The servicegroup to cleanup memory.
 */
void objects::release(servicegroup const* obj) {
  if (obj == NULL)
    return;

  servicesmember const* member = obj->members;
  while ((member = release(member))) {}

  skiplist_delete(object_skiplists[SERVICEGROUP_SKIPLIST], obj);
  remove_object_list(obj, &servicegroup_list, &servicegroup_list_tail);

  delete[] obj->group_name;
  delete[] obj->alias;
  delete[] obj->notes;
  delete[] obj->notes_url;
  delete[] obj->action_url;
  delete obj;
  return;
}

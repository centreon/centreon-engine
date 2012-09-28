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
#include "com/centreon/engine/objects/contact.hh"
#include "com/centreon/engine/objects/contactgroup.hh"
#include "com/centreon/engine/objects/contactsmember.hh"
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
bool link_contactgroup(
       contactgroup* obj,
       contact** members,
       contactgroup** groups) {
  try {
    objects::link(obj, tab2vec(members), tab2vec(groups));
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << e.what();
    return (false);
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << __func__ << " unknow exception";
    return (false);
  }
  return (true);
}


/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::release
 */
void release_contactgroup(contactgroup const* obj) {
  try {
    objects::release(obj);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << __func__ << " unknow exception";
  }
  return;
}

/**
 *  Link a contactgroup with contacts and groups into the engine.
 *
 *  @param[in,out] obj     Object to link with correct group_name.
 *  @param[in]     members The table with contacts member name.
 *  @param[in]     groups  The table with contact groups member name.
 */
void objects::link(
                contactgroup* obj,
                std::vector<contact*> const& members,
                std::vector<contactgroup*> const& groups) {
  // Check object contents.
  if (!obj)
    throw (engine_error() << "contact group is a NULL pointer");
  if (!obj->group_name)
    throw (engine_error() << "contact group invalid group name");

  // Add all contacts into the contactgroup.
  if (!add_contacts_to_object(members, &obj->members))
    throw (engine_error() << "contactgroup '" << obj->group_name
           << "' invalid member");

  // Broker timestamp.
  timeval tv(get_broker_timestamp(NULL));

  // Browse contacts.
  for (std::vector<contact*>::const_iterator
         it(members.begin()), end(members.end());
       it != end;
       ++it) {
    // Link contact group to contact.
    add_object_to_objectlist(&(*it)->contactgroups_ptr, obj);

    // Notify event broker of new member.
    broker_group_member(
      NEBTYPE_CONTACTGROUPMEMBER_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj,
      *it,
      &tv);
  }

  // Add the content of other contactgroups into this contactgroup.
  std::vector<contact*> other_members;
  for (std::vector<contactgroup*>::const_iterator
         it(groups.begin()), end(groups.end());
       it != end;
       ++it) {
    if (!*it)
      throw (engine_error() << "contactgroup '" << obj->group_name
             << "' invalid group member");
    // Browse members.
    for (contactsmember* mbr((*it)->members);
         mbr;
         mbr = mbr->next)
      other_members.push_back(mbr->contact_ptr);
  }

  // Recursive call.
  if (!other_members.empty())
    objects::link(obj, other_members, std::vector<contactgroup*>());
  return;
}

/**
 *  Cleanup memory of contactgroup.
 *
 *  @param[in] obj The contactgroup to cleanup memory.
 */
void objects::release(contactgroup const* obj) {
  if (obj == NULL)
    return;

  contactsmember const* member = obj->members;
  while ((member = release(member)));

  skiplist_delete(object_skiplists[CONTACTGROUP_SKIPLIST], obj);
  remove_object_list(obj, &contactgroup_list, &contactgroup_list_tail);

  delete[] obj->group_name;
  delete[] obj->alias;
  delete obj;
  return;
}

/**
 *  Add somme contactgroups to a generic object with contactgroups member list.
 *
 *  @param[in]  contactgroups     The contactgroups to insert.
 *  @param[out] list_contactgroup The object contactgroup.
 *
 *  @return True if insert sucessfuly, false otherwise.
 */
bool objects::add_contactgroups_to_object(
                std::vector<contactgroup*> const& contactgroups,
                contactgroupsmember** list_contactgroup) {
  if (list_contactgroup == NULL)
    return (false);

  for (std::vector<contactgroup*>::const_iterator
         it = contactgroups.begin(), end = contactgroups.end();
       it != end;
       ++it) {
    if (*it == NULL)
      return (false);

    // create a new contactgroupsmember and add it into the contactgroup list.
    contactgroupsmember* member = new contactgroupsmember;
    memset(member, 0, sizeof(*member));

    member->group_name = my_strdup((*it)->group_name);
    member->next = *list_contactgroup;
    *list_contactgroup = member;

    // add contactgroup to the contactgroupsmember.
    member->group_ptr = *it;
  }
  return (true);
}

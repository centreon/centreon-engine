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
#include "objects/contact.hh"
#include "objects/contactsmember.hh"
#include "objects/contactgroup.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::objects::utils;

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::link
 */
bool link_contactgroup(contactgroup* obj,
                       contact** members,
                       contactgroup** groups) {
  try {
    objects::link(obj, tab2qvec(members), tab2qvec(groups));
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
void release_contactgroup(contactgroup const* obj) {
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
 *  Link a contactgroup with contacts and groups into the engine.
 *
 *  @param[in,out] obj     Object to link with correct group_name.
 *  @param[in]     members The table with contacts member name.
 *  @param[in]     groups  The table with contact groups member name.
 */
void objects::link(contactgroup* obj,
                   QVector<contact*> const& members,
                   QVector<contactgroup*> const& groups) {
  // check object contents.
  if (obj == NULL)
    throw (engine_error() << "contactgroup is a NULL pointer.");
  if (obj->group_name == NULL)
    throw (engine_error() << "contactgroup invalid group name.");

  // add all contacts into the contactgroup.
  if (add_contacts_to_object(members, &obj->members) == false)
    throw (engine_error() << "contactgroup '" << obj->group_name << "' invalid member.");

  // add the content of other contactgroups into this contactgroup.
  for (QVector<contactgroup*>::const_iterator it = groups.begin(), end = groups.end();
       it != end;
       ++it) {
    if (*it == NULL)
      throw (engine_error() << "contactgroup '" << obj->group_name << "' invalid group member.");

    contactsmember* cntctmembers = (*it)->members;
    while (cntctmembers) {
      contactsmember* member = add_contact_to_contactgroup(obj, cntctmembers->contact_name);
      member->contact_ptr = cntctmembers->contact_ptr;
      cntctmembers = cntctmembers->next;
    }
  }

  for (contactsmember const* cm = obj->members; cm != NULL; cm = cm->next)
    add_object_to_objectlist(&cm->contact_ptr->contactgroups_ptr, obj);
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
}

/**
 *  Add somme contactgroups to a generic object with contactgroups member list.
 *
 *  @param[in]  contactgroups     The contactgroups to insert.
 *  @param[out] list_contactgroup The object contactgroup.
 *
 *  @return True if insert sucessfuly, false otherwise.
 */
bool objects::add_contactgroups_to_object(QVector<contactgroup*> const& contactgroups,
                                          contactgroupsmember** list_contactgroup) {
  if (list_contactgroup == NULL)
    return (false);

  for (QVector<contactgroup*>::const_iterator it = contactgroups.begin(),
         end = contactgroups.end();
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

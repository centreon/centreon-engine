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
#include "objects/servicesmember.hh"
#include "objects/servicegroup.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::objects::utils;

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::link
 */
bool link_servicegroup(servicegroup* obj,
                       service** members,
                       servicegroup** groups) {
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
void release_servicegroup(servicegroup const* obj) {
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
 *  Link a servicegroup with services and groups into the engine.
 *
 *  @param[in,out] obj     Object to link with correct group_name.
 *  @param[in]     members The table with services member name.
 *  @param[in]     groups  The table with service groups member name.
 */
void objects::link(servicegroup* obj,
                   QVector<service*> const& members,
                   QVector<servicegroup*> const& groups) {
  // check object contents.
  if (obj == NULL)
    throw (engine_error() << "servicegroup is a NULL pointer.");
  if (obj->group_name == NULL)
    throw (engine_error() << "servicegroup invalid group name.");

  // add all services into the servicegroup.
  for (QVector<service*>::const_iterator it = members.begin(),
  	 end = members.end();
       it != end;
       ++it) {
    if (*it == NULL)
      throw (engine_error() << "servicegroup '" << obj->group_name << "' invalid member.");

    // create a new servicegroupsmember and add it into the servicegroup list.
    servicesmember* member = add_service_to_servicegroup(obj,
							 (*it)->host_name,
							 (*it)->description);
    // add service to the servicesmember.
    member->service_ptr = *it;
  }

  // add the content of other servicegroups into this servicegroup.
  for (QVector<servicegroup*>::const_iterator it = groups.begin(),
	 end = groups.end();
       it != end;
       ++it) {
    if (*it == NULL)
      throw (engine_error() << "servicegroup '" << obj->group_name
	     << "' invalid group member.");

    servicesmember* svcmembers = (*it)->members;
    while (svcmembers) {
      servicesmember* member = add_service_to_servicegroup(obj,
							   svcmembers->host_name,
							   svcmembers->service_description);
      member->service_ptr = svcmembers->service_ptr;
      svcmembers = svcmembers->next;
    }
  }

  for (servicesmember const* sm = obj->members; sm != NULL; sm = sm->next)
    add_object_to_objectlist(&sm->service_ptr->servicegroups_ptr, obj);
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
  while ((member = release(member)));

  skiplist_delete(object_skiplists[SERVICEGROUP_SKIPLIST], obj);
  remove_object_list(obj, &servicegroup_list, &servicegroup_list_tail);

  delete[] obj->group_name;
  delete[] obj->alias;
  delete[] obj->notes;
  delete[] obj->notes_url;
  delete[] obj->action_url;
  delete obj;
}

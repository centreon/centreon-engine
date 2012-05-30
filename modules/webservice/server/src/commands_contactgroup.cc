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

#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/webservice/commands.hh"
#include "com/centreon/engine/modules/webservice/create_object.hh"
#include "com/centreon/engine/modules/webservice/sync_lock.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/objects/contactgroup.hh"
#include "soapH.h"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules;
using namespace com::centreon::engine::modules::webservice;

/**
 *  Create a new contactgroup into the engine.
 *
 *  @param[in] cntctgrp The struct with all information to create new contactgroup.
 */
void webservice::create_contactgroup(
                   ns1__contactgroupType const& cntctgrp) {
  // Create a new contactgroup.
  contactgroup* group(add_contactgroup(
                        cntctgrp.name.c_str(),
                        cntctgrp.alias.c_str()));

  // Add all contacts into the contactgroup.
  QVector<contact*>
    cntct_members(_find<contact>(
                    cntctgrp.members,
                    (void* (*)(char const*))&find_contact));
  if (cntctgrp.members.size()
      != static_cast<size_t>(cntct_members.size())) {
    objects::release(group);
    throw (engine_error() << "contactgroup '" << cntctgrp.name
           << "' has invalid contact member");
  }

  // Add the content of other contactgroups into this contactgroup.
  QVector<contactgroup*>
    cntct_groups(_find<contactgroup>(
                   cntctgrp.contactgroupMembers,
                   (void* (*)(char const*))&find_contactgroup));
  if (cntctgrp.contactgroupMembers.size()
      != static_cast<size_t>(cntct_groups.size())) {
    objects::release(group);
    throw (engine_error() << "contactgroup '" << cntctgrp.name
           << "' has invalid group member");
  }

  try {
    // Link contact group with other objects.
    objects::link(group, cntct_members, cntct_groups);
  }
  catch (std::exception const& e) {
    (void)e;
    objects::release(group);
    throw ;
  }

  return ;
}

/**
 *  Add a contactgroup.
 *
 *  @param[in]  s        SOAP object.
 *  @param[in]  cntctgrp Contact group to add.
 *  @param[out] res      Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactgroupAdd(
      soap* s,
      ns1__contactgroupType* cntctgrp,
      centreonengine__contactgroupAddResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(cntctgrp->name << ", " << cntctgrp->alias)

  // Create contact group.
  create_contactgroup(*cntctgrp);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Modify a contact group.
 *
 *  @param[in]  s        SOAP object.
 *  @param[in]  cntctgrp Contact group information.
 *  @param[out] res      Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactgroupModify(
      soap* s,
      ns1__contactgroupType* cntctgrp,
      centreonengine__contactgroupModifyResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(cntctgrp->name)

  // XXX

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Remove a contact group.
 *
 *  @param[in]  s               SOAP object.
 *  @param[in]  contactgroup_id Contact group to remove.
 *  @param[out] res             Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactgroupRemove(
      soap* s,
      ns1__contactgroupIDType* contactgroup_id,
      centreonengine__contactgroupRemoveResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contactgroup_id->contactgroup)

  // Remove contact group.
  if (!remove_contactgroup_by_id(
         contactgroup_id->contactgroup.c_str())) {
    std::string* error(soap_new_std__string(s, 1));
    *error = "contact group '"
             + contactgroup_id->contactgroup
             + "' not found";
    logger(log_runtime_error, more)
      << "Webservice: " << __func__ << " failed: " << *error;
    return (soap_receiver_fault(
              s,
              "Invalid parameter",
              error->c_str()));
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

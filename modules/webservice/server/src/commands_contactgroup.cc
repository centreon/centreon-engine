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
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/webservice/commands.hh"
#include "com/centreon/engine/modules/webservice/create_object.hh"
#include "com/centreon/engine/modules/webservice/sync_lock.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/objects/contactgroup.hh"
#include "soapH.h"

using namespace com::centreon::engine;
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
                        (cntctgrp.alias && !cntctgrp.alias->empty())
                        ? cntctgrp.alias->c_str()
                        : cntctgrp.name.c_str()));

  // Add all contacts into the contactgroup.
  QVector<contact*>
    cntct_members(_find<contact>(
                    cntctgrp.contactMembers,
                    (void* (*)(char const*))&find_contact));
  if (cntctgrp.contactMembers.size()
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
  COMMAND_BEGIN(cntctgrp->name)

  // Create contact group.
  create_contactgroup(*cntctgrp);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Add a contact to a contactgroup.
 *
 *  @param[in]  s           SOAP object.
 *  @param[in]  cntctgrp_id Target contact group.
 *  @param[in]  cntct_id    Target contact.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactgroupAddContact(
      soap* s,
      ns1__contactgroupIDType* cntctgrp_id,
      ns1__contactIDType* cntct_id,
      centreonengine__contactgroupAddContactResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(cntctgrp_id->name << ", " << cntct_id->name)

  // Find target contact group.
  contactgroup* cntctgrp(find_contactgroup(cntctgrp_id->name.c_str()));
  if (!cntctgrp)
    throw (engine_error() << "cannot link contact '"
           << cntct_id->name << "' to non-existent contact group '"
           << cntctgrp_id->name << "'");

  // Find target contact.
  contact* cntct(find_contact(cntct_id->name.c_str()));
  if (!cntct)
    throw (engine_error() << "cannot link non-existent contact '"
           << cntct_id->name << "' to contact group '"
           << cntctgrp_id->name << "'");

  // Member array.
  QVector<contact*> member;
  member.push_back(cntct);

  // Link contact group to contacts.
  objects::link(cntctgrp, member, QVector<contactgroup*>());

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Add a contactgroup to another contactgroup.
 *
 *  @param[in]  s                  SOAP object.
 *  @param[in]  parent_cntctgrp_id Parent contact group.
 *  @param[in]  child_cntctgrp_id  Child contact group.
 *  @param[out] res                Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactgroupAddContactgroup(
      soap* s,
      ns1__contactgroupIDType* parent_cntctgrp_id,
      ns1__contactgroupIDType* child_cntctgrp_id,
      centreonengine__contactgroupAddContactgroupResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(parent_cntctgrp_id->name << ", " 
                << child_cntctgrp_id->name)

  // Find parent contact group.
  contactgroup*
    parent_grp(find_contactgroup(parent_cntctgrp_id->name.c_str()));
  if (!parent_grp)
    throw (engine_error() << "cannot link contact group '"
           << child_cntctgrp_id->name
           << "' to non-existent contact group '"
           << parent_cntctgrp_id->name << "'");

  // Find child contact group.
  contactgroup*
    child_grp(find_contactgroup(child_cntctgrp_id->name.c_str()));
  if (!child_grp)
    throw (engine_error() << "cannot link non-existent contact group '"
           << child_cntctgrp_id->name << "' to contact group '"
           << parent_cntctgrp_id->name << "'");

  // Member array.
  QVector<contactgroup*> member;
  member.push_back(child_grp);

  // Link contact groups.
  objects::link(parent_grp, QVector<contact*>(), member);

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

  // Find target contact group.
  contactgroup* grp(find_contactgroup(cntctgrp->name.c_str()));
  if (!grp)
    throw (engine_error()
           << "cannot modify non-existent contact group '"
           << cntctgrp->name);

  // Update alias.
  if (cntctgrp->alias) {
    delete [] grp->alias;
    grp->alias = my_strdup(cntctgrp->alias->c_str());
  }

  // Update contact members.
  // XXX

  // Update contact group members.
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
  COMMAND_BEGIN(contactgroup_id->name)

  // Remove contact group.
  if (!remove_contactgroup_by_id(
         contactgroup_id->name.c_str())) {
    std::string* error(soap_new_std__string(s, 1));
    *error = "contact group '"
             + contactgroup_id->name
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

/**
 *  Remove a contact from a contactgroup.
 *
 *  @param[in]  s           SOAP object.
 *  @param[in]  cntctgrp_id Target contact group.
 *  @param[in]  cntct_id    Target contact.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactgroupRemoveContact(
      soap* s,
      ns1__contactgroupIDType* cntctgrp_id,
      ns1__contactIDType* cntct_id,
      centreonengine__contactgroupRemoveContactResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(cntctgrp_id->name << ", " << cntct_id->name)

  // Find target contact group.
  contactgroup* cntctgrp(find_contactgroup(cntctgrp_id->name.c_str()));
  if (!cntctgrp)
    throw (engine_error() << "cannot remove contact '"
           << cntct_id->name << "' from non-existent contact group '"
           << cntctgrp_id->name << "'");

  // Find contact member.
  contactsmember* current;
  contactsmember* prev;
  for (current = cntctgrp->members, prev = NULL;
       current;
       prev = current, current = current->next)
    if (!strcmp(current->contact_ptr->name, cntct_id->name.c_str()))
      break ;
  if (current) {
    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group_member(
      NEBTYPE_CONTACTGROUPMEMBER_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      cntctgrp,
      current->contact_ptr,
      &tv);

    // Backup contact.
    contact* cntct(current->contact_ptr);

    // Remove link from group to contact.
    if (prev)
      prev->next = current->next;
    else
      cntctgrp->members = NULL;
    delete [] current->contact_name;
    delete current;

    // Remove link from contact to group.
    remove_object_to_objectlist(&cntct->contactgroups_ptr, cntctgrp);
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

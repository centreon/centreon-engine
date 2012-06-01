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
#include "com/centreon/engine/modules/webservice/commands.hh"
#include "com/centreon/engine/modules/webservice/create_object.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/objects/servicegroup.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules;
using namespace com::centreon::engine::modules::webservice;

/**
 *  Create a new servicegroup into the engine.
 *
 *  @param[in] svcgrp The struct with all information to create new
 *                    servicegroup.
 */
void webservice::create_servicegroup(ns1__servicegroupType const& svcgrp) {
  // Create new service group.
  servicegroup* grp(add_servicegroup(
                      svcgrp.id->name.c_str(),
                      (svcgrp.alias ? svcgrp.alias->c_str() : NULL),
                      (svcgrp.notes ? svcgrp.notes->c_str() : NULL),
                      (svcgrp.notesUrl
                       ? svcgrp.notesUrl->c_str()
                       : NULL),
                      (svcgrp.actionUrl
                       ? svcgrp.actionUrl->c_str()
                       : NULL)));

  try {
    // Members must be host name and service description.
    if (svcgrp.serviceMembers.size() & 1)
      throw (engine_error() << "service members for service group '"
             << svcgrp.id->name
             << "' provide an odd number whereas it should be even");

    // Add all services into the servicegroup.
    QVector<service*> svc_members(::_find(svcgrp.serviceMembers));
    if (static_cast<int>(svcgrp.serviceMembers.size() / 2)
        != svc_members.size())
      throw (engine_error() << "servicegroup '" << svcgrp.id->name
             << "' invalid group member");

    // Add the content of other servicegroups to this servicegroup.
    QVector<servicegroup*>
      svc_groups(_find<servicegroup>(
                   svcgrp.servicegroupMembers,
                   (void* (*)(char const*))&find_servicegroup));
    if (static_cast<int>(svcgrp.servicegroupMembers.size())
        != svc_groups.size())
      throw (engine_error() << "servicegroup '" << svcgrp.id->name
             << "' invalid group member");

    // Link objects together.
    objects::link(grp, svc_members, svc_groups);
  }
  catch (std::exception const& e) {
    (void)e;

    // Release group in case of error.
    objects::release(grp);

    // Rethrow exception.
    throw ;
  }

  return ;
}

/**
 *  Create new service group.
 *
 *  @param[in]  s            SOAP object.
 *  @param[in]  serviceGroup Servicegroup to add.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__servicegroupAdd(
      soap* s,
      ns1__servicegroupType* serviceGroup,
      centreonengine__servicegroupAddResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(serviceGroup->id->name)

  // Create service group.
  create_servicegroup(*serviceGroup);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Add a service to a service group.
 *
 *  @param[in]  s               SOAP object.
 *  @param[in]  servicegroup_id Target service group.
 *  @param[in]  service_id      Target service.
 *  @param[out] res             Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__servicegroupAddService(
      soap* s,
      ns1__servicegroupIDType* servicegroup_id,
      ns1__serviceIDType* service_id,
      centreonengine__servicegroupAddServiceResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(servicegroup_id->name << ", (" << service_id->host
                << ", " << service_id->service <<")")

  // Find target service group.
  servicegroup* svcgrp(find_servicegroup(
                         servicegroup_id->name.c_str()));
  if (!svcgrp)
    throw (engine_error() << "cannot link service ('"
           << service_id->host->name << "', '" << service_id->service
           << "') to non-existent service group '"
           << servicegroup_id->name << "'");

  // Find target service.
  service* svc(find_service(
                 service_id->host->name.c_str(),
                 service_id->service.c_str()));
  if (!svc)
    throw (engine_error() << "cannot link non-existent service ('"
           << service_id->host->name << "', '" << service_id->service
           << "') to service group '" << servicegroup_id->name << "'");

  // Member array.
  QVector<service*> member;
  member.push_back(svc);

  // Link service and service group.
  objects::link(svcgrp, member, QVector<servicegroup*>());

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Add a service group to a service group.
 *
 *  @param[in]  s         SOAP object.
 *  @param[in]  parent_id Parent service group.
 *  @param[in]  child_id  Child service group.
 *  @param[out] res       Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__servicegroupAddServicegroup(
      soap* s,
      ns1__servicegroupIDType* parent_id,
      ns1__servicegroupIDType* child_id,
      centreonengine__servicegroupAddServicegroupResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(parent_id->name << ", " << child_id->name)

  // Find parent service group.
  servicegroup* parent_grp(find_servicegroup(parent_id->name.c_str()));
  if (!parent_grp)
    throw (engine_error() << "cannot link service group '"
           << child_id->name << "' to non-existent service group '"
           << parent_id->name << "'");

  // Find child service group.
  servicegroup* child_grp(find_servicegroup(child_id->name.c_str()));
  if (!child_grp)
    throw (engine_error() << "cannot link non-existent service group '"
           << child_id->name << "' to service group '"
           << parent_id->name << "'");

  // Member array.
  QVector<servicegroup*> member;
  member.push_back(child_grp);

  // Link service groups.
  objects::link(parent_grp, QVector<service*>(), member);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Modify an existing service group.
 *
 *  @param[in]  s      SOAP object.
 *  @param[in]  svcgrp New service group properties.
 *  @param[out] res    Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__servicegroupModify(
      soap* s,
      ns1__servicegroupType* svcgrp,
      centreonengine__servicegroupModifyResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(svcgrp->id->name)

  // Find target service group.
  servicegroup* grp(find_servicegroup(svcgrp->id->name.c_str()));
  if (!grp)
    throw (engine_error()
           << "cannot modify non-existent service group '"
           << svcgrp->id->name << "'");

  // Update alias.
  if (svcgrp->alias) {
    delete [] grp->alias;
    grp->alias = my_strdup(svcgrp->alias->c_str());
  }

  // Update notes.
  if (svcgrp->notes) {
    delete [] grp->notes;
    grp->notes = my_strdup(svcgrp->notes->c_str());
  }

  // Update notes_url.
  if (svcgrp->notesUrl) {
    delete [] grp->notes_url;
    grp->notes_url = my_strdup(svcgrp->notesUrl->c_str());
  }

  // Update action_url.
  if (svcgrp->actionUrl) {
    delete [] grp->action_url;
    grp->action_url = my_strdup(svcgrp->actionUrl->c_str());
  }

  // Update service members.
  // XXX

  // Update service group members.
  // XXX

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Remove a service group.
 *
 *  @param[in]  s               SOAP object.
 *  @param[in]  servicegroup_id Service group to remove.
 *  @param[out] res             Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__servicegroupRemove(
      soap* s,
      ns1__servicegroupIDType* servicegroup_id,
      centreonengine__servicegroupRemoveResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(servicegroup_id->name)

  // Remove service group.
  if (!remove_servicegroup_by_id(servicegroup_id->name.c_str()))
    throw (engine_error() << "cannot remove service group '"
           << servicegroup_id->name
           << "' (still in use ?, inexistent ?)");

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Remove a service from a service group.
 *
 *  @param[in]  s               SOAP object.
 *  @param[in]  servicegroup_id Target service group.
 *  @param[in]  service_id      Target service.
 *  @param[out] res             Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__servicegroupRemoveService(
      soap* s,
      ns1__servicegroupIDType* servicegroup_id,
      ns1__serviceIDType* service_id,
      centreonengine__servicegroupRemoveServiceResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(servicegroup_id->name << ", " << service_id->service)

  // Find target service group.
  servicegroup* grp(find_servicegroup(servicegroup_id->name.c_str()));
  if (!grp)
    throw (engine_error() << "cannot remove service ('"
           << service_id->host->name << "', '" << service_id->service
           << "') from non-existent service group '"
           << servicegroup_id->name << "'");

  // Find service member.
  servicesmember* current;
  servicesmember** prev;
  for (current = grp->members, prev = &grp->members;
       current;
       prev = &current->next, current = current->next)
    if (!strcmp(
           current->service_ptr->description,
           service_id->service.c_str())
        && !strcmp(
              current->service_ptr->host_ptr->name,
              service_id->host->name.c_str()))
      break ;
  if (current) {
    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group_member(
      NEBTYPE_SERVICEGROUPMEMBER_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      grp,
      current->service_ptr,
      &tv);

    // Backup service.
    service* svc(current->service_ptr);

    // Remove link from group to service.
    *prev = current->next;
    delete [] current->host_name;
    delete [] current->service_description;
    delete current;

    // Remove link from service to group.
    remove_object_to_objectlist(&svc->servicegroups_ptr, grp);
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

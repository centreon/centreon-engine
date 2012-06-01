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
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/objects/hostgroup.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules;
using namespace com::centreon::engine::modules::webservice;

/**
 *  Add a new hostgroup into the engine.
 *
 *  @param[in] hstgrp The struct with all information to create new
 *                    hostgroup.
 */
void webservice::create_hostgroup(ns1__hostgroupType const& hstgrp) {
  // Create new hostgroup.
  hostgroup* grp(add_hostgroup(
                   hstgrp.id->name.c_str(),
                   (hstgrp.alias ? hstgrp.alias->c_str() : NULL),
                   (hstgrp.notes ? hstgrp.notes->c_str() : NULL),
                   (hstgrp.notesUrl ? hstgrp.notesUrl->c_str() : NULL),
                   (hstgrp.actionUrl
                    ? hstgrp.actionUrl->c_str()
                    : NULL)));

  try {
    // Add all hosts into the hostgroup.
    QVector<host*> hst_members(_find<host>(
                                 hstgrp.hostMembers,
                                 (void* (*)(char const*))&find_host));
    if (hstgrp.hostMembers.empty()
        || (static_cast<int>(hstgrp.hostMembers.size())
            != hst_members.size()))
      throw (engine_error() << "hostgroup '"
             << hstgrp.id->name << "' invalid member");

    // Add the content of other hostgroups into this hostgroup.
    QVector<hostgroup*>
      hst_groups(_find<hostgroup>(
                   hstgrp.hostgroupMembers,
                   (void* (*)(char const*))&find_hostgroup));
    if (static_cast<int>(hstgrp.hostgroupMembers.size())
        != hst_groups.size())
      throw (engine_error() << "hostgroup '" << hstgrp.id->name
             << "' invalid group member");

    // Link objects together.
    objects::link(grp, hst_members, hst_groups);
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
 *  Add new hostgroup.
 *
 *  @param[in]  s         SOAP object.
 *  @param[in]  hostGroup Hostgroup to add.
 *  @param[out] res       Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostgroupAdd(
      soap* s,
      ns1__hostgroupType* hostGroup,
      centreonengine__hostgroupAddResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(hostGroup->id->name)

  // Create host group.
  create_hostgroup(*hostGroup);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Add a host to a host group.
 *
 *  @param[in]  s            SOAP object.
 *  @param[in]  hostgroup_id Target host group.
 *  @param[in]  host_id      Target host.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostgroupAddHost(
      soap* s,
      ns1__hostgroupIDType* hostgroup_id,
      ns1__hostIDType* host_id,
      centreonengine__hostgroupAddHostResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(hostgroup_id->name << ", " << host_id->name)

  // Find target host group.
  hostgroup* hstgrp(find_hostgroup(hostgroup_id->name.c_str()));
  if (!hstgrp)
    throw (engine_error() << "cannot link host '" << host_id->name
           << "' to non-existent host group '" << hostgroup_id->name
           << "'");

  // Find target host.
  host* hst(find_host(host_id->name.c_str()));
  if (!hst)
    throw (engine_error() << "cannot link non-existent host '"
           << host_id->name << "' to host group '"
           << hostgroup_id->name << "'");

  // Member array.
  QVector<host*> member;
  member.push_back(hst);

  // Link host and host group.
  objects::link(hstgrp, member, QVector<hostgroup*>());

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Add a hostgroup to another host group.
 *
 *  @param[in]  s         SOAP object.
 *  @param[in]  parent_id Parent host group.
 *  @param[in]  child_id  Child host group.
 *  @param[out] res       Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostgroupAddHostgroup(
      soap* s,
      ns1__hostgroupIDType* parent_id,
      ns1__hostgroupIDType* child_id,
      centreonengine__hostgroupAddHostgroupResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(parent_id->name << ", " << child_id->name)

  // Find parent host group.
  hostgroup* parent_grp(find_hostgroup(parent_id->name.c_str()));
  if (!parent_grp)
    throw (engine_error() << "cannot link host group '"
           << child_id->name << "' to non-existent host group '"
           << parent_id->name << "'");

  // Find child host group.
  hostgroup* child_grp(find_hostgroup(child_id->name.c_str()));
  if (!child_grp)
    throw (engine_error() << "cannot link non-existent host group '"
           << child_id->name << "' to host group '" << parent_id->name
           << "'");

  // Member array.
  QVector<hostgroup*> member;
  member.push_back(child_grp);

  // Link host groups.
  objects::link(parent_grp, QVector<host*>(), member);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Modify an existing host group.
 *
 *  @param[in]  s      SOAP object.
 *  @param[in]  hstgrp Host group.
 *  @param[out] res    Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostgroupModify(
      soap* s,
      ns1__hostgroupType* hstgrp,
      centreonengine__hostgroupModifyResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(hstgrp->id->name)

  // Find target host group.
  hostgroup* grp(find_hostgroup(hstgrp->id->name.c_str()));
  if (!grp)
    throw (engine_error() << "cannot modify non-existent host group '"
           << hstgrp->id->name << "'");

  // Update alias.
  if (hstgrp->alias) {
    delete [] grp->alias;
    grp->alias = my_strdup(hstgrp->alias->c_str());
  }

  // Update notes.
  if (hstgrp->notes) {
    delete [] grp->notes;
    grp->notes = my_strdup(hstgrp->notes->c_str());
  }

  // Update notes_url.
  if (hstgrp->notesUrl) {
    delete [] grp->notes_url;
    grp->notes_url = my_strdup(hstgrp->notesUrl->c_str());
  }

  // Update action_url.
  if (hstgrp->actionUrl) {
    delete [] grp->action_url;
    grp->action_url = my_strdup(hstgrp->actionUrl->c_str());
  }

  // Update host members.
  // XXX

  // Update host group members.
  // XXX

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Remove a host group.
 *
 *  @param[in]  s            SOAP object.
 *  @param[in]  hostgroup_id Host group to remove.
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostgroupRemove(
      soap* s,
      ns1__hostgroupIDType* hostgroup_id,
      centreonengine__hostgroupRemoveResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(hostgroup_id->name)

  // Remove host group.
  if (!remove_hostgroup_by_id(hostgroup_id->name.c_str()))
    throw (engine_error() << "cannot remove host group '"
           << hostgroup_id->name << "' (still in use ?, inexistent ?)");

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Remove a host from a host group.
 *
 *  @param[in]  s
 *  @param[in]  hostgroup_id
 *  @param[in]  host_id
 *  @param[out] res          Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostgroupRemoveHost(
      soap* s,
      ns1__hostgroupIDType* hostgroup_id,
      ns1__hostIDType* host_id,
      centreonengine__hostgroupRemoveHostResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(hostgroup_id->name << ", " << host_id->name)

  // Find target host group.
  hostgroup* hstgrp(find_hostgroup(hostgroup_id->name.c_str()));
  if (!hstgrp)
    throw (engine_error() << "cannot remove host '" << host_id->name
           << "' from non-existent host group '" << hostgroup_id->name
           << "'");

  // Find host member.
  hostsmember* current;
  hostsmember** prev;
  for (current = hstgrp->members, prev = &hstgrp->members;
       current;
       prev = &current->next, current = current->next)
    if (!strcmp(current->host_ptr->name, host_id->name.c_str()))
      break ;
  if (current) {
    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group_member(
      NEBTYPE_HOSTGROUPMEMBER_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      hstgrp,
      current->host_ptr,
      &tv);

    // Backup host.
    host* hst(current->host_ptr);

    // Remove link from group to host.
    *prev = current->next;
    delete [] current->host_name;
    delete current;

    // Remove link from host to group.
    remove_object_to_objectlist(&hst->hostgroups_ptr, hstgrp);
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

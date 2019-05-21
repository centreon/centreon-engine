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
#include "com/centreon/engine/deleter/servicesmember.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/objects/service.hh"
#include "com/centreon/engine/objects/servicegroup.hh"
#include "com/centreon/engine/objects/servicesmember.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       servicesmember const& obj1,
       servicesmember const& obj2) throw () {
  if (is_equal(obj1.host_name, obj2.host_name)
      && is_equal(obj1.service_description, obj2.service_description)) {
    if (!obj1.next || !obj2.next)
      return (!obj1.next && !obj2.next);
    return (*obj1.next == *obj2.next);
  }
  return (false);
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool operator!=(
       servicesmember const& obj1,
       servicesmember const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump servicesmember content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The servicesmember to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, servicesmember const& obj) {
  for (servicesmember const* m(&obj); m; m = m->next)
    os << "(" << chkstr(m->host_name) << ", "
       << chkstr(m->service_description) << (m->next ? "), " : ")");
  return (os);
}

/**
 *  Link a service to its host.
 *
 *  @param[in,out] hst Host.
 *  @param[in]     svc Service.
 *
 *  @return Host-service relation.
 */
servicesmember* add_service_link_to_host(host* hst, service* svc) {
  // Make sure we have the data we need.
  if (!hst || !svc)
    return (NULL);

  // Allocate memory.
  servicesmember* obj(new servicesmember);
  memset(obj, 0, sizeof(*obj));

  try {
    // Initialize values.
    obj->service_ptr = svc;

    // Add the child entry to the host definition.
    obj->next = hst->services;
    hst->services = obj;

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_relation_data(
      NEBTYPE_PARENT_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      hst,
      NULL,
      NULL,
      svc,
      &tv);
  }
  catch (...) {
    deleter::servicesmember(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Add a new service to a service group.
 *
 *  @param[in,out] temp_servicegroup Target service group.
 *  @param[in]     host_name         Host name.
 *  @param[in]     svc_description   Service description.
 *
 *  @return Service group membership.
 */
servicesmember* add_service_to_servicegroup(
                  servicegroup* grp,
                  char const* host_name,
                  char const* svc_description) {
  // Make sure we have the data we need.
  if (!grp
      || !host_name
      || !host_name[0]
      || !svc_description
      || !svc_description[0]) {
    logger(log_config_error, basic)
      << "Error: Servicegroup or group member is NULL";
    return (NULL);
  }

  // Allocate memory for a new member.
  servicesmember* obj(new servicesmember);
  memset(obj, 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->host_name = string::dup(host_name);
    obj->service_description = string::dup(svc_description);

    // Add new member to member list.
    obj->next = grp->members;
    grp->members = obj;

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group_member(
      NEBTYPE_SERVICEGROUPMEMBER_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj,
      grp,
      &tv);
  }
  catch (...) {
    deleter::servicesmember(obj);
    obj = NULL;
  }

  return (obj);
}

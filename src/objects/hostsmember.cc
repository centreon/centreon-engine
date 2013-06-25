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
#include "com/centreon/engine/deleter/hostsmember.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/misc/object.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/objects/hostgroup.hh"
#include "com/centreon/engine/objects/hostsmember.hh"
#include "com/centreon/engine/shared.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::misc;

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       hostsmember const& obj1,
       hostsmember const& obj2) throw () {
  if (is_equal(obj1.host_name, obj2.host_name)) {
    if (!obj1.next || !obj2.next)
      return (!obj1.next && !obj2.next);
    else
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
       hostsmember const& obj1,
       hostsmember const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump hostsmember content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The hostsmember to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, hostsmember const& obj) {
  for (hostsmember const* m(&obj); m; m = m->next)
    os << chkstr(m->host_name) << (m->next ? ", " : "");
  return (os);
}

/**
 *  Adds a child host to a host definition.
 *
 *  @param[in] parent Parent host.
 *  @param[in] child  Child host.
 *
 *  @return Host member.
 */
hostsmember* add_child_link_to_host(host* parent, host* child) {
  // Make sure we have the data we need.
  if (!parent || !child)
    return (NULL);

  // Allocate memory.
  hostsmember* obj(new hostsmember);
  memset(obj, 0, sizeof(*obj));

  try {
    // Initialize values.
    obj->host_ptr = child;

    // Add the child entry to the host definition.
    obj->next = parent->child_hosts;
    parent->child_hosts = obj;

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_relation_data(
      NEBTYPE_PARENT_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      parent,
      NULL,
      child,
      NULL,
      NULL,
      NULL,
      0,
      NULL,
      &tv);
  }
  catch (...) {
    deleter::hostsmember(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Add a new host to a host group.
 *
 *  @param[in] temp_hostgroup Host group object.
 *  @param[in] host_name      Host name.
 *
 *  @return Host group membership.
 */
hostsmember* add_host_to_hostgroup(
               hostgroup* temp_hostgroup,
               char const* host_name) {
  // Make sure we have the data we need.
  if (!temp_hostgroup || !host_name || !host_name[0]) {
    logger(log_config_error, basic)
      << "Error: Hostgroup or group member is NULL";
    return (NULL);
  }

  // Allocate memory for a new member.
  hostsmember* obj(new hostsmember);
  memset(obj, 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->host_name = my_strdup(host_name);

    // Add the new member to the member list, sorted by host name.
    hostsmember* last(temp_hostgroup->members);
    hostsmember* temp;
    for (temp = temp_hostgroup->members; temp; temp = temp->next) {
      if (strcmp(obj->host_name, temp->host_name) < 0) {
        obj->next = temp;
        if (temp == temp_hostgroup->members)
          temp_hostgroup->members = obj;
        else
          last->next = obj;
        break;
      }
      else
        last = temp;
    }
    if (!temp_hostgroup->members)
      temp_hostgroup->members = obj;
    else if (!temp)
      last->next = obj;

    // Notify event broker.
    // XXX
  }
  catch (...) {
    deleter::hostsmember(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Add parent to host.
 *
 *  @param[in] hst       Child host.
 *  @param[in] host_name Parent host name.
 *
 *  @return Parent relationship.
 */
hostsmember* add_parent_host_to_host(
               host* hst,
               char const* host_name) {
  // Make sure we have the data we need.
  if (!hst || !host_name || !host_name[0]) {
    logger(log_config_error, basic)
      << "Error: Host is NULL or parent host name is NULL";
    return (NULL);
  }

  // A host cannot be a parent/child of itself.
  if (!strcmp(host_name, hst->name)) {
    logger(log_config_error, basic)
      << "Error: Host '" << hst->name
      << "' cannot be a child/parent of itself";
    return (NULL);
  }

  // Allocate memory.
  hostsmember* obj(new hostsmember);
  memset(obj, 0, sizeof(*obj));

  try {
    // Duplicate string vars.
    obj->host_name = my_strdup(host_name);

    // Add the parent host entry to the host definition */
    obj->next = hst->parent_hosts;
    hst->parent_hosts = obj;

    // Notify event broker.
    // XXX
  }
  catch (...) {
    deleter::hostsmember(obj);
    obj = NULL;
  }

  return (obj);
}


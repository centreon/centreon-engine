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

#include "com/centreon/engine/deleter/contactgroupsmember.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/contact.hh"
#include "com/centreon/engine/objects/contactgroupsmember.hh"
#include "com/centreon/engine/objects/host.hh"
#include "com/centreon/engine/objects/hostescalation.hh"
#include "com/centreon/engine/objects/service.hh"
#include "com/centreon/engine/objects/serviceescalation.hh"
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
       contactgroupsmember const& obj1,
       contactgroupsmember const& obj2) throw () {
  if (is_equal(obj1.group_name, obj2.group_name)) {
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
       contactgroupsmember const& obj1,
       contactgroupsmember const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Less-than operator.
 *
 *  @param[in] obj1 First object to compare.
 *  @param[in] obj2 Second object to compare.
 *
 *  @return True if the first object is less than the second.
 */
bool operator<(
       contactgroupsmember const& obj1,
       contactgroupsmember const& obj2) throw () {
  if (!!obj1.group_name ^ !!obj2.group_name)
    return (!!obj1.group_name < !!obj2.group_name);
  else if (obj1.group_name
           && obj2.group_name
           && strcmp(obj1.group_name, obj2.group_name))
    return (strcmp(obj1.group_name, obj2.group_name) < 0);
  return (false);
}

/**
 *  Dump contactgroupsmember content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The contactgroupsmember to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, contactgroupsmember const& obj) {
  for (contactgroupsmember const* m(&obj); m; m = m->next)
    os << chkstr(m->group_name) << (m->next ? ", " : "");
  return (os);
}

/**
 *  Add a new contactgroup to a host.
 *
 *  @param[in] hst        Host.
 *  @param[in] group_name Contact group name.
 *
 *  @return Contact group membership object.
 */
contactgroupsmember* add_contactgroup_to_host(
                       host* hst,
                       char const* group_name) {
  // Make sure we have the data we need.
  if (!hst || !group_name || !group_name[0]) {
    logger(log_config_error, basic)
      << "Error: Host or contactgroup member is NULL";
    return (NULL);
  }

  // Allocate memory for a new member.
  contactgroupsmember* obj(new contactgroupsmember);
  memset(obj, 0, sizeof(*obj));

  try {
    // Duplicate string vars.
    obj->group_name = string::dup(group_name);

    // Add the new member to the head of the member list.
    obj->next = hst->contact_groups;
    hst->contact_groups = obj;
  }
  catch (...) {
    deleter::contactgroupsmember(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Adds a contact group to a host escalation.
 *
 *  @param[in] he         Host escalation.
 *  @param[in] group_name Contact group name.
 *
 *  @return Contact group membership object.
 */
contactgroupsmember* add_contactgroup_to_host_escalation(
                       hostescalation* he,
                       char const* group_name) {
  // Bail out if we weren't given the data we need.
  if (!he || !group_name || !group_name[0]) {
    logger(log_config_error, basic)
      << "Error: Host escalation or contactgroup name is NULL";
    return (NULL);
  }

  // Allocate memory for the contactgroups member.
  contactgroupsmember* obj(new contactgroupsmember);
  memset(obj, 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->group_name = string::dup(group_name);

    // Add this contactgroup to the host escalation.
    obj->next = he->contact_groups;
    he->contact_groups = obj;
  }
  catch (...) {
    deleter::contactgroupsmember(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Adds a contact group to a service.
 *
 *  @param[in] svc        Service.
 *  @param[in] group_name Contact group name.
 *
 *  @return Contact group membership object.
 */
contactgroupsmember* add_contactgroup_to_service(
                       service* svc,
                       char const* group_name) {
  // Bail out if we weren't given the data we need.
  if (!svc || !group_name || !group_name[0]) {
    logger(log_config_error, basic)
      << "Error: Service or contactgroup name is NULL";
    return (NULL);
  }

  // Allocate memory for the contactgroups member.
  contactgroupsmember* obj(new contactgroupsmember);
  memset(obj, 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->group_name = string::dup(group_name);

    // Add this contactgroup to the service.
    obj->next = svc->contact_groups;
    svc->contact_groups = obj;
  }
  catch (...) {
    deleter::contactgroupsmember(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Adds a contact group to a service escalation.
 *
 *  @param[in] se         Service escalation.
 *  @param[in] group_name Contact group name.
 *
 *  @return Contact group membership object.
 */
contactgroupsmember* add_contactgroup_to_serviceescalation(
                       serviceescalation* se,
                       char const* group_name) {
  // Bail out if we weren't given the data we need.
  if (!se || !group_name || !group_name[0]) {
    logger(log_config_error, basic)
      << "Error: Service escalation or contactgroup name is NULL";
    return (NULL);
  }

  // Allocate memory for the contactgroups member.
  contactgroupsmember* obj(new contactgroupsmember);
  memset(obj, 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->group_name = string::dup(group_name);

    // Add this contactgroup to the service escalation.
    obj->next = se->contact_groups;
    se->contact_groups = obj;
  }
  catch (...) {
    deleter::contactgroupsmember(obj);
    obj = NULL;
  }

  return (obj);
}

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
#include "com/centreon/engine/deleter/contactsmember.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/contactgroup.hh"
#include "com/centreon/engine/objects/contactsmember.hh"
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
       contactsmember const& obj1,
       contactsmember const& obj2) throw () {
  if (is_equal(obj1.contact_name, obj2.contact_name)) {
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
       contactsmember const& obj1,
       contactsmember const& obj2) throw () {
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
       contactsmember const& obj1,
       contactsmember const& obj2) throw () {
  if (!!obj1.contact_name ^ !!obj2.contact_name)
    return (!!obj1.contact_name < !!obj2.contact_name);
  else if (obj1.contact_name
           && obj2.contact_name
           && strcmp(obj1.contact_name, obj2.contact_name))
    return (strcmp(obj1.contact_name, obj2.contact_name) < 0);
  return (false);
}

/**
 *  Dump contactsmember content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The contactsmember to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, contactsmember const& obj) {
  for (contactsmember const* m(&obj); m; m = m->next)
    os << chkstr(m->contact_name) << (m->next ? ", " : "");
  return (os);
}

/**
 *  Add a new member to a contact group.
 *
 *  @param[in] grp          Contact group.
 *  @param[in] contact_name Contact name.
 *
 *  @return Contact group membership object.
 */
contactsmember* add_contact_to_contactgroup(
                  contactgroup* grp,
                  char const* contact_name) {
  // Make sure we have the data we need.
  if (!grp || !contact_name || !contact_name[0]) {
    logger(log_config_error, basic)
      << "Error: Contactgroup or contact name is NULL";
    return (NULL);
  }

  // Allocate memory for a new member.
  contactsmember* obj(new contactsmember);
  memset(obj, 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->contact_name = string::dup(contact_name);

    // Add the new member to the head of the member list.
    obj->next = grp->members;
    grp->members = obj;

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group_member(
      NEBTYPE_CONTACTGROUPMEMBER_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj,
      grp,
      &tv);
  }
  catch (...) {
    deleter::contactsmember(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Adds a contact to a host.
 *
 *  @param[in] hst          Host.
 *  @param[in] contact_name Contact name.
 *
 *  @return Contact membership object.
 */
contactsmember* add_contact_to_host(host* hst, char const* contact_name) {
  return (add_contact_to_object(&hst->contacts, contact_name));
}

/**
 *  Adds a contact to a host escalation.
 *
 *  @param[in] he           Host escalation.
 *  @param[in] contact_name Contact name.
 *
 *  @return Contact membership object.
 */
contactsmember* add_contact_to_host_escalation(
                  hostescalation* he,
                  char const* contact_name) {
  return (add_contact_to_object(&he->contacts, contact_name));
}

/**
 *  Adds a contact to an object.
 *
 *  @param[in] object_ptr
 *  @param[in] contact_name Contact name.
 *
 *  @return Contact membership object.
 */
contactsmember* add_contact_to_object(
                  contactsmember** object_ptr,
                  char const* contact_name) {
  // Make sure we have the data we need.
  if (!object_ptr) {
    logger(log_config_error, basic)
      << "Error: Contact object is NULL";
    return (NULL);
  }
  if (!contact_name || !contact_name[0]) {
    logger(log_config_error, basic)
      << "Error: Contact name is NULL";
    return (NULL);
  }

  // Allocate memory for a new member.
  contactsmember* obj(new contactsmember);
  memset(obj, 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->contact_name = string::dup(contact_name);

    // Add the new contact to the head of the contact list.
    obj->next = *object_ptr;
    *object_ptr = obj;
  }
  catch (...) {
    deleter::contactsmember(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Adds a contact to a service.
 *
 *  @param[in] svc          Service.
 *  @param[in] contact_name Contact name.
 *
 *  @return Contact membership object.
 */
contactsmember* add_contact_to_service(
                  service* svc,
                  char const* contact_name) {
  return (add_contact_to_object(&svc->contacts, contact_name));
}

/**
 *  Adds a contact to a service escalation.
 *
 *  @param[in] se           Service escalation.
 *  @param[in] contact_name Contact name.
 *
 *  @return Contact membership object.
 */
contactsmember* add_contact_to_serviceescalation(
                  serviceescalation* se,
                  char const* contact_name) {
  return (add_contact_to_object(&se->contacts, contact_name));
}

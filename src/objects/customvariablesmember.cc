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

#include <set>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/deleter/customvariablesmember.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
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
       customvariablesmember const& obj1,
       customvariablesmember const& obj2) throw () {
  if (is_equal(obj1.variable_name, obj2.variable_name)
      && is_equal(obj1.variable_value, obj2.variable_value)
      && obj1.has_been_modified == obj2.has_been_modified
      && obj1.is_sent == obj2.is_sent) {
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
       customvariablesmember const& obj1,
       customvariablesmember const& obj2) throw () {
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
       customvariablesmember const& obj1,
       customvariablesmember const& obj2) throw () {
  if (!obj1.variable_name || !obj2.variable_name)
    return (obj1.variable_name < obj2.variable_name);
  int ret(strcmp(obj1.variable_name, obj2.variable_name));
  if (ret)
    return (ret < 0);
  if (!obj1.variable_value || !obj2.variable_value)
    return (obj1.variable_value < obj2.variable_value);
  return (strcmp(obj1.variable_value, obj2.variable_value) < 0);
}

/**
 *  Dump customvariablesmember content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The customvariablesmember to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, customvariablesmember const& obj) {
  for (customvariablesmember const* m(&obj); m; m = m->next)
    os << "  _" << chkstr(m->variable_name) << ": " << chkstr(m->variable_value) << "\n";
  return (os);
}

/**
 *  Adds a custom variable to a contact.
 *
 *  @param[in] cntct    Contact object.
 *  @param[in] varname  Custom variable name.
 *  @param[in] varvalue Custom variable value.
 *
 *  @return Contact custom variable.
 */
customvariablesmember* add_custom_variable_to_contact(
                         contact* cntct,
                         char const* varname,
                         customvariable const& cv) {
  // Add custom variable to contact.
  customvariablesmember* retval(add_custom_variable_to_object(
                                  &cntct->custom_variables,
                                  varname,
                                  cv.get_value().c_str(),
                                  cv.is_sent()));

  if (cv.is_sent()) {
    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_custom_variable(
      NEBTYPE_CONTACTCUSTOMVARIABLE_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      cntct,
      varname,
      cv.get_value().c_str(),
      &tv);
  }
  return retval;
}

/**
 *  Adds a custom variable to a host
 *
 *  @param[in] hst      Host.
 *  @param[in] varname  Custom variable name.
 *  @param[in] varvalue Custom variable value.
 *
 *  @return New host custom variable.
 */
customvariablesmember* add_custom_variable_to_host(
                         host* hst,
                         char const* varname,
                         customvariable const& cv) {
  // Add custom variable to host.
  customvariablesmember* retval(add_custom_variable_to_object(
                                  &hst->custom_variables,
                                  varname,
                                  cv.get_value().c_str(),
                                  cv.is_sent()));

  if (cv.is_sent()) {
    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_custom_variable(
      NEBTYPE_HOSTCUSTOMVARIABLE_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      hst,
      varname,
      cv.get_value().c_str(),
      &tv);
  }
  return retval;
}

/**
 *  Adds a custom variable to an object.
 *
 *  @param[in] object_ptr Object's custom variables.
 *  @param[in] varname    Custom variable name.
 *  @param[in] varvalue   Custom variable value.
 *
 *  @return New custom variable.
 */
customvariablesmember* add_custom_variable_to_object(
                         customvariablesmember** object_ptr,
                         char const* varname,
                         char const* varvalue,
                         bool is_sent) {
  // Make sure we have the data we need.
  if (!object_ptr) {
    logger(log_config_error, basic)
      << "Error: Custom variable object is NULL";
    return (NULL);
  }
  if (!varname || !varname[0]) {
    logger(log_config_error, basic)
      << "Error: Custom variable name is NULL";
    return (NULL);
  }

  // Allocate memory for a new member.
  customvariablesmember* obj(new customvariablesmember);
  memset(obj, 0, sizeof(*obj));

  try {
    obj->variable_name = string::dup(varname);
    if (varvalue)
      obj->variable_value = string::dup(varvalue);

    obj->is_sent = is_sent;
    // Add the new member to the head of the member list.
    obj->next = *object_ptr;
    *object_ptr = obj;
  }
  catch (...) {
    deleter::customvariablesmember(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Adds a custom variable to a service.
 *
 *  @param[in] svc      Service.
 *  @param[in] varname  Custom variable name.
 *  @param[in] varvalue Custom variable value.
 *
 *  @return New custom variable.
 */
customvariablesmember* add_custom_variable_to_service(
                         service* svc,
                         char const* varname,
                         customvariable const& cv) {
  // Add custom variable to service.
  customvariablesmember* retval(add_custom_variable_to_object(
                                  &svc->custom_variables,
                                  varname,
                                  cv.get_value().c_str(),
                                  cv.is_sent()));

  if (cv.is_sent()) {
    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_custom_variable(
      NEBTYPE_SERVICECUSTOMVARIABLE_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      svc,
      varname,
      cv.get_value().c_str(),
      &tv);
  }
  return retval;
}

/**
 *  Remove all custom variables of a contact.
 *
 *  @param[in,out] cntct  Target contact.
 */
void remove_all_custom_variables_from_contact(com::centreon::engine::contact* cntct) {
  // Browse all custom vars.
  customvariablesmember* m(cntct->custom_variables);
  cntct->custom_variables = NULL;
  while (m) {
    // Point to next custom var.
    customvariablesmember* to_delete(m);
    m = m->next;

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_custom_variable(
      NEBTYPE_CONTACTCUSTOMVARIABLE_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      cntct,
      to_delete->variable_name,
      to_delete->variable_value,
      &tv);

    // Delete custom variable.
    deleter::customvariablesmember(to_delete);
  }
  return ;
}

/**
 *  Remove all custom variables of a host.
 *
 *  @param[in,out] hst  Target host.
 */
void remove_all_custom_variables_from_host(host_struct* hst) {
  // Browse all custom vars.
  customvariablesmember* m(hst->custom_variables);
  hst->custom_variables = NULL;
  while (m) {
    // Point to next custom var.
    customvariablesmember* to_delete(m);
    m = m->next;

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_custom_variable(
      NEBTYPE_HOSTCUSTOMVARIABLE_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      hst,
      to_delete->variable_name,
      to_delete->variable_value,
      &tv);

    // Delete custom variable.
    deleter::customvariablesmember(to_delete);
  }
  return ;
}

/**
 *  Remove all custom variables of a service.
 *
 *  @param[in,out] svc  Target service.
 */
void remove_all_custom_variables_from_service(service_struct* svc) {
  // Browse all custom vars.
  customvariablesmember* m(svc->custom_variables);
  svc->custom_variables = NULL;
  while (m) {
    // Point to next custom var.
    customvariablesmember* to_delete(m);
    m = m->next;

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_custom_variable(
      NEBTYPE_SERVICECUSTOMVARIABLE_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      svc,
      to_delete->variable_name,
      to_delete->variable_value,
      &tv);

    // Delete custom variable.
    deleter::customvariablesmember(to_delete);
  }
  return ;
}

/**
 *  Update the custom variable value.
 *
 *  @param[in,out] lst   The custom variables list.
 *  @param[in]     key   The key to find.
 *  @param[in]     value The new value to set.
 *
 *  @return True if the custom variable change.
 */
bool engine::update_customvariable(
       customvariablesmember* lst,
       std::string const& key,
       std::string const& value) {
  char const* cv_name(key.c_str());
  char const* cv_value(value.c_str());
  for (customvariablesmember* m(lst); m; m = m->next) {
    if (!strcmp(cv_name, m->variable_name)) {
      if (strcmp(cv_value, m->variable_value)) {
        string::setstr(m->variable_value, value);
        m->has_been_modified = true;
      }
      return (true);
    }
  }
  return (false);
}

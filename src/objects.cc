/*
** Copyright 1999-2008 Ethan Galstad
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

#include <cstdio>
#include <cstring>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/xodtemplate.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;

extern "C" {

/**************************************
*                                     *
*      Object Creation Functions      *
*                                     *
**************************************/

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
 *  Add a new command to the list in memory.
 *
 *  @param[in] name  Command name.
 *  @param[in] value Command itself.
 *
 *  @return New command object.
 */
command* add_command(char const* name, char const* value) {
  // Make sure we have the data we need.
  if (!name || !name[0] || !value || !value[0]) {
    logger(log_config_error, basic)
      << "Error: Command name or command line is NULL";
    return (NULL);
  }

  // Allocate memory for the new command.
  shared_ptr<command> obj(new command, deleter::command);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->name = my_strdup(name);
    obj->command_line = my_strdup(value);

    // Add new command to the monitoring engine.
    umap<std::string, shared_ptr<command_struct> >::const_iterator
      it(state::instance().commands().find(name));
    if (it != state::instance().commands().end()) {
      logger(log_config_error, basic)
        << "Error: Command '" << name << "' has already been defined";
      return (NULL);
    }

    // Add new items to the list.
    obj->next = command_list;
    command_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_command_data(
      NEBTYPE_COMMAND_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      name,
      value,
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Add a new contact to the list in memory.
 *
 *  @param[in] name                          Contact name.
 *  @param[in] alias                         Contact alias.
 *  @param[in] email                         Email.
 *  @param[in] pager                         Pager.
 *  @param[in] addresses                     Contact addresses.
 *  @param[in] svc_notification_period       Service notification
 *                                           period.
 *  @param[in] host_notification_period      Host nofication period.
 *  @param[in] notify_service_ok             Contact can be notified
 *                                           when service is ok.
 *  @param[in] notify_service_critical       Contact can be notified
 *                                           when service is critical.
 *  @param[in] notify_service_warning        Contact can be notified
 *                                           when service is warning.
 *  @param[in] notify_service_unknown        Contact can be notified
 *                                           when service is unknown.
 *  @param[in] notify_service_flapping       Contact can be notified
 *                                           when service is flapping.
 *  @param[in] notify_sevice_downtime        Contact can be notified on
 *                                           service downtime.
 *  @param[in] notify_host_up                Contact can be notified
 *                                           when host is up.
 *  @param[in] notify_host_down              Contact can be notified
 *                                           when host is down.
 *  @param[in] notify_host_unreachable       Contact can be notified
 *                                           when host is unreachable.
 *  @param[in] notify_host_flapping          Contact can be notified
 *                                           when host is flapping.
 *  @param[in] notify_host_downtime          Contact can be notified on
 *                                           host downtime.
 *  @param[in] host_notifications_enabled    Are contact host
 *                                           notifications enabled ?
 *  @param[in] service_notifications_enabled Are contact service
 *                                           notifications enabled ?
 *  @param[in] can_submit_commands           Can user submit external
 *                                           commands ?
 *  @param[in] retain_status_information     Shall Engine retain contact
 *                                           status info ?
 *  @param[in] retain_nonstatus_information  Shell Engine retain contact
 *                                           non-status info ?
 *
 *  @return New contact object.
 */
contact* add_contact(
           char const* name,
           char const* alias,
           char const* email,
           char const* pager,
           char const* const* addresses,
           char const* svc_notification_period,
           char const* host_notification_period,
           int notify_service_ok,
           int notify_service_critical,
           int notify_service_warning,
           int notify_service_unknown,
           int notify_service_flapping,
           int notify_service_downtime,
           int notify_host_up,
           int notify_host_down,
           int notify_host_unreachable,
           int notify_host_flapping,
           int notify_host_downtime,
           int host_notifications_enabled,
           int service_notifications_enabled,
           int can_submit_commands,
           int retain_status_information,
           int retain_nonstatus_information) {
  // Make sure we have the data we need.
  if (!name || !name[0]) {
    logger(log_config_error, basic)
      << "Error: Contact name is NULL";
    return (NULL);
  }

  // Allocate memory for a new contact.
  shared_ptr<contact> obj(new contact, deleter::contact);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->name = my_strdup(name);
    obj->alias = my_strdup(!alias ? name : alias);
    if (email)
      obj->email = my_strdup(email);
    if (host_notification_period)
      obj->host_notification_period = my_strdup(host_notification_period);
    if (pager)
      obj->pager = my_strdup(pager);
    if (svc_notification_period)
      obj->service_notification_period = my_strdup(svc_notification_period);
    if (addresses) {
      for (unsigned int x(0); x < MAX_CONTACT_ADDRESSES; ++x)
        if (addresses[x])
          obj->address[x] = my_strdup(addresses[x]);
    }

    // Set remaining contact properties.
    obj->can_submit_commands = (can_submit_commands > 0);
    obj->host_notifications_enabled = (host_notifications_enabled > 0);
    obj->modified_attributes = MODATTR_NONE;
    obj->modified_host_attributes = MODATTR_NONE;
    obj->modified_service_attributes = MODATTR_NONE;
    obj->notify_on_host_down = (notify_host_down > 0);
    obj->notify_on_host_downtime = (notify_host_downtime > 0);
    obj->notify_on_host_flapping = (notify_host_flapping > 0);
    obj->notify_on_host_recovery = (notify_host_up > 0);
    obj->notify_on_host_unreachable = (notify_host_unreachable > 0);
    obj->notify_on_service_critical = (notify_service_critical > 0);
    obj->notify_on_service_downtime = (notify_service_downtime > 0);
    obj->notify_on_service_flapping = (notify_service_flapping > 0);
    obj->notify_on_service_recovery = (notify_service_ok > 0);
    obj->notify_on_service_unknown = (notify_service_unknown > 0);
    obj->notify_on_service_warning = (notify_service_warning > 0);
    obj->retain_nonstatus_information = (retain_nonstatus_information > 0);
    obj->retain_status_information = (retain_status_information > 0);
    obj->service_notifications_enabled = (service_notifications_enabled > 0);

    // Add new contact to the monitoring engine.
    umap<std::string, shared_ptr<contact_struct> >::const_iterator
      it(state::instance().contacts().find(name));
    if (it != state::instance().contacts().end()) {
      logger(log_config_error, basic)
        << "Error: Contact '" << name << "' has already been defined";
      return (NULL);
    }

    // Add new items to the list.
    obj->next = contact_list;
    contact_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_contact_data(
      NEBTYPE_CONTACT_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
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
    obj->contact_name = my_strdup(contact_name);

    // Add the new member to the head of the member list.
    obj->next = grp->members;
    grp->members = obj;

    // Notify event broker.
    // XXX
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
  // XXX: event broker
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
  // XXX: event broker
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
    obj->contact_name = my_strdup(contact_name);

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
  // XXX: event broker
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
  // XXX: event broker
  return (add_contact_to_object(&se->contacts, contact_name));
}

/**
 *  Add a new contact group to the list in memory.
 *
 *  @param[in] name  Contact group name.
 *  @param[in] alias Contact group alias.
 */
contactgroup* add_contactgroup(char const* name, char const* alias) {
  // Make sure we have the data we need.
  if (!name || !name[0]) {
    logger(log_config_error, basic)
      << "Error: Contactgroup name is NULL";
    return (NULL);
  }

  // Allocate memory for a new contactgroup entry.
  shared_ptr<contactgroup> obj(new contactgroup, deleter::contactgroup);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->group_name = my_strdup(name);
    obj->alias = my_strdup(!alias ? name : alias);

    // Add new contact group to the monitoring engine.
    umap<std::string, shared_ptr<contactgroup_struct> >::const_iterator
      it(state::instance().contactgroups().find(name));
    if (it != state::instance().contactgroups().end()) {
      logger(log_config_error, basic)
        << "Error: Contactgroup '" << name << "' has already been defined";
      return (NULL);
    }

    // Add new items to the list.
    obj->next = contactgroup_list;
    contactgroup_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group(
      NEBTYPE_CONTACTGROUP_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
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
    obj->group_name = my_strdup(group_name);

    // Add the new member to the head of the member list.
    obj->next = hst->contact_groups;
    hst->contact_groups = obj;

    // Notify event broker.
    // XXX
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
    obj->group_name = my_strdup(group_name);

    // Add this contactgroup to the host escalation.
    obj->next = he->contact_groups;
    he->contact_groups = obj;

    // Notify event broker.
    // XXX
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
    obj->group_name = my_strdup(group_name);

    // Add this contactgroup to the service.
    obj->next = svc->contact_groups;
    svc->contact_groups = obj;

    // Notify event broker.
    // XXX
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
    obj->group_name = my_strdup(group_name);

    // Add this contactgroup to the service escalation.
    obj->next = se->contact_groups;
    se->contact_groups = obj;

    // Notify event broker.
    // XXX
  }
  catch (...) {
    deleter::contactgroupsmember(obj);
    obj = NULL;
  }

  return (obj);
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
                         char const* varvalue) {
  // Add custom variable to contact.
  customvariablesmember* retval(add_custom_variable_to_object(
                                  &cntct->custom_variables,
                                  varname,
                                  varvalue));

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_custom_variable(
    NEBTYPE_CONTACTCUSTOMVARIABLE_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    cntct,
    varname,
    varvalue,
    &tv);

  return (retval);
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
                         char const* varvalue) {
  // Add custom variable to host.
  customvariablesmember* retval(add_custom_variable_to_object(
                                  &hst->custom_variables,
                                  varname,
                                  varvalue));

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_custom_variable(
    NEBTYPE_HOSTCUSTOMVARIABLE_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    varname,
    varvalue,
    &tv);

  return (retval);
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
                         char const* varvalue) {
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
    obj->variable_name = my_strdup(varname);
    if (varvalue)
      obj->variable_value = my_strdup(varvalue);

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
                         char const* varvalue) {
  // Add custom variable to service.
  customvariablesmember* retval(add_custom_variable_to_object(
                                  &svc->custom_variables,
                                  varname,
                                  varvalue));

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_custom_variable(
    NEBTYPE_SERVICECUSTOMVARIABLE_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    svc,
    varname,
    varvalue,
    &tv);

  return (retval);
}

/**
 *  Add a new exception to a timeperiod.
 *
 *  @param[in] period        Base period.
 *  @param[in] type
 *  @param[in] syear
 *  @param[in] smon
 *  @param[in] smday
 *  @param[in] swday
 *  @param[in] swday_offset
 *  @param[in] eyear
 *  @param[in] emon
 *  @param[in] emday
 *  @param[in] ewday
 *  @param[in] ewday_offset
 *  @param[in] skip_interval
 *
 *  @return Excluded date range.
 */
daterange* add_exception_to_timeperiod(
             timeperiod* period,
             int type,
             int syear,
             int smon,
             int smday,
             int swday,
             int swday_offset,
             int eyear,
             int emon,
             int emday,
             int ewday,
             int ewday_offset,
             int skip_interval) {
  // Make sure we have the data we need.
  if (!period)
    return (NULL);

  // Allocate memory for the date range range.
  daterange* obj(new daterange);
  memset(obj, 0, sizeof(*obj));

  try {
    // Set daterange properties.
    obj->type = type;
    obj->syear = syear;
    obj->smon = smon;
    obj->smday = smday;
    obj->swday = swday;
    obj->swday_offset = swday_offset;
    obj->eyear = eyear;
    obj->emon = emon;
    obj->emday = emday;
    obj->ewday = ewday;
    obj->ewday_offset = ewday_offset;
    obj->skip_interval = skip_interval;

    // Add the new date range to the head of the range
    // list for this exception type.
    obj->next = period->exceptions[type];
    period->exceptions[type] = obj;

    // Notify event broker.
    // XXX
  }
  catch (...) {
    deleter::daterange(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Adds a new exclusion to a timeperiod.
 *
 *  @param[in] period Base timeperiod.
 *  @param[in] name   Exclusion timeperiod name.
 *
 *  @return Timeperiod exclusion object.
 */
timeperiodexclusion* add_exclusion_to_timeperiod(
                       timeperiod* period,
                       char const* name) {
  // Make sure we have enough data.
  if (!period || !name)
    return (NULL);

  // Allocate memory.
  timeperiodexclusion* obj(new timeperiodexclusion);
  memset(obj, 0, sizeof(*obj));

  try {
    // Set exclusion properties.
    obj->timeperiod_name = my_strdup(name);
    obj->next = period->exclusions;
    period->exclusions = obj;

    // Notify event broker.
    // XXX
  }
  catch (...) {
    deleter::timeperiodexclusion(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Add a new host definition.
 *
 *  @param[in] name                          Host name.
 *  @param[in] display_name                  Display name.
 *  @param[in] alias                         Host alias.
 *  @param[in] address                       Host address.
 *  @param[in] check_period                  Check period.
 *  @param[in] initial_state                 Initial host state.
 *  @param[in] check_interval                Normal check interval.
 *  @param[in] retry_interval                Retry check interval.
 *  @param[in] max_attempts                  Max check attempts.
 *  @param[in] notify_up                     Does this host notify when
 *                                           up ?
 *  @param[in] notify_down                   Does this host notify when
 *                                           down ?
 *  @param[in] notify_unreachable            Does this host notify when
 *                                           unreachable ?
 *  @param[in] notify_flapping               Does this host notify for
 *                                           flapping ?
 *  @param[in] notify_downtime               Does this host notify for
 *                                           downtimes ?
 *  @param[in] notification_interval         Notification interval.
 *  @param[in] first_notification_delay      First notification delay.
 *  @param[in] notification_period           Notification period.
 *  @param[in] notifications_enabled         Whether notifications are
 *                                           enabled for this host.
 *  @param[in] check_command                 Active check command name.
 *  @param[in] checks_enabled                Are active checks enabled ?
 *  @param[in] accept_passive_checks         Can we submit passive check
 *                                           results ?
 *  @param[in] event_handler                 Event handler command name.
 *  @param[in] event_handler_enabled         Whether event handler is
 *                                           enabled or not.
 *  @param[in] flap_detection_enabled        Whether flap detection is
 *                                           enabled or not.
 *  @param[in] low_flap_threshold            Low flap threshold.
 *  @param[in] high_flap_threshold           High flap threshold.
 *  @param[in] flap_detection_on_up          Is flap detection enabled
 *                                           for up state ?
 *  @param[in] flap_detection_on_down        Is flap detection enabled
 *                                           for down state ?
 *  @param[in] flap_detection_on_unreachable Is flap detection enabled
 *                                           for unreachable state ?
 *  @param[in] stalk_on_up                   Stalk on up ?
 *  @param[in] stalk_on_down                 Stalk on down ?
 *  @param[in] stalk_on_unreachable          Stalk on unreachable ?
 *  @param[in] process_perfdata              Should host perfdata be
 *                                           processed ?
 *  @param[in] failure_prediction_enabled    Whether or not failure
 *                                           prediction is enabled.
 *  @param[in] check_freshness               Whether or not freshness
 *                                           check is enabled.
 *  @param[in] freshness_threshold           Freshness threshold.
 *  @param[in] notes                         Notes.
 *  @param[in] notes_url                     URL.
 *  @param[in] action_url                    Action URL.
 *  @param[in] icon_image                    Icon image.
 *  @param[in] icon_image_alt                Alternative icon image.
 *  @param[in] vrml_image                    VRML image.
 *  @param[in] statusmap_image               Status-map image.
 *  @param[in] x_2d                          2D x-coord.
 *  @param[in] y_2d                          2D y-coord.
 *  @param[in] have_2d_coords                Whether host has 2D coords.
 *  @param[in] x_3d                          3D x-coord.
 *  @param[in] y_3d                          3D y-coord.
 *  @param[in] z_3d                          3D z-coord.
 *  @param[in] have_3d_coords                Whether host has 3D coords.
 *  @param[in] should_be_drawn               Whether this host should be
 *                                           drawn.
 *  @param[in] retain_status_information     Should Engine retain status
 *                                           information of this host ?
 *  @param[in] retain_nonstatus_information  Should Engine retain
 *                                           non-status information of
 *                                           this host ?
 *  @param[in] obsess_over_host              Should we obsess over this
 *                                           host ?
 *
 *  @return New host.
 */
host* add_host(
        char const* name,
        char const* display_name,
        char const* alias,
        char const* address,
        char const* check_period,
        int initial_state,
        double check_interval,
        double retry_interval,
        int max_attempts,
        int notify_up,
        int notify_down,
        int notify_unreachable,
        int notify_flapping,
        int notify_downtime,
        double notification_interval,
        double first_notification_delay,
        char const* notification_period,
        int notifications_enabled,
        char const* check_command,
        int checks_enabled,
        int accept_passive_checks,
        char const* event_handler,
        int event_handler_enabled,
        int flap_detection_enabled,
        double low_flap_threshold,
        double high_flap_threshold,
        int flap_detection_on_up,
        int flap_detection_on_down,
        int flap_detection_on_unreachable,
        int stalk_on_up,
        int stalk_on_down,
        int stalk_on_unreachable,
        int process_perfdata,
        int failure_prediction_enabled,
        char const* failure_prediction_options,
        int check_freshness,
        int freshness_threshold,
        char const* notes,
        char const* notes_url,
        char const* action_url,
        char const* icon_image,
        char const* icon_image_alt,
        char const* vrml_image,
        char const* statusmap_image,
        int x_2d,
        int y_2d,
        int have_2d_coords,
        double x_3d,
        double y_3d,
        double z_3d,
        int have_3d_coords,
        int should_be_drawn,
        int retain_status_information,
        int retain_nonstatus_information,
        int obsess_over_host) {
  // Make sure we have the data we need.
  if (!name || !name[0] || !address || !address[0]) {
    logger(log_config_error, basic)
      << "Error: Host name or address is NULL";
    return (NULL);
  }
  if (max_attempts <= 0) {
    logger(log_config_error, basic)
      << "Error: Invalid max_check_attempts value for host '"
      << name << "'";
    return (NULL);
  }
  if (check_interval < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid check_interval value for host '"
      << name << "'";
    return (NULL);
  }
  if (notification_interval < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid notification_interval value for host '"
      << name << "'";
    return (NULL);
  }
  if (first_notification_delay < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid first_notification_delay value for host '"
      << name << "'";
    return (NULL);
  }
  if (freshness_threshold < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid freshness_threshold value for host '"
      << name << "'";
    return (NULL);
  }

  // Allocate memory for a new host.
  shared_ptr<host> obj(new host, deleter::host);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate string vars.
    obj->name = my_strdup(name);
    obj->address = my_strdup(address);
    obj->alias = my_strdup(alias ? alias : name);
    obj->display_name = my_strdup(display_name ? display_name : name);
    if (action_url)
      obj->action_url = my_strdup(action_url);
    if (check_period)
      obj->check_period = my_strdup(check_period);
    if (event_handler)
      obj->event_handler = my_strdup(event_handler);
    if (failure_prediction_options)
      obj->failure_prediction_options = my_strdup(failure_prediction_options);
    if (check_command)
      obj->host_check_command = my_strdup(check_command);
    if (icon_image)
      obj->icon_image = my_strdup(icon_image);
    if (icon_image_alt)
      obj->icon_image_alt = my_strdup(icon_image_alt);
    if (notes)
      obj->notes = my_strdup(notes);
    if (notes_url)
      obj->notes_url = my_strdup(notes_url);
    if (notification_period)
      obj->notification_period = my_strdup(notification_period);
    if (statusmap_image)
      obj->statusmap_image = my_strdup(statusmap_image);
    if (vrml_image)
      obj->vrml_image = my_strdup(vrml_image);

    // Duplicate non-string vars.
    obj->accept_passive_host_checks = (accept_passive_checks > 0);
    obj->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
    obj->check_freshness = (check_freshness > 0);
    obj->check_interval = check_interval;
    obj->check_options = CHECK_OPTION_NONE;
    obj->check_type = HOST_CHECK_ACTIVE;
    obj->checks_enabled = (checks_enabled > 0);
    obj->current_attempt = (initial_state == HOST_UP) ? 1 : max_attempts;
    obj->current_state = initial_state;
    obj->event_handler_enabled = (event_handler_enabled > 0);
    obj->failure_prediction_enabled = (failure_prediction_enabled > 0);
    obj->first_notification_delay = first_notification_delay;
    obj->flap_detection_enabled = (flap_detection_enabled > 0);
    obj->flap_detection_on_down = (flap_detection_on_down > 0);
    obj->flap_detection_on_unreachable = (flap_detection_on_unreachable > 0);
    obj->flap_detection_on_up = (flap_detection_on_up > 0);
    obj->freshness_threshold = freshness_threshold;
    obj->have_2d_coords = (have_2d_coords > 0);
    obj->have_3d_coords = (have_3d_coords > 0);
    obj->high_flap_threshold = high_flap_threshold;
    obj->last_hard_state = initial_state;
    obj->last_state = initial_state;
    obj->low_flap_threshold = low_flap_threshold;
    obj->max_attempts = max_attempts;
    obj->modified_attributes = MODATTR_NONE;
    obj->notification_interval = notification_interval;
    obj->notifications_enabled = (notifications_enabled > 0);
    obj->notify_on_down = (notify_down > 0);
    obj->notify_on_downtime = (notify_downtime > 0);
    obj->notify_on_flapping = (notify_flapping > 0);
    obj->notify_on_recovery = (notify_up > 0);
    obj->notify_on_unreachable = (notify_unreachable > 0);
    obj->obsess_over_host = (obsess_over_host > 0);
    obj->process_performance_data = (process_perfdata > 0);
    obj->retain_nonstatus_information = (retain_nonstatus_information > 0);
    obj->retain_status_information = (retain_status_information > 0);
    obj->retry_interval = retry_interval;
    obj->should_be_drawn = (should_be_drawn > 0);
    obj->should_be_scheduled = true;
    obj->stalk_on_down = (stalk_on_down > 0);
    obj->stalk_on_unreachable = (stalk_on_unreachable > 0);
    obj->stalk_on_up = (stalk_on_up > 0);
    obj->state_type = HARD_STATE;
    obj->x_2d = x_2d;
    obj->x_3d = x_3d;
    obj->y_2d = y_2d;
    obj->y_3d = y_3d;
    obj->z_3d = z_3d;

    for (unsigned int x(0); x < MAX_STATE_HISTORY_ENTRIES; ++x)
      obj->state_history[x] = STATE_OK;

    // Add new host to the monitoring engine.
    umap<std::string, shared_ptr<host_struct> >::const_iterator
      it(state::instance().hosts().find(name));
    if (it != state::instance().hosts().end()) {
      logger(log_config_error, basic)
        << "Error: Host '" << name << "' has already been defined";
      return (NULL);
    }

    // Add new items to the list.
    obj->next = host_list;
    host_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_host_data(
      NEBTYPE_HOST_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Adds a host dependency definition.
 *
 *  @param[in] dependent_host_name Dependant host name.
 *  @param[in] host_name           Host name.
 *  @param[in] dependency_type     Dependency type.
 *  @param[in] inherits_parent     Do we inherits from parent ?
 *  @param[in] fail_on_up          Does dependency fail on up ?
 *  @param[in] fail_on_down        Does dependency fail on down ?
 *  @param[in] fail_on_unreachable Does dependency fail on unreachable ?
 *  @param[in] fail_on_pending     Does dependency fail on pending ?
 *  @param[in] dependency_period   Dependency period.
 *
 *  @return New host dependency.
 */
hostdependency* add_host_dependency(
                  char const* dependent_host_name,
                  char const* host_name,
                  int dependency_type,
                  int inherits_parent,
                  int fail_on_up,
                  int fail_on_down,
                  int fail_on_unreachable,
                  int fail_on_pending,
                  char const* dependency_period) {
  // Make sure we have what we need.
  if (!dependent_host_name
      || !dependent_host_name[0]
      || !host_name
      || !host_name[0]) {
    logger(log_config_error, basic)
      << "Error: NULL host name in host dependency definition";
    return (NULL);
  }

  // Allocate memory for a new host dependency entry.
  shared_ptr<hostdependency> obj(new hostdependency, deleter::hostdependency);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->dependent_host_name = my_strdup(dependent_host_name);
    obj->host_name = my_strdup(host_name);
    if (dependency_period)
      obj->dependency_period = my_strdup(dependency_period);
    obj->dependency_type = (dependency_type == EXECUTION_DEPENDENCY ? EXECUTION_DEPENDENCY : NOTIFICATION_DEPENDENCY);
    obj->fail_on_down = (fail_on_down == 1);
    obj->fail_on_pending = (fail_on_pending == 1);
    obj->fail_on_unreachable = (fail_on_unreachable == 1);
    obj->fail_on_up = (fail_on_up == 1);
    obj->inherits_parent = (inherits_parent > 0);

    // Add new hostdependency to the monitoring engine.
    umap<std::string, shared_ptr<hostdependency_struct> >::const_iterator
      it(state::instance().hostdependencies().find(dependent_host_name));
    if (it != state::instance().hostdependencies().end()) {
      logger(log_config_error, basic)
        << "Error: Hostdependency '" << dependent_host_name << "' has already been defined";
      return (NULL);
    }

    // Add new items to the list.
    obj->next = hostdependency_list;
    hostdependency_list = obj.get();

    // Notify event broker.
    // XXX
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Adds a host notification command to a contact definition.
 *
 *  @param[in] cntct        Contact.
 *  @param[in] command_name Notification command name.
 *
 *  @return Contact notification command.
 */
commandsmember* add_host_notification_command_to_contact(
                  contact* cntct,
                  char const* command_name) {
  // Make sure we have the data we need.
  if (!cntct || !command_name || !command_name[0]) {
    logger(log_config_error, basic)
      << "Error: Contact or host notification command is NULL";
    return (NULL);
  }

  // Allocate memory.
  commandsmember* obj(new commandsmember);
  memset(obj, 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->cmd = my_strdup(command_name);

    // Add the notification command.
    obj->next = cntct->host_notification_commands;
    cntct->host_notification_commands = obj;

    // Notify event broker.
    // XXX
  }
  catch (...) {
    deleter::commandsmember(obj);
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
 *  Add a new host escalation to the list in memory.
 *
 *  @param[in] host_name               Host name.
 *  @param[in] first_notification      First notification.
 *  @param[in] last_notification       Last notification.
 *  @param[in] notification_interval   Notification interval.
 *  @param[in] escalation_period       Escalation timeperiod name.
 *  @param[in] escalate_on_down        Escalate on down ?
 *  @param[in] escalate_on_unreachable Escalate on unreachable ?
 *  @param[in] escalate_on_recovery    Escalate on recovery ?
 *
 *  @return New host escalation.
 */
hostescalation* add_host_escalation(
                  char const* host_name,
                  int first_notification,
                  int last_notification,
                  double notification_interval,
                  char const* escalation_period,
                  int escalate_on_down,
                  int escalate_on_unreachable,
                  int escalate_on_recovery) {
  // Make sure we have the data we need.
  if (!host_name || !host_name[0]) {
    logger(log_config_error, basic)
      << "Error: Host escalation host name is NULL";
    return (NULL);
  }

  // Allocate memory for a new host escalation entry.
  shared_ptr<hostescalation> obj(new hostescalation, deleter::hostescalation);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->host_name = my_strdup(host_name);
    if (escalation_period)
      obj->escalation_period = my_strdup(escalation_period);
    obj->escalate_on_down = (escalate_on_down > 0);
    obj->escalate_on_recovery = (escalate_on_recovery > 0);
    obj->escalate_on_unreachable = (escalate_on_unreachable > 0);
    obj->first_notification = first_notification;
    obj->last_notification = last_notification;
    obj->notification_interval = (notification_interval <= 0) ? 0 : notification_interval;

    // Add new hostescalation to the monitoring engine.
    umap<std::string, shared_ptr<hostescalation_struct> >::const_iterator
      it(state::instance().hostescalations().find(host_name));
    if (it != state::instance().hostescalations().end()) {
      logger(log_config_error, basic)
        << "Error: Hostescalation '" << host_name << "' has already been defined";
      return (NULL);
    }

    // Add new items to the list.
    obj->next = hostescalation_list;
    hostescalation_list = obj.get();

    // Notify event broker.
    // XXX
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Add a new host group to the list in memory.
 *
 *  @param[in] name       Host group name.
 *  @param[in] alias      Host group alias.
 *  @param[in] notes      Notes.
 *  @param[in] notes_url  URL.
 *  @param[in] action_url Action URL.
 *
 *  @return New host group.
 */
hostgroup* add_hostgroup(
             char const* name,
             char const* alias,
             char const* notes,
             char const* notes_url,
             char const* action_url) {
  // Make sure we have the data we need.
  if (!name || !name[0]) {
    logger(log_config_error, basic)
      << "Error: Hostgroup name is NULL";
    return (NULL);
  }

  // Allocate memory.
  shared_ptr<hostgroup> obj(new hostgroup, deleter::hostgroup);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->group_name = my_strdup(name);
    obj->alias = my_strdup(alias ? alias : name);
    if (action_url)
      obj->action_url = my_strdup(action_url);
    if (notes)
      obj->notes = my_strdup(notes);
    if (notes_url)
      obj->notes_url = my_strdup(notes_url);

    // Add new hostgroup to the monitoring engine.
    umap<std::string, shared_ptr<hostgroup_struct> >::const_iterator
      it(state::instance().hostgroups().find(name));
    if (it != state::instance().hostgroups().end()) {
      logger(log_config_error, basic)
        << "Error: Hostgroup '" << name << "' has already been defined";
      return (NULL);
    }

    // Add new items to the list.
    obj->next = hostgroup_list;
    hostgroup_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group(
      NEBTYPE_HOSTGROUP_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
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

/**
 *  Add a new service to the list in memory.
 *
 *  @param[in] host_name                    Name of the host this
 *                                          service is running on.
 *  @param[in] description                  Service description.
 *  @param[in] display_name                 Display name.
 *  @param[in] check_period                 Check timeperiod name.
 *  @param[in] initial_state                Initial service state.
 *  @param[in] max_attempts                 Max check attempts.
 *  @param[in] parallelize                  Can active checks be
 *                                          parallelized ?
 *  @param[in] accept_passive_checks        Does this service accept
 *                                          check result submission ?
 *  @param[in] check_interval               Normal check interval.
 *  @param[in] retry_interval               Retry check interval.
 *  @param[in] notification_interval        Notification interval.
 *  @param[in] first_notification_delay     First notification delay.
 *  @param[in] notification_period          Notification timeperiod
 *                                          name.
 *  @param[in] notify_recovery              Does this service notify
 *                                          when recovering ?
 *  @param[in] notify_unknown               Does this service notify in
 *                                          unknown state ?
 *  @param[in] notify_warning               Does this service notify in
 *                                          warning state ?
 *  @param[in] notify_critical              Does this service notify in
 *                                          critical state ?
 *  @param[in] notify_flapping              Does this service notify
 *                                          when flapping ?
 *  @param[in] notify_downtime              Does this service notify on
 *                                          downtime ?
 *  @param[in] notifications_enabled        Are notifications enabled
 *                                          for this service ?
 *  @param[in] is_volatile                  Is this service volatile ?
 *  @param[in] event_handler                Event handler command name.
 *  @param[in] event_handler_enabled        Whether or not event handler
 *                                          is enabled.
 *  @param[in] check_command                Active check command name.
 *  @param[in] checks_enabled               Are active checks enabled ?
 *  @param[in] flap_detection_enabled       Whether or not flap
 *                                          detection is enabled.
 *  @param[in] low_flap_threshold           Low flap threshold.
 *  @param[in] high_flap_threshold          High flap threshold.
 *  @param[in] flap_detection_on_ok         Is flap detection enabled
 *                                          for ok state ?
 *  @param[in] flap_detection_on_warning    Is flap detection enabled
 *                                          for warning state ?
 *  @param[in] flap_detection_on_unknown    Is flap detection enabled
 *                                          for unknown state ?
 *  @param[in] flap_detection_on_critical   Is flap detection enabled
 *                                          for critical state ?
 *  @param[in] stalk_on_ok                  Stalk on ok state ?
 *  @param[in] stalk_on_warning             Stalk on warning state ?
 *  @param[in] stalk_on_unknown             Stalk on unknown state ?
 *  @param[in] stalk_on_critical            Stalk on critical state ?
 *  @param[in] process_perfdata             Whether or not service
 *                                          performance data should be
 *                                          processed.
 *  @param[in] failure_prediction_enabled   Whether failure prediction
 *                                          should be enabled or not.
 *  @param[in] failure_prediction_options   Failure prediction options.
 *  @param[in] check_freshness              Enable freshness check ?
 *  @param[in] freshness_threshold          Freshness threshold.
 *  @param[in] notes                        Notes.
 *  @param[in] notes_url                    URL.
 *  @param[in] action_url                   Action URL.
 *  @param[in] icon_image                   Icon image.
 *  @param[in] icon_image_alt               Alternative icon image.
 *  @param[in] retain_status_information    Should Engine retain service
 *                                          status information ?
 *  @param[in] retain_nonstatus_information Should Engine retain service
 *                                          non-status information ?
 *  @param[in] obsess_over_service          Should we obsess over
 *                                          service ?
 *
 *  @return New service.
 */
service* add_service(
           char const* host_name,
           char const* description,
           char const* display_name,
           char const* check_period,
           int initial_state,
           int max_attempts,
           int parallelize,
           int accept_passive_checks,
           double check_interval,
           double retry_interval,
           double notification_interval,
           double first_notification_delay,
           char const* notification_period,
           int notify_recovery,
           int notify_unknown,
           int notify_warning,
           int notify_critical,
           int notify_flapping,
           int notify_downtime,
           int notifications_enabled,
           int is_volatile,
           char const* event_handler,
           int event_handler_enabled,
           char const* check_command,
           int checks_enabled,
           int flap_detection_enabled,
           double low_flap_threshold,
           double high_flap_threshold,
           int flap_detection_on_ok,
           int flap_detection_on_warning,
           int flap_detection_on_unknown,
           int flap_detection_on_critical,
           int stalk_on_ok,
           int stalk_on_warning,
           int stalk_on_unknown,
           int stalk_on_critical,
           int process_perfdata,
           int failure_prediction_enabled,
           char const* failure_prediction_options,
           int check_freshness,
           int freshness_threshold,
           char const* notes,
           char const* notes_url,
           char const* action_url,
           char const* icon_image,
           char const* icon_image_alt,
           int retain_status_information,
           int retain_nonstatus_information,
           int obsess_over_service) {
  // Make sure we have everything we need.
  if (!description || !description[0]) {
    logger(log_config_error, basic)
      << "error: service description is not set";
    return (NULL);
  }
  else if (!host_name || !host_name[0]) {
    logger(log_config_error, basic)
      << "error: host name of service '"
      << description << "' is not set";
    return (NULL);
  }
  else if (!check_command || !check_command[0]) {
    logger(log_config_error, basic)
      << "error: check command of service '" << description
      << "' on host '" << host_name << "' is not set";
    return (NULL);
  }

  // Check values.
  if ((max_attempts <= 0)
      || (check_interval < 0)
      || (retry_interval <= 0)
      || (notification_interval < 0)) {
    logger(log_config_error, basic)
      << "Error: Invalid max_attempts, check_interval, retry_interval"
         ", or notification_interval value for service '"
      << description << "' on host '" << host_name << "'";
    return (NULL);
  }
  if (first_notification_delay < 0) {
    logger(log_config_error, basic)
      << "Error: Invalid first_notification_delay value for service '"
      << description << "' on host '" << host_name << "'";
    return (NULL);
  }

  // Allocate memory.
  shared_ptr<service> obj(new service, deleter::service);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->host_name = my_strdup(host_name);
    obj->description = my_strdup(description);
    obj->display_name = my_strdup(display_name ? display_name : description);
    obj->service_check_command = my_strdup(check_command);
    if (event_handler)
      obj->event_handler = my_strdup(event_handler);
    if (notification_period)
      obj->notification_period = my_strdup(notification_period);
    if (check_period)
      obj->check_period = my_strdup(check_period);
    if (failure_prediction_options)
      obj->failure_prediction_options = my_strdup(failure_prediction_options);
    if (notes)
      obj->notes = my_strdup(notes);
    if (notes_url)
      obj->notes_url = my_strdup(notes_url);
    if (action_url)
      obj->action_url = my_strdup(action_url);
    if (icon_image)
      obj->icon_image = my_strdup(icon_image);
    if (icon_image_alt)
      obj->icon_image_alt = my_strdup(icon_image_alt);

    obj->accept_passive_service_checks = (accept_passive_checks > 0);
    obj->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
    obj->check_freshness = (check_freshness > 0);
    obj->check_interval = check_interval;
    obj->check_options = CHECK_OPTION_NONE;
    obj->check_type = SERVICE_CHECK_ACTIVE;
    obj->checks_enabled = (checks_enabled > 0);
    obj->current_attempt = (initial_state == STATE_OK) ? 1 : max_attempts;
    obj->current_state = initial_state;
    obj->event_handler_enabled = (event_handler_enabled > 0);
    obj->failure_prediction_enabled = (failure_prediction_enabled > 0);
    obj->first_notification_delay = first_notification_delay;
    obj->flap_detection_enabled = (flap_detection_enabled > 0);
    obj->flap_detection_on_critical = (flap_detection_on_critical > 0);
    obj->flap_detection_on_ok = (flap_detection_on_ok > 0);
    obj->flap_detection_on_unknown = (flap_detection_on_unknown > 0);
    obj->flap_detection_on_warning = (flap_detection_on_warning > 0);
    obj->freshness_threshold = freshness_threshold;
    obj->high_flap_threshold = high_flap_threshold;
    obj->is_volatile = (is_volatile > 0);
    obj->last_hard_state = initial_state;
    obj->last_state = initial_state;
    obj->low_flap_threshold = low_flap_threshold;
    obj->max_attempts = max_attempts;
    obj->modified_attributes = MODATTR_NONE;
    obj->notification_interval = notification_interval;
    obj->notifications_enabled = (notifications_enabled > 0);
    obj->notify_on_critical = (notify_critical > 0);
    obj->notify_on_downtime = (notify_downtime > 0);
    obj->notify_on_flapping = (notify_flapping > 0);
    obj->notify_on_recovery = (notify_recovery > 0);
    obj->notify_on_unknown = (notify_unknown > 0);
    obj->notify_on_warning = (notify_warning > 0);
    obj->obsess_over_service = (obsess_over_service > 0);
    obj->parallelize = (parallelize > 0);
    obj->process_performance_data = (process_perfdata > 0);
    obj->retain_nonstatus_information = (retain_nonstatus_information > 0);
    obj->retain_status_information = (retain_status_information > 0);
    obj->retry_interval = retry_interval;
    obj->should_be_scheduled = true;
    obj->stalk_on_critical = (stalk_on_critical > 0);
    obj->stalk_on_ok = (stalk_on_ok > 0);
    obj->stalk_on_unknown = (stalk_on_unknown > 0);
    obj->stalk_on_warning = (stalk_on_warning > 0);
    obj->state_type = HARD_STATE;

    for (unsigned int x(0); x < MAX_STATE_HISTORY_ENTRIES; ++x)
      obj->state_history[x] = STATE_OK;

    umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
      it(state::instance().services().find(std::make_pair(host_name, description)));
    if (it != state::instance().services().end()) {
      logger(log_config_error, basic)
        << "Error: Service '" << description << "' on host '"
        << host_name << "' has already been defined";
      return (NULL);
    }

    // Add new items to the list.
    obj->next = service_list;
    service_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_service_data(
      NEBTYPE_SERVICE_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Adds a service dependency definition.
 *
 *  @param[in] dependent_host_name           Dependent host name.
 *  @param[in] dependent_service_description Dependent service
 *                                           description.
 *  @param[in] host_name                     Host name.
 *  @param[in] service_description           Service description.
 *  @param[in] dependency_type               Type of dependency.
 *  @param[in] inherits_parent               Inherits parent ?
 *  @param[in] fail_on_ok                    Does dependency fail on
 *                                           ok state ?
 *  @param[in] fail_on_warning               Does dependency fail on
 *                                           warning state ?
 *  @param[in] fail_on_unknown               Does dependency fail on
 *                                           unknown state ?
 *  @param[in] fail_on_critical              Does dependency fail on
 *                                           critical state ?
 *  @param[in] fail_on_pending               Does dependency fail on
 *                                           pending state ?
 *  @param[in] dependency_period             Dependency timeperiod name.
 *
 *  @return Service dependency.
 */
servicedependency* add_service_dependency(
                     char const* dependent_host_name,
                     char const* dependent_service_description,
                     char const* host_name,
                     char const* service_description,
                     int dependency_type,
                     int inherits_parent,
                     int fail_on_ok,
                     int fail_on_warning,
                     int fail_on_unknown,
                     int fail_on_critical,
                     int fail_on_pending,
                     char const* dependency_period) {
  // Make sure we have what we need.
  if (!host_name
      || !host_name[0]
      || !service_description
      || !service_description[0]) {
    logger(log_config_error, basic)
      << "Error: NULL master service description/host "
         "name in service dependency definition";
    return (NULL);
  }
  if (!dependent_host_name
      || !dependent_host_name[0]
      || !dependent_service_description
      || !dependent_service_description[0]) {
    logger(log_config_error, basic)
      << "Error: NULL dependent service description/host "
         "name in service dependency definition";
    return (NULL);
  }

  // Allocate memory for a new service dependency entry.
  shared_ptr<servicedependency> obj(new servicedependency, deleter::servicedependency);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->dependent_host_name = my_strdup(dependent_host_name);
    obj->dependent_service_description = my_strdup(dependent_service_description);
    obj->host_name = my_strdup(host_name);
    obj->service_description = my_strdup(service_description);
    if (dependency_period)
      obj->dependency_period = my_strdup(dependency_period);

    obj->dependency_type = (dependency_type == EXECUTION_DEPENDENCY) ? EXECUTION_DEPENDENCY : NOTIFICATION_DEPENDENCY;
    obj->fail_on_critical = (fail_on_critical == 1);
    obj->fail_on_ok = (fail_on_ok == 1);
    obj->fail_on_pending = (fail_on_pending == 1);
    obj->fail_on_unknown = (fail_on_unknown == 1);
    obj->fail_on_warning = (fail_on_warning == 1);
    obj->inherits_parent = (inherits_parent > 0);

    umultimap<std::pair<std::string, std::string>, shared_ptr<servicedependency_struct> >::const_iterator
      it(state::instance().servicedependencies().find(std::make_pair(dependent_host_name, dependent_service_description)));
    if (it != state::instance().servicedependencies().end()) {
      logger(log_config_error, basic)
        << "Error: Servicedependency '" << dependent_service_description
        << "' on host '" << dependent_host_name
        << "' has already been defined";
      return (NULL);
    }

    // Add new items to the list.
    obj->next = servicedependency_list;
    servicedependency_list = obj.get();

    // Notify event broker.
    // XXX
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
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
    // XXX
  }
  catch (...) {
    deleter::servicesmember(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Adds a service notification command to a contact definition.
 *
 *  @param[in,out] cntct        Target contact.
 *  @param[in]     command_name Service notification command name.
 *
 *  @return Service notification command of contact.
 */
commandsmember* add_service_notification_command_to_contact(
                  contact*  cntct,
                  char const* command_name) {
  // Make sure we have the data we need.
  if (!cntct || !command_name || !command_name[0]) {
    logger(log_config_error, basic)
      << "Error: Contact or service notification command is NULL";
    return (NULL);
  }

  // Allocate memory.
  commandsmember* obj(new commandsmember);
  memset(obj, 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->cmd = my_strdup(command_name);

    // Add the notification command.
    obj->next = cntct->service_notification_commands;
    cntct->service_notification_commands = obj;

    // Notify event broker.
    // XXX
  }
  catch (...) {
    deleter::commandsmember(obj);
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
                  servicegroup* temp_servicegroup,
                  char const* host_name,
                  char const* svc_description) {
  // Make sure we have the data we need.
  if (!temp_servicegroup
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
    obj->host_name = my_strdup(host_name);
    obj->service_description = my_strdup(svc_description);

    // Add new member to member list, sorted by host name then
    // service description.
    servicesmember* last(temp_servicegroup->members);
    servicesmember* temp;
    for (temp = temp_servicegroup->members; temp; temp = temp->next) {
      if (strcmp(obj->host_name, temp->host_name) < 0) {
        obj->next = temp;
        if (temp == temp_servicegroup->members)
          temp_servicegroup->members = obj;
        else
          last->next = obj;
        break;
      }
      else if (!strcmp(obj->host_name, temp->host_name)
               && (strcmp(obj->service_description, temp->service_description) < 0)) {
        obj->next = temp;
        if (temp == temp_servicegroup->members)
          temp_servicegroup->members = obj;
        else
          last->next = obj;
        break;
      }
      else
        last = temp;
    }
    if (!temp_servicegroup->members)
      temp_servicegroup->members = obj;
    else if (!temp)
      last->next = obj;

    // Notify event broker.
    // XXX
  }
  catch (...) {
    deleter::servicesmember(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Add a new service escalation to the list in memory.
 *
 *  @param[in] host_name             Host name.
 *  @param[in] description           Description.
 *  @param[in] first_notification    First notification.
 *  @param[in] last_notification     Last notification.
 *  @param[in] notification_interval Notification interval.
 *  @param[in] escalation_period     Escalation timeperiod name.
 *  @param[in] escalate_on_warning   Do we escalate on warning ?
 *  @param[in] escalate_on_unknown   Do we escalate on unknown ?
 *  @param[in] escalate_on_critical  Do we escalate on critical ?
 *  @param[in] escalate_on_recovery  Do we escalate on recovery ?
 *
 *  @return New service escalation.
 */
serviceescalation* add_service_escalation(
                     char const* host_name,
                     char const* description,
                     int first_notification,
                     int last_notification,
                     double notification_interval,
                     char const* escalation_period,
                     int escalate_on_warning,
                     int escalate_on_unknown,
                     int escalate_on_critical,
                     int escalate_on_recovery) {
  // Make sure we have the data we need.
  if (!host_name
      || !host_name[0]
      || !description
      || !description[0]) {
    logger(log_config_error, basic)
      << "Error: Service escalation host name or description is NULL";
    return (NULL);
  }

  // Allocate memory for a new service escalation entry.
  shared_ptr<serviceescalation> obj(new serviceescalation, deleter::serviceescalation);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->host_name = my_strdup(host_name);
    obj->description = my_strdup(description);
    if (escalation_period)
      obj->escalation_period = my_strdup(escalation_period);

    obj->escalate_on_critical = (escalate_on_critical > 0);
    obj->escalate_on_recovery = (escalate_on_recovery > 0);
    obj->escalate_on_unknown = (escalate_on_unknown > 0);
    obj->escalate_on_warning = (escalate_on_warning > 0);
    obj->first_notification = first_notification;
    obj->last_notification = last_notification;
    obj->notification_interval = (notification_interval <= 0) ? 0 : notification_interval;

    // XXX: Add new serviceescalation to the monitoring engine.
    umultimap<std::pair<std::string, std::string>, shared_ptr<serviceescalation_struct> >::const_iterator
      it(state::instance().serviceescalations().find(std::make_pair(host_name, description)));
    if (it != state::instance().serviceescalations().end()) {
      logger(log_config_error, basic)
        << "Error: Serviceescalation '" << description << "' on host '"
        << host_name << "' has already been defined";
      return (NULL);
    }

    // Add new items to tail the list.
    obj->next = serviceescalation_list;
    serviceescalation_list = obj.get();

    // Notify event broker.
    // XXX
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Add a new service group to the list in memory.
 *
 *  @param[in] name       Group name.
 *  @param[in] alias      Group alias.
 *  @param[in] notes      Notes.
 *  @param[in] notes_url  URL.
 *  @param[in] action_url Action URL.
 *
 *  @return New service group.
 */
servicegroup* add_servicegroup(
                char const* name,
                char const* alias,
                char const* notes,
                char const* notes_url,
                char const* action_url) {
  // Make sure we have the data we need.
  if (!name || !name[0]) {
    logger(log_config_error, basic)
      << "Error: Servicegroup name is NULL";
    return (NULL);
  }

  // Allocate memory.
  shared_ptr<servicegroup> obj(new servicegroup, deleter::servicegroup);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->group_name = my_strdup(name);
    obj->alias = my_strdup(alias ? alias : name);
    if (action_url)
      obj->action_url = my_strdup(action_url);
    if (notes)
      obj->notes = my_strdup(notes);
    if (notes_url)
      obj->notes_url = my_strdup(notes_url);

    // Add new servicegroup to the monitoring engine.
    umap<std::string, shared_ptr<servicegroup_struct> >::const_iterator
      it(state::instance().servicegroups().find(name));
    if (it != state::instance().servicegroups().end()) {
      logger(log_config_error, basic)
        << "Error: Servicegroup '" << name << "' has already been defined";
      return (NULL);
    }

    // Add  new items to the list.
    obj->next = servicegroup_list;
    servicegroup_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group(
      NEBTYPE_SERVICEGROUP_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Add a new timeperiod to the list in memory.
 *
 *  @param[in] name  Time period name.
 *  @param[in] alias Time period alias.
 *
 *  @return New timeperiod object.
 */
timeperiod* add_timeperiod(char const* name, char const* alias) {
  // Make sure we have the data we need.
  if (!name || !name[0] || !alias || !alias[0]) {
    logger(log_config_error, basic)
      << "Error: Name or alias for timeperiod is NULL";
    return (NULL);
  }

  // Allocate memory for the new timeperiod.
  shared_ptr<timeperiod> obj(new timeperiod, deleter::timeperiod);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Copy string vars.
    obj->name = my_strdup(name);
    obj->alias = my_strdup(alias);

    // Add new timeperiod to the monitoring engine.
    umap<std::string, shared_ptr<timeperiod_struct> >::const_iterator
      it(state::instance().timeperiods().find(name));
    if (it != state::instance().timeperiods().end()) {
      logger(log_config_error, basic)
        << "Error: Timeperiod '" << name << "' has already been defined";
      return (NULL);
    }

    // Add new items to the list.
    obj->next = timeperiod_list;
    timeperiod_list = obj.get();

    // Notify event broker.
    // XXX
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Add a new timerange to a timeperiod.
 *
 *  @param[in,out] period     Target period.
 *  @param[in]     day        Day of the range ([0-6]).
 *  @param[in]     start_time Range start time.
 *  @param[in]     end_time   Range end time.
 *
 *  @return New time range.
 */
timerange* add_timerange_to_timeperiod(
             timeperiod* period,
             int day,
             unsigned long start_time,
             unsigned long end_time) {
  // Make sure we have the data we need.
  if (!period)
    return (NULL);
  if (day < 0 || day > 6) {
    logger(log_config_error, basic)
      << "Error: Day " << day
      << " is not valid for timeperiod '" << period->name << "'";
    return (NULL);
  }
  if (start_time > 86400) {
    logger(log_config_error, basic)
      << "Error: Start time " << start_time << " on day " << day
      << " is not valid for timeperiod '" << period->name << "'";
    return (NULL);
  }
  if (end_time > 86400) {
    logger(log_config_error, basic)
      << "Error: End time " << end_time << " on day " << day
      << " is not value for timeperiod '" << period->name << "'";
    return (NULL);
  }

  // Allocate memory for the new time range.
  timerange* obj(new timerange);
  memset(obj, 0, sizeof(*obj));

  try {
    obj->range_start = start_time;
    obj->range_end = end_time;

    // Add the new time range to the head of the range list for this day.
    obj->next = period->days[day];
    period->days[day] = obj;

    // Notify event broker.
    // XXX
  }
  catch (...) {
    deleter::timerange(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Add a new timerange to a daterange.
 *
 *  @param[in,out] drange     Target date range.
 *  @param[in]     start_time Range start time.
 *  @param[in]     end_time   Range end time.
 *
 *  @return New timerange.
 */
timerange* add_timerange_to_daterange(
             daterange* drange,
             unsigned long start_time,
             unsigned long end_time) {
  // Make sure we have the data we need.
  if (!drange)
    return (NULL);
  if (start_time > 86400) {
    logger(log_config_error, basic)
      << "Error: Start time " << start_time
      << " is not valid for timeperiod";
    return (NULL);
  }
  if (end_time > 86400) {
    logger(log_config_error, basic)
      << "Error: End time " << end_time
      << " is not value for timeperiod";
    return (NULL);
  }

  // Allocate memory for the new time range.
  timerange* obj(new timerange);
  memset(obj, 0, sizeof(*obj));

  try {
    obj->range_start = start_time;
    obj->range_end = end_time;

    // Add the new time range to the head of the range
    // list for this date range.
    obj->next = drange->times;
    drange->times = obj;

    // Notify event broker.
    // XXX
  }
  catch (...) {
    deleter::timerange(obj);
    obj = NULL;
  }

  return (obj);
}

/**************************************
*                                     *
*      Object Cleanup Functions       *
*                                     *
**************************************/

/**
 *  Free all allocated memory for objects.
 *
 *  @return OK on success.
 */
int free_object_data() {
  // XXX: remove this code !
  // // Free memory for the timeperiod list.
  // for (timeperiod *this_timeperiod(timeperiod_list), *next_timeperiod;
  //      this_timeperiod;
  //      this_timeperiod = next_timeperiod) {
  //   // Free the exception time ranges contained in this timeperiod.
  //   for (unsigned int x(0); x < DATERANGE_TYPES; ++x) {
  //     for (daterange
  //            *this_daterange(this_timeperiod->exceptions[x]),
  //            *next_daterange;
  //          this_daterange;
  //          this_daterange = next_daterange) {
  //       next_daterange = this_daterange->next;
  //       for (timerange
  //              *this_timerange(this_daterange->times),
  //              *next_timerange;
  //            this_timerange;
  //            this_timerange = next_timerange) {
  //         next_timerange = this_timerange->next;
  //         delete this_timerange;
  //       }
  //       delete this_daterange;
  //     }
  //   }

  //   // Free the day time ranges contained in this timeperiod.
  //   for (unsigned int x(0); x < 7; ++x) {
  //     for (timerange
  //            *this_timerange(this_timeperiod->days[x]),
  //            *next_timerange;
  //          this_timerange;
  //          this_timerange = next_timerange) {
  //       next_timerange = this_timerange->next;
  //       delete this_timerange;
  //     }
  //   }

  //   // Free exclusions.
  //   for (timeperiodexclusion
  //          *this_timeperiodexclusion(this_timeperiod->exclusions),
  //          *next_timeperiodexclusion;
  //        this_timeperiodexclusion;
  //        this_timeperiodexclusion = next_timeperiodexclusion) {
  //     next_timeperiodexclusion = this_timeperiodexclusion->next;
  //     delete[] this_timeperiodexclusion->timeperiod_name;
  //     delete this_timeperiodexclusion;
  //   }

  //   // Next timeperiod.
  //   next_timeperiod = this_timeperiod->next;

  //   // Free current timeperiod.
  //   delete[] this_timeperiod->name;
  //   delete[] this_timeperiod->alias;
  //   delete this_timeperiod;
  // }
  // timeperiod_list = NULL;

  // // Free memory for the host list.
  // for (host *this_host(host_list), *next_host;
  //      this_host;
  //      this_host = next_host) {
  //   // Free memory for parent hosts.
  //   for (hostsmember
  //          *this_hostsmember(this_host->parent_hosts),
  //          *next_hostsmember;
  //        this_hostsmember;
  //        this_hostsmember = next_hostsmember) {
  //     next_hostsmember = this_hostsmember->next;
  //     delete[] this_hostsmember->host_name;
  //     delete this_hostsmember;
  //   }

  //   // Free memory for child host links.
  //   for (hostsmember
  //          *this_hostsmember(this_host->child_hosts),
  //          *next_hostsmember;
  //        this_hostsmember;
  //        this_hostsmember = next_hostsmember) {
  //     next_hostsmember = this_hostsmember->next;
  //     delete[] this_hostsmember->host_name;
  //     delete this_hostsmember;
  //   }

  //   // Free memory for service links.
  //   for (servicesmember
  //          *this_servicesmember(this_host->services),
  //          *next_servicesmember;
  //        this_servicesmember;
  //        this_servicesmember = next_servicesmember) {
  //     next_servicesmember = this_servicesmember->next;
  //     delete[] this_servicesmember->host_name;
  //     delete[] this_servicesmember->service_description;
  //     delete this_servicesmember;
  //   }

  //   // Free memory for contact groups.
  //   for (contactgroupsmember
  //          *this_contactgroupsmember(this_host->contact_groups),
  //          *next_contactgroupsmember;
  //        this_contactgroupsmember;
  //        this_contactgroupsmember = next_contactgroupsmember) {
  //     next_contactgroupsmember = this_contactgroupsmember->next;
  //     delete[] this_contactgroupsmember->group_name;
  //     delete this_contactgroupsmember;
  //   }

  //   // Free memory for contacts.
  //   for (contactsmember
  //          *this_contactsmember(this_host->contacts),
  //          *next_contactsmember;
  //        this_contactsmember;
  //        this_contactsmember = next_contactsmember) {
  //     next_contactsmember = this_contactsmember->next;
  //     delete[] this_contactsmember->contact_name;
  //     delete this_contactsmember;
  //   }

  //   // Free memory for custom variables.
  //   for (customvariablesmember
  //          *this_customvariablesmember(this_host->custom_variables),
  //          *next_customvariablesmember;
  //        this_customvariablesmember;
  //        this_customvariablesmember = next_customvariablesmember) {
  //     next_customvariablesmember = this_customvariablesmember->next;
  //     delete[] this_customvariablesmember->variable_name;
  //     delete[] this_customvariablesmember->variable_value;
  //     delete this_customvariablesmember;
  //   }

  //   // Next host.
  //   next_host = this_host->next;

  //   // Free current host.
  //   delete[] this_host->name;
  //   delete[] this_host->display_name;
  //   delete[] this_host->alias;
  //   delete[] this_host->address;
  //   delete[] this_host->plugin_output;
  //   delete[] this_host->long_plugin_output;
  //   delete[] this_host->perf_data;
  //   free_objectlist(&this_host->hostgroups_ptr);
  //   delete[] this_host->check_period;
  //   delete[] this_host->host_check_command;
  //   delete[] this_host->event_handler;
  //   delete[] this_host->failure_prediction_options;
  //   delete[] this_host->notification_period;
  //   delete[] this_host->notes;
  //   delete[] this_host->notes_url;
  //   delete[] this_host->action_url;
  //   delete[] this_host->icon_image;
  //   delete[] this_host->icon_image_alt;
  //   delete[] this_host->vrml_image;
  //   delete[] this_host->statusmap_image;
  //   delete this_host;
  // }
  // host_list = NULL;

  // // Free memory for the host group list.
  // for (hostgroup *this_hostgroup(hostgroup_list), *next_hostgroup;
  //      this_hostgroup;
  //      this_hostgroup = next_hostgroup) {
  //   // Free memory for the group members.
  //   for (hostsmember
  //          *this_hostsmember(this_hostgroup->members),
  //          *next_hostsmember;
  //        this_hostsmember;
  //        this_hostsmember = next_hostsmember) {
  //     next_hostsmember = this_hostsmember->next;
  //     delete[] this_hostsmember->host_name;
  //     delete this_hostsmember;
  //   }

  //   // Next host group.
  //   next_hostgroup = this_hostgroup->next;

  //   // Free current host group.
  //   delete[] this_hostgroup->group_name;
  //   delete[] this_hostgroup->alias;
  //   delete[] this_hostgroup->notes;
  //   delete[] this_hostgroup->notes_url;
  //   delete[] this_hostgroup->action_url;
  //   delete this_hostgroup;
  // }
  // hostgroup_list = NULL;

  // // Free memory for the service group list.
  // for (servicegroup
  //        *this_servicegroup(servicegroup_list),
  //        *next_servicegroup;
  //      this_servicegroup;
  //      this_servicegroup = next_servicegroup) {
  //   // Free memory for the group members.
  //   for (servicesmember
  //          *this_servicesmember(this_servicegroup->members),
  //          *next_servicesmember;
  //        this_servicesmember;
  //        this_servicesmember = next_servicesmember) {
  //     next_servicesmember = this_servicesmember->next;
  //     delete[] this_servicesmember->host_name;
  //     delete[] this_servicesmember->service_description;
  //     delete this_servicesmember;
  //   }

  //   // Next service group.
  //   next_servicegroup = this_servicegroup->next;

  //   // Free current service group.
  //   delete[] this_servicegroup->group_name;
  //   delete[] this_servicegroup->alias;
  //   delete[] this_servicegroup->notes;
  //   delete[] this_servicegroup->notes_url;
  //   delete[] this_servicegroup->action_url;
  //   delete this_servicegroup;
  // }
  // servicegroup_list = NULL;

  // // Free memory for the contact list.
  // for (contact *this_contact(contact_list), *next_contact;
  //      this_contact;
  //      this_contact = next_contact) {
  //   // Free memory for the host notification commands.
  //   for (commandsmember
  //          *this_commandsmember(this_contact->host_notification_commands),
  //          *next_commandsmember;
  //        this_commandsmember;
  //        this_commandsmember = next_commandsmember) {
  //     next_commandsmember = this_commandsmember->next;
  //     if (this_commandsmember->cmd)
  //       delete[] this_commandsmember->cmd;
  //     delete this_commandsmember;
  //   }

  //   // Free memory for the service notification commands.
  //   for (commandsmember
  //          *this_commandsmember(this_contact->service_notification_commands),
  //          *next_commandsmember;
  //        this_commandsmember;
  //        this_commandsmember = next_commandsmember) {
  //     next_commandsmember = this_commandsmember->next;
  //     if (this_commandsmember->cmd)
  //       delete[] this_commandsmember->cmd;
  //     delete this_commandsmember;
  //   }

  //   // Free memory for custom variables.
  //   for (customvariablesmember
  //          *this_customvariablesmember(this_contact->custom_variables),
  //          *next_customvariablesmember;
  //        this_customvariablesmember;
  //        this_customvariablesmember = next_customvariablesmember) {
  //     next_customvariablesmember = this_customvariablesmember->next;
  //     delete[] this_customvariablesmember->variable_name;
  //     delete[] this_customvariablesmember->variable_value;
  //     delete this_customvariablesmember;
  //   }

  //   // Next contact.
  //   next_contact = this_contact->next;

  //   // Free current contact.
  //   delete[] this_contact->name;
  //   delete[] this_contact->alias;
  //   delete[] this_contact->email;
  //   delete[] this_contact->pager;
  //   for (unsigned int i(0); i < MAX_CONTACT_ADDRESSES; ++i)
  //     delete[] this_contact->address[i];
  //   delete[] this_contact->host_notification_period;
  //   delete[] this_contact->service_notification_period;
  //   free_objectlist(&this_contact->contactgroups_ptr);
  //   delete this_contact;
  // }
  // contact_list = NULL;

  // // Free memory for the contact group list.
  // for (contactgroup
  //        *this_contactgroup(contactgroup_list),
  //        *next_contactgroup;
  //      this_contactgroup;
  //      this_contactgroup = next_contactgroup) {
  //   // Free memory for the group members.
  //   for (contactsmember
  //          *this_contactsmember(this_contactgroup->members),
  //          *next_contactsmember;
  //        this_contactsmember;
  //        this_contactsmember = next_contactsmember) {
  //     next_contactsmember = this_contactsmember->next;
  //     delete[] this_contactsmember->contact_name;
  //     delete this_contactsmember;
  //   }

  //   // Next contact group.
  //   next_contactgroup = this_contactgroup->next;

  //   // Free current contact group.
  //   delete[] this_contactgroup->group_name;
  //   delete[] this_contactgroup->alias;
  //   delete this_contactgroup;
  // }
  // contactgroup_list = NULL;

  // // Free memory for the service list.
  // for (service *this_service(service_list), *next_service;
  //      this_service;
  //      this_service = next_service) {
  //   // Free memory for contact groups.
  //   for (contactgroupsmember
  //          *this_contactgroupsmember(this_service->contact_groups),
  //          *next_contactgroupsmember;
  //        this_contactgroupsmember;
  //        this_contactgroupsmember = next_contactgroupsmember) {
  //     next_contactgroupsmember = this_contactgroupsmember->next;
  //     delete[] this_contactgroupsmember->group_name;
  //     delete this_contactgroupsmember;
  //   }

  //   // Free memory for contacts.
  //   for (contactsmember
  //          *this_contactsmember(this_service->contacts),
  //          *next_contactsmember;
  //        this_contactsmember;
  //        this_contactsmember = next_contactsmember) {
  //     next_contactsmember = this_contactsmember->next;
  //     delete[] this_contactsmember->contact_name;
  //     delete this_contactsmember;
  //   }

  //   // Free memory for custom variables.
  //   for (customvariablesmember
  //          *this_customvariablesmember(this_service->custom_variables),
  //          *next_customvariablesmember;
  //        this_customvariablesmember;
  //        this_customvariablesmember = next_customvariablesmember) {
  //     next_customvariablesmember = this_customvariablesmember->next;
  //     delete[] this_customvariablesmember->variable_name;
  //     delete[] this_customvariablesmember->variable_value;
  //     delete this_customvariablesmember;
  //   }

  //   // Next service.
  //   next_service = this_service->next;

  //   // Free current service.
  //   delete[] this_service->host_name;
  //   delete[] this_service->description;
  //   delete[] this_service->display_name;
  //   delete[] this_service->service_check_command;
  //   delete[] this_service->plugin_output;
  //   delete[] this_service->long_plugin_output;
  //   delete[] this_service->perf_data;
  //   delete[] this_service->event_handler_args;
  //   delete[] this_service->check_command_args;
  //   free_objectlist(&this_service->servicegroups_ptr);
  //   delete[] this_service->notification_period;
  //   delete[] this_service->check_period;
  //   delete[] this_service->event_handler;
  //   delete[] this_service->failure_prediction_options;
  //   delete[] this_service->notes;
  //   delete[] this_service->notes_url;
  //   delete[] this_service->action_url;
  //   delete[] this_service->icon_image;
  //   delete[] this_service->icon_image_alt;
  //   delete this_service;
  // }
  // service_list = NULL;

  // // Free memory for the command list.
  // for (command *this_command(command_list), *next_command;
  //      this_command;
  //      this_command = next_command) {
  //   next_command = this_command->next;
  //   delete[] this_command->name;
  //   delete[] this_command->command_line;
  //   delete this_command;
  // }
  // command_list = NULL;

  // // Free memory for the service escalation list.
  // for (serviceescalation
  //        *this_serviceescalation(serviceescalation_list),
  //        *next_serviceescalation;
  //      this_serviceescalation;
  //      this_serviceescalation = next_serviceescalation) {
  //   // Free memory for the contact group members.
  //   for (contactgroupsmember
  //          *this_contactgroupsmember(this_serviceescalation->contact_groups),
  //          *next_contactgroupsmember;
  //        this_contactgroupsmember;
  //        this_contactgroupsmember = next_contactgroupsmember) {
  //     next_contactgroupsmember = this_contactgroupsmember->next;
  //     delete[] this_contactgroupsmember->group_name;
  //     delete this_contactgroupsmember;
  //   }

  //   // Free memory for contacts.
  //   for (contactsmember
  //          *this_contactsmember(this_serviceescalation->contacts),
  //          *next_contactsmember;
  //        this_contactsmember;
  //        this_contactsmember = next_contactsmember) {
  //     next_contactsmember = this_contactsmember->next;
  //     delete[] this_contactsmember->contact_name;
  //     delete this_contactsmember;
  //   }

  //   // Next service escalation.
  //   next_serviceescalation = this_serviceescalation->next;

  //   // Free current service escalation.
  //   delete[] this_serviceescalation->host_name;
  //   delete[] this_serviceescalation->description;
  //   delete[] this_serviceescalation->escalation_period;
  //   delete this_serviceescalation;
  // }
  // serviceescalation_list = NULL;

  // // Free memory for the service dependency list.
  // for (servicedependency
  //        *this_servicedependency(servicedependency_list),
  //        *next_servicedependency;
  //      this_servicedependency;
  //      this_servicedependency = next_servicedependency) {
  //   next_servicedependency = this_servicedependency->next;
  //   delete[] this_servicedependency->dependency_period;
  //   delete[] this_servicedependency->dependent_host_name;
  //   delete[] this_servicedependency->dependent_service_description;
  //   delete[] this_servicedependency->host_name;
  //   delete[] this_servicedependency->service_description;
  //   delete this_servicedependency;
  // }
  // servicedependency_list = NULL;

  // // Free memory for the host dependency list.
  // for (hostdependency
  //        *this_hostdependency(hostdependency_list),
  //        *next_hostdependency;
  //      this_hostdependency;
  //      this_hostdependency = next_hostdependency) {
  //   next_hostdependency = this_hostdependency->next;
  //   delete[] this_hostdependency->dependency_period;
  //   delete[] this_hostdependency->dependent_host_name;
  //   delete[] this_hostdependency->host_name;
  //   delete this_hostdependency;
  // }
  // hostdependency_list = NULL;

  // // Free memory for the host escalation list.
  // for (hostescalation
  //        *this_hostescalation(hostescalation_list),
  //        *next_hostescalation;
  //      this_hostescalation;
  //      this_hostescalation = next_hostescalation) {
  //   // Free memory for the contact group members.
  //   for (contactgroupsmember
  //          *this_contactgroupsmember(this_hostescalation->contact_groups),
  //          *next_contactgroupsmember;
  //        this_contactgroupsmember;
  //        this_contactgroupsmember = next_contactgroupsmember) {
  //     next_contactgroupsmember = this_contactgroupsmember->next;
  //     delete[] this_contactgroupsmember->group_name;
  //     delete this_contactgroupsmember;
  //   }

  //   // Free memory for contacts.
  //   for (contactsmember
  //          *this_contactsmember(this_hostescalation->contacts),
  //          *next_contactsmember;
  //        this_contactsmember;
  //        this_contactsmember = next_contactsmember) {
  //     next_contactsmember = this_contactsmember->next;
  //     delete[] this_contactsmember->contact_name;
  //     delete this_contactsmember;
  //   }

  //   // Next host escalation.
  //   next_hostescalation = this_hostescalation->next;

  //   // Free current host escalation.
  //   delete[] this_hostescalation->host_name;
  //   delete[] this_hostescalation->escalation_period;
  //   delete this_hostescalation;
  // }
  // hostescalation_list = NULL;

  // // Free object skiplists.
  // free_object_skiplists();

  return (OK);
}

// /**************************************
// *                                     *
// *    Object Modification Functions    *
// *                                     *
// **************************************/

// /**
//  *  Modify an existing command.
//  *
//  *  @param[in] name  Command name.
//  *  @param[in] value New command line.
//  *
//  *  @return OK on success.
//  */
// int modify_command(char const* name, char const* value) {
//   // Make sure we have the data we need.
//   if (!name || !name[0] || !value || !value[0]) {
//     logger(log_config_error, basic)
//       << "Error: Command name or command line is NULL";
//     return (ERROR);
//   }

//   // Find command object.
//   command* this_command(command_list);
//   while (this_command && strcmp(this_command->name, name))
//     this_command = this_command->next;
//   if (!this_command)
//     return (ERROR);

//   // Modify command.
//   delete[] this_command->command_line;
//   this_command->command_line = my_strdup(value);

//   // Notify event broker.
//   timeval tv(get_broker_timestamp(NULL));
//   broker_command_data(
//     NEBTYPE_COMMAND_UPDATE,
//     NEBFLAG_NONE,
//     NEBATTR_NONE,
//     this_command->name,
//     this_command->command_line,
//     &tv);

//   return (OK);
// }

// /**************************************
// *                                     *
// *      Object Removal Functions       *
// *                                     *
// **************************************/

// /**
//  *  Remove a command.
//  *
//  *  @param[in] this_command Command to remove.
//  *
//  *  @return 1 on successful removal.
//  */
// static int remove_command(command* this_command) {
//   // Update the command skiplist.
//   skiplist_delete_all(
//     object_skiplists[COMMAND_SKIPLIST],
//     this_command);

//   // Delete command.
//   delete[] this_command->name;
//   delete[] this_command->command_line;
//   delete this_command;

//   return (1);
// }

// /**
//  *  Remove command by ID.
//  *
//  *  @param[in] name Command name.
//  *
//  *  @return 0 if command could not be found, 1 on successful removal
//  *          2 if command is in use.
//  */
// int remove_command_by_id(char const* name) {
//   if (!name)
//     return (0);

//   // Find command object.
//   command* this_command(command_list);
//   command* prev_command(NULL);
//   while (this_command && strcmp(this_command->name, name)) {
//     prev_command = this_command;
//     this_command = this_command->next;
//   }
//   if (!this_command)
//     return (0);

//   // Check if command is used by a host.
//   for (host* hst(host_list); hst; hst = hst->next)
//     if ((hst->event_handler_ptr == this_command)
// 	|| (hst->check_command_ptr == this_command))
//       return (2);

//   // Check if command is used by a service.
//   for (service* svc(service_list); svc; svc = svc->next)
//     if ((svc->event_handler_ptr == this_command)
// 	|| (svc->check_command_ptr == this_command))
//       return (2);

//   // Check if command is used by a contact.
//   for (contact* cntct(contact_list); cntct; cntct = cntct->next) {
//     for (commandsmember* cntctsmember(cntct->host_notification_commands);
// 	 cntctsmember;
// 	 cntctsmember = cntctsmember->next)
//       if (cntctsmember->command_ptr == this_command)
// 	return (2);
//     for (commandsmember* cntctsmember(cntct->service_notification_commands);
//          cntctsmember;
//          cntctsmember = cntctsmember->next)
//       if (cntctsmember->command_ptr == this_command)
// 	return (2);
//   }

//   // Update the command list.
//   if (!prev_command)
//     command_list = this_command->next;
//   else
//     prev_command->next = this_command->next;
//   if (!this_command->next)
//     command_list_tail = prev_command;

//   // Notify event broker.
//   timeval tv(get_broker_timestamp(NULL));
//   broker_command_data(
//     NEBTYPE_COMMAND_DELETE,
//     NEBFLAG_NONE,
//     NEBATTR_NONE,
//     this_command->name,
//     this_command->command_line,
//     &tv);

//   return (remove_command(this_command));
// }

// /**
//  *  Remove contact from a contactsmember list.
//  *
//  *  @param[in,out] cntctsmember Target list.
//  *  @param[in]     cntct        Contact to remove.
//  *
//  *  @return 0 if contact was not found in list, 1 on successful removal.
//  */
// static int remove_contact_to_contactsmember(
//              contactsmember** cntctsmember,
//              contact* cntct) {
//   // Find contact.
//   contactsmember* this_item(*cntctsmember);
//   contactsmember* prev_item(NULL);
//   while (this_item && this_item->contact_ptr != cntct) {
//     prev_item = this_item;
//     this_item = this_item->next;
//   }

//   // Check we have find a contacts member.
//   if (!this_item)
//     return (0);

//   // Update list.
//   if (!prev_item)
//     *cntctsmember = this_item->next;
//   else
//     prev_item->next = this_item->next;

//   // Remove contact member.
//   delete[] this_item->contact_name;
//   delete this_item;

//   return (1);
// }

// /**
//  *  Remove a contact.
//  *
//  *  @param[in] this_contact Contact to remove.
//  *
//  *  @return 1 on successful removal.
//  */
// static int remove_contact(contact* this_contact) {
//   // Remove contact from contactgroup.
//   for (contactgroup* this_contactgroup(contactgroup_list);
//        this_contactgroup;
//        this_contactgroup = this_contactgroup->next)
//     remove_contact_to_contactsmember(
//       &this_contactgroup->members,
//       this_contact);

//   // Remove contact from host.
//   for (host* this_host(host_list);
//        this_host;
//        this_host = this_host->next)
//     remove_contact_to_contactsmember(
//       &this_host->contacts,
//       this_contact);

//   // Remove contact from service.
//   for (service* this_service(service_list);
//        this_service;
//        this_service = this_service->next)
//     remove_contact_to_contactsmember(
//       &this_service->contacts,
//       this_contact);

//   // Remove contact from hostescalation.
//   for (hostescalation* this_hostescalation(hostescalation_list);
//        this_hostescalation;
//        this_hostescalation = this_hostescalation->next)
//     remove_contact_to_contactsmember(
//       &this_hostescalation->contacts,
//       this_contact);

//   // Remove contact from serviceescalation.
//   for (serviceescalation* this_serviceescalation(serviceescalation_list);
//        this_serviceescalation;
//        this_serviceescalation = this_serviceescalation->next)
//     remove_contact_to_contactsmember(
//       &this_serviceescalation->contacts,
//       this_contact);

//   // Free memory for the host notification commands.
//   for (commandsmember
//          *this_commandsmember(this_contact->host_notification_commands),
//          *next_commandsmember;
//        this_commandsmember;
//        this_commandsmember = next_commandsmember) {
//     next_commandsmember = this_commandsmember->next;
//     if (this_commandsmember->cmd)
//       delete[] this_commandsmember->cmd;
//     delete this_commandsmember;
//   }

//   // Free memory for the service notification commands.
//   for (commandsmember
//          *this_commandsmember(this_contact->service_notification_commands),
//          *next_commandsmember;
//        this_commandsmember;
//        this_commandsmember = next_commandsmember) {
//     next_commandsmember = this_commandsmember->next;
//     if (this_commandsmember->cmd)
//       delete[] this_commandsmember->cmd;
//     delete this_commandsmember;
//   }

//   // Free memory for custom variables.
//   for (customvariablesmember
//          *this_customvariablesmember(this_contact->custom_variables),
//          *next_customvariablesmember;
//        this_customvariablesmember;
//        this_customvariablesmember = next_customvariablesmember) {
//     next_customvariablesmember = this_customvariablesmember->next;
//     delete[] this_customvariablesmember->variable_name;
//     delete[] this_customvariablesmember->variable_value;
//     delete this_customvariablesmember;
//   }

//   // Update the contact skiplist.
//   skiplist_delete_all(object_skiplists[CONTACT_SKIPLIST], this_contact);

//   // Update the object list.
//   free_objectlist(&this_contact->contactgroups_ptr);

//   // Free remaining properties.
//   for (unsigned int i(0); i < MAX_CONTACT_ADDRESSES; ++i)
//     delete[] this_contact->address[i];
//   delete[] this_contact->name;
//   delete[] this_contact->alias;
//   delete[] this_contact->email;
//   delete[] this_contact->pager;
//   delete[] this_contact->host_notification_period;
//   delete[] this_contact->service_notification_period;
//   delete this_contact;

//   return (1);
// }

// /**
//  *  Remove contact by ID.
//  *
//  *  @param[in] name Contact name.
//  *
//  *  @return 0 if contact could not be found and 1 on successful removal.
//  */
// int remove_contact_by_id(char const* name) {
//   if (!name)
//     return (0);

//   // Find contact.
//   contact* this_contact(contact_list);
//   contact* prev_contact(NULL);
//   while (this_contact && strcmp(this_contact->name, name)) {
//     prev_contact = this_contact;
//     this_contact = this_contact->next;
//   }

//   // check we have find a contact.
//   if (!this_contact)
//     return (0);

//   // Update the contact list.
//   if (!prev_contact)
//     contact_list = this_contact->next;
//   else
//     prev_contact->next = this_contact->next;
//   if (!this_contact->next)
//     contact_list_tail = prev_contact;

//   // Notify event broker.
//   timeval tv(get_broker_timestamp(NULL));
//   broker_adaptive_contact_data(
//     NEBTYPE_CONTACT_DELETE,
//     NEBFLAG_NONE,
//     NEBATTR_NONE,
//     this_contact,
//     CMD_NONE,
//     MODATTR_ALL,
//     MODATTR_ALL,
//     MODATTR_ALL,
//     MODATTR_ALL,
//     MODATTR_ALL,
//     MODATTR_ALL,
//     &tv);

//   return (remove_contact(this_contact));
// }

// /**
//  *  Remove a contactgroup from a contactgroupsmember list.
//  *
//  *  @param[in,out] groupsmember Target list.
//  *  @param[in]     group        Group to remove.
//  *
//  *  @return 1 on successful removal.
//  */
// static int remove_contactgroup_to_contactgroupsmember(
//              contactgroupsmember** groupsmember,
//              contactgroup* group) {
//   // Find contact group member.
//   contactgroupsmember* this_item(*groupsmember);
//   contactgroupsmember* prev_item(NULL);
//   while (this_item && (this_item->group_ptr != group)) {
//     prev_item = this_item;
//     this_item = this_item->next;
//   }

//   // Check we have find a contact groups member.
//   if (!this_item)
//     return (0);

//   // Update list.
//   if (!prev_item)
//     *groupsmember = this_item->next;
//   else
//     prev_item->next = this_item->next;

//   // Remove member.
//   delete[] this_item->group_name;
//   delete this_item;

//   return (1);
// }

// /**
//  *  Remove contact group.
//  *
//  *  @param[in] this_contactgroup Contact group to remove.
//  *
//  *  @return 1 on successful removal.
//  */
// static int remove_contactgroup(contactgroup* this_contactgroup) {
//   // Check we have find a contact group.
//   if (!this_contactgroup)
//     return (0);

//   // Remove contactgroup from host.
//   for (host* this_host(host_list);
//        this_host;
//        this_host = this_host->next)
//     remove_contactgroup_to_contactgroupsmember(
//       &this_host->contact_groups,
//       this_contactgroup);

//   // Remove contactgroup from service.
//   for (service* this_service(service_list);
//        this_service;
//        this_service = this_service->next)
//     remove_contactgroup_to_contactgroupsmember(
//       &this_service->contact_groups,
//       this_contactgroup);

//   // Remove contactgroup from hostescalation.
//   for (hostescalation* this_hostescalation(hostescalation_list);
//        this_hostescalation;
//        this_hostescalation = this_hostescalation->next)
//     remove_contactgroup_to_contactgroupsmember(
//       &this_hostescalation->contact_groups,
//       this_contactgroup);

//   // Remove contactgroup from serviceescalation.
//   for (serviceescalation* this_serviceescalation(serviceescalation_list);
//        this_serviceescalation;
//        this_serviceescalation = this_serviceescalation->next)
//     remove_contactgroup_to_contactgroupsmember(
//       &this_serviceescalation->contact_groups,
//       this_contactgroup);

//    // Update the contactgroup skiplist.
//   skiplist_delete_all(
//     object_skiplists[CONTACTGROUP_SKIPLIST],
//     this_contactgroup);

//   // Free remaining contact group properties.
//   delete[] this_contactgroup->group_name;
//   delete[] this_contactgroup->alias;
//   delete this_contactgroup;

//   return (1);
// }

// /**
//  *  Remove contact group by ID.
//  *
//  *  @param[in] name Contact group name.
//  *
//  *  @return 0 if contact group could not be found, 1 on successful
//  *          removal.
//  */
// int remove_contactgroup_by_id(char const* name) {
//   if (!name)
//     return (0);

//   // Find contact group.
//   contactgroup* this_contactgroup(contactgroup_list);
//   contactgroup* prev_contactgroup(NULL);
//   while (this_contactgroup
// 	 && strcmp(this_contactgroup->group_name, name)) {
//     prev_contactgroup = this_contactgroup;
//     this_contactgroup = this_contactgroup->next;
//   }

//   // Check we found a contact group.
//   if (!this_contactgroup)
//     return (0);

//   // Update the contactgroup list.
//   if (!prev_contactgroup)
//     contactgroup_list = this_contactgroup->next;
//   else
//     prev_contactgroup->next = this_contactgroup->next;
//   if (!this_contactgroup->next)
//     contactgroup_list_tail = prev_contactgroup;

//   // Notify event broker.
//   timeval tv(get_broker_timestamp(NULL));
//   broker_group(
//     NEBTYPE_CONTACTGROUP_DELETE,
//     NEBFLAG_NONE,
//     NEBATTR_NONE,
//     this_contactgroup,
//     &tv);

//   return (remove_contactgroup(this_contactgroup));
// }

// /**
//  *  Remove host.
//  *
//  *  @param[in] this_host Host to remove.
//  *
//  *  @return 1 on successful removal.
//  */
// static int remove_host(host* this_host) {
//   // Check we have a host.
//   if (!this_host)
//     return (0);

//   // Update the event list low.
//   for (timed_event* temp_event(event_list_low);
//        temp_event;
//        temp_event = temp_event->next)
//     if (temp_event->event_data == this_host) {
//       remove_event(temp_event, &event_list_low, &event_list_low_tail);
//       delete temp_event;
//       break;
//     }

//   // Update the event list high.
//   for (timed_event* temp_event(event_list_high);
//        temp_event;
//        temp_event = temp_event->next)
//     if (temp_event->event_data == this_host) {
//       remove_event(temp_event, &event_list_high, &event_list_high_tail);
//       delete temp_event;
//       break;
//     }

//   // Update the hostdependency list.
//   for (hostdependency *dep(hostdependency_list), *prev(NULL);
//        dep;
//        dep = dep->next) {
//     if ((dep->master_host_ptr == this_host)
//         || (dep->dependent_host_ptr == this_host)) {
//       skiplist_delete(object_skiplists[HOSTDEPENDENCY_SKIPLIST], dep);
//       if (!prev)
// 	hostdependency_list = dep->next;
//       else
// 	prev->next = dep->next;
//       if (!dep->next)
// 	hostdependency_list_tail = prev;
//       delete[] dep->dependent_host_name;
//       delete[] dep->host_name;
//       delete[] dep->dependency_period;
//       delete dep;
//       break;
//     }
//     prev = dep;
//   }

//   // Update the hostescalation list.
//   for (hostescalation *escalation(hostescalation_list), *prev(NULL);
//        escalation;
//        prev = escalation, escalation = escalation->next) {
//     skiplist_delete(
//       object_skiplists[HOSTESCALATION_SKIPLIST],
//       escalation);
//     if (escalation->host_ptr == this_host) {
//       if (!prev)
// 	hostescalation_list = escalation->next;
//       else
// 	prev->next = escalation->next;
//       if (!escalation->next)
// 	hostescalation_list_tail = prev;
//       delete[] escalation->host_name;
//       delete[] escalation->escalation_period;
//       delete escalation;
//       break;
//     }
//   }

//   // Update the host skiplist.
//   skiplist_delete_all(object_skiplists[HOST_SKIPLIST], this_host);

//   // Free memory for parent hosts.
//   for (hostsmember
//          *this_hostsmember(this_host->parent_hosts),
//          *next_hostsmember;
//        this_hostsmember;
//        this_hostsmember = next_hostsmember) {
//     next_hostsmember = this_hostsmember->next;
//     delete[] this_hostsmember->host_name;
//     delete this_hostsmember;
//   }

//   // Free memory for child host links.
//   for (hostsmember
//          *this_hostsmember(this_host->child_hosts),
//          *next_hostsmember;
//        this_hostsmember;
//        this_hostsmember = next_hostsmember) {
//     next_hostsmember = this_hostsmember->next;
//     delete[] this_hostsmember->host_name;
//     delete this_hostsmember;
//   }

//   // Free memory for service links.
//   for (servicesmember
//          *this_servicesmember(this_host->services),
//          *next_servicesmember;
//        this_servicesmember;
//        this_servicesmember = next_servicesmember) {
//     next_servicesmember = this_servicesmember->next;
//     service* svc(this_servicesmember->service_ptr);
//     delete[] this_servicesmember->host_name;
//     delete[] this_servicesmember->service_description;
//     delete this_servicesmember;
//     remove_service_by_id(svc->host_name, svc->description);
//   }

//   // Free memory for contact groups.
//   for (contactgroupsmember
//          *this_contactgroupsmember(this_host->contact_groups),
//          *next_contactgroupsmember;
//        this_contactgroupsmember;
//        this_contactgroupsmember = next_contactgroupsmember) {
//     next_contactgroupsmember = this_contactgroupsmember->next;
//     delete[] this_contactgroupsmember->group_name;
//     delete this_contactgroupsmember;
//   }

//   // Free memory for contacts.
//   for (contactsmember
//          *this_contactsmember(this_host->contacts),
//          *next_contactsmember;
//        this_contactsmember;
//        this_contactsmember = next_contactsmember) {
//     next_contactsmember = this_contactsmember->next;
//     delete[] this_contactsmember->contact_name;
//     delete this_contactsmember;
//   }

//   // Free memory for custom variables.
//   for (customvariablesmember
//          *this_customvariablesmember(this_host->custom_variables),
//          *next_customvariablesmember;
//        this_customvariablesmember;
//        this_customvariablesmember = next_customvariablesmember) {
//     next_customvariablesmember = this_customvariablesmember->next;
//     delete[] this_customvariablesmember->variable_name;
//     delete[] this_customvariablesmember->variable_value;
//     delete this_customvariablesmember;
//   }

//   // Free remaining host properties.
//   delete[] this_host->name;
//   delete[] this_host->display_name;
//   delete[] this_host->alias;
//   delete[] this_host->address;
//   delete[] this_host->plugin_output;
//   delete[] this_host->long_plugin_output;
//   delete[] this_host->perf_data;
//   free_objectlist(&this_host->hostgroups_ptr);
//   delete[] this_host->check_period;
//   delete[] this_host->host_check_command;
//   delete[] this_host->event_handler;
//   delete[] this_host->failure_prediction_options;
//   delete[] this_host->notification_period;
//   delete[] this_host->notes;
//   delete[] this_host->notes_url;
//   delete[] this_host->action_url;
//   delete[] this_host->icon_image;
//   delete[] this_host->icon_image_alt;
//   delete[] this_host->vrml_image;
//   delete[] this_host->statusmap_image;
//   delete this_host;

//   return (1);
// }

// /**
//  *  Remove host by ID.
//  *
//  *  @param[in] name Host name.
//  *
//  *  @return 1 on successful removal.
//  */
// int remove_host_by_id(char const* name) {
//   if (!name)
//     return (0);

//   // Find host.
//   host* this_host(host_list);
//   host* prev_host(NULL);
//   while (this_host && strcmp(this_host->name, name)) {
//     prev_host = this_host;
//     this_host = this_host->next;
//   }

//   // Check we have find a host.
//   if (!this_host)
//     return (0);

//   // Update the host list.
//   if (!prev_host)
//     host_list = this_host->next;
//   else
//     prev_host->next = this_host->next;
//   if (!this_host->next)
//     host_list_tail = prev_host;

//   return (remove_host(this_host));
// }

// /**
//  *  Remove host dependency by ID.
//  *
//  *  @param[in] host_name           Host name.
//  *  @param[in] dependent_host_name Dependant host name.
//  *
//  *  @return 0 on successful removal.
//  */
// int remove_host_dependency_by_id(
//       char const* host_name,
//       char const* dependent_host_name) {
//   if (!host_name || !dependent_host_name)
//      return (0);

//   // Find host dependency.
//   hostdependency* this_hostdependency(hostdependency_list);
//   hostdependency* prev_hostdependency(NULL);
//   while (this_hostdependency
// 	 && strcmp(this_hostdependency->host_name, host_name)
// 	 && strcmp(
//               this_hostdependency->dependent_host_name,
//               dependent_host_name)) {
//     prev_hostdependency = this_hostdependency;
//     this_hostdependency = this_hostdependency->next;
//   }

//   // Check we have find a hostdependency.
//   if (!this_hostdependency)
//     return (0);

//   // Update the hostdependency list.
//   if (!prev_hostdependency)
//     hostdependency_list = this_hostdependency->next;
//   else
//     prev_hostdependency->next = this_hostdependency->next;
//   if (!this_hostdependency->next)
//     hostdependency_list_tail = prev_hostdependency;

//   // Update the hostdependency skiplist.
//   skiplist_delete_all(
//     object_skiplists[HOSTDEPENDENCY_SKIPLIST],
//     this_hostdependency);

//   // Free dependency.
//   delete[] this_hostdependency->dependent_host_name;
//   delete[] this_hostdependency->host_name;
//   delete[] this_hostdependency->dependency_period;
//   delete this_hostdependency;

//   return (1);
// }

// /**
//  *  Remove host escalation by ID.
//  *
//  *  @param[in] host_name Host name.
//  *
//  *  @return 1 on successful removal.
//  */
// int remove_host_escalation_by_id(char const* host_name) {
//   if (!host_name)
//     return (0);

//   // Find host escalation.
//   hostescalation* this_hostescalation(hostescalation_list);
//   hostescalation* prev_hostescalation(NULL);
//   while (this_hostescalation
// 	 && strcmp(this_hostescalation->host_name, host_name)) {
//     prev_hostescalation = this_hostescalation;
//     this_hostescalation = this_hostescalation->next;
//   }

//   // Check we have find a hostescalation.
//   if (!this_hostescalation)
//     return (0);

//   // Update the hostescalation list.
//   if (!prev_hostescalation)
//     hostescalation_list = this_hostescalation->next;
//   else
//     prev_hostescalation->next = this_hostescalation->next;
//   if (!this_hostescalation->next)
//     hostescalation_list_tail = prev_hostescalation;

//   for (contactgroupsmember
//          *this_contactgroupsmembers(this_hostescalation->contact_groups),
//          *next_contactgroupsmembers;
//        this_contactgroupsmembers;
//        this_contactgroupsmembers = next_contactgroupsmembers) {
//     next_contactgroupsmembers = this_contactgroupsmembers->next;
//     delete[] this_contactgroupsmembers->group_name;
//     delete this_contactgroupsmembers;
//   }

//   for (contactsmember
//          *this_contactsmember(this_hostescalation->contacts),
//          *next_contactsmember;
//        this_contactsmember;
//        this_contactsmember = next_contactsmember) {
//     next_contactsmember = this_contactsmember->next;
//     delete[] this_contactsmember->contact_name;
//     delete this_contactsmember;
//   }

//   // Update the hostescalation skiplist.
//   skiplist_delete_all(
//     object_skiplists[HOSTESCALATION_SKIPLIST],
//     this_hostescalation);

//   delete[] this_hostescalation->host_name;
//   delete[] this_hostescalation->escalation_period;
//   delete this_hostescalation;

//   return (1);
// }

// /**
//  *  Remove host group.
//  *
//  *  @param[in] this_hostgroup Host group to remove.
//  *
//  *  @return 1 on successful removal.
//  */
// static int remove_host_group(hostgroup* this_hostgroup) {
//   // Update host list.
//   for (host* hst = host_list; hst; hst = hst->next)
//     remove_object_to_objectlist(&hst->hostgroups_ptr, this_hostgroup);

//   // Remove members.
//   for (hostsmember
//          *this_hostsmember(this_hostgroup->members),
//          *next_hostsmember;
//        this_hostsmember;
//        this_hostsmember = next_hostsmember) {
//     next_hostsmember = this_hostsmember->next;
//     delete[] this_hostsmember->host_name;
//     delete this_hostsmember;
//   }

//   // Update the hostgroup skiplist.
//   skiplist_delete_all(
//     object_skiplists[HOSTGROUP_SKIPLIST],
//     this_hostgroup);

//   delete[] this_hostgroup->group_name;
//   delete[] this_hostgroup->alias;
//   delete[] this_hostgroup->notes;
//   delete[] this_hostgroup->notes_url;
//   delete[] this_hostgroup->action_url;
//   delete this_hostgroup;

//   return (1);
// }

// /**
//  *  Remove host group by ID.
//  *
//  *  @param[in] name Host group name.
//  *
//  *  @return 1 on successful removal.
//  */
// int remove_hostgroup_by_id(char const* name) {
//   if (!name)
//     return (0);

//   // Find host group.
//   hostgroup* this_hostgroup(hostgroup_list);
//   hostgroup* prev_hostgroup(NULL);
//   while (this_hostgroup && strcmp(this_hostgroup->group_name, name)) {
//     prev_hostgroup = this_hostgroup;
//     this_hostgroup = this_hostgroup->next;
//   }

//   // Check we have find a host group.
//   if (!this_hostgroup)
//     return (0);

//   // Update the hostgroup list.
//   if (!prev_hostgroup)
//     hostgroup_list = this_hostgroup->next;
//   else
//     prev_hostgroup->next = this_hostgroup->next;
//   if (!this_hostgroup->next)
//     hostgroup_list_tail = prev_hostgroup;

//   return (remove_host_group(this_hostgroup));
// }

// /**
//  *  Remove service.
//  *
//  *  @param[in] this_service Service to remove.
//  *
//  *  @return 1 on successful removal.
//  */
// static int remove_service(service* this_service) {
//   // Check we have a service.
//   if (!this_service)
//     return (0);

//   // Update host service list.
//   host* hst(find_host(this_service->host_name));
//   if (hst) {
//     for (servicesmember *svcmbr(hst->services), *prev(NULL);
// 	 svcmbr;
// 	 svcmbr = svcmbr->next) {
//       if (svcmbr->service_ptr == this_service) {
// 	if (!prev)
// 	  hst->services = svcmbr->next;
// 	else
// 	  prev->next = svcmbr->next;
// 	delete[] svcmbr->host_name;
// 	delete[] svcmbr->service_description;
// 	delete svcmbr;
// 	break;
//       }
//       prev = svcmbr;
//     }
//   }

//   // Update the event list low.
//   for (timed_event* temp_event(event_list_low);
//        temp_event;
//        temp_event = temp_event->next)
//     if (temp_event->event_data == this_service) {
//       remove_event(temp_event, &event_list_low, &event_list_low_tail);
//       delete temp_event;
//       break;
//     }

//   // Update the event list high.
//   for (timed_event* temp_event(event_list_high);
//        temp_event;
//        temp_event = temp_event->next)
//     if (temp_event->event_data == this_service) {
//       remove_event(temp_event, &event_list_high, &event_list_high_tail);
//       delete temp_event;
//       break;
//     }

//   // Update the servicedependency list.
//   for (servicedependency
//          *dep(servicedependency_list),
//          *prev(NULL);
//        dep;
//        dep = dep->next) {
//     if ((dep->master_service_ptr == this_service)
//         || (dep->dependent_service_ptr == this_service)) {
//       skiplist_delete(
//         object_skiplists[SERVICEDEPENDENCY_SKIPLIST],
//         dep);
//       if (!prev)
// 	servicedependency_list = dep->next;
//       else
// 	prev->next = dep->next;
//       if (!dep->next)
// 	servicedependency_list_tail = prev;
//       delete[] dep->dependent_host_name;
//       delete[] dep->dependent_service_description;
//       delete[] dep->host_name;
//       delete[] dep->service_description;
//       delete[] dep->dependency_period;
//       delete dep;
//       break;
//     }
//     prev = dep;
//   }

//   // Update the serviceescalation list.
//   for (serviceescalation
//          *escalation(serviceescalation_list),
//          *prev(NULL);
//        escalation;
//        prev = escalation, escalation = escalation->next) {
//     skiplist_delete(
//       object_skiplists[SERVICEESCALATION_SKIPLIST],
//       escalation);
//     if (escalation->service_ptr == this_service) {
//       if (!prev)
// 	serviceescalation_list = escalation->next;
//       else
// 	prev->next = escalation->next;
//       if (!escalation->next)
// 	serviceescalation_list_tail = prev;
//       delete[] escalation->host_name;
//       delete[] escalation->description;
//       delete[] escalation->escalation_period;
//       delete escalation;
//       break;
//     }
//   }

//   // Update the service skiplist.
//   skiplist_delete_all(object_skiplists[SERVICE_SKIPLIST], this_service);

//   // Free memory for contact groups.
//   for (contactgroupsmember
//          *this_contactgroupsmember(this_service->contact_groups),
//          *next_contactgroupsmember;
//        this_contactgroupsmember;
//        this_contactgroupsmember = next_contactgroupsmember) {
//     next_contactgroupsmember = this_contactgroupsmember->next;
//     delete[] this_contactgroupsmember->group_name;
//     delete this_contactgroupsmember;
//   }

//   // Free memory for contacts.
//   for (contactsmember
//          *this_contactsmember(this_service->contacts),
//          *next_contactsmember;
//        this_contactsmember;
//        this_contactsmember = next_contactsmember) {
//     next_contactsmember = this_contactsmember->next;
//     delete[] this_contactsmember->contact_name;
//     delete this_contactsmember;
//   }

//   // Free memory for custom variables.
//   for (customvariablesmember
//          *this_customvariablesmember(this_service->custom_variables),
//          *next_customvariablesmember;
//        this_customvariablesmember;
//        this_customvariablesmember = next_customvariablesmember) {
//     next_customvariablesmember = this_customvariablesmember->next;
//     delete[] this_customvariablesmember->variable_name;
//     delete[] this_customvariablesmember->variable_value;
//     delete this_customvariablesmember;
//   }

//   // Cleanup memory.
//   delete[] this_service->host_name;
//   delete[] this_service->description;
//   delete[] this_service->display_name;
//   delete[] this_service->service_check_command;
//   delete[] this_service->plugin_output;
//   delete[] this_service->long_plugin_output;
//   delete[] this_service->perf_data;
//   delete[] this_service->event_handler_args;
//   delete[] this_service->check_command_args;
//   free_objectlist(&this_service->servicegroups_ptr);
//   delete[] this_service->notification_period;
//   delete[] this_service->check_period;
//   delete[] this_service->event_handler;
//   delete[] this_service->failure_prediction_options;
//   delete[] this_service->notes;
//   delete[] this_service->notes_url;
//   delete[] this_service->action_url;
//   delete[] this_service->icon_image;
//   delete[] this_service->icon_image_alt;
//   delete this_service;

//   return (1);
// }

// /**
//  *  Remove service by ID.
//  *
//  *  @param[in] host_name   Host name.
//  *  @param[in] description Service description.
//  *
//  *  @return 1 on successful removal.
//  */
// int remove_service_by_id(
//       char const* host_name,
//       char const* description) {
//   if (!host_name || !description)
//     return (0);

//   // Find service.
//   service* this_service(service_list);
//   service* prev_service(NULL);
//   while (this_service) {
//     if (!strcmp(this_service->host_name, host_name)
// 	&& !strcmp(this_service->description, description))
//       break;
//     prev_service = this_service;
//     this_service = this_service->next;
//   }

//   // Check we have find a service.
//   if (!this_service)
//     return (0);

//   // Remove service from host list.
//   if (this_service->host_ptr) {
//     servicesmember* current;
//     servicesmember** prev;
//     for (current = this_service->host_ptr->services,
//            prev = &this_service->host_ptr->services;
//          current;
//          prev = &current->next, current = current->next)
//       if (current->service_ptr == this_service)
//         break;
//     if (current) {
//       *prev = current->next;
//       delete[] current->host_name;
//       delete[] current->service_description;
//       delete current;
//     }
//   }

//   // Update the service list.
//   if (!prev_service)
//     service_list = this_service->next;
//   else
//     prev_service->next = this_service->next;
//   if (!this_service->next)
//     service_list_tail = prev_service;

//   return (remove_service(this_service));
// }

// /**
//  *  Remove a service dependency.
//  *
//  *  @param[in] host_name                     Host name.
//  *  @param[in] service_description           Service description.
//  *  @param[in] dependent_host_name           Dependent host name.
//  *  @param[in] dependent_service_description Dependent service
//  *                                           description.
//  *
//  *  @return 1 on successful removal.
//  */
// int remove_service_dependency_by_id(
//       char const* host_name,
//       char const* service_description,
//       char const* dependent_host_name,
//       char const* dependent_service_description) {
//   if (!host_name
//       || !service_description
//       || !dependent_host_name
//       || !dependent_service_description)
//     return (0);

//   // Find service dependency.
//   servicedependency* this_servicedependency(servicedependency_list);
//   servicedependency* prev_servicedependency(NULL);
//   while (this_servicedependency
// 	 && strcmp(this_servicedependency->host_name, host_name)
// 	 && strcmp(
//               this_servicedependency->service_description,
//               service_description)
// 	 && strcmp(
//               this_servicedependency->dependent_host_name,
//               dependent_host_name)
// 	 && strcmp(
//               this_servicedependency->dependent_service_description,
//               dependent_service_description)) {
//     prev_servicedependency = this_servicedependency;
//     this_servicedependency = this_servicedependency->next;
//   }

//   // Check we have find a servicedependency.
//   if (!this_servicedependency)
//     return (0);

//   // Update the servicedependency list.
//   if (!prev_servicedependency)
//     servicedependency_list = this_servicedependency->next;
//   else
//     prev_servicedependency->next = this_servicedependency->next;
//   if (!this_servicedependency->next)
//     servicedependency_list_tail = prev_servicedependency;

//   // Update the servicedependency skiplist.
//   skiplist_delete_all(
//     object_skiplists[SERVICEDEPENDENCY_SKIPLIST],
//     this_servicedependency);

//   // Free service dependency.
//   delete[] this_servicedependency->dependent_host_name;
//   delete[] this_servicedependency->dependent_service_description;
//   delete[] this_servicedependency->host_name;
//   delete[] this_servicedependency->service_description;
//   delete[] this_servicedependency->dependency_period;
//   delete this_servicedependency;

//   return (1);
// }

// /**
//  *  Remove service escalation by ID.
//  *
//  *  @param[in] host_name           Host name.
//  *  @param[in] service_description Service description.
//  *
//  *  @return 1 on successful removal.
//  */
// int remove_service_escalation_by_id(
//       char const* host_name,
//       char const* service_description) {
//   if (!host_name || !service_description)
//     return (0);

//   // Find service escalation.
//   serviceescalation* this_serviceescalation(serviceescalation_list);
//   serviceescalation* prev_serviceescalation(NULL);
//   while (this_serviceescalation
// 	 && strcmp(this_serviceescalation->host_name, host_name)
// 	 && strcmp(
//               this_serviceescalation->description,
//               service_description)) {
//     prev_serviceescalation = this_serviceescalation;
//     this_serviceescalation = this_serviceescalation->next;
//   }

//   // Check we have find a serviceescalation.
//   if (!this_serviceescalation)
//     return (0);

//   // Update the serviceescalation list.
//   if (!prev_serviceescalation)
//     serviceescalation_list = this_serviceescalation->next;
//   else
//     prev_serviceescalation->next = this_serviceescalation->next;
//   if (!this_serviceescalation->next)
//     serviceescalation_list_tail = prev_serviceescalation;

//   // Delete contact group members.
//   for (contactgroupsmember
//          *this_contactgroupsmembers(this_serviceescalation->contact_groups),
//          *next_contactgroupsmembers;
//        this_contactgroupsmembers;
//        this_contactgroupsmembers = next_contactgroupsmembers) {
//     next_contactgroupsmembers = this_contactgroupsmembers->next;
//     delete[] this_contactgroupsmembers->group_name;
//     delete this_contactgroupsmembers;
//   }

//   // Delete contact members.
//   for (contactsmember
//          *this_contactsmember(this_serviceescalation->contacts),
//          *next_contactsmember;
//        this_contactsmember;
//        this_contactsmember = next_contactsmember) {
//     next_contactsmember = this_contactsmember->next;
//     delete[] this_contactsmember->contact_name;
//     delete this_contactsmember;
//   }

//   // Update the serviceescalation skiplist.
//   skiplist_delete_all(
//     object_skiplists[SERVICEESCALATION_SKIPLIST],
//     this_serviceescalation);

//   // Free service escalation.
//   delete[] this_serviceescalation->host_name;
//   delete[] this_serviceescalation->description;
//   delete[] this_serviceescalation->escalation_period;
//   delete this_serviceescalation;

//   return (1);
// }

// /**
//  *  Remove service group.
//  *
//  *  @param[in] this_servicegroup Service group to remove.
//  *
//  *  @return 1 on successful removal.
//  */
// static int remove_service_group(servicegroup* this_servicegroup) {
//   // Update service list.
//   for (service* svc(service_list); svc; svc = svc->next)
//     remove_object_to_objectlist(
//       &svc->servicegroups_ptr,
//       this_servicegroup);

//   // Delete service members.
//   for (servicesmember
//          *this_servicesmember(this_servicegroup->members),
//          *next_servicesmember;
//        this_servicesmember;
//        this_servicesmember = next_servicesmember) {
//     next_servicesmember = this_servicesmember->next;
//     delete[] this_servicesmember->host_name;
//     delete[] this_servicesmember->service_description;
//     delete this_servicesmember;
//   }

//   // Update the servicegroup skiplist.
//   skiplist_delete_all(
//     object_skiplists[SERVICEGROUP_SKIPLIST],
//     this_servicegroup);

//   // Delete service group.
//   delete[] this_servicegroup->group_name;
//   delete[] this_servicegroup->alias;
//   delete[] this_servicegroup->notes;
//   delete[] this_servicegroup->notes_url;
//   delete[] this_servicegroup->action_url;
//   delete this_servicegroup;

//   return (1);
// }

// /**
//  *  Remove service group by ID.
//  *
//  *  @param[in] name Name of service group to remove.
//  *
//  *  @return 1 on successful removal.
//  */
// int remove_servicegroup_by_id(char const* name) {
//   if (!name)
//     return (0);

//   // Find service group.
//   servicegroup* this_servicegroup(servicegroup_list);
//   servicegroup* prev_servicegroup(NULL);
//   while (this_servicegroup
//          && strcmp(this_servicegroup->group_name, name)) {
//     prev_servicegroup = this_servicegroup;
//     this_servicegroup = this_servicegroup->next;
//   }

//   // Check we have find a service group.
//   if (!this_servicegroup)
//     return (0);

//   // Update the servicegroup list.
//   if (!prev_servicegroup)
//     servicegroup_list = this_servicegroup->next;
//   else
//     prev_servicegroup->next = this_servicegroup->next;
//   if (!this_servicegroup->next)
//     servicegroup_list_tail = prev_servicegroup;

//   return (remove_service_group(this_servicegroup));
// }

// /**
//  *  Remove a timerange.
//  *
//  *  @param[in] this_timerange Timerange to remove.
//  *
//  *  @return 1.
//  */
// static int remove_timerange(timerange* this_timerange) {
//   while (this_timerange) {
//     timerange* tmp(this_timerange->next);
//     delete this_timerange;
//     this_timerange = tmp;
//   }
//   return (1);
// }

// /**
//  *  Remove a daterange.
//  *
//  *  @param[in] this_daterange Daterange to remove.
//  *
//  *  @return 1.
//  */
// static int remove_daterange(daterange* this_daterange) {
//   while (this_daterange) {
//     daterange* tmp(this_daterange->next);
//     remove_timerange(this_daterange->times);
//     delete this_daterange;
//     this_daterange = tmp;
//   }
//   return (1);
// }

// /**
//  *  Remove a timeperiod exclusion.
//  *
//  *  @param[in] this_timeperiodexclusion Timeperiod exclusion to remove.
//  *
//  *  @return 1.
//  */
// static int remove_timeperiodexclusions(
//              timeperiodexclusion* this_timeperiodexclusion) {
//   while (this_timeperiodexclusion) {
//     timeperiodexclusion* tmp(this_timeperiodexclusion->next);
//     delete[] this_timeperiodexclusion->timeperiod_name;
//     delete this_timeperiodexclusion;
//     this_timeperiodexclusion = tmp;
//   }
//   return (1);
// }

// /**
//  *  Remove a timeperiod.
//  *
//  *  @param[in] this_timeperiod Timeperiod to remove.
//  *
//  *  @return 1 on successful removal.
//  */
// static int remove_timeperiod(timeperiod* this_timeperiod) {
//   if (!this_timeperiod)
//     return (0);

//   // Remove all timerange.
//   for (unsigned int i(0);
//        i < sizeof(this_timeperiod->days) / sizeof(*this_timeperiod->days);
//        ++i)
//     remove_timerange(this_timeperiod->days[i]);

//   // Remove all exceptions.
//   for (unsigned int i(0);
//        i < sizeof(this_timeperiod->exceptions) / sizeof(*this_timeperiod->exceptions);
//        ++i)
//     remove_daterange(this_timeperiod->exceptions[i]);

//   // Remove all exclusions.
//   remove_timeperiodexclusions(this_timeperiod->exclusions);

//   // Remove all timeperiod used by contacts.
//   for (contact* cntct(contact_list); cntct; cntct = cntct->next) {
//     if (cntct->host_notification_period_ptr == this_timeperiod)
//       cntct->host_notification_period_ptr = NULL;
//     if (cntct->service_notification_period_ptr == this_timeperiod)
//       cntct->service_notification_period_ptr = NULL;
//   }

//   // Remove all timeperiod used by hosts.
//   for (host* hst(host_list); hst; hst = hst->next) {
//     if (hst->check_period_ptr == this_timeperiod)
//       hst->check_period_ptr = NULL;
//     if (hst->notification_period_ptr == this_timeperiod)
//       hst->notification_period_ptr = NULL;
//   }

//   // Remove all timeperiod used by services.
//   for (service* svc(service_list); svc; svc = svc->next) {
//     if (svc->check_period_ptr == this_timeperiod)
//       svc->check_period_ptr = NULL;
//     if (svc->notification_period_ptr == this_timeperiod)
//       svc->notification_period_ptr = NULL;
//   }

//   // Remove all timeperiod used by serviceescalations.
//   for (serviceescalation* se(serviceescalation_list);
//        se;
//        se = se->next)
//     if (se->escalation_period_ptr == this_timeperiod)
//       se->escalation_period_ptr = NULL;

//   // Remove all timeperiod used by servicedependencies.
//   for (servicedependency* sd(servicedependency_list);
//        sd;
//        sd = sd->next)
//     if (sd->dependency_period_ptr == this_timeperiod)
//       sd->dependency_period_ptr = NULL;

//   // Remove all timeperiod used by hostescalations.
//   for (hostescalation* he(hostescalation_list); he; he = he->next)
//     if (he->escalation_period_ptr == this_timeperiod)
//       he->escalation_period_ptr = NULL;

//   // Remove all timeperiod used by hostdependencies.
//   for (hostdependency* hd(hostdependency_list); hd; hd = hd->next)
//     if (hd->dependency_period_ptr == this_timeperiod)
//       hd->dependency_period_ptr = NULL;

//   // Update the timeperiod skiplist.
//   skiplist_delete_all(
//     object_skiplists[TIMEPERIOD_SKIPLIST],
//     this_timeperiod);

//   // Remove timeperiod properties.
//   delete[] this_timeperiod->name;
//   delete[] this_timeperiod->alias;
//   delete this_timeperiod;

//   return (1);
// }

// /**
//  *  Remove timeperiod by ID.
//  *
//  *  @param[in] name Timeperiod name.
//  *
//  *  @return 1 on successful timeperiod removal.
//  */
// int remove_timeperiod_by_id(char const* name) {
//   if (!name)
//     return (0);

//   // Find timeperiod.
//   timeperiod* this_timeperiod(timeperiod_list);
//   timeperiod* prev_timeperiod(NULL);
//   while (this_timeperiod
// 	 && strcmp(this_timeperiod->name, name)) {
//     prev_timeperiod = this_timeperiod;
//     this_timeperiod = this_timeperiod->next;
//   }

//   // Check we have find a timeperiod.
//   if (!this_timeperiod)
//     return (0);

//   // Update the timeperiod list.
//   if (!prev_timeperiod)
//     timeperiod_list = this_timeperiod->next;
//   else
//     prev_timeperiod->next = this_timeperiod->next;
//   if (!this_timeperiod->next)
//     timeperiod_list_tail = prev_timeperiod;

//   return (remove_timeperiod(this_timeperiod));
// }

/**************************************
*                                     *
*           Count Functions           *
*                                     *
**************************************/

/**
 *  Get number of registered hosts.
 *
 *  @return Number of registered hosts.
 */
int get_host_count() {
  return (state::instance().hosts().size());
}

/**
 *  Get number of registered services.
 *
 *  @return Number of registered services.
 */
int get_service_count() {
  return (state::instance().services().size());
}

/**
 *  Returns a count of the immediate children for a given host.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] hst Target host.
 *
 *  @return Number of immediate child hosts.
 */
int number_of_immediate_child_hosts(host* hst) {
  int children(0);
  for (host* tmp(host_list); tmp; tmp = tmp->next)
    if (is_host_immediate_child_of_host(hst, tmp))
      ++children;
  return (children);
}

/**
 *  Get the number of immediate parent hosts for a given host.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] hst Target host.
 *
 *  @return Number of immediate parent hosts.
 */
int number_of_immediate_parent_hosts(host* hst) {
  int parents(0);
  for (host* tmp(host_list); tmp; tmp = tmp->next)
    if (is_host_immediate_parent_of_host(hst, tmp))
      ++parents;
  return (parents);
}

/**
 *  Returns a count of the total children for a given host.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] hst Target host.
 *
 *  @return Number of total child hosts.
 */
int number_of_total_child_hosts(host* hst) {
  int children(0);
  for (host* tmp(host_list); tmp; tmp = tmp->next)
    if (is_host_immediate_child_of_host(hst, tmp))
      children += number_of_total_child_hosts(tmp) + 1;
  return (children);
}

/**
 *  Get the total number of parent hosts for a given host.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] hst Target host.
 *
 *  @return Number of total parent hosts.
 */
int number_of_total_parent_hosts(host* hst) {
  int parents(0);
  for (host* tmp(host_list); tmp; tmp = tmp->next)
    if (is_host_immediate_parent_of_host(hst, tmp))
      parents += number_of_total_parent_hosts(tmp) + 1;
  return (parents);
}

/**************************************
*                                     *
*     Object Traversal Functions      *
*                                     *
**************************************/

/**
 *  Find first host dependency from dependent host.
 *
 *  @param[in]  host_name Host name.
 *  @param[out] ptr       Set to the container node.
 *
 *  @return Host dependency object if found, NULL otherwise.
 *
 *  @see get_next_hostdependency_by_dependent_host
 */
hostdependency* get_first_host_dependency_by_dependent_host(
                  char const* host_name,
                  void** ptr) {
  if (!host_name)
    return (NULL);

  hostdependency temp_hostdependency;
  temp_hostdependency.dependent_host_name = const_cast<char*>(host_name);
  return (static_cast<hostdependency*>(
            skiplist_find_first(
              object_skiplists[HOSTDEPENDENCY_SKIPLIST],
              &temp_hostdependency,
              ptr)));
}

/**
 *  Find first host escalation of host.
 *
 *  @param[in]  host_name Host name.
 *  @param[out] ptr       Set to the container node.
 *
 *  @return Host escalation object if found, NULL otherwise.
 *
 *  @see get_next_host_escalation_by_host
 */
hostescalation* get_first_host_escalation_by_host(
                  char const* host_name,
                  void** ptr) {
  if (!host_name)
    return (NULL);

  hostescalation temp_hostescalation;
  temp_hostescalation.host_name = const_cast<char*>(host_name);
  return (static_cast<hostescalation*>(
            skiplist_find_first(
              object_skiplists[HOSTESCALATION_SKIPLIST],
              &temp_hostescalation,
              ptr)));
}

/**
 *  Find first service dependency by dependent service.
 *
 *  @param[in]  host_name       Host name.
 *  @param[in]  svc_description Service description.
 *  @param[out] ptr             Set to the container node.
 *
 *  @return Service dependency object if found, NULL otherwise.
 *
 *  @see get_next_servicedependency_by_dependent_service
 */
servicedependency* get_first_service_dependency_by_dependent_service(
                     char const* host_name,
                     char const* svc_description,
                     void** ptr) {
  if (!host_name || !svc_description)
    return (NULL);

  servicedependency temp_servicedependency;
  temp_servicedependency.dependent_host_name
    = const_cast<char*>(host_name);
  temp_servicedependency.dependent_service_description
    = const_cast<char*>(svc_description);
  return (static_cast<servicedependency*>(
            skiplist_find_first(
              object_skiplists[SERVICEDEPENDENCY_SKIPLIST],
              &temp_servicedependency,
              ptr)));
}

/**
 *  Find first service escalation of service.
 *
 *  @param[in]  host_name
 *  @param[in]  svc_description
 *  @param[out] ptr             Set to the container node.
 *
 *  @return Service escalation object if found, NULL otherwise.
 *
 *  @see get_next_service_escalation_by_service
 */
serviceescalation* get_first_service_escalation_by_service(
                     char const* host_name,
                     char const* svc_description,
                     void** ptr) {
  if (!host_name || !svc_description)
    return (NULL);

  serviceescalation temp_serviceescalation;
  temp_serviceescalation.host_name = const_cast<char*>(host_name);
  temp_serviceescalation.description
    = const_cast<char*>(svc_description);
  return (static_cast<serviceescalation*>(
            skiplist_find_first(
              object_skiplists[SERVICEESCALATION_SKIPLIST],
              &temp_serviceescalation,
              ptr)));
}

/**
 *  Get next host dependency.
 *
 *  @param[in]     host_name Host name
 *  @param[in,out] ptr       Container node.
 *
 *  @return Next host dependency if found, NULL otherwise.
 *
 *  @see get_first_hostdependency_by_dependent_host
 */
hostdependency* get_next_host_dependency_by_dependent_host(
                  char const* host_name,
                  void** ptr) {
  if (!host_name || !ptr)
    return (NULL);

  hostdependency temp_hostdependency;
  temp_hostdependency.dependent_host_name
    = const_cast<char*>(host_name);
  return (static_cast<hostdependency*>(
            skiplist_find_next(
              object_skiplists[HOSTDEPENDENCY_SKIPLIST],
              &temp_hostdependency,
              ptr)));
}

/**
 *  Get next host escalation.
 *
 *  @param[in]     host_name Host name.
 *  @param[in,out] ptr       Container node.
 *
 *  @return Next host escalation if found, NULL otherwise.
 *
 *  @see get_first_host_escalation_by_host
 */
hostescalation* get_next_host_escalation_by_host(
                  char const* host_name,
                  void** ptr) {
  if (!host_name)
    return (NULL);

  hostescalation temp_hostescalation;
  temp_hostescalation.host_name = const_cast<char*>(host_name);
  return (static_cast<hostescalation*>(
            skiplist_find_next(
              object_skiplists[HOSTESCALATION_SKIPLIST],
              &temp_hostescalation,
              ptr)));
}

/**
 *  Get next service dependency.
 *
 *  @param[in]     host_name       Host name.
 *  @param[in]     svc_description Service description.
 *  @param[in,out] ptr             Container node.
 *
 *  @return Next service dependency if found, NULL otherwise.
 *
 *  @see get_first_service_dependency_by_dependent_service
 */
servicedependency* get_next_service_dependency_by_dependent_service(
                     char const* host_name,
                     char const* svc_description,
                     void** ptr) {
  if (!host_name || !svc_description || !ptr)
    return (NULL);

  servicedependency temp_servicedependency;
  temp_servicedependency.dependent_host_name
    = const_cast<char*>(host_name);
  temp_servicedependency.dependent_service_description
    = const_cast<char*>(svc_description);
  return (static_cast<servicedependency*>(
            skiplist_find_next(
              object_skiplists[SERVICEDEPENDENCY_SKIPLIST],
              &temp_servicedependency,
              ptr)));
}

/**
 *  Get next service escalation.
 *
 *  @param[in]     host_name       Host name.
 *  @param[in]     svc_description Service description.
 *  @param[in,out] ptr             Container node.
 *
 *  @return Next service escalation if found, NULL otherwise.
 *
 *  @see get_first_service_escalation_by_service
 */
serviceescalation* get_next_service_escalation_by_service(
                     char const* host_name,
                     char const* svc_description,
                     void** ptr) {
  if (!host_name || !svc_description)
    return (NULL);

  serviceescalation temp_serviceescalation;
  temp_serviceescalation.host_name = const_cast<char*>(host_name);
  temp_serviceescalation.description
    = const_cast<char*>(svc_description);
  return (static_cast<serviceescalation*>(
            skiplist_find_next(
              object_skiplists[SERVICEESCALATION_SKIPLIST],
              &temp_serviceescalation,
              ptr)));
}

/**************************************
*                                     *
*       Object Query Functions        *
*                                     *
**************************************/

/**
 *  Tests whether a contact is a contact for a particular host.
 *
 *  @param[in] hst   Target host.
 *  @param[in] cntct Target contact.
 *
 *  @return true or false.
 */
int is_contact_for_host(host* hst, contact* cntct) {
  if (!hst || !cntct)
    return (false);

  // Search all individual contacts of this host.
  for (contactsmember* member(hst->contacts);
       member;
       member = member->next)
    if (member->contact_ptr == cntct)
      return (true);

  // Search all contactgroups of this host.
  for (contactgroupsmember* member(hst->contact_groups);
       member;
       member = member->next)
    if (is_contact_member_of_contactgroup(member->group_ptr, cntct))
      return (true);

  return (false);
}

/**
 *  Tests whether a contact is a contact for a particular service.
 *
 *  @param[in] svc   Target service.
 *  @param[in] cntct Target contact.
 *
 *  @return true or false.
 */
int is_contact_for_service(service* svc, contact* cntct) {
  if (!svc || !cntct)
    return (false);

  // Search all individual contacts of this service.
  for (contactsmember* member(svc->contacts);
       member;
       member = member->next)
    if (member->contact_ptr == cntct)
      return (true);

  // Search all contactgroups of this service.
  for (contactgroupsmember* member(svc->contact_groups);
       member;
       member = member->next)
    if (is_contact_member_of_contactgroup(member->group_ptr, cntct))
      return (true);

  return (false);
}

/**
 *  Tests whether a contact is a member of a particular contactgroup.
 *  The mk-livestatus eventbroker module uses this, so it must hang
 *  around until 4.0 to prevent API breakage.
 *
 *  The cgi's stopped using it quite long ago though, so we need only
 *  compile it if we're building the core
 *
 *  @param[in] group Target contact group.
 *  @param[in] cntct Target contact.
 *
 *  @return true or false.
 */
int is_contact_member_of_contactgroup(
      contactgroup* group,
      contact* cntct) {
  if (!group || !cntct)
    return (false);

  // Search all contacts in this contact group.
  for (contactsmember* member(group->members);
       member;
       member = member->next)
    if (member->contact_ptr == cntct)
      return (true);

  return (false);
}

/**
 *  Tests whether or not a contact is an escalated contact for a
 *  particular host.
 *
 *  @param[in] hst   Target host.
 *  @param[in] cntct Target contact.
 *
 *  @return true or false.
 */
int is_escalated_contact_for_host(host* hst, contact* cntct) {
  if (!hst || !cntct)
    return (false);

  // Search all host escalations.
  void* ptr(NULL);
  for (hostescalation* hstescalation(get_first_host_escalation_by_host(hst->name, &ptr));
       hstescalation;
       hstescalation = get_next_host_escalation_by_host(hst->name, &ptr)) {
    // Search all contacts of this host escalation.
    for (contactsmember* member(hstescalation->contacts);
         member;
         member = member->next)
      if (member->contact_ptr == cntct)
        return (true);

    // Search all contactgroups of this host escalation.
    for (contactgroupsmember* member(hstescalation->contact_groups);
         member;
         member = member->next)
      if (is_contact_member_of_contactgroup(member->group_ptr, cntct))
        return (true);
  }

  return (false);
}

/**
 *  Tests whether or not a contact is an escalated contact for a
 *  particular service.
 *
 *  @param[in] svc   Target service.
 *  @param[in] cntct Target contact.
 *
 *  @return true or false.
 */
int is_escalated_contact_for_service(service* svc, contact* cntct) {
  if (!svc || !cntct)
    return (false);

  // Search all the service escalations.
  void* ptr(NULL);
  for (serviceescalation* svcescalation(get_first_service_escalation_by_service(svc->host_name, svc->description, &ptr));
       svcescalation;
       svcescalation = get_next_service_escalation_by_service(svc->host_name, svc->description, &ptr)) {
    // Search all contacts of this service escalation.
    for (contactsmember* member(svcescalation->contacts);
         member;
         member = member->next)
      if (member->contact_ptr == cntct)
        return (true);

    // Search all contactgroups of this service escalation.
    for (contactgroupsmember* member(svcescalation->contact_groups);
         member;
         member = member->next)
      if (is_contact_member_of_contactgroup(member->group_ptr, cntct))
        return (true);
  }

  return (false);
}

/**
 *  Determines whether or not a specific host is an immediate child of
 *  another host.
 *
 *  @param[in] parent_host Parent host.
 *  @param[in] child_host  Child host.
 *
 *  @return true or false.
 */
int is_host_immediate_child_of_host(
      host* parent_host,
      host* child_host) {
  // Not enough data.
  if (!child_host)
    return (false);

  // Root/top-level hosts.
  if (!parent_host) {
    if (!child_host->parent_hosts)
      return (true);
  }
  // Mid-level/bottom hosts.
  else {
    for (hostsmember* member(child_host->parent_hosts);
         member;
         member = member->next)
      if (member->host_ptr == parent_host)
        return (true);
  }

  return (false);
}

/**
 *  Determines whether or not a specific host is an immediate parent of
 *  another host.
 *
 *  @param[in] child_host  Child host.
 *  @param[in] parent_host Parent host.
 *
 *  @return true or false.
 */
int is_host_immediate_parent_of_host(
      host* child_host,
      host* parent_host) {
  if (is_host_immediate_child_of_host(parent_host, child_host) == true)
    return (true);
  return (false);
}

/**
 *  Tests whether a host is a member of a particular hostgroup.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] group Target host group.
 *  @param[in] hst   Target host.
 *
 *  @return true or false.
 */
int is_host_member_of_hostgroup(hostgroup* group, host* hst) {
  if (!group || !hst)
    return (false);

  for (hostsmember* member(group->members);
       member;
       member = member->next)
    if (member->host_ptr == hst)
      return (true);
  return (false);
}

/**
 *  Tests whether a host is a member of a particular servicegroup.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] group Target service group.
 *  @param[in] hst   Target host.
 *
 *  @return true or false.
 */
int is_host_member_of_servicegroup(servicegroup* group, host* hst) {
  if (!group || !hst)
    return (false);

  for (servicesmember* member(group->members);
       member;
       member = member->next)
    if (member->service_ptr
        && (member->service_ptr->host_ptr == hst))
      return (true);
  return (false);
}

/**
 *  Tests whether a service is a member of a particular servicegroup.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] group Target group.
 *  @param[in] svc   Target service.
 *
 *  @return true or false.
 */
int is_service_member_of_servicegroup(
      servicegroup* group,
      service* svc) {
  if (!group || !svc)
    return (false);

  for (servicesmember* member(group->members);
       member;
       member = member->next)
    if (member->service_ptr == svc)
      return (true);
  return (false);
}

/**************************************
*                                     *
* Circular Dependency Check Functions *
*                                     *
**************************************/

/**
 *  Checks to see if there exists a circular dependency for a host.
 *
 *  @param[in] root_dep        Root dependency.
 *  @param[in] dep             Dependency.
 *  @param[in] dependency_type Dependency type.
 *
 *  @return true if circular path was found, false otherwise.
 */
int check_for_circular_hostdependency_path(
      hostdependency* root_dep,
      hostdependency* dep,
      int dependency_type) {
  if (!root_dep || !dep)
    return (false);

  // This is not the proper dependency type.
  if ((root_dep->dependency_type != dependency_type)
      || (dep->dependency_type != dependency_type))
    return (false);

  // Don't go into a loop, don't bother checking anymore if we know this
  // dependency already has a loop.
  if (root_dep->contains_circular_path == true)
    return (true);

  // Dependency has already been checked - there is a path somewhere,
  // but it may not be for this particular dep... This should speed up
  // detection for some loops.
  if (dep->circular_path_checked == true)
    return (false);

  // Set the check flag so we don't get into an infinite loop.
  dep->circular_path_checked = true;

  // Is this host dependent on the root host?
  if (dep != root_dep) {
    if (root_dep->dependent_host_ptr == dep->master_host_ptr) {
      root_dep->contains_circular_path = true;
      dep->contains_circular_path = true;
      return (true);
    }
  }

  // Notification dependencies are ok at this point as long as they
  // don't inherit.
  if ((dependency_type == NOTIFICATION_DEPENDENCY)
      && (dep->inherits_parent == false))
    return (false);

  // Check all parent dependencies.
  for (hostdependency* temp_hd(hostdependency_list);
       temp_hd;
       temp_hd = temp_hd->next) {
    // Only check parent dependencies.
    if (dep->master_host_ptr != temp_hd->dependent_host_ptr)
      continue;

    if (check_for_circular_hostdependency_path(
          root_dep,
          temp_hd,
          dependency_type) == true)
      return (true);
  }

  return (false);
}

/**
 *  Checks to see if there exists a circular dependency for a service.
 *
 *  @param[in] root_dep        Root dependency.
 *  @param[in] dep             Dependency.
 *  @param[in] dependency_type Dependency type.
 *
 *  @return true if circular path was found, false otherwise.
 */
int check_for_circular_servicedependency_path(
      servicedependency* root_dep,
      servicedependency* dep,
      int dependency_type) {
  if (!root_dep || !dep)
    return (false);

  // This is not the proper dependency type.
  if ((root_dep->dependency_type != dependency_type)
      || (dep->dependency_type != dependency_type))
    return (false);

  // Don't go into a loop, don't bother checking anymore if we know this
  // dependency already has a loop.
  if (root_dep->contains_circular_path == true)
    return (true);

  // Dependency has already been checked - there is a path somewhere,
  // but it may not be for this particular dep... This should speed up
  // detection for some loops.
  if (dep->circular_path_checked == true)
    return (false);

  // Set the check flag so we don't get into an infinite loop.
  dep->circular_path_checked = true;

  // Is this service dependent on the root service?
  if (dep != root_dep) {
    if (root_dep->dependent_service_ptr == dep->master_service_ptr) {
      root_dep->contains_circular_path = true;
      dep->contains_circular_path = true;
      return (true);
    }
  }

  // Notification dependencies are ok at this point as long as they
  // don't inherit.
  if ((dependency_type == NOTIFICATION_DEPENDENCY)
      && (dep->inherits_parent == false))
    return (false);

  // Check all parent dependencies.
  for (servicedependency* temp_sd(servicedependency_list);
       temp_sd;
       temp_sd = temp_sd->next) {
    // Only check parent dependencies.
    if (dep->master_service_ptr != temp_sd->dependent_service_ptr)
      continue;

    if (check_for_circular_servicedependency_path(
          root_dep,
          temp_sd,
          dependency_type) == true)
      return (true);
  }

  return (false);
}

/**************************************
*                                     *
*       Object Lists Functions        *
*                                     *
**************************************/

/**
 *  Adds a object to a list of objects.
 *
 *  @param[in,out] list       Object list.
 *  @param[in]     object_ptr Object.
 *
 *  @return OK on success.
 */
int add_object_to_objectlist(objectlist** list, void* object_ptr) {
  if (!list || !object_ptr)
    return (ERROR);

  // Skip this object if its already in the list.
  objectlist* temp;
  for (temp = *list; temp; temp = temp->next)
    if (temp->object_ptr == object_ptr)
      break;
  if (temp)
    return (OK);

  // Allocate memory for a new list item.
  objectlist* obj(new objectlist);
  memset(obj, 0, sizeof(*obj));

  try {
    // Initialize vars.
    obj->object_ptr = object_ptr;

    // Add new item to head of list.
    obj->next = *list;
    *list = obj;
  }
  catch (...) {
    deleter::objectlist(obj);
    obj = NULL;
  }

  return (OK);
}

/**
 *  Frees memory allocated to a temporary object list.
 *
 *  @param[in,out] temp_list List to free.
 *
 *  @return OK on success.
 */
int free_objectlist(objectlist** temp_list) {
  if (!temp_list)
    return (ERROR);

  // Free memory allocated to object list.
  for (objectlist *this_objectlist(*temp_list), *next_objectlist;
       this_objectlist;
       this_objectlist = next_objectlist) {
    next_objectlist = this_objectlist->next;
    delete this_objectlist;
  }
  *temp_list = NULL;

  return (OK);
}

/**
 *  Remove a object to a list of objects.
 *
 *  @param[in,out] list       Object list.
 *  @param[in]     object_ptr Object.
 *
 *  @return OK on success.
 */
int remove_object_to_objectlist(objectlist** list, void* object_ptr) {
  if (!list)
    return (ERROR);

  for (objectlist *obj(*list), *prev(NULL);
       obj;
       prev = obj, obj = obj->next) {
    if (obj == object_ptr) {
      if (!prev)
	*list = obj->next;
      else
	prev->next = obj->next;
      delete obj;
      return (OK);
    }
  }
  return (ERROR);
}

/**************************************
*                                     *
*          Config Functions           *
*                                     *
**************************************/

/**
 *  Read all host configuration data from external source.
 *
 *  @param[in] main_config_file Main configuration file.
 *  @param[in] options          Options.
 *  @param[in] cache            Cache.
 *  @param[in] precache         Precache.
 *
 *  @return 0 on success.
 */
int read_object_config_data(
      char const* main_config_file,
      int options,
      int cache,
      int precache) {
  // Initialize object skiplists.
  init_object_skiplists();

  // Read in data from all text host config files (template-based).
  return (xodtemplate_read_config_data(
            main_config_file,
            options,
            cache,
            precache) != OK);
}

}

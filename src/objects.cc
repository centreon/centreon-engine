/*
** Copyright 1999-2008 Ethan Galstad
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

#include <memory>
#include <stdio.h>
#include <string.h>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/skiplist.hh"
#include "com/centreon/engine/xodtemplate.hh"

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
  // Return value.
  std::auto_ptr<hostsmember> new_hostsmember;

  // Make sure we have the data we need.
  if (parent && child) {
    // Allocate memory.
    new_hostsmember.reset(new hostsmember);

    // Initialize values.
    new_hostsmember->host_name = NULL;
    new_hostsmember->host_ptr = child;

    // Add the child entry to the host definition.
    new_hostsmember->next = parent->child_hosts;
    parent->child_hosts = new_hostsmember.get();

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

  return (new_hostsmember.release());
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
  std::auto_ptr<command> new_command(new command);
  memset(new_command.get(), 0, sizeof(*new_command));

  // Duplicate vars.
  new_command->name = my_strdup(name);
  new_command->command_line = my_strdup(value);

  // Add new command to skiplist.
  int result;
  result = skiplist_insert(
             object_skiplists[COMMAND_SKIPLIST],
             new_command.get());
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logger(log_config_error, basic)
      << "Error: Command '" << name << "' has already been defined";
    result = ERROR;
    break ;
  case SKIPLIST_OK:
    result = OK;
    break ;
  default:
    logger(log_config_error, basic)
      << "Error: Could not add command '" << name << "' to skiplist";
    result = ERROR;
    break ;
  }

  // Handle errors.
  if (result == ERROR) {
    delete [] new_command->command_line;
    delete [] new_command->name;
    return (NULL);
  }

  // Commands are sorted alphabetically,
  // so add new items to tail of list.
  if (!command_list) {
    command_list = new_command.get();
    command_list_tail = command_list;
  }
  else {
    command_list_tail->next = new_command.get();
    command_list_tail = new_command.get();
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_command_data(
    NEBTYPE_COMMAND_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    name,
    value,
    &tv);

  return (new_command.release());
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
    logger(log_config_error, basic) << "Error: Contact name is NULL";
    return (NULL);
  }

  // Allocate memory for a new contact.
  std::auto_ptr<contact> new_contact(new contact);
  memset(new_contact.get(), 0, sizeof(*new_contact));

  // Duplicate vars.
  new_contact->name = my_strdup(name);
  new_contact->alias = my_strdup(!alias ? name : alias);
  if (email)
    new_contact->email = my_strdup(email);
  if (pager)
    new_contact->pager = my_strdup(pager);
  if (svc_notification_period)
    new_contact->service_notification_period
      = my_strdup(svc_notification_period);
  if (host_notification_period)
    new_contact->host_notification_period
      = my_strdup(host_notification_period);
  if (addresses) {
    for (unsigned int x(0); x < MAX_CONTACT_ADDRESSES; ++x)
      if (addresses[x])
        new_contact->address[x] = my_strdup(addresses[x]);
  }

  // Set remaining contact properties.
  new_contact->notify_on_service_recovery
    = (notify_service_ok > 0) ? TRUE : FALSE;
  new_contact->notify_on_service_critical
    = (notify_service_critical > 0) ? TRUE : FALSE;
  new_contact->notify_on_service_warning
    = (notify_service_warning > 0) ? TRUE : FALSE;
  new_contact->notify_on_service_unknown
    = (notify_service_unknown > 0) ? TRUE : FALSE;
  new_contact->notify_on_service_flapping
    = (notify_service_flapping > 0) ? TRUE : FALSE;
  new_contact->notify_on_service_downtime
    = (notify_service_downtime > 0) ? TRUE : FALSE;
  new_contact->notify_on_host_recovery
    = (notify_host_up > 0) ? TRUE : FALSE;
  new_contact->notify_on_host_down
    = (notify_host_down > 0) ? TRUE : FALSE;
  new_contact->notify_on_host_unreachable
    = (notify_host_unreachable > 0) ? TRUE : FALSE;
  new_contact->notify_on_host_flapping
    = (notify_host_flapping > 0) ? TRUE : FALSE;
  new_contact->notify_on_host_downtime
    = (notify_host_downtime > 0) ? TRUE : FALSE;
  new_contact->host_notifications_enabled
    = (host_notifications_enabled > 0) ? TRUE : FALSE;
  new_contact->service_notifications_enabled
    = (service_notifications_enabled > 0) ? TRUE : FALSE;
  new_contact->can_submit_commands
    = (can_submit_commands > 0) ? TRUE : FALSE;
  new_contact->retain_status_information
    = (retain_status_information > 0) ? TRUE : FALSE;
  new_contact->retain_nonstatus_information
    = (retain_nonstatus_information > 0) ? TRUE : FALSE;
  new_contact->last_host_notification = (time_t)0L;
  new_contact->last_service_notification = (time_t)0L;
  new_contact->modified_attributes = MODATTR_NONE;
  new_contact->modified_host_attributes = MODATTR_NONE;
  new_contact->modified_service_attributes = MODATTR_NONE;
  new_contact->host_notification_period_ptr = NULL;
  new_contact->service_notification_period_ptr = NULL;
  new_contact->contactgroups_ptr = NULL;

  // Add new contact to skiplist.
  int result;
  result = skiplist_insert(
             object_skiplists[CONTACT_SKIPLIST],
             new_contact.get());
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logger(log_config_error, basic)
      << "Error: Contact '" << name << "' has already been defined";
    result = ERROR;
    break ;
  case SKIPLIST_OK:
    result = OK;
    break ;
  default:
    logger(log_config_error, basic)
      << "Error: Could not add contact '" << name << "' to skiplist";
    result = ERROR;
    break ;
  }

  // Handle errors.
  if (result == ERROR) {
    for (unsigned x(0); x < MAX_CONTACT_ADDRESSES; ++x)
      delete [] new_contact->address[x];
    delete [] new_contact->name;
    delete [] new_contact->alias;
    delete [] new_contact->email;
    delete [] new_contact->pager;
    delete [] new_contact->service_notification_period;
    delete [] new_contact->host_notification_period;
    return (NULL);
  }

  // Contacts are sorted alphabetically,
  // so add new items to tail of list.
  if (!contact_list) {
    contact_list = new_contact.get();
    contact_list_tail = contact_list;
  }
  else {
    contact_list_tail->next = new_contact.get();
    contact_list_tail = new_contact.get();
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_contact_data(
    NEBTYPE_CONTACT_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    new_contact.get(),
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);

  return (new_contact.release());
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
  std::auto_ptr<contactsmember> new_contactsmember(new contactsmember);
  memset(new_contactsmember.get(), 0, sizeof(*new_contactsmember));

  // Duplicate vars.
  new_contactsmember->contact_name = my_strdup(contact_name);

  // Add the new member to the head of the member list.
  new_contactsmember->next = grp->members;
  grp->members = new_contactsmember.get();

  // Notify event broker.
  // XXX

  return (new_contactsmember.release());
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
  std::auto_ptr<contactsmember> new_contactsmember(new contactsmember);
  new_contactsmember->contact_name = my_strdup(contact_name);

  // Set initial values.
  new_contactsmember->contact_ptr = NULL;

  // Add the new contact to the head of the contact list.
  new_contactsmember->next = *object_ptr;
  *object_ptr = new_contactsmember.get();

  return (new_contactsmember.release());
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
    logger(log_config_error, basic) << "Error: Contactgroup name is NULL";
    return (NULL);
  }

  // Allocate memory for a new contactgroup entry.
  std::auto_ptr<contactgroup> new_contactgroup(new contactgroup);
  memset(new_contactgroup.get(), 0, sizeof(*new_contactgroup));

  // Duplicate vars.
  new_contactgroup->group_name = my_strdup(name);
  new_contactgroup->alias = my_strdup(!alias ? name : alias);

  // Add new contact group to skiplist.
  int result;
  result = skiplist_insert(
             object_skiplists[CONTACTGROUP_SKIPLIST],
             new_contactgroup.get());
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logger(log_config_error, basic)
      << "Error: Contactgroup '" << name << "' has already been defined";
    result = ERROR;
    break ;
  case SKIPLIST_OK:
    result = OK;
    break ;
  default:
    logger(log_config_error, basic)
      << "Error: Could not add contactgroup '" << name << "' to skiplist";
    result = ERROR;
    break ;
  }

  // Handle errors.
  if (result == ERROR) {
    delete [] new_contactgroup->alias;
    delete [] new_contactgroup->group_name;
    return (NULL);
  }

  // Contact groups are sorted alphabetically,
  // so add new items to tail of list.
  if (!contactgroup_list) {
    contactgroup_list = new_contactgroup.get();
    contactgroup_list_tail = contactgroup_list;
  }
  else {
    contactgroup_list_tail->next = new_contactgroup.get();
    contactgroup_list_tail = new_contactgroup.get();
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group(
    NEBTYPE_CONTACTGROUP_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    new_contactgroup.get(),
    &tv);

  return (new_contactgroup.release());
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
  std::auto_ptr<contactgroupsmember>
    new_contactgroupsmember(new contactgroupsmember);
  memset(
    new_contactgroupsmember.get(),
    0,
    sizeof(*new_contactgroupsmember));

  // Duplicate string vars.
  new_contactgroupsmember->group_name = my_strdup(group_name);

  // Add the new member to the head of the member list.
  new_contactgroupsmember->next = hst->contact_groups;
  hst->contact_groups = new_contactgroupsmember.get();

  // Notify event broker.
  // XXX

  return (new_contactgroupsmember.release());
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
  std::auto_ptr<contactgroupsmember>
    new_contactgroupsmember(new contactgroupsmember);
  memset(
    new_contactgroupsmember.get(),
    0,
    sizeof(*new_contactgroupsmember));

  // Duplicate vars.
  new_contactgroupsmember->group_name = my_strdup(group_name);

  // Add this contactgroup to the host escalation.
  new_contactgroupsmember->next = he->contact_groups;
  he->contact_groups = new_contactgroupsmember.get();

  // Notify event broker.
  // XXX

  return (new_contactgroupsmember.release());
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
  std::auto_ptr<contactgroupsmember>
    new_contactgroupsmember(new contactgroupsmember);
  memset(
    new_contactgroupsmember.get(),
    0,
    sizeof(*new_contactgroupsmember));

  // Duplicate vars.
  new_contactgroupsmember->group_name = my_strdup(group_name);

  // Add this contactgroup to the service.
  new_contactgroupsmember->next = svc->contact_groups;
  svc->contact_groups = new_contactgroupsmember.get();

  // Notify event broker.
  // XXX

  return (new_contactgroupsmember.release());
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
  std::auto_ptr<contactgroupsmember>
    new_contactgroupsmember(new contactgroupsmember);
  memset(
    new_contactgroupsmember.get(),
    0,
    sizeof(*new_contactgroupsmember));

  // Duplicate vars.
  new_contactgroupsmember->group_name = my_strdup(group_name);

  // Add this contactgroup to the service escalation.
  new_contactgroupsmember->next = se->contact_groups;
  se->contact_groups = new_contactgroupsmember.get();

  // Notify event broker.
  // XXX

  return (new_contactgroupsmember.release());
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
  std::auto_ptr<customvariablesmember>
    new_customvariablesmember(new customvariablesmember);
  new_customvariablesmember->variable_name = my_strdup(varname);
  if (varvalue)
    new_customvariablesmember->variable_value = my_strdup(varvalue);
  else
    new_customvariablesmember->variable_value = NULL;

  // Set initial values.
  new_customvariablesmember->has_been_modified = FALSE;

  // Add the new member to the head of the member list.
  new_customvariablesmember->next = *object_ptr;
  *object_ptr = new_customvariablesmember.get();

  return (new_customvariablesmember.release());
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
  std::auto_ptr<daterange> new_daterange(new daterange);

  // Set daterange properties.
  new_daterange->times = NULL;
  new_daterange->next = NULL;
  new_daterange->type = type;
  new_daterange->syear = syear;
  new_daterange->smon = smon;
  new_daterange->smday = smday;
  new_daterange->swday = swday;
  new_daterange->swday_offset = swday_offset;
  new_daterange->eyear = eyear;
  new_daterange->emon = emon;
  new_daterange->emday = emday;
  new_daterange->ewday = ewday;
  new_daterange->ewday_offset = ewday_offset;
  new_daterange->skip_interval = skip_interval;

  // Add the new date range to the head of the range
  // list for this exception type.
  new_daterange->next = period->exceptions[type];
  period->exceptions[type] = new_daterange.get();

  // Notify event broker.
  // XXX

  return (new_daterange.release());
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
  std::auto_ptr<timeperiodexclusion>
    new_timeperiodexclusion(new timeperiodexclusion);

  // Set exclusion properties.
  new_timeperiodexclusion->timeperiod_name = my_strdup(name);
  new_timeperiodexclusion->next = period->exclusions;
  period->exclusions = new_timeperiodexclusion.get();

  // Notify event broker.
  // XXX

  return (new_timeperiodexclusion.release());
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
  std::auto_ptr<host> new_host(new host);
  memset(new_host.get(), 0, sizeof(*new_host));

  // Duplicate string vars.
  new_host->name = my_strdup(name);
  new_host->display_name
    = my_strdup(!display_name ? name : display_name);
  new_host->alias = my_strdup((alias == NULL) ? name : alias);
  new_host->address = my_strdup(address);
  if (check_period)
    new_host->check_period = my_strdup(check_period);
  if (notification_period)
    new_host->notification_period = my_strdup(notification_period);
  if (check_command)
    new_host->host_check_command = my_strdup(check_command);
  if (event_handler)
    new_host->event_handler = my_strdup(event_handler);
  if (failure_prediction_options)
    new_host->failure_prediction_options
      = my_strdup(failure_prediction_options);
  if (notes)
    new_host->notes = my_strdup(notes);
  if (notes_url)
    new_host->notes_url = my_strdup(notes_url);
  if (action_url)
    new_host->action_url = my_strdup(action_url);
  if (icon_image)
    new_host->icon_image = my_strdup(icon_image);
  if (icon_image_alt)
    new_host->icon_image_alt = my_strdup(icon_image_alt);
  if (vrml_image)
    new_host->vrml_image = my_strdup(vrml_image);
  if (statusmap_image)
    new_host->statusmap_image = my_strdup(statusmap_image);

  // Duplicate non-string vars.
  new_host->max_attempts = max_attempts;
  new_host->check_interval = check_interval;
  new_host->retry_interval = retry_interval;
  new_host->notification_interval = notification_interval;
  new_host->first_notification_delay = first_notification_delay;
  new_host->notify_on_recovery = (notify_up > 0) ? TRUE : FALSE;
  new_host->notify_on_down = (notify_down > 0) ? TRUE : FALSE;
  new_host->notify_on_unreachable
    = (notify_unreachable > 0) ? TRUE : FALSE;
  new_host->notify_on_flapping = (notify_flapping > 0) ? TRUE : FALSE;
  new_host->notify_on_downtime = (notify_downtime > 0) ? TRUE : FALSE;
  new_host->flap_detection_enabled
    = (flap_detection_enabled > 0) ? TRUE : FALSE;
  new_host->low_flap_threshold = low_flap_threshold;
  new_host->high_flap_threshold = high_flap_threshold;
  new_host->flap_detection_on_up
    = (flap_detection_on_up > 0) ? TRUE : FALSE;
  new_host->flap_detection_on_down
    = (flap_detection_on_down > 0) ? TRUE : FALSE;
  new_host->flap_detection_on_unreachable
    = (flap_detection_on_unreachable > 0) ? TRUE : FALSE;
  new_host->stalk_on_up = (stalk_on_up > 0) ? TRUE : FALSE;
  new_host->stalk_on_down = (stalk_on_down > 0) ? TRUE : FALSE;
  new_host->stalk_on_unreachable
    = (stalk_on_unreachable > 0) ? TRUE : FALSE;
  new_host->process_performance_data
    = (process_perfdata > 0) ? TRUE : FALSE;
  new_host->check_freshness = (check_freshness > 0) ? TRUE : FALSE;
  new_host->freshness_threshold = freshness_threshold;
  new_host->checks_enabled = (checks_enabled > 0) ? TRUE : FALSE;
  new_host->accept_passive_host_checks
    = (accept_passive_checks > 0) ? TRUE : FALSE;
  new_host->event_handler_enabled
    = (event_handler_enabled > 0) ? TRUE : FALSE;
  new_host->failure_prediction_enabled
    = (failure_prediction_enabled > 0) ? TRUE : FALSE;
  new_host->x_2d = x_2d;
  new_host->y_2d = y_2d;
  new_host->have_2d_coords = (have_2d_coords > 0) ? TRUE : FALSE;
  new_host->x_3d = x_3d;
  new_host->y_3d = y_3d;
  new_host->z_3d = z_3d;
  new_host->have_3d_coords = (have_3d_coords > 0) ? TRUE : FALSE;
  new_host->should_be_drawn = (should_be_drawn > 0) ? TRUE : FALSE;
  new_host->obsess_over_host = (obsess_over_host > 0) ? TRUE : FALSE;
  new_host->retain_status_information
    = (retain_status_information > 0) ? TRUE : FALSE;
  new_host->retain_nonstatus_information
    = (retain_nonstatus_information > 0) ? TRUE : FALSE;
  new_host->current_state = initial_state;
  new_host->current_event_id = 0L;
  new_host->last_event_id = 0L;
  new_host->current_problem_id = 0L;
  new_host->last_problem_id = 0L;
  new_host->last_state = initial_state;
  new_host->last_hard_state = initial_state;
  new_host->check_type = HOST_CHECK_ACTIVE;
  new_host->last_host_notification = (time_t)0;
  new_host->next_host_notification = (time_t)0;
  new_host->next_check = (time_t)0;
  new_host->should_be_scheduled = TRUE;
  new_host->last_check = (time_t)0;
  new_host->current_attempt
    = (initial_state == HOST_UP) ? 1 : max_attempts;
  new_host->state_type = HARD_STATE;
  new_host->execution_time = 0.0;
  new_host->is_executing = FALSE;
  new_host->latency = 0.0;
  new_host->last_state_change = (time_t)0;
  new_host->last_hard_state_change = (time_t)0;
  new_host->last_time_up = (time_t)0;
  new_host->last_time_down = (time_t)0;
  new_host->last_time_unreachable = (time_t)0;
  new_host->has_been_checked = FALSE;
  new_host->is_being_freshened = FALSE;
  new_host->problem_has_been_acknowledged = FALSE;
  new_host->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
  new_host->notifications_enabled
    = (notifications_enabled > 0) ? TRUE : FALSE;
  new_host->notified_on_down = FALSE;
  new_host->notified_on_unreachable = FALSE;
  new_host->current_notification_number = 0;
  new_host->current_notification_id = 0L;
  new_host->no_more_notifications = FALSE;
  new_host->check_flapping_recovery_notification = FALSE;
  new_host->scheduled_downtime_depth = 0;
  new_host->check_options = CHECK_OPTION_NONE;
  new_host->pending_flex_downtime = 0;
  for (unsigned int x(0); x < MAX_STATE_HISTORY_ENTRIES; ++x)
    new_host->state_history[x] = STATE_OK;
  new_host->state_history_index = 0;
  new_host->last_state_history_update = (time_t)0;
  new_host->is_flapping = FALSE;
  new_host->flapping_comment_id = 0;
  new_host->percent_state_change = 0.0;
  new_host->total_services = 0;
  new_host->total_service_check_interval = 0L;
  new_host->modified_attributes = MODATTR_NONE;
  new_host->circular_path_checked = FALSE;
  new_host->contains_circular_path = FALSE;

  // Add new host to skiplist.
  int result;
  result = skiplist_insert(
             object_skiplists[HOST_SKIPLIST],
             new_host.get());
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logger(log_config_error, basic)
      << "Error: Host '" << name << "' has already been defined";
    result = ERROR;
    break ;
  case SKIPLIST_OK:
    result = OK;
    break ;
  default:
    logger(log_config_error, basic)
      << "Error: Could not add host '" << name << "' to skiplist";
    result = ERROR;
    break ;
  }

  // Handle errors.
  if (result == ERROR) {
    delete [] new_host->plugin_output;
    delete [] new_host->long_plugin_output;
    delete [] new_host->perf_data;
    delete [] new_host->statusmap_image;
    delete [] new_host->vrml_image;
    delete [] new_host->icon_image_alt;
    delete [] new_host->icon_image;
    delete [] new_host->action_url;
    delete [] new_host->notes_url;
    delete [] new_host->notes;
    delete [] new_host->failure_prediction_options;
    delete [] new_host->event_handler;
    delete [] new_host->host_check_command;
    delete [] new_host->notification_period;
    delete [] new_host->check_period;
    delete [] new_host->address;
    delete [] new_host->alias;
    delete [] new_host->display_name;
    delete [] new_host->name;
    return (NULL);
  }

  // Hosts are sorted alphabetically, so add new items to tail of list.
  if (!host_list) {
    host_list = new_host.get();
    host_list_tail = host_list;
  }
  else {
    host_list_tail->next = new_host.get();
    host_list_tail = new_host.get();
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_host_data(
    NEBTYPE_HOST_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    new_host.get(),
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);

  return (new_host.release());
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
  std::auto_ptr<hostdependency> new_hostdependency(new hostdependency);
  memset(new_hostdependency.get(), 0, sizeof(*new_hostdependency));

  // Duplicate vars.
  new_hostdependency->dependent_host_name
    = my_strdup(dependent_host_name);
  new_hostdependency->host_name = my_strdup(host_name);
  if (dependency_period)
    new_hostdependency->dependency_period
      = my_strdup(dependency_period);
  new_hostdependency->dependency_type
    = (dependency_type == EXECUTION_DEPENDENCY)
    ? EXECUTION_DEPENDENCY
    : NOTIFICATION_DEPENDENCY;
  new_hostdependency->inherits_parent
    = (inherits_parent > 0) ? TRUE : FALSE;
  new_hostdependency->fail_on_up = (fail_on_up == 1) ? TRUE : FALSE;
  new_hostdependency->fail_on_down = (fail_on_down == 1) ? TRUE : FALSE;
  new_hostdependency->fail_on_unreachable
    = (fail_on_unreachable == 1) ? TRUE : FALSE;
  new_hostdependency->fail_on_pending
    = (fail_on_pending == 1) ? TRUE : FALSE;
  new_hostdependency->circular_path_checked = FALSE;
  new_hostdependency->contains_circular_path = FALSE;

  // Add new host dependency to skiplist.
  int result;
  result = skiplist_insert(
             object_skiplists[HOSTDEPENDENCY_SKIPLIST],
             new_hostdependency.get());
  switch (result) {
  case SKIPLIST_OK:
    result = OK;
    break ;
  default:
    logger(log_config_error, basic)
      << "Error: Could not add host dependency to skiplist";
    result = ERROR;
    break ;
  }

  // Handle errors.
  if (result == ERROR) {
    delete [] new_hostdependency->host_name;
    delete [] new_hostdependency->dependent_host_name;
    return (NULL);
  }

  // Host dependencies are sorted alphabetically,
  // so add new items to tail of list.
  if (!hostdependency_list) {
    hostdependency_list = new_hostdependency.get();
    hostdependency_list_tail = hostdependency_list;
  }
  else {
    hostdependency_list_tail->next = new_hostdependency.get();
    hostdependency_list_tail = new_hostdependency.get();
  }

  // Notify event broker.
  // XXX

  return (new_hostdependency.release());
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
  std::auto_ptr<commandsmember> new_commandsmember(new commandsmember);
  memset(new_commandsmember.get(), 0, sizeof(*new_commandsmember));

  // Duplicate vars.
  new_commandsmember->cmd = my_strdup(command_name);

  // Add the notification command.
  new_commandsmember->next = cntct->host_notification_commands;
  cntct->host_notification_commands = new_commandsmember.get();

  // Notify event broker.
  // XXX

  return (new_commandsmember.release());
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
  std::auto_ptr<hostsmember> new_member(new hostsmember);
  memset(new_member.get(), 0, sizeof(*new_member));

  // Duplicate vars.
  new_member->host_name = my_strdup(host_name);

  // Add the new member to the member list, sorted by host name.
  hostsmember* last_member(temp_hostgroup->members);
  hostsmember* temp_member;
  for (temp_member = temp_hostgroup->members;
       temp_member;
       temp_member = temp_member->next) {
    if (strcmp(new_member->host_name, temp_member->host_name) < 0) {
      new_member->next = temp_member;
      if (temp_member == temp_hostgroup->members)
        temp_hostgroup->members = new_member.get();
      else
        last_member->next = new_member.get();
      break ;
    }
    else
      last_member = temp_member;
  }
  if (!temp_hostgroup->members) {
    new_member->next = NULL;
    temp_hostgroup->members = new_member.get();
  }
  else if (temp_member == NULL) {
    new_member->next = NULL;
    last_member->next = new_member.get();
  }

  // Notify event broker.
  // XXX

  return (new_member.release());
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
  std::auto_ptr<hostescalation> new_hostescalation(new hostescalation);
  memset(new_hostescalation.get(), 0, sizeof(*new_hostescalation));

  // Duplicate vars.
  new_hostescalation->host_name = my_strdup(host_name);
  if (escalation_period)
    new_hostescalation->escalation_period
      = my_strdup(escalation_period);
  new_hostescalation->first_notification = first_notification;
  new_hostescalation->last_notification = last_notification;
  new_hostescalation->notification_interval
    = (notification_interval <= 0) ? 0 : notification_interval;
  new_hostescalation->escalate_on_recovery
    = (escalate_on_recovery > 0) ? TRUE : FALSE;
  new_hostescalation->escalate_on_down
    = (escalate_on_down > 0) ? TRUE : FALSE;
  new_hostescalation->escalate_on_unreachable
    = (escalate_on_unreachable > 0) ? TRUE : FALSE;

  // Add new hostescalation to skiplist.
  int result;
  result = skiplist_insert(
             object_skiplists[HOSTESCALATION_SKIPLIST],
             new_hostescalation.get());
  switch (result) {
  case SKIPLIST_OK:
    result = OK;
    break ;
  default:
    logger(log_config_error, basic)
      << "Error: Could not add hostescalation '"
      << host_name << "' to skiplist";
    result = ERROR;
    break ;
  }

  // Handle errors.
  if (result == ERROR) {
    delete [] new_hostescalation->host_name;
    delete [] new_hostescalation->escalation_period;
    return (NULL);
  }

  // Host escalations are sorted alphabetically,
  // so add new items to tail of list.
  if (hostescalation_list == NULL) {
    hostescalation_list = new_hostescalation.get();
    hostescalation_list_tail = hostescalation_list;
  }
  else {
    hostescalation_list_tail->next = new_hostescalation.get();
    hostescalation_list_tail = new_hostescalation.get();
  }

  // Notify event broker.
  // XXX

  return (new_hostescalation.release());
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
  std::auto_ptr<hostgroup> new_hostgroup(new hostgroup);
  memset(new_hostgroup.get(), 0, sizeof(*new_hostgroup));

  // Duplicate vars.
  new_hostgroup->group_name = my_strdup(name);
  new_hostgroup->alias = my_strdup((alias == NULL) ? name : alias);
  if (notes)
    new_hostgroup->notes = my_strdup(notes);
  if (notes_url)
    new_hostgroup->notes_url = my_strdup(notes_url);
  if (action_url)
    new_hostgroup->action_url = my_strdup(action_url);

  // Add new host group to skiplist.
  int result;
  result = skiplist_insert(
             object_skiplists[HOSTGROUP_SKIPLIST],
             new_hostgroup.get());
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logger(log_config_error, basic)
      << "Error: Hostgroup '" << name << "' has already been defined";
    result = ERROR;
    break ;
  case SKIPLIST_OK:
    result = OK;
    break ;
  default:
    logger(log_config_error, basic)
      << "Error: Could not add hostgroup '" << name << "' to skiplist";
    result = ERROR;
    break ;
  }

  // Handle errors.
  if (result == ERROR) {
    delete [] new_hostgroup->alias;
    delete [] new_hostgroup->group_name;
    return (NULL);
  }

  // Hostgroups are sorted alphabetically,
  // so add new items to tail of list.
  if (hostgroup_list == NULL) {
    hostgroup_list = new_hostgroup.get();
    hostgroup_list_tail = hostgroup_list;
  }
  else {
    hostgroup_list_tail->next = new_hostgroup.get();
    hostgroup_list_tail = new_hostgroup.get();
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group(
    NEBTYPE_HOSTGROUP_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    new_hostgroup.get(),
    &tv);

  return (new_hostgroup.release());
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
  std::auto_ptr<hostsmember> new_hostsmember(new hostsmember);
  memset(new_hostsmember.get(), 0, sizeof(*new_hostsmember));

  // Duplicate string vars.
  new_hostsmember->host_name = my_strdup(host_name);

  // Add the parent host entry to the host definition */
  new_hostsmember->next = hst->parent_hosts;
  hst->parent_hosts = new_hostsmember.get();

  // Notify event broker.
  // XXX

  return (new_hostsmember.release());
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
    logger(log_config_error, basic) << "error: host name of service '"
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
      << "Error: Invalid max_attempts, check_interval, retry_interval" \
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
  std::auto_ptr<service> new_service(new service);
  memset(new_service.get(), 0, sizeof(*new_service));

  // Duplicate vars.
  new_service->host_name = my_strdup(host_name);
  new_service->description = my_strdup(description);
  new_service->display_name
    = my_strdup(!display_name ? description : display_name);
  new_service->service_check_command = my_strdup(check_command);
  if (event_handler)
    new_service->event_handler = my_strdup(event_handler);
  if (notification_period)
    new_service->notification_period = my_strdup(notification_period);
  if (check_period)
    new_service->check_period = my_strdup(check_period);
  if (failure_prediction_options)
    new_service->failure_prediction_options
      = my_strdup(failure_prediction_options);
  if (notes)
    new_service->notes = my_strdup(notes);
  if (notes_url)
    new_service->notes_url = my_strdup(notes_url);
  if (action_url)
    new_service->action_url = my_strdup(action_url);
  if (icon_image)
    new_service->icon_image = my_strdup(icon_image);
  if (icon_image_alt)
    new_service->icon_image_alt = my_strdup(icon_image_alt);
  new_service->check_interval = check_interval;
  new_service->retry_interval = retry_interval;
  new_service->max_attempts = max_attempts;
  new_service->parallelize = (parallelize > 0) ? TRUE : FALSE;
  new_service->notification_interval = notification_interval;
  new_service->first_notification_delay = first_notification_delay;
  new_service->notify_on_unknown = (notify_unknown > 0) ? TRUE : FALSE;
  new_service->notify_on_warning = (notify_warning > 0) ? TRUE : FALSE;
  new_service->notify_on_critical
    = (notify_critical > 0) ? TRUE : FALSE;
  new_service->notify_on_recovery
    = (notify_recovery > 0) ? TRUE : FALSE;
  new_service->notify_on_flapping
    = (notify_flapping > 0) ? TRUE : FALSE;
  new_service->notify_on_downtime
    = (notify_downtime > 0) ? TRUE : FALSE;
  new_service->is_volatile = (is_volatile > 0) ? TRUE : FALSE;
  new_service->flap_detection_enabled
    = (flap_detection_enabled > 0) ? TRUE : FALSE;
  new_service->low_flap_threshold = low_flap_threshold;
  new_service->high_flap_threshold = high_flap_threshold;
  new_service->flap_detection_on_ok
    = (flap_detection_on_ok > 0) ? TRUE : FALSE;
  new_service->flap_detection_on_warning
    = (flap_detection_on_warning > 0) ? TRUE : FALSE;
  new_service->flap_detection_on_unknown
    = (flap_detection_on_unknown > 0) ? TRUE : FALSE;
  new_service->flap_detection_on_critical
    = (flap_detection_on_critical > 0) ? TRUE : FALSE;
  new_service->stalk_on_ok = (stalk_on_ok > 0) ? TRUE : FALSE;
  new_service->stalk_on_warning = (stalk_on_warning > 0) ? TRUE : FALSE;
  new_service->stalk_on_unknown = (stalk_on_unknown > 0) ? TRUE : FALSE;
  new_service->stalk_on_critical
    = (stalk_on_critical > 0) ? TRUE : FALSE;
  new_service->process_performance_data
    = (process_perfdata > 0) ? TRUE : FALSE;
  new_service->check_freshness = (check_freshness > 0) ? TRUE : FALSE;
  new_service->freshness_threshold = freshness_threshold;
  new_service->accept_passive_service_checks
    = (accept_passive_checks > 0) ? TRUE : FALSE;
  new_service->event_handler_enabled
    = (event_handler_enabled > 0) ? TRUE : FALSE;
  new_service->checks_enabled = (checks_enabled > 0) ? TRUE : FALSE;
  new_service->retain_status_information
    = (retain_status_information > 0) ? TRUE : FALSE;
  new_service->retain_nonstatus_information
    = (retain_nonstatus_information > 0) ? TRUE : FALSE;
  new_service->notifications_enabled
    = (notifications_enabled > 0) ? TRUE : FALSE;
  new_service->obsess_over_service
    = (obsess_over_service > 0) ? TRUE : FALSE;
  new_service->failure_prediction_enabled
    = (failure_prediction_enabled > 0) ? TRUE : FALSE;
  new_service->problem_has_been_acknowledged = FALSE;
  new_service->acknowledgement_type = ACKNOWLEDGEMENT_NONE;
  new_service->check_type = SERVICE_CHECK_ACTIVE;
  new_service->current_attempt
    = (initial_state == STATE_OK) ? 1 : max_attempts;
  new_service->current_state = initial_state;
  new_service->current_event_id = 0L;
  new_service->last_event_id = 0L;
  new_service->current_problem_id = 0L;
  new_service->last_problem_id = 0L;
  new_service->last_state = initial_state;
  new_service->last_hard_state = initial_state;
  new_service->state_type = HARD_STATE;
  new_service->host_problem_at_last_check = FALSE;
  new_service->check_flapping_recovery_notification = FALSE;
  new_service->next_check = (time_t)0;
  new_service->should_be_scheduled = TRUE;
  new_service->last_check = (time_t)0;
  new_service->last_notification = (time_t)0;
  new_service->next_notification = (time_t)0;
  new_service->no_more_notifications = FALSE;
  new_service->last_state_change = (time_t)0;
  new_service->last_hard_state_change = (time_t)0;
  new_service->last_time_ok = (time_t)0;
  new_service->last_time_warning = (time_t)0;
  new_service->last_time_unknown = (time_t)0;
  new_service->last_time_critical = (time_t)0;
  new_service->has_been_checked = FALSE;
  new_service->is_being_freshened = FALSE;
  new_service->notified_on_unknown = FALSE;
  new_service->notified_on_warning = FALSE;
  new_service->notified_on_critical = FALSE;
  new_service->current_notification_number = 0;
  new_service->current_notification_id = 0L;
  new_service->latency = 0.0;
  new_service->execution_time = 0.0;
  new_service->is_executing = FALSE;
  new_service->check_options = CHECK_OPTION_NONE;
  new_service->scheduled_downtime_depth = 0;
  new_service->pending_flex_downtime = 0;
  for (unsigned int x(0); x < MAX_STATE_HISTORY_ENTRIES; ++x)
    new_service->state_history[x] = STATE_OK;
  new_service->state_history_index = 0;
  new_service->is_flapping = FALSE;
  new_service->flapping_comment_id = 0;
  new_service->percent_state_change = 0.0;
  new_service->modified_attributes = MODATTR_NONE;

  // Add new service to skiplist.
  int result;
  result = skiplist_insert(
             object_skiplists[SERVICE_SKIPLIST],
             new_service.get());
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logger(log_config_error, basic)
      << "Error: Service '" << description << "' on host '"
      << host_name << "' has already been defined";
    result = ERROR;
    break ;
  case SKIPLIST_OK:
    result = OK;
    break ;
  default:
    logger(log_config_error, basic)
      << "Error: Could not add service '" << description
      << "' on host '" << host_name << "' to skiplist";
    result = ERROR;
    break ;
  }

  // Handle errors.
  if (result == ERROR) {
    delete [] new_service->perf_data;
    delete [] new_service->plugin_output;
    delete [] new_service->long_plugin_output;
    delete [] new_service->failure_prediction_options;
    delete [] new_service->notification_period;
    delete [] new_service->event_handler;
    delete [] new_service->service_check_command;
    delete [] new_service->display_name;
    delete [] new_service->description;
    delete [] new_service->host_name;
    return (NULL);
  }

  // Services are sorted alphabetically,
  // so add new items to tail of list.
  if (!service_list) {
    service_list = new_service.get();
    service_list_tail = service_list;
  }
  else {
    service_list_tail->next = new_service.get();
    service_list_tail = new_service.get();
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_service_data(
    NEBTYPE_SERVICE_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    new_service.get(),
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);

  return (new_service.release());
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
      << "Error: NULL master service description/host " \
         "name in service dependency definition";
    return (NULL);
  }
  if (!dependent_host_name
      || !dependent_host_name[0]
      || !dependent_service_description
      || !dependent_service_description[0]) {
    logger(log_config_error, basic)
      << "Error: NULL dependent service description/host " \
         "name in service dependency definition";
    return (NULL);
  }

  // Allocate memory for a new service dependency entry.
  std::auto_ptr<servicedependency>
    new_servicedependency(new servicedependency);
  memset(
    new_servicedependency.get(),
    0,
    sizeof(*new_servicedependency));

  // Duplicate vars.
  new_servicedependency->dependent_host_name
    = my_strdup(dependent_host_name);
  new_servicedependency->dependent_service_description
    = my_strdup(dependent_service_description);
  new_servicedependency->host_name = my_strdup(host_name);
  new_servicedependency->service_description
    = my_strdup(service_description);
  if (dependency_period)
    new_servicedependency->dependency_period
      = my_strdup(dependency_period);
  new_servicedependency->dependency_type
    = (dependency_type == EXECUTION_DEPENDENCY)
      ? EXECUTION_DEPENDENCY
    : NOTIFICATION_DEPENDENCY;
  new_servicedependency->inherits_parent
    = (inherits_parent > 0) ? TRUE : FALSE;
  new_servicedependency->fail_on_ok = (fail_on_ok == 1) ? TRUE : FALSE;
  new_servicedependency->fail_on_warning
    = (fail_on_warning == 1) ? TRUE : FALSE;
  new_servicedependency->fail_on_unknown
    = (fail_on_unknown == 1) ? TRUE : FALSE;
  new_servicedependency->fail_on_critical
    = (fail_on_critical == 1) ? TRUE : FALSE;
  new_servicedependency->fail_on_pending
    = (fail_on_pending == 1) ? TRUE : FALSE;
  new_servicedependency->circular_path_checked = FALSE;
  new_servicedependency->contains_circular_path = FALSE;

  // Add new service dependency to skiplist.
  int result;
  result = skiplist_insert(
             object_skiplists[SERVICEDEPENDENCY_SKIPLIST],
             new_servicedependency.get());
  switch (result) {
  case SKIPLIST_OK:
    result = OK;
    break ;
  default:
    logger(log_config_error, basic)
      << "Error: Could not add service dependency to skiplist";
    result = ERROR;
    break ;
  }

  // Handle errors.
  if (result == ERROR) {
    delete [] new_servicedependency->host_name;
    delete [] new_servicedependency->service_description;
    delete [] new_servicedependency->dependent_host_name;
    delete [] new_servicedependency->dependent_service_description;
    return (NULL);
  }

  // Service dependencies are sorted alphabetically,
  // so add new items to tail of list.
  if (!servicedependency_list) {
    servicedependency_list = new_servicedependency.get();
    servicedependency_list_tail = servicedependency_list;
  }
  else {
    servicedependency_list_tail->next = new_servicedependency.get();
    servicedependency_list_tail = new_servicedependency.get();
  }

  // Notify event broker.
  // XXX

  return (new_servicedependency.release());
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
  std::auto_ptr<servicesmember> new_servicesmember(new servicesmember);
  memset(new_servicesmember.get(), 0, sizeof(*new_servicesmember));

  // Initialize values.
  new_servicesmember->service_ptr = svc;

  // Add the child entry to the host definition.
  new_servicesmember->next = hst->services;
  hst->services = new_servicesmember.get();

  // Notify event broker.
  // XXX

  return (new_servicesmember.release());
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
  std::auto_ptr<commandsmember> new_commandsmember(new commandsmember);
  memset(new_commandsmember.get(), 0, sizeof(*new_commandsmember));

  // Duplicate vars.
  new_commandsmember->cmd = my_strdup(command_name);

  // Add the notification command.
  new_commandsmember->next = cntct->service_notification_commands;
  cntct->service_notification_commands = new_commandsmember.get();

  // Notify event broker.
  // XXX

  return (new_commandsmember.release());
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
  std::auto_ptr<servicesmember> new_member(new servicesmember);
  memset(new_member.get(), 0, sizeof(*new_member));

  // Duplicate vars.
  new_member->host_name = my_strdup(host_name);
  new_member->service_description = my_strdup(svc_description);

  // Add new member to member list, sorted by host name then
  // service description.
  servicesmember* last_member(temp_servicegroup->members);
  servicesmember* temp_member;
  for (temp_member = temp_servicegroup->members;
       temp_member;
       temp_member = temp_member->next) {
    if (strcmp(new_member->host_name, temp_member->host_name) < 0) {
      new_member->next = temp_member;
      if (temp_member == temp_servicegroup->members)
        temp_servicegroup->members = new_member.get();
      else
        last_member->next = new_member.get();
      break ;
    }
    else if (!strcmp(new_member->host_name, temp_member->host_name)
             && (strcmp(
                   new_member->service_description,
                   temp_member->service_description) < 0)) {
      new_member->next = temp_member;
      if (temp_member == temp_servicegroup->members)
        temp_servicegroup->members = new_member.get();
      else
        last_member->next = new_member.get();
      break ;
    }
    else
      last_member = temp_member;
  }
  if (!temp_servicegroup->members) {
    new_member->next = NULL;
    temp_servicegroup->members = new_member.get();
  }
  else if (!temp_member) {
    new_member->next = NULL;
    last_member->next = new_member.get();
  }

  // Notify event broker.
  // XXX

  return (new_member.release());
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
  std::auto_ptr<serviceescalation>
    new_serviceescalation(new serviceescalation);
  memset(
    new_serviceescalation.get(),
    0,
    sizeof(*new_serviceescalation));

  // Duplicate vars.
  new_serviceescalation->host_name = my_strdup(host_name);
  new_serviceescalation->description = my_strdup(description);
  if (escalation_period)
    new_serviceescalation->escalation_period
      = my_strdup(escalation_period);
  new_serviceescalation->first_notification = first_notification;
  new_serviceescalation->last_notification = last_notification;
  new_serviceescalation->notification_interval
    = (notification_interval <= 0) ? 0 : notification_interval;
  new_serviceescalation->escalate_on_recovery
    = (escalate_on_recovery > 0) ? TRUE : FALSE;
  new_serviceescalation->escalate_on_warning
    = (escalate_on_warning > 0) ? TRUE : FALSE;
  new_serviceescalation->escalate_on_unknown
    = (escalate_on_unknown > 0) ? TRUE : FALSE;
  new_serviceescalation->escalate_on_critical
    = (escalate_on_critical > 0) ? TRUE : FALSE;

  // Add new serviceescalation to skiplist.
  int result;
  result = skiplist_insert(
             object_skiplists[SERVICEESCALATION_SKIPLIST],
             new_serviceescalation.get());
  switch (result) {
  case SKIPLIST_OK:
    result = OK;
    break ;
  default:
    logger(log_config_error, basic)
      << "Error: Could not add escalation for service '" << description
      << "' on host '" << host_name << "' to skiplist";
    result = ERROR;
    break ;
  }

  // Handle errors.
  if (result == ERROR) {
    delete [] new_serviceescalation->host_name;
    delete [] new_serviceescalation->description;
    delete [] new_serviceescalation->escalation_period;
    return (NULL);
  }

  // Service escalations are sorted alphabetically,
  // so add new items to tail of list.
  if (!serviceescalation_list) {
    serviceescalation_list = new_serviceescalation.get();
    serviceescalation_list_tail = serviceescalation_list;
  }
  else {
    serviceescalation_list_tail->next = new_serviceescalation.get();
    serviceescalation_list_tail = new_serviceescalation.get();
  }

  // Notify event broker.
  // XXX

  return (new_serviceescalation.release());
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
  std::auto_ptr<servicegroup> new_servicegroup(new servicegroup);
  memset(new_servicegroup.get(), 0, sizeof(*new_servicegroup));

  // Duplicate vars.
  new_servicegroup->group_name = my_strdup(name);
  new_servicegroup->alias = my_strdup((alias == NULL) ? name : alias);
  if (notes)
    new_servicegroup->notes = my_strdup(notes);
  if (notes_url)
    new_servicegroup->notes_url = my_strdup(notes_url);
  if (action_url)
    new_servicegroup->action_url = my_strdup(action_url);

  // Add new service group to skiplist.
  int result;
  result = skiplist_insert(
             object_skiplists[SERVICEGROUP_SKIPLIST],
             new_servicegroup.get());
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logger(log_config_error, basic)
      << "Error: Servicegroup '" << name << "' has already been defined";
    result = ERROR;
    break ;
  case SKIPLIST_OK:
    result = OK;
    break ;
  default:
    logger(log_config_error, basic)
      << "Error: Could not add servicegroup '" << name << "' to skiplist";
    result = ERROR;
    break ;
  }

  // Handle errors.
  if (result == ERROR) {
    delete [] new_servicegroup->alias;
    delete [] new_servicegroup->group_name;
    return (NULL);
  }

  // Servicegroups are sorted alphabetically,
  // so add new items to tail of list.
  if (!servicegroup_list) {
    servicegroup_list = new_servicegroup.get();
    servicegroup_list_tail = servicegroup_list;
  }
  else {
    servicegroup_list_tail->next = new_servicegroup.get();
    servicegroup_list_tail = new_servicegroup.get();
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group(
    NEBTYPE_SERVICEGROUP_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    new_servicegroup.get(),
    &tv);

  return (new_servicegroup.release());
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
  std::auto_ptr<timeperiod> new_timeperiod(new timeperiod);
  memset(new_timeperiod.get(), 0, sizeof(*new_timeperiod));

  // Copy string vars.
  new_timeperiod->name = my_strdup(name);
  new_timeperiod->alias = my_strdup(alias);

  // Add new timeperiod to skiplist.
  int result;
  result = skiplist_insert(
             object_skiplists[TIMEPERIOD_SKIPLIST],
             new_timeperiod.get());
  switch (result) {
  case SKIPLIST_ERROR_DUPLICATE:
    logger(log_config_error, basic)
      << "Error: Timeperiod '" << name << "' has already been defined";
    result = ERROR;
    break ;
  case SKIPLIST_OK:
    result = OK;
    break ;
  default:
    logger(log_config_error, basic)
      << "Error: Could not add timeperiod '" << name << "' to skiplist";
    result = ERROR;
    break ;
  }

  // Handle errors.
  if (result == ERROR) {
    delete [] new_timeperiod->alias;
    delete [] new_timeperiod->name;
    return (NULL);
  }

  // Timeperiods are registered alphabetically,
  // so add new items to tail of list.
  if (!timeperiod_list) {
    timeperiod_list = new_timeperiod.get();
    timeperiod_list_tail = timeperiod_list;
  }
  else {
    timeperiod_list_tail->next = new_timeperiod.get();
    timeperiod_list_tail = new_timeperiod.get();
  }

  // Notify event broker.
  // XXX

  return (new_timeperiod.release());
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
  std::auto_ptr<timerange> new_timerange(new timerange);
  new_timerange->range_start = start_time;
  new_timerange->range_end = end_time;

  // Add the new time range to the head of the range list for this day.
  new_timerange->next = period->days[day];
  period->days[day] = new_timerange.get();

  // Notify event broker.
  // XXX

  return (new_timerange.release());
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
    logger(log_config_error, basic) << "Error: Start time "
      << start_time << " is not valid for timeperiod";
    return (NULL);
  }
  if (end_time > 86400) {
    logger(log_config_error, basic) << "Error: End time "
      << end_time << " is not value for timeperiod";
    return (NULL);
  }

  // Allocate memory for the new time range.
  std::auto_ptr<timerange> new_timerange(new timerange);
  new_timerange->range_start = start_time;
  new_timerange->range_end = end_time;

  // Add the new time range to the head of the range
  // list for this date range.
  new_timerange->next = drange->times;
  drange->times = new_timerange.get();

  // Notify event broker.
  // XXX

  return (new_timerange.release());
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
  // Free memory for the timeperiod list.
  for (timeperiod *this_timeperiod(timeperiod_list), *next_timeperiod;
       this_timeperiod;
       this_timeperiod = next_timeperiod) {
    // Free the exception time ranges contained in this timeperiod.
    for (unsigned int x(0); x < DATERANGE_TYPES; ++x) {
      for (daterange
             *this_daterange(this_timeperiod->exceptions[x]),
             *next_daterange;
           this_daterange;
           this_daterange = next_daterange) {
        next_daterange = this_daterange->next;
        for (timerange
               *this_timerange(this_daterange->times),
               *next_timerange;
             this_timerange;
             this_timerange = next_timerange) {
          next_timerange = this_timerange->next;
          delete this_timerange;
        }
        delete this_daterange;
      }
    }

    // Free the day time ranges contained in this timeperiod.
    for (unsigned int x(0); x < 7; ++x) {
      for (timerange
             *this_timerange(this_timeperiod->days[x]),
             *next_timerange;
           this_timerange;
           this_timerange = next_timerange) {
        next_timerange = this_timerange->next;
        delete this_timerange;
      }
    }

    // Free exclusions.
    for (timeperiodexclusion
           *this_timeperiodexclusion(this_timeperiod->exclusions),
           *next_timeperiodexclusion;
         this_timeperiodexclusion;
         this_timeperiodexclusion = next_timeperiodexclusion) {
      next_timeperiodexclusion = this_timeperiodexclusion->next;
      delete [] this_timeperiodexclusion->timeperiod_name;
      delete this_timeperiodexclusion;
    }

    // Next timeperiod.
    next_timeperiod = this_timeperiod->next;

    // Free current timeperiod.
    delete [] this_timeperiod->name;
    delete [] this_timeperiod->alias;
    delete this_timeperiod;
  }
  timeperiod_list = NULL;

  // Free memory for the host list.
  for (host *this_host(host_list), *next_host;
       this_host;
       this_host = next_host) {
    // Free memory for parent hosts.
    for (hostsmember
           *this_hostsmember(this_host->parent_hosts),
           *next_hostsmember;
         this_hostsmember;
         this_hostsmember = next_hostsmember) {
      next_hostsmember = this_hostsmember->next;
      delete [] this_hostsmember->host_name;
      delete this_hostsmember;
    }

    // Free memory for child host links.
    for (hostsmember
           *this_hostsmember(this_host->child_hosts),
           *next_hostsmember;
         this_hostsmember;
         this_hostsmember = next_hostsmember) {
      next_hostsmember = this_hostsmember->next;
      delete [] this_hostsmember->host_name;
      delete this_hostsmember;
    }

    // Free memory for service links.
    for (servicesmember
           *this_servicesmember(this_host->services),
           *next_servicesmember;
         this_servicesmember;
         this_servicesmember = next_servicesmember) {
      next_servicesmember = this_servicesmember->next;
      delete [] this_servicesmember->host_name;
      delete [] this_servicesmember->service_description;
      delete this_servicesmember;
    }

    // Free memory for contact groups.
    for (contactgroupsmember
           *this_contactgroupsmember(this_host->contact_groups),
           *next_contactgroupsmember;
         this_contactgroupsmember;
         this_contactgroupsmember = next_contactgroupsmember) {
      next_contactgroupsmember = this_contactgroupsmember->next;
      delete [] this_contactgroupsmember->group_name;
      delete this_contactgroupsmember;
    }

    // Free memory for contacts.
    for (contactsmember
           *this_contactsmember(this_host->contacts),
           *next_contactsmember;
         this_contactsmember;
         this_contactsmember = next_contactsmember) {
      next_contactsmember = this_contactsmember->next;
      delete [] this_contactsmember->contact_name;
      delete this_contactsmember;
    }

    // Free memory for custom variables.
    for (customvariablesmember
           *this_customvariablesmember(this_host->custom_variables),
           *next_customvariablesmember;
         this_customvariablesmember;
         this_customvariablesmember = next_customvariablesmember) {
      next_customvariablesmember = this_customvariablesmember->next;
      delete [] this_customvariablesmember->variable_name;
      delete [] this_customvariablesmember->variable_value;
      delete this_customvariablesmember;
    }

    // Next host.
    next_host = this_host->next;

    // Free current host.
    delete [] this_host->name;
    delete [] this_host->display_name;
    delete [] this_host->alias;
    delete [] this_host->address;
    delete [] this_host->plugin_output;
    delete [] this_host->long_plugin_output;
    delete [] this_host->perf_data;
    free_objectlist(&this_host->hostgroups_ptr);
    delete [] this_host->check_period;
    delete [] this_host->host_check_command;
    delete [] this_host->event_handler;
    delete [] this_host->failure_prediction_options;
    delete [] this_host->notification_period;
    delete [] this_host->notes;
    delete [] this_host->notes_url;
    delete [] this_host->action_url;
    delete [] this_host->icon_image;
    delete [] this_host->icon_image_alt;
    delete [] this_host->vrml_image;
    delete [] this_host->statusmap_image;
    delete this_host;
  }
  host_list = NULL;

  // Free memory for the host group list.
  for (hostgroup *this_hostgroup(hostgroup_list), *next_hostgroup;
       this_hostgroup;
       this_hostgroup = next_hostgroup) {
    // Free memory for the group members.
    for (hostsmember
           *this_hostsmember(this_hostgroup->members),
           *next_hostsmember;
         this_hostsmember;
         this_hostsmember = next_hostsmember) {
      next_hostsmember = this_hostsmember->next;
      delete [] this_hostsmember->host_name;
      delete this_hostsmember;
    }

    // Next host group.
    next_hostgroup = this_hostgroup->next;

    // Free current host group.
    delete [] this_hostgroup->group_name;
    delete [] this_hostgroup->alias;
    delete [] this_hostgroup->notes;
    delete [] this_hostgroup->notes_url;
    delete [] this_hostgroup->action_url;
    delete this_hostgroup;
  }
  hostgroup_list = NULL;

  // Free memory for the service group list.
  for (servicegroup
         *this_servicegroup(servicegroup_list),
         *next_servicegroup;
       this_servicegroup;
       this_servicegroup = next_servicegroup) {
    // Free memory for the group members.
    for (servicesmember
           *this_servicesmember(this_servicegroup->members),
           *next_servicesmember;
         this_servicesmember;
         this_servicesmember = next_servicesmember) {
      next_servicesmember = this_servicesmember->next;
      delete [] this_servicesmember->host_name;
      delete [] this_servicesmember->service_description;
      delete this_servicesmember;
    }

    // Next service group.
    next_servicegroup = this_servicegroup->next;

    // Free current service group.
    delete [] this_servicegroup->group_name;
    delete [] this_servicegroup->alias;
    delete [] this_servicegroup->notes;
    delete [] this_servicegroup->notes_url;
    delete [] this_servicegroup->action_url;
    delete this_servicegroup;
  }
  servicegroup_list = NULL;

  // Free memory for the contact list.
  for (contact *this_contact(contact_list), *next_contact;
       this_contact;
       this_contact = next_contact) {
    // Free memory for the host notification commands.
    for (commandsmember
           *this_commandsmember(this_contact->host_notification_commands),
           *next_commandsmember;
         this_commandsmember;
         this_commandsmember = next_commandsmember) {
      next_commandsmember = this_commandsmember->next;
      if (this_commandsmember->cmd)
        delete [] this_commandsmember->cmd;
      delete this_commandsmember;
    }

    // Free memory for the service notification commands.
    for (commandsmember
           *this_commandsmember(this_contact->service_notification_commands),
           *next_commandsmember;
         this_commandsmember;
         this_commandsmember = next_commandsmember) {
      next_commandsmember = this_commandsmember->next;
      if (this_commandsmember->cmd)
        delete [] this_commandsmember->cmd;
      delete this_commandsmember;
    }

    // Free memory for custom variables.
    for (customvariablesmember
           *this_customvariablesmember(this_contact->custom_variables),
           *next_customvariablesmember;
         this_customvariablesmember;
         this_customvariablesmember = next_customvariablesmember) {
      next_customvariablesmember = this_customvariablesmember->next;
      delete [] this_customvariablesmember->variable_name;
      delete [] this_customvariablesmember->variable_value;
      delete this_customvariablesmember;
    }

    // Next contact.
    next_contact = this_contact->next;

    // Free current contact.
    delete [] this_contact->name;
    delete [] this_contact->alias;
    delete [] this_contact->email;
    delete [] this_contact->pager;
    for (unsigned int i(0); i < MAX_CONTACT_ADDRESSES; ++i)
      delete [] this_contact->address[i];
    delete [] this_contact->host_notification_period;
    delete [] this_contact->service_notification_period;
    free_objectlist(&this_contact->contactgroups_ptr);
    delete this_contact;
  }
  contact_list = NULL;

  // Free memory for the contact group list.
  for (contactgroup
         *this_contactgroup(contactgroup_list),
         *next_contactgroup;
       this_contactgroup;
       this_contactgroup = next_contactgroup) {
    // Free memory for the group members.
    for (contactsmember
           *this_contactsmember(this_contactgroup->members),
           *next_contactsmember;
         this_contactsmember;
         this_contactsmember = next_contactsmember) {
      next_contactsmember = this_contactsmember->next;
      delete [] this_contactsmember->contact_name;
      delete this_contactsmember;
    }

    // Next contact group.
    next_contactgroup = this_contactgroup->next;

    // Free current contact group.
    delete [] this_contactgroup->group_name;
    delete [] this_contactgroup->alias;
    delete this_contactgroup;
  }
  contactgroup_list = NULL;

  // Free memory for the service list.
  for (service *this_service(service_list), *next_service;
       this_service;
       this_service = next_service) {
    // Free memory for contact groups.
    for (contactgroupsmember
           *this_contactgroupsmember(this_service->contact_groups),
           *next_contactgroupsmember;
         this_contactgroupsmember;
         this_contactgroupsmember = next_contactgroupsmember) {
      next_contactgroupsmember = this_contactgroupsmember->next;
      delete [] this_contactgroupsmember->group_name;
      delete this_contactgroupsmember;
    }

    // Free memory for contacts.
    for (contactsmember
           *this_contactsmember(this_service->contacts),
           *next_contactsmember;
         this_contactsmember;
         this_contactsmember = next_contactsmember) {
      next_contactsmember = this_contactsmember->next;
      delete [] this_contactsmember->contact_name;
      delete this_contactsmember;
    }

    // Free memory for custom variables.
    for (customvariablesmember
           *this_customvariablesmember(this_service->custom_variables),
           *next_customvariablesmember;
         this_customvariablesmember;
         this_customvariablesmember = next_customvariablesmember) {
      next_customvariablesmember = this_customvariablesmember->next;
      delete [] this_customvariablesmember->variable_name;
      delete [] this_customvariablesmember->variable_value;
      delete this_customvariablesmember;
    }

    // Next service.
    next_service = this_service->next;

    // Free current service.
    delete [] this_service->host_name;
    delete [] this_service->description;
    delete [] this_service->display_name;
    delete [] this_service->service_check_command;
    delete [] this_service->plugin_output;
    delete [] this_service->long_plugin_output;
    delete [] this_service->perf_data;
    delete [] this_service->event_handler_args;
    delete [] this_service->check_command_args;
    free_objectlist(&this_service->servicegroups_ptr);
    delete [] this_service->notification_period;
    delete [] this_service->check_period;
    delete [] this_service->event_handler;
    delete [] this_service->failure_prediction_options;
    delete [] this_service->notes;
    delete [] this_service->notes_url;
    delete [] this_service->action_url;
    delete [] this_service->icon_image;
    delete [] this_service->icon_image_alt;
    delete this_service;
  }
  service_list = NULL;

  // Free memory for the command list.
  for (command *this_command(command_list), *next_command;
       this_command;
       this_command = next_command) {
    next_command = this_command->next;
    delete [] this_command->name;
    delete [] this_command->command_line;
    delete this_command;
  }
  command_list = NULL;

  // Free memory for the service escalation list.
  for (serviceescalation
         *this_serviceescalation(serviceescalation_list),
         *next_serviceescalation;
       this_serviceescalation;
       this_serviceescalation = next_serviceescalation) {
    // Free memory for the contact group members.
    for (contactgroupsmember
           *this_contactgroupsmember(this_serviceescalation->contact_groups),
           *next_contactgroupsmember;
         this_contactgroupsmember;
         this_contactgroupsmember = next_contactgroupsmember) {
      next_contactgroupsmember = this_contactgroupsmember->next;
      delete [] this_contactgroupsmember->group_name;
      delete this_contactgroupsmember;
    }

    // Free memory for contacts.
    for (contactsmember
           *this_contactsmember(this_serviceescalation->contacts),
           *next_contactsmember;
         this_contactsmember;
         this_contactsmember = next_contactsmember) {
      next_contactsmember = this_contactsmember->next;
      delete [] this_contactsmember->contact_name;
      delete this_contactsmember;
    }

    // Next service escalation.
    next_serviceescalation = this_serviceescalation->next;

    // Free current service escalation.
    delete [] this_serviceescalation->host_name;
    delete [] this_serviceescalation->description;
    delete [] this_serviceescalation->escalation_period;
    delete this_serviceescalation;
  }
  serviceescalation_list = NULL;

  // Free memory for the service dependency list.
  for (servicedependency
         *this_servicedependency(servicedependency_list),
         *next_servicedependency;
       this_servicedependency;
       this_servicedependency = next_servicedependency) {
    next_servicedependency = this_servicedependency->next;
    delete [] this_servicedependency->dependency_period;
    delete [] this_servicedependency->dependent_host_name;
    delete [] this_servicedependency->dependent_service_description;
    delete [] this_servicedependency->host_name;
    delete [] this_servicedependency->service_description;
    delete this_servicedependency;
  }
  servicedependency_list = NULL;

  // Free memory for the host dependency list.
  for (hostdependency
         *this_hostdependency(hostdependency_list),
         *next_hostdependency;
       this_hostdependency;
       this_hostdependency = next_hostdependency) {
    next_hostdependency = this_hostdependency->next;
    delete [] this_hostdependency->dependency_period;
    delete [] this_hostdependency->dependent_host_name;
    delete [] this_hostdependency->host_name;
    delete this_hostdependency;
  }
  hostdependency_list = NULL;

  // Free memory for the host escalation list.
  for (hostescalation
         *this_hostescalation(hostescalation_list),
         *next_hostescalation;
       this_hostescalation;
       this_hostescalation = next_hostescalation) {
    // Free memory for the contact group members.
    for (contactgroupsmember
           *this_contactgroupsmember(this_hostescalation->contact_groups),
           *next_contactgroupsmember;
         this_contactgroupsmember;
         this_contactgroupsmember = next_contactgroupsmember) {
      next_contactgroupsmember = this_contactgroupsmember->next;
      delete [] this_contactgroupsmember->group_name;
      delete this_contactgroupsmember;
    }

    // Free memory for contacts.
    for (contactsmember
           *this_contactsmember(this_hostescalation->contacts),
           *next_contactsmember;
         this_contactsmember;
         this_contactsmember = next_contactsmember) {
      next_contactsmember = this_contactsmember->next;
      delete [] this_contactsmember->contact_name;
      delete this_contactsmember;
    }

    // Next host escalation.
    next_hostescalation = this_hostescalation->next;

    // Free current host escalation.
    delete [] this_hostescalation->host_name;
    delete [] this_hostescalation->escalation_period;
    delete this_hostescalation;
  }
  hostescalation_list = NULL;

  // Free object skiplists.
  free_object_skiplists();

  return (OK);
}

/**************************************
*                                     *
*    Object Modification Functions    *
*                                     *
**************************************/

/**
 *  Modify an existing command.
 *
 *  @param[in] name  Command name.
 *  @param[in] value New command line.
 *
 *  @return OK on success.
 */
int modify_command(char const* name, char const* value) {
  // Make sure we have the data we need.
  if (!name || !name[0] || !value || !value[0]) {
    logger(log_config_error, basic)
      << "Error: Command name or command line is NULL";
    return (ERROR);
  }

  // Find command object.
  command* this_command(command_list);
  while (this_command && strcmp(this_command->name, name))
    this_command = this_command->next;
  if (!this_command)
    return (ERROR);

  // Modify command.
  delete [] this_command->command_line;
  this_command->command_line = my_strdup(value);

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_command_data(
    NEBTYPE_COMMAND_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this_command->name,
    this_command->command_line,
    &tv);

  return (OK);
}

/**************************************
*                                     *
*      Object Removal Functions       *
*                                     *
**************************************/

/**
 *  Remove a command.
 *
 *  @param[in] this_command Command to remove.
 *
 *  @return 1 on successful removal.
 */
static int remove_command(command* this_command) {
  // Update the command skiplist.
  skiplist_delete_all(
    object_skiplists[COMMAND_SKIPLIST],
    this_command);

  // Delete command.
  delete [] this_command->name;
  delete [] this_command->command_line;
  delete this_command;

  return (1);
}

/**
 *  Remove command by ID.
 *
 *  @param[in] name Command name.
 *
 *  @return 0 if command could not be found, 1 on successful removal
 *          2 if command is in use.
 */
int remove_command_by_id(char const* name) {
  if (!name)
    return (0);

  // Find command object.
  command* this_command(command_list);
  command* prev_command(NULL);
  while (this_command && strcmp(this_command->name, name)) {
    prev_command = this_command;
    this_command = this_command->next;
  }
  if (!this_command)
    return (0);

  // Check if command is used by a host.
  for (host* hst(host_list); hst; hst = hst->next)
    if ((hst->event_handler_ptr == this_command)
	|| (hst->check_command_ptr == this_command))
      return (2);

  // Check if command is used by a service.
  for (service* svc(service_list); svc; svc = svc->next)
    if ((svc->event_handler_ptr == this_command)
	|| (svc->check_command_ptr == this_command))
      return (2);

  // Check if command is used by a contact.
  for (contact* cntct(contact_list); cntct; cntct = cntct->next) {
    for (commandsmember* cntctsmember(cntct->host_notification_commands);
	 cntctsmember;
	 cntctsmember = cntctsmember->next)
      if (cntctsmember->command_ptr == this_command)
	return (2);
    for (commandsmember* cntctsmember(cntct->service_notification_commands);
         cntctsmember;
         cntctsmember = cntctsmember->next)
      if (cntctsmember->command_ptr == this_command)
	return (2);
  }

  // Update the command list.
  if (!prev_command)
    command_list = this_command->next;
  else
    prev_command->next = this_command->next;
  if (!this_command->next)
    command_list_tail = prev_command;

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_command_data(
    NEBTYPE_COMMAND_DELETE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this_command->name,
    this_command->command_line,
    &tv);

  return (remove_command(this_command));
}

/**
 *  Remove contact from a contactsmember list.
 *
 *  @param[in,out] cntctsmember Target list.
 *  @param[in]     cntct        Contact to remove.
 *
 *  @return 0 if contact was not found in list, 1 on successful removal.
 */
static int remove_contact_to_contactsmember(
             contactsmember** cntctsmember,
             contact* cntct) {
  // Find contact.
  contactsmember* this_item(*cntctsmember);
  contactsmember* prev_item(NULL);
  while (this_item && this_item->contact_ptr != cntct) {
    prev_item = this_item;
    this_item = this_item->next;
  }

  // Check we have find a contacts member.
  if (!this_item)
    return (0);

  // Update list.
  if (!prev_item)
    *cntctsmember = this_item->next;
  else
    prev_item->next = this_item->next;

  // Remove contact member.
  delete [] this_item->contact_name;
  delete this_item;

  return (1);
}

/**
 *  Remove a contact.
 *
 *  @param[in] this_contact Contact to remove.
 *
 *  @return 1 on successful removal.
 */
static int remove_contact(contact* this_contact) {
  // Remove contact from contactgroup.
  for (contactgroup* this_contactgroup(contactgroup_list);
       this_contactgroup;
       this_contactgroup = this_contactgroup->next)
    remove_contact_to_contactsmember(
      &this_contactgroup->members,
      this_contact);

  // Remove contact from host.
  for (host* this_host(host_list);
       this_host;
       this_host = this_host->next)
    remove_contact_to_contactsmember(
      &this_host->contacts,
      this_contact);

  // Remove contact from service.
  for (service* this_service(service_list);
       this_service;
       this_service = this_service->next)
    remove_contact_to_contactsmember(
      &this_service->contacts,
      this_contact);

  // Remove contact from hostescalation.
  for (hostescalation* this_hostescalation(hostescalation_list);
       this_hostescalation;
       this_hostescalation = this_hostescalation->next)
    remove_contact_to_contactsmember(
      &this_hostescalation->contacts,
      this_contact);

  // Remove contact from serviceescalation.
  for (serviceescalation* this_serviceescalation(serviceescalation_list);
       this_serviceescalation;
       this_serviceescalation = this_serviceescalation->next)
    remove_contact_to_contactsmember(
      &this_serviceescalation->contacts,
      this_contact);

  // Free memory for the host notification commands.
  for (commandsmember
         *this_commandsmember(this_contact->host_notification_commands),
         *next_commandsmember;
       this_commandsmember;
       this_commandsmember = next_commandsmember) {
    next_commandsmember = this_commandsmember->next;
    if (this_commandsmember->cmd)
      delete [] this_commandsmember->cmd;
    delete this_commandsmember;
  }

  // Free memory for the service notification commands.
  for (commandsmember
         *this_commandsmember(this_contact->service_notification_commands),
         *next_commandsmember;
       this_commandsmember;
       this_commandsmember = next_commandsmember) {
    next_commandsmember = this_commandsmember->next;
    if (this_commandsmember->cmd)
      delete [] this_commandsmember->cmd;
    delete this_commandsmember;
  }

  // Free memory for custom variables.
  for (customvariablesmember
         *this_customvariablesmember(this_contact->custom_variables),
         *next_customvariablesmember;
       this_customvariablesmember;
       this_customvariablesmember = next_customvariablesmember) {
    next_customvariablesmember = this_customvariablesmember->next;
    delete [] this_customvariablesmember->variable_name;
    delete [] this_customvariablesmember->variable_value;
    delete this_customvariablesmember;
  }

  // Update the contact skiplist.
  skiplist_delete_all(object_skiplists[CONTACT_SKIPLIST], this_contact);

  // Update the object list.
  free_objectlist(&this_contact->contactgroups_ptr);

  // Free remaining properties.
  for (unsigned int i(0); i < MAX_CONTACT_ADDRESSES; ++i)
    delete [] this_contact->address[i];
  delete [] this_contact->name;
  delete [] this_contact->alias;
  delete [] this_contact->email;
  delete [] this_contact->pager;
  delete [] this_contact->host_notification_period;
  delete [] this_contact->service_notification_period;
  delete this_contact;

  return (1);
}

/**
 *  Remove contact by ID.
 *
 *  @param[in] name Contact name.
 *
 *  @return 0 if contact could not be found and 1 on successful removal.
 */
int remove_contact_by_id(char const* name) {
  if (!name)
    return (0);

  // Find contact.
  contact* this_contact(contact_list);
  contact* prev_contact(NULL);
  while (this_contact && strcmp(this_contact->name, name)) {
    prev_contact = this_contact;
    this_contact = this_contact->next;
  }

  // check we have find a contact.
  if (!this_contact)
    return (0);

  // Update the contact list.
  if (!prev_contact)
    contact_list = this_contact->next;
  else
    prev_contact->next = this_contact->next;
  if (!this_contact->next)
    contact_list_tail = prev_contact;

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_contact_data(
    NEBTYPE_CONTACT_DELETE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this_contact,
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);

  return (remove_contact(this_contact));
}

/**
 *  Remove a contactgroup from a contactgroupsmember list.
 *
 *  @param[in,out] groupsmember Target list.
 *  @param[in]     group        Group to remove.
 *
 *  @return 1 on successful removal.
 */
static int remove_contactgroup_to_contactgroupsmember(
             contactgroupsmember** groupsmember,
             contactgroup* group) {
  // Find contact group member.
  contactgroupsmember* this_item(*groupsmember);
  contactgroupsmember* prev_item(NULL);
  while (this_item && (this_item->group_ptr != group)) {
    prev_item = this_item;
    this_item = this_item->next;
  }

  // Check we have find a contact groups member.
  if (!this_item)
    return (0);

  // Update list.
  if (!prev_item)
    *groupsmember = this_item->next;
  else
    prev_item->next = this_item->next;

  // Remove member.
  delete [] this_item->group_name;
  delete this_item;

  return (1);
}

/**
 *  Remove contact group.
 *
 *  @param[in] this_contactgroup Contact group to remove.
 *
 *  @return 1 on successful removal.
 */
static int remove_contactgroup(contactgroup* this_contactgroup) {
  // Check we have find a contact group.
  if (!this_contactgroup)
    return (0);

  // Remove contactgroup from host.
  for (host* this_host(host_list);
       this_host;
       this_host = this_host->next)
    remove_contactgroup_to_contactgroupsmember(
      &this_host->contact_groups,
      this_contactgroup);

  // Remove contactgroup from service.
  for (service* this_service(service_list);
       this_service;
       this_service = this_service->next)
    remove_contactgroup_to_contactgroupsmember(
      &this_service->contact_groups,
      this_contactgroup);

  // Remove contactgroup from hostescalation.
  for (hostescalation* this_hostescalation(hostescalation_list);
       this_hostescalation;
       this_hostescalation = this_hostescalation->next)
    remove_contactgroup_to_contactgroupsmember(
      &this_hostescalation->contact_groups,
      this_contactgroup);

  // Remove contactgroup from serviceescalation.
  for (serviceescalation* this_serviceescalation(serviceescalation_list);
       this_serviceescalation;
       this_serviceescalation = this_serviceescalation->next)
    remove_contactgroup_to_contactgroupsmember(
      &this_serviceescalation->contact_groups,
      this_contactgroup);

  // Update the contactgroup skiplist.
  skiplist_delete_all(
    object_skiplists[CONTACTGROUP_SKIPLIST],
    this_contactgroup);

  // Free remaining contact group properties.
  delete [] this_contactgroup->group_name;
  delete [] this_contactgroup->alias;
  delete this_contactgroup;

  return (1);
}

/**
 *  Remove contact group by ID.
 *
 *  @param[in] name Contact group name.
 *
 *  @return 0 if contact group could not be found, 1 on successful
 *          removal.
 */
int remove_contactgroup_by_id(char const* name) {
  if (!name)
    return (0);

  // Find contact group.
  contactgroup* this_contactgroup(contactgroup_list);
  contactgroup* prev_contactgroup(NULL);
  while (this_contactgroup
	 && strcmp(this_contactgroup->group_name, name)) {
    prev_contactgroup = this_contactgroup;
    this_contactgroup = this_contactgroup->next;
  }

  // Check we found a contact group.
  if (!this_contactgroup)
    return (0);

  // Update the contactgroup list.
  if (!prev_contactgroup)
    contactgroup_list = this_contactgroup->next;
  else
    prev_contactgroup->next = this_contactgroup->next;
  if (!this_contactgroup->next)
    contactgroup_list_tail = prev_contactgroup;

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group(
    NEBTYPE_CONTACTGROUP_DELETE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    this_contactgroup,
    &tv);

  return (remove_contactgroup(this_contactgroup));
}

/**
 *  Remove host.
 *
 *  @param[in] this_host Host to remove.
 *
 *  @return 1 on successful removal.
 */
static int remove_host(host* this_host) {
  // Check we have a host.
  if (!this_host)
    return (0);

  // Update the event list low.
  for (timed_event* temp_event(event_list_low);
       temp_event;
       temp_event = temp_event->next)
    if (temp_event->event_data == this_host) {
      remove_event(temp_event, &event_list_low, &event_list_low_tail);
      delete temp_event;
      break ;
    }

  // Update the event list high.
  for (timed_event* temp_event(event_list_high);
       temp_event;
       temp_event = temp_event->next)
    if (temp_event->event_data == this_host) {
      remove_event(temp_event, &event_list_high, &event_list_high_tail);
      delete temp_event;
      break ;
    }

  // Update the hostdependency list.
  for (hostdependency *dep(hostdependency_list), *prev(NULL);
       dep;
       dep = dep->next) {
    if ((dep->master_host_ptr == this_host)
        || (dep->dependent_host_ptr == this_host)) {
      skiplist_delete(object_skiplists[HOSTDEPENDENCY_SKIPLIST], dep);
      if (!prev)
	hostdependency_list = dep->next;
      else
	prev->next = dep->next;
      if (!dep->next)
	hostdependency_list_tail = prev;
      delete [] dep->dependent_host_name;
      delete [] dep->host_name;
      delete [] dep->dependency_period;
      delete dep;
      break ;
    }
    prev = dep;
  }

  // Update the hostescalation list.
  for (hostescalation *escalation(hostescalation_list), *prev(NULL);
       escalation;
       prev = escalation, escalation = escalation->next) {
    skiplist_delete(
      object_skiplists[HOSTESCALATION_SKIPLIST],
      escalation);
    if (escalation->host_ptr == this_host) {
      if (!prev)
	hostescalation_list = escalation->next;
      else
	prev->next = escalation->next;
      if (!escalation->next)
	hostescalation_list_tail = prev;
      delete [] escalation->host_name;
      delete [] escalation->escalation_period;
      delete escalation;
      break ;
    }
  }

  // Update the host skiplist.
  skiplist_delete_all(object_skiplists[HOST_SKIPLIST], this_host);

  // Free memory for parent hosts.
  for (hostsmember
         *this_hostsmember(this_host->parent_hosts),
         *next_hostsmember;
       this_hostsmember;
       this_hostsmember = next_hostsmember) {
    next_hostsmember = this_hostsmember->next;
    delete [] this_hostsmember->host_name;
    delete this_hostsmember;
  }

  // Free memory for child host links.
  for (hostsmember
         *this_hostsmember(this_host->child_hosts),
         *next_hostsmember;
       this_hostsmember;
       this_hostsmember = next_hostsmember) {
    next_hostsmember = this_hostsmember->next;
    delete [] this_hostsmember->host_name;
    delete this_hostsmember;
  }

  // Free memory for service links.
  for (servicesmember
         *this_servicesmember(this_host->services),
         *next_servicesmember;
       this_servicesmember;
       this_servicesmember = next_servicesmember) {
    next_servicesmember = this_servicesmember->next;
    service* svc(this_servicesmember->service_ptr);
    delete [] this_servicesmember->host_name;
    delete [] this_servicesmember->service_description;
    delete this_servicesmember;
    remove_service_by_id(svc->host_name, svc->description);
  }

  // Free memory for contact groups.
  for (contactgroupsmember
         *this_contactgroupsmember(this_host->contact_groups),
         *next_contactgroupsmember;
       this_contactgroupsmember;
       this_contactgroupsmember = next_contactgroupsmember) {
    next_contactgroupsmember = this_contactgroupsmember->next;
    delete [] this_contactgroupsmember->group_name;
    delete this_contactgroupsmember;
  }

  // Free memory for contacts.
  for (contactsmember
         *this_contactsmember(this_host->contacts),
         *next_contactsmember;
       this_contactsmember;
       this_contactsmember = next_contactsmember) {
    next_contactsmember = this_contactsmember->next;
    delete [] this_contactsmember->contact_name;
    delete this_contactsmember;
  }

  // Free memory for custom variables.
  for (customvariablesmember
         *this_customvariablesmember(this_host->custom_variables),
         *next_customvariablesmember;
       this_customvariablesmember;
       this_customvariablesmember = next_customvariablesmember) {
    next_customvariablesmember = this_customvariablesmember->next;
    delete [] this_customvariablesmember->variable_name;
    delete [] this_customvariablesmember->variable_value;
    delete this_customvariablesmember;
  }

  // Free remaining host properties.
  delete [] this_host->name;
  delete [] this_host->display_name;
  delete [] this_host->alias;
  delete [] this_host->address;
  delete [] this_host->plugin_output;
  delete [] this_host->long_plugin_output;
  delete [] this_host->perf_data;
  free_objectlist(&this_host->hostgroups_ptr);
  delete [] this_host->check_period;
  delete [] this_host->host_check_command;
  delete [] this_host->event_handler;
  delete [] this_host->failure_prediction_options;
  delete [] this_host->notification_period;
  delete [] this_host->notes;
  delete [] this_host->notes_url;
  delete [] this_host->action_url;
  delete [] this_host->icon_image;
  delete [] this_host->icon_image_alt;
  delete [] this_host->vrml_image;
  delete [] this_host->statusmap_image;
  delete this_host;

  return (1);
}

/**
 *  Remove host by ID.
 *
 *  @param[in] name Host name.
 *
 *  @return 1 on successful removal.
 */
int remove_host_by_id(char const* name) {
  if (!name)
    return (0);

  // Find host.
  host* this_host(host_list);
  host* prev_host(NULL);
  while (this_host && strcmp(this_host->name, name)) {
    prev_host = this_host;
    this_host = this_host->next;
  }

  // Check we have find a host.
  if (!this_host)
    return (0);

  // Update the host list.
  if (!prev_host)
    host_list = this_host->next;
  else
    prev_host->next = this_host->next;
  if (!this_host->next)
    host_list_tail = prev_host;

  return (remove_host(this_host));
}

/**
 *  Remove host dependency by ID.
 *
 *  @param[in] host_name           Host name.
 *  @param[in] dependent_host_name Dependant host name.
 *
 *  @return 0 on successful removal.
 */
int remove_host_dependency_by_id(
      char const* host_name,
      char const* dependent_host_name) {
  if (!host_name || !dependent_host_name)
     return (0);

  // Find host dependency.
  hostdependency* this_hostdependency(hostdependency_list);
  hostdependency* prev_hostdependency(NULL);
  while (this_hostdependency
	 && strcmp(this_hostdependency->host_name, host_name)
	 && strcmp(
              this_hostdependency->dependent_host_name,
              dependent_host_name)) {
    prev_hostdependency = this_hostdependency;
    this_hostdependency = this_hostdependency->next;
  }

  // Check we have find a hostdependency.
  if (!this_hostdependency)
    return (0);

  // Update the hostdependency list.
  if (!prev_hostdependency)
    hostdependency_list = this_hostdependency->next;
  else
    prev_hostdependency->next = this_hostdependency->next;
  if (!this_hostdependency->next)
    hostdependency_list_tail = prev_hostdependency;

  // Update the hostdependency skiplist.
  skiplist_delete_all(
    object_skiplists[HOSTDEPENDENCY_SKIPLIST],
    this_hostdependency);

  // Free dependency.
  delete [] this_hostdependency->dependent_host_name;
  delete [] this_hostdependency->host_name;
  delete [] this_hostdependency->dependency_period;
  delete this_hostdependency;

  return (1);
}

/**
 *  Remove host escalation by ID.
 *
 *  @param[in] host_name Host name.
 *
 *  @return 1 on successful removal.
 */
int remove_host_escalation_by_id(char const* host_name) {
  if (!host_name)
    return (0);

  // Find host escalation.
  hostescalation* this_hostescalation(hostescalation_list);
  hostescalation* prev_hostescalation(NULL);
  while (this_hostescalation
	 && strcmp(this_hostescalation->host_name, host_name)) {
    prev_hostescalation = this_hostescalation;
    this_hostescalation = this_hostescalation->next;
  }

  // Check we have find a hostescalation.
  if (!this_hostescalation)
    return (0);

  // Update the hostescalation list.
  if (!prev_hostescalation)
    hostescalation_list = this_hostescalation->next;
  else
    prev_hostescalation->next = this_hostescalation->next;
  if (!this_hostescalation->next)
    hostescalation_list_tail = prev_hostescalation;

  for (contactgroupsmember
         *this_contactgroupsmembers(this_hostescalation->contact_groups),
         *next_contactgroupsmembers;
       this_contactgroupsmembers;
       this_contactgroupsmembers = next_contactgroupsmembers) {
    next_contactgroupsmembers = this_contactgroupsmembers->next;
    delete [] this_contactgroupsmembers->group_name;
    delete this_contactgroupsmembers;
  }

  for (contactsmember
         *this_contactsmember(this_hostescalation->contacts),
         *next_contactsmember;
       this_contactsmember;
       this_contactsmember = next_contactsmember) {
    next_contactsmember = this_contactsmember->next;
    delete [] this_contactsmember->contact_name;
    delete this_contactsmember;
  }

  // Update the hostescalation skiplist.
  skiplist_delete_all(
    object_skiplists[HOSTESCALATION_SKIPLIST],
    this_hostescalation);

  delete [] this_hostescalation->host_name;
  delete [] this_hostescalation->escalation_period;
  delete this_hostescalation;

  return (1);
}

/**
 *  Remove host group.
 *
 *  @param[in] this_hostgroup Host group to remove.
 *
 *  @return 1 on successful removal.
 */
static int remove_host_group(hostgroup* this_hostgroup) {
  // Update host list.
  for (host* hst = host_list; hst; hst = hst->next)
    remove_object_to_objectlist(&hst->hostgroups_ptr, this_hostgroup);

  // Remove members.
  for (hostsmember
         *this_hostsmember(this_hostgroup->members),
         *next_hostsmember;
       this_hostsmember;
       this_hostsmember = next_hostsmember) {
    next_hostsmember = this_hostsmember->next;
    delete [] this_hostsmember->host_name;
    delete this_hostsmember;
  }

  // Update the hostgroup skiplist.
  skiplist_delete_all(
    object_skiplists[HOSTGROUP_SKIPLIST],
    this_hostgroup);

  delete [] this_hostgroup->group_name;
  delete [] this_hostgroup->alias;
  delete [] this_hostgroup->notes;
  delete [] this_hostgroup->notes_url;
  delete [] this_hostgroup->action_url;
  delete this_hostgroup;

  return (1);
}

/**
 *  Remove host group by ID.
 *
 *  @param[in] name Host group name.
 *
 *  @return 1 on successful removal.
 */
int remove_hostgroup_by_id(char const* name) {
  if (!name)
    return (0);

  // Find host group.
  hostgroup* this_hostgroup(hostgroup_list);
  hostgroup* prev_hostgroup(NULL);
  while (this_hostgroup && strcmp(this_hostgroup->group_name, name)) {
    prev_hostgroup = this_hostgroup;
    this_hostgroup = this_hostgroup->next;
  }

  // Check we have find a host group.
  if (!this_hostgroup)
    return (0);

  // Update the hostgroup list.
  if (!prev_hostgroup)
    hostgroup_list = this_hostgroup->next;
  else
    prev_hostgroup->next = this_hostgroup->next;
  if (!this_hostgroup->next)
    hostgroup_list_tail = prev_hostgroup;

  return (remove_host_group(this_hostgroup));
}

/**
 *  Remove service.
 *
 *  @param[in] this_service Service to remove.
 *
 *  @return 1 on successful removal.
 */
static int remove_service(service* this_service) {
  // Check we have a service.
  if (!this_service)
    return (0);

  // Update host service list.
  host* hst(find_host(this_service->host_name));
  if (hst) {
    for (servicesmember *svcmbr(hst->services), *prev(NULL);
	 svcmbr;
	 svcmbr = svcmbr->next) {
      if (svcmbr->service_ptr == this_service) {
	if (!prev)
	  hst->services = svcmbr->next;
	else
	  prev->next = svcmbr->next;
	delete [] svcmbr->host_name;
	delete [] svcmbr->service_description;
	delete svcmbr;
	break ;
      }
      prev = svcmbr;
    }
  }

  // Update the event list low.
  for (timed_event* temp_event(event_list_low);
       temp_event;
       temp_event = temp_event->next)
    if (temp_event->event_data == this_service) {
      remove_event(temp_event, &event_list_low, &event_list_low_tail);
      delete temp_event;
      break ;
    }

  // Update the event list high.
  for (timed_event* temp_event(event_list_high);
       temp_event;
       temp_event = temp_event->next)
    if (temp_event->event_data == this_service) {
      remove_event(temp_event, &event_list_high, &event_list_high_tail);
      delete temp_event;
      break ;
    }

  // Update the servicedependency list.
  for (servicedependency
         *dep(servicedependency_list),
         *prev(NULL);
       dep;
       dep = dep->next) {
    if ((dep->master_service_ptr == this_service)
        || (dep->dependent_service_ptr == this_service)) {
      skiplist_delete(
        object_skiplists[SERVICEDEPENDENCY_SKIPLIST],
        dep);
      if (!prev)
	servicedependency_list = dep->next;
      else
	prev->next = dep->next;
      if (!dep->next)
	servicedependency_list_tail = prev;
      delete [] dep->dependent_host_name;
      delete [] dep->dependent_service_description;
      delete [] dep->host_name;
      delete [] dep->service_description;
      delete [] dep->dependency_period;
      delete dep;
      break ;
    }
    prev = dep;
  }

  // Update the serviceescalation list.
  for (serviceescalation
         *escalation(serviceescalation_list),
         *prev(NULL);
       escalation;
       prev = escalation, escalation = escalation->next) {
    skiplist_delete(
      object_skiplists[SERVICEESCALATION_SKIPLIST],
      escalation);
    if (escalation->service_ptr == this_service) {
      if (!prev)
	serviceescalation_list = escalation->next;
      else
	prev->next = escalation->next;
      if (!escalation->next)
	serviceescalation_list_tail = prev;
      delete [] escalation->host_name;
      delete [] escalation->description;
      delete [] escalation->escalation_period;
      delete escalation;
      break ;
    }
  }

  // Update the service skiplist.
  skiplist_delete_all(object_skiplists[SERVICE_SKIPLIST], this_service);

  // Free memory for contact groups.
  for (contactgroupsmember
         *this_contactgroupsmember(this_service->contact_groups),
         *next_contactgroupsmember;
       this_contactgroupsmember;
       this_contactgroupsmember = next_contactgroupsmember) {
    next_contactgroupsmember = this_contactgroupsmember->next;
    delete [] this_contactgroupsmember->group_name;
    delete this_contactgroupsmember;
  }

  // Free memory for contacts.
  for (contactsmember
         *this_contactsmember(this_service->contacts),
         *next_contactsmember;
       this_contactsmember;
       this_contactsmember = next_contactsmember) {
    next_contactsmember = this_contactsmember->next;
    delete [] this_contactsmember->contact_name;
    delete this_contactsmember;
  }

  // Free memory for custom variables.
  for (customvariablesmember
         *this_customvariablesmember(this_service->custom_variables),
         *next_customvariablesmember;
       this_customvariablesmember;
       this_customvariablesmember = next_customvariablesmember) {
    next_customvariablesmember = this_customvariablesmember->next;
    delete [] this_customvariablesmember->variable_name;
    delete [] this_customvariablesmember->variable_value;
    delete this_customvariablesmember;
  }

  // Cleanup memory.
  delete [] this_service->host_name;
  delete [] this_service->description;
  delete [] this_service->display_name;
  delete [] this_service->service_check_command;
  delete [] this_service->plugin_output;
  delete [] this_service->long_plugin_output;
  delete [] this_service->perf_data;
  delete [] this_service->event_handler_args;
  delete [] this_service->check_command_args;
  free_objectlist(&this_service->servicegroups_ptr);
  delete [] this_service->notification_period;
  delete [] this_service->check_period;
  delete [] this_service->event_handler;
  delete [] this_service->failure_prediction_options;
  delete [] this_service->notes;
  delete [] this_service->notes_url;
  delete [] this_service->action_url;
  delete [] this_service->icon_image;
  delete [] this_service->icon_image_alt;
  delete this_service;

  return (1);
}

/**
 *  Remove service by ID.
 *
 *  @param[in] host_name   Host name.
 *  @param[in] description Service description.
 *
 *  @return 1 on successful removal.
 */
int remove_service_by_id(
      char const* host_name,
      char const* description) {
  if (!host_name || !description)
    return (0);

  // Find service.
  service* this_service(service_list);
  service* prev_service(NULL);
  while (this_service) {
    if (!strcmp(this_service->host_name, host_name)
	&& !strcmp(this_service->description, description))
      break ;
    prev_service = this_service;
    this_service = this_service->next;
  }

  // Check we have find a service.
  if (!this_service)
    return (0);

  // Remove service from host list.
  if (this_service->host_ptr) {
    servicesmember* current;
    servicesmember** prev;
    for (current = this_service->host_ptr->services,
           prev = &this_service->host_ptr->services;
         current;
         prev = &current->next, current = current->next)
      if (current->service_ptr == this_service)
        break ;
    if (current) {
      *prev = current->next;
      delete [] current->host_name;
      delete [] current->service_description;
      delete current;
    }
  }

  // Update the service list.
  if (!prev_service)
    service_list = this_service->next;
  else
    prev_service->next = this_service->next;
  if (!this_service->next)
    service_list_tail = prev_service;

  return (remove_service(this_service));
}

/**
 *  Remove a service dependency.
 *
 *  @param[in] host_name                     Host name.
 *  @param[in] service_description           Service description.
 *  @param[in] dependent_host_name           Dependent host name.
 *  @param[in] dependent_service_description Dependent service
 *                                           description.
 *
 *  @return 1 on successful removal.
 */
int remove_service_dependency_by_id(
      char const* host_name,
      char const* service_description,
      char const* dependent_host_name,
      char const* dependent_service_description) {
  if (!host_name
      || !service_description
      || !dependent_host_name
      || !dependent_service_description)
    return (0);

  // Find service dependency.
  servicedependency* this_servicedependency(servicedependency_list);
  servicedependency* prev_servicedependency(NULL);
  while (this_servicedependency
	 && strcmp(this_servicedependency->host_name, host_name)
	 && strcmp(
              this_servicedependency->service_description,
              service_description)
	 && strcmp(
              this_servicedependency->dependent_host_name,
              dependent_host_name)
	 && strcmp(
              this_servicedependency->dependent_service_description,
              dependent_service_description)) {
    prev_servicedependency = this_servicedependency;
    this_servicedependency = this_servicedependency->next;
  }

  // Check we have find a servicedependency.
  if (!this_servicedependency)
    return (0);

  // Update the servicedependency list.
  if (!prev_servicedependency)
    servicedependency_list = this_servicedependency->next;
  else
    prev_servicedependency->next = this_servicedependency->next;
  if (!this_servicedependency->next)
    servicedependency_list_tail = prev_servicedependency;

  // Update the servicedependency skiplist.
  skiplist_delete_all(
    object_skiplists[SERVICEDEPENDENCY_SKIPLIST],
    this_servicedependency);

  // Free service dependency.
  delete [] this_servicedependency->dependent_host_name;
  delete [] this_servicedependency->dependent_service_description;
  delete [] this_servicedependency->host_name;
  delete [] this_servicedependency->service_description;
  delete [] this_servicedependency->dependency_period;
  delete this_servicedependency;

  return (1);
}

/**
 *  Remove service escalation by ID.
 *
 *  @param[in] host_name           Host name.
 *  @param[in] service_description Service description.
 *
 *  @return 1 on successful removal.
 */
int remove_service_escalation_by_id(
      char const* host_name,
      char const* service_description) {
  if (!host_name || !service_description)
    return (0);

  // Find service escalation.
  serviceescalation* this_serviceescalation(serviceescalation_list);
  serviceescalation* prev_serviceescalation(NULL);
  while (this_serviceescalation
	 && strcmp(this_serviceescalation->host_name, host_name)
	 && strcmp(
              this_serviceescalation->description,
              service_description)) {
    prev_serviceescalation = this_serviceescalation;
    this_serviceescalation = this_serviceescalation->next;
  }

  // Check we have find a serviceescalation.
  if (!this_serviceescalation)
    return (0);

  // Update the serviceescalation list.
  if (!prev_serviceescalation)
    serviceescalation_list = this_serviceescalation->next;
  else
    prev_serviceescalation->next = this_serviceescalation->next;
  if (!this_serviceescalation->next)
    serviceescalation_list_tail = prev_serviceescalation;

  // Delete contact group members.
  for (contactgroupsmember
         *this_contactgroupsmembers(this_serviceescalation->contact_groups),
         *next_contactgroupsmembers;
       this_contactgroupsmembers;
       this_contactgroupsmembers = next_contactgroupsmembers) {
    next_contactgroupsmembers = this_contactgroupsmembers->next;
    delete [] this_contactgroupsmembers->group_name;
    delete this_contactgroupsmembers;
  }

  // Delete contact members.
  for (contactsmember
         *this_contactsmember(this_serviceescalation->contacts),
         *next_contactsmember;
       this_contactsmember;
       this_contactsmember = next_contactsmember) {
    next_contactsmember = this_contactsmember->next;
    delete [] this_contactsmember->contact_name;
    delete this_contactsmember;
  }

  // Update the serviceescalation skiplist.
  skiplist_delete_all(
    object_skiplists[SERVICEESCALATION_SKIPLIST],
    this_serviceescalation);

  // Free service escalation.
  delete [] this_serviceescalation->host_name;
  delete [] this_serviceescalation->description;
  delete [] this_serviceescalation->escalation_period;
  delete this_serviceescalation;

  return (1);
}

/**
 *  Remove service group.
 *
 *  @param[in] this_servicegroup Service group to remove.
 *
 *  @return 1 on successful removal.
 */
static int remove_service_group(servicegroup* this_servicegroup) {
  // Update service list.
  for (service* svc(service_list); svc; svc = svc->next)
    remove_object_to_objectlist(
      &svc->servicegroups_ptr,
      this_servicegroup);

  // Delete service members.
  for (servicesmember
         *this_servicesmember(this_servicegroup->members),
         *next_servicesmember;
       this_servicesmember;
       this_servicesmember = next_servicesmember) {
    next_servicesmember = this_servicesmember->next;
    delete [] this_servicesmember->host_name;
    delete [] this_servicesmember->service_description;
    delete this_servicesmember;
  }

  // Update the servicegroup skiplist.
  skiplist_delete_all(
    object_skiplists[SERVICEGROUP_SKIPLIST],
    this_servicegroup);

  // Delete service group.
  delete [] this_servicegroup->group_name;
  delete [] this_servicegroup->alias;
  delete [] this_servicegroup->notes;
  delete [] this_servicegroup->notes_url;
  delete [] this_servicegroup->action_url;
  delete this_servicegroup;

  return (1);
}

/**
 *  Remove service group by ID.
 *
 *  @param[in] name Name of service group to remove.
 *
 *  @return 1 on successful removal.
 */
int remove_servicegroup_by_id(char const* name) {
  if (!name)
    return (0);

  // Find service group.
  servicegroup* this_servicegroup(servicegroup_list);
  servicegroup* prev_servicegroup(NULL);
  while (this_servicegroup
         && strcmp(this_servicegroup->group_name, name)) {
    prev_servicegroup = this_servicegroup;
    this_servicegroup = this_servicegroup->next;
  }

  // Check we have find a service group.
  if (!this_servicegroup)
    return (0);

  // Update the servicegroup list.
  if (!prev_servicegroup)
    servicegroup_list = this_servicegroup->next;
  else
    prev_servicegroup->next = this_servicegroup->next;
  if (!this_servicegroup->next)
    servicegroup_list_tail = prev_servicegroup;

  return (remove_service_group(this_servicegroup));
}

/**
 *  Remove a timerange.
 *
 *  @param[in] this_timerange Timerange to remove.
 *
 *  @return 1.
 */
static int remove_timerange(timerange* this_timerange) {
  while (this_timerange) {
    timerange* tmp(this_timerange->next);
    delete this_timerange;
    this_timerange = tmp;
  }
  return (1);
}

/**
 *  Remove a daterange.
 *
 *  @param[in] this_daterange Daterange to remove.
 *
 *  @return 1.
 */
static int remove_daterange(daterange* this_daterange) {
  while (this_daterange) {
    daterange* tmp(this_daterange->next);
    remove_timerange(this_daterange->times);
    delete this_daterange;
    this_daterange = tmp;
  }
  return (1);
}

/**
 *  Remove a timeperiod exclusion.
 *
 *  @param[in] this_timeperiodexclusion Timeperiod exclusion to remove.
 *
 *  @return 1.
 */
static int remove_timeperiodexclusions(
             timeperiodexclusion* this_timeperiodexclusion) {
  while (this_timeperiodexclusion) {
    timeperiodexclusion* tmp(this_timeperiodexclusion->next);
    delete [] this_timeperiodexclusion->timeperiod_name;
    delete this_timeperiodexclusion;
    this_timeperiodexclusion = tmp;
  }
  return (1);
}

/**
 *  Remove a timeperiod.
 *
 *  @param[in] this_timeperiod Timeperiod to remove.
 *
 *  @return 1 on successful removal.
 */
static int remove_timeperiod(timeperiod* this_timeperiod) {
  if (!this_timeperiod)
    return (0);

  // Remove all timerange.
  for (unsigned int i(0);
       i < sizeof(this_timeperiod->days) / sizeof(*this_timeperiod->days);
       ++i)
    remove_timerange(this_timeperiod->days[i]);

  // Remove all exceptions.
  for (unsigned int i(0);
       i < sizeof(this_timeperiod->exceptions) / sizeof(*this_timeperiod->exceptions);
       ++i)
    remove_daterange(this_timeperiod->exceptions[i]);

  // Remove all exclusions.
  remove_timeperiodexclusions(this_timeperiod->exclusions);

  // Remove all timeperiod used by contacts.
  for (contact* cntct(contact_list); cntct; cntct = cntct->next) {
    if (cntct->host_notification_period_ptr == this_timeperiod)
      cntct->host_notification_period_ptr = NULL;
    if (cntct->service_notification_period_ptr == this_timeperiod)
      cntct->service_notification_period_ptr = NULL;
  }

  // Remove all timeperiod used by hosts.
  for (host* hst(host_list); hst; hst = hst->next) {
    if (hst->check_period_ptr == this_timeperiod)
      hst->check_period_ptr = NULL;
    if (hst->notification_period_ptr == this_timeperiod)
      hst->notification_period_ptr = NULL;
  }

  // Remove all timeperiod used by services.
  for (service* svc(service_list); svc; svc = svc->next) {
    if (svc->check_period_ptr == this_timeperiod)
      svc->check_period_ptr = NULL;
    if (svc->notification_period_ptr == this_timeperiod)
      svc->notification_period_ptr = NULL;
  }

  // Remove all timeperiod used by serviceescalations.
  for (serviceescalation* se(serviceescalation_list);
       se;
       se = se->next)
    if (se->escalation_period_ptr == this_timeperiod)
      se->escalation_period_ptr = NULL;

  // Remove all timeperiod used by servicedependencies.
  for (servicedependency* sd(servicedependency_list);
       sd;
       sd = sd->next)
    if (sd->dependency_period_ptr == this_timeperiod)
      sd->dependency_period_ptr = NULL;

  // Remove all timeperiod used by hostescalations.
  for (hostescalation* he(hostescalation_list); he; he = he->next)
    if (he->escalation_period_ptr == this_timeperiod)
      he->escalation_period_ptr = NULL;

  // Remove all timeperiod used by hostdependencies.
  for (hostdependency* hd(hostdependency_list); hd; hd = hd->next)
    if (hd->dependency_period_ptr == this_timeperiod)
      hd->dependency_period_ptr = NULL;

  // Update the timeperiod skiplist.
  skiplist_delete_all(
    object_skiplists[TIMEPERIOD_SKIPLIST],
    this_timeperiod);

  // Remove timeperiod properties.
  delete [] this_timeperiod->name;
  delete [] this_timeperiod->alias;
  delete this_timeperiod;

  return (1);
}

/**
 *  Remove timeperiod by ID.
 *
 *  @param[in] name Timeperiod name.
 *
 *  @return 1 on successful timeperiod removal.
 */
int remove_timeperiod_by_id(char const* name) {
  if (!name)
    return (0);

  // Find timeperiod.
  timeperiod* this_timeperiod(timeperiod_list);
  timeperiod* prev_timeperiod(NULL);
  while (this_timeperiod
	 && strcmp(this_timeperiod->name, name)) {
    prev_timeperiod = this_timeperiod;
    this_timeperiod = this_timeperiod->next;
  }

  // Check we have find a timeperiod.
  if (!this_timeperiod)
    return (0);

  // Update the timeperiod list.
  if (!prev_timeperiod)
    timeperiod_list = this_timeperiod->next;
  else
    prev_timeperiod->next = this_timeperiod->next;
  if (!this_timeperiod->next)
    timeperiod_list_tail = prev_timeperiod;

  return (remove_timeperiod(this_timeperiod));
}

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
  return (object_skiplists[HOST_SKIPLIST]
          ? object_skiplists[HOST_SKIPLIST]->items
          : 0);
}

/**
 *  Get number of registered services.
 *
 *  @return Number of registered services.
 */
int get_service_count() {
  return (object_skiplists[SERVICE_SKIPLIST]
          ? object_skiplists[SERVICE_SKIPLIST]->items
          : 0);
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
  for (host* temp_host(host_list);
       temp_host;
       temp_host = temp_host->next)
    if (is_host_immediate_child_of_host(hst, temp_host) == TRUE)
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
  for (host* temp_host(host_list);
       temp_host;
       temp_host = temp_host->next)
    if (is_host_immediate_parent_of_host(hst, temp_host) == TRUE)
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
  for (host* temp_host(host_list);
       temp_host;
       temp_host = temp_host->next)
    if (is_host_immediate_child_of_host(hst, temp_host) == TRUE)
      children += number_of_total_child_hosts(temp_host) + 1;
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
  for (host* temp_host(host_list);
       temp_host;
       temp_host = temp_host->next)
    if (is_host_immediate_parent_of_host(hst, temp_host) == TRUE)
      parents += number_of_total_parent_hosts(temp_host) + 1;
  return (parents);
}

/**************************************
*                                     *
*       Object Search Functions       *
*                                     *
**************************************/

/**
 *  Given a command name, find a command from the list in memory.
 *
 *  @param[in] name Command name.
 *
 *  @return Command object if found, NULL otherwise.
 */
command* find_command(char const* name) {
  if (!name)
    return (NULL);

  command temp_command;
  temp_command.name = const_cast<char*>(name);
  return (static_cast<command*>(skiplist_find_first(
                                  object_skiplists[COMMAND_SKIPLIST],
                                  &temp_command,
                                  NULL)));
}

/**
 *  Find a contact from the list in memory.
 *
 *  @param[in] name Contact name.
 *
 *  @return Contact object if found, NULL otherwise.
 */
contact* find_contact(char const* name) {
  if (!name)
    return (NULL);

  contact temp_contact;
  temp_contact.name = const_cast<char*>(name);
  return (static_cast<contact*>(skiplist_find_first(
                                  object_skiplists[CONTACT_SKIPLIST],
                                  &temp_contact,
                                  NULL)));
}

/**
 *  Find a contact group from the list in memory.
 *
 *  @param[in] name Contact group name.
 *
 *  @return Contact group object if found, NULL otherwise.
 */
contactgroup* find_contactgroup(char const* name) {
  if (!name)
    return (NULL);

  contactgroup temp_contactgroup;
  temp_contactgroup.group_name = const_cast<char*>(name);
  return (static_cast<contactgroup*>(
            skiplist_find_first(
              object_skiplists[CONTACTGROUP_SKIPLIST],
              &temp_contactgroup,
              NULL)));
}

/**
 *  Given a host name, find it in the list in memory.
 *
 *  @param[in] name Host name.
 *
 *  @return Host object if found, NULL otherwise.
 */
host* find_host(char const* name) {
  if (!name)
    return (NULL);

  host temp_host;
  temp_host.name = const_cast<char*>(name);
  return (static_cast<host*>(skiplist_find_first(
                               object_skiplists[HOST_SKIPLIST],
                               &temp_host,
                               NULL)));
}

/**
 *  Find a hostgroup from the list in memory.
 *
 *  @param[in] name Host group name.
 *
 *  @return Host group object if found, NULL otherwise.
 */
hostgroup* find_hostgroup(char const* name) {
  if (!name)
    return (NULL);

  hostgroup temp_hostgroup;
  temp_hostgroup.group_name = const_cast<char*>(name);
  return (static_cast<hostgroup*>(
            skiplist_find_first(
              object_skiplists[HOSTGROUP_SKIPLIST],
              &temp_hostgroup,
              NULL)));
}

/**
 *  Given a host/service name, find the service in the list in memory.
 *
 *  @param[in] host_name Host name.
 *  @param[in] svc_desc  Service description.
 *
 *  @return Service object if found, NULL otherwise.
 */
service* find_service(char const* host_name, char const* svc_desc) {
  if (!host_name || !svc_desc)
    return (NULL);

  service temp_service;
  temp_service.host_name = const_cast<char*>(host_name);
  temp_service.description = const_cast<char*>(svc_desc);
  return (static_cast<service*>(skiplist_find_first(
                                  object_skiplists[SERVICE_SKIPLIST],
                                  &temp_service,
                                  NULL)));
}

/**
 *  Find a servicegroup from the list in memory.
 *
 *  @param[in] name Service group name.
 *
 *  @return Service group object if found, NULL otherwise.
 */
servicegroup* find_servicegroup(char const* name) {
  if (!name)
    return (NULL);

  servicegroup temp_servicegroup;
  temp_servicegroup.group_name = const_cast<char*>(name);
  return (static_cast<servicegroup*>(
            skiplist_find_first(
              object_skiplists[SERVICEGROUP_SKIPLIST],
              &temp_servicegroup,
              NULL)));
}

/**
 *  Given a timeperiod name, find the timeperiod from the list in memory.
 *
 *  @param[in] name Timeperiod name.
 *
 *  @return Timeperiod object if found, NULL otherwise.
 */
timeperiod* find_timeperiod(char const* name) {
  if (!name)
    return (NULL);

  timeperiod temp_timeperiod;
  temp_timeperiod.name = const_cast<char*>(name);
  return (static_cast<timeperiod*>(
            skiplist_find_first(
              object_skiplists[TIMEPERIOD_SKIPLIST],
              &temp_timeperiod,
              NULL)));
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
 *  @return TRUE or FALSE.
 */
int is_contact_for_host(host* hst, contact* cntct) {
  if (!hst || !cntct)
    return (FALSE);

  // Search all individual contacts of this host.
  for (contactsmember* temp_contactsmember(hst->contacts);
       temp_contactsmember;
       temp_contactsmember = temp_contactsmember->next) {
    contact* temp_contact(temp_contactsmember->contact_ptr);
    if (!temp_contact)
      continue ;
    if (temp_contact == cntct)
      return (TRUE);
  }

  // Search all contactgroups of this host.
  for (contactgroupsmember*
         temp_contactgroupsmember(hst->contact_groups);
       temp_contactgroupsmember;
       temp_contactgroupsmember = temp_contactgroupsmember->next) {
    contactgroup*
      temp_contactgroup(temp_contactgroupsmember->group_ptr);
    if (!temp_contactgroup)
      continue ;
    if (is_contact_member_of_contactgroup(temp_contactgroup, cntct))
      return (TRUE);
  }

  return (FALSE);
}

/**
 *  Tests whether a contact is a contact for a particular service.
 *
 *  @param[in] svc   Target service.
 *  @param[in] cntct Target contact.
 *
 *  @return TRUE or FALSE.
 */
int is_contact_for_service(service* svc, contact* cntct) {
  if (!svc || !cntct)
    return (FALSE);

  // Search all individual contacts of this service.
  for (contactsmember* temp_contactsmember(svc->contacts);
       temp_contactsmember;
       temp_contactsmember = temp_contactsmember->next) {
    contact* temp_contact(temp_contactsmember->contact_ptr);
    if (temp_contact == cntct)
      return (TRUE);
  }

  // Search all contactgroups of this service.
  for (contactgroupsmember*
         temp_contactgroupsmember(svc->contact_groups);
       temp_contactgroupsmember;
       temp_contactgroupsmember = temp_contactgroupsmember->next) {
    contactgroup*
      temp_contactgroup(temp_contactgroupsmember->group_ptr);
    if (!temp_contactgroup)
      continue ;
    if (is_contact_member_of_contactgroup(temp_contactgroup, cntct))
      return (TRUE);
  }

  return (FALSE);
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
 *  @return TRUE or FALSE.
 */
int is_contact_member_of_contactgroup(
      contactgroup* group,
      contact* cntct) {
  if (!group || !cntct)
    return (FALSE);

  // Search all contacts in this contact group.
  for (contactsmember* member(group->members);
       member;
       member = member->next) {
    contact* temp_contact(member->contact_ptr);
    if (!temp_contact)
      continue ;
    if (temp_contact == cntct)
      return (TRUE);
  }
  return (FALSE);
}

/**
 *  Tests whether or not a contact is an escalated contact for a
 *  particular host.
 *
 *  @param[in] hst   Target host.
 *  @param[in] cntct Target contact. 
 *
 *  @return TRUE or FALSE.
 */
int is_escalated_contact_for_host(host* hst, contact* cntct) {
  // Search all host escalations.
  void* ptr(NULL);
  for (hostescalation*
         temp_hostescalation(
           get_first_host_escalation_by_host(hst->name, &ptr));
       temp_hostescalation;
       temp_hostescalation
         = get_next_host_escalation_by_host(hst->name, &ptr)) {
    // Search all contacts of this host escalation.
    for (contactsmember*
           temp_contactsmember(temp_hostescalation->contacts);
         temp_contactsmember;
         temp_contactsmember = temp_contactsmember->next) {
      contact* temp_contact(temp_contactsmember->contact_ptr);
      if (!temp_contact)
        continue ;
      if (temp_contact == cntct)
        return (TRUE);
    }

    // Search all contactgroups of this host escalation.
    for (contactgroupsmember* temp_contactgroupsmember(
                                temp_hostescalation->contact_groups);
         temp_contactgroupsmember;
         temp_contactgroupsmember = temp_contactgroupsmember->next) {
      contactgroup*
        temp_contactgroup(temp_contactgroupsmember->group_ptr);
      if (!temp_contactgroup)
        continue ;
      if (is_contact_member_of_contactgroup(temp_contactgroup, cntct))
        return (TRUE);
    }
  }

  return (FALSE);
}

/**
 *  Tests whether or not a contact is an escalated contact for a
 *  particular service.
 *
 *  @param[in] svc   Target service.
 *  @param[in] cntct Target contact.
 *
 *  @return TRUE or FALSE.
 */
int is_escalated_contact_for_service(service* svc, contact* cntct) {
  // Search all the service escalations.
  void* ptr(NULL);
  for (serviceescalation* temp_serviceescalation(
                            get_first_service_escalation_by_service(
                              svc->host_name,
                              svc->description,
                              &ptr));
       temp_serviceescalation;
       temp_serviceescalation = get_next_service_escalation_by_service(
                                  svc->host_name,
                                  svc->description,
                                  &ptr)) {
    // Search all contacts of this service escalation.
    for (contactsmember*
           temp_contactsmember(temp_serviceescalation->contacts);
         temp_contactsmember;
         temp_contactsmember = temp_contactsmember->next) {
      contact* temp_contact(temp_contactsmember->contact_ptr);
      if (!temp_contact)
        continue ;
      if (temp_contact == cntct)
        return (TRUE);
    }

    // Search all contactgroups of this service escalation.
    for (contactgroupsmember* temp_contactgroupsmember(
                                temp_serviceescalation->contact_groups);
         temp_contactgroupsmember;
         temp_contactgroupsmember = temp_contactgroupsmember->next) {
      contactgroup*
        temp_contactgroup(temp_contactgroupsmember->group_ptr);
      if (!temp_contactgroup)
        continue ;
      if (is_contact_member_of_contactgroup(temp_contactgroup, cntct))
        return (TRUE);
    }
  }

  return (FALSE);
}

/**
 *  Determines whether or not a specific host is an immediate child of
 *  another host.
 *
 *  @param[in] parent_host Parent host.
 *  @param[in] child_host  Child host.
 *
 *  @return TRUE or FALSE.
 */
int is_host_immediate_child_of_host(
      host* parent_host,
      host* child_host) {
  // Not enough data.
  if (!child_host)
    return (FALSE);

  // Root/top-level hosts.
  if (!parent_host) {
    if (!child_host->parent_hosts)
      return (TRUE);
  }
  // Mid-level/bottom hosts.
  else {
    for (hostsmember* temp_hostsmember(child_host->parent_hosts);
         temp_hostsmember;
         temp_hostsmember = temp_hostsmember->next)
      if (temp_hostsmember->host_ptr == parent_host)
        return (TRUE);
  }

  return (FALSE);
}

/**
 *  Determines whether or not a specific host is an immediate parent of
 *  another host.
 *
 *  @param[in] child_host  Child host.
 *  @param[in] parent_host Parent host.
 *
 *  @return TRUE or FALSE.
 */
int is_host_immediate_parent_of_host(
      host* child_host,
      host* parent_host) {
  if (is_host_immediate_child_of_host(parent_host, child_host) == TRUE)
    return (TRUE);
  return (FALSE);
}

/**
 *  Tests whether a host is a member of a particular hostgroup.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] group Target host group.
 *  @param[in] hst   Target host.
 *
 *  @return TRUE or FALSE.
 */
int is_host_member_of_hostgroup(hostgroup* group, host* hst) {
  if (!group || !hst)
    return (FALSE);

  for (hostsmember* temp_hostsmember(group->members);
       temp_hostsmember;
       temp_hostsmember = temp_hostsmember->next)
    if (temp_hostsmember->host_ptr == hst)
      return (TRUE);
  return (FALSE);
}

/**
 *  Tests whether a host is a member of a particular servicegroup.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] group Target service group.
 *  @param[in] hst   Target host.
 *
 *  @return TRUE or FALSE.
 */
int is_host_member_of_servicegroup(servicegroup* group, host* hst) {
  if (!group || !hst)
    return (FALSE);

  for (servicesmember* temp_servicesmember(group->members);
       temp_servicesmember;
       temp_servicesmember = temp_servicesmember->next)
    if (temp_servicesmember->service_ptr
        && (temp_servicesmember->service_ptr->host_ptr == hst))
      return (TRUE);
  return (FALSE);
}

/**
 *  Tests whether a service is a member of a particular servicegroup.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] group Target group.
 *  @param[in] svc   Target service.
 *
 *  @return TRUE or FALSE.
 */
int is_service_member_of_servicegroup(
      servicegroup* group,
      service* svc) {
  if (!group || !svc)
    return (FALSE);

  for (servicesmember* temp_servicesmember(group->members);
       temp_servicesmember;
       temp_servicesmember = temp_servicesmember->next)
    if (temp_servicesmember->service_ptr == svc)
      return (TRUE);
  return (FALSE);
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
 *  @return TRUE if circular path was found, FALSE otherwise.
 */
int check_for_circular_hostdependency_path(
      hostdependency* root_dep,
      hostdependency* dep,
      int dependency_type) {
  if (!root_dep || !dep)
    return (FALSE);

  // This is not the proper dependency type.
  if ((root_dep->dependency_type != dependency_type)
      || (dep->dependency_type != dependency_type))
    return (FALSE);

  // Don't go into a loop, don't bother checking anymore if we know this
  // dependency already has a loop.
  if (root_dep->contains_circular_path == TRUE)
    return (TRUE);

  // Dependency has already been checked - there is a path somewhere,
  // but it may not be for this particular dep... This should speed up
  // detection for some loops.
  if (dep->circular_path_checked == TRUE)
    return (FALSE);

  // Set the check flag so we don't get into an infinite loop.
  dep->circular_path_checked = TRUE;

  // Is this host dependent on the root host?
  if (dep != root_dep) {
    if (root_dep->dependent_host_ptr == dep->master_host_ptr) {
      root_dep->contains_circular_path = TRUE;
      dep->contains_circular_path = TRUE;
      return (TRUE);
    }
  }

  // Notification dependencies are ok at this point as long as they
  // don't inherit.
  if ((dependency_type == NOTIFICATION_DEPENDENCY)
      && (dep->inherits_parent == FALSE))
    return (FALSE);

  // Check all parent dependencies.
  for (hostdependency* temp_hd(hostdependency_list);
       temp_hd;
       temp_hd = temp_hd->next) {
    // Only check parent dependencies.
    if (dep->master_host_ptr != temp_hd->dependent_host_ptr)
      continue ;

    if (check_for_circular_hostdependency_path(
          root_dep,
          temp_hd,
          dependency_type) == TRUE)
      return (TRUE);
  }

  return (FALSE);
}

/**
 *  Checks to see if there exists a circular dependency for a service.
 *
 *  @param[in] root_dep        Root dependency.
 *  @param[in] dep             Dependency.
 *  @param[in] dependency_type Dependency type.
 *
 *  @return TRUE if circular path was found, FALSE otherwise.
 */
int check_for_circular_servicedependency_path(
      servicedependency* root_dep,
      servicedependency* dep,
      int dependency_type) {
  if (!root_dep || !dep)
    return (FALSE);

  // This is not the proper dependency type.
  if ((root_dep->dependency_type != dependency_type)
      || (dep->dependency_type != dependency_type))
    return (FALSE);

  // Don't go into a loop, don't bother checking anymore if we know this
  // dependency already has a loop.
  if (root_dep->contains_circular_path == TRUE)
    return (TRUE);

  // Dependency has already been checked - there is a path somewhere,
  // but it may not be for this particular dep... This should speed up
  // detection for some loops.
  if (dep->circular_path_checked == TRUE)
    return (FALSE);

  // Set the check flag so we don't get into an infinite loop.
  dep->circular_path_checked = TRUE;

  // Is this service dependent on the root service?
  if (dep != root_dep) {
    if (root_dep->dependent_service_ptr == dep->master_service_ptr) {
      root_dep->contains_circular_path = TRUE;
      dep->contains_circular_path = TRUE;
      return (TRUE);
    }
  }

  // Notification dependencies are ok at this point as long as they
  // don't inherit.
  if ((dependency_type == NOTIFICATION_DEPENDENCY)
      && (dep->inherits_parent == FALSE))
    return (FALSE);

  // Check all parent dependencies.
  for (servicedependency* temp_sd(servicedependency_list);
       temp_sd;
       temp_sd = temp_sd->next) {
    // Only check parent dependencies.
    if (dep->master_service_ptr != temp_sd->dependent_service_ptr)
      continue ;

    if (check_for_circular_servicedependency_path(
          root_dep,
          temp_sd,
          dependency_type) == TRUE)
      return (TRUE);
  }

  return (FALSE);
}

/**************************************
*                                     *
*      Object Skiplist Functions      *
*                                     *
**************************************/

/**
 *  Free all object skiplists.
 *
 *  @return OK.
 */
int free_object_skiplists() {
  for (unsigned int x(0); x < NUM_OBJECT_SKIPLISTS; ++x)
    skiplist_free(&object_skiplists[x]);
  return (OK);
}

/**
 *  Initialize all object skiplists.
 *
 *  @return OK on successful initialization.
 */
int init_object_skiplists() {
  // Reset pointers.
  for (unsigned int x(0); x < NUM_OBJECT_SKIPLISTS; ++x)
    object_skiplists[x] = NULL;

  // Allocate skiplists.
  object_skiplists[HOST_SKIPLIST]
    = skiplist_new(15, 0.5, FALSE, FALSE, skiplist_compare_host);
  object_skiplists[SERVICE_SKIPLIST]
    = skiplist_new(15, 0.5, FALSE, FALSE, skiplist_compare_service);
  object_skiplists[COMMAND_SKIPLIST]
    = skiplist_new(10, 0.5, FALSE, FALSE, skiplist_compare_command);
  object_skiplists[TIMEPERIOD_SKIPLIST]
    = skiplist_new(10, 0.5, FALSE, FALSE, skiplist_compare_timeperiod);
  object_skiplists[CONTACT_SKIPLIST]
    = skiplist_new(10, 0.5, FALSE, FALSE, skiplist_compare_contact);
  object_skiplists[CONTACTGROUP_SKIPLIST]
    = skiplist_new(10, 0.5, FALSE, FALSE, skiplist_compare_contactgroup);
  object_skiplists[HOSTGROUP_SKIPLIST]
    = skiplist_new(10, 0.5, FALSE, FALSE, skiplist_compare_hostgroup);
  object_skiplists[SERVICEGROUP_SKIPLIST]
    = skiplist_new(10, 0.5, FALSE, FALSE, skiplist_compare_servicegroup);
  object_skiplists[HOSTESCALATION_SKIPLIST]
    = skiplist_new(15, 0.5, TRUE, FALSE, skiplist_compare_hostescalation);
  object_skiplists[SERVICEESCALATION_SKIPLIST]
    = skiplist_new(15, 0.5, TRUE, FALSE, skiplist_compare_serviceescalation);
  object_skiplists[HOSTDEPENDENCY_SKIPLIST]
    = skiplist_new(15, 0.5, TRUE, FALSE, skiplist_compare_hostdependency);
  object_skiplists[SERVICEDEPENDENCY_SKIPLIST]
    = skiplist_new(15, 0.5, TRUE, FALSE, skiplist_compare_servicedependency);

  return (OK);
}

/**
 *  Compare two commands.
 *
 *  @param[in] a Uncasted command #1.
 *  @param[in] b Uncasted command #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_command(void const* a, void const* b) {
  command const* oa(static_cast<command const*>(a));
  command const* ob(static_cast<command const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

/**
 *  Compare two contacts.
 *
 *  @param[in] a Uncasted command #1.
 *  @param[in] b Uncasted command #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_contact(void const* a, void const* b) {
  contact const* oa(static_cast<contact const*>(a));
  contact const* ob(static_cast<contact const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

/**
 *  Compare two contact groups.
 *
 *  @param[in] a Uncasted contactgroup #1.
 *  @param[in] b Uncasted contactgroup #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_contactgroup(void const* a, void const* b) {
  contactgroup const* oa(static_cast<contactgroup const*>(a));
  contactgroup const* ob(static_cast<contactgroup const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(
            oa->group_name,
            NULL,
            ob->group_name,
            NULL));
}

/**
 *  Compare two hosts.
 *
 *  @param[in] a Uncasted host #1.
 *  @param[in] b Uncasted host #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_host(void const* a, void const* b) {
  host const* oa(static_cast<host const*>(a));
  host const* ob(static_cast<host const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
}

/**
 *  Compare two host dependencies.
 *
 *  @param[in] a Uncasted host dependency #1.
 *  @param[in] b Uncasted host dependency #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_hostdependency(void const* a, void const* b) {
  hostdependency const* oa(static_cast<hostdependency const*>(a));
  hostdependency const* ob(static_cast<hostdependency const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(
            oa->dependent_host_name,
            NULL,
            ob->dependent_host_name,
            NULL));
}

/**
 *  Compare two host escalations.
 *
 *  @param[in] a Uncasted host escalation #1.
 *  @param[in] b Uncasted host escalation #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_hostescalation(void const* a, void const* b) {
  hostescalation const* oa(static_cast<hostescalation const*>(a));
  hostescalation const* ob(static_cast<hostescalation const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(
            oa->host_name,
            NULL,
            ob->host_name,
            NULL));
}

/**
 *  Compare two host groups.
 *
 *  @param[in] a Uncasted host group #1.
 *  @param[in] b Uncasted host group #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_hostgroup(void const* a, void const* b) {
  hostgroup const* oa(static_cast<hostgroup const*>(a));
  hostgroup const* ob(static_cast<hostgroup const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(
            oa->group_name,
            NULL,
            ob->group_name,
            NULL));
}

/**
 *  Compare two services.
 *
 *  @param[in] a Uncasted service #1.
 *  @param[in] b Uncasted service #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_service(void const* a, void const* b) {
  service const* oa(static_cast<service const*>(a));
  service const* ob(static_cast<service const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(
            oa->host_name,
            oa->description,
            ob->host_name,
            ob->description));
}

/**
 *  Compare two service dependencies.
 *
 *  @param[in] a Uncasted service #1.
 *  @param[in] b Uncasted service #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_servicedependency(void const* a, void const* b) {
  servicedependency const* oa(static_cast<servicedependency const*>(a));
  servicedependency const* ob(static_cast<servicedependency const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(
            oa->dependent_host_name,
            oa->dependent_service_description,
            ob->dependent_host_name,
            ob->dependent_service_description));
}

/**
 *  Compare two service escalations.
 *
 *  @param[in] a Uncasted service escalation #1.
 *  @param[in] b Uncasted service escalation #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_serviceescalation(void const* a, void const* b) {
  serviceescalation const* oa(static_cast<serviceescalation const*>(a));
  serviceescalation const* ob(static_cast<serviceescalation const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(
            oa->host_name,
            oa->description,
            ob->host_name,
            ob->description));
}

/**
 *  Compare two service groups.
 *
 *  @param[in] a Uncasted service group #1.
 *  @param[in] b Uncasted service group #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_servicegroup(void const* a, void const* b) {
  servicegroup const* oa(static_cast<servicegroup const*>(a));
  servicegroup const* ob(static_cast<servicegroup const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(
            oa->group_name,
            NULL,
            ob->group_name,
            NULL));
}

/**
 *  Compare four strings, two by two.
 *
 *  @param[in] val1a Compared to val2a.
 *  @param[in] val1b Compared to val2b.
 *  @param[in] val2a Compared to val1a.
 *  @param[in] val2b Compared to val1b.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_text(
      char const* val1a,
      char const* val1b,
      char const* val2a,
      char const* val2b) {
  int result(0);

  // Check first name.
  if (!val1a && !val2a)
    result = 0;
  else if (!val1a)
    result = 1;
  else if (!val2a)
    result = -1;
  else
    result = strcmp(val1a, val2a);

  // Check second name if necessary.
  if (result == 0) {
    if (!val1b && !val2b)
      result = 0;
    else if (!val1b)
      result = 1;
    else if (!val2b)
      result = -1;
    else
      result = strcmp(val1b, val2b);
  }

  return (result);
}

/**
 *  Compare two timeperiods.
 *
 *  @param[in] a Uncasted timeperiod #1.
 *  @param[in] b Uncasted timeperiod #2.
 *
 *  @return Similar to strcmp.
 */
int skiplist_compare_timeperiod(void const* a, void const* b) {
  timeperiod const* oa(static_cast<timeperiod const*>(a));
  timeperiod const* ob(static_cast<timeperiod const*>(b));
  if (!oa && !ob)
    return (0);
  if (!oa)
    return (1);
  if (!ob)
    return (-1);
  return (skiplist_compare_text(oa->name, NULL, ob->name, NULL));
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
  objectlist* temp_item;
  for (temp_item = *list; temp_item; temp_item = temp_item->next)
    if (temp_item->object_ptr == object_ptr)
      break ;
  if (temp_item)
    return (OK);

  // Allocate memory for a new list item.
  std::auto_ptr<objectlist> new_item(new objectlist);

  // Initialize vars.
  new_item->object_ptr = object_ptr;

  // Add new item to head of list.
  new_item->next = *list;
  *list = new_item.release();

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

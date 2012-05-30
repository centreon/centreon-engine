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

#include <map>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/webservice/commands.hh"
#include "com/centreon/engine/modules/webservice/create_object.hh"
#include "com/centreon/engine/modules/webservice/sync_lock.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/objects/contact.hh"
#include "soapH.h"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules;
using namespace com::centreon::engine::modules::webservice;

/**
 *  Find target contact.
 *
 *  @param[in] name Target contact name.
 *
 *  @return Contact object.
 */
static contact* find_target_contact(char const* name) {
  contact* cntct(find_contact(name));
  if (!cntct)
    throw (engine_error() << "Contact '" << name << "' not found");
  return (cntct);
}

/**
 *  Create a new contact into the engine.
 *
 *  @param[in] cntct The struct with all information to create new contact.
 */
void webservice::create_contact(ns1__contactType const& cntct) {
  /*
  ** Check all arguments and set default option for optional options.
  */

  // Host notification options.
  std::map<char, bool> host_opt(get_options(
                                  &cntct.hostNotificationOptions,
                                  "rdufs",
                                  "n"));
  if (host_opt.empty())
    throw (engine_error() << "contact '" << cntct.name
           << "' has invalid host notification options");

  // Service notification options.
  std::map<char, bool> service_opt(get_options(
                                     &cntct.serviceNotificationOptions,
                                     "rcwufs",
                                     "n"));
  if (service_opt.empty())
    throw (engine_error() << "contact '" << cntct.name
           << "' has invalid service notification options");

  // Host notification period.
  timeperiod* host_notification_period(
                find_timeperiod(cntct.hostNotificationPeriod.c_str()));
  if (!host_notification_period)
    throw (engine_error() << "contact '" << cntct.name
           << "' has invalid host notification period '"
           << cntct.hostNotificationPeriod << "'");

  // Service notification period.
  timeperiod* service_notification_period(
                find_timeperiod(cntct.serviceNotificationPeriod.c_str()));
  if (!service_notification_period)
    throw (engine_error() << "contact '" << cntct.name
           << "' has invalid service notification period '"
           << cntct.serviceNotificationPeriod << "'");

  // Other options.
  char const* email(cntct.email ? cntct.email->c_str() : NULL);
  char const* pager(cntct.pager ? cntct.pager->c_str() : NULL);
  char const* alias(cntct.alias ? cntct.alias->c_str() : NULL);
  bool can_submit_commands(
         cntct.canSubmitCommands ? *cntct.canSubmitCommands : true);
  bool retain_status_information(
         cntct.retainStatusInformation
         ? *cntct.retainStatusInformation
         : true);
  bool retain_nonstatus_information(
         cntct.retainNonstatusInformation
         ? *cntct.retainNonstatusInformation
         : true);

  // Create address array.
  QScopedArrayPointer<char const*>
    address(new char const*[MAX_CONTACT_ADDRESSES]);
  memset(address.data(), 0, sizeof(*address) * MAX_CONTACT_ADDRESSES);
  for (unsigned int i(0), end(cntct.address.size()); i < end; ++i)
    address[i] = cntct.address[i].c_str();

  // Create a new contact.
  contact* new_cntct(add_contact(
                       cntct.name.c_str(),
                       alias,
                       email,
                       pager,
                       (cntct.address.empty() ? NULL : &(*address)),
                       cntct.serviceNotificationPeriod.c_str(),
                       cntct.hostNotificationPeriod.c_str(),
                       service_opt['r'],
                       service_opt['c'],
                       service_opt['w'],
                       service_opt['u'],
                       service_opt['f'],
                       service_opt['s'],
                       host_opt['r'],
                       host_opt['d'],
                       host_opt['u'],
                       host_opt['f'],
                       host_opt['s'],
                       cntct.hostNotificationsEnabled,
                       cntct.serviceNotificationsEnabled,
                       can_submit_commands,
                       retain_status_information,
                       retain_nonstatus_information));

  // Find contact groups.
  QVector<contactgroup*> cntct_contactgroups(
                           _find<contactgroup>(
                             cntct.contactgroups,
                             (void* (*)(char const*))&find_contactgroup));
  if (cntct.contactgroups.size()
      != static_cast<size_t>(cntct_contactgroups.size())) {
    objects::release(new_cntct);
    throw (engine_error() << "contact '" << cntct.name
           << "' has invalid contactgroup");
  }

  // Find host notification commands.
  QVector<command*> cntct_host_notification_commands(
                      _find<command>(
                        cntct.hostNotificationCommands,
                        (void* (*)(char const*))&find_command));
  if (cntct.hostNotificationCommands.size()
      != static_cast<size_t>(cntct_host_notification_commands.size())) {
    objects::release(new_cntct);
    throw (engine_error() << "contact '" << cntct.name
           << "' has invalid host notification commands");
  }

  // Find service notification commands.
  QVector<command*> cntct_service_notification_commands(
                      _find<command>(
                        cntct.serviceNotificationCommands,
                        (void* (*)(char const*))&find_command));
  if (cntct.serviceNotificationCommands.size()
      != static_cast<size_t>(cntct_service_notification_commands.size())) {
    objects::release(new_cntct);
    throw (engine_error() << "contact '" << cntct.name
           << "' has invalid service notification commands");
  }

  // Generate custom variables.
  QVector<QString> cntct_customvar(std2qt(cntct.customVariables));

  try {
    // Link object.
    objects::link(
               new_cntct,
               host_notification_period,
               service_notification_period,
               cntct_contactgroups,
               cntct_host_notification_commands,
               cntct_service_notification_commands,
               cntct_customvar);
  }
  catch (std::exception const& e) {
    (void)e;
    objects::release(new_cntct);
    throw ;
  }

  return ;
}

/**
 *  Add a new contact.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  contact Contact to add.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactAdd(
      soap* s,
      ns1__contactType* contact,
      centreonengine__contactAddResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact->name)

  // Create new contact.
  create_contact(*contact);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the contact alias.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] alias      Contact alias.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetAlias(
      soap* s,
      ns1__contactIDType* contact_id,
      std::string& alias) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set alias.
  if (cntct->alias)
    alias = cntct->alias;
  else
    alias.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if the contact can submit external commands.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] can_submit Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetCanSubmitCommands(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& can_submit) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set value.
  can_submit = cntct->can_submit_commands;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the contact email.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] email      Email.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetEmail(
      soap* s,
      ns1__contactIDType* contact_id,
      std::string& email) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set email.
  if (cntct->email)
    email = cntct->email;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the contact modified attributes.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] modattr    Modified attributes.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetModifiedAttributes(
      soap* s,
      ns1__contactIDType* contact_id,
      unsigned int& modattr) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set modified attributes.
  modattr = cntct->modified_attributes;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the contact host modified attributes.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] modhattr   Modified host attributes.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetModifiedAttributesHost(
      soap* s,
      ns1__contactIDType* contact_id,
      unsigned int& modhattr) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set modified host attributes.
  modhattr = cntct->modified_host_attributes;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the contact service modified attributes.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] modsattr   Modified service attributes.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetModifiedAttributesService(
      soap* s,
      ns1__contactIDType* contact_id,
      unsigned int& modsattr) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set modified service attributes.
  modsattr = cntct->modified_service_attributes;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the host notification command of the contact.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] cmd        Host notification command.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnHostCommand(
      soap* s,
      ns1__contactIDType* contact_id,
      centreonengine__contactGetNotificationsOnHostCommandResponse& cmd) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set notification command.
  if (cntct->host_notification_commands->cmd) {
    cmd.command = soap_new_ns1__commandIDType(s, 1);
    cmd.command->command = cntct->host_notification_commands->cmd;
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified by down hosts.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] notified   Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnHostDown(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& notified) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  notified = cntct->notify_on_host_down;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified by hosts downtimes.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] notified   Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnHostDowntime(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& notified) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  notified = cntct->notify_on_host_downtime;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact will be notified by host events.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] notified   Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnHostEnabled(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& notified) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  notified = cntct->host_notifications_enabled;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when hosts are flapping.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] notified   Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnHostFlapping(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& notified) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  notified = cntct->notify_on_host_flapping;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last time the contact received a host notification.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] last_time  Requested time.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnHostLast(
      soap* s,
      ns1__contactIDType* contact_id,
      unsigned long& last_time) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set time.
  last_time = cntct->last_host_notification;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when hosts recover.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] notified   Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnHostRecovery(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& notified) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean flag.
  notified = cntct->notify_on_host_recovery;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the host notification timeperiod of the contact.
 *
 *  @param[in]  s           SOAP object.
 *  @param[in]  contact_id  Target contact.
 *  @param[out] time_period Timeperiod.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnHostTimeperiod(
      soap* s,
      ns1__contactIDType* contact_id,
      centreonengine__contactGetNotificationsOnHostTimeperiodResponse& time_period) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set timeperiod.
  if (cntct->host_notification_period_ptr->name) {
    time_period.val = soap_new_ns1__timeperiodIDType(s, 1);
    time_period.val->timeperiod
      = cntct->host_notification_period_ptr->name;
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when hosts are unreachable.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] notified   Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnHostUnreachable(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& notified) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  notified = cntct->notify_on_host_unreachable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the service notification command of the contact.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] cmd        Service notification command.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnServiceCommand(
      soap* s,
      ns1__contactIDType* contact_id,
      centreonengine__contactGetNotificationsOnServiceCommandResponse& cmd) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set notification command.
  if (cntct->service_notification_commands->cmd) {
    cmd.command = soap_new_ns1__commandIDType(s, 1);
    cmd.command->command = cntct->service_notification_commands->cmd;
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when services are critical.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] notified   Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnServiceCritical(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& notified) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  notified = cntct->notify_on_service_critical;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified by services downtimes.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] notified   Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnServiceDowntime(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& notified) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  notified = cntct->notify_on_service_downtime;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact will be notified by service events.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] notified   Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnServiceEnabled(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& notified) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  notified = cntct->service_notifications_enabled;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when services are flapping.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] notified   Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnServiceFlapping(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& notified) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  notified = cntct->notify_on_service_flapping;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last time the contact received a service notification.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] last_time  Requested time.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnServiceLast(
      soap* s,
      ns1__contactIDType* contact_id,
      unsigned long& last_time) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set requested time.
  last_time = cntct->last_service_notification;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when services recover.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] notified   Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnServiceRecovery(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& notified) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  notified = cntct->notify_on_service_recovery;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the service notification timeperiod of the contact.
 *
 *  @param[in]  s           SOAP object.
 *  @param[in]  contact_id  Target contact.
 *  @param[out] time_period Service notification timeperiod.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnServiceTimeperiod(
      soap* s,
      ns1__contactIDType* contact_id,
      centreonengine__contactGetNotificationsOnServiceTimeperiodResponse& time_period) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set timeperiod.
  if (cntct->service_notification_period_ptr->name) {
    time_period.val = soap_new_ns1__timeperiodIDType(s, 1);
    time_period.val->timeperiod
      = cntct->service_notification_period_ptr->name;
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when services states are unknown.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] notified   Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnServiceUnknown(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& notified) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  notified = cntct->notify_on_service_unknown;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact should be notified when services states are warning.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] notified   Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetNotificationsOnServiceWarning(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& notified) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  notified = cntct->notify_on_service_warning;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the contact pager.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] pager      Pager.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetPager(
      soap* s,
      ns1__contactIDType* contact_id,
      std::string& pager) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set pager.
  if (cntct->pager)
    pager = cntct->pager;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact status information should be retained.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] retain     Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetRetainStatusInformation(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& retain) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  retain = cntct->retain_status_information;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if contact non status information should be retained.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[out] retain     Boolean value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactGetRetainStatusNonInformation(
      soap* s,
      ns1__contactIDType* contact_id,
      bool& retain) {
  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  retain = cntct->retain_nonstatus_information;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Modify a contact.
 *
 *  @param[in]  s     SOAP object.
 *  @param[in]  cntct Contact information.
 *  @param[out] res   Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactModify(
      soap* s,
      ns1__contactType* cntct,
      centreonengine__contactModifyResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(cntct->name)

  // XXX

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Remove a contact.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Contact to remove.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactRemove(
      soap* s,
      ns1__contactIDType* contact_id,
      centreonengine__contactRemoveResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Remove contact.
  if (!remove_contact_by_id(contact_id->contact.c_str()))
    throw (engine_error() << "Contact '" << contact_id->contact
           << "' not found");

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the contact alias.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  alias      New contact alias.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetAlias(
      soap* s,
      ns1__contactIDType* contact_id,
      std::string alias,
      centreonengine__contactSetAliasResponse& res) {
  (void)res;

  // Begin try block
  COMMAND_BEGIN(contact_id->contact << ", " << alias)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set new alias.
  delete [] cntct->alias;
  cntct->alias = my_strdup(alias.c_str());

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable a contact to submit commands.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetCanSubmitCommands(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetCanSubmitCommandsResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->can_submit_commands = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the email address of a contact.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  email      New email.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetEmail(
      soap* s,
      ns1__contactIDType* contact_id,
      std::string email,
      centreonengine__contactSetEmailResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << email)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set new email.
  delete [] cntct->email;
  cntct->email = my_strdup(email.c_str());

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on host down.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnHostDown(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetNotificationsOnHostDownResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->notify_on_host_down = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on host downtime.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnHostDowntime(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetNotificationsOnHostDowntimeResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->notify_on_host_downtime = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on host.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnHostEnabled(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetNotificationsOnHostEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->host_notifications_enabled = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on flappy hosts.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnHostFlapping(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetNotificationsOnHostFlappingResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->notify_on_host_flapping = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on host recover.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnHostRecovery(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetNotificationsOnHostRecoveryResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->notify_on_host_recovery = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the host notifications timeperiod of the contact.
 *
 *  @param[in]  s             SOAP object.
 *  @param[in]  contact_id    Target contact.
 *  @param[in]  timeperiod_id New host notification timeperiod.
 *  @param[out] res           Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnHostTimeperiod(
      soap* s,
      ns1__contactIDType* contact_id,
      ns1__timeperiodIDType* timeperiod_id,
      centreonengine__contactSetNotificationsOnHostTimeperiodResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << timeperiod_id->timeperiod)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set new timeperiod.
  delete [] cntct->host_notification_period;
  cntct->host_notification_period
    = my_strdup(timeperiod_id->timeperiod.c_str());

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on host unreachable.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnHostUnreachable(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetNotificationsOnHostUnreachableResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->notify_on_host_unreachable = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service critical.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnServiceCritical(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetNotificationsOnServiceCriticalResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->notify_on_service_critical = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service downtime.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnServiceDowntime(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetNotificationsOnServiceDowntimeResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->notify_on_service_downtime = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnServiceEnabled(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetNotificationsOnServiceEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->service_notifications_enabled = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service flapping.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnServiceFlapping(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetNotificationsOnServiceFlappingResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->notify_on_service_flapping = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service recovery.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnServiceRecovery(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetNotificationsOnServiceRecoveryResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->notify_on_service_recovery = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the service notification timeperiod of the contact.
 *
 *  @param[in]  s             SOAP object.
 *  @param[in]  contact_id    Target contact.
 *  @param[in]  timeperiod_id Service notification timeperiod.
 *  @param[out] res           Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnServiceTimeperiod(
      soap* s,
      ns1__contactIDType* contact_id,
      ns1__timeperiodIDType* timeperiod_id,
      centreonengine__contactSetNotificationsOnServiceTimeperiodResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << timeperiod_id->timeperiod)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set new timeperiod.
  delete [] cntct->service_notification_period;
  cntct->service_notification_period
    = my_strdup(timeperiod_id->timeperiod.c_str());

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service unknown.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnServiceUnknown(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetNotificationsOnServiceUnknownResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->notify_on_service_unknown = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on service warning.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnServiceWarning(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetNotificationsOnServiceWarningResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->notify_on_service_warning = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the contact pager.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  pager      New contact pager.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetPager(
      soap* s,
      ns1__contactIDType* contact_id,
      std::string pager,
      centreonengine__contactSetPagerResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << pager)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set new pager.
  delete [] cntct->pager;
  cntct->pager = my_strdup(pager.c_str());

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable status information retention for contact.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetRetainStatusInformation(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetRetainStatusInformationResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean vlaue.
  cntct->retain_status_information = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable non status information retention for contact.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  enable     true to enable, false to disable.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetRetainStatusNonInformation(
      soap* s,
      ns1__contactIDType* contact_id,
      bool enable,
      centreonengine__contactSetRetainStatusNonInformationResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << enable)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set boolean value.
  cntct->retain_nonstatus_information = enable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the contact notification command for host events.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  command_id New host notification command.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnHostCommand(
      soap* s,
      ns1__contactIDType* contact_id,
      ns1__commandIDType* command_id,
      centreonengine__contactSetNotificationsOnHostCommandResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << command_id->command)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set new notification command.
  delete [] cntct->host_notification_commands->cmd;
  cntct->host_notification_commands->cmd
    = my_strdup(command_id->command.c_str());

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the service notification command.
 *
 *  @param[in]  s          SOAP object.
 *  @param[in]  contact_id Target contact.
 *  @param[in]  command_id New service notification command.
 *  @param[out] res        Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__contactSetNotificationsOnServiceCommand(
      soap* s,
      ns1__contactIDType* contact_id,
      ns1__commandIDType* command_id,
      centreonengine__contactSetNotificationsOnServiceCommandResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(contact_id->contact << ", " << command_id->command)

  // Find target contact.
  contact* cntct(find_target_contact(contact_id->contact.c_str()));

  // Set new notification command.
  delete [] cntct->service_notification_commands->cmd;
  cntct->service_notification_commands->cmd
    = my_strdup(command_id->command.c_str());

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

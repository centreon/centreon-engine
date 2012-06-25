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

#include <QString>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/modules/webservice/commands.hh"
#include "com/centreon/engine/modules/webservice/create_object.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/objects/host.hh"
#include "soapH.h"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules;
using namespace com::centreon::engine::modules::webservice;

/**
 *  Create a new host into the engine.
 *
 *  @param[in] hst The struct with all information to create new host.
 */
void webservice::create_host(ns1__hostType const& hst) {
  // Check all arguments and set default option for optional options.
  if (hst.contacts.empty() && hst.contactGroups.empty())
    throw (engine_error() << "host '" << hst.id->name
           << "' no contact or no contact groups are defined");

  // Notification options.
  std::map<char, bool>
    notif_opt(get_options(hst.notificationOptions, "durfs", "n"));
  if (notif_opt.empty())
    throw (engine_error() << "host '" << hst.id->name
           << "' invalid notification options");

  // Flap detection options.
  std::map<char, bool> flap_detection_opt(get_options(
                                            hst.flapDetectionOptions,
                                            "odu",
                                            "n"));
  if (flap_detection_opt.empty())
    throw (engine_error() << "host '" << hst.id->name
           << "' invalid flap detection options");

  // Stalking options.
  std::map<char, bool>
    stalk_opt(get_options(hst.stalkingOptions, "odu", "n"));
  if (stalk_opt.empty())
    throw (engine_error() << "host '" << hst.id->name
           << "' invalid stalking options");

  // Initial state.
  int initial_state(HOST_UP);
  QString initial_state_options(hst.initialState
                                ? hst.initialState->c_str()
                                : "o");
  initial_state_options.toLower().trimmed();
  if ((initial_state_options == "o") || (initial_state_options == "up"))
    initial_state = HOST_UP;
  else if ((initial_state_options == "d")
           || (initial_state_options == "down"))
    initial_state = HOST_DOWN;
  else if ((initial_state_options == "u")
           || (initial_state_options == "unreachable"))
    initial_state = HOST_UNREACHABLE;
  else
    throw (engine_error() << "host '" << hst.id->name
           << "' invalid initial state");

  // Check period.
  timeperiod* check_period(find_timeperiod(hst.checkPeriod.c_str()));
  if (!check_period)
    throw (engine_error() << "host '" << hst.id->name
           << "' invalid check period");

  // Notification period.
  timeperiod* notification_period(find_timeperiod(
                                    hst.notificationPeriod.c_str()));
  if (!notification_period)
    throw (engine_error() << "host '" << hst.id->name
           << "' invalid notification period");

  // Event handler.
  command* cmd_event_handler(NULL);
  if (hst.eventHandler
      && !(cmd_event_handler = find_command(hst.eventHandler->c_str())))
    throw (engine_error() << "host '" << hst.id->name
           << "' invalid event handler");

  // Check command.
  command* cmd_check_command(NULL);
  if (hst.checkCommand != NULL
      && !(cmd_check_command
             = find_command(hst.checkCommand->c_str())))
    throw (engine_error() << "host '" << hst.id->name
           << "' invalid check command");

  // String properties.
  char const* display_name(hst.displayName
                           ? hst.displayName->c_str()
                           : NULL);
  char const* check_command(hst.checkCommand
                            ? hst.checkCommand->c_str()
                            : NULL);
  char const* event_handler(hst.eventHandler
                            ? hst.eventHandler->c_str()
                            : NULL);
  char const* notes(hst.notes
                    ? hst.notes->c_str()
                    : NULL);
  char const* notes_url(hst.notesUrl
                        ? hst.notesUrl->c_str()
                        : NULL);
  char const* action_url(hst.actionUrl
                         ? hst.actionUrl->c_str()
                         : NULL);
  char const* icon_image(hst.iconImage
                         ? hst.iconImage->c_str()
                         : NULL);
  char const* icon_image_alt(hst.iconImageAlt
                             ? hst.iconImageAlt->c_str()
                             : NULL);
  char const* vrml_image(hst.vrmlImage
                         ? hst.vrmlImage->c_str()
                         : NULL);
  char const* statusmap_image(hst.statusmapImage
                              ? hst.statusmapImage->c_str()
                              : NULL);

  // Remaining properties.
  unsigned int check_interval(hst.checkInterval
                              ? *hst.checkInterval
                              : 5.0);
  unsigned int retry_interval(hst.retryInterval
                              ? *hst.retryInterval
                              : 1.0);
  bool active_checks_enabled(hst.activeChecksEnabled
                             ? *hst.activeChecksEnabled
                             : true);
  bool passive_checks_enabled(hst.passiveChecksEnabled
                              ? *hst.passiveChecksEnabled
                              : true);
  bool obsess_over_host(hst.obsessOverHost
                        ? *hst.obsessOverHost
                        : true);
  bool event_handler_enabled(hst.eventHandlerEnabled
                             ? *hst.eventHandlerEnabled
                             : true);
  bool flap_detection_enabled(hst.flapDetectionEnabled
                              ? *hst.flapDetectionEnabled
                              : true);
  bool notifications_enabled(hst.notificationsEnabled
                             ? *hst.notificationsEnabled
                             : true);
  bool process_perfdata(hst.processPerfData
                        ? *hst.processPerfData
                        : true);
  bool retain_status_information(hst.retainStatusInformation
                                 ? *hst.retainStatusInformation
                                 : true);
  bool retain_nonstatus_information(hst.retainNonstatusInformation
                                    ? *hst.retainNonstatusInformation
                                    : true);
  int first_notification_delay(hst.firstNotificationDelay
                               ? * hst.firstNotificationDelay
                               : 60);
  double low_flap_threshold(hst.lowFlapThreshold
                            ? *hst.lowFlapThreshold / 100
                            : 0.0);
  double high_flap_threshold(hst.highFlapThreshold
                             ? *hst.highFlapThreshold / 100
                             : 0.0);
  bool check_freshness(hst.checkFreshness
                       ? *hst.checkFreshness
                       : false);
  int freshness_threshold(hst.freshnessThreshold
                          ? *hst.freshnessThreshold
                          : false);

  // XXX: add check host dependency for child

  // Create a new host.
  host* new_hst(add_host(
                  hst.id->name.c_str(),
                  display_name,
                  hst.alias.c_str(),
                  hst.address.c_str(),
                  hst.checkPeriod.c_str(),
                  initial_state,
                  check_interval,
                  retry_interval,
                  hst.maxCheckAttempts,
                  notif_opt['r'],
                  notif_opt['d'],
                  notif_opt['u'],
                  notif_opt['f'],
                  notif_opt['s'],
                  hst.notificationInterval,
                  first_notification_delay,
                  hst.notificationPeriod.c_str(),
                  notifications_enabled,
                  check_command,
                  active_checks_enabled,
                  passive_checks_enabled,
                  event_handler,
                  event_handler_enabled,
                  flap_detection_enabled,
                  low_flap_threshold,
                  high_flap_threshold,
                  flap_detection_opt['o'],
                  flap_detection_opt['d'],
                  flap_detection_opt['u'],
                  stalk_opt['o'],
                  stalk_opt['d'],
                  stalk_opt['u'],
                  process_perfdata,
                  true,   // no documentation for
                  NULL,   // failure_prediction_options in nagios 3.
                  check_freshness,
                  freshness_threshold,
                  notes,
                  notes_url,
                  action_url,
                  icon_image,
                  icon_image_alt,
                  vrml_image,
                  statusmap_image,
                  -1,     // 2d_coords not used by the Centreon suite
                  -1,
                  false,
                  0,      // 3d_coords not used by the Centreon suite
                  0,
                  0,
                  false,
                  true,
                  retain_status_information,
                  retain_nonstatus_information,
                  obsess_over_host));

  try {
    // Add parents.
    std::vector<host*> hst_parents(
                         _find<host>(
                           hst.parents,
                           (void* (*)(char const*))&find_host));
    if (hst.parents.size() != hst_parents.size())
      throw (engine_error() << "host '" << hst.id->name
             << "' invalid parent");

    // Link with contacts.
    std::vector<contact*> hst_contacts(
                            _find<contact>(
                              hst.contacts,
                              (void* (*)(char const*))&find_contact));
    if (hst.contacts.size() != hst_contacts.size())
      throw (engine_error() << "host '" << hst.id->name
             << "' invalid contact");

    // Link with contact groups.
    std::vector<contactgroup*>
      hst_contactgroups(_find<contactgroup>(
                          hst.contactGroups,
                          (void* (*)(char const*))&find_contactgroup));
    if (hst.contactGroups.size() != hst_contactgroups.size())
      throw (engine_error() << "host '" << hst.id->name
             << "' invalid contact group");

    // Link with host groups.
    std::vector<hostgroup*>
      hst_hostgroups(
        _find<hostgroup>(
          hst.hostgroups,
          (void* (*)(char const*))&find_hostgroup));
    if (hst.hostgroups.size() != hst_hostgroups.size())
      throw (engine_error() << "host '" << hst.id->name
             << "' invalid host group");

    // Link objects together.
    objects::link(
               new_hst,
               hst_parents,
               hst_contacts,
               hst_contactgroups,
               hst_hostgroups,
               hst.customVariables,
               initial_state,
               check_period,
               notification_period,
               cmd_event_handler,
               cmd_check_command);
  }
  catch (std::exception const& e) {
    (void)e;

    // Release host in case of error.
    objects::release(new_hst);

    // Rethrow exception.
    throw ;
  }

  return ;
}

/**
 *  Find target host.
 *
 *  @param[in] name Host name.
 *
 *  @return Host object.
 */
static host* find_target_host(char const* name) {
  host* hst(find_host(name));
  if (!hst)
    throw (engine_error() << "cannot find host '" << name << "'");
  return (hst);
}

/**
 *  Notify event broker of host update.
 *
 *  @param[in] hst Host pointer.
 */
static void notify_event_broker(host* hst) {
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_host_data(
    NEBTYPE_HOST_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    hst,
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);
  return ;
}

/**************************************
*                                     *
*               Global                *
*                                     *
**************************************/

/**
 *  Create a new host.
 *
 *  @param[in]  s   SOAP object.
 *  @param[in]  hst Host to add.
 *  @param[out] res Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostAdd(
      soap* s,
      ns1__hostType* hst,
      centreonengine__hostAddResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(hst->id->name)

  // Create host.
  create_host(*hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Modify an existing host.
 *
 *  @param[in]  s   SOAP object.
 *  @param[in]  hst Host properties.
 *  @param[out] res Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostModify(
      soap* s,
      ns1__hostType* hst,
      centreonengine__hostModifyResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(hst->id->name)

  // XXX

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Remove a host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostRemove(
      soap* s,
      ns1__hostIDType* host_id,
      centreonengine__hostRemoveResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find host.
  host* hst(find_host(host_id->name.c_str()));
  if (hst) {
    // Check services.
    if (hst->services)
      throw (engine_error() << "cannot remove host '" << host_id->name
             << "': has services");

    // Check parents.
    if (hst->parent_hosts)
      throw (engine_error() << "cannot remove host '" << host_id->name
             << "': host is child of other host(s)");

    // Check children.
    if (hst->child_hosts)
      throw (engine_error() << "cannot remove host '" << host_id->name
             << "': host is parent of other host(s)");

    // Check host groups.
    if (hst->hostgroups_ptr)
      throw (engine_error() << "cannot remove host '" << host_id->name
             << "': used by host group(s)");

    // Remove host.
    if (!remove_host_by_id(host_id->name.c_str()))
      throw (engine_error() << "error while removing host '"
             << host_id->name << "'");
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*          Acknowledgement.           *
*                                     *
**************************************/

/**
 *  Check whether or not the host is acknowledged.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true is host is acknowledged.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetAcknowledgementIsSet(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested property.
  res = (hst->acknowledgement_type != ACKNOWLEDGEMENT_NONE);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the current type of the acknowledgement on a host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Acknowledgement type.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetAcknowledgementType(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned short& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->acknowledgement_type;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*               Address               *
*                                     *
**************************************/

/**
 *  Get the address of the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Host address.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetAddress(
      soap* s,
      ns1__hostIDType* host_id,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set address.
  if (hst->address)
    res = hst->address;
  else
    res.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the address of the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  address New host address.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetAddress(
      soap* s,
      ns1__hostIDType* host_id,
      std::string address,
      centreonengine__hostSetAddressResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set new host address.
  delete [] hst->address;
  hst->address = my_strdup(address.c_str());

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*                Check                *
*                                     *
**************************************/

/**
 *  Check if active checks are enabled on the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if active checks are enabled on host.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckActiveEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  res = hst->checks_enabled;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the host check command.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Host check command.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckCommand(
      soap* s,
      ns1__hostIDType* host_id,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  if (hst->host_check_command)
    res = hst->host_check_command;
  else
    res.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the current check attempt of the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Current check attempt.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckCurrentAttempt(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  res = hst->current_attempt;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the normal check interval.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host
 *  @param[out] res     Normal check interval.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckIntervalNormal(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  res = static_cast<unsigned int>(hst->check_interval);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the retry check interval.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Retry check interval.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckIntervalRetry(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  res = static_cast<unsigned int>(hst->retry_interval);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the date when the last check was executed.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Last check execution time.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckLast(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  res = hst->last_check;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the max check attempts of the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Max check attempt.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckMaxAttempts(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  res = hst->max_attempts;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the time at which the next host check is scheduled to run.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Next check execution time.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckNext(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  res = hst->next_check;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the host check options.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Check options.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckOptions(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  res = hst->check_options;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if passive checks are enabled on this host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if passive checks are enabled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckPassiveEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  res = hst->accept_passive_host_checks;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the host check period.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Check period.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckPeriod(
      soap* s,
      ns1__hostIDType* host_id,
      centreonengine__hostGetCheckPeriodResponse& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set period.
  res.val = soap_new_ns1__timeperiodIDType(s, 1);
  if (hst->check_period)
    res.val->name = hst->check_period;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if the host should be scheduled.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if checks should be scheduled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckShouldBeScheduled(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  res = hst->should_be_scheduled;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the type of the host check.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Type of the host check.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCheckType(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned short& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  res = hst->check_type;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable active checks on the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Host to set data.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCheckActiveEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetCheckActiveEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set new value.
  hst->checks_enabled = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the host check command.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  cmd     New check command.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCheckCommand(
      soap* s,
      ns1__hostIDType* host_id,
      std::string cmd,
      centreonengine__hostSetCheckCommandResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << cmd)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Update check command.
  if (!cmd.empty()) {
    // Find target command.
    command* cmd_ptr(find_command(cmd.c_str()));
    if (!cmd_ptr)
      throw (engine_error() << "cannot update check command of host '"
             << host_id->name << "': command '" << cmd
             << "' does not exist");

    // Set new command.
    delete [] hst->host_check_command;
    hst->host_check_command = my_strdup(cmd.c_str());
    hst->check_command_ptr = cmd_ptr;
  }
  // Remove check command.
  else {
    delete [] hst->host_check_command;
    hst->host_check_command = NULL;
    hst->check_command_ptr = NULL;
  }

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the normal check interval of the host.
 *
 *  @param[in]  s        SOAP object.
 *  @param[in]  host_id  Target host.
 *  @param[in]  interval Check interval.
 *  @param[out] res      Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCheckIntervalNormal(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int interval,
      centreonengine__hostSetCheckIntervalNormalResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << interval)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set normal check interval.
  hst->check_interval = interval;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the retry check interval of the host.
 *
 *  @param[in]  s        SOAP object.
 *  @param[in]  host_id  Target host.
 *  @param[in]  interval Check interval.
 *  @param[out] res      Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCheckIntervalRetry(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int interval,
      centreonengine__hostSetCheckIntervalRetryResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << interval)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set retry check interval.
  hst->retry_interval = interval;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the max check attempts of the host.
 *
 *  @param[in]  s        SOAP object.
 *  @param[in]  host_id  Target host.
 *  @param[in]  attempts Max check attempts.
 *  @param[out] res      Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCheckMaxAttempts(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int attempts,
      centreonengine__hostSetCheckMaxAttemptsResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << attempts)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Check max attempts value.
  if (!attempts)
    throw (engine_error()
           << "attempt to set maximum to check attempts of host '"
           << host_id->name << "' to 0, which is forbidden");

  // Set max check attempts.
  hst->max_attempts = attempts;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable host_id passive checks.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCheckPassiveEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetCheckPassiveEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable passive checks.
  hst->accept_passive_host_checks = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Update or remove check timeperiod.
 *
 *  @param[in]  s             SOAP object.
 *  @param[in]  host_id       Target host.
 *  @param[in]  timeperiod_id Target timeperiod.
 *  @param[out] res           Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCheckPeriod(
      soap* s,
      ns1__hostIDType* host_id,
      ns1__timeperiodIDType* timeperiod_id,
      centreonengine__hostSetCheckPeriodResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << timeperiod_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Update timeperiod.
  if (!timeperiod_id->name.empty()) {
    // Find target timeperiod.
    timeperiod*
      tmprd(find_timeperiod(timeperiod_id->name.c_str()));
    if (!tmprd)
      throw (engine_error() << "cannot update check period of host '"
             << host_id->name << "': timeperiod '"
             << timeperiod_id->name << "' does not exist");

    // Set timeperiod.
    delete [] hst->check_period;
    hst->check_period = my_strdup(timeperiod_id->name.c_str());
    hst->check_period_ptr = tmprd;
  }
  // Remove timeperiod.
  else {
    delete [] hst->check_period;
    hst->check_period = NULL;
    hst->check_period_ptr = NULL;
  }

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*            Circular Path            *
*                                     *
**************************************/

/**
 *  Check if the host has been checked for circular path.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if host has been checked for circular path.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCircularPathChecked(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get value.
  res = hst->circular_path_checked;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if the host has circular path.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if host has circular path.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCircularPathHas(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->contains_circular_path;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*           Custom Variable           *
*                                     *
**************************************/

/**
 *  Get a custom variable.
 *
 *  @param[in]  s        SOAP object.
 *  @param[in]  host_id  Target host.
 *  @param[in]  varname  Custom variable name.
 *  @param[out] varvalue Custom variable value.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetCustomVariable(
      soap* s,
      ns1__hostIDType* host_id,
      std::string varname,
      std::string& varvalue) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << varname)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Browse variables.
  varvalue.clear();
  for (customvariablesmember* cvar(hst->custom_variables);
       cvar;
       cvar = cvar->next)
    if (cvar->variable_name
        && !strcmp(cvar->variable_name, varname.c_str())
        && cvar->variable_value)
      varvalue = cvar->variable_value;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set a host custom variable.
 *
 *  @param[in]  s        SOAP object.
 *  @param[in]  host_id  Target host.
 *  @param[in]  varname  Target variable.
 *  @param[in]  varvalue New variable value.
 *  @param[out] res      Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetCustomVariable(
      soap* s,
      ns1__hostIDType* host_id,
      std::string varname,
      std::string varvalue,
      centreonengine__hostSetCustomVariableResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << varname << ", " << varvalue)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Find existing custom variable.
  customvariablesmember** cvar;
  for (cvar = &hst->custom_variables; *cvar; cvar = &(*cvar)->next)
    if ((*cvar)->variable_name
        && !strcmp((*cvar)->variable_name, varname.c_str()))
      break ;

  // Update variable.
  if (!varvalue.empty()) {
    // Create new variable if not existing.
    if (!*cvar) {
      *cvar = new customvariablesmember;
      (*cvar)->next = NULL;
      (*cvar)->variable_name = my_strdup(varname.c_str());
    }
    else {
      delete [] (*cvar)->variable_value;
      (*cvar)->variable_value = NULL;
    }

    // Set new value.
    (*cvar)->variable_value = my_strdup(varvalue.c_str());
  }
  // Delete variable.
  if (*cvar) {
    customvariablesmember* to_delete(*cvar);
    *cvar = (*cvar)->next;
    delete [] to_delete->variable_name;
    delete [] to_delete->variable_value;
    delete to_delete;
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*              Downtime.              *
*                                     *
**************************************/

/**
 *  Get the host downtime depth.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Downtime depth.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetDowntimeDepth(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  res = hst->scheduled_downtime_depth;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if host has a pending flexible downtime.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if host has pending flexible downtime.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetDowntimeFlexPending(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->pending_flex_downtime;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*            Event Handler            *
*                                     *
**************************************/

/**
 *  Get the host event handler.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Event handler.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetEventHandler(
      soap* s,
      ns1__hostIDType* host_id,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set event handler.
  if (hst->event_handler)
    res = hst->event_handler;
  else
    res.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if the host event handler is enabled.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if event handler is enabled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetEventHandlerEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->event_handler_enabled;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the host event handler.
 *
 *  @param[in]  s             SOAP object.
 *  @param[in]  host_id       Target host.
 *  @param[in]  event_handler Event handler.
 *  @param[out] res           Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetEventHandler(
      soap* s,
      ns1__hostIDType* host_id,
      std::string event_handler,
      centreonengine__hostSetEventHandlerResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << event_handler)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Update event handler.
  if (!event_handler.empty()) {
    // Find target command.
    command* cmd(find_command(event_handler.c_str()));
    if (!cmd)
      throw (engine_error() << "cannot update event handler of host '"
             << host_id->name << "': command '" << event_handler
             << "' does not exist");

    // Set new event handler.
    delete [] hst->event_handler;
    hst->event_handler = my_strdup(event_handler.c_str());
    hst->event_handler_ptr = cmd;
  }
  // Remove event handler.
  else {
    delete [] hst->event_handler;
    hst->event_handler = NULL;
    hst->event_handler_ptr = NULL;
  }

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable host event handler.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetEventHandlerEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetEventHandlerEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set value.
  hst->event_handler_enabled = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*         Failure Prediction          *
*                                     *
**************************************/

/**
 *  Check if failure prediction is enabled on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if failure prediction is enabled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFailurePredictionEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  res = hst->failure_prediction_enabled;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get host failure prediction options.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Failure prediction options.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFailurePredictionOptions(
      soap* s,
      ns1__hostIDType* host_id,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set requested value.
  if (hst->failure_prediction_options)
    res = hst->failure_prediction_options;
  else
    res.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable failure prediction on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFailurePredictionEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetFailurePredictionEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set property.
  hst->failure_prediction_enabled = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*           Flap Detection            *
*                                     *
**************************************/

/**
 *  Get the flap detection comment ID of the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Flap detection comment ID.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionCommentID(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->flapping_comment_id;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check whether flap detection is enabled on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if flap detection is enabled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->flap_detection_enabled;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if host is flapping.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if host is flapping.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionIsFlapping(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->is_flapping;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on down state.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if flap detection is enabled on down state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionOnDown(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->flap_detection_on_down;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on unreachable state.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if flap detection is enabled on unreachable
 *                      state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionOnUnreachable(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->flap_detection_on_unreachable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if flap detection is enabled on up state.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if flap detection is enabled on up state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionOnUp(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->flap_detection_on_up;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last time the flap detection state history was updated.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Time at which the flap detection history was
 *                      last updated.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionStateHistoryLastUpdate(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->last_state_history_update;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the host high flap threshold.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Host high flap threshold.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionThresholdHigh(
      soap* s,
      ns1__hostIDType* host_id,
      double& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->high_flap_threshold;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the host low flap threshold.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Hight low flap threshold.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFlapDetectionThresholdLow(
      soap* s,
      ns1__hostIDType* host_id,
      double& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->low_flap_threshold;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFlapDetectionEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetFlapDetectionEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable flap detection on host.
  hst->flap_detection_enabled = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on down state.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFlapDetectionOnDown(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetFlapDetectionOnDownResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable flap detection on down state.
  hst->flap_detection_on_down = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on unreachable state.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFlapDetectionOnUnreachable(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetFlapDetectionOnUnreachableResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable flap detection on unreachable state.
  hst->flap_detection_on_unreachable = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable flap detection on up state.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFlapDetectionOnUp(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetFlapDetectionOnUpResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable flap detection on up state.
  hst->flap_detection_on_up = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the high flap threshold of the host.
 *
 *  @param[in]  s         SOAP object.
 *  @param[in]  host_id   Target host.
 *  @param[in]  threshold New threshold.
 *  @param[out] res       Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFlapDetectionThresholdHigh(
      soap* s,
      ns1__hostIDType* host_id,
      double threshold,
      centreonengine__hostSetFlapDetectionThresholdHighResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << threshold)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set new high flap threshold.
  hst->high_flap_threshold = threshold;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the low flap threshold of the host.
 *
 *  @param[in]  s         SOAP object.
 *  @param[in]  host_id   Target host.
 *  @param[in]  threshold New threshold.
 *  @param[out] res       Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFlapDetectionThresholdLow(
      soap* s,
      ns1__hostIDType* host_id,
      double threshold,
      centreonengine__hostSetFlapDetectionThresholdLowResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << threshold)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set new low flap threshold.
  hst->low_flap_threshold = threshold;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*              Freshness              *
*                                     *
**************************************/

/**
 *  Check if freshness checks are enabled on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if freshness checks are enabled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFreshnessCheckEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->check_freshness;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if freshness check is active on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Check if freshness is active on host.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFreshnessIsActive(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->is_being_freshened;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the freshness threshold of the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Freshness threshold.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetFreshnessThreshold(
      soap* s,
      ns1__hostIDType* host_id,
      int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->freshness_threshold;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable freshness checks on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFreshnessCheckEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetFreshnessCheckEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable freshness checks on host.
  hst->check_freshness = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the host freshness threshold.
 *
 *  @param[in]  s         SOAP object.
 *  @param[in]  host_id   Target host.
 *  @param[in]  threshold New threshold.
 *  @param[out] res       Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetFreshnessThreshold(
      soap* s,
      ns1__hostIDType* host_id,
      int threshold,
      centreonengine__hostSetFreshnessThresholdResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << threshold)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set new freshness threshold.
  hst->freshness_threshold = threshold;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*         Modified Attributes         *
*                                     *
**************************************/

/**
 *  Get the modified attributes on the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Modified attributes.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetModifiedAttributes(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->modified_attributes;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*                Name                 *
*                                     *
**************************************/

/**
 *  Get the host alias.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Host alias.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNameAlias(
      soap* s,
      ns1__hostIDType* host_id,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set new alias.
  if (hst->alias)
    res = hst->alias;
  else
    res.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the host display name.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Display name.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNameDisplay(
      soap* s,
      ns1__hostIDType* host_id,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get display name.
  if (hst->display_name)
    res = hst->display_name;
  else
    res.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the host alias.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  alias   New alias.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNameAlias(
      soap* s,
      ns1__hostIDType* host_id,
      std::string alias,
      centreonengine__hostSetNameAliasResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << alias)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set alias.
  delete [] hst->alias;
  hst->alias = my_strdup(alias.c_str());

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the display name of the host.
 *
 *  @param[in]  s           SOAP object.
 *  @param[in]  host_id     Target host.
 *  @param[in]  displayname New display name.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNameDisplay(
      soap* s,
      ns1__hostIDType* host_id,
      std::string displayname,
      centreonengine__hostSetNameDisplayResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << displayname)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set new display name.
  delete [] hst->display_name;
  hst->display_name = my_strdup(displayname.c_str());

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*            Notification             *
*                                     *
**************************************/

/**
 *  Get the ID of the current host_id notification.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Current notification ID.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsCurrentID(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->current_notification_id;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the current notification number of the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Current host notification number.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsCurrentNumber(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->current_notification_number;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if notifications are enabled on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if notifications are enabled on host.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->notifications_enabled;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the first notification delay of the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Get first notification delay.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsFirstDelay(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = static_cast<unsigned int>(hst->first_notification_delay);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the notification interval of the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Notification interval.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsInterval(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = static_cast<unsigned int>(hst->notification_interval);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the time at which the last notification was sent.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Time at which the last host notification was
 *                      sent.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsLast(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->last_host_notification;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the time at which the next notification will be sent.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] val     Time at which the next notification will be sent.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsNext(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->next_host_notification;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host is down.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if notifications are sent when host is
 *                      down.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsOnDown(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->notify_on_down;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host is on downtime.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if notifications are sent when host enters
 *                      or leave downtime.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsOnDowntime(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->notify_on_downtime;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host is flappy.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if notifications are sent when host flaps.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsOnFlapping(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->notify_on_flapping;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host recovers.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if notifications are sent when host
 *                      recovers.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsOnRecovery(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->notify_on_recovery;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if notifications are sent if host is unreachable.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if notifications are sent when host is
 *                      unreachable.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsOnUnreachable(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->notify_on_unreachable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the notification period of the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Notifications period.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetNotificationsPeriod(
      soap* s,
      ns1__hostIDType* host_id,
      centreonengine__hostGetNotificationsPeriodResponse& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  if (hst->notification_period) {
    res.val = soap_new_ns1__timeperiodIDType(s, 1);
    res.val->name = hst->notification_period;
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable all notifications beyond a host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsBeyondEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetNotificationsBeyondEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable.
  if (enable)
    enable_and_propagate_notifications(hst, 0, false, true, true);
  else
    disable_and_propagate_notifications(hst, 0, false, true, true);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetNotificationsEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable host notifications.
  hst->notifications_enabled = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the time after which the first host_id notification will be sent.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  delay   Delay of the first notification.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsFirstDelay(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int delay,
      centreonengine__hostSetNotificationsFirstDelayResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << delay)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set new first notification delay.
  hst->first_notification_delay = delay;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set the notification interval of the host.
 *
 *  @param[in]  s        SOAP object.
 *  @param[in]  host_id  Target host.
 *  @param[in]  interval Notification interval.
 *  @param[out] res      Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsInterval(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int interval,
      centreonengine__hostSetNotificationsIntervalResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << interval)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Set notification interval.
  hst->notification_interval = interval;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host is down.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsOnDown(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetNotificationsOnDownResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable down notifications.
  hst->notify_on_down = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host_id is in downtime.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsOnDowntime(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetNotificationsOnDowntimeResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable downtime notifications.
  hst->notify_on_downtime = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host is flappy.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsOnFlapping(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetNotificationsOnFlappingResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable flapping notifications.
  hst->notify_on_flapping = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host_id recovers.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsOnRecovery(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetNotificationsOnRecoveryResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable recovery notifications.
  hst->notify_on_recovery = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable notifications of a host and its children.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsOnSelfAndChildrenEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetNotificationsOnSelfAndChildrenEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable.
  if (enable)
    enable_and_propagate_notifications(hst, 0, true, true, false);
  else
    disable_and_propagate_notifications(hst, 0, true, true, false);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable notifications when host_id is unreachable.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsOnUnreachable(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetNotificationsOnUnreachableResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable unreachable notifications.
  hst->notify_on_unreachable = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Set notification period of host.
 *
 *  @param[in]  s             SOAP object.
 *  @param[in]  host_id       Target host.
 *  @param[in]  timeperiod_id Notification timeperiod.
 *  @param[out] res           Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetNotificationsPeriod(
      soap* s,
      ns1__hostIDType* host_id,
      ns1__timeperiodIDType* timeperiod_id,
      centreonengine__hostSetNotificationsPeriodResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << timeperiod_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Update timeperiod.
  if (!timeperiod_id->name.empty()) {
    // Find target timeperiod.
    timeperiod*
      tmprd(find_timeperiod(timeperiod_id->name.c_str()));
    if (!tmprd)
      throw (engine_error()
             << "cannot update notification period of host '"
             << host_id->name << "': timeperiod '"
             << timeperiod_id->name << "' does not exist");

    // Set timeperiod.
    delete [] hst->notification_period;
    hst->notification_period
      = my_strdup(timeperiod_id->name.c_str());
    hst->notification_period_ptr = tmprd;
  }
  // Remove timeperiod.
  else {
    delete [] hst->notification_period;
    hst->notification_period = NULL;
    hst->notification_period_ptr = NULL;
  }

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*              Obsession              *
*                                     *
**************************************/

/**
 *  Check whether or not host is being obsessed over.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if host is being obsessed over.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetObsessOver(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->obsess_over_host;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable host_id obsession.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetObsessOver(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetObsessOverResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable obsession.
  hst->obsess_over_host = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*              Perfdata               *
*                                     *
**************************************/

/**
 *  Check if perfdata processing is enabled on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if perfdata processing is enabled on host.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetPerfdataProcessingEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->process_performance_data;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable perfdata processing.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetPerfdataProcessingEnabled(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetPerfdataProcessingEnabledResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable performance data processing.
  hst->process_performance_data = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*               Plugin                *
*                                     *
**************************************/

/**
 *  Get the last execution time of the plugin.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Last plugin execution time.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetPluginExecutionTime(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = static_cast<unsigned int>(hst->execution_time);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if host check if currently executing.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if host plugin is being executed.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetPluginIsExecuting(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->is_executing;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the host latency.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Plugin latency.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetPluginLatency(
      soap* s,
      ns1__hostIDType* host_id,
      double& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->latency;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the plugin output.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Plugin output.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetPluginOutput(
      soap* s,
      ns1__hostIDType* host_id,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  if (hst->plugin_output)
    res = hst->plugin_output;
  else
    res.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the plugin perfdata.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Perfdata.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetPluginPerfdata(
      soap* s,
      ns1__hostIDType* host_id,
      std::string& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested data.
  if (hst->perf_data)
    res = hst->perf_data;
  else
    res.clear();

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*              Retention              *
*                                     *
**************************************/

/**
 *  Check if host status information are retained.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if status information are retained.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetRetainStatusInformation(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->retain_status_information;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if host non status information are retained.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if non-status information are retained.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetRetainNonStatusInformation(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->retain_nonstatus_information;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable status information retention on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetRetainStatusInformation(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetRetainStatusInformationResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable retention.
  hst->retain_status_information = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable non status information retention on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetRetainNonStatusInformation(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetRetainNonStatusInformationResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable retention.
  hst->retain_nonstatus_information = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*              Services               *
*                                     *
**************************************/

/**
 *  Get the number of services on this host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Service count.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetServicesCount(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->total_services;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the total check interval on this host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Services total check interval.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetServicesTotalCheckInterval(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned int& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->total_service_check_interval;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*              Stalking               *
*                                     *
**************************************/

/**
 *  Check if stalking on down is enabled on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if stalk on down is enabled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStalkOnDown(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->stalk_on_down;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if stalking on unreachable is enabled on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if stalk on unreachable is enabled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStalkOnUnreachable(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->stalk_on_unreachable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Check if stalking on up is enabled on host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     true if stalk on up is enabled.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStalkOnUp(
      soap* s,
      ns1__hostIDType* host_id,
      bool& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->stalk_on_up;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on down.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetStalkOnDown(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetStalkOnDownResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable stalking on down state.
  hst->stalk_on_down = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on unreachable.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetStalkOnUnreachable(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetStalkOnUnreachableResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable stalking on unreachable state.
  hst->stalk_on_unreachable = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Enable or disable stalking on up.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[in]  enable  true to enable, false to disable.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostSetStalkOnUp(
      soap* s,
      ns1__hostIDType* host_id,
      bool enable,
      centreonengine__hostSetStalkOnUpResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(host_id->name << ", " << enable)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Enable or disable stalking on up state.
  hst->stalk_on_up = enable;

  // Notify event broker.
  notify_event_broker(hst);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**************************************
*                                     *
*                State                *
*                                     *
**************************************/

/**
 *  Get the current state of the state.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Current host state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateCurrent(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned short& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->current_state;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the initial state of the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Initial host state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateInitial(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned short& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->initial_state;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last host_id state.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Get last host state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateLast(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned short& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->last_state;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last time the state changed.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Time at which the host last changed state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateLastChange(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->last_state_change;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last time the host was in a down state.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Last time the host was down.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateLastDown(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->last_time_down;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the host last hard state.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Last hard host state.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateLastHard(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->last_hard_state;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last time at which the hard state changed.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Last time at which the host hard state changed.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateLastHardChange(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->last_hard_state_change;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last time the host was in an unreachable state.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Last time the host was unreachable.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateLastUnreachable(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->last_time_unreachable;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the last time the host was in an up state.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Last time host was up.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateLastUp(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->last_time_up;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get the percent state change of the host.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     Percent state change.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStatePercentChange(
      soap* s,
      ns1__hostIDType* host_id,
      ULONG64& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = static_cast<time_t>(hst->percent_state_change);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Get state type (hard or soft).
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  host_id Target host.
 *  @param[out] res     State type.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__hostGetStateType(
      soap* s,
      ns1__hostIDType* host_id,
      unsigned long& res) {
  // Begin try block.
  COMMAND_BEGIN(host_id->name)

  // Find target host.
  host* hst(find_target_host(host_id->name.c_str()));

  // Get requested value.
  res = hst->state_type;

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

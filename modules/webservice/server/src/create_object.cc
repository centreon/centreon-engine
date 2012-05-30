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

#include <algorithm>
#include <QHash>
#include <QRegExp>
#include <QScopedArrayPointer>
#include <QVector>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/webservice/create_object.hh"
#include "com/centreon/engine/modules/webservice/schedule_object.hh"
#include "com/centreon/engine/objects/command.hh"
#include "com/centreon/engine/objects/contact.hh"
#include "com/centreon/engine/objects/contactgroup.hh"
#include "com/centreon/engine/objects/host.hh"
#include "com/centreon/engine/objects/hostdependency.hh"
#include "com/centreon/engine/objects/hostescalation.hh"
#include "com/centreon/engine/objects/hostgroup.hh"
#include "com/centreon/engine/objects/service.hh"
#include "com/centreon/engine/objects/servicedependency.hh"
#include "com/centreon/engine/objects/serviceescalation.hh"
#include "com/centreon/engine/objects/servicegroup.hh"
#include "com/centreon/engine/objects/timeperiod.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules;

/**
 *  Parse and return object options.
 *
 *  @param[in] opt         The option to parse.
 *  @param[in] pattern     The option list.
 *  @param[in] default_opt The default option.
 *
 *  @return The options list.
 */
std::map<char, bool> webservice::get_options(
                                   std::string const* opt,
                                   std::string const& pattern,
                                   char const* default_opt) {
  std::map<char, bool> res;
  QString _opt(opt ? opt->c_str() : default_opt);
  _opt.toLower().trimmed();
  if (_opt.contains(QRegExp(
                      QString("[^") + pattern.c_str() + "na, ]",
                      Qt::CaseInsensitive)))
    return (res);

  for (std::string::const_iterator
         it(pattern.begin()),
         end(pattern.end());
       it != end;
       ++it)
    if (_opt == "n")
      res[*it] = false;
    else if (_opt == "a")
      res[*it] = true;
    else
      res[*it] = (_opt.indexOf(*it) != -1);
  return (res);
}

/**
 *  Create a Qt stirng vector from a std string vector.
 *
 *  @param[in] vec The std vector.
 *
 *  @return The Qt vector.
 */
QVector<QString> webservice::std2qt(std::vector<std::string> const& vec) {
  QVector<QString> res;
  res.reserve(vec.size());
  for (std::vector<std::string>::const_iterator it = vec.begin(), end = vec.end();
       it != end;
       ++it)
    res.push_back(QString::fromAscii(it->data(), static_cast<int>(it->size())));
  return (res);
}

/**
 *  Find services by name and description to create a table of it.
 *
 *  @param[in]  services The object to find.
 *
 *  @return The service's table, stop when the first service are not found.
 */
static QVector<service*> _find(std::vector<std::string> const& objs) {
  QVector<service*> res;
  if (objs.size() % 2)
    return (res);

  res.reserve(objs.size() / 2);
  for (std::vector<std::string>::const_iterator it = objs.begin(),
	 end = objs.end();
       it != end;
       ++it) {
    // check if the object exist..
    char const* host_name = it->c_str();
    char const* service_description = (++it)->c_str();
    void* obj = find_service(host_name, service_description);
    if (obj == NULL)
      return (res);
    res.push_back(static_cast<service*>(obj));
  }
  return (res);
}

template<class T, class U>
static void _extract_object_from_objectgroup(QVector<T*> const& groups,
                                             QVector<U*>& objects) {
  for (typename QVector<T*>::const_iterator it = groups.begin(),
         end = groups.end();
       it != end;
       ++it) {
    for (hostsmember* member = (*it)->members;
         member != NULL;
         member = member->next) {
      objects.push_back(member->host_ptr);
    }
  }
  qSort(objects.begin(), objects.end());
  std::unique(objects.begin(), objects.end());
}

/**
 *  Add a new hostgroup into the engine.
 *
 *  @param[in] hstgrp The struct with all information to create new hostgroup.
 */
void webservice::create_host_group(ns1__hostGroupType const& hstgrp) {
  char const* notes = (hstgrp.notes ? hstgrp.notes->c_str() : NULL);
  char const* notes_url = (hstgrp.notesUrl ? hstgrp.notesUrl->c_str() : NULL);
  char const* action_url = (hstgrp.actionUrl ? hstgrp.actionUrl->c_str() : NULL);

  // create a new hostgroup.
  hostgroup* group = add_hostgroup(hstgrp.name.c_str(),
                                   hstgrp.alias.c_str(),
                                   notes,
                                   notes_url,
                                   action_url);

  // add all host into the hostgroup.
  QVector<host*> hst_members = _find<host>(hstgrp.members, (void* (*)(char const*))&find_host);
  if (hstgrp.members.empty() || static_cast<int>(hstgrp.members.size()) != hst_members.size()) {
    objects::release(group);
    throw (engine_error() << "hostgroup '" << hstgrp.name << "' invalid member.");
  }

  // add the content of other hostgroups into this hostgroup.
  QVector<hostgroup*> hst_groups =
    _find<hostgroup>(hstgrp.hostgroupMembers, (void* (*)(char const*))&find_hostgroup);
  if (static_cast<int>(hstgrp.hostgroupMembers.size()) != hst_groups.size()) {
    objects::release(group);
    throw (engine_error() << "hostgroup '" << hstgrp.name << "' invalid group member.");
  }

  try {
    objects::link(group, hst_members, hst_groups);
  }
  catch (std::exception const& e) {
    (void)e;
    objects::release(group);
    throw;
  }
}

/**
 *  Create a new servicegroup into the engine.
 *
 *  @param[in] svcgrp The struct with all information to create new servicegroup.
 */
void webservice::create_service_group(ns1__serviceGroupType const& svcgrp) {
  // check if service have host name and service description.
  if (svcgrp.members.size() % 2)
    throw (engine_error() << "servicegroup '" << svcgrp.name << "' invalid members.");

  char const* notes = (svcgrp.notes ? svcgrp.notes->c_str() : NULL);
  char const* notes_url = (svcgrp.notesUrl ? svcgrp.notesUrl->c_str() : NULL);
  char const* action_url = (svcgrp.actionUrl ? svcgrp.actionUrl->c_str() : NULL);

  // create a new service group.
  servicegroup* group = add_servicegroup(svcgrp.name.c_str(),
                                         svcgrp.alias.c_str(),
                                         notes,
                                         notes_url,
                                         action_url);

  // Add all services into the servicegroup.
  QVector<service*> svc_members(::_find(svcgrp.members));
  if (static_cast<int>(svcgrp.members.size() / 2) != svc_members.size()) {
    objects::release(group);
    throw (engine_error() << "servicegroup '" << svcgrp.name << "' invalid group member.");
  }

  QVector<servicegroup*> svc_groups =
    _find<servicegroup>(svcgrp.servicegroupMembers, (void* (*)(char const*))&find_servicegroup);
  if (static_cast<int>(svcgrp.servicegroupMembers.size()) != svc_groups.size()) {
    objects::release(group);
    throw (engine_error() << "servicegroup '" << svcgrp.name << "' invalid group member.");
  }

  try {
    objects::link(group, svc_members, svc_groups);
  }
  catch (std::exception const& e) {
    (void)e;
    objects::release(group);
    throw;
  }
}

/**
 *  Create a new host into the engine.
 *
 *  @param[in] hst The struct with all information to create new host.
 */
void webservice::create_host(ns1__hostType const& hst) {
  // check all arguments and set default option for optional options.
  if (hst.contacts.empty() == true && hst.contactGroups.empty() == true)
    throw (engine_error() << "host '" << hst.name
           << "' no contact or no contact groups are defined.");

  std::map<char, bool> notif_opt = get_options(hst.notificationOptions, "durfs", "n");
  if (notif_opt.empty())
    throw (engine_error() << "host '" << hst.name << "' invalid notification options.");

  std::map<char, bool> flap_detection_opt = get_options(hst.flapDetectionOptions, "odu", "n");
  if (flap_detection_opt.empty())
    throw (engine_error() << "host '" << hst.name << "' invalid flap detection options.");

  std::map<char, bool> stalk_opt = get_options(hst.stalkingOptions, "odu", "n");
  if (stalk_opt.empty())
    throw (engine_error() << "host '" << hst.name << "' invalid stalking options.");

  int initial_state = 0;
  QString initial_state_options(hst.initialState ? hst.initialState->c_str() : "o");
  initial_state_options.toLower().trimmed();
  if (initial_state_options == "o" || initial_state_options == "up")
    initial_state = 0;
  else if (initial_state_options == "d" || initial_state_options == "down")
    initial_state = 1;
  else if (initial_state_options == "u" || initial_state_options == "unreachable")
    initial_state = 2;
  else
    throw (engine_error() << "host '" << hst.name << "' invalid initial state.");

  timeperiod* check_period = find_timeperiod(hst.checkPeriod.c_str());
  if (check_period == NULL)
    throw (engine_error() << "host '" << hst.name << "' invalid check period.");

  timeperiod* notification_period = find_timeperiod(hst.notificationPeriod.c_str());
  if (notification_period == NULL)
    throw (engine_error() << "host '" << hst.name << "' invalid notification period.");

  command* cmd_event_handler = NULL;
  if (hst.eventHandler != NULL
      && (cmd_event_handler = find_command(hst.eventHandler->c_str())) == NULL)
    throw (engine_error() << "host '" << hst.name << "' invalid event handler.");

  command* cmd_check_command = NULL;
  if (hst.checkCommand != NULL
      && (cmd_check_command = find_command(hst.checkCommand->c_str())) == NULL)
    throw (engine_error() << "host '" << hst.name << "' invalid check command.");

  char const* display_name = (hst.displayName ? hst.displayName->c_str() : NULL);
  char const* check_command = (hst.checkCommand ? hst.checkCommand->c_str() : NULL);
  char const* event_handler = (hst.eventHandler ? hst.eventHandler->c_str() : NULL);
  char const* notes = (hst.notes ? hst.notes->c_str() : NULL);
  char const* notes_url = (hst.notesUrl ? hst.notesUrl->c_str() : NULL);
  char const* action_url = (hst.actionUrl ? hst.actionUrl->c_str() : NULL);
  char const* icon_image = (hst.iconImage ? hst.iconImage->c_str() : NULL);
  char const* icon_image_alt = (hst.iconImageAlt ? hst.iconImageAlt->c_str() : NULL);
  char const* vrml_image = (hst.vrmlImage ? hst.vrmlImage->c_str() : NULL);
  char const* statusmap_image = (hst.statusmapImage ? hst.statusmapImage->c_str() : NULL);

  unsigned int check_interval = (hst.checkInterval ? *hst.checkInterval : 5.0);
  unsigned int retry_interval = (hst.retryInterval ? *hst.retryInterval : 1.0);
  bool active_checks_enabled = (hst.activeChecksEnabled ? *hst.activeChecksEnabled : true);
  bool passive_checks_enabled = (hst.passiveChecksEnabled ? *hst.passiveChecksEnabled : true);
  bool obsess_over_host = (hst.obsessOverHost ? *hst.obsessOverHost : true);
  bool event_handler_enabled = (hst.eventHandlerEnabled ? *hst.eventHandlerEnabled : true);
  bool flap_detection_enabled = (hst.flapDetectionEnabled ? *hst.flapDetectionEnabled : true);
  bool notifications_enabled = (hst.notificationsEnabled ? *hst.notificationsEnabled : true);
  bool process_perfdata = (hst.processPerfData ? *hst.processPerfData : true);
  bool retain_status_information = (hst.retainStatusInformation
				    ? *hst.retainStatusInformation : true);
  bool retain_nonstatus_information = (hst.retainNonstatusInformation
				       ? *hst.retainNonstatusInformation : true);

  int first_notification_delay = (hst.firstNotificationDelay ? * hst.firstNotificationDelay : 60);

  double low_flap_threshold = (hst.lowFlapThreshold ? *hst.lowFlapThreshold / 100 : 0.0);
  double high_flap_threshold = (hst.highFlapThreshold ? *hst.highFlapThreshold / 100 : 0.0);

  bool check_freshness = (hst.checkFreshness ? *hst.checkFreshness : false);
  int freshness_threshold = (hst.freshnessThreshold ? *hst.freshnessThreshold : false);

  // XXX: add check host dependency for child.

  // create a new host.
  host* new_hst = add_host(hst.name.c_str(),
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
                           -1,     // 2d_coords not used in centreon.
                           -1,
                           false,
                           0,      // 3d_coords not used in centreon.
                           0,
                           0,
                           false,
                           true,
                           retain_status_information,
                           retain_nonstatus_information,
                           obsess_over_host);

  // add host parents.
  QVector<host*> hst_parents = _find<host>(hst.parents, (void* (*)(char const*))&find_host);
  if (static_cast<int>(hst.parents.size()) != hst_parents.size()) {
    objects::release(new_hst);
    throw (engine_error() << "host '" << hst.name << "' invalid parent.");
  }

  QVector<contact*> hst_contacts =
    _find<contact>(hst.contacts, (void* (*)(char const*))&find_contact);
  if (static_cast<int>(hst.contacts.size()) != hst_contacts.size()) {
    objects::release(new_hst);
    throw (engine_error() << "host '" << hst.name << "' invalid contact.");
  }

  QVector<contactgroup*> hst_contactgroups =
    _find<contactgroup>(hst.contactGroups, (void* (*)(char const*))&find_contactgroup);
  if (static_cast<int>(hst.contactGroups.size()) != hst_contactgroups.size()) {
    objects::release(new_hst);
    throw (engine_error() << "host '" << hst.name << "' invalid contact group.");
  }

  QVector<hostgroup*> hst_hostgroups =
    _find<hostgroup>(hst.hostgroups, (void* (*)(char const*))&find_hostgroup);
  if (static_cast<int>(hst.hostgroups.size()) != hst_hostgroups.size()) {
    objects::release(new_hst);
    throw (engine_error() << "host '" << hst.name << "' invalid host group.");
  }

  QVector<QString> hst_customvar = std2qt(hst.customVariables);

  try {
    objects::link(new_hst,
                  hst_parents,
                  hst_contacts,
                  hst_contactgroups,
                  hst_hostgroups,
                  hst_customvar,
                  initial_state,
                  check_period,
                  notification_period,
                  cmd_event_handler,
                  cmd_check_command);
  }
  catch (std::exception const& e) {
    (void)e;
    objects::release(new_hst);
    throw;
  }
}

/**
 *  Create a new service into the engine.
 *
 *  @param[in] svc The struct with all information to create new service.
 */
void webservice::create_service(ns1__serviceType const& svc) {
  // check all arguments and set default option for optional options.
  if (svc.contacts.empty() == true && svc.contactGroups.empty() == true)
    throw (engine_error() << "service '" << svc.hostName << "', "
           << svc.serviceDescription << "' no contact or no contact groups are defined.");

  std::map<char, bool> notif_opt = get_options(svc.notificationOptions, "wucrfs", "n");
  if (notif_opt.empty())
    throw (engine_error() << "service '" << svc.hostName << ", "
	   << svc.serviceDescription << "' invalid notification options.");

  std::map<char, bool> stalk_opt = get_options(svc.stalkingOptions, "owuc", "n");
  if (stalk_opt.empty())
    throw (engine_error() << "service '" << svc.hostName << ", "
	   << svc.serviceDescription << "' invalid stalking options.");

  std::map<char, bool> flap_detection_opt = get_options(svc.flapDetectionOptions, "owuc", "n");
  if (flap_detection_opt.empty())
    throw (engine_error() << "service '" << svc.hostName << ", "
	   << svc.serviceDescription << "' invalid flap detection options.");

  int initial_state = STATE_OK;
  QString initial_state_options(svc.initialState ? svc.initialState->c_str() : "o");
  initial_state_options.toLower().trimmed();
  if (initial_state_options == "o" || initial_state_options == "ok")
    initial_state = STATE_OK;
  else if (initial_state_options == "w" || initial_state_options == "warning")
    initial_state = STATE_WARNING;
  else if (initial_state_options == "u" || initial_state_options == "unknown")
    initial_state = STATE_UNKNOWN;
  else if (initial_state_options == "c" || initial_state_options == "critical")
    initial_state = STATE_CRITICAL;
  else
    throw (engine_error() << "service '" << svc.hostName << ", "
	   << svc.serviceDescription << "' invalid initial state.");

  timeperiod* check_period = find_timeperiod(svc.checkPeriod.c_str());
  if (check_period == NULL)
    throw (engine_error() << "service '" << svc.hostName << ", "
	   << svc.serviceDescription << "' invalid check period.");

  timeperiod* notification_period = find_timeperiod(svc.notificationPeriod.c_str());
  if (notification_period == NULL)
    throw (engine_error() << "service '" << svc.hostName << ", "
	   << svc.serviceDescription << "' invalid notification period.");

  command* cmd_event_handler = NULL;
  if (svc.eventHandler != NULL) {
    std::string cmd_name(*svc.eventHandler, 0, svc.eventHandler->find('!'));
    if ((cmd_event_handler = find_command(cmd_name.c_str())) == NULL)
      throw (engine_error() << "service '" << svc.hostName << ", "
             << svc.serviceDescription << "' invalid event handler.");
  }

  std::string cmd_name(svc.checkCommand, 0, svc.checkCommand.find('!'));
  command* cmd_check_command = find_command(cmd_name.c_str());
  if (cmd_check_command == NULL)
    throw (engine_error() << "service '" << svc.hostName << ", "
	   << svc.serviceDescription << "' invalid check command.");

  host* hst = find_host(svc.hostName.c_str());
  if (hst == NULL)
    throw (engine_error() << "service '" << svc.hostName << ", "
	   << svc.serviceDescription << "' invalid host name.");

  char const* display_name = (svc.displayName ? svc.displayName->c_str() : NULL);
  char const* event_handler = (svc.eventHandler ? svc.eventHandler->c_str() : NULL);
  char const* notes = (svc.notes ? svc.notes->c_str() : NULL);
  char const* notes_url = (svc.notesUrl ? svc.notesUrl->c_str() : NULL);
  char const* action_url = (svc.actionUrl ? svc.actionUrl->c_str() : NULL);
  char const* icon_image = (svc.iconImage ? svc.iconImage->c_str() : NULL);
  char const* icon_image_alt = (svc.iconImageAlt ? svc.iconImageAlt->c_str() : NULL);

  bool active_checks_enabled = (svc.activeChecksEnabled ? *svc.activeChecksEnabled : true);
  bool passive_checks_enabled = (svc.passiveChecksEnabled ? *svc.passiveChecksEnabled : true);
  bool obsess_over_service = (svc.obsessOverService ? *svc.obsessOverService : true);
  bool event_handler_enabled = (svc.eventHandlerEnabled ? *svc.eventHandlerEnabled : true);
  bool flap_detection_enabled = (svc.flapDetectionEnabled ? *svc.flapDetectionEnabled : true);
  bool notifications_enabled = (svc.notificationsEnabled ? *svc.notificationsEnabled : true);
  bool process_perfdata = (svc.processPerfData ? *svc.processPerfData : true);
  bool retain_status_information = (svc.retainStatusInformation
				    ? *svc.retainStatusInformation : true);
  bool retain_nonstatus_information = (svc.retainNonstatusInformation
				       ? *svc.retainNonstatusInformation : true);

  bool is_volatile = (svc.isVolatile ? *svc.isVolatile : true);

  int first_notification_delay = (svc.firstNotificationDelay ? * svc.firstNotificationDelay : 60);

  double low_flap_threshold = (svc.lowFlapThreshold ? *svc.lowFlapThreshold / 100 : 0.0);
  double high_flap_threshold = (svc.highFlapThreshold ? *svc.highFlapThreshold / 100 : 0.0);

  bool check_freshness = (svc.checkFreshness ? *svc.checkFreshness : false);
  int freshness_threshold = (svc.freshnessThreshold ? *svc.freshnessThreshold : false);

  // create a new service.
  service* new_svc = add_service(svc.hostName.c_str(),
                                 svc.serviceDescription.c_str(),
                                 display_name,
                                 svc.checkPeriod.c_str(),
                                 initial_state,
                                 svc.maxCheckAttempts,
                                 true, // no documentation for parallelize in nagios 3.
                                 passive_checks_enabled,
                                 svc.checkInterval,
                                 svc.retryInterval,
                                 svc.notificationInterval,
                                 first_notification_delay,
                                 svc.notificationPeriod.c_str(),
                                 notif_opt['r'],
                                 notif_opt['u'],
                                 notif_opt['w'],
                                 notif_opt['c'],
                                 notif_opt['f'],
                                 notif_opt['s'],
                                 notifications_enabled,
                                 is_volatile,
                                 event_handler,
                                 event_handler_enabled,
                                 svc.checkCommand.c_str(),
                                 active_checks_enabled,
                                 flap_detection_enabled,
                                 low_flap_threshold,
                                 high_flap_threshold,
                                 flap_detection_opt['o'],
                                 flap_detection_opt['u'],
                                 flap_detection_opt['w'],
                                 flap_detection_opt['c'],
                                 stalk_opt['o'],
                                 stalk_opt['u'],
                                 stalk_opt['w'],
                                 stalk_opt['c'],
                                 process_perfdata,
                                 true,
                                 NULL,
                                 check_freshness,
                                 freshness_threshold,
                                 notes,
                                 notes_url,
                                 action_url,
                                 icon_image,
                                 icon_image_alt,
                                 retain_status_information,
                                 retain_nonstatus_information,
                                 obsess_over_service);

  QVector<contact*> svc_contacts =
    _find<contact>(svc.contacts, (void* (*)(char const*))&find_contact);
  if (static_cast<int>(svc.contacts.size()) != svc_contacts.size()) {
    objects::release(new_svc);
    throw (engine_error() << "service '" << svc.hostName << ", "
           << svc.serviceDescription << "' invalid contact.");
  }

  QVector<contactgroup*> svc_contactgroups =
    _find<contactgroup>(svc.contactGroups, (void* (*)(char const*))&find_contactgroup);
  if (static_cast<int>(svc.contactGroups.size()) != svc_contactgroups.size()) {
    objects::release(new_svc);
    throw (engine_error() << "service '" << svc.hostName << ", "
           << svc.serviceDescription << "' invalid contact group.");
  }

  QVector<servicegroup*> svc_servicegroups =
    _find<servicegroup>(svc.servicegroups, (void* (*)(char const*))&find_servicegroup);
  if (static_cast<int>(svc.servicegroups.size()) != svc_servicegroups.size()) {
    objects::release(new_svc);
    throw (engine_error() << "service '" << svc.hostName << ", "
           << svc.serviceDescription << "' invalid service group.");
  }

  QVector<QString> svc_customvar = std2qt(svc.customVariables);

  try {
    objects::link(new_svc,
                  svc_contacts,
                  svc_contactgroups,
                  svc_servicegroups,
                  svc_customvar,
                  initial_state,
                  check_period,
                  notification_period,
                  cmd_event_handler,
                  cmd_check_command);
  }
  catch (std::exception const& e) {
    (void)e;
    objects::release(new_svc);
    throw;
  }
}

/**
 *  Create a new host dependency into the engine.
 *
 *  @param[in] hostdependency The struct with all information to create new host dependency.
 */
void webservice::create_host_dependency(ns1__hostDependencyType const& hstdependency) {
  // check all arguments and set default option for optional options.
  if (!hstdependency.executionFailureCriteria
      && !hstdependency.notificationFailureCriteria)
    throw (engine_error() << "hostdependency have no notification failure "
           << "criteria and no execution failure criteria define.");

  if (hstdependency.hostgroupsName.empty() == true
      && hstdependency.hostsName.empty() == true)
    throw (engine_error() << "hostdependency have no hosts and no host groups define.");
  if (hstdependency.dependentHostgroupsName.empty() == true
      && hstdependency.dependentHostsName.empty() == true)
    throw (engine_error() << "hostdependency have no dependency hosts "
           << "and no dependency host groups define.");

  std::map<char, bool> execution_opt = get_options(hstdependency.executionFailureCriteria, "odup", "n");
  if (execution_opt.empty())
    throw (engine_error() << "hostdependency invalid execution failure criteria.");

  std::map<char, bool> notif_opt = get_options(hstdependency.notificationFailureCriteria, "odup", "n");
  if (notif_opt.empty())
    throw (engine_error() << "hostdependency invalid notification failure criteria.");

  char const* dependency_period = NULL;
  timeperiod* dependency_period_ptr = NULL;
  if (hstdependency.dependencyPeriod != NULL) {
    dependency_period = hstdependency.dependencyPeriod->c_str();
    if ((dependency_period_ptr = find_timeperiod(dependency_period)) == NULL)
      throw (engine_error() << "hostdependency invalid dependency period.");
  }

  QVector<host*> hstdep_hosts =
    _find<host>(hstdependency.hostsName, (void* (*)(char const*))&find_host);
  if (static_cast<int>(hstdependency.hostsName.size()) != hstdep_hosts.size())
    throw (engine_error() << "hostdependency invalid hosts name.");

  QVector<host*> hstdep_dependent_hosts =
    _find<host>(hstdependency.dependentHostsName, (void* (*)(char const*))&find_host);
  if (static_cast<int>(hstdependency.dependentHostsName.size()) != hstdep_dependent_hosts.size())
    throw (engine_error() << "hostdependency invalid dependent hosts name.");

  QVector<hostgroup*> hstdep_hostgroups =
    _find<hostgroup>(hstdependency.hostgroupsName, (void* (*)(char const*))&find_hostgroup);
  if (static_cast<int>(hstdependency.hostgroupsName.size()) != hstdep_hostgroups.size())
    throw (engine_error() << "hostdependency invalid host groups name.");

  QVector<hostgroup*> hstdep_dependent_hostgroups =
    _find<hostgroup>(hstdependency.dependentHostgroupsName, (void* (*)(char const*))&find_hostgroup);
  if (static_cast<int>(hstdependency.dependentHostgroupsName.size()) != hstdep_dependent_hostgroups.size())
    throw (engine_error() << "hostdependency invalid dependent host groups name.");

  _extract_object_from_objectgroup(hstdep_dependent_hostgroups, hstdep_dependent_hosts);
  _extract_object_from_objectgroup(hstdep_hostgroups, hstdep_hosts);

  bool inherits_parent = (hstdependency.inheritsParent
                          ? *hstdependency.inheritsParent : true);

  for (QVector<host*>::const_iterator it_dep = hstdep_dependent_hosts.begin(),
         end = hstdep_dependent_hosts.end();
       it_dep != end;
       ++it_dep) {
    for (QVector<host*>::const_iterator it = hstdep_hosts.begin(),
           end = hstdep_hosts.end();
         it != end;
         ++it) {
      if (hstdependency.executionFailureCriteria != NULL) {
        hostdependency* new_hstdependency =
          add_host_dependency((*it_dep)->name,
                              (*it)->name,
                              EXECUTION_DEPENDENCY,
                              inherits_parent,
                              execution_opt['o'],
                              execution_opt['d'],
                              execution_opt['u'],
                              execution_opt['p'],
                              dependency_period);

        try {
          objects::link(new_hstdependency, dependency_period_ptr);
        }
        catch (std::exception const& e) {
          (void)e;
          objects::release(new_hstdependency);
          throw;
        }
      }

      if (hstdependency.notificationFailureCriteria != NULL) {
        hostdependency* new_hstdependency =
          add_host_dependency((*it_dep)->name,
                              (*it)->name,
                              NOTIFICATION_DEPENDENCY,
                              inherits_parent,
                              notif_opt['o'],
                              notif_opt['d'],
                              notif_opt['u'],
                              notif_opt['p'],
                              dependency_period);

        try {
          objects::link(new_hstdependency, dependency_period_ptr);
        }
        catch (std::exception const& e) {
          (void)e;
          objects::release(new_hstdependency);
          throw;
        }
      }
    }
  }
}

/**
 *  Create a new host escalation into the engine.
 *
 *  @param[in] hostescalation The struct with all information to create new host escalation.
 */
void webservice::create_host_escalation(ns1__hostEscalationType const& hstescalation) {
  // check all arguments and set default option for optional options.
  if (hstescalation.contacts.empty() == true && hstescalation.contactGroups.empty() == true)
    throw (engine_error() << "hostescalation no contact and no contact groups are defined.");

  if (hstescalation.hostsName.empty() == true && hstescalation.hostgroupsName.empty() == true)
    throw (engine_error() << "hostescalation no host and no host groups are defined.");

  std::map<char, bool> escalation_opt = get_options(hstescalation.escalationOptions, "dur" , "n");
  if (escalation_opt.empty())
    throw (engine_error() << "hostescalation invalid escalation options.");

  char const* escalation_period = NULL;
  timeperiod* escalation_period_ptr = NULL;
  if (hstescalation.escalationPeriod != NULL) {
    escalation_period = hstescalation.escalationPeriod->c_str();
    if ((escalation_period_ptr = find_timeperiod(escalation_period)) == NULL)
      throw (engine_error() << "hostescalation invalid check period.");
  }

  QVector<contact*> hstesc_contacts =
    _find<contact>(hstescalation.contacts, (void* (*)(char const*))&find_contact);
  if (static_cast<int>(hstescalation.contacts.size()) != hstesc_contacts.size())
    throw (engine_error() << "hostescalation invalid contacts.");

  QVector<contactgroup*> hstesc_contactgroups =
    _find<contactgroup>(hstescalation.contactGroups, (void* (*)(char const*))&find_contactgroup);
  if (static_cast<int>(hstescalation.contactGroups.size()) != hstesc_contactgroups.size())
    throw (engine_error() << "hostescalation invalid contact groups.");

  QVector<hostgroup*> hstesc_hostgroups =
    _find<hostgroup>(hstescalation.hostgroupsName, (void* (*)(char const*))&find_hostgroup);
  if (static_cast<int>(hstescalation.hostgroupsName.size()) != hstesc_hostgroups.size())
    throw (engine_error() << "hostescalation invalid host groups.");

  QVector<host*> hstesc_hosts =
    _find<host>(hstescalation.hostsName, (void* (*)(char const*))&find_host);
  if (static_cast<int>(hstescalation.hostsName.size()) != hstesc_hosts.size())
    throw (engine_error() << "hostescalation invalid hosts.");

  _extract_object_from_objectgroup(hstesc_hostgroups, hstesc_hosts);

  for (QVector<host*>::const_iterator it = hstesc_hosts.begin(),
         end = hstesc_hosts.end();
       it != end;
       ++it) {
    hostescalation* new_hstescalation =
      add_host_escalation((*it)->name,
                         hstescalation.firstNotification,
                         hstescalation.lastNotification,
                         hstescalation.notificationInterval,
                         escalation_period,
                         escalation_opt['d'],
                         escalation_opt['u'],
                         escalation_opt['r']);

    try {
      objects::link(new_hstescalation,
                    hstesc_contacts,
                    hstesc_contactgroups,
                    escalation_period_ptr);
    }
    catch (std::exception const& e) {
      (void)e;
      objects::release(new_hstescalation);
      throw;
    }
  }
}

/**
 *  Create a new service dependency into the engine.
 *
 *  @param[in] servicedependency The struct with all
 *  information to create new service dependency.
 */
void webservice::create_service_dependency(ns1__serviceDependencyType const& svcdependency) {
  // check all arguments and set default option for optional options.
  if (!svcdependency.executionFailureCriteria
      && !svcdependency.notificationFailureCriteria)
    throw (engine_error() << "servicedependency have no notification failure "
           << "criteria and no execution failure criteria define.");

  if (svcdependency.hostgroupsName.empty() == true
      && svcdependency.hostsName.empty() == true)
    throw (engine_error() << "serviceependency have no hosts and no host groups define.");

  if (svcdependency.dependentHostsName.empty() == true
      && svcdependency.dependentHostgroupsName.empty() == true)
    throw (engine_error() << "serviceependency have no dependency hosts "
           << "and no dependency host groups define.");

  std::map<char, bool> execution_opt = get_options(svcdependency.executionFailureCriteria, "owucp", "n");
  if (execution_opt.empty())
    throw (engine_error() << "servicedependency invalid execution failure criteria.");

  std::map<char, bool> notif_opt = get_options(svcdependency.notificationFailureCriteria, "owucp", "n");
  if (notif_opt.empty())
    throw (engine_error() << "servicedependency invalid notification failure criteria.");

  char const* dependency_period = NULL;
  timeperiod* dependency_period_ptr = NULL;
  if (svcdependency.dependencyPeriod != NULL) {
    dependency_period = svcdependency.dependencyPeriod->c_str();
    if ((dependency_period_ptr = find_timeperiod(dependency_period)) == NULL)
      throw (engine_error() << "servicedependency invalid dependency period.");
  }

  QVector<host*> svcdep_hosts =
    _find<host>(svcdependency.hostsName, (void* (*)(char const*))&find_host);
  if (static_cast<int>(svcdependency.hostsName.size()) != svcdep_hosts.size())
    throw (engine_error() << "servicedependency invalid hosts name.");

  QVector<host*> svcdep_dependent_hosts =
    _find<host>(svcdependency.dependentHostsName, (void* (*)(char const*))&find_host);
  if (static_cast<int>(svcdependency.dependentHostsName.size()) != svcdep_dependent_hosts.size())
    throw (engine_error() << "servicedependency invalid dependent hosts name.");

  QVector<hostgroup*> svcdep_hostgroups =
    _find<hostgroup>(svcdependency.hostgroupsName, (void* (*)(char const*))&find_hostgroup);
  if (static_cast<int>(svcdependency.hostgroupsName.size()) != svcdep_hostgroups.size())
    throw (engine_error() << "servicedependency invalid host groups name.");

  QVector<hostgroup*> svcdep_dependent_hostgroups =
    _find<hostgroup>(svcdependency.dependentHostgroupsName, (void* (*)(char const*))&find_hostgroup);
  if (static_cast<int>(svcdependency.dependentHostgroupsName.size()) != svcdep_dependent_hostgroups.size())
    throw (engine_error() << "servicedependency invalid dependent host groups name.");

  _extract_object_from_objectgroup(svcdep_dependent_hostgroups, svcdep_dependent_hosts);
  _extract_object_from_objectgroup(svcdep_hostgroups, svcdep_hosts);

  bool inherits_parent = (svcdependency.inheritsParent
                          ? *svcdependency.inheritsParent : true);

  char const* service_description = svcdependency.serviceDescription.c_str();
  char const* dependent_service_description = svcdependency.dependentServiceDescription.c_str();

  for (QVector<host*>::const_iterator it_dep = svcdep_dependent_hosts.begin(),
         end = svcdep_dependent_hosts.end();
       it_dep != end;
       ++it_dep) {
    for (QVector<host*>::const_iterator it = svcdep_hosts.begin(),
           end = svcdep_hosts.end();
         it != end;
         ++it) {
      if (svcdependency.executionFailureCriteria != NULL) {
        servicedependency* new_svcdependency =
          add_service_dependency((*it_dep)->name,
                                 dependent_service_description,
                                 (*it)->name,
                                 service_description,
                                 EXECUTION_DEPENDENCY,
                                 inherits_parent,
                                 execution_opt['o'],
                                 execution_opt['w'],
                                 execution_opt['u'],
                                 execution_opt['c'],
                                 execution_opt['p'],
                                 dependency_period);

        try {
          objects::link(new_svcdependency, dependency_period_ptr);
        }
        catch (std::exception const& e) {
          (void)e;
          objects::release(new_svcdependency);
          throw;
        }
      }

      if (svcdependency.notificationFailureCriteria != NULL) {
        servicedependency* new_svcdependency =
          add_service_dependency((*it_dep)->name,
                                 dependent_service_description,
                                 (*it)->name,
                                 service_description,
                                 NOTIFICATION_DEPENDENCY,
                                 inherits_parent,
                                 notif_opt['o'],
                                 notif_opt['w'],
                                 notif_opt['u'],
                                 notif_opt['c'],
                                 notif_opt['p'],
                                 dependency_period);

        try {
          objects::link(new_svcdependency, dependency_period_ptr);
        }
        catch (std::exception const& e) {
          (void)e;
          objects::release(new_svcdependency);
          throw;
        }
      }
    }
  }
}

/**
 *  Create a new service escalation into the engine.
 *
 *  @param[in] serviceescalation The struct with all
 *  information to create new service escalation.
 */
void webservice::create_service_escalation(ns1__serviceEscalationType const& svcescalation) {
  // check all arguments and set default option for optional options.
  if (svcescalation.contacts.empty() == true && svcescalation.contactGroups.empty() == true)
    throw (engine_error() << "serviceescalation no contact and no contact groups are defined.");

  if (svcescalation.hostsName.empty() == true && svcescalation.hostgroupsName.empty() == true)
    throw (engine_error() << "serviceescalation no host and no host groups are defined.");

  std::map<char, bool> escalation_opt = get_options(svcescalation.escalationOptions, "wucr" , "n");
  if (escalation_opt.empty())
    throw (engine_error() << "serviceescalation invalid escalation options.");

  char const* escalation_period = NULL;
  timeperiod* escalation_period_ptr = NULL;
  if (svcescalation.escalationPeriod != NULL) {
    escalation_period = svcescalation.escalationPeriod->c_str();
    if ((escalation_period_ptr = find_timeperiod(escalation_period)) == NULL)
      throw (engine_error() << "serviceescalation invalid check period.");
  }

  QVector<contact*> svcesc_contacts =
    _find<contact>(svcescalation.contacts, (void* (*)(char const*))&find_contact);
  if (static_cast<int>(svcescalation.contacts.size()) != svcesc_contacts.size())
    throw (engine_error() << "serviceescalation invalid contacts.");

  QVector<contactgroup*> svcesc_contactgroups =
    _find<contactgroup>(svcescalation.contactGroups, (void* (*)(char const*))&find_contactgroup);
  if (static_cast<int>(svcescalation.contactGroups.size()) != svcesc_contactgroups.size())
    throw (engine_error() << "serviceescalation invalid contact groups.");

  QVector<hostgroup*> svcesc_hostgroups =
    _find<hostgroup>(svcescalation.hostgroupsName, (void* (*)(char const*))&find_hostgroup);
  if (static_cast<int>(svcescalation.hostgroupsName.size()) != svcesc_hostgroups.size())
    throw (engine_error() << "serviceescalation invalid host groups.");

  QVector<host*> svcesc_hosts =
    _find<host>(svcescalation.hostsName, (void* (*)(char const*))&find_host);
  if (static_cast<int>(svcescalation.hostsName.size()) != svcesc_hosts.size())
    throw (engine_error() << "serviceescalation invalid hosts.");

  _extract_object_from_objectgroup(svcesc_hostgroups, svcesc_hosts);

  char const* service_description = svcescalation.serviceDescription.c_str();
  for (QVector<host*>::const_iterator it = svcesc_hosts.begin(),
         end = svcesc_hosts.end();
       it != end;
       ++it) {
    serviceescalation* new_svcescalation =
      add_service_escalation((*it)->name,
                            service_description,
                            svcescalation.firstNotification,
                            svcescalation.lastNotification,
                            svcescalation.notificationInterval,
                            escalation_period,
                            escalation_opt['w'],
                            escalation_opt['u'],
                            escalation_opt['c'],
                            escalation_opt['r']);

    try {
      objects::link(new_svcescalation,
                    svcesc_contacts,
                    svcesc_contactgroups,
                    escalation_period_ptr);
    }
    catch (std::exception const& e) {
      (void)e;
      objects::release(new_svcescalation);
      throw;
    }
  }
}

/**
 *  Create a new timeperiod into the engine.
 *
 *  @param[in] tperiod The struct with all
 *  information to create new timeperiod.
 */
void webservice::create_timeperiod(ns1__timeperiodType const& tperiod) {
  objects::add_timeperiod(tperiod.name.c_str(),
                          tperiod.alias.c_str(),
                          std2qt(tperiod.range),
                          std2qt(tperiod.exclude));
}

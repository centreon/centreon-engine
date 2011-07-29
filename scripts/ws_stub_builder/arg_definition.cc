/*
** Copyright 2011 Merethis
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

#include "error.hh"
#include "arg_definition.hh"

using namespace com::centreon::engine::script;

/**
 *  Get instance of arg_definition.
 *
 *  @return Return the uniq instance of arg_definition.
 */
arg_definition& arg_definition::instance() {
  static arg_definition instance;
  return (instance);
}

/**
 *  Check if an argument type exist.
 *
 *  @param[in] type The type name of argument.
 *
 *  @return Return true if type exist into argument list, false otherwise.
 */
bool arg_definition::exist_argument(QString const& type) const {
  for (QList<argument>::const_iterator it = _list.begin(), end = _list.end();
       it != end;
       ++it) {
    if (it->get_type() == type) {
      return (true);
    }
  }
  return (false);
}

/**
 *  Find a specific argument.
 *
 *  @param[in] type The type name to find.
 *
 *  @return Return the argument with the good type name.
 */
argument const& arg_definition::find_argument(QString const& type) const {
  for (QList<argument>::const_iterator it = _list.begin(), end = _list.end();
       it != end;
       ++it) {
    if (it->get_type() == type) {
      return (*it);
    }
  }
  throw (error(qPrintable(type)));
}

/**
 *  Get the list of arguments who define by this object.
 *
 *  @return Return the list of all arguments.
 */
QList<argument> const& arg_definition::get_arguments() const throw() {
  return (_list);
}

/**
 *  Default constructor.
 */
arg_definition::arg_definition() {
  argument arg_bool("bool", "value");
  argument arg_double("double", "value");
  argument arg_int("int", "value");
  argument arg_string("std::string", "value");
  argument arg_time("time_t", "value");
  argument arg_uint("unsigned int", "value");
  argument arg_ulong64("ULONG64", "value");
  argument arg_vectorstr("std::vector<std::string>", "value");

  argument acknowledgement("ns1__acknowledgementType", "acknowledgement");
  acknowledgement.add(arg_string).set_name("author");
  acknowledgement.add(arg_string).set_name("comment");
  acknowledgement.add(arg_bool).set_name("notify");
  acknowledgement.add(arg_bool).set_name("persistent");
  acknowledgement.add(arg_bool).set_name("sticky");

  argument check_result("ns1__checkResultType", "result");
  check_result.add(arg_int).set_name("retval");
  check_result.add(arg_string).set_name("output");

  argument command_id("ns1__commandIDType", "command");
  command_id.add(arg_string).set_name("command")
    .set_help("command_id");

  argument command_type("ns1__commandType", "command");
  command_type.add(arg_string).set_name("name");
  command_type.add(arg_string).set_name("commandLine");

  argument contact_id("ns1__contactIDType", "contact");
  contact_id.add(arg_string).set_name("contact")
    .set_help("contact_id");

  argument contact_type("ns1__contactType", "contact");
  contact_type.add(arg_string).set_name("name");
  contact_type.add(arg_string).set_name("alias")
    .set_is_optional(true);
  contact_type.add(arg_vectorstr).set_name("contactgroups");
  contact_type.add(arg_bool).set_name("hostNotificationsEnabled")
    .set_help("host_notifications_enabled");
  contact_type.add(arg_bool).set_name("serviceNotificationsEnabled")
    .set_help("service_notifications_enabled");
  contact_type.add(arg_string).set_name("hostNotificationPeriod")
    .set_help("host_notification_period");
  contact_type.add(arg_string).set_name("serviceNotificationPeriod")
    .set_help("service_notification_period");
  contact_type.add(arg_string).set_name("hostNotificationOptions")
    .set_help("host_notification_options");
  contact_type.add(arg_string).set_name("serviceNotificationOptions")
    .set_help("service_notification_options");
  contact_type.add(arg_vectorstr).set_name("hostNotificationCommands")
    .set_help("host_notification_commands");
  contact_type.add(arg_vectorstr).set_name("serviceNotificationCommands")
    .set_help("service_notification_commands");
  contact_type.add(arg_string).set_name("email")
    .set_is_optional(true);
  contact_type.add(arg_string).set_name("pager")
    .set_is_optional(true);
  contact_type.add(arg_vectorstr).set_name("address");
  contact_type.add(arg_bool).set_name("canSubmitCommands")
    .set_help("can_submit_commands")
    .set_is_optional(true);
  contact_type.add(arg_bool).set_name("retainStatusInformation")
    .set_help("retain_status_information")
    .set_is_optional(true);
  contact_type.add(arg_bool).set_name("retainNonstatusInformation")
    .set_help("retain_nonstatus_information")
    .set_is_optional(true);
  contact_type.add(arg_vectorstr).set_name("customVariables")
    .set_help("custom_variables");

  argument contactgroup_id("ns1__contactGroupIDType", "contactGroup");
  contactgroup_id.add(arg_string).set_name("name")
    .set_help("contactgroup_id");

  argument contactgroup_type("ns1__contactGroupType", "contactGroup");
  contactgroup_type.add(arg_string).set_name("name");
  contactgroup_type.add(arg_string).set_name("alias");
  contactgroup_type.add(arg_vectorstr).set_name("contacts");
  contactgroup_type.add(arg_vectorstr).set_name("contactgroups");

  argument downtime_id("ns1__downtimeIDType", "downtime");
  downtime_id.add(arg_ulong64).set_name("downtime");

  argument downtime("ns1__downtimeType", "downtime");
  downtime.add(arg_time).set_name("starttime");
  downtime.add(arg_time).set_name("endtime");
  downtime.add(arg_bool).set_name("fixed");
  downtime.add(downtime_id).set_name("triggerid");
  downtime.add(arg_double).set_name("duration");
  downtime.add(arg_string).set_name("author");
  downtime.add(arg_string).set_name("comment");

  argument host_id("ns1__hostIDType", "host");
  host_id.add(arg_string).set_name("name")
    .set_help("host_id");

  argument host_type("ns1__hostType", "host");
  host_type.add(arg_string).set_name("name");
  host_type.add(arg_string).set_name("alias");
  host_type.add(arg_string).set_name("displayName")
    .set_help("display_name")
    .set_is_optional(true);
  host_type.add(arg_string).set_name("address");
  host_type.add(arg_vectorstr).set_name("parents");
  host_type.add(arg_vectorstr).set_name("hostgroups");
  host_type.add(arg_string).set_name("checkCommand")
    .set_help("check_command")
    .set_is_optional(true);
  host_type.add(arg_string).set_name("initialState")
    .set_help("initial_state")
    .set_is_optional(true);
  host_type.add(arg_uint).set_name("maxCheckAttempts")
    .set_help("max_check_attempts");
  host_type.add(arg_uint).set_name("checkInterval")
    .set_help("check_interval")
    .set_is_optional(true);
  host_type.add(arg_uint).set_name("retryInterval")
    .set_help("retry_interval")
    .set_is_optional(true);
  host_type.add(arg_bool).set_name("activeChecksEnabled")
    .set_help("active_checks_enabled")
    .set_is_optional(true);
  host_type.add(arg_bool).set_name("passiveChecksEnabled")
    .set_help("passive_checks_enabled")
    .set_is_optional(true);
  host_type.add(arg_string).set_name("checkPeriod")
    .set_help("check_period");
  host_type.add(arg_bool).set_name("obsessOverHost")
    .set_help("obsess_over_host")
    .set_is_optional(true);
  host_type.add(arg_bool).set_name("checkFreshness")
    .set_help("check_freshness")
    .set_is_optional(true);
  host_type.add(arg_uint).set_name("freshnessThreshold")
    .set_help("freshness_threshold")
    .set_is_optional(true);
  host_type.add(arg_string).set_name("eventHandler")
    .set_help("event_handler")
    .set_is_optional(true);
  host_type.add(arg_bool).set_name("eventHandlerEnabled")
    .set_help("event_handler_enabled")
    .set_is_optional(true);
  host_type.add(arg_uint).set_name("lowFlapThreshold")
    .set_help("low_flap_threshold")
    .set_is_optional(true);
  host_type.add(arg_uint).set_name("highFlapThreshold")
    .set_help("high_flap_threshold")
    .set_is_optional(true);
  host_type.add(arg_bool).set_name("flapDetectionEnabled")
    .set_help("flap_detection_enabled")
    .set_is_optional(true);
  host_type.add(arg_string).set_name("flapDetectionOptions")
    .set_help("flap_detection_options")
    .set_is_optional(true);
  host_type.add(arg_bool).set_name("processPerfData")
    .set_help("process_perfdata")
    .set_is_optional(true);
  host_type.add(arg_bool).set_name("retainStatusInformation")
    .set_help("retain_status_information")
    .set_is_optional(true);
  host_type.add(arg_bool).set_name("retainNonstatusInformation")
    .set_help("retain_nonstatus_information")
    .set_is_optional(true);
  host_type.add(arg_vectorstr).set_name("contacts");
  host_type.add(arg_vectorstr).set_name("contactGroups")
    .set_help("contact_groups");
  host_type.add(arg_uint).set_name("notificationInterval")
    .set_help("notification_interval");
  host_type.add(arg_uint).set_name("firstNotificationDelay")
    .set_help("first_notification_delay")
    .set_is_optional(true);
  host_type.add(arg_string).set_name("notificationPeriod")
    .set_help("notification_period");
  host_type.add(arg_string).set_name("notificationOptions")
    .set_help("notification_options")
    .set_is_optional(true);
  host_type.add(arg_bool).set_name("notificationsEnabled")
    .set_help("notifications_enabled")
    .set_is_optional(true);
  host_type.add(arg_string).set_name("stalkingOptions")
    .set_help("stalking_options")
    .set_is_optional(true);
  host_type.add(arg_string).set_name("notes")
    .set_is_optional(true);
  host_type.add(arg_string).set_name("notesUrl")
    .set_help("notes_url")
    .set_is_optional(true);
  host_type.add(arg_string).set_name("actionUrl")
    .set_help("action_url")
    .set_is_optional(true);
  host_type.add(arg_string).set_name("iconImage")
    .set_help("icon_image")
    .set_is_optional(true);
  host_type.add(arg_string).set_name("iconImageAlt")
    .set_help("icon_image_alt")
    .set_is_optional(true);
  host_type.add(arg_string).set_name("vrmlImage")
    .set_help("vrml_image")
    .set_is_optional(true);
  host_type.add(arg_string).set_name("statusmapImage")
    .set_help("statusmap_image")
    .set_is_optional(true);
  host_type.add(arg_vectorstr).set_name("customVariables")
    .set_help("custom_variables");

  argument hostgroup_id("ns1__hostGroupIDType", "hostGroup");
  hostgroup_id.add(arg_string).set_name("name")
    .set_help("hostgroup_id");

  argument hostgroup_type("ns1__hostGroupType", "hostGroup");
  hostgroup_type.add(arg_string).set_name("name");
  hostgroup_type.add(arg_string).set_name("alias");
  hostgroup_type.add(arg_vectorstr).set_name("hosts");
  hostgroup_type.add(arg_vectorstr).set_name("hostgroups");
  hostgroup_type.add(arg_string).set_name("notes")
    .set_is_optional(true);
  hostgroup_type.add(arg_string).set_name("notesUrl")
    .set_is_optional(true);
  hostgroup_type.add(arg_string).set_name("actionUrl")
    .set_is_optional(true);

  argument notification("ns1__notificationType", "notification");
  notification.add(arg_string).set_name("author");
  notification.add(arg_bool).set_name("broadcast");
  notification.add(arg_string).set_name("comment");
  notification.add(arg_bool).set_name("forced");
  notification.add(arg_bool).set_name("increment");

  argument service_id("ns1__serviceIDType", "service");
  service_id.add(host_id).set_name("host");
  service_id.add(arg_string).set_name("service")
    .set_help("service_id");

  argument service_type("ns1__serviceType", "service");
  service_type.add(arg_string).set_name("hostName")
    .set_help("host_name");
  service_type.add(arg_string).set_name("hostgroupName")
    .set_help("hostgroup_name")
    .set_is_optional(true);
  service_type.add(arg_string).set_name("serviceDescription")
    .set_help("service_description");
  service_type.add(arg_string).set_name("displayName")
    .set_help("display_name")
    .set_is_optional(true);
  service_type.add(arg_vectorstr).set_name("servicegroups");
  service_type.add(arg_bool).set_name("isVolatile")
    .set_help("is_volatile")
    .set_is_optional(true);
  service_type.add(arg_string).set_name("checkCommand")
    .set_help("check_command");
  service_type.add(arg_string).set_name("initialState")
    .set_help("initial_state")
    .set_is_optional(true);
  service_type.add(arg_uint).set_name("maxCheckAttempts")
    .set_help("max_check_attempts");
  service_type.add(arg_uint).set_name("checkInterval")
    .set_help("check_interval");
  service_type.add(arg_uint).set_name("retryInterval")
    .set_help("retry_interval");
  service_type.add(arg_bool).set_name("activeChecksEnabled")
    .set_help("active_checks_enabled")
    .set_is_optional(true);
  service_type.add(arg_bool).set_name("passiveChecksEnabled")
    .set_help("passive_checks_enabled")
    .set_is_optional(true);
  service_type.add(arg_string).set_name("checkPeriod")
    .set_help("check_period");
  service_type.add(arg_bool).set_name("obsessOverService")
    .set_help("obsess_over_service")
    .set_is_optional(true);
  service_type.add(arg_bool).set_name("checkFreshness")
    .set_help("check_freshness")
    .set_is_optional(true);
  service_type.add(arg_uint).set_name("freshnessThreshold")
    .set_help("freshness_threshold")
    .set_is_optional(true);
  service_type.add(arg_string).set_name("eventHandler")
    .set_help("event_handler")
    .set_is_optional(true);
  service_type.add(arg_bool).set_name("eventHandlerEnabled")
    .set_help("event_handler_enabled")
    .set_is_optional(true);
  service_type.add(arg_uint).set_name("lowFlapThreshold")
    .set_help("low_flap_threshold")
    .set_is_optional(true);
  service_type.add(arg_uint).set_name("highFlapThreshold")
    .set_help("high_flap_threshold")
    .set_is_optional(true);
  service_type.add(arg_bool).set_name("flapDetectionEnabled")
    .set_help("flap_detection_enabled")
    .set_is_optional(true);
  service_type.add(arg_string).set_name("flapDetectionOptions")
    .set_help("flap_detection_options")
    .set_is_optional(true);
  service_type.add(arg_bool).set_name("processPerfData")
    .set_help("process_perfdata")
    .set_is_optional(true);
  service_type.add(arg_bool).set_name("retainStatusInformation")
    .set_help("retain_status_information")
    .set_is_optional(true);
  service_type.add(arg_bool).set_name("retainNonstatusInformation")
    .set_help("retain_nonstatus_information")
    .set_is_optional(true);
  service_type.add(arg_uint).set_name("notificationInterval")
    .set_help("notification_interval");
  service_type.add(arg_uint).set_name("firstNotificationDelay")
    .set_help("first_notification_delay")
    .set_is_optional(true);
  service_type.add(arg_string).set_name("notificationPeriod")
    .set_help("notification_period");
  service_type.add(arg_string).set_name("notificationOptions")
    .set_help("notification_options")
    .set_is_optional(true);
  service_type.add(arg_bool).set_name("notificationsEnabled")
    .set_help("notifications_enabled")
    .set_is_optional(true);
  service_type.add(arg_vectorstr).set_name("contacts");
  service_type.add(arg_vectorstr).set_name("contactGroups")
    .set_help("contact_groups");
  service_type.add(arg_string).set_name("stalkingOptions")
    .set_help("stalking_options")
    .set_is_optional(true);
  service_type.add(arg_string).set_name("notes")
    .set_is_optional(true);
  service_type.add(arg_string).set_name("notesUrl")
    .set_help("notes_url")
    .set_is_optional(true);
  service_type.add(arg_string).set_name("actionUrl")
    .set_help("action_url")
    .set_is_optional(true);
  service_type.add(arg_string).set_name("iconImage")
    .set_help("icon_image")
    .set_is_optional(true);
  service_type.add(arg_string).set_name("iconImageAlt")
    .set_help("icon_image_alt")
    .set_is_optional(true);
  service_type.add(arg_vectorstr).set_name("customVariables")
    .set_help("custom_variables");

  argument servicegroup_id("ns1__serviceGroupIDType", "serviceGroup");
  servicegroup_id.add(arg_string).set_name("name").set_help("servicegroup_id");

  argument servicegroup_type("ns1__serviceGroupType", "serviceGroup");
  servicegroup_type.add(arg_string).set_name("name");
  servicegroup_type.add(arg_string).set_name("alias");
  servicegroup_type.add(arg_vectorstr).set_name("services");
  servicegroup_type.add(arg_vectorstr).set_name("servicegroups");
  servicegroup_type.add(arg_string).set_name("notes")
    .set_is_optional(true);
  servicegroup_type.add(arg_string).set_name("notesUrl")
    .set_help("notes_url")
    .set_is_optional(true);
  servicegroup_type.add(arg_string).set_name("actionUrl")
    .set_help("action_url")
    .set_is_optional(true);

  argument timeperiod_id("ns1__timeperiodIDType", "timeperiod");
  timeperiod_id.add(arg_string).set_name("timeperiod")
    .set_help("timeperiod_id");

  argument serviceescalation_id("ns1__serviceEscalationIDType", "escalation");
  serviceescalation_id.add(arg_string).set_name("name")
    .set_help("host_name");
  serviceescalation_id.add(arg_string).set_name("description")
    .set_help("service_description");

  argument servicedependency_id("ns1__serviceDependencyIDType", "dependency");
  servicedependency_id.add(arg_string).set_name("hostName")
    .set_help("host_name");
  servicedependency_id.add(arg_string).set_name("serviceDescription")
    .set_help("service_description");
  servicedependency_id.add(arg_string).set_name("dependentHostName")
    .set_help("dependent_host_name");
  servicedependency_id.add(arg_string).set_name("dependentServiceDescription")
    .set_help("dependent_service_description");

  _list.push_back(arg_bool);
  _list.push_back(arg_double);
  _list.push_back(arg_int);
  _list.push_back(arg_string);
  _list.push_back(arg_time);
  _list.push_back(arg_uint);
  _list.push_back(acknowledgement);
  _list.push_back(check_result);
  _list.push_back(command_id);
  _list.push_back(command_type);
  _list.push_back(contact_id);
  _list.push_back(contact_type);
  _list.push_back(contactgroup_id);
  _list.push_back(contactgroup_type);
  _list.push_back(downtime_id);
  _list.push_back(downtime);
  _list.push_back(host_id);
  _list.push_back(host_type);
  _list.push_back(hostgroup_id);
  _list.push_back(hostgroup_type);
  _list.push_back(notification);
  _list.push_back(service_id);
  _list.push_back(service_type);
  _list.push_back(servicegroup_id);
  _list.push_back(servicegroup_type);
  _list.push_back(timeperiod_id);
  _list.push_back(serviceescalation_id);
  _list.push_back(servicedependency_id);

  argument param_1("centreonengine__getHostsEventHandlerResponse", "_param_1");
  param_1.add(command_id).set_name("command");

  argument param_2("centreonengine__getServicesEventHandlerResponse", "_param_2");
  param_2.add(command_id).set_name("command");

  argument param_23("centreonengine__hostGetCheckPeriodResponse", "_param_23");
  param_23.add(timeperiod_id).set_name("val");

  argument param_44("centreonengine__hostGetNotificationsPeriodResponse", "_param_44");
  param_44.add(timeperiod_id).set_name("val");

  argument param_65("centreonengine__serviceGetCheckPeriodResponse", "_param_65");
  param_65.add(timeperiod_id).set_name("val");

  argument param_86("centreonengine__serviceGetNotificationsPeriodResponse", "_param_86");
  param_86.add(timeperiod_id).set_name("val");

  argument param_117("centreonengine__contactGetNotificationsOnHostCommandResponse", "_param_117");
  param_117.add(command_id).set_name("command");

  argument param_118("centreonengine__contactGetNotificationsOnHostTimeperiodResponse", "_param_118");
  param_118.add(timeperiod_id).set_name("val");

  argument param_119("centreonengine__contactGetNotificationsOnServiceCommandResponse", "_param_119");
  param_119.add(command_id).set_name("command");

  argument param_120("centreonengine__contactGetNotificationsOnServiceTimeperiodResponse", "_param_120");
  param_120.add(timeperiod_id).set_name("val");

  argument param_145("centreonengine__downtimeAddToHostResponse", "_param_145");
  param_145.add(downtime_id).set_name("downtimeid");

  argument param_146("centreonengine__downtimeAddAndPropagateToHostResponse", "_param_146");
  param_146.add(downtime_id).set_name("downtimeid");

  argument param_147("centreonengine__downtimeAddAndPropagateTriggeredToHostResponse", "_param_147");
  param_147.add(downtime_id).set_name("downtimeid");

  argument param_149("centreonengine__downtimeAddToServiceResponse", "_param_149");
  param_149.add(downtime_id).set_name("downtimeid");

  _list.push_back(param_1);
  _list.push_back(param_2);
  _list.push_back(param_23);
  _list.push_back(param_44);
  _list.push_back(param_65);
  _list.push_back(param_86);
  _list.push_back(param_117);
  _list.push_back(param_118);
  _list.push_back(param_119);
  _list.push_back(param_120);
  _list.push_back(param_145);
  _list.push_back(param_146);
  _list.push_back(param_147);
  _list.push_back(param_149);
}

/**
 *  Default destructor.
 */
arg_definition::~arg_definition() throw() {

}

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
  command_id.add(arg_string).set_name("command").set_help("command_id");

  argument contact_id("ns1__contactIDType", "contact");
  contact_id.add(arg_string).set_name("contact").set_help("contact_id");

  argument contactgroup_id("ns1__contactGroupIDType", "contactGroup");
  contactgroup_id.add(arg_string).set_name("name").set_help("contactgroup_id");

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
  host_id.add(arg_string).set_name("name").set_help("host_id");

  argument hostgroup_id("ns1__hostGroupIDType", "hostGroup");
  hostgroup_id.add(arg_string).set_name("name").set_help("hostgroup_id");

  argument notification("ns1__notificationType", "notification");
  notification.add(arg_string).set_name("author");
  notification.add(arg_bool).set_name("broadcast");
  notification.add(arg_string).set_name("comment");
  notification.add(arg_bool).set_name("forced");
  notification.add(arg_bool).set_name("increment");

  argument service_id("ns1__serviceIDType", "service");
  service_id.add(host_id).set_name("host");
  service_id.add(arg_string).set_name("service").set_help("service_id");

  argument servicegroup_id("ns1__serviceGroupIDType", "serviceGroup");
  servicegroup_id.add(arg_string).set_name("name").set_help("servicegroup_id");

  argument timeperiod_id("ns1__timeperiodIDType", "timeperiod");
  timeperiod_id.add(arg_string).set_name("timeperiod").set_help("timeperiod_id");

  argument serviceescalation_id("ns1__serviceEscalationIDType", "escalation");
  serviceescalation_id.add(arg_string).set_name("name").set_help("host_name");
  serviceescalation_id.add(arg_string).set_name("description").set_help("service_description");

  argument servicedependency_id("ns1__serviceDependencyIDType", "dependency");
  servicedependency_id.add(arg_string).set_name("hostName").set_help("host_name");
  servicedependency_id.add(arg_string).set_name("serviceDescription").set_help("service_description");
  servicedependency_id.add(arg_string).set_name("dependentHostName").set_help("dependent_host_name");
  servicedependency_id.add(arg_string).set_name("dependentServiceDescription").set_help("dependent_service_description");

  _list.push_back(arg_bool);
  _list.push_back(arg_double);
  _list.push_back(arg_int);
  _list.push_back(arg_string);
  _list.push_back(arg_time);
  _list.push_back(arg_uint);
  _list.push_back(acknowledgement);
  _list.push_back(check_result);
  _list.push_back(command_id);
  _list.push_back(contact_id);
  _list.push_back(contactgroup_id);
  _list.push_back(downtime_id);
  _list.push_back(downtime);
  _list.push_back(host_id);
  _list.push_back(hostgroup_id);
  _list.push_back(notification);
  _list.push_back(service_id);
  _list.push_back(servicegroup_id);
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

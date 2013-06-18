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

#include <cassert>
#include <cstdlib>
#include "arg_definition.hh"
#include "error.hh"

using namespace com::centreon::engine::script;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Check if an argument type exist.
 *
 *  @param[in] type The type name of argument.
 *
 *  @return Return true if type exist into argument list, false
 *          otherwise.
 */
bool arg_definition::exist_argument(std::string const& type) const {
  for (std::list<argument>::const_iterator
         it(_list.begin()),
         end(_list.end());
       it != end;
       ++it) {
    if (it->get_type() == type)
      return (true);
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
argument const& arg_definition::find_argument(
                                  std::string const& type) const {
  for (std::list<argument>::const_iterator
         it(_list.begin()),
         end(_list.end());
       it != end;
       ++it) {
    if (it->get_type() == type)
      return (*it);
  }
  throw (error(type.c_str()));
}

/**
 *  Get the list of arguments who define by this object.
 *
 *  @return Return the list of all arguments.
 */
std::list<argument> const& arg_definition::get_arguments() const throw () {
  return (_list);
}

/**
 *  Get instance of arg_definition.
 *
 *  @return Return the uniq instance of arg_definition.
 */
arg_definition& arg_definition::instance() {
  static arg_definition instance;
  return (instance);
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
arg_definition::arg_definition() {
  argument arg_float("float", "value");
  argument arg_bool("bool", "value");
  argument arg_double("double", "value");
  argument arg_int("int", "value");
  argument arg_string("std::string", "value");
  argument arg_time("time_t", "value");
  argument arg_uint("unsigned int", "value");
  argument arg_ulong64("ULONG64", "value");
  argument arg_vectorstr("std::vector<std::string>", "value");

  argument acknowledgement(
             "ns1__acknowledgementType",
             "acknowledgement");
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

  argument resource_id("ns1__resourceUserIDType", "resource");
  resource_id.add(arg_uint).set_name("id")
    .set_help("resource_id");

  argument comment_id("ns1__commentIDType", "comment");
  comment_id.add(arg_ulong64).set_name("comment");

  argument comment("ns1__commentType", "comment");
  comment.add(arg_string).set_name("author");
  comment.add(arg_string).set_name("text");
  comment.add(arg_bool).set_name("persistent");

  argument contact_id("ns1__contactIDType", "contact");
  contact_id.add(arg_string).set_name("name")
    .set_help("contact_id");

  argument contactgroup_id("ns1__contactgroupIDType", "contactgroup");
  contactgroup_id.add(arg_string).set_name("name")
    .set_help("contactgroup_id");

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
  argument vector_host_id("std::vector<ns1__hostIDType>", "value");

  argument hostgroup_id("ns1__hostgroupIDType", "hostgroup");
  hostgroup_id.add(arg_string).set_name("name")
    .set_help("hostgroup_id");
  argument vector_hostgroup_id("std::vector<ns1__hostgroupIDType>", "value");

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
  argument vector_service_id("std::vector<ns1__serviceIDType>", "value");

  argument servicegroup_id("ns1__servicegroupIDType", "servicegroup");
  servicegroup_id.add(arg_string).set_name("name").set_help("servicegroup_id");
  argument vector_servicegroup_id("std::vector<ns1__servicegroupIDType>", "value");

  argument servicegroup_type("ns1__servicegroupType", "servicegroup");
  servicegroup_type.add(servicegroup_id).set_name("id");
  servicegroup_type.add(arg_string).set_name("alias")
    .set_is_optional(true);
  servicegroup_type.add(arg_vectorstr).set_name("serviceMembers")
    .set_is_optional(true)
    .set_is_array(true);
  servicegroup_type.add(arg_vectorstr).set_name("servicegroupMembers")
    .set_help("servicegroup_members")
    .set_is_optional(true)
    .set_is_array(true);
  servicegroup_type.add(arg_string).set_name("notes")
    .set_is_optional(true);
  servicegroup_type.add(arg_string).set_name("notesUrl")
    .set_help("notes_url")
    .set_is_optional(true);
  servicegroup_type.add(arg_string).set_name("actionUrl")
    .set_help("action_url")
    .set_is_optional(true);

  argument timeperiod_id("ns1__timeperiodIDType", "timeperiod");
  timeperiod_id.add(arg_string).set_name("name")
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

  _list.push_back(arg_float);
  _list.push_back(arg_bool);
  _list.push_back(arg_double);
  _list.push_back(arg_int);
  _list.push_back(arg_string);
  _list.push_back(arg_time);
  _list.push_back(arg_ulong64);
  _list.push_back(arg_uint);
  _list.push_back(acknowledgement);
  _list.push_back(check_result);
  _list.push_back(command_id);
  _list.push_back(resource_id);
  _list.push_back(comment_id);
  _list.push_back(comment);
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

  // argument param_1("centreonengine__addCommandResponse", "_param_1");
  // param_.add().set_name("");
  // _list.push_back(param_1);

  // argument param_2("centreonengine__addContactGroupResponse", "_param_2");
  // param_2.add().set_name("");
  // _list.push_back(param_2);

  // argument param_3("centreonengine__addHostGroupResponse", "_param_3");
  // param_3.add().set_name("");
  // _list.push_back(param_3);

  // argument param_4("centreonengine__addServiceGroupResponse", "_param_4");
  // param_4.add().set_name("");
  // _list.push_back(param_4);

  // argument param_5("centreonengine__addHostResponse", "_param_5");
  // param_5.add().set_name("");
  // _list.push_back(param_5);

  // argument param_6("centreonengine__addServiceResponse", "_param_6");
  // param_6.add().set_name("");
  // _list.push_back(param_6);

  // argument param_7("centreonengine__addContactResponse", "_param_7");
  // param_7.add().set_name("");
  // _list.push_back(param_7);

  // argument param_8("centreonengine__removeHostResponse", "_param_8");
  // param_8.add().set_name("");
  // _list.push_back(param_8);

  // argument param_9("centreonengine__removeHostGroupResponse", "_param_9");
  // param_9.add().set_name("");
  // _list.push_back(param_9);

  // argument param_10("centreonengine__removeServiceResponse", "_param_10");
  // param_10.add().set_name("");
  // _list.push_back(param_10);

  // argument param_11("centreonengine__removeServiceGroupResponse", "_param_11");
  // param_11.add().set_name("");
  // _list.push_back(param_11);

  // argument param_12("centreonengine__removeContactResponse", "_param_12");
  // param_12.add().set_name("");
  // _list.push_back(param_12);

  // argument param_13("centreonengine__removeContactGroupResponse", "_param_13");
  // param_13.add().set_name("");
  // _list.push_back(param_13);

  // argument param_14("centreonengine__removeCommandResponse", "_param_14");
  // param_14.add().set_name("");
  // _list.push_back(param_14);

  // argument param_15("centreonengine__removeServiceEscalationResponse", "_param_15");
  // param_15.add().set_name("");
  // _list.push_back(param_15);

  // argument param_16("centreonengine__removeServiceDependencyResponse", "_param_16");
  // param_16.add().set_name("");
  // _list.push_back(param_16);

  argument param_17("centreonengine__getHostsEventHandlerResponse", "_param_17");
  param_17.add(command_id).set_name("command");
  _list.push_back(param_17);

  argument param_18("centreonengine__getServicesEventHandlerResponse", "_param_18");
  param_18.add(command_id).set_name("command");
  _list.push_back(param_18);

  // argument param_19("centreonengine__setEventHandlersEnabledResponse", "_param_19");
  // param_19.add().set_name("");
  // _list.push_back(param_19);

  // argument param_20("centreonengine__setFailurePredictionEnabledResponse", "_param_20");
  // param_20.add().set_name("");
  // _list.push_back(param_20);

  // argument param_21("centreonengine__setFlapDetectionEnabledResponse", "_param_21");
  // param_21.add().set_name("");
  // _list.push_back(param_21);

  // argument param_22("centreonengine__setHostsChecksActiveEnabledResponse", "_param_22");
  // param_22.add().set_name("");
  // _list.push_back(param_22);

  // argument param_23("centreonengine__setHostsChecksPassiveEnabledResponse", "_param_23");
  // param_23.add().set_name("");
  // _list.push_back(param_23);

  // argument param_24("centreonengine__setHostsEventHandlerResponse", "_param_24");
  // param_24.add().set_name("");
  // _list.push_back(param_24);

  // argument param_25("centreonengine__setHostsFreshnessChecksEnabledResponse", "_param_25");
  // param_25.add().set_name("");
  // _list.push_back(param_25);

  // argument param_26("centreonengine__setHostsObsessOverEnabledResponse", "_param_26");
  // param_26.add().set_name("");
  // _list.push_back(param_26);

  // argument param_27("centreonengine__setNotificationsEnabledResponse", "_param_27");
  // param_27.add().set_name("");
  // _list.push_back(param_27);

  // argument param_28("centreonengine__setPerfdataProcessingEnabledResponse", "_param_28");
  // param_28.add().set_name("");
  // _list.push_back(param_28);

  // argument param_29("centreonengine__setServicesChecksActiveEnabledResponse", "_param_29");
  // param_29.add().set_name("");
  // _list.push_back(param_29);

  // argument param_30("centreonengine__setServicesChecksPassiveEnabledResponse", "_param_30");
  // param_30.add().set_name("");
  // _list.push_back(param_30);

  // argument param_31("centreonengine__setServicesEventHandlerResponse", "_param_31");
  // param_31.add().set_name("");
  // _list.push_back(param_31);

  // argument param_32("centreonengine__setServicesFreshnessChecksEnabledResponse", "_param_32");
  // param_32.add().set_name("");
  // _list.push_back(param_32);

  // argument param_33("centreonengine__setServicesObsessOverEnabledResponse", "_param_33");
  // param_33.add().set_name("");
  // _list.push_back(param_33);

  // argument param_34("centreonengine__processRestartResponse", "_param_34");
  // param_34.add().set_name("");
  // _list.push_back(param_34);

  // argument param_35("centreonengine__processShutdownResponse", "_param_35");
  // param_35.add().set_name("");
  // _list.push_back(param_35);

  // argument param_36("centreonengine__stateInformationLoadResponse", "_param_36");
  // param_36.add().set_name("");
  // _list.push_back(param_36);

  // argument param_37("centreonengine__stateInformationSaveResponse", "_param_37");
  // param_37.add().set_name("");
  // _list.push_back(param_37);

  // argument param_38("centreonengine__hostSetAddressResponse", "_param_38");
  // param_38.add().set_name("");
  // _list.push_back(param_38);

  argument param_39("centreonengine__hostGetCheckPeriodResponse", "_param_39");
  param_39.add(timeperiod_id).set_name("val");
  _list.push_back(param_39);

  // argument param_40("centreonengine__hostSetCheckActiveEnabledResponse", "_param_40");
  // param_40.add().set_name("");
  // _list.push_back(param_40);

  // argument param_41("centreonengine__hostSetCheckCommandResponse", "_param_41");
  // param_41.add().set_name("");
  // _list.push_back(param_41);

  // argument param_42("centreonengine__hostSetCheckIntervalNormalResponse", "_param_42");
  // param_42.add().set_name("");
  // _list.push_back(param_42);

  // argument param_43("centreonengine__hostSetCheckIntervalRetryResponse", "_param_43");
  // param_43.add().set_name("");
  // _list.push_back(param_43);

  // argument param_44("centreonengine__hostSetCheckMaxAttemptsResponse", "_param_44");
  // param_44.add().set_name("");
  // _list.push_back(param_44);

  // argument param_45("centreonengine__hostSetCheckPassiveEnabledResponse", "_param_45");
  // param_45.add().set_name("");
  // _list.push_back(param_45);

  // argument param_46("centreonengine__hostSetCheckPeriodResponse", "_param_46");
  // param_46.add().set_name("");
  // _list.push_back(param_46);

  // argument param_47("centreonengine__hostSetEventHandlerResponse", "_param_47");
  // param_47.add().set_name("");
  // _list.push_back(param_47);

  // argument param_48("centreonengine__hostSetEventHandlerEnabledResponse", "_param_48");
  // param_48.add().set_name("");
  // _list.push_back(param_48);

  // argument param_49("centreonengine__hostSetFailurePredictionEnabledResponse", "_param_49");
  // param_49.add().set_name("");
  // _list.push_back(param_49);

  // argument param_50("centreonengine__hostSetFlapDetectionEnabledResponse", "_param_50");
  // param_50.add().set_name("");
  // _list.push_back(param_50);

  // argument param_51("centreonengine__hostSetFlapDetectionOnDownResponse", "_param_51");
  // param_51.add().set_name("");
  // _list.push_back(param_51);

  // argument param_52("centreonengine__hostSetFlapDetectionOnUnreachableResponse", "_param_52");
  // param_52.add().set_name("");
  // _list.push_back(param_52);

  // argument param_53("centreonengine__hostSetFlapDetectionOnUpResponse", "_param_53");
  // param_53.add().set_name("");
  // _list.push_back(param_53);

  // argument param_54("centreonengine__hostSetFlapDetectionThresholdHighResponse", "_param_54");
  // param_54.add().set_name("");
  // _list.push_back(param_54);

  // argument param_55("centreonengine__hostSetFlapDetectionThresholdLowResponse", "_param_55");
  // param_55.add().set_name("");
  // _list.push_back(param_55);

  // argument param_56("centreonengine__hostSetFreshnessCheckEnabledResponse", "_param_56");
  // param_56.add().set_name("");
  // _list.push_back(param_56);

  // argument param_57("centreonengine__hostSetFreshnessThresholdResponse", "_param_57");
  // param_57.add().set_name("");
  // _list.push_back(param_57);

  // argument param_58("centreonengine__hostSetNameAliasResponse", "_param_58");
  // param_58.add().set_name("");
  // _list.push_back(param_58);

  // argument param_59("centreonengine__hostSetNameDisplayResponse", "_param_59");
  // param_59.add().set_name("");
  // _list.push_back(param_59);

  argument param_60("centreonengine__hostGetNotificationsPeriodResponse", "_param_60");
  param_60.add(timeperiod_id).set_name("val");
  _list.push_back(param_60);

  // argument param_61("centreonengine__hostSetNotificationsBeyondEnabledResponse", "_param_61");
  // param_61.add().set_name("");
  // _list.push_back(param_61);

  // argument param_62("centreonengine__hostSetNotificationsEnabledResponse", "_param_62");
  // param_62.add().set_name("");
  // _list.push_back(param_62);

  // argument param_63("centreonengine__hostSetNotificationsFirstDelayResponse", "_param_63");
  // param_63.add().set_name("");
  // _list.push_back(param_63);

  // argument param_64("centreonengine__hostSetNotificationsIntervalResponse", "_param_64");
  // param_64.add().set_name("");
  // _list.push_back(param_64);

  // argument param_65("centreonengine__hostSetNotificationsOnDownResponse", "_param_65");
  // param_65.add().set_name("");
  // _list.push_back(param_65);

  // argument param_66("centreonengine__hostSetNotificationsOnDowntimeResponse", "_param_66");
  // param_66.add().set_name("");
  // _list.push_back(param_66);

  // argument param_67("centreonengine__hostSetNotificationsOnFlappingResponse", "_param_67");
  // param_67.add().set_name("");
  // _list.push_back(param_67);

  // argument param_68("centreonengine__hostSetNotificationsOnRecoveryResponse", "_param_68");
  // param_68.add().set_name("");
  // _list.push_back(param_68);

  // argument param_69("centreonengine__hostSetNotificationsOnSelfAndChildrenEnabledResponse", "_param_69");
  // param_69.add().set_name("");
  // _list.push_back(param_69);

  // argument param_70("centreonengine__hostSetNotificationsOnUnreachableResponse", "_param_70");
  // param_70.add().set_name("");
  // _list.push_back(param_70);

  // argument param_71("centreonengine__hostSetNotificationsPeriodResponse", "_param_71");
  // param_71.add().set_name("");
  // _list.push_back(param_71);

  // argument param_72("centreonengine__hostSetObsessOverResponse", "_param_72");
  // param_72.add().set_name("");
  // _list.push_back(param_72);

  // argument param_73("centreonengine__hostSetPerfdataProcessingEnabledResponse", "_param_73");
  // param_73.add().set_name("");
  // _list.push_back(param_73);

  // argument param_74("centreonengine__hostSetRetainStatusInformationResponse", "_param_74");
  // param_74.add().set_name("");
  // _list.push_back(param_74);

  // argument param_75("centreonengine__hostSetRetainNonStatusInformationResponse", "_param_75");
  // param_75.add().set_name("");
  // _list.push_back(param_75);

  // argument param_76("centreonengine__hostSetServicesCheckActiveEnabledResponse", "_param_76");
  // param_76.add().set_name("");
  // _list.push_back(param_76);

  // argument param_77("centreonengine__hostSetServicesNotificationsEnabledResponse", "_param_77");
  // param_77.add().set_name("");
  // _list.push_back(param_77);

  // argument param_78("centreonengine__hostSetStalkOnDownResponse", "_param_78");
  // param_78.add().set_name("");
  // _list.push_back(param_78);

  // argument param_79("centreonengine__hostSetStalkOnUnreachableResponse", "_param_79");
  // param_79.add().set_name("");
  // _list.push_back(param_79);

  // argument param_80("centreonengine__hostSetStalkOnUpResponse", "_param_80");
  // param_80.add().set_name("");
  // _list.push_back(param_80);

  // argument param_81("centreonengine__serviceGetCheckPeriodResponse", "_param_81");
  // param_81.add(timeperiod_id).set_name("val");
  // _list.push_back(param_81);

  // argument param_82("centreonengine__serviceSetCheckActiveEnabledResponse", "_param_82");
  // param_82.add().set_name("");
  // _list.push_back(param_82);

  // argument param_83("centreonengine__serviceSetCheckCommandResponse", "_param_83");
  // param_83.add().set_name("");
  // _list.push_back(param_83);

  // argument param_84("centreonengine__serviceSetCheckIntervalNormalResponse", "_param_84");
  // param_84.add().set_name("");
  // _list.push_back(param_84);

  // argument param_84("centreonengine__serviceSetCheckIntervalRetryResponse", "_param_84");
  // param_84.add().set_name("");
  // _list.push_back(param_84);

  // argument param_85("centreonengine__serviceSetCheckMaxAttemptsResponse", "_param_85");
  // param_85.add().set_name("");
  // _list.push_back(param_85);

  // argument param_86("centreonengine__serviceSetCheckPassiveEnabledResponse", "_param_86");
  // param_86.add().set_name("");
  // _list.push_back(param_86);

  // argument param_87("centreonengine__serviceSetCheckPeriodResponse", "_param_87");
  // param_87.add().set_name("");
  // _list.push_back(param_87);

  // argument param_88("centreonengine__serviceSetEventHandlerResponse", "_param_88");
  // param_88.add().set_name("");
  // _list.push_back(param_88);

  // argument param_89("centreonengine__serviceSetEventHandlerEnabledResponse", "_param_89");
  // param_89.add().set_name("");
  // _list.push_back(param_89);

  // argument param_90("centreonengine__serviceSetFailurePredictionEnabledResponse", "_param_90");
  // param_90.add().set_name("");
  // _list.push_back(param_90);

  // argument param_91("centreonengine__serviceSetFlapDetectionEnabledResponse", "_param_91");
  // param_91.add().set_name("");
  // _list.push_back(param_91);

  // argument param_92("centreonengine__serviceSetFlapDetectionOnCriticalResponse", "_param_92");
  // param_92.add().set_name("");
  // _list.push_back(param_92);

  // argument param_93("centreonengine__serviceSetFlapDetectionOnOkResponse", "_param_93");
  // param_93.add().set_name("");
  // _list.push_back(param_93);

  // argument param_94("centreonengine__serviceSetFlapDetectionOnUnknownResponse", "_param_94");
  // param_94.add().set_name("");
  // _list.push_back(param_94);

  // argument param_95("centreonengine__serviceSetFlapDetectionOnWarningResponse", "_param_95");
  // param_95.add().set_name("");
  // _list.push_back(param_95);

  // argument param_96("centreonengine__serviceSetFlapDetectionThresholdHighResponse", "_param_96");
  // param_96.add().set_name("");
  // _list.push_back(param_96);

  // argument param_97("centreonengine__serviceSetFlapDetectionThresholdLowResponse", "_param_97");
  // param_97.add().set_name("");
  // _list.push_back(param_97);

  // argument param_98("centreonengine__serviceSetFreshnessCheckEnabledResponse", "_param_98");
  // param_98.add().set_name("");
  // _list.push_back(param_98);

  // argument param_99("centreonengine__serviceSetFreshnessThresholdResponse", "_param_99");
  // param_99.add().set_name("");
  // _list.push_back(param_99);

  // argument param_100("centreonengine__serviceSetNameDisplayResponse", "_param_100");
  // param_100.add().set_name("");
  // _list.push_back(param_100);

  argument param_101("centreonengine__serviceGetNotificationsPeriodResponse", "_param_101");
  param_101.add(timeperiod_id).set_name("val");
  _list.push_back(param_101);

  // argument param_102("centreonengine__serviceSetNotificationsEnabledResponse", "_param_102");
  // param_102.add().set_name("");
  // _list.push_back(param_102);

  // argument param_103("centreonengine__serviceSetNotificationsFirstDelayResponse", "_param_103");
  // param_103.add().set_name("");
  // _list.push_back(param_103);

  // argument param_104("centreonengine__serviceSetNotificationsIntervalResponse", "_param_104");
  // param_104.add().set_name("");
  // _list.push_back(param_104);

  // argument param_105("centreonengine__serviceSetNotificationsOnCriticalResponse", "_param_105");
  // param_105.add().set_name("");
  // _list.push_back(param_105);

  // argument param_106("centreonengine__serviceSetNotificationsOnDowntimeResponse", "_param_106");
  // param_106.add().set_name("");
  // _list.push_back(param_106);

  // argument param_107("centreonengine__serviceSetNotificationsOnFlappingResponse", "_param_107");
  // param_107.add().set_name("");
  // _list.push_back(param_107);

  // argument param_108("centreonengine__serviceSetNotificationsOnRecoveryResponse", "_param_108");
  // param_108.add().set_name("");
  // _list.push_back(param_108);

  // argument param_109("centreonengine__serviceSetNotificationsOnUnknownResponse", "_param_109");
  // param_109.add().set_name("");
  // _list.push_back(param_109);

  // argument param_110("centreonengine__serviceSetNotificationsOnWarningResponse", "_param_110");
  // param_110.add().set_name("");
  // _list.push_back(param_110);

  // argument param_111("centreonengine__serviceSetNotificationsPeriodResponse", "_param_111");
  // param_111.add().set_name("");
  // _list.push_back(param_111);

  // argument param_112("centreonengine__serviceSetObsessOverResponse", "_param_112");
  // param_112.add().set_name("");
  // _list.push_back(param_112);

  // argument param_113("centreonengine__serviceSetPerfdataProcessingEnabledResponse", "_param_113");
  // param_113.add().set_name("");
  // _list.push_back(param_113);

  // argument param_114("centreonengine__serviceSetRetainStatusInformationResponse", "_param_114");
  // param_114.add().set_name("");
  // _list.push_back(param_114);

  // argument param_115("centreonengine__serviceSetRetainNonStatusInformationResponse", "_param_115");
  // param_115.add().set_name("");
  // _list.push_back(param_115);

  // argument param_116("centreonengine__serviceSetStalkOnCriticalResponse", "_param_116");
  // param_116.add().set_name("");
  // _list.push_back(param_116);

  // argument param_117("centreonengine__serviceSetStalkOnOkResponse", "_param_117");
  // param_117.add().set_name("");
  // _list.push_back(param_117);

  // argument param_118("centreonengine__serviceSetStalkOnUnknownResponse", "_param_118");
  // param_118.add().set_name("");
  // _list.push_back(param_118);

  // argument param_119("centreonengine__serviceSetStalkOnWarningResponse", "_param_119");
  // param_119.add().set_name("");
  // _list.push_back(param_119);

  // argument param_120("centreonengine__acknowledgementOnHostAddResponse", "_param_120");
  // param_120.add().set_name("");
  // _list.push_back(param_120);

  // argument param_121("centreonengine__acknowledgementOnHostRemoveResponse", "_param_121");
  // param_121.add().set_name("");
  // _list.push_back(param_121);

  // argument param_122("centreonengine__acknowledgementOnServiceAddResponse", "_param_122");
  // param_122.add().set_name("");
  // _list.push_back(param_122);

  // argument param_123("centreonengine__acknowledgementOnServiceRemoveResponse", "_param_123");
  // param_123.add().set_name("");
  // _list.push_back(param_123);

  // argument param_124("centreonengine__checkHostProcessResultResponse", "_param_124");
  // param_124.add().set_name("");
  // _list.push_back(param_124);

  // argument param_125("centreonengine__checkHostScheduleResponse", "_param_125");
  // param_125.add().set_name("");
  // _list.push_back(param_125);

  // argument param_126("centreonengine__checkHostScheduleForcedResponse", "_param_126");
  // param_126.add().set_name("");
  // _list.push_back(param_126);

  // argument param_127("centreonengine__checkHostScheduleServicesResponse", "_param_127");
  // param_127.add().set_name("");
  // _list.push_back(param_127);

  // argument param_128("centreonengine__checkHostScheduleServicesForcedResponse", "_param_128");
  // param_128.add().set_name("");
  // _list.push_back(param_128);

  // argument param_129("centreonengine__checkServiceProcessResultResponse", "_param_129");
  // param_129.add().set_name("");
  // _list.push_back(param_129);

  // argument param_130("centreonengine__checkServiceScheduleResponse", "_param_130");
  // param_130.add().set_name("");
  // _list.push_back(param_130);

  // argument param_131("centreonengine__checkServiceScheduleForcedResponse", "_param_131");
  // param_131.add().set_name("");
  // _list.push_back(param_131);

  // argument param_132("centreonengine__contactGetNotificationsOnHostCommandResponse", "_param_132");
  // param_132.add(command_id).set_name("command");
  // _list.push_back(param_132);

  argument param_133("centreonengine__contactGetNotificationsOnHostTimeperiodResponse", "_param_133");
  param_133.add(timeperiod_id).set_name("val");
  _list.push_back(param_133);

  // argument param_134("centreonengine__contactGetNotificationsOnServiceCommandResponse", "_param_134");
  // param_134.add(command_id).set_name("command");
  // _list.push_back(param_134);

  argument param_135("centreonengine__contactGetNotificationsOnServiceTimeperiodResponse", "_param_135");
  param_135.add(timeperiod_id).set_name("val");
  _list.push_back(param_135);

  // argument param_136("centreonengine__contactSetAliasResponse", "_param_136");
  // param_136.add().set_name("");
  // _list.push_back(param_136);

  // argument param_137("centreonengine__contactSetCanSubmitCommandsResponse", "_param_137");
  // param_137.add().set_name("");
  // _list.push_back(param_137);

  // argument param_138("centreonengine__contactSetEmailResponse", "_param_138");
  // param_138.add().set_name("");
  // _list.push_back(param_138);

  // argument param_139("centreonengine__contactSetNotificationsOnHostCommandResponse", "_param_139");
  // param_139.add().set_name("");
  // _list.push_back(param_139);

  // argument param_140("centreonengine__contactSetNotificationsOnHostDownResponse", "_param_140");
  // param_140.add().set_name("");
  // _list.push_back(param_140);

  // argument param_141("centreonengine__contactSetNotificationsOnHostDowntimeResponse", "_param_141");
  // param_141.add().set_name("");
  // _list.push_back(param_141);

  // argument param_142("centreonengine__contactSetNotificationsOnHostEnabledResponse", "_param_142");
  // param_142.add().set_name("");
  // _list.push_back(param_142);

  // argument param_143("centreonengine__contactSetNotificationsOnHostFlappingResponse", "_param_143");
  // param_143.add().set_name("");
  // _list.push_back(param_143);

  // argument param_144("centreonengine__contactSetNotificationsOnHostRecoveryResponse", "_param_144");
  // param_144.add().set_name("");
  // _list.push_back(param_144);

  // argument param_145("centreonengine__contactSetNotificationsOnHostTimeperiodResponse", "_param_145");
  // param_145.add().set_name("");
  // _list.push_back(param_145);

  // argument param_146("centreonengine__contactSetNotificationsOnHostUnreachableResponse", "_param_146");
  // param_146.add().set_name("");
  // _list.push_back(param_146);

  // argument param_147("centreonengine__contactSetNotificationsOnServiceCommandResponse", "_param_147");
  // param_147.add().set_name("");
  // _list.push_back(param_147);

  // argument param_148("centreonengine__contactSetNotificationsOnServiceCriticalResponse", "_param_148");
  // param_148.add().set_name("");
  // _list.push_back(param_148);

  // argument param_149("centreonengine__contactSetNotificationsOnServiceDowntimeResponse", "_param_149");
  // param_149.add().set_name("");
  // _list.push_back(param_149);

  // argument param_150("centreonengine__contactSetNotificationsOnServiceEnabledResponse", "_param_150");
  // param_150.add().set_name("");
  // _list.push_back(param_150);

  // argument param_151("centreonengine__contactSetNotificationsOnServiceFlappingResponse", "_param_151");
  // param_151.add().set_name("");
  // _list.push_back(param_151);

  // argument param_152("centreonengine__contactSetNotificationsOnServiceRecoveryResponse", "_param_152");
  // param_152.add().set_name("");
  // _list.push_back(param_152);

  // argument param_153("centreonengine__contactSetNotificationsOnServiceTimeperiodResponse", "_param_153");
  // param_153.add().set_name("");
  // _list.push_back(param_153);

  // argument param_154("centreonengine__contactSetNotificationsOnServiceUnknownResponse", "_param_154");
  // param_154.add().set_name("");
  // _list.push_back(param_154);

  // argument param_155("centreonengine__contactSetNotificationsOnServiceWarningResponse", "_param_155");
  // param_155.add().set_name("");
  // _list.push_back(param_155);

  // argument param_156("centreonengine__contactSetPagerResponse", "_param_156");
  // param_156.add().set_name("");
  // _list.push_back(param_156);

  // argument param_157("centreonengine__contactSetRetainStatusInformationResponse", "_param_157");
  // param_157.add().set_name("");
  // _list.push_back(param_157);

  // argument param_158("centreonengine__contactSetRetainStatusNonInformationResponse", "_param_158");
  // param_158.add().set_name("");
  // _list.push_back(param_158);

  // argument param_159("centreonengine__downtimeDeleteResponse", "_param_159");
  // param_159.add().set_name("");
  // _list.push_back(param_159);

  argument param_160("centreonengine__downtimeAddToHostResponse", "_param_160");
  param_160.add(downtime_id).set_name("downtimeid");
  _list.push_back(param_160);

  argument param_161("centreonengine__downtimeAddAndPropagateToHostResponse", "_param_161");
  param_161.add(downtime_id).set_name("downtimeid");
  _list.push_back(param_161);

  argument param_162("centreonengine__downtimeAddAndPropagateTriggeredToHostResponse", "_param_162");
  param_162.add(downtime_id).set_name("downtimeid");
  _list.push_back(param_162);

  // argument param_163("centreonengine__downtimeAddToHostServicesResponse", "_param_163");
  // param_163.add().set_name("");
  // _list.push_back(param_163);

  argument param_164("centreonengine__downtimeAddToServiceResponse", "_param_164");
  param_164.add(downtime_id).set_name("downtimeid");
  _list.push_back(param_164);

  // argument param_165("centreonengine__notificationHostDelayResponse", "_param_165");
  // param_165.add().set_name("");
  // _list.push_back(param_165);

  // argument param_166("centreonengine__notificationHostSendResponse", "_param_166");
  // param_166.add().set_name("");
  // _list.push_back(param_166);

  // argument param_167("centreonengine__notificationServiceDelayResponse", "_param_167");
  // param_167.add().set_name("");
  // _list.push_back(param_167);

  // argument param_168("centreonengine__notificationServiceSendResponse", "_param_168");
  // param_168.add().set_name("");
  // _list.push_back(param_168);

  // argument param_169("centreonengine__addHostDependencyResponse", "_param_169");
  // param_169.add().set_name("");
  // _list.push_back(param_169);

  // argument param_170("centreonengine__addHostEscalationResponse", "_param_170");
  // param_170.add().set_name("");
  // _list.push_back(param_170);

  // argument param_171("centreonengine__addServiceDependencyResponse", "_param_171");
  // param_171.add().set_name("");
  // _list.push_back(param_171);

  // argument param_172("centreonengine__addServiceEscalationResponse", "_param_172");
  // param_172.add().set_name("");
  // _list.push_back(param_172);

  // argument param_173("centreonengine__addTimeperiodResponse", "_param_173");
  // param_173.add().set_name("");
  // _list.push_back(param_173);


  // argument param_174("centreonengine__dumpObjectListResponse", "_param_174");
  // param_174.add().set_name("");
  // _list.push_back(param_174);
}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
arg_definition::arg_definition(arg_definition const& right) {
  (void)right;
  assert(!"argument definition is not copyable");
  abort();
}

/**
 *  Destructor.
 */
arg_definition::~arg_definition() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
arg_definition& arg_definition::operator=(arg_definition const& right) {
  (void)right;
  assert(!"argument definition is not copyable");
  abort();
  return (*this);
}

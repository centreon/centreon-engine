/*
 * Copyright 2019 Centreon (https://www.centreon.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For more information : contact@centreon.com
 *
 */

#include "test_engine.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::downtimes;

configuration::contact TestEngine::new_configuration_contact(
    std::string const& name,
    bool full) const {
  if (full) {
    // Add command.
    {
      configuration::command cmd;
      cmd.parse("command_name", "cmd");
      cmd.parse("command_line", "true");
      configuration::applier::command aplyr;
      aplyr.add_object(cmd);
    }
    // Add timeperiod.
    {
      configuration::timeperiod tperiod;
      tperiod.parse("timeperiod_name", "24x7");
      tperiod.parse("alias", "24x7");
      tperiod.parse("monday", "00:00-24:00");
      tperiod.parse("tuesday", "00:00-24:00");
      tperiod.parse("wednesday", "00:00-24:00");
      tperiod.parse("thursday", "00:00-24:00");
      tperiod.parse("friday", "00:00-24:00");
      tperiod.parse("saterday", "00:00-24:00");
      tperiod.parse("sunday", "00:00-24:00");
      configuration::applier::timeperiod aplyr;
      aplyr.add_object(tperiod);
    }
  }
  // Valid contact configuration
  // (will generate 0 warnings or 0 errors).
  configuration::contact ctct;
  ctct.parse("contact_name", name.c_str());
  ctct.parse("host_notification_period", "24x7");
  ctct.parse("service_notification_period", "24x7");
  ctct.parse("host_notification_commands", "cmd");
  ctct.parse("service_notification_commands", "cmd");
  ctct.parse("host_notification_options", "d,r,f,s");
  ctct.parse("service_notification_options", "a");
  ctct.parse("host_notifications_enabled", "1");
  ctct.parse("service_notifications_enabled", "1");
  return ctct;
}

configuration::contactgroup TestEngine::new_configuration_contactgroup(
    std::string const& name,
    std::string const& contactname) {
  configuration::contactgroup cg;
  cg.parse("contactgroup_name", name.c_str());
  cg.parse("alias", name.c_str());
  cg.parse("members", contactname.c_str());
  return cg;
}

configuration::serviceescalation
TestEngine::new_configuration_serviceescalation(
    std::string const& hostname,
    std::string const& svc_desc,
    std::string const& contactgroup) {
  configuration::serviceescalation se;
  se.parse("first_notification", "2");
  se.parse("last_notification", "11");
  se.parse("notification_interval", "9");
  se.parse("escalation_options", "w,u,c,r");
  se.parse("host_name", hostname.c_str());
  se.parse("service_description", svc_desc.c_str());
  se.parse("contact_groups", contactgroup.c_str());
  return se;
}

configuration::hostdependency TestEngine::new_configuration_hostdependency(
    std::string const& hostname,
    std::string const& dep_hostname) {
  configuration::hostdependency hd;
  hd.parse("master_host", hostname.c_str());
  hd.parse("dependent_host", dep_hostname.c_str());
  hd.parse("notification_failure_options", "u,d");
  hd.parse("inherits_parent", "1");
  hd.dependency_type(configuration::hostdependency::notification_dependency);
  return hd;
}

configuration::servicedependency
TestEngine::new_configuration_servicedependency(
    std::string const& hostname,
    std::string const& service,
    std::string const& dep_hostname,
    std::string const& dep_service) {
  configuration::servicedependency sd;
  sd.parse("master_host", hostname.c_str());
  sd.parse("master_description", service.c_str());
  sd.parse("dependent_host", dep_hostname.c_str());
  sd.parse("dependent_description", dep_service.c_str());
  sd.parse("notification_failure_options", "u,w,c");
  sd.dependency_type(configuration::servicedependency::notification_dependency);
  return sd;
}

configuration::host TestEngine::new_configuration_host(
    std::string const& hostname,
    std::string const& contacts,
    uint64_t hst_id) {
  configuration::host hst;
  hst.parse("host_name", hostname.c_str());
  hst.parse("address", "127.0.0.1");
  hst.parse("_HOST_ID", std::to_string(hst_id).c_str());
  hst.parse("contacts", contacts.c_str());

  configuration::command cmd("hcmd");
  cmd.parse("command_line", "echo 0");
  hst.parse("check_command", "hcmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);

  return hst;
}

configuration::hostescalation TestEngine::new_configuration_hostescalation(
    std::string const& hostname,
    std::string const& contactgroup,
    uint32_t first_notif,
    uint32_t last_notif,
    uint32_t interval_notif) {
  configuration::hostescalation he;
  he.parse("first_notification", std::to_string(first_notif).c_str());
  he.parse("last_notification", std::to_string(last_notif).c_str());
  he.parse("notification_interval", std::to_string(interval_notif).c_str());
  he.parse("escalation_options", "d,u,r");
  he.parse("host_name", hostname.c_str());
  he.parse("contact_groups", contactgroup.c_str());
  return he;
}

configuration::service TestEngine::new_configuration_service(
    std::string const& hostname,
    std::string const& description,
    std::string const& contacts,
    uint64_t svc_id) {
  configuration::service svc;
  svc.parse("host_name", hostname.c_str());
  svc.parse("description", description.c_str());
  svc.parse("_HOST_ID", "12");
  svc.parse("_SERVICE_ID", std::to_string(svc_id).c_str());
  svc.parse("contacts", contacts.c_str());

  // We fake here the expand_object on configuration::service
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);

  return svc;
}

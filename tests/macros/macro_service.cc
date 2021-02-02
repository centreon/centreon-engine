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

#include <gtest/gtest.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <fmt/format.h>
#include "com/centreon/engine/globals.hh"

#include "../helper.hh"
#include "../test_engine.hh"
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/checks/checker.hh"
#include <com/centreon/engine/configuration/applier/command.hh>
#include <com/centreon/engine/configuration/applier/host.hh>
#include <com/centreon/engine/configuration/applier/hostgroup.hh>
#include <com/centreon/engine/configuration/applier/service.hh>
#include <com/centreon/engine/configuration/applier/contact.hh>
#include <com/centreon/engine/configuration/applier/state.hh>
#include <com/centreon/engine/configuration/applier/timeperiod.hh>
#include <com/centreon/engine/configuration/applier/servicegroup.hh>
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/serviceescalation.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include <com/centreon/engine/configuration/parser.hh>
#include <com/centreon/engine/hostescalation.hh>
#include <com/centreon/engine/macros/grab_host.hh>
#include <com/centreon/engine/macros/process.hh>
#include <com/centreon/engine/macros.hh>
#include "com/centreon/engine/timeperiod.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

class MacroService : public TestEngine {
 public:
  void SetUp() override {
    init_config_state();
        _tp = _creator.new_timeperiod();
    for (int i(0); i < 7; ++i)
      _creator.new_timerange(0, 0, 24, 0, i);
    _now = strtotimet("2016-11-24 08:00:00");
    set_time(_now);
  }

  void TearDown() override {
    _host.reset();
    _host2.reset();
    _host3.reset();
    _svc.reset();
    deinit_config_state();
  }
  
  protected:
  std::shared_ptr<engine::host> _host, _host2,_host3;
  std::shared_ptr<engine::service> _svc;
  timeperiod_creator _creator;
  time_t _now;
  timeperiod* _tp;
};

// Given host configuration without host_id
// Then the applier add_object throws an exception.
TEST_F(MacroService, TotalServicesOkZero) {
  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$TOTALSERVICESOK$", out, 0);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, ServiceMacro) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  // We fake the expand_object
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();

  nagios_macros *mac(get_global_macros());
  std::string out;
  host::hosts["test_host"]->set_current_state(host::state_up);
  host::hosts["test_host"]->set_has_been_checked(true);
  service::services[std::make_pair("test_host", "test_svc")]->set_plugin_output("foo bar!");
  process_macros_r(mac, "$SERVICEOUTPUT:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "foo bar!");
}

TEST_F(MacroService, ServiceDesc) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  // We fake the expand_object
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();

  std::string out;
  nagios_macros *mac(get_global_macros());
  
  process_macros_r(mac, "$SERVICEDESC:test_host:test_svc$", out, 0);
  ASSERT_EQ(out, "test_svc");
}

TEST_F(MacroService, ServiceState) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  // We fake the expand_object
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();

  std::string out;
  nagios_macros *mac(get_global_macros());
  
  process_macros_r(mac, "$SERVICESTATE:test_host:test_svc$", out, 0);
  ASSERT_EQ(out, "OK");
}

TEST_F(MacroService, ServiceStateID) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  // We fake the expand_object
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();

  std::string out;
  nagios_macros *mac(get_global_macros());
  
  process_macros_r(mac, "$SERVICESTATEID:test_host:test_svc$", out, 0);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, ServiceAttempt) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  // We fake the expand_object
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();

  std::string out;
  nagios_macros *mac(get_global_macros());
  
  process_macros_r(mac, "$SERVICEATTEMPT:test_host:test_svc$", out, 0);
  ASSERT_EQ(out, "1");
}

TEST_F(MacroService, ServiceisVolatile) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  // We fake the expand_object
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();

  std::string out;
  nagios_macros *mac(get_global_macros());
  
  process_macros_r(mac, "$SERVICEISVOLATILE:test_host:test_svc$", out, 0);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, LastServiceCheck) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  // We fake the expand_object
  svc.set_host_id(12);

  int now{500000000};
  set_time(now);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();

  std::string out;
  nagios_macros *mac(get_global_macros());
  host::hosts["test_host"]->set_current_state(host::state_up);
  host::hosts["test_host"]->set_has_been_checked(true);
  process_macros_r(mac, "$LASTSERVICECHECK:test_host:test_svc$", out, 0);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, LastServiceStateChange) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  // We fake the expand_object
  svc.set_host_id(12);

  int now{500000000};
  set_time(now);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();

  std::string out;
  nagios_macros *mac(get_global_macros());
  host::hosts["test_host"]->set_current_state(host::state_up);
  host::hosts["test_host"]->set_has_been_checked(true);
  process_macros_r(mac, "$LASTSERVICESTATECHANGE:test_host:test_svc$", out, 0);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, ServicePerfData) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  // We fake the expand_object
  svc.set_host_id(12);

  int now{500000000};
  set_time(now);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();

  std::string out;
  nagios_macros *mac(get_global_macros());
  host::hosts["test_host"]->set_current_state(host::state_up);
  host::hosts["test_host"]->set_has_been_checked(true);
  service::services[std::make_pair("test_host", "test_svc")]->set_perf_data("foo");
  process_macros_r(mac, "$SERVICEPERFDATA:test_host:test_svc$", out, 0);
  ASSERT_EQ(out, "foo");
}

TEST_F(MacroService, ServiceExecutionTime) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::applier::contact cnt_aply;
  configuration::service svc;
  configuration::host hst;
  configuration::contact cnt;
  ASSERT_TRUE(cnt.parse("contact_name", "user"));
  ASSERT_TRUE(cnt.parse("email", "contact@centreon.com"));
  ASSERT_TRUE(cnt.parse("pager", "0473729383"));
  ASSERT_TRUE(cnt.parse("host_notification_period", "24x7"));
  ASSERT_TRUE(cnt.parse("service_notification_period", "24x7"));
  cnt_aply.add_object(cnt);

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  //ASSERT_TRUE(hst.parse("contact_name", "testeeeeee"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  // We fake the expand_object
  svc.set_host_id(12);

  int now{500000000};
  set_time(now);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();

  std::string out;
  nagios_macros *mac(get_global_macros());
  host::hosts["test_host"]->set_current_state(host::state_up);
  host::hosts["test_host"]->set_has_been_checked(true);
  service::services[std::make_pair("test_host", "test_svc")]->set_execution_time(20.00);
  process_macros_r(mac, "$SERVICEEXECUTIONTIME:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "20.000");
}

TEST_F(MacroService, ServiceLatency) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::applier::contact cnt_aply;
  configuration::service svc;
  configuration::host hst;
  configuration::contact cnt;
  ASSERT_TRUE(cnt.parse("contact_name", "user"));
  ASSERT_TRUE(cnt.parse("email", "contact@centreon.com"));
  ASSERT_TRUE(cnt.parse("pager", "0473729383"));
  ASSERT_TRUE(cnt.parse("host_notification_period", "24x7"));
  ASSERT_TRUE(cnt.parse("service_notification_period", "24x7"));
  cnt_aply.add_object(cnt);

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  //ASSERT_TRUE(hst.parse("contact_name", "testeeeeee"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  // We fake the expand_object
  svc.set_host_id(12);

  int now{500000000};
  set_time(now);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();

  std::string out;
  nagios_macros *mac(get_global_macros());
  host::hosts["test_host"]->set_current_state(host::state_up);
  host::hosts["test_host"]->set_has_been_checked(true);
  service::services[std::make_pair("test_host", "test_svc")]->set_latency(20.00);
  process_macros_r(mac, "$SERVICELATENCY:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "20.000");
}

TEST_F(MacroService, ServiceDuration) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::applier::contact cnt_aply;
  configuration::service svc;
  configuration::host hst;
  configuration::contact cnt;
  ASSERT_TRUE(cnt.parse("contact_name", "user"));
  ASSERT_TRUE(cnt.parse("email", "contact@centreon.com"));
  ASSERT_TRUE(cnt.parse("pager", "0473729383"));
  ASSERT_TRUE(cnt.parse("host_notification_period", "24x7"));
  ASSERT_TRUE(cnt.parse("service_notification_period", "24x7"));
  cnt_aply.add_object(cnt);

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  //ASSERT_TRUE(hst.parse("contact_name", "testeeeeee"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  // We fake the expand_object
  svc.set_host_id(12);

  int now{500000000};
  set_time(now);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();

  std::string out;
  nagios_macros *mac(get_global_macros());
  host::hosts["test_host"]->set_current_state(host::state_up);
  host::hosts["test_host"]->set_has_been_checked(true);
  service::services[std::make_pair("test_host", "test_svc")]->set_latency(20.00);
  process_macros_r(mac, "$SERVICEDURATION:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "5787d 0h 53m 20s");
}

TEST_F(MacroService, ServiceDurationSec) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::applier::contact cnt_aply;
  configuration::service svc;
  configuration::host hst;
  configuration::contact cnt;
  ASSERT_TRUE(cnt.parse("contact_name", "user"));
  ASSERT_TRUE(cnt.parse("email", "contact@centreon.com"));
  ASSERT_TRUE(cnt.parse("pager", "0473729383"));
  ASSERT_TRUE(cnt.parse("host_notification_period", "24x7"));
  ASSERT_TRUE(cnt.parse("service_notification_period", "24x7"));
  cnt_aply.add_object(cnt);

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  // We fake the expand_object
  svc.set_host_id(12);

  int now{500000000};
  set_time(now);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();

  std::string out;
  nagios_macros *mac(get_global_macros());
  host::hosts["test_host"]->set_current_state(host::state_up);
  host::hosts["test_host"]->set_has_been_checked(true);
  service::services[std::make_pair("test_host", "test_svc")]->set_latency(20.00);
  process_macros_r(mac, "$SERVICEDURATIONSEC:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "500000000");
}

TEST_F(MacroService, ServiceDownTime) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$SERVICEDOWNTIME:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, ServiceStateType) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$SERVICESTATETYPE:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "HARD");
}

TEST_F(MacroService, ServicePercentChange) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$SERVICEPERCENTCHANGE:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0.00");
}

TEST_F(MacroService, ServiceGroupName) {
  configuration::applier::host aply_hst;
  configuration::applier::service aply_svc;
  configuration::applier::command aply_cmd;
  configuration::applier::servicegroup aply_grp;
  configuration::servicegroup grp("test_group");
  configuration::host hst;
  configuration::command cmd("cmd");
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  aply_hst.add_object(hst);
  configuration::service svc;
  ASSERT_TRUE(svc.parse("service_description", "test"));
  ASSERT_TRUE(svc.parse("hosts", "test_host"));
  ASSERT_TRUE(svc.parse("service_id", "18"));
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  aply_cmd.add_object(cmd);

  // We fake here the expand_object on configuration::service
  svc.set_host_id(12);

  aply_svc.add_object(svc);
  ASSERT_TRUE(svc.parse("servicegroups", "test_group"));
  grp.parse("members", "test_host,test");
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  ASSERT_NO_THROW(aply_grp.resolve_object(grp));

  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$SERVICEGROUPNAME:test_group$", out, 1);
  ASSERT_EQ(out, "test_group");
}

TEST_F(MacroService, ServiceGroupAlias) {
  configuration::applier::host aply_hst;
  configuration::applier::service aply_svc;
  configuration::applier::command aply_cmd;
  configuration::applier::servicegroup aply_grp;
  configuration::servicegroup grp("test_group");
  configuration::host hst;
  configuration::command cmd("cmd");
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  aply_hst.add_object(hst);
  configuration::service svc;
  ASSERT_TRUE(svc.parse("service_description", "test"));
  ASSERT_TRUE(svc.parse("hosts", "test_host"));
  ASSERT_TRUE(svc.parse("service_id", "18"));
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  aply_cmd.add_object(cmd);

  // We fake here the expand_object on configuration::service
  svc.set_host_id(12);

  aply_svc.add_object(svc);
  ASSERT_TRUE(svc.parse("servicegroups", "test_group"));
  grp.parse("members", "test_host,test");
  grp.parse("alias", "test_group_alias");
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  ASSERT_NO_THROW(aply_grp.resolve_object(grp));

  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$SERVICEGROUPALIAS:test_group$", out, 1);
  ASSERT_EQ(out, "test_group_alias");
}

TEST_F(MacroService, LastServiceOK) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_last_time_ok(20);
  process_macros_r(mac, "$LASTSERVICEOK:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "20");
}

TEST_F(MacroService, LastServiceWarning) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_last_time_warning(30);
  process_macros_r(mac, "$LASTSERVICEWARNING:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "30");
}

TEST_F(MacroService, LastServiceUnknown) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_last_time_unknown(40);
  process_macros_r(mac, "$LASTSERVICEUNKNOWN:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "40");
}

TEST_F(MacroService, LastServiceCritical) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_last_time_critical(50);
  process_macros_r(mac, "$LASTSERVICECRITICAL:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "50");
}

TEST_F(MacroService, ServiceCheckCommand) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_last_time_critical(50);
  process_macros_r(mac, "$SERVICECHECKCOMMAND:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "cmd");
}

TEST_F(MacroService, ServicePerfDataFile) {
  configuration::parser parser;
  configuration::state st;

  std::remove("/tmp/test-config.cfg");

  std::ofstream ofs("/tmp/test-config.cfg");
  ofs << "service_perfdata_file=/var/log/centreon-engine/service-perfdata.dat" << std::endl;
  ofs << "log_file=\"\"" << std::endl;
  ofs.close();

  parser.parse("/tmp/test-config.cfg", st);
  configuration::applier::state::instance().apply(st);
  init_macros();

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$SERVICEPERFDATAFILE:test_host$", out, 1);
  ASSERT_EQ(out, "/var/log/centreon-engine/service-perfdata.dat");
}

TEST_F(MacroService, ServiceDisplayName) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_last_time_critical(50);
  process_macros_r(mac, "$SERVICEDISPLAYNAME:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "test_svc");
}

TEST_F(MacroService, ServiceNotesUrl) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("notes_url", "http://192.168.0.172/centreon/main.php"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$SERVICENOTESURL:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "http://192.168.0.172/centreon/main.php");
}

TEST_F(MacroService, ServiceNotes) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("notes", "test_notes"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$SERVICENOTES:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "test_notes");
}

TEST_F(MacroService, ServiceActionUrl) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$SERVICEACTIONURL:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "test_action_url");
}

TEST_F(MacroService, TotalServicesWarning) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$TOTALSERVICESWARNING:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, TotalServicesCritical) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$TOTALSERVICESCRITICAL:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, TotalServicesUnknown) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$TOTALSERVICESUNKNOWN:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, TotalServicesWarningUnhandled) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$TOTALSERVICESWARNINGUNHANDLED:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, TotalServicesCriticalUnhandled) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$TOTALSERVICESCRITICALUNHANDLED:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, TotalServicesUnknownUnhandled) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$TOTALSERVICESUNKNOWNUNHANDLED:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, TotalServiceProblems) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$TOTALSERVICEPROBLEMS:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, TotalServiceProblemsUnhandled) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$TOTALSERVICEPROBLEMSUNHANDLED:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, ServiceCheckType) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$SERVICECHECKTYPE:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "ACTIVE");
}

TEST_F(MacroService, LongServiceOutput) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_long_plugin_output("test_long_output");
  process_macros_r(mac, "$LONGSERVICEOUTPUT:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "test_long_output");
}

TEST_F(MacroService, ServiceNotificationID) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_long_plugin_output("test_long_output");
  process_macros_r(mac, "$SERVICENOTIFICATIONID:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, ServiceEventID) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_long_plugin_output("test_long_output");
  process_macros_r(mac, "$SERVICEEVENTID:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, LastServiceEventID) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_long_plugin_output("test_long_output");
  process_macros_r(mac, "$LASTSERVICEEVENTID:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, ServiceGroupNames) {
  configuration::applier::host aply_hst;
  configuration::applier::service aply_svc;
  configuration::applier::command aply_cmd;
  configuration::applier::servicegroup aply_grp;
  configuration::servicegroup grp("test_group");
  configuration::host hst;
  configuration::command cmd("cmd");
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  aply_hst.add_object(hst);
  configuration::service svc;
  ASSERT_TRUE(svc.parse("service_description", "test"));
  ASSERT_TRUE(svc.parse("hosts", "test_host"));
  ASSERT_TRUE(svc.parse("service_id", "18"));
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  aply_cmd.add_object(cmd);

  // We fake here the expand_object on configuration::service
  svc.set_host_id(12);

  aply_svc.add_object(svc);
  ASSERT_TRUE(svc.parse("servicegroups", "test_group"));
  grp.parse("members", "test_host,test");
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  ASSERT_NO_THROW(aply_grp.resolve_object(grp));

  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test")]->set_long_plugin_output("test_long_output");
  process_macros_r(mac, "$SERVICEGROUPNAMES:test_host:test$", out, 1);
  ASSERT_EQ(out, "test_group");
}

TEST_F(MacroService, MaxServiceAttempts) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_long_plugin_output("test_long_output");
  process_macros_r(mac, "$MAXSERVICEATTEMPTS:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "3");
}

TEST_F(MacroService, ServiceGroupNotes) {
  configuration::applier::host aply_hst;
  configuration::applier::service aply_svc;
  configuration::applier::command aply_cmd;
  configuration::applier::servicegroup aply_grp;
  configuration::servicegroup grp("test_group");
  configuration::host hst;
  configuration::command cmd("cmd");
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  aply_hst.add_object(hst);
  configuration::service svc;
  ASSERT_TRUE(svc.parse("service_description", "test"));
  ASSERT_TRUE(svc.parse("hosts", "test_host"));
  ASSERT_TRUE(svc.parse("service_id", "18"));
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  aply_cmd.add_object(cmd);

  // We fake here the expand_object on configuration::service
  svc.set_host_id(12);

  aply_svc.add_object(svc);
  ASSERT_TRUE(svc.parse("servicegroups", "test_group"));
  grp.parse("members", "test_host,test");
  ASSERT_TRUE(grp.parse("notes", "test_notes"));
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  ASSERT_NO_THROW(aply_grp.resolve_object(grp));
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test")]->set_long_plugin_output("test_long_output");
  process_macros_r(mac, "$SERVICEGROUPNOTES:test_group$", out, 1);
  ASSERT_EQ(out, "test_notes");
}

TEST_F(MacroService, ServiceGroupNotesUrl) {
  configuration::applier::host aply_hst;
  configuration::applier::service aply_svc;
  configuration::applier::command aply_cmd;
  configuration::applier::servicegroup aply_grp;
  configuration::servicegroup grp("test_group");
  configuration::host hst;
  configuration::command cmd("cmd");
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  aply_hst.add_object(hst);
  configuration::service svc;
  ASSERT_TRUE(svc.parse("service_description", "test"));
  ASSERT_TRUE(svc.parse("hosts", "test_host"));
  ASSERT_TRUE(svc.parse("service_id", "18"));
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  aply_cmd.add_object(cmd);

  // We fake here the expand_object on configuration::service
  svc.set_host_id(12);

  aply_svc.add_object(svc);
  ASSERT_TRUE(svc.parse("servicegroups", "test_group"));
  grp.parse("members", "test_host,test");
  ASSERT_TRUE(grp.parse("notes_url", "test_notes_url"));
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  ASSERT_NO_THROW(aply_grp.resolve_object(grp));
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$SERVICEGROUPNOTESURL:test_group$", out, 1);
  ASSERT_EQ(out, "test_notes_url");
}

TEST_F(MacroService, ServiceGroupActionUrl) {
  configuration::applier::host aply_hst;
  configuration::applier::service aply_svc;
  configuration::applier::command aply_cmd;
  configuration::applier::servicegroup aply_grp;
  configuration::servicegroup grp("test_group");
  configuration::host hst;
  configuration::command cmd("cmd");
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  aply_hst.add_object(hst);
  configuration::service svc;
  ASSERT_TRUE(svc.parse("service_description", "test"));
  ASSERT_TRUE(svc.parse("hosts", "test_host"));
  ASSERT_TRUE(svc.parse("service_id", "18"));
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  aply_cmd.add_object(cmd);

  // We fake here the expand_object on configuration::service
  svc.set_host_id(12);

  aply_svc.add_object(svc);
  ASSERT_TRUE(svc.parse("servicegroups", "test_group"));
  grp.parse("members", "test_host,test");
  ASSERT_TRUE(grp.parse("action_url", "test_notes_url"));
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  ASSERT_NO_THROW(aply_grp.resolve_object(grp));
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$SERVICEGROUPACTIONURL:test_group$", out, 1);
  ASSERT_EQ(out, "test_notes_url");
}

TEST_F(MacroService, ServiceGroupMembers) {
  configuration::applier::host aply_hst;
  configuration::applier::service aply_svc;
  configuration::applier::command aply_cmd;
  configuration::applier::servicegroup aply_grp;
  configuration::servicegroup grp("test_group");
  configuration::host hst;
  configuration::command cmd("cmd");
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  aply_hst.add_object(hst);
  configuration::service svc;
  ASSERT_TRUE(svc.parse("service_description", "test"));
  ASSERT_TRUE(svc.parse("hosts", "test_host"));
  ASSERT_TRUE(svc.parse("service_id", "18"));
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  aply_cmd.add_object(cmd);

  // We fake here the expand_object on configuration::service
  svc.set_host_id(12);

  aply_svc.add_object(svc);
  ASSERT_TRUE(svc.parse("servicegroups", "test_group"));
  grp.parse("members", "test_host,test");
  ASSERT_TRUE(grp.parse("action_url", "test_notes_url"));
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  ASSERT_NO_THROW(aply_grp.resolve_object(grp));
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$SERVICEGROUPMEMBERS:test_group$", out, 1);
  ASSERT_EQ(out, "test_host,test");
}

TEST_F(MacroService, ServiceID) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("service_id", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  process_macros_r(mac, "$SERVICEID:test_host:test_svc$", out, 0);
  ASSERT_EQ(out, "13");
}

TEST_F(MacroService, ServiceTimeZone) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  svc.parse("timezone", "test_time");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_long_plugin_output("test_long_output");
  process_macros_r(mac, "$SERVICETIMEZONE:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "test_time");
}

TEST_F(MacroService, LastServiceState) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_long_plugin_output("test_long_output");
  process_macros_r(mac, "$LASTSERVICESTATE:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "OK");
}

TEST_F(MacroService, LastServiceStateId) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_long_plugin_output("test_long_output");
  process_macros_r(mac, "$LASTSERVICESTATEID:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0");
}

TEST_F(MacroService, ServiceProblemID) {
  init_macros();

  configuration::applier::contact ct_aply;
  configuration::contact ctct{new_configuration_contact("admin", true)};
  ct_aply.add_object(ctct);
  configuration::contact ctct1{new_configuration_contact("admin1", false, "c,r")};
  ct_aply.add_object(ctct1);
  ct_aply.expand_objects(*config);
  ct_aply.resolve_object(ctct);
  ct_aply.resolve_object(ctct1);

  configuration::host hst{new_configuration_host("test_host", "admin")};
  configuration::applier::host hst_aply;
  hst_aply.add_object(hst);

  configuration::service svc{
      new_configuration_service("test_host", "test_svc", "admin,admin1")};
  configuration::applier::service svc_aply;
  svc_aply.add_object(svc);

  hst_aply.resolve_object(hst);
  svc_aply.resolve_object(svc);

  host_map const& hm{engine::host::hosts};
  _host3 = hm.begin()->second;
  _host3->set_current_state(engine::host::state_up);
  _host3->set_state_type(checkable::hard);
  _host3->set_problem_has_been_acknowledged(false);
  _host3->set_notify_on(static_cast<uint32_t>(-1));

  std::string out;
  service_map const& sm{engine::service::services};
  _svc = sm.begin()->second;
  _svc->set_current_state(engine::service::state_ok);
  _svc->set_state_type(checkable::hard);
  _svc->set_problem_has_been_acknowledged(true);
  _svc->set_notify_on(static_cast<uint32_t>(-1));

  set_time(50000);
  _svc->set_current_state(engine::service::state_ok);
  _svc->set_last_hard_state(engine::service::state_ok);
  _svc->set_last_hard_state_change(50000);
  _svc->set_state_type(checkable::hard);
  _svc->set_first_notification_delay(3);

  for (int i = 1; i < 4; i++) {
    // When i == 0, the state_down is soft => no notification
    // When i == 1, the state_down is soft => no notification
    // When i == 2, the state_down is hard down => notification
    set_time(50000 + i * 60);
    _svc->set_last_state(_svc->get_current_state());
    if (notifier::hard == _svc->get_state_type())
      _svc->set_last_hard_state(_svc->get_current_state());
    std::ostringstream oss;
    std::time_t now{std::time(nullptr)};
    oss << '[' << now << ']'
        << " PROCESS_SERVICE_CHECK_RESULT;test_host;test_svc;2;service "
           "critical";
    std::string cmd{oss.str()};
    process_external_command(cmd.c_str());
    checks::checker::instance().reap();

  }
    nagios_macros *mac(get_global_macros());
    process_macros_r(mac, "$SERVICEPROBLEMID:test_host:test_svc$", out, 0);
  ASSERT_EQ(out, "4");
}

TEST_F(MacroService, LastServiceProblemID) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  ASSERT_TRUE(svc.parse("description", "test_svc"));
  ASSERT_TRUE(svc.parse("host_name", "test_host"));
  ASSERT_TRUE(svc.parse("_HOST_ID", "12"));
  ASSERT_TRUE(svc.parse("_SERVICE_ID", "13"));
  ASSERT_TRUE(svc.parse("action_url", "test_action_url"));
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));
  ASSERT_EQ(1u, service::services.size());
  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros *mac(get_global_macros());
  service::services[std::make_pair("test_host", "test_svc")]->set_long_plugin_output("test_long_output");
  process_macros_r(mac, "$LASTSERVICEPROBLEMID:test_host:test_svc$", out, 1);
  ASSERT_EQ(out, "0");
}
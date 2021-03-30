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
#include <com/centreon/clib.hh>
#include <com/centreon/engine/broker/loader.hh>
#include <com/centreon/engine/checks/checker.hh>
#include <com/centreon/engine/configuration/applier/command.hh>
#include <com/centreon/engine/configuration/applier/contact.hh>
#include <com/centreon/engine/configuration/applier/host.hh>
#include <com/centreon/engine/configuration/applier/hostgroup.hh>
#include <com/centreon/engine/configuration/applier/service.hh>
#include <com/centreon/engine/configuration/applier/servicegroup.hh>
#include <com/centreon/engine/configuration/applier/state.hh>
#include <com/centreon/engine/configuration/applier/timeperiod.hh>
#include <com/centreon/engine/configuration/parser.hh>
#include <com/centreon/engine/hostescalation.hh>
#include <com/centreon/engine/macros.hh>
#include <com/centreon/engine/macros/grab_host.hh>
#include <com/centreon/engine/macros/process.hh>
#include <com/centreon/engine/timezone_manager.hh>
#include <fstream>
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

extern configuration::state* config;

class Macro : public ::testing::Test {
 public:
  void SetUp() override {
    clib::load();
    com::centreon::logging::engine::load();
    if (config == nullptr)
      config = new configuration::state;
    timezone_manager::load();
    configuration::applier::state::load();  // Needed to create a contact
    broker::loader::load();
  }

  void TearDown() override {
    broker::loader::unload();
    configuration::applier::state::unload();
    checks::checker::unload();
    timezone_manager::unload();
    delete config;
    config = nullptr;
    com::centreon::logging::engine::unload();
    clib::unload();
  }
};

// Given host configuration without host_id
// Then the applier add_object throws an exception.
TEST_F(Macro, TotalServicesOkZero) {
  std::string out;
  nagios_macros mac;
  process_macros_r(&mac, "$TOTALSERVICESOK$", out, 0);
  ASSERT_EQ(out, "0");
}

// Given host configuration without host_id
// Then the applier add_object throws an exception.
TEST_F(Macro, TotalHostOk) {
  configuration::applier::host hst_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  init_macros();

  nagios_macros mac;
  std::string out;
  host::hosts["test_host"]->set_current_state(host::state_up);
  host::hosts["test_host"]->set_has_been_checked(true);
  process_macros_r(&mac, "$TOTALHOSTSUP$", out, 1);
  ASSERT_EQ(out, "1");
}

// Given host configuration without host_id
// Then the applier add_object throws an exception.
TEST_F(Macro, TotalHostServicesCritical) {
  configuration::applier::host hst_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  init_macros();

  nagios_macros mac;
  std::string out;
  host::hosts["test_host"]->set_current_state(host::state_up);
  host::hosts["test_host"]->set_has_been_checked(true);
  process_macros_r(&mac, "$TOTALHOSTSERVICESCRITICAL:test_host$", out, 1);
  ASSERT_EQ(out, "0");
}

// Given host configuration without host_id
// Then the applier add_object throws an exception.
TEST_F(Macro, TotalHostServicesCriticalError) {
  configuration::applier::host hst_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());
  init_macros();

  nagios_macros mac;
  std::string out;
  host::hosts["test_host"]->set_current_state(host::state_up);
  host::hosts["test_host"]->set_has_been_checked(true);
  /* The call of this variable needs a host name */
  process_macros_r(&mac, "$TOTALHOSTSERVICESCRITICAL$", out, 1);
  ASSERT_EQ(out, "");
}

TEST_F(Macro, TimeT) {
  configuration::applier::host hst_aply;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_EQ(1u, host::hosts.size());

  int now{500000000};
  set_time(now);
  init_macros();

  std::string out;
  nagios_macros* mac(get_global_macros());
  process_macros_r(mac, "$TIMET:test_host$", out, 0);
  ASSERT_EQ(out, "500000000");
}

TEST_F(Macro, HostGroupName) {
  configuration::applier::hostgroup hg_aply;
  configuration::applier::host hst_aply;
  configuration::hostgroup hg;
  configuration::host hst_a;
  configuration::host hst_c;

  ASSERT_TRUE(hst_a.parse("host_name", "a"));
  ASSERT_TRUE(hst_a.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst_a.parse("_HOST_ID", "1"));

  ASSERT_TRUE(hst_c.parse("host_name", "c"));
  ASSERT_TRUE(hst_c.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst_c.parse("_HOST_ID", "2"));

  hst_aply.add_object(hst_a);
  hst_aply.add_object(hst_c);

  ASSERT_TRUE(hg.parse("hostgroup_name", "temphg"));
  ASSERT_TRUE(hg.parse("members", "a,c"));
  ASSERT_NO_THROW(hg_aply.add_object(hg));

  ASSERT_NO_THROW(hst_aply.expand_objects(*config));
  ASSERT_NO_THROW(hst_aply.expand_objects(*config));
  ASSERT_NO_THROW(hg_aply.expand_objects(*config));

  ASSERT_NO_THROW(hst_aply.resolve_object(hst_a));
  ASSERT_NO_THROW(hst_aply.resolve_object(hst_c));
  ASSERT_NO_THROW(hg_aply.resolve_object(hg));

  int now{500000000};
  set_time(now);
  init_macros();

  std::string out;
  nagios_macros* mac(get_global_macros());
  process_macros_r(mac, "$HOSTGROUPNAME:a$", out, 1);
  ASSERT_EQ(out, "temphg");
}

TEST_F(Macro, ServiceGroupName) {
  configuration::applier::host aply_hst;
  configuration::applier::service aply_svc;
  configuration::applier::servicegroup aply_grp;
  configuration::servicegroup grp("test_group");
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  aply_hst.add_object(hst);
  configuration::service svc;
  ASSERT_TRUE(svc.parse("service_description", "test"));
  ASSERT_TRUE(svc.parse("hosts", "test_host"));
  ASSERT_TRUE(svc.parse("service_id", "18"));
  ASSERT_TRUE(svc.parse("check_command", "cmd"));
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
  nagios_macros* mac(get_global_macros());
  process_macros_r(mac, "$SERVICEGROUPNAME:test_host:test$", out, 1);
  ASSERT_EQ(out, "test_group");
}

TEST_F(Macro, ContactGroupName) {
  configuration::applier::contact ct_aply;
  configuration::contact ctct;
  configuration::applier::contactgroup cg_aply;
  configuration::contactgroup cg;

  ctct.parse("contact_name", "test_contact");
  ct_aply.add_object(ctct);

  cg.parse("contactgroup_name", "test_cg");
  cg.parse("alias", "test_cg");
  cg.parse("members", "test_contact");
  cg_aply.add_object(cg);
  cg_aply.expand_objects(*config);
  cg_aply.resolve_object(cg);

  init_macros();
  int now{500000000};
  set_time(now);

  std::string out;
  nagios_macros* mac(get_global_macros());
  process_macros_r(mac, "$CONTACTGROUPNAME:test_contact$", out, 1);
  ASSERT_EQ(out, "test_cg");
}

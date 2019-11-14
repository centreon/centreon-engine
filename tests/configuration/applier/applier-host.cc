/*
 * Copyright 2017 - 2019 Centreon (https://www.centreon.com/)
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
#include <cstring>
#include <iostream>
#include <memory>
#include "../../timeperiod/utils.hh"
#include "com/centreon/clib.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/macros/grab_host.hh"
#include "com/centreon/engine/macros/grab_value.hh"
#include "com/centreon/engine/timezone_manager.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ApplierHost : public ::testing::Test {
 public:
  void SetUp() override {
    clib::load();
    com::centreon::logging::engine::load();
    if (config == NULL)
      config = new configuration::state;
    timezone_manager::load();
    configuration::applier::state::load();  // Needed to create a contact
    checks::checker::load();
  }

  void TearDown() override {
    configuration::applier::state::unload();
    checks::checker::unload();
    delete config;
    config = nullptr;
    timezone_manager::unload();
    com::centreon::logging::engine::unload();
    clib::unload();
  }
};

// Given host configuration without host_id
// Then the applier add_object throws an exception.
TEST_F(ApplierHost, NewHostWithoutHostId) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_THROW(hst_aply.add_object(hst), std::exception);
}

// Given a host configuration
// When we change the host name in the configuration
// Then the applier modify_object changes the host name without changing
// the host id.
TEST_F(ApplierHost, HostRenamed) {
  configuration::applier::host hst_aply;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  hst_aply.add_object(hst);
  host_map const& hm(engine::host::hosts);
  ASSERT_EQ(hm.size(), 1u);
  std::shared_ptr<com::centreon::engine::host> h1(hm.begin()->second);
  ASSERT_TRUE(h1->get_name() == "test_host");

  ASSERT_TRUE(hst.parse("host_name", "test_host1"));
  hst_aply.modify_object(hst);
  ASSERT_EQ(hm.size(), 1u);
  h1 = hm.begin()->second;
  ASSERT_TRUE(h1->get_name() == "test_host1");
  ASSERT_EQ(get_host_id(h1->get_name()), 12u);
}

TEST_F(ApplierHost, HostRemoved) {
  configuration::applier::host hst_aply;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  hst_aply.add_object(hst);
  host_map const& hm(engine::host::hosts);
  ASSERT_EQ(hm.size(), 1u);
  std::shared_ptr<com::centreon::engine::host> h1(hm.begin()->second);
  ASSERT_TRUE(h1->get_name() == "test_host");

  ASSERT_TRUE(hst.parse("host_name", "test_host1"));
  hst_aply.remove_object(hst);

  ASSERT_EQ(hm.size(), 0u);
  ASSERT_TRUE(hst.parse("host_name", "test_host1"));
  hst_aply.add_object(hst);
  h1 = hm.begin()->second;
  ASSERT_EQ(hm.size(), 1u);
  ASSERT_TRUE(h1->get_name() == "test_host1");
  ASSERT_EQ(get_host_id(h1->get_name()), 12u);
}

TEST_F(ApplierHost, HostParentChildUnreachable) {
  configuration::applier::host hst_aply;
  configuration::applier::command cmd_aply;
  configuration::host hst_child;
  configuration::host hst_parent;

  configuration::command cmd("base_centreon_ping");
  cmd.parse("command_line",
            "$USER1$/check_icmp -H $HOSTADDRESS$ -n $_HOSTPACKETNUMBER$ -w "
            "$_HOSTWARNING$ -c $_HOSTCRITICAL$");
  cmd_aply.add_object(cmd);

  ASSERT_TRUE(hst_child.parse("host_name", "child_host"));
  ASSERT_TRUE(hst_child.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst_child.parse("parents", "parent_host"));
  ASSERT_TRUE(hst_child.parse("_HOST_ID", "1"));
  ASSERT_TRUE(hst_child.parse("_PACKETNUMBER", "42"));
  ASSERT_TRUE(hst_child.parse("_WARNING", "200,20%"));
  ASSERT_TRUE(hst_child.parse("_CRITICAL", "400,50%"));
  ASSERT_TRUE(hst_child.parse("check_command", "base_centreon_ping"));
  hst_aply.add_object(hst_child);

  ASSERT_TRUE(hst_parent.parse("host_name", "parent_host"));
  ASSERT_TRUE(hst_parent.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst_parent.parse("_HOST_ID", "2"));
  ASSERT_TRUE(hst_parent.parse("_PACKETNUMBER", "42"));
  ASSERT_TRUE(hst_parent.parse("_WARNING", "200,20%"));
  ASSERT_TRUE(hst_parent.parse("_CRITICAL", "400,50%"));
  ASSERT_TRUE(hst_parent.parse("check_command", "base_centreon_ping"));
  hst_aply.add_object(hst_parent);

  ASSERT_EQ(engine::host::hosts.size(), 2u);

  hst_aply.expand_objects(*config);
  hst_aply.resolve_object(hst_child);
  hst_aply.resolve_object(hst_parent);

  host_map::iterator child = engine::host::hosts.find("child_host");
  host_map::iterator parent = engine::host::hosts.find("parent_host");

  ASSERT_EQ(parent->second->child_hosts.size(), 1u);
  ASSERT_EQ(child->second->parent_hosts.size(), 1u);

  engine::host::host_state result;
  parent->second->run_sync_check_3x(&result, 0, 0, 0);
  ASSERT_EQ(parent->second->get_current_state(), engine::host::state_down);
  child->second->run_sync_check_3x(&result, 0, 0, 0);
  ASSERT_EQ(child->second->get_current_state(),
            engine::host::state_unreachable);
}

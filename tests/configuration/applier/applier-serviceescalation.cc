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
#include <com/centreon/engine/checks/checker.hh>
#include <com/centreon/engine/configuration/applier/command.hh>
#include <com/centreon/engine/configuration/applier/host.hh>
#include <com/centreon/engine/configuration/applier/service.hh>
#include <com/centreon/engine/configuration/applier/serviceescalation.hh>
#include <com/centreon/engine/configuration/applier/state.hh>
#include <com/centreon/engine/configuration/state.hh>
#include <com/centreon/engine/events/loop.hh>
#include <com/centreon/engine/host.hh>
#include <com/centreon/engine/service.hh>
#include <com/centreon/engine/serviceescalation.hh>
#include <com/centreon/engine/timezone_manager.hh>
#include <com/centreon/logging/engine.hh>
#include <com/centreon/logging/logger.hh>

using namespace com::centreon;
using namespace com::centreon::engine;

extern configuration::state* config;

class ApplierServiceEscalation : public ::testing::Test {
 public:
  void SetUp() override {
    clib::load();
    com::centreon::logging::engine::load();
    if (config == NULL)
      config = new configuration::state;
    timezone_manager::load();
    configuration::applier::state::load();  // Needed to create a contact
    checks::checker::load();
    events::loop::load();
  }

  void TearDown() override {
    events::loop::unload();
    configuration::applier::state::unload();
    checks::checker::unload();
    delete config;
    config = nullptr;
    timezone_manager::unload();
    com::centreon::logging::engine::unload();
    clib::unload();
  }
};

TEST_F(ApplierServiceEscalation, AddEscalation) {
  configuration::applier::host hst_aply;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  hst_aply.add_object(hst);
  ASSERT_EQ(host::hosts.size(), 1u);

  configuration::applier::command cmd_aply;
  configuration::command cmd;
  cmd.parse("command_name", "cmd");
  cmd.parse("command_line", "echo 1");
  cmd_aply.add_object(cmd);

  configuration::applier::service svc_aply;
  configuration::service svc;
  ASSERT_TRUE(svc.parse("host", "test_host"));
  ASSERT_TRUE(svc.parse("service_description", "test_svc"));
  ASSERT_TRUE(svc.parse("service_id", "12"));
  ASSERT_TRUE(svc.parse("check_command", "cmd"));
  svc.set_host_id(12);
  svc_aply.add_object(svc);
  ASSERT_EQ(service::services.size(), 1u);

  configuration::applier::serviceescalation se_apply;
  configuration::serviceescalation se;
  ASSERT_TRUE(se.parse("host", "test_host"));
  ASSERT_TRUE(se.parse("service_description", "test_svc"));
  ASSERT_TRUE(se.parse("first_notification", "4"));
  se_apply.add_object(se);
  ASSERT_EQ(serviceescalation::serviceescalations.size(), 1u);
  ASSERT_TRUE(se.parse("first_notification", "8"));
  se_apply.add_object(se);
  ASSERT_EQ(serviceescalation::serviceescalations.size(), 2u);
}

TEST_F(ApplierServiceEscalation, RemoveEscalation) {
  configuration::applier::host hst_aply;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  hst_aply.add_object(hst);
  ASSERT_EQ(host::hosts.size(), 1u);

  configuration::applier::command cmd_aply;
  configuration::command cmd;
  cmd.parse("command_name", "cmd");
  cmd.parse("command_line", "echo 1");
  cmd_aply.add_object(cmd);

  configuration::applier::service svc_aply;
  configuration::service svc;
  ASSERT_TRUE(svc.parse("host", "test_host"));
  ASSERT_TRUE(svc.parse("service_description", "test_svc"));
  ASSERT_TRUE(svc.parse("service_id", "12"));
  ASSERT_TRUE(svc.parse("check_command", "cmd"));
  svc.set_host_id(12);
  svc_aply.add_object(svc);
  ASSERT_EQ(service::services.size(), 1u);

  configuration::applier::serviceescalation se_apply;
  configuration::serviceescalation se1,se2;
  ASSERT_TRUE(se1.parse("host_name", "test_host"));
  ASSERT_TRUE(se1.parse("service_description", "test_svc"));
  ASSERT_TRUE(se1.parse("first_notification", "4"));
  se_apply.add_object(se1);
  ASSERT_TRUE(se2.parse("host_name", "test_host"));
  ASSERT_TRUE(se2.parse("service_description", "test_svc"));
  ASSERT_TRUE(se2.parse("first_notification", "8"));
  ASSERT_TRUE(se2.parse("first_notification", "8"));
  se_apply.add_object(se2);
  ASSERT_EQ(serviceescalation::serviceescalations.size(), 2u);

  se_apply.remove_object(se1);
  ASSERT_EQ(serviceescalation::serviceescalations.size(), 1u);
  se_apply.remove_object(se2);
  ASSERT_EQ(serviceescalation::serviceescalations.size(), 0u);
}

TEST_F(ApplierServiceEscalation, RemoveEscalationFromRemovedService) {
  configuration::applier::host hst_aply;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  hst_aply.add_object(hst);
  ASSERT_EQ(host::hosts.size(), 1u);

  configuration::applier::command cmd_aply;
  configuration::command cmd;
  cmd.parse("command_name", "cmd");
  cmd.parse("command_line", "echo 1");
  cmd_aply.add_object(cmd);

  configuration::applier::service svc_aply;
  configuration::service svc;
  ASSERT_TRUE(svc.parse("host", "test_host"));
  ASSERT_TRUE(svc.parse("service_description", "test_svc"));
  ASSERT_TRUE(svc.parse("service_id", "12"));
  ASSERT_TRUE(svc.parse("check_command", "cmd"));
  svc.set_host_id(12);
  svc_aply.add_object(svc);
  ASSERT_EQ(service::services.size(), 1u);

  configuration::applier::serviceescalation se_apply;
  configuration::serviceescalation se;
  ASSERT_TRUE(se.parse("host_name", "test_host"));
  ASSERT_TRUE(se.parse("service_description", "test_svc"));
  ASSERT_TRUE(se.parse("first_notification", "4"));
  se_apply.add_object(se);
  ASSERT_EQ(serviceescalation::serviceescalations.size(), 1u);
  ASSERT_TRUE(se.parse("first_notification", "8"));
  se_apply.add_object(se);
  ASSERT_EQ(serviceescalation::serviceescalations.size(), 2u);

  hst_aply.remove_object(hst);
  ASSERT_EQ(host::hosts.size(), 0u);
  svc_aply.remove_object(svc);
  ASSERT_EQ(service::services.size(), 0u);

  se_apply.remove_object(se);
  ASSERT_EQ(serviceescalation::serviceescalations.size(), 1u);
  ASSERT_TRUE(se.parse("first_notification", "4"));
  se_apply.remove_object(se);
  ASSERT_EQ(serviceescalation::serviceescalations.size(), 0u);
}

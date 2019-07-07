/*
 * Copyright 2005 - 2019 Centreon (https://www.centreon.com/)
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


#include <iostream>
#include <gtest/gtest.h>
#include "../timeperiod/utils.hh"
#include "com/centreon/clib.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/timezone_manager.hh"
#include <com/centreon/engine/configuration/applier/macros.hh>

using namespace com::centreon;
using namespace com::centreon::engine;

extern configuration::state* config;

class ServiceExternalCommand : public ::testing::Test {
 public:
  void SetUp() override {
    clib::load();
    com::centreon::logging::engine::load();
    if (config == nullptr)
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

TEST_F(ServiceExternalCommand, AddServiceDowntime) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::applier::command cmd_aply;
  configuration::service svc;
  configuration::host hst;
  configuration::command cmd("cmd");

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("host_id", "1"));

  ASSERT_TRUE(svc.parse("host", "test_host"));
  ASSERT_TRUE(svc.parse("service_description", "test_description"));
  ASSERT_TRUE(svc.parse("service_id", "3"));

  cmd.parse("command_line", "/usr/bin/echo 1");
  cmd_aply.add_object(cmd);

  hst.parse("check_command", "cmd");
  svc.parse("check_command", "cmd");

  hst_aply.add_object(hst);
  svc_aply.add_object(svc);

  hst_aply.expand_objects(*config);
  svc_aply.expand_objects(*config);

  hst_aply.resolve_object(hst);
  svc_aply.resolve_object(svc);

  set_time(20000);
  time_t now = time(nullptr);

  std::string str{"test_host;test_description;1;|"};

  testing::internal::CaptureStdout();
  cmd_process_service_check_result(
    CMD_PROCESS_SERVICE_CHECK_RESULT, now, const_cast<char *>(str.c_str()));
  checks::checker::instance().reap();

  std::string const& out{testing::internal::GetCapturedStdout()};

  ASSERT_NE(out.find("PASSIVE SERVICE CHECK"), std::string::npos);
}

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
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/timezone_manager.hh"
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"

using namespace com::centreon::engine;

extern configuration::state* config;

class HostExternalCommand : public ::testing::Test {
 public:
  void SetUp() override {
    if (config == nullptr)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contact
    timezone_manager::load();
    checks::checker::load();
  }

  void TearDown() override {
    checks::checker::unload();
    timezone_manager::unload();
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }
};

TEST_F(HostExternalCommand, AddUnkownHostDowntime) {
}

TEST_F(HostExternalCommand, AddHostDowntime) {
  configuration::applier::host hst_aply;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_srv"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "1"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));

  set_time(20000);
  time_t now = time(nullptr);

  std::string cmd{"test_srv;1;|"};

  testing::internal::CaptureStdout();
  cmd_process_host_check_result(
    CMD_PROCESS_HOST_CHECK_RESULT, now, const_cast<char *>(cmd.c_str()));
  checks::checker::instance().reap();

  std::string const& out{testing::internal::GetCapturedStdout()};

  ASSERT_NE(out.find("PASSIVE HOST CHECK"), std::string::npos);
  ASSERT_NE(out.find("HOST ALERT"), std::string::npos);
}
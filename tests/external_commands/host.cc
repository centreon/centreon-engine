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

#include "com/centreon/engine/configuration/applier/host.hh"
#include <gtest/gtest.h>
#include <com/centreon/engine/configuration/applier/macros.hh>
#include <iostream>
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "com/centreon/engine/timezone_manager.hh"

using namespace com::centreon;
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
    timezone_manager::unload();
    configuration::applier::state::unload();
    checks::checker::unload();
    delete config;
    config = nullptr;
  }
};

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
  cmd_process_host_check_result(CMD_PROCESS_HOST_CHECK_RESULT, now,
                                const_cast<char*>(cmd.c_str()));
  checks::checker::instance().reap();

  std::string const& out{testing::internal::GetCapturedStdout()};

  ASSERT_NE(out.find("PASSIVE HOST CHECK"), std::string::npos);
  ASSERT_NE(out.find("HOST ALERT"), std::string::npos);
}

TEST_F(HostExternalCommand, AddHostComment) {
  configuration::applier::host hst_aply;
  configuration::host hst;
  configuration::host hst2;

  ASSERT_TRUE(hst.parse("host_name", "test_srv"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "1"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));

  ASSERT_TRUE(hst2.parse("host_name", "test_srv2"));
  ASSERT_TRUE(hst2.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst2.parse("_HOST_ID", "2"));
  ASSERT_NO_THROW(hst_aply.add_object(hst2));

  set_time(20000);
  time_t now = time(nullptr);

  std::string cmd_com1{"test_srv;1;user;this is a first comment"};
  std::string cmd_com2{"test_srv;1;user;this is a second comment"};
  std::string cmd_com3{"test_srv;1;user;this is a third comment"};
  std::string cmd_com4{"test_srv;1;user;this is a fourth comment"};
  std::string cmd_com5{"test_srv2;1;user;this is a fifth comment"};
  std::string cmd_del{"1"};
  std::string cmd_del_last{"5"};
  std::string cmd_del_all{"test_srv"};

  cmd_add_comment(CMD_ADD_HOST_COMMENT, now,
                  const_cast<char*>(cmd_com1.c_str()));
  ASSERT_EQ(comment::comments.size(), 1u);
  cmd_add_comment(CMD_ADD_HOST_COMMENT, now,
                  const_cast<char*>(cmd_com2.c_str()));
  ASSERT_EQ(comment::comments.size(), 2u);
  cmd_add_comment(CMD_ADD_HOST_COMMENT, now,
                  const_cast<char*>(cmd_com3.c_str()));
  ASSERT_EQ(comment::comments.size(), 3u);
  cmd_add_comment(CMD_ADD_HOST_COMMENT, now,
                  const_cast<char*>(cmd_com4.c_str()));
  ASSERT_EQ(comment::comments.size(), 4u);
  cmd_add_comment(CMD_ADD_HOST_COMMENT, now,
                  const_cast<char*>(cmd_com5.c_str()));
  ASSERT_EQ(comment::comments.size(), 5u);
  cmd_delete_comment(CMD_DEL_HOST_COMMENT, const_cast<char*>(cmd_del.c_str()));
  ASSERT_EQ(comment::comments.size(), 4u);
  cmd_delete_all_comments(CMD_DEL_ALL_HOST_COMMENTS,
                          const_cast<char*>(cmd_del_all.c_str()));
  ASSERT_EQ(comment::comments.size(), 1u);
  cmd_delete_comment(CMD_DEL_HOST_COMMENT,
                     const_cast<char*>(cmd_del_last.c_str()));
  ASSERT_EQ(comment::comments.size(), 0u);
}

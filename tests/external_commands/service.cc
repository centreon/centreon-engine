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

  // We fake here the expand_object on configuration::service
  svc.set_host_id(1);

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

TEST_F(ServiceExternalCommand, AddServiceDowntimeByHostIpAddress) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::applier::command cmd_aply;
  configuration::service svc;
  configuration::host hst;
  configuration::command cmd("cmd");

  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.3"));
  ASSERT_TRUE(hst.parse("host_id", "1"));

  ASSERT_TRUE(svc.parse("host", "test_host"));
  ASSERT_TRUE(svc.parse("service_description", "test_description"));
  ASSERT_TRUE(svc.parse("service_id", "3"));

  cmd.parse("command_line", "/usr/bin/echo 1");
  cmd_aply.add_object(cmd);

  hst.parse("check_command", "cmd");
  svc.parse("check_command", "cmd");

  hst_aply.add_object(hst);

  // We fake here the expand_object on configuration::service
  svc.set_host_id(1);

  svc_aply.add_object(svc);

  hst_aply.expand_objects(*config);
  svc_aply.expand_objects(*config);

  hst_aply.resolve_object(hst);
  svc_aply.resolve_object(svc);

  set_time(20000);
  time_t now = time(nullptr);

  std::string str{"127.0.0.3;test_description;1;|"};

  testing::internal::CaptureStdout();
  cmd_process_service_check_result(
    CMD_PROCESS_SERVICE_CHECK_RESULT, now, const_cast<char *>(str.c_str()));
  checks::checker::instance().reap();

  std::string const& out{testing::internal::GetCapturedStdout()};

  ASSERT_NE(out.find("PASSIVE SERVICE CHECK"), std::string::npos);
}

TEST_F(ServiceExternalCommand, AddServiceComment) {
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

  // We fake here the expand_object on configuration::service
  svc.set_host_id(1);

  svc_aply.add_object(svc);

  hst_aply.expand_objects(*config);
  svc_aply.expand_objects(*config);

  hst_aply.resolve_object(hst);
  svc_aply.resolve_object(svc);

  std::string cmd_com1{"test_host;test_description;1;user;this is a first comment"};
  std::string cmd_com2{"test_host;test_description;1;user;this is a second comment"};
  std::string cmd_com3{"test_host;test_description;1;user;this is a third comment"};
  std::string cmd_com4{"test_host;test_description;1;user;this is a fourth comment"};
  std::string cmd_del{"1"};
  std::string cmd_del_last{"5"};
  std::string cmd_del_all{"test_host;test_description"};

  set_time(20000);
  time_t now = time(nullptr);


  cmd_add_comment(CMD_ADD_SVC_COMMENT, now, const_cast<char *>(cmd_com1.c_str()));
  ASSERT_EQ(comment::comments.size(), 1u);
  cmd_add_comment(CMD_ADD_SVC_COMMENT, now, const_cast<char *>(cmd_com2.c_str()));
  ASSERT_EQ(comment::comments.size(), 2u);
  cmd_add_comment(CMD_ADD_SVC_COMMENT, now, const_cast<char *>(cmd_com3.c_str()));
  ASSERT_EQ(comment::comments.size(), 3u);
  cmd_add_comment(CMD_ADD_SVC_COMMENT, now, const_cast<char *>(cmd_com4.c_str()));
  ASSERT_EQ(comment::comments.size(), 4u);
  cmd_delete_comment(CMD_ADD_SVC_COMMENT, const_cast<char *>(cmd_del.c_str()));
  ASSERT_EQ(comment::comments.size(), 3u);
  cmd_delete_all_comments(CMD_DEL_ALL_SVC_COMMENTS, const_cast<char *>(cmd_del_all.c_str()));
  ASSERT_EQ(comment::comments.size(), 0u);
}

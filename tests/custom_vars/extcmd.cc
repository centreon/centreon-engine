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

#include <gtest/gtest.h>
#include <com/centreon/engine/macros.hh>
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/macros/grab_host.hh"
#include "com/centreon/engine/modules/external_commands/commands.hh"
#include "helper.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

class CustomVar : public ::testing::Test {
 public:
  void SetUp() override { init_config_state(); }

  void TearDown() override { deinit_config_state(); }
};

// Given simple command (without connector) applier already applied with
// all objects created.
// When the command is removed from the configuration,
// Then the command is totally removed.
TEST_F(CustomVar, UpdateHostCustomVar) {
  configuration::applier::command cmd_aply;
  configuration::applier::host hst_aply;
  configuration::applier::contact cnt_aply;

  configuration::command cmd("base_centreon_ping");
  cmd.parse("command_line",
            "$USER1$/check_icmp -H $HOSTADDRESS$ -n $_HOSTPACKETNUMBER$ -w "
            "$_HOSTWARNING$ -c $_HOSTCRITICAL$ $CONTACTNAME$");
  cmd_aply.add_object(cmd);

  configuration::contact cnt;
  ASSERT_TRUE(cnt.parse("contact_name", "user"));
  ASSERT_TRUE(cnt.parse("email", "contact@centreon.com"));
  ASSERT_TRUE(cnt.parse("pager", "0473729383"));
  ASSERT_TRUE(cnt.parse("host_notification_period", "24x7"));
  ASSERT_TRUE(cnt.parse("service_notification_period", "24x7"));
  cnt_aply.add_object(cnt);

  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "hst_test"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "1"));
  ASSERT_TRUE(hst.parse("_PACKETNUMBER", "42"));
  ASSERT_TRUE(hst.parse("_WARNING", "200,20%"));
  ASSERT_TRUE(hst.parse("_CRITICAL", "400,50%"));
  ASSERT_TRUE(hst.parse("check_command", "base_centreon_ping"));
  ASSERT_TRUE(hst.parse("contacts", "user"));
  hst_aply.add_object(hst);

  command_map::iterator cmd_found{
      commands::command::commands.find("base_centreon_ping")};
  ASSERT_NE(cmd_found, commands::command::commands.end());
  ASSERT_TRUE(config->commands().size() == 1);

  host_map::iterator hst_found{engine::host::hosts.find("hst_test")};
  ASSERT_NE(hst_found, engine::host::hosts.end());
  ASSERT_TRUE(config->hosts().size() == 1);

  hst_aply.expand_objects(*config);
  hst_aply.resolve_object(hst);
  ASSERT_TRUE(hst_found->second->custom_variables.size() == 3);
  nagios_macros* macros(get_global_macros());
  grab_host_macros_r(macros, hst_found->second.get());
  std::string processed_cmd(
      hst_found->second->get_check_command_ptr()->process_cmd(macros));
  ASSERT_EQ(processed_cmd,
            "/check_icmp -H 127.0.0.1 -n 42 -w 200,20% -c 400,50% user");

  cmd_change_object_custom_var(CMD_CHANGE_CUSTOM_HOST_VAR,
                               "hst_test;PACKETNUMBER;44");
  grab_host_macros_r(macros, hst_found->second.get());
  std::string processed_cmd2(
      hst_found->second->get_check_command_ptr()->process_cmd(macros));
  ASSERT_EQ(processed_cmd2,
            "/check_icmp -H 127.0.0.1 -n 44 -w 200,20% -c 400,50% user");
}

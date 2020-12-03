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
#include <com/centreon/engine/macros.hh>
#include <memory>
#include <string>
#include <unordered_map>
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/connector.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/command.hh"
#include "com/centreon/engine/configuration/connector.hh"
#include "com/centreon/engine/macros/grab_host.hh"
#include "helper.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

class ApplierCommand : public ::testing::Test {
 public:
  void SetUp() override { init_config_state(); }

  void TearDown() override { deinit_config_state(); }
};

// Given a command applier
// And a configuration command just with a name
// Then the applier add_object adds the command in the configuration set
// but not in the commands map (the command is unusable).
TEST_F(ApplierCommand, UnusableCommandFromConfig) {
  configuration::applier::command aply;
  configuration::command cmd("cmd");
  ASSERT_THROW(aply.add_object(cmd), std::exception);
  set_command s(config->commands());
  ASSERT_EQ(s.size(), 1u);
  std::unordered_map<std::string, std::shared_ptr<commands::command>> cm(
      commands::command::commands);
  ASSERT_EQ(cm.size(), 0u);
}

// Given a command applier
// And a configuration command with a name and a command line
// Then the applier add_object adds the command into the configuration set
// and the commands map (accessible from commands::set::instance()).
TEST_F(ApplierCommand, NewCommandFromConfig) {
  configuration::applier::command aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  aply.add_object(cmd);
  set_command s(config->commands());
  ASSERT_EQ(s.size(), 1u);
  command_map::iterator found{commands::command::commands.find("cmd")};
  ASSERT_FALSE(found == commands::command::commands.end());
  ASSERT_FALSE(!found->second);
  ASSERT_EQ(found->second->get_name(), "cmd");
  ASSERT_EQ(found->second->get_command_line(), "echo 1");
}

// Given a command applier
// And a configuration command with a name, a command line and a connector
// Then the applier add_object adds the command into the configuration set
// but not in the commands map (the connector is not defined).
TEST_F(ApplierCommand, NewCommandWithEmptyConnectorFromConfig) {
  configuration::applier::command aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  cmd.parse("connector", "perl");
  ASSERT_THROW(aply.add_object(cmd), std::exception);
  set_command s(config->commands());
  ASSERT_EQ(s.size(), 1u);
  command_map::iterator found{commands::command::commands.find("cmd")};
  ASSERT_TRUE(found == commands::command::commands.end());
}

// Given a command applier
// And a configuration command with a name, a command line and a connector
// And the connector is well defined.
// Then the applier add_object adds the command into the configuration set
// but not in the commands map (the connector is not defined).
TEST_F(ApplierCommand, NewCommandWithConnectorFromConfig) {
  configuration::applier::command aply;
  configuration::applier::connector cnn_aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  cmd.parse("connector", "perl");
  configuration::connector cnn("perl");

  cnn_aply.add_object(cnn);
  aply.add_object(cmd);

  //  set_command s(config->commands());
  //  ASSERT_EQ(s.size(), 1);
  //  commands::command const*
  //  cc(applier::state::instance().find_command("cmd"));
  //  ASSERT_EQ(cc->get_name(), "cmd");
  //  ASSERT_EQ(cc->get_command_line(), "echo 1");
  //  aply.resolve_object(cmd);
}

// Given some command/connector appliers
// And a configuration command
// And a connector with the same name.
// Then the applier add_object adds the command into the configuration set
// but not in the commands map (the connector is not defined).
TEST_F(ApplierCommand, NewCommandAndConnectorWithSameName) {
  configuration::applier::command aply;
  configuration::applier::connector cnn_aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  configuration::connector cnn("cmd");
  cnn.parse("connector_line", "echo 2");

  cnn_aply.add_object(cnn);
  aply.add_object(cmd);

  set_command s(config->commands());
  ASSERT_EQ(s.size(), 1u);
  command_map::iterator found{commands::command::commands.find("cmd")};
  ASSERT_FALSE(found == commands::command::commands.end());
  ASSERT_FALSE(!found->second);

  ASSERT_EQ(found->second->get_name(), "cmd");
  ASSERT_EQ(found->second->get_command_line(), "echo 1");

  aply.resolve_object(cmd);
  connector_map::iterator found_con{
      commands::connector::connectors.find("cmd")};
  ASSERT_TRUE(found_con != commands::connector::connectors.end());
  ASSERT_TRUE(found_con->second);

  found = commands::command::commands.find("cmd");
  ASSERT_TRUE(found != commands::command::commands.end());
}

// Given some command and connector appliers already applied with
// all objects created.
// When the command is changed from the configuration,
// Then the modify_object() method updated correctly the command.
TEST_F(ApplierCommand, ModifyCommandWithConnector) {
  configuration::applier::command aply;
  configuration::applier::connector cnn_aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  cmd.parse("connector", "perl");
  configuration::connector cnn("perl");

  cnn_aply.add_object(cnn);
  aply.add_object(cmd);

  cmd.parse("command_line", "date");
  aply.modify_object(cmd);
  //  commands::command const*
  //  cc(applier::state::instance().find_command("cmd"));
  //  ASSERT_EQ(cc->get_name(), "cmd");
  //  ASSERT_EQ(cc->get_command_line(), "date");
}

// When a non existing command is removed
// Then an exception is thrown.
TEST_F(ApplierCommand, RemoveNonExistingCommand) {
  configuration::applier::command aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");

  ASSERT_THROW(aply.remove_object(cmd), std::exception);
}

// Given simple command (without connector) applier already applied with
// all objects created.
// When the command is removed from the configuration,
// Then the command is totally removed.
TEST_F(ApplierCommand, RemoveCommand) {
  configuration::applier::command aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");

  aply.add_object(cmd);

  aply.remove_object(cmd);
  command_map::iterator found{commands::command::commands.find("cmd")};
  ASSERT_EQ(found, commands::command::commands.end());
  ASSERT_TRUE(config->commands().size() == 0);
}

// Given some command and connector appliers already applied with
// all objects created.
// When the command is removed from the configuration,
// Then the command is totally removed.
TEST_F(ApplierCommand, RemoveCommandWithConnector) {
  configuration::applier::command aply;
  configuration::applier::connector cnn_aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  cmd.parse("connector", "perl");
  configuration::connector cnn("perl");

  cnn_aply.add_object(cnn);
  aply.add_object(cmd);

  aply.remove_object(cmd);
  command_map::iterator found{commands::command::commands.find("cmd")};
  ASSERT_EQ(found, commands::command::commands.end());
  ASSERT_TRUE(config->commands().size() == 0);
}

// Given simple command (without connector) applier already applied with
// all objects created.
// When the command is removed from the configuration,
// Then the command is totally removed.
TEST_F(ApplierCommand, ComplexCommand) {
  configuration::applier::command cmd_aply;
  configuration::applier::host hst_aply;

  configuration::command cmd("base_centreon_ping");
  cmd.parse("command_line",
            "$USER1$/check_icmp -H $HOSTADDRESS$ -n $_HOSTPACKETNUMBER$ -w "
            "$_HOSTWARNING$ -c $_HOSTCRITICAL$");
  cmd_aply.add_object(cmd);

  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "hst_test"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "1"));
  ASSERT_TRUE(hst.parse("_PACKETNUMBER", "42"));
  ASSERT_TRUE(hst.parse("_WARNING", "200,20%"));
  ASSERT_TRUE(hst.parse("_CRITICAL", "400,50%"));
  ASSERT_TRUE(hst.parse("check_command", "base_centreon_ping"));
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
            "/check_icmp -H 127.0.0.1 -n 42 -w 200,20% -c 400,50%");
}

// Given simple command (without connector) applier already applied with
// all objects created.
// When the command is removed from the configuration,
// Then the command is totally removed.
TEST_F(ApplierCommand, ComplexCommandWithContact) {
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
}

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
#include <memory>
#include <string>
#include <unordered_map>
#include "../../timeperiod/utils.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/connector.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/command.hh"
#include "com/centreon/engine/configuration/connector.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ApplierCommand : public ::testing::Test {
 public:
  void SetUp() override {
    if (config == nullptr)
      config = new configuration::state;
    applier::state::load();  // Needed to create a contact
  }

  void TearDown() override {
    configuration::applier::state::unload();
    delete config;
    config = nullptr;
  }

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
  command_map::iterator found{
    commands::command::commands.find("cmd")};
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
  command_map::iterator found{
    commands::command::commands.find("cmd")};
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
//  commands::command const* cc(applier::state::instance().find_command("cmd"));
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
  command_map::iterator found{
    commands::command::commands.find("cmd")};
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
//  commands::command const* cc(applier::state::instance().find_command("cmd"));
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
  command_map::iterator found{
    commands::command::commands.find("cmd")};
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
  command_map::iterator found{
    commands::command::commands.find("cmd")};
  ASSERT_EQ(found, commands::command::commands.end());
  ASSERT_TRUE(config->commands().size() == 0);
}

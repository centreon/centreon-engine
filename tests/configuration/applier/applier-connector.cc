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
#include <memory>
#include <gtest/gtest.h>
#include "../../timeperiod/utils.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/connector.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/command.hh"
#include "com/centreon/engine/configuration/connector.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/timezone_manager.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ApplierConnector : public ::testing::Test {
 public:
  void SetUp() override {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contact
    timezone_manager::load();
    checks::checker::load();
  }

  void TearDown() override {
    configuration::applier::state::unload();
    checks::checker::unload();
    delete config;
    config = nullptr;
    timezone_manager::unload();
  }

};

// Given a connector applier
// And a configuration connector just with a name
// Then the applier add_object adds the connector in the configuration set
// and in the connectors map.
TEST_F(ApplierConnector, UnusableConnectorFromConfig) {
  configuration::applier::connector aply;
  configuration::connector cnn("connector");
  aply.add_object(cnn);
  set_connector s(config->connectors());
  ASSERT_EQ(s.size(), 1u);
  ASSERT_EQ(commands::connector::connectors.size(), 1u);

}

// Given a connector applier already applied
// When the connector is modified from the configuration,
// Then the modify_object() method updated correctly the connector.
TEST_F(ApplierConnector, ModifyConnector) {
  configuration::applier::connector aply;
  configuration::connector cnn("connector");
  cnn.parse("connector_line", "perl");

  aply.add_object(cnn);

  cnn.parse("connector_line", "date");
  aply.modify_object(cnn);

  connector_map::iterator found_con{
    commands::connector::connectors.find("connector")};
  ASSERT_FALSE(found_con == commands::connector::connectors.end());
  ASSERT_FALSE(!found_con->second);

  ASSERT_EQ(found_con->second->get_name(), "connector");
  ASSERT_EQ(found_con->second->get_command_line(), "date");
}

// When a non existing connector is modified
// Then an exception is thrown.
TEST_F(ApplierConnector, ModifyNonExistingConnector) {
  configuration::applier::connector aply;
  configuration::connector cnn("connector");
  cnn.parse("connector_line", "echo 1");

  ASSERT_THROW(aply.modify_object(cnn), std::exception);
}

// When a non existing connector is removed
// Then nothing is done.
TEST_F(ApplierConnector, RemoveNonExistingConnector) {
  configuration::applier::connector aply;
  configuration::connector cnn("connector");
  cnn.parse("connector_line", "echo 1");

  ASSERT_TRUE(config->connectors().size() == 0);
  ASSERT_TRUE(commands::connector::connectors.size() == 0);
}

// Given simple connector applier already applied
// When the connector is removed from the configuration,
// Then the connector is totally removed.
TEST_F(ApplierConnector, RemoveConnector) {
  configuration::applier::connector aply;
  configuration::connector cnn("connector");
  cnn.parse("connector_line", "echo 1");

  aply.add_object(cnn);
  aply.remove_object(cnn);
  ASSERT_TRUE(config->connectors().size() == 0);
  ASSERT_TRUE(commands::connector::connectors.size() == 0);
}

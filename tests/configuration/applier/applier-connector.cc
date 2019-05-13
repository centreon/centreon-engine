/*
** Copyright 2017 Centreon
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version 2
** as published by the Free Software Foundation.
**
** Centreon Engine is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Centreon Engine. If not, see
** <http://www.gnu.org/licenses/>.
*/
#include <memory>
#include <gtest/gtest.h>
#include "../../timeperiod/utils.hh"
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/connector.hh"
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

class ApplierConnector : public ::testing::Test {
 public:
  void SetUp() override {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contact
    commands::set::load();
  }

  void TearDown() override {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
    commands::set::unload();
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
  ASSERT_EQ(s.size(), 1);
  ASSERT_EQ(configuration::applier::state::instance().connectors().size(), 1);

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

  commands::connector* cc(applier::state::instance().find_connector("connector"));
  ASSERT_EQ(cc->get_name(), "connector");
  ASSERT_EQ(cc->get_command_line(), "date");
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
  ASSERT_TRUE(configuration::applier::state::instance().connectors().size() == 0);
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
  ASSERT_TRUE(configuration::applier::state::instance().connectors().size() == 0);
}

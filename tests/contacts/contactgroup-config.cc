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
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contactgroup.hh"
#include "com/centreon/engine/configuration/state.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ContactgroupConfig : public ::testing::Test {
 public:
  void SetUp() override {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contactgroup
  }

  void TearDown() override {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }
};

// Given an empty name
// When a contactgroup is created with it as argument,
// Then it throws an exception.
// TEST_F(ContactgroupConfig, NewContactgroupWithNoName) {
//  configuration::contactgroup ctg("");
//  ASSERT_THROW(new engine::contactgroup(ctg), std::exception);
//}

// Given a non empty name
// When a contactgroup is created with it as argument,
// Then a contactgroup is created and the method returns a pointer to it
// And since alias is empty, it is replaced by the name.
// TEST_F(ContactgroupConfig, NewContactgroupWithName) {
//  configuration::contactgroup ctg("test");
//  std::auto_ptr<engine::contactgroup> c(new engine::contactgroup(ctg));
//  ASSERT_EQ(c->get_name(), "test");
//  ASSERT_EQ(c->get_alias(), "test");
//}

// Given a non empty name and a non empty alias
// When a contactgroup is created with them as argument,
// Then the contactgroup is created with the given name and alias.
// TEST_F(ContactgroupConfig, NewContactgroupWithNonEmptyAlias) {
//  configuration::contactgroup ctg("test");
//  ctg.parse("alias", "alias_test");
//  std::auto_ptr<engine::contactgroup> c(new engine::contactgroup(ctg));
//  ASSERT_EQ(c->get_alias(), "alias_test");
//}

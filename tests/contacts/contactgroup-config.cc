/*
** Copyright 2017-2018 Centreon
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
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contactgroup.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ContactgroupConfig : public ::testing::Test {
 public:
  void SetUp() {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contactgroup
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

};

// Given an empty name
// When a contactgroup is created with it as argument,
// Then it throws an exception.
//TEST_F(ContactgroupConfig, NewContactgroupWithNoName) {
//  configuration::contactgroup ctg("");
//  ASSERT_THROW(new engine::contactgroup(ctg), std::exception);
//}

// Given a non empty name
// When a contactgroup is created with it as argument,
// Then a contactgroup is created and the method returns a pointer to it
// And since alias is empty, it is replaced by the name.
//TEST_F(ContactgroupConfig, NewContactgroupWithName) {
//  configuration::contactgroup ctg("test");
//  std::auto_ptr<engine::contactgroup> c(new engine::contactgroup(ctg));
//  ASSERT_EQ(c->get_name(), "test");
//  ASSERT_EQ(c->get_alias(), "test");
//}

// Given a non empty name and a non empty alias
// When a contactgroup is created with them as argument,
// Then the contactgroup is created with the given name and alias.
//TEST_F(ContactgroupConfig, NewContactgroupWithNonEmptyAlias) {
//  configuration::contactgroup ctg("test");
//  ctg.parse("alias", "alias_test");
//  std::auto_ptr<engine::contactgroup> c(new engine::contactgroup(ctg));
//  ASSERT_EQ(c->get_alias(), "alias_test");
//}

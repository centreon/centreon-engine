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
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contactgroup.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class SimpleContactgroup : public ::testing::Test {
 public:
  void SetUp() override {
//    set_time(20);
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contact
  }

  void TearDown() override {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

};

// Given an empty name
// When the add_contactgroup function is called with it as argument,
// Then it returns a NULL pointer.
//TEST_F(SimpleContactgroup, NewContactgroupWithNoName) {
//  ASSERT_TRUE(engine::contactgroup::add_contactgroup("") == NULL);
//}

// Given a non empty name
// When the add_contactgroup function is called with it as argument,
// Then a contactgroup is created and the method returns a pointer to it
// And since alias is empty, it is replaced by the name.
//TEST_F(SimpleContactgroup, NewContactgroupWithName) {
//  engine::contactgroup* c(engine::contactgroup::add_contactgroup("test"));
//  ASSERT_EQ(c->get_name(), "test");
//  ASSERT_EQ(c->get_alias(), "test");
//}

// Given a non empty name
// When the add_contactgroup function is called with it as argument two times,
// Then the contactgroup is well created but just one time.
//TEST_F(SimpleContactgroup, NewContactgroupRepeatedTwoTimes) {
//  engine::contactgroup* c(engine::contactgroup::add_contactgroup("test"));
//  ASSERT_EQ(c->get_name(), "test");
//  ASSERT_TRUE(engine::contactgroup::add_contactgroup("test") == NULL);
//}

// Given a non empty name and a non empty alias
// When the add_contactgroup function is called with them as argument,
// Then the contactgroup is created with the given name and alias.
//TEST_F(SimpleContactgroup, NewContactgroupWithNonEmptyAlias) {
//  engine::contactgroup* c(engine::contactgroup::add_contactgroup(
//                       "test",
//                       "alias_test"));
//  ASSERT_EQ(c->get_alias(), "alias_test");
//}

// Given an empty contactgroup
// When the check method is executed
// Then no warning, nor error are given.
// And the method returns true (errors count == 0)
//TEST_F(SimpleContactgroup, TestContactgroupCheck) {
//  engine::contactgroup* c(engine::contactgroup::add_contactgroup("test"));
//  int w = 0, e = 0;
//  ASSERT_TRUE(c->check(&w, &e));
//}

// Given a contactgroup
// When the add_contact method is called with an empty name
// Then an exception is thrown.
//TEST_F(SimpleContactgroup, TestContactgroupCheckWithBadContact) {
//  engine::contactgroup* c(engine::contactgroup::add_contactgroup("test"));
//  ASSERT_THROW(c->add_contact(""), std::exception);
//}

// Given a contactgroup
// When the add_contact method is called with a non empty name
// corresponding to a non existing contact
// Then an error is given
// And the method returns false.
//TEST_F(SimpleContactgroup, TestContactgroupCheckWithOneInexistentContact) {
//  engine::contactgroup* c(engine::contactgroup::add_contactgroup("test"));
//  c->add_contact("centreon");
//  int w(0), e(0);
//  ASSERT_FALSE(c->check(&w, &e));
//  ASSERT_EQ(w, 0);
//  ASSERT_EQ(e, 1);
//}

//// Given a contactgroup
//// When the add_contact method is called with a non empty name
//// corresponding to an existing contact
//// Then an error is given
//// And the method returns false.
//// Then the contact group contains only the add user.
//// When the clear_members() is executed, the contact group contains no more
//// member.
//TEST_F(SimpleContactgroup, TestContactgroupCheckWithOneContact) {
//  engine::contactgroup* c(engine::contactgroup::add_contactgroup("test"));
//  c->add_contact("centreon");
//  shared_ptr<engine::contact> user(new engine::contact("centreon"));
//  contact_map& cm(configuration::applier::state::instance().contacts());
//  cm["centreon"] = user;
//  int w(0), e(0);
//  ASSERT_TRUE(c->check(&w, &e));
//  ASSERT_EQ(w, 0);
//  ASSERT_EQ(e, 0);
//
//  ASSERT_FALSE(c->contains_member("nagios"));
//  ASSERT_TRUE(c->contains_member("centreon"));
//
//  c->clear_members();
//  ASSERT_TRUE(c->get_members().empty());
//}

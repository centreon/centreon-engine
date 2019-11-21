/*
 * Copyright 2018 - 2019 Centreon (https://www.centreon.com/)
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
#include "../../timeperiod/utils.hh"
#include "com/centreon/clib.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/contactgroup.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;
extern int config_errors;
extern int config_warnings;

class ApplierContactgroup : public ::testing::Test {
 public:
  void SetUp() override {
    config_errors = 0;
    config_warnings = 0;
    if (config == NULL)
      config = new configuration::state;
    clib::load();
    com::centreon::logging::engine::load();
    configuration::applier::state::load();  // Needed to create a contact
  }

  void TearDown() override {
    configuration::applier::state::unload();
    com::centreon::logging::engine::unload();
    clib::unload();
    delete config;
    config = NULL;
  }
};

// Given a contactgroup applier
// And a configuration contactgroup
// When we modify the contactgroup configuration with a non existing
// contactgroup configuration
// Then an exception is thrown.
TEST_F(ApplierContactgroup, ModifyUnexistingContactgroupConfigFromConfig) {
  configuration::applier::contactgroup aply;
  configuration::contactgroup cg("test");
  ASSERT_TRUE(cg.parse("members", "contact"));
  ASSERT_THROW(aply.modify_object(cg), std::exception);
}

// Given a contactgroup applier
// And a configuration contactgroup
// When we modify the contactgroup configuration with a non existing
// contactgroup
// Then an exception is thrown.
TEST_F(ApplierContactgroup, ModifyUnexistingContactgroupFromConfig) {
  configuration::applier::contactgroup aply;
  configuration::contactgroup cg("test");
  ASSERT_TRUE(cg.parse("members", "contact"));
  config->contactgroups().insert(cg);
  ASSERT_THROW(aply.modify_object(cg), std::exception);
}

// Given a contactgroup applier
// And a configuration contactgroup in configuration
// When we modify the contactgroup configuration
// Then the applier modify_object updates the contactgroup.
// TEST_F(ApplierContactgroup, ModifyContactgroupFromConfig) {
//  configuration::applier::contactgroup aply;
//  configuration::contactgroup cg("test");
//  ASSERT_TRUE(cg.parse("members", "contact"));
//  aply.add_object(cg);
//  contactgroup_map::const_iterator
//    it(configuration::applier::state::instance().contactgroups().find("test"));
//  ASSERT_TRUE(it->second->get_alias() == "test");
//
//  ASSERT_TRUE(cg.parse("alias", "test_renamed"));
//  aply.modify_object(cg);
//  it = configuration::applier::state::instance().contactgroups().find("test");
//  ASSERT_TRUE(it->second->get_alias() == "test_renamed");
//}

// Given a contactgroup applier
// And a configuration contactgroup in configuration
// When we change remove the configuration
// Then it is really removed
TEST_F(ApplierContactgroup, RemoveContactgroupFromConfig) {
  configuration::applier::contact caply;
  configuration::applier::contactgroup aply;
  configuration::contactgroup cg("test");
  configuration::contact ct("contact");

  caply.add_object(ct);
  ASSERT_TRUE(cg.parse("members", "contact"));
  aply.add_object(cg);
  ASSERT_FALSE(engine::contactgroup::contactgroups.empty());

  aply.remove_object(cg);
  ASSERT_TRUE(engine::contactgroup::contactgroups.empty());
}

// Given a contactgroup applier and a configuration contactgroup
// Then the add_object() of the applier creates the contactgroup.
// TEST_F(ApplierContactgroup, NewContactgroupFromConfig) {
//  configuration::applier::contactgroup aplyr;
//  configuration::contactgroup grp("test_group");
//  aplyr.add_object(grp);
//  contactgroup_map const&
//  cgs(configuration::applier::state::instance().contactgroups());
//  ASSERT_EQ(cgs.size(), 1);
//}
//
//// Given an empty contactgroup
//// When the resolve_object() method is called
//// Then no warning, nor error are given
// TEST_F(ApplierContactgroup, ResolveEmptyContactgroup) {
//  configuration::applier::contactgroup aplyr;
//  configuration::contactgroup grp("test");
//  aplyr.add_object(grp);
//  aplyr.expand_objects(*config);
//  aplyr.resolve_object(grp);
//  ASSERT_EQ(config_warnings, 0);
//  ASSERT_EQ(config_errors, 0);
//}

// Given a contactgroup with a non-existing contact
// When the resolve_object() method is called
// Then an exception is thrown
// And the method returns 1 error
TEST_F(ApplierContactgroup, ResolveInexistentContact) {
  configuration::applier::contactgroup aplyr;
  configuration::contactgroup grp("test");
  grp.parse("members", "non_existing_contact");
  ASSERT_THROW(aplyr.add_object(grp), std::exception);
}

// Given a contactgroup with a contact
// When the resolve_object() method is called
// Then the contact is really added to the contact group.
TEST_F(ApplierContactgroup, ResolveContactgroup) {
  configuration::applier::contact aply;
  configuration::applier::contactgroup aply_grp;
  configuration::contactgroup grp("test_group");
  configuration::contact ctct("test");
  aply.add_object(ctct);
  ASSERT_TRUE(ctct.parse("contactgroups", "test_group"));
  grp.parse("members", "test");
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  ASSERT_NO_THROW(aply_grp.resolve_object(grp));
}

// Given a contactgroup with a contact already configured
// And a second contactgroup configuration
// When we set the first one as contactgroup member to the second
// Then the parse method returns true and set the first one contacts
// to the second one.
TEST_F(ApplierContactgroup, SetContactgroupMembers) {
  configuration::applier::contact aply;
  configuration::applier::contactgroup aply_grp;
  configuration::contactgroup grp("test_group");
  configuration::contact ctct("test");
  aply.add_object(ctct);
  grp.parse("members", "test");
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  aply_grp.resolve_object(grp);
  ASSERT_TRUE(grp.members().size() == 1);

  configuration::contactgroup grp1("big_group");
  ASSERT_TRUE(grp1.parse("contactgroup_members", "test_group"));
  aply_grp.add_object(grp1);
  aply_grp.expand_objects(*config);

  // grp1 must be reload because the expand_objects reload them totally.
  ASSERT_TRUE(config->contactgroups_find("big_group")->members().size() == 1);
}

TEST_F(ApplierContactgroup, ContactRemove) {
  configuration::applier::contact aply;
  configuration::applier::contactgroup aply_grp;
  configuration::contactgroup grp("test_group");

  configuration::contact ctct("test");
  aply.add_object(ctct);

  configuration::contact ctct2("test2");
  aply.add_object(ctct2);


  grp.parse("members", "test, test2");
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  aply_grp.resolve_object(grp);
  ASSERT_EQ(engine::contactgroup::contactgroups["test_group"]->get_members().size(), 2u);

  aply.remove_object(ctct2);
  ASSERT_EQ(engine::contactgroup::contactgroups["test_group"]->get_members().size(), 1u);
  grp.parse("members", "test");
  aply_grp.modify_object(grp);
}

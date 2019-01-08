/*
** Copyright 2018 Centreon
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

#include <cstring>
#include <memory>
#include <gtest/gtest.h>
#include "../../timeperiod/utils.hh"
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/servicegroup.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;
extern int config_errors;
extern int config_warnings;

class ApplierServicegroup : public ::testing::Test {
 public:
  void SetUp() {
    config_errors = 0;
    config_warnings = 0;
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a service
    commands::set::load();
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }
};

// Given a servicegroup applier
// And a configuration servicegroup
// When we modify the servicegroup configuration with a non existing
// servicegroup configuration
// Then an exception is thrown.
TEST_F(ApplierServicegroup, ModifyUnexistingServicegroupConfigFromConfig) {
  configuration::applier::servicegroup aply;
  configuration::servicegroup sg("test");
  ASSERT_TRUE(sg.parse("members", "host1,service1"));
  ASSERT_THROW(aply.modify_object(sg), std::exception);
}

// Given a servicegroup applier
// And a configuration servicegroup
// When we modify the servicegroup configuration with a non existing
// servicegroup
// Then an exception is thrown.
TEST_F(ApplierServicegroup, ModifyUnexistingServicegroupFromConfig) {
  configuration::applier::servicegroup aply;
  configuration::servicegroup sg("test");
  ASSERT_TRUE(sg.parse("members", "host1,service1"));
  config->servicegroups().insert(sg);
  ASSERT_THROW(aply.modify_object(sg), std::exception);
}

// Given a servicegroup applier
// And a configuration servicegroup in configuration
// When we modify the servicegroup configuration
// Then the applier modify_object updates the servicegroup.
TEST_F(ApplierServicegroup, ModifyServicegroupFromConfig) {
  configuration::applier::servicegroup aply;
  configuration::servicegroup sg("test");
  ASSERT_TRUE(sg.parse("members", "host1,service1"));
  aply.add_object(sg);
  umap<std::string, shared_ptr<servicegroup_struct> >::const_iterator
    it(configuration::applier::state::instance().servicegroups_find("test"));
  ASSERT_TRUE(strcmp(it->second->alias, "test") == 0);

  ASSERT_TRUE(sg.parse("alias", "test_renamed"));
  aply.modify_object(sg);
  it = configuration::applier::state::instance().servicegroups().find("test");
  ASSERT_TRUE(strcmp(it->second->alias, "test_renamed") == 0);
}

// Given an empty servicegroup
// When the resolve_object() method is called
// Then no warning, nor error are given
TEST_F(ApplierServicegroup, ResolveEmptyservicegroup) {
  configuration::applier::servicegroup aplyr;
  configuration::servicegroup grp("test");
  aplyr.add_object(grp);
  aplyr.expand_objects(*config);
  aplyr.resolve_object(grp);
  ASSERT_EQ(config_warnings, 0);
  ASSERT_EQ(config_errors, 0);
}

// Given a servicegroup with a non-existing service
// When the resolve_object() method is called
// Then an exception is thrown
// And the method returns 1 error
TEST_F(ApplierServicegroup, ResolveInexistentService) {
  configuration::applier::servicegroup aplyr;
  configuration::servicegroup grp("test");
  grp.parse("members", "host1,non_existing_service");
  aplyr.add_object(grp);
  aplyr.expand_objects(*config);
  ASSERT_THROW(aplyr.resolve_object(grp), std::exception);
  ASSERT_EQ(config_warnings, 0);
  ASSERT_EQ(config_errors, 1);
}

// Given a servicegroup with a service and a host
// When the resolve_object() method is called
// Then the service is really added to the service group.
TEST_F(ApplierServicegroup, ResolveServicegroup) {
  configuration::applier::host aply_hst;
  configuration::applier::service aply_svc;
  configuration::applier::command aply_cmd;
  configuration::applier::servicegroup aply_grp;
  configuration::servicegroup grp("test_group");
  configuration::host hst;
  configuration::command cmd("cmd");
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  aply_hst.add_object(hst);
  configuration::service svc;
  ASSERT_TRUE(svc.parse("service_description", "test"));
  ASSERT_TRUE(svc.parse("hosts", "test_host"));
  ASSERT_TRUE(svc.parse("service_id", "18"));
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  aply_cmd.add_object(cmd);
  aply_svc.add_object(svc);
  ASSERT_TRUE(svc.parse("servicegroups", "test_group"));
  grp.parse("members", "test_host,test");
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  ASSERT_NO_THROW(aply_grp.resolve_object(grp));
}

// Given a servicegroup with a service already configured
// And a second servicegroup configuration
// When we set the first one as servicegroup member to the second
// Then the parse method returns true and set the first one service
// to the second one.
TEST_F(ApplierServicegroup, SetServicegroupMembers) {
  configuration::applier::host aply_hst;
  configuration::applier::service aply_svc;
  configuration::applier::command aply_cmd;
  configuration::applier::servicegroup aply_grp;
  configuration::servicegroup grp("test_group");
  configuration::host hst;
  configuration::command cmd("cmd");
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  aply_hst.add_object(hst);
  configuration::service svc;
  ASSERT_TRUE(svc.parse("service_description", "test"));
  ASSERT_TRUE(svc.parse("hosts", "test_host"));
  ASSERT_TRUE(svc.parse("service_id", "18"));
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  aply_cmd.add_object(cmd);
  aply_svc.add_object(svc);
  ASSERT_TRUE(svc.parse("servicegroups", "test_group"));
  grp.parse("members", "test_host,test");
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  aply_grp.resolve_object(grp);
  ASSERT_TRUE(grp.members().size() == 1);

  configuration::servicegroup grp1("big_group");
  ASSERT_TRUE(grp1.parse("servicegroup_members", "test_group"));
  aply_grp.add_object(grp1);
  aply_grp.expand_objects(*config);

  // grp1 must be reload because the expand_objects reload them totally.
  ASSERT_TRUE(config->servicegroups_find("big_group")->members().size() == 1);
}

// Given a servicegroup applier
// And a configuration servicegroup in configuration
// When we remove the configuration
// Then it is really removed
TEST_F(ApplierServicegroup, RemoveServicegroupFromConfig) {
  configuration::applier::host aply_hst;
  configuration::applier::service aply_svc;
  configuration::applier::command aply_cmd;
  configuration::applier::servicegroup aply_grp;
  configuration::servicegroup grp("test_group");
  configuration::host hst;
  configuration::command cmd("cmd");
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  aply_hst.add_object(hst);
  configuration::service svc;
  ASSERT_TRUE(svc.parse("service_description", "test"));
  ASSERT_TRUE(svc.parse("hosts", "test_host"));
  ASSERT_TRUE(svc.parse("service_id", "18"));
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  aply_cmd.add_object(cmd);
  aply_svc.add_object(svc);
  ASSERT_TRUE(svc.parse("servicegroups", "test_group"));
  grp.parse("members", "test_host,test");
  aply_grp.add_object(grp);
  aply_grp.expand_objects(*config);
  aply_grp.resolve_object(grp);
  ASSERT_TRUE(grp.members().size() == 1);

  configuration::servicegroup grp1("big_group");
  ASSERT_TRUE(grp1.parse("servicegroup_members", "test_group"));
  aply_grp.add_object(grp1);
  aply_grp.expand_objects(*config);
  grp1 = *config->servicegroups_find("big_group");
  ASSERT_TRUE(grp1.members().size() == 1);

  ASSERT_TRUE(
    configuration::applier::state::instance().servicegroups().size() == 2);
  aply_grp.remove_object(grp);
  ASSERT_TRUE(
    configuration::applier::state::instance().servicegroups().size() == 1);
}

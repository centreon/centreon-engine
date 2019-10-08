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

#include <cstring>
#include <iostream>
#include <memory>
#include <gtest/gtest.h>
#include "../../timeperiod/utils.hh"
#include "com/centreon/clib.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/hostgroup.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/hostgroup.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/host.hh"
#include "com/centreon/engine/macros/grab_host.hh"
#include "com/centreon/engine/macros/grab_value.hh"
#include "com/centreon/engine/timezone_manager.hh"
#include "com/centreon/engine/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ApplierHostGroup : public ::testing::Test {
 public:
  void SetUp() override {
    clib::load();
    com::centreon::logging::engine::load();
    if (config == nullptr)
      config = new configuration::state;
    timezone_manager::load();
    configuration::applier::state::load();  // Needed to create a contact
    checks::checker::load();
  }

  void TearDown() override {
    configuration::applier::state::unload();
    checks::checker::unload();
    delete config;
    config = nullptr;
    timezone_manager::unload();
    com::centreon::logging::engine::unload();
    clib::unload();
  }

};

// Given host configuration without host_id
// Then the applier add_object throws an exception.
TEST_F(ApplierHostGroup, NewHostGroup) {
  configuration::applier::hostgroup hg_aply;
  configuration::applier::host hst_aply;
  configuration::hostgroup hg;
  configuration::host hst_a;
  configuration::host hst_b;
  configuration::host hst_c;

  ASSERT_TRUE(hst_a.parse("host_name", "a"));
  ASSERT_TRUE(hst_a.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst_a.parse("_HOST_ID", "1"));

  ASSERT_TRUE(hst_b.parse("host_name", "b"));
  ASSERT_TRUE(hst_b.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst_b.parse("_HOST_ID", "2"));

  ASSERT_TRUE(hst_c.parse("host_name", "c"));
  ASSERT_TRUE(hst_c.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst_c.parse("_HOST_ID", "3"));
  hst_aply.add_object(hst_a);
  hst_aply.add_object(hst_b);
  hst_aply.add_object(hst_c);

  ASSERT_TRUE(hg.parse("hostgroup_name", "temphg"));
  ASSERT_TRUE(hg.parse("members", "a,b,c"));
  ASSERT_NO_THROW(hg_aply.add_object(hg));

  ASSERT_NO_THROW(hst_aply.expand_objects(*config));
  ASSERT_NO_THROW(hst_aply.expand_objects(*config));
  ASSERT_NO_THROW(hst_aply.expand_objects(*config));
  ASSERT_NO_THROW(hg_aply.expand_objects(*config));

  ASSERT_NO_THROW(hst_aply.resolve_object(hst_a));
  ASSERT_NO_THROW(hst_aply.resolve_object(hst_b));
  ASSERT_NO_THROW(hst_aply.resolve_object(hst_c));
  ASSERT_NO_THROW(hg_aply.resolve_object(hg));

  ASSERT_EQ(engine::hostgroup::hostgroups.size(), 1u);
  ASSERT_EQ(engine::hostgroup::hostgroups.begin()->second->members.size(), 3u);
}

// Given a host configuration
// When we change the host name in the configuration
// Then the applier modify_object changes the host name without changing
// the host id.
TEST_F(ApplierHostGroup, HostRenamed) {
  configuration::applier::hostgroup hg_aply;
  configuration::applier::host hst_aply;
  configuration::hostgroup hg;
  configuration::host hst_a;
  configuration::host hst_c;

  ASSERT_TRUE(hst_a.parse("host_name", "a"));
  ASSERT_TRUE(hst_a.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst_a.parse("_HOST_ID", "1"));

  ASSERT_TRUE(hst_c.parse("host_name", "c"));
  ASSERT_TRUE(hst_c.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst_c.parse("_HOST_ID", "2"));

  hst_aply.add_object(hst_a);
  hst_aply.add_object(hst_c);

  ASSERT_TRUE(hg.parse("hostgroup_name", "temphg"));
  ASSERT_TRUE(hg.parse("members", "a,c"));
  ASSERT_NO_THROW(hg_aply.add_object(hg));

  ASSERT_NO_THROW(hst_aply.expand_objects(*config));
  ASSERT_NO_THROW(hst_aply.expand_objects(*config));
  ASSERT_NO_THROW(hg_aply.expand_objects(*config));

  ASSERT_NO_THROW(hst_aply.resolve_object(hst_a));
  ASSERT_NO_THROW(hst_aply.resolve_object(hst_c));
  ASSERT_NO_THROW(hg_aply.resolve_object(hg));

  ASSERT_NO_THROW(hg_aply.remove_object(hg));
  ASSERT_TRUE(hg.parse("hostgroup_name", "temp_hg"));
  ASSERT_NO_THROW(hg_aply.add_object(hg));

  ASSERT_NO_THROW(hst_aply.expand_objects(*config));
  ASSERT_NO_THROW(hst_aply.expand_objects(*config));
  ASSERT_NO_THROW(hg_aply.expand_objects(*config));

  ASSERT_EQ(engine::hostgroup::hostgroups.size(), 1u);
  ASSERT_EQ(engine::hostgroup::hostgroups.begin()->second->members.size(), 2u);
  ASSERT_EQ(engine::hostgroup::hostgroups.begin()->second->get_group_name(),
            "temp_hg");
}

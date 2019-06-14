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

#include <cstring>
#include <iostream>
#include <memory>
#include <gtest/gtest.h>
#include "../../timeperiod/utils.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/error.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ApplierHost : public ::testing::Test {
 public:
  void SetUp() override {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contact
    // Do not unload this in the tear down function, it is done by the
    // other unload function... :-(
  }

  void TearDown() override {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

};

// Given host configuration without host_id
// Then the applier add_object throws an exception.
TEST_F(ApplierHost, NewHostWithoutHostId) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_THROW(hst_aply.add_object(hst), std::exception);
}

// Given a host configuration
// When we change the host name in the configuration
// Then the applier modify_object changes the host name without changing
// the host id.
TEST_F(ApplierHost, HostRenamed) {
  configuration::applier::host hst_aply;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("_HOST_ID", "12"));
  hst_aply.add_object(hst);
  umap<unsigned long, std::shared_ptr<engine::host> > const&
    hm(configuration::applier::state::instance().hosts());
  ASSERT_EQ(hm.size(), 1);
  std::shared_ptr<com::centreon::engine::host> h1(hm.begin()->second);
  ASSERT_TRUE(h1->get_name() == "test_host");

  ASSERT_TRUE(hst.parse("host_name", "test_host1"));
  hst_aply.modify_object(hst);
  ASSERT_EQ(hm.size(), 1);
  h1 = hm.begin()->second;
  ASSERT_TRUE(h1->get_name() == "test_host1");
  ASSERT_EQ(get_host_id(h1->get_name()), 12);
}

/*
** Copyright 2019 Centreon
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
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

extern configuration::state* config;

class ApplierService : public ::testing::Test {
 public:
  void SetUp() {
    if (config == NULL)
      config = new configuration::state;
    configuration::applier::state::load();  // Needed to create a contact
  }

  void TearDown() {
    configuration::applier::state::unload();
    delete config;
    config = NULL;
  }

};

// Given service configuration with an host not defined
// Then the applier add_object throws an exception because it needs a service
// command.
TEST_F(ApplierService, NewServiceWithHostNotDefinedFromConfig) {
  configuration::applier::service svc_aply;
  configuration::service svc;
  ASSERT_TRUE(svc.parse("hosts", "test_host"));
  ASSERT_TRUE(svc.parse("service_description", "test description"));
  ASSERT_TRUE(svc.parse("_TEST", "Value1"));
  ASSERT_THROW(svc_aply.add_object(svc), std::exception);
}

// Given service configuration with a host defined
// Then the applier add_object creates the service
TEST_F(ApplierService, NewServiceFromConfig) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_TRUE(svc.parse("host", "test_host"));
  ASSERT_TRUE(svc.parse("service_description", "test description"));

  configuration::applier::command cmd_aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  cmd_aply.add_object(cmd);

  svc_aply.add_object(svc);
  svc_aply.expand_objects(*config);
  umap<std::pair<std::string, std::string>, shared_ptr<service_struct> > const&
    sm(configuration::applier::state::instance().services());
  ASSERT_EQ(sm.size(), 1);
  ASSERT_EQ(sm.begin()->first.first, "test_host");
  ASSERT_EQ(sm.begin()->first.second, "test description");

  // Service is not resolved, host is null now.
  ASSERT_TRUE(sm.begin()->second->host_ptr == NULL);
  ASSERT_TRUE(strcmp(sm.begin()->second->description, "test description") == 0);
}

// Given a service configuration,
// When we duplicate it, we get a configuration equal to the previous one.
// When two services are generated from the same configuration
// Then they are equal.
// When Modifying a configuration changes,
// Then the '<' effect on configurations.
TEST_F(ApplierService, ServicesEquality) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service csvc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  hst_aply.add_object(hst);
  ASSERT_TRUE(csvc.parse("hosts", "test_host"));
  ASSERT_TRUE(csvc.parse("service_description", "test description1"));
  ASSERT_TRUE(csvc.parse("service_id", "12345"));
  ASSERT_TRUE(csvc.parse("acknowledgement_timeout", "21"));

  configuration::applier::command cmd_aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  csvc.parse("check_command", "cmd");
  cmd_aply.add_object(cmd);

  svc_aply.add_object(csvc);
  ASSERT_TRUE(csvc.parse("service_description", "test description2"));
  svc_aply.add_object(csvc);
  umap<std::pair<std::string, std::string>, shared_ptr<service_struct> > const&
    sm(configuration::applier::state::instance().services());
  ASSERT_EQ(sm.size(), 2);
  umap<std::pair<std::string, std::string>, shared_ptr<service_struct> >::const_iterator
    it(sm.begin());
  shared_ptr<service_struct> svc1(it->second);
  ++it;
  shared_ptr<service_struct> svc2(it->second);
  configuration::service csvc1(csvc);
  ASSERT_EQ(csvc, csvc1);
  ASSERT_TRUE(csvc1.parse("recovery_notification_delay", "120"));
  ASSERT_TRUE(csvc < csvc1);
  ASSERT_TRUE(csvc.parse("retry_interval", "120"));
  ASSERT_TRUE(csvc1 < csvc);

  ASSERT_EQ(svc1, svc1);
  if (strcmp(svc1->description, "test description1") == 0)
    ASSERT_TRUE(svc1.get() < svc2.get());
  else if (strcmp(svc1->description, "test description2") == 0)
    ASSERT_TRUE(svc2.get() < svc1.get());
  else
    FAIL();
}

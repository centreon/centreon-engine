/*
 * Copyright 2019 Centreon (https://www.centreon.com/)
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
#include "../../test_engine.hh"
#include "../../timeperiod/utils.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/host.hh"
#include "com/centreon/engine/configuration/service.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/service.hh"
#include "helper.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

class ApplierService : public TestEngine {
 public:
  void SetUp() override {
    init_config_state();
  }

  void TearDown() override {
    deinit_config_state();
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

// Given host configuration without host_id
// Then the applier add_object throws an exception.
TEST_F(ApplierService, NewHostWithoutHostId) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_THROW(hst_aply.add_object(hst), std::exception);
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
  // The host id is not given
  ASSERT_THROW(hst_aply.add_object(hst), std::exception);
  ASSERT_TRUE(hst.parse("host_id", "1"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_TRUE(svc.parse("host", "test_host"));
  ASSERT_TRUE(svc.parse("service_description", "test description"));
  ASSERT_TRUE(svc.parse("service_id", "3"));

  configuration::applier::command cmd_aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  cmd_aply.add_object(cmd);

  // No need here to call svc_aply.expand_objects(*config) because the
  // configuration service is not stored in configuration::state. We just have
  // to set the host_id manually.
  svc.set_host_id(1);
  svc_aply.add_object(svc);
  service_id_map const& sm(engine::service::services_by_id);
  ASSERT_EQ(sm.size(), 1u);
  ASSERT_EQ(sm.begin()->first.first, 1u);
  ASSERT_EQ(sm.begin()->first.second, 3u);

  // Service is not resolved, host is null now.
  ASSERT_TRUE(!sm.begin()->second->get_host_ptr());
  ASSERT_TRUE(sm.begin()->second->get_description() == "test description");
}

// Given service configuration with a host defined
// Then the applier add_object creates the service
TEST_F(ApplierService, RenameServiceFromConfig) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  // The host id is not given
  ASSERT_THROW(hst_aply.add_object(hst), std::exception);
  ASSERT_TRUE(hst.parse("host_id", "1"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_TRUE(svc.parse("host", "test_host"));
  ASSERT_TRUE(svc.parse("service_description", "test description"));
  ASSERT_TRUE(svc.parse("service_id", "3"));

  configuration::applier::command cmd_aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  cmd_aply.add_object(cmd);

  // We fake here the expand_object on configuration::service
  svc.set_host_id(1);

  svc_aply.add_object(svc);

  ASSERT_TRUE(svc.parse("service_description", "test description2"));
  svc_aply.modify_object(svc);
  svc_aply.expand_objects(*config);

  service_id_map const& sm(engine::service::services_by_id);
  ASSERT_EQ(sm.size(), 1u);
  ASSERT_EQ(sm.begin()->first.first, 1u);
  ASSERT_EQ(sm.begin()->first.second, 3u);

  // Service is not resolved, host is null now.
  ASSERT_TRUE(!sm.begin()->second->get_host_ptr());
  ASSERT_TRUE(sm.begin()->second->get_description() == "test description2");

  std::string s{engine::service::services[{"test_host", "test description2"}]
                    ->get_description()};
  ASSERT_TRUE(s == "test description2");
}

// Given service configuration with a host defined
// Then the applier add_object creates the service
TEST_F(ApplierService, RemoveServiceFromConfig) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  // The host id is not given
  ASSERT_THROW(hst_aply.add_object(hst), std::exception);
  ASSERT_TRUE(hst.parse("host_id", "1"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_TRUE(svc.parse("host", "test_host"));
  ASSERT_TRUE(svc.parse("service_description", "test description"));
  ASSERT_TRUE(svc.parse("service_id", "3"));

  configuration::applier::command cmd_aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  cmd_aply.add_object(cmd);

  // We fake here the expand_object on configuration::service
  svc.set_host_id(1);

  svc_aply.add_object(svc);

  ASSERT_EQ(engine::service::services_by_id.size(), 1u);
  svc_aply.remove_object(svc);
  ASSERT_EQ(engine::service::services_by_id.size(), 0u);

  ASSERT_TRUE(svc.parse("service_description", "test description2"));

  // We have to fake the expand_object on configuration::service
  svc.set_host_id(1);

  svc_aply.add_object(svc);

  service_id_map const& sm(engine::service::services_by_id);
  ASSERT_EQ(sm.size(), 1u);
  ASSERT_EQ(sm.begin()->first.first, 1u);
  ASSERT_EQ(sm.begin()->first.second, 3u);

  // Service is not resolved, host is null now.
  ASSERT_TRUE(!sm.begin()->second->get_host_ptr());
  ASSERT_TRUE(sm.begin()->second->get_description() == "test description2");

  std::string s{engine::service::services[{"test_host", "test description2"}]
                    ->get_description()};
  ASSERT_TRUE(s == "test description2");
}

// Given a service configuration,
// When we duplicate it, we get a configuration equal to the previous one.
// When two services are generated from the same configuration
// Then they are equal.
// When Modifying a configuration changes,
// Then the '!=' effect on configurations.
TEST_F(ApplierService, ServicesEquality) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service csvc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("host_id", "1"));
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

  // We have to fake the expand_object on configuration::service
  csvc.set_host_id(1);

  svc_aply.add_object(csvc);
  ASSERT_TRUE(csvc.parse("service_description", "test description2"));
  ASSERT_THROW(svc_aply.add_object(csvc), std::exception);
  ASSERT_TRUE(csvc.parse("service_id", "12346"));
  ASSERT_NO_THROW(svc_aply.add_object(csvc));
  service_map const& sm(engine::service::services);
  ASSERT_EQ(sm.size(), 2u);
  service_map::const_iterator it(sm.begin());
  std::shared_ptr<com::centreon::engine::service> svc1(it->second);
  ++it;
  std::shared_ptr<com::centreon::engine::service> svc2(it->second);
  configuration::service csvc1(csvc);
  ASSERT_EQ(csvc, csvc1);
  ASSERT_TRUE(csvc1.parse("recovery_notification_delay", "120"));
  ASSERT_TRUE(csvc < csvc1);
  ASSERT_TRUE(csvc.parse("retry_interval", "120"));
  ASSERT_TRUE(csvc1 < csvc);

  ASSERT_TRUE(svc1 != svc2);
}

// Given a service configuration applied to a service,
// When the check_validity() method is executed on the configuration,
// Then it throws an exception because:
//  1. it does not provide a service description
//  2. it is not attached to a host
//  3. the service does not contain any check command.
TEST_F(ApplierService, ServicesCheckValidity) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service csvc;

  // No service description
  ASSERT_THROW(csvc.check_validity(), engine::exceptions::error);

  ASSERT_TRUE(csvc.parse("service_description", "check description"));
  ASSERT_TRUE(csvc.parse("service_id", "53"));

  // No host attached to
  ASSERT_THROW(csvc.check_validity(), engine::exceptions::error);

  ASSERT_TRUE(csvc.parse("hosts", "test_host"));

  // No check command attached to
  ASSERT_THROW(csvc.check_validity(), engine::exceptions::error);

  configuration::applier::command cmd_aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  csvc.parse("check_command", "cmd");
  cmd_aply.add_object(cmd);

  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "10.11.12.13"));
  ASSERT_TRUE(hst.parse("host_id", "124"));
  hst_aply.add_object(hst);

  // We fake here the expand_object on configuration::service
  csvc.set_host_id(124);

  svc_aply.add_object(csvc);
  ASSERT_TRUE(csvc.parse("service_description", "foo"));

  // No check command
  ASSERT_NO_THROW(csvc.check_validity());
  svc_aply.resolve_object(csvc);

  service_map const& sm(engine::service::services);
  ASSERT_EQ(sm.size(), 1u);

  host_map const& hm(engine::host::hosts);
  ASSERT_EQ(sm.begin()->second->get_host_ptr(), hm.begin()->second.get());
}

// Given a service configuration,
// When the flap_detection_options is set to none,
// Then it is well recorded with only none.
TEST_F(ApplierService, ServicesFlapOptionsNone) {
  configuration::service csvc;
  ASSERT_TRUE(csvc.parse("service_description", "test description"));
  ASSERT_TRUE(csvc.parse("hosts", "test_host"));

  csvc.parse("flap_detection_options", "n");
  ASSERT_EQ(csvc.flap_detection_options(), configuration::service::none);
}

// Given a service configuration,
// When the flap_detection_options is set to all,
// Then it is well recorded with all.
TEST_F(ApplierService, ServicesFlapOptionsAll) {
  configuration::service csvc;
  csvc.parse("flap_detection_options", "a");
  ASSERT_EQ(csvc.flap_detection_options(),
            configuration::service::ok | configuration::service::warning |
                configuration::service::critical |
                configuration::service::unknown);
}

// Given a service configuration,
// When the initial_state value is set to unknown,
// Then it is well recorded with unknown.
// When the initial_state value is set to whatever
// Then the parse method returns false.
TEST_F(ApplierService, ServicesInitialState) {
  configuration::service csvc;
  ASSERT_TRUE(csvc.parse("initial_state", "u"));
  ASSERT_EQ(csvc.initial_state(), engine::service::state_unknown);
  ASSERT_FALSE(csvc.parse("initial_state", "g"));
}

// Given a service configuration,
// When the stalking options are set to "c,w",
// Then they are well recorded with "critical | warning"
// When the initial_state value is set to "a"
// Then they are well recorded with "ok | warning | unknown | critical"
TEST_F(ApplierService, ServicesStalkingOptions) {
  configuration::service csvc;
  ASSERT_TRUE(csvc.parse("stalking_options", "c,w"));
  ASSERT_EQ(csvc.stalking_options(),
            configuration::service::critical | configuration::service::warning);

  ASSERT_TRUE(csvc.parse("stalking_options", "a"));
  ASSERT_EQ(csvc.stalking_options(), configuration::service::ok |
                                         configuration::service::warning |
                                         configuration::service::unknown |
                                         configuration::service::critical);
}

// Given a viable contact
// When it is added to a contactgroup
// And when this contactgroup is added to a service
// Then after the service resolution, we can see the contactgroup stored in the
// service with the contact inside it.
TEST_F(ApplierService, ContactgroupResolution) {
  configuration::contact ctct{new_configuration_contact("admin", true)};
  configuration::applier::contact ct_aply;
  ct_aply.add_object(ctct);
  configuration::contactgroup cg;
  cg.parse("contactgroup_name", "contactgroup_test");
  cg.parse("members", "admin");
  configuration::applier::contactgroup cg_aply;
  cg_aply.add_object(cg);
  cg_aply.resolve_object(cg);
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  hst.parse("host_id", "1");
  hst_aply.add_object(hst);
  svc.parse("host", "test_host");
  svc.parse("service_description", "test description");
  svc.parse("service_id", "3");
  svc.parse("contact_groups", "contactgroup_test");

  configuration::applier::command cmd_aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  cmd_aply.add_object(cmd);

  // We fake here the expand_object on configuration::service
  svc.set_host_id(1);

  svc_aply.add_object(svc);
  svc_aply.resolve_object(svc);
  service_id_map const& sm(engine::service::services_by_id);
  ASSERT_EQ(sm.size(), 1u);
  ASSERT_EQ(sm.begin()->first.first, 1u);
  ASSERT_EQ(sm.begin()->first.second, 3u);

  contactgroup_map_unsafe cgs{sm.begin()->second->get_contactgroups()};
  ASSERT_EQ(cgs.size(), 1u);
  ASSERT_EQ(cgs.begin()->first, "contactgroup_test");
  contact_map_unsafe::iterator itt{
      cgs.begin()->second->get_members().find("admin")};

  ASSERT_NE(itt, cgs.begin()->second->get_members().end());

  contact_map::const_iterator it{engine::contact::contacts.find("admin")};
  ASSERT_NE(it, engine::contact::contacts.end());

  ASSERT_EQ(itt->second, it->second.get());
}

TEST_F(ApplierService, StalkingOptionsWhenServiceIsModified) {
  configuration::applier::host hst_aply;
  configuration::applier::service svc_aply;
  configuration::service svc;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  // The host id is not given
  ASSERT_THROW(hst_aply.add_object(hst), std::exception);
  ASSERT_TRUE(hst.parse("host_id", "1"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_TRUE(svc.parse("host", "test_host"));
  ASSERT_TRUE(svc.parse("service_description", "test description"));
  ASSERT_TRUE(svc.parse("service_id", "3"));

  configuration::applier::command cmd_aply;
  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 1");
  svc.parse("check_command", "cmd");
  cmd_aply.add_object(cmd);
  svc.parse("stalking_options", "");
  svc.parse("notification_options", "a");

  // We fake here the expand_object on configuration::service
  svc.set_host_id(1);

  svc_aply.add_object(svc);

  service_id_map const& sm(engine::service::services_by_id);
  std::shared_ptr<engine::service> serv = sm.begin()->second;

  ASSERT_FALSE(serv->get_stalk_on(engine::service::ok));
  ASSERT_FALSE(serv->get_stalk_on(engine::service::warning));
  ASSERT_FALSE(serv->get_stalk_on(engine::service::critical));
  ASSERT_FALSE(serv->get_stalk_on(engine::service::unknown));

  ASSERT_TRUE(serv->get_notify_on(engine::service::ok));
  ASSERT_TRUE(serv->get_notify_on(engine::service::warning));
  ASSERT_TRUE(serv->get_notify_on(engine::service::critical));
  ASSERT_TRUE(serv->get_notify_on(engine::service::unknown));

  ASSERT_TRUE(svc.parse("service_description", "test description2"));
  svc_aply.modify_object(svc);
  svc_aply.expand_objects(*config);

  ASSERT_EQ(sm.size(), 1u);
  ASSERT_EQ(sm.begin()->first.first, 1u);
  ASSERT_EQ(sm.begin()->first.second, 3u);

  // Service is not resolved, host is null now.
  serv = sm.begin()->second;

  ASSERT_TRUE(!serv->get_host_ptr());
  ASSERT_TRUE(serv->get_description() == "test description2");

  std::string s{engine::service::services[{"test_host", "test description2"}]
                    ->get_description()};
  ASSERT_TRUE(s == "test description2");

  ASSERT_FALSE(serv->get_stalk_on(engine::service::ok));
  ASSERT_FALSE(serv->get_stalk_on(engine::service::warning));
  ASSERT_FALSE(serv->get_stalk_on(engine::service::critical));
  ASSERT_FALSE(serv->get_stalk_on(engine::service::unknown));

  ASSERT_TRUE(serv->get_notify_on(engine::service::ok));
  ASSERT_TRUE(serv->get_notify_on(engine::service::warning));
  ASSERT_TRUE(serv->get_notify_on(engine::service::critical));
  ASSERT_TRUE(serv->get_notify_on(engine::service::unknown));
}

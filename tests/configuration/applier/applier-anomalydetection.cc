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
#include "com/centreon/engine/configuration/applier/anomalydetection.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/anomalydetection.hh"
#include "helper.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::configuration::applier;

class ApplierAnomalydetection : public TestEngine {
 public:
  void SetUp() override {
    init_config_state();
  }

  void TearDown() override {
    deinit_config_state();
  }
};

// Given an AD configuration with a host not defined
// Then the applier add_object throws an exception because it needs a service
// command.
TEST_F(ApplierAnomalydetection, NewAnomalydetectionWithHostNotDefinedFromConfig) {
  configuration::applier::anomalydetection ad_aply;
  configuration::anomalydetection ad;
  ASSERT_TRUE(ad.parse("host_name", "test_host"));
  ASSERT_TRUE(ad.parse("service_description", "test description"));
  ASSERT_THROW(ad_aply.add_object(ad), std::exception);
}

// Given host configuration without host_id
// Then the applier add_object throws an exception.
TEST_F(ApplierAnomalydetection, NewHostWithoutHostId) {
  configuration::applier::host hst_aply;
  configuration::applier::service ad_aply;
  configuration::anomalydetection ad;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_THROW(hst_aply.add_object(hst), std::exception);
}

// Given service configuration with a host defined
// Then the applier add_object creates the service
TEST_F(ApplierAnomalydetection, NewADFromConfig) {
  configuration::applier::host hst_aply;
  configuration::applier::anomalydetection ad_aply;
  configuration::anomalydetection ad;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  // The host id is not given
  ASSERT_THROW(hst_aply.add_object(hst), std::exception);
  ASSERT_TRUE(hst.parse("host_id", "12"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));

  configuration::applier::service svc_aply;
  configuration::service svc;
  svc.parse("host_name", "test_host");
  svc.parse("description", "test_description");
  svc.parse("_HOST_ID", "12");
  svc.parse("_SERVICE_ID", "13");

  // We fake here the expand_object on configuration::service
  svc.set_host_id(12);

  configuration::command cmd("cmd");
  cmd.parse("command_line", "echo 'output| metric=12;50;75'");
  svc.parse("check_command", "cmd");
  configuration::applier::command cmd_aply;
  cmd_aply.add_object(cmd);
  ASSERT_NO_THROW(svc_aply.add_object(svc));

  ASSERT_TRUE(ad.parse("service_description", "test description"));
  ASSERT_TRUE(ad.parse("dependent_service_id", "13"));
  ASSERT_TRUE(ad.parse("service_id", "4"));
  ASSERT_TRUE(ad.parse("host_id", "12"));
  ASSERT_TRUE(ad.parse("host_name", "test_host"));
  ASSERT_TRUE(ad.parse("metric_name", "foo"));
  ASSERT_TRUE(ad.parse("thresholds_file", "/etc/centreon-broker/thresholds.json"));

  // No need here to call ad_aply.expand_objects(*config) because the
  // configuration service is not stored in configuration::state. We just have
  // to set the host_id manually.
  ad_aply.add_object(ad);
  service_id_map const& sm(engine::service::services_by_id);
  ASSERT_EQ(sm.size(), 2u);
  ASSERT_EQ(sm.begin()->first.first, 12u);
  ASSERT_EQ(sm.begin()->first.second, 4u);

  // Service is not resolved, host is null now.
  ASSERT_TRUE(!sm.begin()->second->get_host_ptr());
  ASSERT_TRUE(sm.begin()->second->get_description() == "test description");
}

// Given service configuration without service_id
// Then the applier add_object throws an exception
TEST_F(ApplierAnomalydetection, NewADNoServiceId) {
  configuration::applier::host hst_aply;
  configuration::applier::anomalydetection ad_aply;
  configuration::anomalydetection ad;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  // The host id is not given
  ASSERT_THROW(hst_aply.add_object(hst), std::exception);
  ASSERT_TRUE(hst.parse("host_id", "1"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_TRUE(ad.parse("service_description", "test description"));
  ASSERT_TRUE(ad.parse("host_id", "1"));
  ASSERT_TRUE(ad.parse("host_name", "test_host"));

  // No need here to call ad_aply.expand_objects(*config) because the
  // configuration service is not stored in configuration::state. We just have
  // to set the host_id manually.
  ASSERT_THROW(ad_aply.add_object(ad), std::exception);
}

// Given service configuration without host_id
// Then the applier add_object throws an exception
TEST_F(ApplierAnomalydetection, NewADNoHostId) {
  configuration::applier::host hst_aply;
  configuration::applier::anomalydetection ad_aply;
  configuration::anomalydetection ad;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("host_id", "1"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_TRUE(ad.parse("service_description", "test description"));
  ASSERT_TRUE(ad.parse("service_id", "4"));
  ASSERT_TRUE(ad.parse("host_name", "test_host"));

  ASSERT_THROW(ad_aply.add_object(ad), std::exception);
}

// Given service configuration with bad host_id
// Then the applier add_object throws an exception
TEST_F(ApplierAnomalydetection, NewADBadHostId) {
  configuration::applier::host hst_aply;
  configuration::applier::anomalydetection ad_aply;
  configuration::anomalydetection ad;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("host_id", "1"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_TRUE(ad.parse("service_description", "test description"));
  ASSERT_TRUE(ad.parse("host_id", "2"));
  ASSERT_TRUE(ad.parse("service_id", "4"));
  ASSERT_TRUE(ad.parse("dependent_service_id", "3"));
  ASSERT_TRUE(ad.parse("host_name", "test_host"));

  // No need here to call ad_aply.expand_objects(*config) because the
  // configuration service is not stored in configuration::state. We just have
  // to set the host_id manually.
  ASSERT_THROW(ad_aply.add_object(ad), std::exception);
}

// Given service configuration without metric_name
// Then the applier add_object throws an exception
TEST_F(ApplierAnomalydetection, NewADNoMetric) {
  configuration::applier::host hst_aply;
  configuration::applier::anomalydetection ad_aply;
  configuration::anomalydetection ad;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("host_id", "1"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_TRUE(ad.parse("service_description", "test description"));
  ASSERT_TRUE(ad.parse("host_id", "1"));
  ASSERT_TRUE(ad.parse("service_id", "4"));
  ASSERT_TRUE(ad.parse("dependent_service_id", "3"));
  ASSERT_TRUE(ad.parse("host_name", "test_host"));

  // No need here to call ad_aply.expand_objects(*config) because the
  // configuration service is not stored in configuration::state. We just have
  // to set the host_id manually.
  ASSERT_THROW(ad_aply.add_object(ad), std::exception);
}

// Given service configuration without metric_name
// Then the applier add_object throws an exception
TEST_F(ApplierAnomalydetection, NewADNoThresholds) {
  configuration::applier::host hst_aply;
  configuration::applier::anomalydetection ad_aply;
  configuration::anomalydetection ad;
  configuration::host hst;
  ASSERT_TRUE(hst.parse("host_name", "test_host"));
  ASSERT_TRUE(hst.parse("address", "127.0.0.1"));
  ASSERT_TRUE(hst.parse("host_id", "1"));
  ASSERT_NO_THROW(hst_aply.add_object(hst));
  ASSERT_TRUE(ad.parse("service_description", "test description"));
  ASSERT_TRUE(ad.parse("host_id", "1"));
  ASSERT_TRUE(ad.parse("service_id", "4"));
  ASSERT_TRUE(ad.parse("dependent_service_id", "3"));
  ASSERT_TRUE(ad.parse("host_name", "test_host"));
  ASSERT_TRUE(ad.parse("metric_name", "bar"));

  // No need here to call ad_aply.expand_objects(*config) because the
  // configuration service is not stored in configuration::state. We just have
  // to set the host_id manually.
  ASSERT_THROW(ad_aply.add_object(ad), std::exception);
}

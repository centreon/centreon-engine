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

#include "enginerpc.hh"

#include <gtest/gtest.h>

#include <cstdio>

#include "../test_engine.hh"
#include "com/centreon/engine/anomalydetection.hh"
#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/command_manager.hh"
#include "com/centreon/engine/configuration/applier/anomalydetection.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/host.hh"
#include "com/centreon/engine/configuration/applier/service.hh"
#include "com/centreon/engine/configuration/contact.hh"
#include "com/centreon/engine/events/loop.hh"
#include "com/centreon/engine/timezone_manager.hh"
#include "com/centreon/engine/version.hh"
#include "helper.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

class EngineRpc : public TestEngine {
 public:
  void SetUp() override {
    init_config_state();

    // Do not unload this in the tear down function, it is done by the
    // other unload function... :-(
    configuration::applier::contact ct_aply;
    configuration::contact ctct{new_configuration_contact("admin", true)};
    ct_aply.add_object(ctct);
    ct_aply.expand_objects(*config);
    ct_aply.resolve_object(ctct);

    configuration::host hst{new_configuration_host("test_host", "admin")};
    configuration::applier::host hst_aply;
    hst_aply.add_object(hst);

    configuration::service svc{
        new_configuration_service("test_host", "test_svc", "admin")};
    configuration::applier::service svc_aply;
    svc_aply.add_object(svc);

    hst_aply.resolve_object(hst);
    svc_aply.resolve_object(svc);

    configuration::anomalydetection ad{new_configuration_anomalydetection(
        "test_host", "test_ad", "admin",
        12,  // service_id of the anomalydetection
        13,  // service_id of the dependent service
        "/tmp/thresholds_status_change.json")};
    configuration::applier::anomalydetection ad_aply;
    ad_aply.add_object(ad);

    ad_aply.resolve_object(ad);

    host_map const& hm{engine::host::hosts};
    _host = hm.begin()->second;
    _host->set_current_state(engine::host::state_up);
    _host->set_state_type(checkable::hard);
    _host->set_problem_has_been_acknowledged(false);
    _host->set_notify_on(static_cast<uint32_t>(-1));

    service_map const& sm{engine::service::services};
    for (auto& p : sm) {
      std::shared_ptr<engine::service> svc = p.second;
      if (svc->get_service_id() == 12)
        _ad = std::static_pointer_cast<engine::anomalydetection>(svc);
      else
        _svc = svc;
    }
  }

  void TearDown() override {
    _host.reset();
    _svc.reset();
    _ad.reset();
    deinit_config_state();
  }

  std::list<std::string> execute(const std::string& command) {
    std::list<std::string> retval;
    char path[1024];
    std::ostringstream oss;
    oss << "tests/rpc_client " << command;

    FILE* fp = popen(oss.str().c_str(), "r");
    while (fgets(path, sizeof(path), fp) != nullptr) {
      size_t count = strlen(path);
      if (count > 0)
        --count;
      retval.push_back(std::string(path, count));
    }
    pclose(fp);
    return retval;
  }

  void CreateFile(std::string const& filename, std::string const& content) {
    std::ofstream oss(filename);
    oss << content;
  }

 protected:
  std::shared_ptr<engine::host> _host;
  std::shared_ptr<engine::service> _svc;
  std::shared_ptr<engine::anomalydetection> _ad;
};

TEST_F(EngineRpc, StartStop) {
  enginerpc erpc("0.0.0.0", 40001);
  ASSERT_NO_THROW(erpc.shutdown());
}

TEST_F(EngineRpc, GetVersion) {
  std::ostringstream oss;
  oss << "GetVersion: major: " << CENTREON_ENGINE_VERSION_MAJOR;
  enginerpc erpc("0.0.0.0", 40001);
  auto output = execute("GetVersion");
  //ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output.front(), oss.str());
  if (output.size() == 2) {
    oss.str("");
    oss << "minor: " << CENTREON_ENGINE_VERSION_MINOR;
    ASSERT_EQ(output.back(), oss.str());
  } else {
    oss.str("");
    oss << "patch: " << CENTREON_ENGINE_VERSION_PATCH;
    ASSERT_EQ(output.back(), oss.str());
  }
  erpc.shutdown();
}

TEST_F(EngineRpc, ProcessServiceCheckResult) {
  enginerpc erpc("0.0.0.0", 40001);
  auto output = execute("ProcessServiceCheckResult test_host test_svc 0");
  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output.front(), "ProcessServiceCheckResult: 0");
  erpc.shutdown();
}

TEST_F(EngineRpc, ProcessServiceCheckResultBadHost) {
  enginerpc erpc("0.0.0.0", 40001);
  auto output = execute("ProcessServiceCheckResult \"\" test_svc 0");
  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output.front(), "ProcessServiceCheckResult failed.");
  erpc.shutdown();
}

TEST_F(EngineRpc, ProcessServiceCheckResultBadService) {
  enginerpc erpc("0.0.0.0", 40001);
  auto output = execute("ProcessServiceCheckResult test_host \"\" 0");
  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output.front(), "ProcessServiceCheckResult failed.");
  erpc.shutdown();
}

TEST_F(EngineRpc, ProcessHostCheckResult) {
  enginerpc erpc("0.0.0.0", 40001);
  auto output = execute("ProcessHostCheckResult test_host 0");
  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output.front(), "ProcessHostCheckResult: 0");
  erpc.shutdown();
}

TEST_F(EngineRpc, ProcessHostCheckResultBadHost) {
  enginerpc erpc("0.0.0.0", 40001);
  auto output = execute("ProcessHostCheckResult '' 0");
  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output.front(), "ProcessHostCheckResult failed.");
  erpc.shutdown();
}

TEST_F(EngineRpc, NewThresholdsFile) {
  CreateFile(
      "/tmp/thresholds_file.json",
      "[{\n \"host_id\": \"12\",\n \"service_id\": \"12\",\n \"metric_name\": "
      "\"metric\",\n \"predict\": [{\n \"timestamp\": 50000,\n \"upper\": "
      "84,\n \"lower\": 74,\n \"fit\": 79\n }, {\n \"timestamp\": 100000,\n "
      "\"upper\": 10,\n \"lower\": 5,\n \"fit\": 51.5\n }, {\n \"timestamp\": "
      "150000,\n \"upper\": 100,\n \"lower\": 93,\n \"fit\": 96.5\n }, {\n "
      "\"timestamp\": 200000,\n \"upper\": 100,\n \"lower\": 97,\n \"fit\": "
      "98.5\n }, {\n \"timestamp\": 250000,\n \"upper\": 100,\n \"lower\": "
      "21,\n \"fit\": 60.5\n }\n]}]");
  enginerpc erpc("0.0.0.0", 40001);
  auto output = execute("NewThresholdsFile /tmp/thresholds_file.json");
  ASSERT_EQ(output.size(), 1);
  ASSERT_EQ(output.front(), "NewThresholdsFile: 0");
  command_manager::instance().execute();
  ASSERT_EQ(_ad->get_thresholds_file(), "/tmp/thresholds_file.json");
}

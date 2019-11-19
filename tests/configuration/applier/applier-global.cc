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

#include <fstream>
#include <gtest/gtest.h>
#include <com/centreon/clib.hh>
#include <com/centreon/engine/checks/checker.hh>
#include <com/centreon/engine/configuration/parser.hh>
#include <com/centreon/engine/configuration/applier/host.hh>
#include <com/centreon/engine/configuration/applier/hostescalation.hh>
#include <com/centreon/engine/configuration/applier/state.hh>
#include <com/centreon/engine/configuration/state.hh>
#include <com/centreon/engine/host.hh>
#include <com/centreon/engine/hostescalation.hh>
#include <com/centreon/engine/timezone_manager.hh>
#include <com/centreon/logging/engine.hh>

using namespace com::centreon;
using namespace com::centreon::engine;

extern configuration::state* config;

class ApplierGlobal : public ::testing::Test {
 public:
  void SetUp() override {
    clib::load();
    com::centreon::logging::engine::load();
    if (config == NULL)
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
TEST_F(ApplierGlobal, pollerName) {
  configuration::parser parser;
  configuration::state st;

  ASSERT_EQ(st.poller_name(), "unknown");

  std::remove("/tmp/test-config.cfg");

  std::ofstream ofs("/tmp/test-config.cfg");
  ofs << "poller_name=poller-test" << std::endl;
  ofs.close();

  parser.parse("/tmp/test-config.cfg", st);
  std::remove("/tmp/test-config.cfg");

  ASSERT_EQ(st.poller_name(), "poller-test");
}
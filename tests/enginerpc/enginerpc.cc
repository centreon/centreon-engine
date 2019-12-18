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
#include <cstdio>
#include "enginerpc.hh"
#include "com/centreon/clib.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/version.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

extern configuration::state* config;

class EngineRpc : public testing::Test {
 public:
  void SetUp() override {
    clib::load();
    com::centreon::logging::engine::load();
    if (!config)
      config = new configuration::state;
  }

  void TearDown() override {
    delete config;
    config = nullptr;
    com::centreon::logging::engine::unload();
    clib::unload();
  }

  std::list<std::string> execute(const std::string& command) {
    std::list<std::string> retval;
    char path[1024];
    std::ostringstream oss;
    oss << "tests/rpc_client "
      << command;

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

};

TEST_F(EngineRpc, StartStop) {
  enginerpc erpc("0.0.0.0", 50051);
  ASSERT_NO_THROW(erpc.shutdown());
}

TEST_F(EngineRpc, GetVersion) {
  std::ostringstream oss;
  oss << "GetVersion: major: " << CENTREON_ENGINE_VERSION_MAJOR;
  enginerpc erpc("0.0.0.0", 50051);
  auto output = execute("GetVersion");
  ASSERT_EQ(output.size(), 2);
  ASSERT_EQ(output.front(), oss.str());
  oss.str("");
  oss << "minor: " << CENTREON_ENGINE_VERSION_MINOR;
  ASSERT_EQ(output.back(), oss.str());
  erpc.shutdown();
}

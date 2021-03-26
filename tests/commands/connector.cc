/*
 * Copyright 2019-2021 Centreon (https://www.centreon.com/)
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

#include "com/centreon/engine/commands/connector.hh"
#include <gtest/gtest.h>
#include <signal.h>
#include <condition_variable>
#include <mutex>
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/commands/forward.hh"
#include "com/centreon/process_manager.hh"
#include "helper.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

class my_listener : public commands::command_listener {
 public:
  result const& get_result() const {
    std::lock_guard<std::mutex> guard(_mutex);
    return _res;
  }

  void finished(result const& res) throw() override {
    std::lock_guard<std::mutex> guard(_mutex);
    _res = res;
  }

  void clear() {
    _res.command_id = 0;
    _res.output = "";
  }

 private:
  mutable std::mutex _mutex;
  commands::result _res;
};

class Connector : public ::testing::Test {
 public:
  void SetUp() override {
    signal(SIGPIPE, SIG_IGN);
    init_config_state();
  }

  void TearDown() override {
    deinit_config_state();
  }
};


// Given an empty name
// When the add_command method is called with it as argument,
// Then it returns a NULL pointer.
TEST_F(Connector, NewConnector) {
  ASSERT_THROW(new commands::connector("", "bar"), std::exception);
}

TEST_F(Connector, ForwardWithoutName) {
  auto c = std::make_shared<commands::connector>(
      "test segfault", "tests/bin_connector_test_run --kill=2");
  ASSERT_THROW(new commands::forward("", "bar", c), std::exception);
}

TEST_F(Connector, ForwardWithoutCmd) {
  auto c = std::make_shared<commands::connector>(
      "test segfault", "tests/bin_connector_test_run --kill=2");
  ASSERT_THROW(new commands::forward("foo", "", c), std::exception);
}

// Given an already existing command
// When the add_command method is called with the same name
// Then it returns a NULL pointer.
TEST_F(Connector, SimpleConnector) {
  commands::connector c("toto", "/bin/ls");
}

// This test is just a test of the run command in usual conditions.
// We don't test timeout because time is replaced by a fake function
// and we don't control it during the execution of run().
TEST_F(Connector, RunWithTimeout) {
  nagios_macros macros = nagios_macros();
  connector cmd_connector("RunWithTimeout", "tests/bin_connector_test_run");
  result res;
  cmd_connector.run("commande --timeout=on", macros, 1, res);

  ASSERT_TRUE(res.command_id != 0);
}

TEST_F(Connector, RunConnectorAsync) {
  std::unique_ptr<my_listener> lstnr(new my_listener);
  nagios_macros macros = nagios_macros();
  connector cmd_connector("RunConnectorAsync", "tests/bin_connector_test_run");
  cmd_connector.set_listener(lstnr.get());
  cmd_connector.run("commande", macros, 1);

  int timeout = 0;
  int max_timeout{15};
  while (timeout < max_timeout && lstnr->get_result().output == "") {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    set_time(std::time(nullptr) + 1);
    ++timeout;
  }
  result res{lstnr->get_result()};
  ASSERT_NE(res.command_id, 0);
  ASSERT_EQ(res.output, "commande");
}

TEST_F(Connector, RunWithConnectorSwitchedOff) {
  connector cmd_connector("RunWithConnectorSwitchedOff",
                          "tests/bin_connector_test_run");
  {
    std::unique_ptr<my_listener> lstnr(new my_listener);
    nagios_macros macros = nagios_macros();
    cmd_connector.set_listener(lstnr.get());
    cmd_connector.run("commande --kill=1", macros, 1);
  
    int timeout = 0;
    int max_timeout{15};
    while (timeout < max_timeout && lstnr->get_result().output == "") {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      set_time(std::time(nullptr) + 1);
      ++timeout;
    }
    result res{lstnr->get_result()};
    ASSERT_EQ(res.command_id, 0);
    ASSERT_EQ(res.output, "");
  }
}

TEST_F(Connector, RunConnectorSetCommandLine) {
  my_listener lstnr;
  nagios_macros macros = nagios_macros();
  connector cmd_connector("SetCommandLine", "tests/bin_connector_test_run");
  cmd_connector.set_listener(&lstnr);
  cmd_connector.run("commande1", macros, 1);

  int timeout = 0;
  int max_timeout{15};
  while (timeout < max_timeout && lstnr.get_result().output == "") {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    set_time(std::time(nullptr) + 1);
    ++timeout;
  }
  result res{lstnr.get_result()};
  ASSERT_NE(res.command_id, 0);
  ASSERT_EQ(res.output, "commande1");

  lstnr.clear();
  cmd_connector.set_command_line("tests/bin_connector_test_run");
  cmd_connector.run("commande2", macros, 1);

  timeout = 0;
  max_timeout = 15;
  while (timeout < max_timeout && lstnr.get_result().output == "") {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    set_time(std::time(nullptr) + 1);
    ++timeout;
  }
  res = lstnr.get_result();
  ASSERT_NE(res.command_id, 0);
  ASSERT_EQ(res.output, "commande2");
}

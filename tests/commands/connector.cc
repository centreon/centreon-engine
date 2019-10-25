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

#include "com/centreon/engine/commands/connector.hh"
#include <gtest/gtest.h>
#include <condition_variable>
#include <memory>
#include <mutex>
#include "../timeperiod/utils.hh"
#include "com/centreon/clib.hh"
#include "com/centreon/engine/commands/forward.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/service.hh"
#include "com/centreon/process_manager.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

extern configuration::state* config;

class Connector : public ::testing::Test {
 public:
  void SetUp() override {
    clib::load();
    com::centreon::logging::engine::load();
    configuration::applier::state::load();  // Needed to store commands
                                            //    set_time(20);
    if (config == NULL)
      config = new configuration::state;
  }

  void TearDown() override {
    configuration::applier::state::unload();  // Needed to store commands
    delete config;
    config = NULL;
    com::centreon::logging::engine::unload();
    clib::unload();
  }
};

class wait_process : public commands::command_listener {
  mutable std::mutex _mutex;
  mutable std::condition_variable _condvar;
  commands::result _res;
  command* _cmd;

 public:
  wait_process(command* cmd) : _cmd{cmd} { _cmd->set_listener(this); }

  wait_process(wait_process const&) = delete;
  ~wait_process() = default;

  result const& get_result() const {
    std::lock_guard<std::mutex> guard(_mutex);
    return _res;
  }

  void wait() const noexcept {
    std::unique_lock<std::mutex> lock(_mutex);
    _condvar.wait(lock, [this] { return _res.command_id != 0; });
  }

  void finished(result const& res) throw() override {
    std::cout << "WAIT LISTENER FINISHED\n";
    std::lock_guard<std::mutex> guard(_mutex);
    _res = res;
    _cmd->set_listener(nullptr);
    _condvar.notify_all();
  }
};

// Given an empty name
// When the add_command method is called with it as argument,
// Then it returns a NULL pointer.
TEST_F(Connector, NewConnector) {
  ASSERT_THROW(new commands::connector("", "bar"), std::exception);
}

// Given an already existing command
// When the add_command method is called with the same name
// Then it returns a NULL pointer.
TEST_F(Connector, SimpleConnector) {
  commands::connector c("toto", "/bin/ls");
}

// Given a connector that segfault.
// Then engine does not crash and executes the next check.
TEST_F(Connector, NewConnectorSync) {
  nagios_macros macros = nagios_macros();
  commands::connector c("test segfault",
                        "tests/bin_connector_test_run --kill=2");
  commands::forward f("after segfault",
                      "test/bin_connector_test_run --timeout=on", c);
  wait_process w(&c);
  unsigned long id = f.run(f.get_command_line(), macros, 0);
  w.wait();

  result const& res{w.get_result()};
  ASSERT_EQ(res.command_id, id);
  ASSERT_EQ(res.exit_code, 0);
  ASSERT_EQ(res.output, f.get_command_line());
  ASSERT_EQ(res.exit_status, process::normal);
}

TEST_F(Connector, RunWithTimeout) {
  nagios_macros macros = nagios_macros();
  connector cmd_connector(
      "RunWithTimeout",
      "tests/bin_connector_test_run");
  forward cmd_forward("RunWithTimeout",
      "tests/bin_connector_test_run --timeout=on", cmd_connector);

  result res;
  cmd_forward.run(cmd_forward.get_command_line(), macros, 1, res);

  ASSERT_TRUE(res.command_id != 0);
  ASSERT_TRUE(res.exit_code == engine::service::state_unknown);
  ASSERT_EQ(res.output, "(Process Timeout)");
  ASSERT_TRUE(res.exit_status == process::timeout);
}

TEST_F(Connector, RunWithoutTimeout) {
  nagios_macros macros = nagios_macros();
  connector cmd_connector(
              "RunWithoutTimeout",
              "tests/bin_connector_test_run");
  forward cmd_forward(
            "RunWithoutTimeout",
            "tests/bin_connector_test_run --timeout=off",
            cmd_connector);

  result res;
  cmd_forward.run(cmd_forward.get_command_line(), macros, 0, res);

  ASSERT_TRUE(res.command_id != 0);
  ASSERT_TRUE(res.exit_code == engine::service::state_ok);
  ASSERT_EQ(res.output, cmd_forward.get_command_line());
  ASSERT_TRUE(res.exit_status == process::normal);
}

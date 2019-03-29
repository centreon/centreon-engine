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
#include "../timeperiod/utils.hh"
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/objects/command.hh"
#include "com/centreon/process_manager.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

extern configuration::state* config;

class SimpleCommand : public ::testing::Test {
 public:
  void SetUp() {
//    set_time(20);
    if (config == NULL)
      config = new configuration::state;
    process_manager::load();
    configuration::applier::state::load();  // Needed to store commands
    set::load();
  }

  void TearDown() {
    configuration::applier::state::unload();  // Needed to store commands
    process_manager::unload();
    delete config;
    config = NULL;
  }
};

class my_listener : public commands::command_listener {
 public:
  result const& get_result() const {
    return (_res);
  }

  void finished(result const& res) throw () {
    _res = res;
  }

 private:
  commands::result _res;
};

// Given an empty name
// When the add_command method is called with it as argument,
// Then it returns a NULL pointer.
TEST_F(SimpleCommand, NewCommandWithNoName) {
  ASSERT_TRUE(add_command("", "bar") == NULL);
}

// Given a command to store,
// When the add_command method is called with an empty value,
// Then it returns a NULL pointer.
TEST_F(SimpleCommand, NewCommandWithNoValue) {
  ASSERT_TRUE(add_command("foo", "") == NULL);
}

// Given an already existing command
// When the add_command method is called with the same name
// Then it returns a NULL pointer.
TEST_F(SimpleCommand, CommandAlreadyExisting) {
  add_command("toto", "/bin/ls");
  ASSERT_TRUE(add_command("toto", "/bin/ls") == NULL);
}

// Given a name and a command line
// When the add_command method is called
// Then a new raw command is built
// When sync executed
// Then we have the output in the result class.
TEST_F(SimpleCommand, NewCommandSync) {
  shared_ptr<commands::raw> cmd(new raw("test", "/bin/echo bonjour"));
  set::instance().add_command(cmd);

  nagios_macros mac;
  memset(&mac, 0, sizeof(mac));
  commands::result res;
  std::string cc(cmd->process_cmd(&mac));
  ASSERT_EQ(cc, "/bin/echo bonjour");
  cmd->run(cc, mac, 2, res);
  ASSERT_EQ(res.output, "bonjour\n");
}

// Given a name and a command line
// When the add_command method is called
// Then a new raw command is built
// When async executed
// Then we have the output in the result class.
TEST_F(SimpleCommand, NewCommandAsync) {
  std::auto_ptr<my_listener> lstnr(new my_listener);
  shared_ptr<commands::raw> cmd(new raw("test", "/bin/echo bonjour"));
  cmd->set_listener(lstnr.get());
  set::instance().add_command(cmd);

  nagios_macros mac;
  memset(&mac, 0, sizeof(mac));
  commands::result res;
  std::string cc(cmd->process_cmd(&mac));
  ASSERT_EQ(cc, "/bin/echo bonjour");

  unsigned long id(cmd->run(cc, mac, 2));
  int timeout(0);
  while (timeout < 10 && lstnr->get_result().output == "") {
    usleep(100000);
    ++timeout;
  }
  ASSERT_TRUE(timeout < 10);
  ASSERT_EQ(lstnr->get_result().output, "bonjour\n");
}

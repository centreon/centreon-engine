/*
** Copyright 2016 Centreon
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

#include <gtest/gtest.h>
#include "com/centreon/clib.hh"

class CentreonEngineEnvironment : public testing::Environment {
 public:
  void SetUp() override {
    setenv("TZ", ":Europe/Paris", 1);
    return;
  }

  void TearDown() override {
    return;
  }
};

/**
 *  Tester entry point.
 *
 *  @param[in] argc  Argument count.
 *  @param[in] argv  Argument values.
 *
 *  @return 0 on success, any other value on failure.
 */
int main(int argc, char* argv[]) {
  // GTest initialization.
  testing::InitGoogleTest(&argc, argv);

  // Set specific environment.
  testing::AddGlobalTestEnvironment(new CentreonEngineEnvironment());

  // Run all tests.
  return (RUN_ALL_TESTS());
}

/*
** Copyright 2011-2013 Merethis
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

#include "com/centreon/engine/commands/environment.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

// External environment array.
extern "C" char** environ;

/**
 *  Check build environment with reference array.
 *
 *  @param[in] ref  The reference array.
 *  @param[in] env  The environement to check.
 *
 *  @return True if is the same, otherwise false.
 */
static bool check_env(std::vector<std::string> const& ref, char** env) {
  unsigned int i(0);
  while (i < ref.size() && env[i]) {
    if (ref[i] != env[i])
      return (false);
    ++i;
  }
  if (ref.size() != i || env[i])
    return (false);
  return (true);
}

/**
 *  Test add line on char type.
 */
static void add_by_key_value_char() {
  unsigned int size(0);
  std::vector<std::string> ref;
  environment env;
  for (unsigned int i(0); size < 1024 * 8; ++i) {
    std::ostringstream name;
    name << "key_" << i;
    std::ostringstream value;
    value << "value_" << i;

    std::string line(name.str() + "=" + value.str());
    ref.push_back(line);
    env.add(name.str().c_str(), value.str().c_str());
    size += line.size();
  }
  if (!check_env(ref, env.data()))
    throw(engine_error() << "add line char failed");
}

/**
 *  Test add line on string type.
 */
static void add_by_key_value_string() {
  unsigned int size(0);
  std::vector<std::string> ref;
  environment env;
  for (unsigned int i(0); size < 1024 * 8; ++i) {
    std::ostringstream name;
    name << "key_" << i;
    std::ostringstream value;
    value << "value_" << i;

    std::string line(name.str() + "=" + value.str());
    ref.push_back(line);
    env.add(name.str(), value.str());
    size += line.size();
  }
  if (!check_env(ref, env.data()))
    throw(engine_error() << "add line string failed");
}

/**
 *  Test add line on char type.
 */
static void add_by_lines_char() {
  unsigned int size(0);
  std::vector<std::string> ref;
  environment env;
  for (unsigned int i(0); size < 1024 * 8; ++i) {
    std::ostringstream oss;
    oss << "key_" << i << "="
        << "value_" << i;
    ref.push_back(oss.str());
    env.add(oss.str().c_str());
    size += oss.str().size();
  }
  if (!check_env(ref, env.data()))
    throw(engine_error() << "add line char failed");
}

/**
 *  Test add line on string type.
 */
static void add_by_lines_string() {
  unsigned int size(0);
  std::vector<std::string> ref;
  environment env;
  for (unsigned int i(0); size < 1024 * 8; ++i) {
    std::ostringstream oss;
    oss << "key_" << i << "="
        << "value_" << i;
    ref.push_back(oss.str());
    env.add(oss.str());
    size += oss.str().size();
  }
  if (!check_env(ref, env.data()))
    throw(engine_error() << "add line string failed");
}

/**
 *  Check environment add.
 */
static int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  add_by_lines_char();
  add_by_lines_string();
  add_by_key_value_char();
  add_by_key_value_string();

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

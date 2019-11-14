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

#include <cstdio>
#include <exception>
#include <fstream>
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/io/file_stream.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

/**
 *  Check parse with directory.
 */
static void check_directory() {
  try {
    config->parse("./");
  } catch (std::exception const& e) {
    (void)e;
    return;
  }
  throw(engine_error() << "try to parse directory.");
}

/**
 *  Check pase with noexist file.
 */
static void check_noexist_file() {
  try {
    config->parse("./test_noexist_file.cfg");
  } catch (std::exception const& e) {
    (void)e;
    return;
  }
  throw(engine_error() << "try to parse noexisting file.");
}

/**
 *  Check parse with exist file.
 */
static void check_exist_file() {
  char const* tmp(io::file_stream::temp_path());
  if (!tmp)
    throw(engine_error() << "generate temporary file failed");
  try {
    std::ofstream file(tmp, std::ios_base::out | std::ios_base::trunc);
    config->parse(tmp);
  } catch (...) {
    remove(tmp);
    throw;
  }
  remove(tmp);
  return;
}

/**
 *  Check the parsing argument.
 *
 *  @param[in] argc Unused.
 *  @param[in] argv Unused.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char* argv[]) {
  (void)argc;
  (void)argv;

  // Initialization.
  config->log_archive_path(".");

  // Tests.
  check_directory();
  check_noexist_file();
  check_exist_file();

  // Success.
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

/*
** Copyright 2011-2012 Merethis
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

#include <exception>
#include <fstream>
#include <stdio.h>
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/globals.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Check parse with directory.
 */
static void check_directory() {
  try {
    config.parse("./");
  }
  catch (std::exception const& e) {
    (void)e;
    return;
  }
  throw (engine_error() << "try to parse directory.");
}

/**
 *  Check pase with noexist file.
 */
static void check_noexist_file() {
  try {
    config.parse("./test_noexist_file.cfg");
  }
  catch (std::exception const& e) {
    (void)e;
    return;
  }
  throw (engine_error() << "try to parse noexisting file.");
}

/**
 *  Check parse with exist file.
 */
static void check_exist_file() {
  char const* tmp(tempnam("./", "test_cfg."));
  if (!tmp)
    throw (engine_error() << "generate temporary file failed");
  try {
    std::ofstream file(tmp, std::ios_base::out | std::ios_base::trunc);
    config.parse(tmp);
  }
  catch (...) {
    remove(tmp);
    throw;
  }
  remove(tmp);
}

/**
 *  Check the parsing argument.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  config.set_log_archive_path("./");

  check_directory();
  check_noexist_file();
  check_exist_file();

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

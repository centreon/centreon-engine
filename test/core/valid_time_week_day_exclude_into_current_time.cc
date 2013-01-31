/*
** Copyright 2013 Merethis
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
#include <iostream>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "test/core/libtesttime.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;

/**
 *  Check that the get_next_valid_time function works properly.
 *
 *  @return 0 on success.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  // globale variables.
  time_t now(time(NULL));
  std::string name("testing_timeperiod");
  std::vector<std::string> range;
  std::vector<std::string> exclude;

  // check with week days and exclude time period.
  // range exclude into the current time.

  init_object_skiplists();

  std::string ename("exclude_timeperiod");
  std::vector<std::string> erange;
  core::build_week_days(erange, "11:00-13:00");
  timeperiod* e(core::build_timeperiod(ename, erange, exclude));

  exclude.push_back(ename);
  core::build_week_days(range, "00:00-24:00");
  timeperiod* p(core::build_timeperiod(name, range, exclude));
  bool ret(core::check_valid_time(p, now, now + HOUR(1) + SEC(1)));
  objects::release(p);
  objects::release(e);

  if (!ret) {
    free_memory(get_global_macros());
    throw (engine_error() << "week days and exclude: "
           "range exclude into the current time failed");
  }

  free_object_skiplists();

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

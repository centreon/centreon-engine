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

#include <exception>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/broker/loader.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/broker.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/logging/engine.hh"
#include "test/unittest.hh"

using namespace com::centreon;

/**************************************
 *                                     *
 *          Static Functions           *
 *                                     *
 **************************************/

/**
 *  Check set_show_pid exception.
 *
 *  @param[in, out] obj  The broker object to set.
 *
 *  @return True on success, otherwise false.
 */
static bool check_show_pid(engine::logging::broker& obj) {
  try {
    obj.show_pid(true);
  } catch (std::exception const& e) {
    (void)e;
    return (true);
  }
  return (false);
}

/**
 *  Check set_show_timestamp exception.
 *
 *  @param[in, out] obj  The broker object to set.
 *
 *  @return True on success, otherwise false.
 */
static bool check_show_timestamp(engine::logging::broker& obj) {
  try {
    obj.show_thread_id(true);
  } catch (std::exception const& e) {
    (void)e;
    return (true);
  }
  return (false);
}

/**
 *  Check set_show_pid exception.
 *
 *  @param[in, out] obj  The broker object to set.
 *
 *  @return True on success, otherwise false.
 */
static bool check_show_thread_id(engine::logging::broker& obj) {
  try {
    obj.show_thread_id(true);
  } catch (std::exception const& e) {
    (void)e;
    return (true);
  }
  return (false);
}

/**************************************
 *                                     *
 *         Exported Functions          *
 *                                     *
 **************************************/

/**
 *  Check the logging broker working.
 */
int main_test(int argc, char** argv) {
  (void)argc;
  (void)argv;

  engine::logging::broker obj;
  if (!check_show_pid(obj))
    throw(engine_error() << "check show pid failed");
  if (!check_show_timestamp(obj))
    throw(engine_error() << "check show timestamp failed");
  if (!check_show_thread_id(obj))
    throw(engine_error() << "check show thread id");
  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  engine::unittest utest(argc, argv, &main_test);
  return (utest.run());
}

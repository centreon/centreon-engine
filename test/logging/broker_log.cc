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
 *           Global Objects            *
 *                                     *
 **************************************/

static unsigned int const NB_DBG_TYPE = 18;
static unsigned int const NB_LOG_TYPE = 21;
static char const LOG_MESSAGE[] = "~!@#$%^&*()_+09/qwerty \n";

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

  // Add event logged data to broker.
  config->event_broker_options(BROKER_LOGGED_DATA);

  // Get instance of the module loader.
  engine::broker::loader& loader(engine::broker::loader::instance());

  // Load dummy module.
  std::shared_ptr<engine::broker::handle> mod(
      loader.add_module("./dummymod.so"));
  mod->open();

  // Get instance of logging engine.
  logging::engine& e(logging::engine::instance());

  // Add new object (broker) to log into engine.
  engine::logging::broker obj;
  e.add(&obj, engine::logging::log_all, engine::logging::most);

  // Send message on all different logging type.
  for (unsigned int i(0); i < NB_LOG_TYPE; ++i)
    e.log(i, 0, LOG_MESSAGE, sizeof(LOG_MESSAGE));

  // Send message on all different debug logging type.
  for (unsigned int i(0); i < NB_DBG_TYPE; ++i)
    e.log(i + 32, 0, LOG_MESSAGE, sizeof(LOG_MESSAGE));

  // Remove object (broker).
  e.remove(&obj);

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  engine::unittest utest(argc, argv, &main_test);
  return (utest.run());
}

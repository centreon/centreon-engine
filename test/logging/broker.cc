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
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/broker/loader.hh"
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/broker.hh"
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/engine/logging/object.hh"
#include "com/centreon/shared_ptr.hh"
#include "test/unittest.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::broker;

/**************************************
*                                     *
*           Global Objects            *
*                                     *
**************************************/

static const unsigned int NB_DBG_TYPE  = 18;
static const unsigned int NB_LOG_TYPE  = 21;
static const char*        LOG_MESSAGE  = "~!@#$%^&*()_+09/qwerty \n";

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
  config->set_event_broker_options(BROKER_LOGGED_DATA);

  // Get instance of the module loader.
  broker::loader& loader(broker::loader::instance());

  // Load dummy module.
  shared_ptr<handle> mod(loader.add_module("./dummymod.so"));
  mod->open();

  // Get instance of logging engine.
  logging::engine& engine(logging::engine::instance());

  // Add new object (broker) to log into engine.
  com::centreon::shared_ptr<logging::object> obj(new logging::broker);
  logging::engine::obj_info info(
                              obj,
                              logging::log_all,
                              logging::most);
  unsigned int id(engine.add_object(info));

  // Send message on all different logging type.
  for (unsigned int i(0); i < NB_LOG_TYPE; ++i)
    engine.log(LOG_MESSAGE, 1ull << i, 0);

  // Send message on all different debug logging type.
  for (unsigned int i(0); i < NB_DBG_TYPE; ++i)
    engine.log(LOG_MESSAGE, 1ull << (i + 32), 0);

  // Remove object (broker).
  engine.remove_object(id);

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  unittest utest(argc, argv, &main_test);
  return (utest.run());
}

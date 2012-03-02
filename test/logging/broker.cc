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

#include <QCoreApplication>
#include <QDebug>
#include <exception>
#include "test/unittest.hh"
#include "broker.hh"
#include "broker/loader.hh"
#include "common.hh"
#include "error.hh"
#include "globals.hh"
#include "logging/engine.hh"
#include "logging/broker.hh"
#include "logging/object.hh"

using namespace com::centreon::engine;

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
int main_test() {
  // Add event logged data to broker.
  config.set_event_broker_options(BROKER_LOGGED_DATA);

  // Get instance of the module loader.
  broker::loader& loader(broker::loader::instance());

  // Load dummy module.
  if (loader.load_directory(".") != 1)
    throw (engine_error() << "module loading failed");

  // Get instance of logging engine.
  logging::engine& engine(logging::engine::instance());

  // Add new object (broker) to log into engine.
  QSharedPointer<logging::broker> obj(new logging::broker);
  logging::engine::obj_info info(obj,
                                 logging::log_all,
                                 logging::most);
  unsigned int id = engine.add_object(info);

  // Send message on all different logging type.
  for (unsigned int i = 0; i < NB_LOG_TYPE; ++i) {
    engine.log(LOG_MESSAGE, 1ull << i, 0);
  }

  // Send message on all different debug logging type.
  for (unsigned int i = 0; i < NB_DBG_TYPE; ++i) {
    engine.log(LOG_MESSAGE, 1ull << (i + 32), 0);
  }

  // Remove object (broker).
  engine.remove_object(id);

  return (0);
}

/**
 *  Init unit test.
 */
int main(int argc, char** argv) {
  QCoreApplication app(argc, argv);
  unittest utest(&main_test);
  QObject::connect(&utest, SIGNAL(finished()), &app, SLOT(quit()));
  utest.start();
  app.exec();
  return (utest.ret());
}

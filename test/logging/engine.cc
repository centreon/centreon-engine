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
#include <QCoreApplication>
#include <time.h>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/engine.hh"
#include "com/centreon/shared_ptr.hh"
#include "test/logging/test.hh"
#include "test/unittest.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/**************************************
*                                     *
*           Global Objects            *
*                                     *
**************************************/

static const unsigned int NB_LOG_TYPE  = 21;
static const unsigned int NB_LOG_LEVEL = 3;
static const unsigned int NB_DBG_TYPE  = 18;
static const char*        LOG_MESSAGE  = "~!@#$%^&*()_+09/qwerty \n";

/**************************************
*                                     *
*         Exported Functions          *
*                                     *
**************************************/

/**
 *  Check the engine working.
 */
int main_test() {
  // Get instance of logging engine.
  engine& engine = engine::instance();

  // Add new object (test) to log into engine.
  // One object by logging type and level.
  unsigned long id[NB_LOG_TYPE * NB_LOG_LEVEL + NB_DBG_TYPE];
  for (unsigned int j(0); j < NB_LOG_LEVEL; ++j) {
    for (unsigned int i(0); i < NB_LOG_TYPE; ++i) {
      if (i == 8)
        continue ;
      unsigned long long type(1ull << i);
      com::centreon::shared_ptr<object>
        obj(new test(LOG_MESSAGE, type, j, j + 1));
      engine::obj_info info(obj, type, j);
      id[i + j * NB_LOG_TYPE] = engine.add_object(info);
    }
  }

  // Add new object (test) to log into engine.
  // One object by debug logging type.
  for (unsigned int i(0); i < NB_DBG_TYPE; ++i) {
    unsigned long long type(1ull << (i + 32));
    com::centreon::shared_ptr<object>
      obj(new test(LOG_MESSAGE, type, 0, 1));
    engine::obj_info info(obj, type, 0);
    id[NB_LOG_TYPE * NB_LOG_LEVEL + i] = engine.add_object(info);
  }

  // Send message on all different debug logging type.
  for (unsigned int i(0); i < NB_DBG_TYPE; ++i)
    engine.log(LOG_MESSAGE, 1ull << (i + 32), 0);

  // Send message on all different logging type and level.
  for (unsigned int j(0); j < NB_LOG_LEVEL; ++j)
    for (unsigned int i(0); i < NB_LOG_TYPE; ++i)
      engine.log(LOG_MESSAGE, 1ull << i, j);

  // Remove object (broker).
  for (unsigned int i(0);
       i < NB_LOG_TYPE * NB_LOG_LEVEL + NB_DBG_TYPE;
       ++i)
    engine.remove_object(id[i]);

  // Check if all object was remove.
  if (test::get_nb_instance() != 0)
    throw (engine_error() << "remove_object failed");

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

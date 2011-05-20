/*
** Copyright 2011 Merethis
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

#include <QDebug>
#include <exception>

#include "globals.hh"
#include "nebmods.hh"
#include "common.hh"
#include "error.hh"
#include "broker.hh"
#include "logging/engine.hh"
#include "logging/broker.hh"
#include "logging/object.hh"

using namespace com::centreon::engine::logging;

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
int main(void) {
  try {
    // Add event logged data to broker.
    config.set_event_broker_options(BROKER_LOGGED_DATA);

    // Get instance of logging engine.
    engine& engine = engine::instance();

    // Add and load dummy module.
    if (neb_add_module("./libdummybidulemod.so", "", 0) != 0
        || neb_load_all_modules() != 0)
      throw (engine_error() << "load module failed.");

    // Add new object (broker) to log into engine.
    QSharedPointer<broker> obj(new broker);
    engine::obj_info info(obj, log_all, most);
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
  }
  catch (std::exception const& e) {
    qDebug() << "error: " << e.what();
    return (1);
  }
  catch (...) {
    qDebug() << "error: catch all.";
    return (1);
  }
  return (0);
}


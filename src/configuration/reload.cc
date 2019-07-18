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

#include "com/centreon/concurrency/locker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/configuration/reload.hh"
#include "com/centreon/engine/configuration/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
reload::reload() : _is_finished(true) {}

/**
 *  Destructor.
 */
reload::~reload() throw () {}

/**
 *  Get if the reload finished.
 *
 *  @return True if the reload finished.
 */
bool reload::is_finished() const {
  concurrency::locker lock(&_lock);
  return _is_finished;
}

/**
 *  Start the reload thread.
 */
void reload::start() {
  _set_is_finished(false);
  exec();
}

/**
 *  Wait for the reload thread termination.
 */
void reload::wait() {
  concurrency::thread::wait();
}

/**
 *  Start thread.
 */
void reload::_run() {
  _set_is_finished(false);
  try {
    configuration::state config;
    {
      configuration::parser p;
      std::string path(::config->cfg_main());
      p.parse(path, config);
    }
    configuration::applier::state::instance().apply(config);
  }
  catch (std::exception const& e) {
    logger(log_config_error, most)
      << "Error: " << e.what();
  }
  _set_is_finished(true);
}

/**
 *  Set if the reload finished.
 *
 *  @param[in] value The new value.
 */
void reload::_set_is_finished(bool value) {
  concurrency::locker lock(&_lock);
  _is_finished = value;
}

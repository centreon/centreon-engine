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

#include <cstddef>
#include "com/centreon/engine/configuration/applier/engine.hh"
#include "com/centreon/engine/configuration/applier/globals.hh"
#include "com/centreon/engine/configuration/applier/logging.hh"
#include "com/centreon/engine/configuration/applier/macros.hh"
#include "com/centreon/engine/configuration/applier/scheduler.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/configuration/state.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;

static engine* _instance(NULL);

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void engine::apply(configuration::state& config) {
  applier::logging::instance().apply(config);
  applier::globals::instance().apply(config);
  applier::macros::instance().apply(config);
  applier::state::instance().apply(config);
  applier::scheduler::instance().apply(config);
}

/**
 *  Get the singleton instance of engine applier.
 *
 *  @return Singleton instance.
 */
engine& engine::instance() {
  return (*_instance);
}

/**
 *  Load engine applier singleton.
 */
void engine::load() {
  if (!_instance)
    _instance = new engine;
}

/**
 *  Unload engine applier singleton.
 */
void engine::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
engine::engine() {
  applier::logging::load();
  applier::globals::load();
  applier::macros::load();
  applier::scheduler::load();
  applier::state::load();
}

/**
 *  Default destructor.
 */
engine::~engine() throw() {
  applier::state::unload();
  applier::scheduler::unload();
  applier::macros::unload();
  applier::globals::unload();
  applier::logging::unload();
}

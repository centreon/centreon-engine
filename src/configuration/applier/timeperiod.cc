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

#include "com/centreon/engine/configuration/applier/timeperiod.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

static applier::timeperiod* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::timeperiod::apply(state const& config) {
  _diff(::config->timeperiods(), config.timeperiods());
}

/**
 *  Get the singleton instance of timeperiod applier.
 *
 *  @return Singleton instance.
 */
applier::timeperiod& applier::timeperiod::instance() {
  return (*_instance);
}

/**
 *  Load timeperiod applier singleton.
 */
void applier::timeperiod::load() {
  if (!_instance)
    _instance = new applier::timeperiod;
}

/**
 *  Unload timeperiod applier singleton.
 */
void applier::timeperiod::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::timeperiod::timeperiod() {

}

/**
 *  Destructor.
 */
applier::timeperiod::~timeperiod() throw () {

}

/**
 *  Add new timeperiod.
 *
 *  @param[in] obj The new timeperiod to add into the monitoring engine.
 */
void applier::timeperiod::_add_object(timeperiod_ptr obj) {

}

/**
 *  modified timeperiod.
 *
 *  @param[in] obj The new timeperiod to modify into the monitoring engine.
 */
void applier::timeperiod::_modify_object(timeperiod_ptr obj) {

}

/**
 *  Remove old timeperiod.
 *
 *  @param[in] obj The new timeperiod to remove from the monitoring engine.
 */
void applier::timeperiod::_remove_object(timeperiod_ptr obj) {

}

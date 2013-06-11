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

#include "com/centreon/engine/configuration/applier/servicedependency.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::servicedependency::servicedependency() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::servicedependency::servicedependency(
                              applier::servicedependency const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::servicedependency::~servicedependency() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::servicedependency& applier::servicedependency::operator=(
                              applier::servicedependency const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new servicedependency.
 *
 *  @param[in] obj The new servicedependency to add into the monitoring engine.
 */
void applier::servicedependency::add_object(servicedependency_ptr obj) {
  // XXX
}

/**
 *  Modified servicedependency.
 *
 *  @param[in] obj The new servicedependency to modify into the monitoring engine.
 */
void applier::servicedependency::modify_object(servicedependency_ptr obj) {
  // XXX
}

/**
 *  Remove old servicedependency.
 *
 *  @param[in] obj The new servicedependency to remove from the monitoring engine.
 */
void applier::servicedependency::remove_object(servicedependency_ptr obj) {
  // XXX
}

/**
 *  Resolve a servicedependency.
 *
 *  @param[in] obj Servicedependency object.
 */
void applier::servicedependency::resolve_object(servicedependency_ptr obj) {
  // XXX
}

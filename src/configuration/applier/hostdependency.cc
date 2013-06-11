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

#include "com/centreon/engine/configuration/applier/hostdependency.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::hostdependency::hostdependency() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::hostdependency::hostdependency(
                           applier::hostdependency const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::hostdependency::~hostdependency() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::hostdependency& applier::hostdependency::operator=(
                           applier::hostdependency const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new hostdependency.
 *
 *  @param[in] obj The new hostdependency to add into the monitoring engine.
 */
void applier::hostdependency::add_object(hostdependency_ptr obj) {
  // XXX
}

/**
 *  Modified hostdependency.
 *
 *  @param[in] obj The new hostdependency to modify into the monitoring engine.
 */
void applier::hostdependency::modify_object(hostdependency_ptr obj) {
  // XXX
}

/**
 *  Remove old hostdependency.
 *
 *  @param[in] obj The new hostdependency to remove from the monitoring engine.
 */
void applier::hostdependency::remove_object(hostdependency_ptr obj) {
  // XXX
}

/**
 *  Resolve a hostdependency.
 *
 *  @param[in] obj Hostdependency object.
 */
void applier::hostdependency::resolve_object(hostdependency_ptr obj) {
  // XXX
}

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

#include "com/centreon/engine/configuration/applier/hostescalation.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::hostescalation::hostescalation() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::hostescalation::hostescalation(
                           applier::hostescalation const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::hostescalation::~hostescalation() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::hostescalation& applier::hostescalation::operator=(
                           applier::hostescalation const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new hostescalation.
 *
 *  @param[in] obj The new hostescalation to add into the monitoring engine.
 */
void applier::hostescalation::add_object(hostescalation_ptr obj) {
  // XXX
}

/**
 *  Modified hostescalation.
 *
 *  @param[in] obj The new hostescalation to modify into the monitoring engine.
 */
void applier::hostescalation::modify_object(hostescalation_ptr obj) {
  // XXX
}

/**
 *  Remove old hostescalation.
 *
 *  @param[in] obj The new hostescalation to remove from the monitoring engine.
 */
void applier::hostescalation::remove_object(hostescalation_ptr obj) {
  // XXX
}

/**
 *  Resolve a hostescalation.
 *
 *  @param[in] obj Hostescalation object.
 */
void applier::hostescalation::resolve_object(hostescalation_ptr obj) {
  // XXX
}

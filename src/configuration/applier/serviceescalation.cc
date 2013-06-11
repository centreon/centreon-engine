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

#include "com/centreon/engine/configuration/applier/serviceescalation.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::serviceescalation::serviceescalation() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::serviceescalation::serviceescalation(
                              applier::serviceescalation const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::serviceescalation::~serviceescalation() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::serviceescalation& applier::serviceescalation::operator=(
                              applier::serviceescalation const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new serviceescalation.
 *
 *  @param[in] obj The new serviceescalation to add into the monitoring engine.
 */
void applier::serviceescalation::add_object(serviceescalation_ptr obj) {
  // XXX
}

/**
 *  Modified serviceescalation.
 *
 *  @param[in] obj The new serviceescalation to modify into the monitoring engine.
 */
void applier::serviceescalation::modify_object(serviceescalation_ptr obj) {
  // XXX
}

/**
 *  Remove old serviceescalation.
 *
 *  @param[in] obj The new serviceescalation to remove from the monitoring engine.
 */
void applier::serviceescalation::remove_object(serviceescalation_ptr obj) {
  // XXX
}

/**
 *  Resolve a serviceescalation.
 *
 *  @param[in] obj Serviceescalation object.
 */
void applier::serviceescalation::resolve_object(serviceescalation_ptr obj) {
  // XXX
}

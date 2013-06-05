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

#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

static applier::contact* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::contact::apply(state const& config) {
  _diff(::config->contacts(), config.contacts());
}

/**
 *  Get the singleton instance of contact applier.
 *
 *  @return Singleton instance.
 */
applier::contact& applier::contact::instance() {
  return (*_instance);
}

/**
 *  Load contact applier singleton.
 */
void applier::contact::load() {
  if (!_instance)
    _instance = new applier::contact;
}

/**
 *  Unload contact applier singleton.
 */
void applier::contact::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::contact::contact() {

}

/**
 *  Destructor.
 */
applier::contact::~contact() throw () {

}

/**
 *  Add new contact.
 *
 *  @param[in] obj The new contact to add into the monitoring engine.
 */
void applier::contact::_add_object(contact_ptr obj) {

}

/**
 *  Modified contact.
 *
 *  @param[in] obj The new contact to modify into the monitoring engine.
 */
void applier::contact::_modify_object(contact_ptr obj) {

}

/**
 *  Remove old contact.
 *
 *  @param[in] obj The new contact to remove from the monitoring engine.
 */
void applier::contact::_remove_object(contact_ptr obj) {

}

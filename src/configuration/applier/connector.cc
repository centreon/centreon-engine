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

#include "com/centreon/engine/configuration/applier/connector.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/misc/difference.hh"

using namespace com::centreon::engine::configuration;

static applier::connector* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::connector::apply(state const& config) {
  // list_connector const& old_connectors(::config->connectors());
  // list_connector const& new_connectors(config.connectors());

  // misc::difference<list_connector> diff(old_connectors, new_connectors);
  // list_connector const& added(diff.added());
  // list_connector const& deleted(diff.deleted());
  // list_connector const& modified(diff.modified());

  // _add_connectors(added);
  // _remove_connectors(deleted);
  // _modify_connectors(modified);
}

/**
 *  Get the singleton instance of connector applier.
 *
 *  @return Singleton instance.
 */
applier::connector& applier::connector::instance() {
  return (*_instance);
}

/**
 *  Load connector applier singleton.
 */
void applier::connector::load() {
  if (!_instance)
    _instance = new applier::connector;
}

/**
 *  Unload connector applier singleton.
 */
void applier::connector::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::connector::connector() {

}

/**
 *  Destructor.
 */
applier::connector::~connector() throw () {

}

/**
 *  Add new connectors.
 *
 *  @param[in] data All connector to add into the monitoring engine.
 */
void applier::connector::_add_connectors(list_connector const& data) {
  for (list_connector::const_iterator
         it(data.begin()), end(data.end());
       it != end;
       ++it) {
    // commands::connector* obj(it->second->create());
    // XXX: todo.
  }
}

/**
 *  Modified connectors.
 *
 *  @param[in] data All connector to modify into the monitoring engine.
 */
void applier::connector::_modify_connectors(list_connector const& data) {

}

/**
 *  Remove old connectors.
 *
 *  @param[in] data All connector to remove from the monitoring engine.
 */
void applier::connector::_remove_connectors(list_connector const& data) {

}

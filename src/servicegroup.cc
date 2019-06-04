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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/servicesmember.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/servicegroup.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

servicegroup_map servicegroup::servicegroups;


/**
 *  Create a new service group
 *
 *  @param[in] name       Group name.
 *  @param[in] alias      Group alias.
 *  @param[in] notes      Notes.
 *  @param[in] notes_url  URL.
 *  @param[in] action_url Action URL.
 *
 */
servicegroup::servicegroup(std::string const& group_name,
                           std::string const& alias,
                           std::string const& notes,
                           std::string const& notes_url,
                           std::string const& action_url)
    : _group_name{group_name},
      _alias{alias.empty() ? alias : group_name},
      _notes{notes},
      _notes_url{notes_url},
      _action_url{action_url} {

  // Check if the service group already exist.
  if (is_servicegroup_exist(group_name)) {
    logger(log_config_error, basic)
      << "Error: Servicegroup '" << group_name << "' has already been defined";
    throw (engine_error() << "Could not register service group '"
                          << group_name << "'");
 }
}

uint64_t servicegroup::get_id() const {
  return _id;
}

void servicegroup::set_id(uint64_t id) {
  _id = id;
}

std::string const& servicegroup::get_group_name() const {
  return _group_name;
}

void servicegroup::set_group_name(std::string const& group_name) {
  _group_name = group_name;
}

std::string const& servicegroup::get_alias() const {
  return _alias;
}

void servicegroup::set_alias(std::string const& alias) {
  _alias = alias;
}

std::string const& servicegroup::get_notes() const {
  return _notes;
}

void servicegroup::set_notes(std::string const& notes) {
  _notes = notes;
}

std::string const& servicegroup::get_notes_url() const {
  return _notes_url;
}

void servicegroup::set_notes_url(std::string const& notes_url) {
  _notes_url = notes_url;
}

std::string const& servicegroup::get_action_url() const {
  return _action_url;
}

void servicegroup::set_action_url(std::string const& action_url) {
  _action_url = action_url;
}

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool servicegroup::operator==(servicegroup const& obj) throw () {
  return (_group_name ==  obj.get_group_name()
          && _alias == obj.get_alias()
          && is_equal(members, obj.members)
          && _notes == obj.get_notes()
          && _notes_url == obj.get_notes_url()
          && _action_url == obj.get_action_url());
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool servicegroup::operator!=(servicegroup const& obj) throw () {
  return !(*this == obj);
}

/**
 *  Dump servicegroup content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The servicegroup to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, servicegroup const& obj) {
  os << "servicegroup {\n"
    "  group_name: " << obj.get_group_name() << "\n"
    "  alias:      " << obj.get_alias() << "\n"
    "  members:    " << chkobj(obj.members) << "\n"
    "  notes:      " << obj.get_notes() << "\n"
    "  notes_url:  " << obj.get_notes_url() << "\n"
    "  action_url: " << obj.get_action_url() << "\n"
    "}\n";
  return os;
}

/**
 *  Get servicegroup by name.
 *
 *  @param[in] name The servicegroup name.
 *
 *  @return The struct servicegroup or throw exception if the
 *          servicegroup is not found.
 */
servicegroup& engine::find_servicegroup(std::string const& name) {
  umap<std::string, std::shared_ptr<com::centreon::engine::servicegroup> >::const_iterator
    it(state::instance().servicegroups().find(name));
  if (it == state::instance().servicegroups().end())
    throw (engine_error() << "Service group "
           << name << "' was not found");
  return *it->second;
}

/**
 *  Get if servicegroup exist.
 *
 *  @param[in] name The servicegroup name.
 *
 *  @return True if the servicegroup is found, otherwise false.
 */
bool engine::is_servicegroup_exist(std::string const& name) throw () {
  umap<std::string, std::shared_ptr<com::centreon::engine::servicegroup> >::const_iterator
    it(state::instance().servicegroups().find(name));
  return it != state::instance().servicegroups().end();
}

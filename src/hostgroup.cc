/*
** Copyright 2011-2019 Centreon
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
#include "com/centreon/engine/hostgroup.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

hostgroup_map com::centreon::engine::hostgroup::hostgroups;

/*
 *  Constructor
 *
 *  @param[in] id         Host group id.
 *  @param[in] name       Host group name.
 *  @param[in] alias      Host group alias.
 *  @param[in] notes      Notes.
 *  @param[in] notes_url  URL.
 *  @param[in] action_url Action URL.
 */
hostgroup::hostgroup(uint64_t id,
                     std::string const& name,
                     std::string const& alias,
                     std::string const& notes,
                     std::string const& notes_url,
                     std::string const& action_url)
  : _id{id},
    _group_name{name},
    _alias{alias},
    _notes{notes},
    _notes_url{notes_url},
    _action_url{action_url} {

// Make sure we have the data we need.
  if (name.empty()) {
    logger(log_config_error, basic)
      << "Error: Hostgroup name is NULL";
    throw (engine_error() << "Could not register host group '"
                          << name << "'");
  }

  // Check if the host group already exist.
  hostgroup_map::const_iterator found(hostgroup::hostgroups.find(name));
  if (found != hostgroup::hostgroups.end()) {
    logger(log_config_error, basic)
      << "Error: Hostgroup '" << name << "' has already been defined";
    throw (engine_error() << "Could not register host group '"
                          << name << "'");
  }
}

uint64_t hostgroup::get_id() const {
  return _id;
}

void hostgroup::set_id(uint64_t id)
{
  _id = id;
}

std::string const& hostgroup::get_group_name() const {
  return _group_name;
}

std::string const& hostgroup::get_alias() const {
  return _alias;
}

std::string const& hostgroup::get_notes() const {
  return _notes;
}

std::string const& hostgroup::get_notes_url() const {
  return _notes_url;
}

std::string const& hostgroup::get_action_url() const {
  return _action_url;
}

void hostgroup::set_group_name(std::string const& group_name) {
  _group_name = group_name;
}

void hostgroup::set_alias(std::string const& alias) {
  _alias = alias;
}

void hostgroup::set_notes(std::string const& notes) {
  _notes = notes;
}

void hostgroup::set_notes_url(std::string const& notes_url) {
  _notes_url = notes_url;
}

void hostgroup::set_action_url(std::string const& action_url) {
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
bool hostgroup::operator==(hostgroup const& obj) throw () {
  return (_group_name == obj._group_name
          && _alias == obj._alias
          && ((members.size() == obj.members.size()) &&
            std::equal(members.begin(),
                      members.end(),
                       obj.members.begin()))
          && _notes == obj._notes
          && _notes_url == obj._notes_url
          && _action_url == obj._action_url);
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool hostgroup::operator!=(hostgroup const& obj) throw () {
  return !(*this == obj);
}

/**
 *  Dump hostgroup content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The hostgroup to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os,
  com::centreon::engine::hostgroup const& obj) {
  os << "hostgroup {\n"
    "  group_name: " << obj.get_group_name() << "\n"
    "  alias:      " << obj.get_alias() << "\n"
    "  members:    " << obj.members << "\n"
    "  notes:      " << obj.get_notes() << "\n"
    "  notes_url:  " << obj.get_notes_url() << "\n"
    "  action_url: " << obj.get_action_url() << "\n"
    "}\n";
  return (os);
}

void hostgroup::resolve(int& w, int& e) {
  (void)w;
  int errors{0};

  // Check all group members.
  for (host_map_unsafe::iterator it{members.begin()},
       end{members.end()};
       it != end; ++it) {
    host_map::const_iterator it_host{host::hosts.find(it->first)};
    if (it_host == host::hosts.end() || !it_host->second) {
      logger(log_verification_error, basic)
          << "Error: Host '" << it->first << "' specified in host group '"
          << get_group_name() << "' is not defined anywhere!";
      it->second = nullptr;
      errors++;
    }
    // Save a pointer to this hostgroup for faster host/group
    // membership lookups later.
    else {
      //Update or add of group for name
      if (it_host->second.get() != it->second) {
        // Notify event broker.
        timeval tv = get_broker_timestamp(nullptr);
        broker_group_member(
          NEBTYPE_HOSTGROUPMEMBER_ADD,
          NEBFLAG_NONE,
          NEBATTR_NONE,
          it_host->second.get(),
          this,
          &tv);
      }
      it_host->second->get_parent_groups().push_back(this);
      // Save host pointer for later.
      it->second = it_host->second.get();
    }
  }

  // Check for illegal characters in hostgroup name.
  if (contains_illegal_object_chars(get_group_name().c_str())) {
    logger(log_verification_error, basic)
        << "Error: The name of hostgroup '" << get_group_name()
        << "' contains one or more illegal characters.";
    errors++;
  }

  if (errors) {
    e += errors;
    throw engine_error() << "Cannot resolve host group '" << get_group_name() << "'";
  }
}

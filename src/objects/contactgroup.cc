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
#include "com/centreon/engine/deleter/contactgroup.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/contactgroup.hh"
#include "com/centreon/engine/objects/contactsmember.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       contactgroup const& obj1,
       contactgroup const& obj2) throw () {
  return (is_equal(obj1.group_name, obj2.group_name)
          && is_equal(obj1.alias, obj2.alias)
          && is_equal(obj1.members, obj2.members));
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool operator!=(
       contactgroup const& obj1,
       contactgroup const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump contactgroup content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The contactgroup to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, contactgroup const& obj) {
  os << "contactgroup {\n"
    "  group_name: " << chkstr(obj.group_name) << "\n"
    "  alias:      " << chkstr(obj.alias) << "\n"
    "  members:    " << chkobj(obj.members) << "\n"
    "}\n";
  return (os);
}

/**
 *  Add a new contact group to the list in memory.
 *
 *  @param[in] name  Contact group name.
 *  @param[in] alias Contact group alias.
 */
contactgroup* add_contactgroup(char const* name, char const* alias) {
  // Make sure we have the data we need.
  if (!name || !name[0]) {
    logger(log_config_error, basic)
      << "Error: Contactgroup name is NULL";
    return (NULL);
  }

  // Check if the contact group already exist.
  std::string id(name);
  umap<std::string, shared_ptr<contactgroup_struct> >::const_iterator
    it(state::instance().contactgroups().find(id));
  if (it != state::instance().contactgroups().end()) {
    logger(log_config_error, basic)
      << "Error: Contactgroup '" << name << "' has already been defined";
    return (NULL);
  }

  // Allocate memory for a new contactgroup entry.
  shared_ptr<contactgroup> obj(new contactgroup, deleter::contactgroup);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->group_name = string::dup(name);
    obj->alias = string::dup(!alias ? name : alias);

    // Add new items to the configuration state.
    state::instance().contactgroups()[id] = obj;

    // Add new items to the list.
    obj->next = contactgroup_list;
    contactgroup_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group(
      NEBTYPE_CONTACTGROUP_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      obj.get(),
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

/**
 *  Tests whether a contact is a member of a particular contactgroup.
 *  The mk-livestatus eventbroker module uses this, so it must hang
 *  around until 4.0 to prevent API breakage.
 *
 *  The cgi's stopped using it quite long ago though, so we need only
 *  compile it if we're building the core
 *
 *  @param[in] group Target contact group.
 *  @param[in] cntct Target contact.
 *
 *  @return true or false.
 */
int is_contact_member_of_contactgroup(
      contactgroup* group,
      contact* cntct) {
  if (!group || !cntct)
    return (false);

  // Search all contacts in this contact group.
  for (contactsmember* member(group->members);
       member;
       member = member->next)
    if (member->contact_ptr == cntct)
      return (true);

  return (false);
}

/**
 *  Get contactgroup by name.
 *
 *  @param[in] name The contactgroup name.
 *
 *  @return The struct contactgroup or throw exception if the
 *          contactgroup is not found.
 */
contactgroup& engine::find_contactgroup(std::string const& name) {
  umap<std::string, shared_ptr<contactgroup_struct> >::const_iterator
    it(state::instance().contactgroups().find(name));
  if (it == state::instance().contactgroups().end())
    throw (engine_error() << "Contact group '"
           << name << "' was not found");
  return (*it->second);
}

/**
 *  Get if contactgroup exist.
 *
 *  @param[in] name The contactgroup name.
 *
 *  @return True if the contactgroup is found, otherwise false.
 */
bool engine::is_contactgroup_exist(std::string const& name) throw () {
  umap<std::string, shared_ptr<contactgroup_struct> >::const_iterator
    it(state::instance().contactgroups().find(name));
  return (it != state::instance().contactgroups().end());
}

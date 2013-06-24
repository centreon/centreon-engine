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
#include "com/centreon/engine/deleter/hostgroup.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/misc/object.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/objects/hostgroup.hh"
#include "com/centreon/engine/objects/hostsmember.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::misc;

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       hostgroup const& obj1,
       hostgroup const& obj2) throw () {
  return (is_equal(obj1.group_name, obj2.group_name)
          && is_equal(obj1.alias, obj2.alias)
          && is_equal(obj1.members, obj2.members)
          && is_equal(obj1.notes, obj2.notes)
          && is_equal(obj1.notes_url, obj2.notes_url)
          && is_equal(obj1.action_url, obj2.action_url));
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
       hostgroup const& obj1,
       hostgroup const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump hostgroup content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The hostgroup to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, hostgroup const& obj) {
  os << "hostgroup {\n"
    "  group_name: " << chkstr(obj.group_name) << "\n"
    "  alias:      " << chkstr(obj.alias) << "\n"
    "  members:    " << chkobj(obj.members) << "\n"
    "  notes:      " << chkstr(obj.notes) << "\n"
    "  notes_url:  " << chkstr(obj.notes_url) << "\n"
    "  action_url: " << chkstr(obj.action_url) << "\n"
    "}\n";
  return (os);
}

/**
 *  Add a new host group to the list in memory.
 *
 *  @param[in] name       Host group name.
 *  @param[in] alias      Host group alias.
 *  @param[in] notes      Notes.
 *  @param[in] notes_url  URL.
 *  @param[in] action_url Action URL.
 *
 *  @return New host group.
 */
hostgroup* add_hostgroup(
             char const* name,
             char const* alias,
             char const* notes,
             char const* notes_url,
             char const* action_url) {
  // Make sure we have the data we need.
  if (!name || !name[0]) {
    logger(log_config_error, basic)
      << "Error: Hostgroup name is NULL";
    return (NULL);
  }

  // Allocate memory.
  shared_ptr<hostgroup> obj(new hostgroup, deleter::hostgroup);
  memset(obj.get(), 0, sizeof(hostgroup));

  try {
    // Duplicate vars.
    obj->group_name = my_strdup(name);
    obj->alias = my_strdup(alias ? alias : name);
    if (action_url)
      obj->action_url = my_strdup(action_url);
    if (notes)
      obj->notes = my_strdup(notes);
    if (notes_url)
      obj->notes_url = my_strdup(notes_url);

    // Add new hostgroup to the monitoring engine.
    std::string id(name);
    umap<std::string, shared_ptr<hostgroup_struct> >::const_iterator
      it(state::instance().hostgroups().find(id));
    if (it != state::instance().hostgroups().end()) {
      logger(log_config_error, basic)
        << "Error: Hostgroup '" << name << "' has already been defined";
      return (NULL);
    }

    // Add new items to the configuration state.
    state::instance().hostgroups()[id] = obj;

    // Add new items to the list.
    obj->next = hostgroup_list;
    hostgroup_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group(
      NEBTYPE_HOSTGROUP_ADD,
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
 *  Tests whether a host is a member of a particular hostgroup.
 *
 *  @deprecated This function is only used by the CGIS.
 *
 *  @param[in] group Target host group.
 *  @param[in] hst   Target host.
 *
 *  @return true or false.
 */
int is_host_member_of_hostgroup(hostgroup* group, host* hst) {
  if (!group || !hst)
    return (false);

  for (hostsmember* member(group->members);
       member;
       member = member->next)
    if (member->host_ptr == hst)
      return (true);
  return (false);
}

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

#include "com/centreon/engine/configuration/applier/hostgroup.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/hostgroup.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::hostgroup::hostgroup() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::hostgroup::hostgroup(applier::hostgroup const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::hostgroup::~hostgroup() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::hostgroup& applier::hostgroup::operator=(
                      applier::hostgroup const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new hostgroup.
 *
 *  @param[in] obj The new hostgroup to add into the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::hostgroup::add_object(
                           configuration::hostgroup const& obj,
                           configuration::state const& s) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new hostgroup '" << obj.hostgroup_name() << "'.";

  // Create hostgroup.
  shared_ptr<hostgroup_struct>
    hg(
      add_hostgroup(
        obj.hostgroup_name().c_str(),
        NULL_IF_EMPTY(obj.alias()),
        NULL_IF_EMPTY(obj.notes()),
        NULL_IF_EMPTY(obj.notes_url()),
        NULL_IF_EMPTY(obj.action_url())),
      &deleter::hostgroup);
  if (!hg.get())
    throw (engine_error() << "Error: Could not register hostgroup '"
           << obj.hostgroup_name() << "'.");

  return ;
}

/**
 *  Modified hostgroup.
 *
 *  @param[in] obj The new hostgroup to modify into the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::hostgroup::modify_object(
                           configuration::hostgroup const& obj,
                           configuration::state const& s) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying hostgroup '" << obj.hostgroup_name() << "'.";

  // XXX

  return ;
}

/**
 *  Remove old hostgroup.
 *
 *  @param[in] obj The new hostgroup to remove from the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::hostgroup::remove_object(
                           configuration::hostgroup const& obj,
                           configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing hostgroup '" << obj.hostgroup_name() << "'.";

  // Unregister host.
  unregister_object<hostgroup_struct, &hostgroup_struct::group_name>(
    &hostgroup_list,
    obj.hostgroup_name().c_str());
  applier::state::instance().hostgroups().erase(obj.hostgroup_name());

  return ;
}

/**
 *  Resolve a hostgroup.
 *
 *  @param[in,out] obj Hostgroup object.
 *  @param[in]     s   Configuration being applied.
 */
void applier::hostgroup::resolve_object(
                           configuration::hostgroup const& obj,
                           configuration::state const& s) {
  // Only process if hostgroup has not been resolved already.
  if (!obj.is_resolved()) {
    // Logging.
    logger(logging::dbg_config, logging::more)
      << "Resolving hostgroup '" << obj.hostgroup_name() << "'.";

    // Mark object as resolved.
    obj.set_resolved(true);

    // Add base members.
    for (list_string::const_iterator
           it(obj.members().begin()),
           end(obj.members().end());
         it != end;
         ++it)
      obj.resolved_members().insert(*it);

    // Add hostgroup members.
    for (list_string::const_iterator
           it(obj.hostgroup_members().begin()),
           end(obj.hostgroup_members().end());
         it != end;
         ++it) {
      // Find hostgroup entry.
      set_hostgroup::const_iterator
        it2(s.hostgroups().begin()),
        end2(s.hostgroups().end());
      while (it2 != end2) {
        if ((*it2)->hostgroup_name() == *it)
          break ;
        ++it2;
      }
      if (it2 == s.hostgroups().end())
        throw (engine_error()
               << "Error: Could not add non-existing hostgroup member '"
               << *it << "' to hostgroup '" << obj.hostgroup_name()
               << "'.");

      // Resolve hostgroup member.
      resolve_object(**it2, s);

      // Add hostgroup member members to members.
      for (set_string::const_iterator
             it3((*it2)->resolved_members().begin()),
             end3((*it2)->resolved_members().end());
           it3 != end3;
           ++it3)
        obj.resolved_members().insert(*it3);
    }

    // Apply resolved hosts on hostgroup.
    shared_ptr<hostgroup_struct>&
      hg(applier::state::instance().hostgroups()[obj.hostgroup_name()]);
    for (set_string::const_iterator
           it(obj.resolved_members().begin()),
           end(obj.resolved_members().end());
         it != end;
         ++it)
      if (!add_host_to_hostgroup(hg.get(), it->c_str()))
        throw (engine_error() << "Error: Could not add host member '"
               << *it << "' to host group '" << obj.hostgroup_name()
               << "'.");
  }

  return ;
}

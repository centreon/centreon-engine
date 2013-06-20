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

#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/servicegroup.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/servicegroup.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::servicegroup::servicegroup() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::servicegroup::servicegroup(
                         applier::servicegroup const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::servicegroup::~servicegroup() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::servicegroup& applier::servicegroup::operator=(
                         applier::servicegroup const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new servicegroup.
 *
 *  @param[in] obj The new servicegroup to add into the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::servicegroup::add_object(
                              configuration::servicegroup const& obj,
                              configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new servicegroup '" << obj.servicegroup_name() << "'.";

  // Create servicegroup.
  shared_ptr<servicegroup_struct>
    sg(
      add_servicegroup(
        obj.servicegroup_name().c_str(),
        NULL_IF_EMPTY(obj.alias()),
        NULL_IF_EMPTY(obj.notes()),
        NULL_IF_EMPTY(obj.notes_url()),
        NULL_IF_EMPTY(obj.action_url())),
      &deleter::servicegroup);
  if (!sg.get())
    throw (engine_error() << "Error: Could not register service group '"
           << obj.servicegroup_name() << "'.");

  return ;
}

/**
 *  Modify servicegroup.
 *
 *  @param[in] obj The new servicegroup to modify into the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::servicegroup::modify_object(
                              configuration::servicegroup const& obj,
                              configuration::state const& s) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying servicegroup '" << obj.servicegroup_name() << "'.";

  // XXX

  return ;
}

/**
 *  Remove old servicegroup.
 *
 *  @param[in] obj The new servicegroup to remove from the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::servicegroup::remove_object(
                              configuration::servicegroup const& obj,
                              configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing servicegroup '" << obj.servicegroup_name() << "'.";

  // Unregister servicegroup.
  unregister_object<servicegroup_struct, &servicegroup_struct::group_name>(
    &servicegroup_list,
    obj.servicegroup_name().c_str());
  applier::state::instance().servicegroups().erase(obj.servicegroup_name());

  return ;
}

/**
 *  Resolve a servicegroup.
 *
 *  @param[in,out] obj Servicegroup object.
 *  @param[in]     s   Configuration being applied.
 */
void applier::servicegroup::resolve_object(
                              configuration::servicegroup const& obj,
                              configuration::state const& s) {
  // Only process if servicegroup has not been resolved already.
  if (!obj.is_resolved()) {
    // Logging.
    logger(logging::dbg_config, logging::more)
      << "Resolving servicegroup '" << obj.servicegroup_name() << "'.";

    // Mark object as resolved.
    obj.set_resolved(true);

    // Add base members.
    for (list_string::const_iterator
           it(obj.members().begin()),
           end(obj.members().end());
         it != end;
         ++it) {
      list_string::const_iterator it_prev(it++);
      if (it == end)
        throw (engine_error() << "Error: members of service group '"
               << obj.servicegroup_name()
               << "' were not specified as a list of host-service pairs"
               << " (host1,service1,host2,service2,...)");
      obj.resolved_members().insert(std::make_pair(*it_prev, *it));
    }

    // Add servicegroup members.
    for (list_string::const_iterator
           it(obj.servicegroup_members().begin()),
           end(obj.servicegroup_members().end());
         it != end;
         ++it) {
      // Find servicegroup entry.
      set_servicegroup::const_iterator
        it2(s.servicegroups().begin()),
        end2(s.servicegroups().end());
      while (it2 != end2) {
        if ((*it2)->servicegroup_name() == *it)
          break ;
        ++it2;
      }
      if (it2 == s.servicegroups().end())
        throw (engine_error()
               << "Error: Could not add non-existing servicegroup member '"
               << *it << "' to servicegroup '"
               << obj.servicegroup_name() << "'.");

      // Resolve servicegroup member.
      resolve_object(**it2, s);

      // Add servicegroup member members to members.
      for (set_pair_string::const_iterator
             it3((*it2)->resolved_members().begin()),
             end3((*it2)->resolved_members().end());
           it3 != end3;
           ++it3)
        obj.resolved_members().insert(*it3);
    }

    // Apply resolved services on servicegroup.
    shared_ptr<servicegroup_struct>&
      sg(applier::state::instance().servicegroups()[obj.servicegroup_name()]);
    for (set_pair_string::const_iterator
           it(obj.resolved_members().begin()),
           end(obj.resolved_members().end());
         it != end;
         ++it)
      if (!add_service_to_servicegroup(
             sg.get(),
             it->first.c_str(),
             it->second.c_str()))
        throw (engine_error() << "Error: Could not add service member '"
               << it->second << "' of host '" << it->first
               << "' to service group '" << obj.servicegroup_name()
               << "'.");
  }

  return ;
}

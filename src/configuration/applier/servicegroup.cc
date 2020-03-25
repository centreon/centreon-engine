/*
** Copyright 2011-2013,2015,2017 Centreon
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

#include "com/centreon/engine/configuration/applier/servicegroup.hh"
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

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
applier::servicegroup::servicegroup(applier::servicegroup const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::servicegroup::~servicegroup() throw() {}

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
 *  @param[in] obj  The new servicegroup to add into the monitoring
 *                  engine.
 */
void applier::servicegroup::add_object(configuration::servicegroup const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Creating new servicegroup '" << obj.servicegroup_name() << "'";

  // Add service group to the global configuration set.
  config->servicegroups().insert(obj);

  // Create servicegroup.
  std::shared_ptr<engine::servicegroup> sg{new engine::servicegroup(
      obj.servicegroup_id(), obj.servicegroup_name(), obj.alias(), obj.notes(),
      obj.notes_url(), obj.action_url())};

  // Add  new items to the list.
  engine::servicegroup::servicegroups.insert({sg->get_group_name(), sg});

  // Add servicegroup id to the other props.
  sg->set_id(obj.servicegroup_id());

  // Notify event broker.
  timeval tv(get_broker_timestamp(nullptr));
  broker_group(NEBTYPE_SERVICEGROUP_ADD, NEBFLAG_NONE, NEBATTR_NONE, sg.get(),
               &tv);

  // Apply resolved services on servicegroup.
  for (set_pair_string::const_iterator it(obj.members().begin()),
       end(obj.members().end());
       it != end; ++it)
    sg->members[{it->first, it->second}] = nullptr;
}

/**
 *  Expand all service groups.
 *
 *  @param[in,out] s  State being applied.
 */
void applier::servicegroup::expand_objects(configuration::state& s) {
  // Resolve groups.
  _resolved.clear();
  for (configuration::set_servicegroup::const_iterator
           it(s.servicegroups().begin()),
       end(s.servicegroups().end());
       it != end; ++it)
    _resolve_members(*it, s);

  // Save resolved groups in the configuration set.
  s.servicegroups().clear();
  for (resolved_set::const_iterator it(_resolved.begin()), end(_resolved.end());
       it != end; ++it)
    s.servicegroups().insert(it->second);
}

/**
 *  Modify servicegroup.
 *
 *  @param[in] obj  The new servicegroup to modify into the monitoring
 *                  engine.
 */
void applier::servicegroup::modify_object(
    configuration::servicegroup const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Modifying servicegroup '" << obj.servicegroup_name() << "'";

  // Find old configuration.
  set_servicegroup::iterator it_cfg(config->servicegroups_find(obj.key()));
  if (it_cfg == config->servicegroups().end())
    throw(engine_error() << "Could not modify non-existing "
                         << "service group '" << obj.servicegroup_name()
                         << "'");

  // Find service group object.
  servicegroup_map::iterator it_obj{
      engine::servicegroup::servicegroups.find(obj.key())};

  if (it_obj == engine::servicegroup::servicegroups.end())
    throw(engine_error() << "Could not modify non-existing "
                         << "service group object '" << obj.servicegroup_name()
                         << "'");
  engine::servicegroup* sg(it_obj->second.get());

  // Update the global configuration set.
  configuration::servicegroup old_cfg(*it_cfg);
  config->servicegroups().erase(it_cfg);
  config->servicegroups().insert(obj);

  // Modify properties.
  sg->set_id(obj.servicegroup_id());
  sg->set_action_url(obj.action_url());
  sg->set_alias((obj.alias().empty() ? obj.servicegroup_name() : obj.alias()));
  sg->set_notes(obj.notes());
  sg->set_notes_url(obj.notes_url());

  // Were members modified ?
  if (obj.members() != old_cfg.members()) {
    // Delete all old service group members.
    for (service_map_unsafe::iterator it(it_obj->second->members.begin()),
         end(it_obj->second->members.end());
         it != end; ++it) {
      timeval tv(get_broker_timestamp(NULL));
      broker_group_member(NEBTYPE_SERVICEGROUPMEMBER_DELETE, NEBFLAG_NONE,
                          NEBATTR_NONE, it->second, sg, &tv);
    }
    it_obj->second->members.clear();

    // Create new service group members.
    for (set_pair_string::const_iterator it(obj.members().begin()),
         end(obj.members().end());
         it != end; ++it)
      sg->members[{it->first, it->second}] = nullptr;
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group(NEBTYPE_SERVICEGROUP_UPDATE, NEBFLAG_NONE, NEBATTR_NONE, sg,
               &tv);
}

/**
 *  Remove old servicegroup.
 *
 *  @param[in] obj  The new servicegroup to remove from the monitoring
 *                  engine.
 */
void applier::servicegroup::remove_object(
    configuration::servicegroup const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Removing servicegroup '" << obj.servicegroup_name() << "'";

  // Find service group.
  servicegroup_map::iterator it{
      engine::servicegroup::servicegroups.find(obj.key())};
  if (it != engine::servicegroup::servicegroups.end()) {
    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group(NEBTYPE_SERVICEGROUP_DELETE, NEBFLAG_NONE, NEBATTR_NONE,
                 it->second.get(), &tv);

    // Remove service dependency from its list.
    engine::servicegroup::servicegroups.erase(it);
  }

  // Remove service group from the global configuration state.
  config->servicegroups().erase(obj);
}

/**
 *  Resolve a servicegroup.
 *
 *  @param[in,out] obj  Servicegroup object.
 */
void applier::servicegroup::resolve_object(
    configuration::servicegroup const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
      << "Removing service group '" << obj.servicegroup_name() << "'";

  // Find service group.
  servicegroup_map::const_iterator it{
      engine::servicegroup::servicegroups.find(obj.key())};
  if (it == engine::servicegroup::servicegroups.end())
    throw engine_error() << "Cannot resolve non-existing "
                         << "service group '" << obj.servicegroup_name() << "'";

  // Resolve service group.
  it->second->resolve(config_warnings, config_errors);
}

/**
 *  Resolve members of a service group.
 *
 *  @param[in,out] obj  Service group object.
 *  @param[in]     s    Configuration being applied.
 */
void applier::servicegroup::_resolve_members(
    configuration::servicegroup const& obj,
    configuration::state const& s) {
  // Only process if servicegroup has not been resolved already.
  if (_resolved.find(obj.key()) == _resolved.end()) {
    // Logging.
    logger(logging::dbg_config, logging::more)
        << "Resolving members of service group '" << obj.servicegroup_name()
        << "'";

    // Mark object as resolved.
    configuration::servicegroup& resolved_obj(_resolved[obj.key()]);

    // Insert base members.
    resolved_obj = obj;
    resolved_obj.servicegroup_members().clear();

    // Add servicegroup members.
    for (set_string::const_iterator it(obj.servicegroup_members().begin()),
         end(obj.servicegroup_members().end());
         it != end; ++it) {
      // Find servicegroup entry.
      set_servicegroup::iterator it2(s.servicegroups_find(*it));
      if (it2 == s.servicegroups().end())
        throw(engine_error()
              << "Could not add non-existing service group member '" << *it
              << "' to service group '" << obj.servicegroup_name() << "'");

      // Resolve servicegroup member.
      _resolve_members(*it2, s);

      // Add servicegroup member members to members.
      for (set_pair_string::const_iterator it3(it2->members().begin()),
           end3(it2->members().end());
           it3 != end3; ++it3)
        resolved_obj.members().insert(*it3);
    }
  }
}

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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/servicegroup.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
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
 *  @param[in] obj  The new servicegroup to add into the monitoring
 *                  engine.
 */
void applier::servicegroup::add_object(
                              configuration::servicegroup const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new servicegroup '" << obj.servicegroup_name() << "'";

  // Add service group to the global configuration set.
  config->servicegroups().insert(obj);

  // Add servicegroup id to the other props.
  engine::servicegroup::servicegroups[obj.servicegroup_name()]->set_id(obj.servicegroup_id());

  // Create servicegroup.
  std::shared_ptr<engine::servicegroup> sg{new engine::servicegroup(
                            obj.servicegroup_name().c_str(),
                            NULL_IF_EMPTY(obj.alias()),
                            NULL_IF_EMPTY(obj.notes()),
                            NULL_IF_EMPTY(obj.notes_url()),
                            NULL_IF_EMPTY(obj.action_url()))};

  // Add new items to the configuration state.
  state::instance().servicegroups().insert({sg->get_group_name(), sg});
  // Add  new items to the list.
  engine::servicegroup::servicegroups.insert({sg->get_group_name(), sg});

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group(
    NEBTYPE_SERVICEGROUP_ADD,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    sg.get(),
    &tv);

  // Apply resolved services on servicegroup.
  for (set_pair_string::const_iterator
         it(obj.members().begin()),
         end(obj.members().end());
       it != end;
       ++it) {
    sg->members[{it->first, it->second}] = nullptr;
    // Notify event broker.
    tv = get_broker_timestamp(NULL);
    broker_group_member(
      NEBTYPE_SERVICEGROUPMEMBER_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      nullptr,
      sg.get(),
      &tv);
  }

  return ;
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
       it != end;
       ++it)
    _resolve_members(*it, s);

  // Save resolved groups in the configuration set.
  s.servicegroups().clear();
  for (resolved_set::const_iterator
         it(_resolved.begin()),
         end(_resolved.end());
       it != end;
       ++it)
    s.servicegroups().insert(it->second);

  return ;
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
  set_servicegroup::iterator
    it_cfg(config->servicegroups_find(obj.key()));
  if (it_cfg == config->servicegroups().end())
    throw (engine_error() << "Could not modify non-existing "
           << "service group '" << obj.servicegroup_name() << "'");

  // Find service group object.
  servicegroup_map::iterator it_obj(
    applier::state::instance().servicegroups_find(obj.key()));

  if (it_obj == applier::state::instance().servicegroups().end())
    throw (engine_error() << "Could not modify non-existing "
           << "service group object '" << obj.servicegroup_name()
           << "'");
  engine::servicegroup* sg(it_obj->second.get());

  // Update the global configuration set.
  configuration::servicegroup old_cfg(*it_cfg);
  config->servicegroups().erase(it_cfg);
  config->servicegroups().insert(obj);
  engine::servicegroup::servicegroups[obj.servicegroup_name()]->set_id(obj.servicegroup_id());

  // Modify properties.
  sg->set_action_url(obj.action_url());
  sg->set_alias((obj.alias().empty() ? obj.servicegroup_name() : obj.alias()));
  sg->set_notes(obj.notes());
  sg->set_notes_url(obj.notes_url());

  // Were members modified ?
  if (obj.members() != old_cfg.members()) {
    // Delete all old service group members.
      for (service_map::iterator
             it(it_obj->second->members.begin()),
             end(it_obj->second->members.end());
           it != end;
           ++it) {
      timeval tv(get_broker_timestamp(NULL));
      broker_group_member(
        NEBTYPE_SERVICEGROUPMEMBER_DELETE,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        it->second.get(),
        sg,
        &tv);
    }
    it_obj->second->members.clear();

    // Create new service group members.
    for (set_pair_string::const_iterator
           it(obj.members().begin()),
           end(obj.members().end());
         it != end;
         ++it) {
      sg->members[{it->first, it->second}] = nullptr;
      // Notify event broker.
      timeval tv = get_broker_timestamp(NULL);
      broker_group_member(
        NEBTYPE_SERVICEGROUPMEMBER_ADD,
        NEBFLAG_NONE,
        NEBATTR_NONE,
        nullptr,
        sg,
        &tv);
    }
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group(
    NEBTYPE_SERVICEGROUP_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    sg,
    &tv);

  return ;
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
  umap<std::string, std::shared_ptr<com::centreon::engine::servicegroup> >::iterator
    it(applier::state::instance().servicegroups_find(obj.key()));
  if (it != applier::state::instance().servicegroups().end()) {
    com::centreon::engine::servicegroup* grp(it->second.get());

    // Remove service dependency from its list.
    engine::servicegroup::servicegroups.erase(it);

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_group(
      NEBTYPE_SERVICEGROUP_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      grp,
      &tv);

    // Erase service group object (will effectively delete the object).
    engine::servicegroup::servicegroups.erase(obj.servicegroup_name());
    applier::state::instance().servicegroups().erase(it);
  }

  // Remove service group from the global configuration state.
  config->servicegroups().erase(obj);

  return ;
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
  umap<std::string, std::shared_ptr<com::centreon::engine::servicegroup> >::const_iterator
    it(applier::state::instance().servicegroups_find(obj.key()));
  if (applier::state::instance().servicegroups().end() == it)
    throw (engine_error() << "Cannot resolve non-existing "
           << "service group '" << obj.servicegroup_name() << "'");

  // Resolve service group.
  if (!check_servicegroup(it->second.get(), &config_warnings, &config_errors))
    throw (engine_error() << "Cannot resolve service group '"
           << obj.servicegroup_name() << "'");

  return ;
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
      << "Resolving members of service group '"
      << obj.servicegroup_name() << "'";

    // Mark object as resolved.
    configuration::servicegroup& resolved_obj(_resolved[obj.key()]);

    // Insert base members.
    resolved_obj = obj;
    resolved_obj.servicegroup_members().clear();

    // Add servicegroup members.
    for (set_string::const_iterator
           it(obj.servicegroup_members().begin()),
           end(obj.servicegroup_members().end());
         it != end;
         ++it) {
      // Find servicegroup entry.
      set_servicegroup::iterator it2(s.servicegroups_find(*it));
      if (it2 == s.servicegroups().end())
        throw (engine_error()
               << "Could not add non-existing service group member '"
               << *it << "' to service group '"
               << obj.servicegroup_name() << "'");

      // Resolve servicegroup member.
      _resolve_members(*it2, s);

      // Add servicegroup member members to members.
      for (set_pair_string::const_iterator
             it3(it2->members().begin()),
             end3(it2->members().end());
           it3 != end3;
           ++it3)
        resolved_obj.members().insert(*it3);
    }
  }

  return ;
}

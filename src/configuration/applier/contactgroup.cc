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
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/contactgroup.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine::configuration;
using namespace com::centreon::engine::logging;

/**
 *  Default constructor.
 */
applier::contactgroup::contactgroup() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::contactgroup::contactgroup(
                         applier::contactgroup const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::contactgroup::~contactgroup() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::contactgroup& applier::contactgroup::operator=(
                         applier::contactgroup const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new contactgroup
 *
 *  @param[in] obj  The new contactgroup to add into the monitoring engine.
 */
void applier::contactgroup::add_object(
                              configuration::contactgroup const& obj) {
  std::string const& name(obj.contactgroup_name());

  // Logging.
  logger(logging::dbg_config, logging::more) << "Creating new contactgroup '" 
    << name << "'.";

  if(engine::contactgroup::contactgroups.find(name) !=
    engine::contactgroup::contactgroups.end())
    throw engine_error() << "Contactgroup '" << name <<
        "' has already been defined";

  // Add contact group to the global configuration set.
  config->contactgroups().insert(obj);

  // Create contact group.
  std::shared_ptr<engine::contactgroup> cg{new engine::contactgroup(obj)};

  for (set_string::const_iterator
         it(obj.members().begin()),
         end(obj.members().end());
       it != end;
       ++it) {
    contact_map::iterator ct_it{engine::contact::contacts.find(*it)};
    if (ct_it == engine::contact::contacts.end()) {
     logger(log_verification_error, basic)
        << "Error: Contact '" << *it
        << "' specified in contact group '" << cg->get_name()
        << "' is not defined anywhere!";
      throw engine_error() << "Error: Cannot resolve contact group "
         << obj.contactgroup_name() << "'";
    } else {
      cg->get_members().insert({ct_it->first, ct_it->second.get()});
      timeval tv(get_broker_timestamp(nullptr));
      broker_group(NEBTYPE_CONTACTGROUP_ADD,
          NEBFLAG_NONE,
          NEBATTR_NONE,
          cg.get(),
          &tv);
    }
  }

  engine::contactgroup::contactgroups.insert({name, cg});
}

/**
 *  Expand all contactgroups.
 *
 *  @param[in,out] s  State being applied.
 */
void applier::contactgroup::expand_objects(configuration::state& s) {
  // Resolve groups.
  _resolved.clear();
  for (configuration::set_contactgroup::iterator
         it(s.contactgroups().begin()),
         end(s.contactgroups().end());
       it != end;
       ++it)
    _resolve_members(s, *it);

  // Save resolved groups in the configuration set.
  s.contactgroups().clear();
  for (resolved_set::const_iterator
         it(_resolved.begin()),
         end(_resolved.end());
       it != end;
       ++it)
    s.contactgroups().insert(it->second);
}

/**
 *  Modified contactgroup.
 *
 *  @param[in] obj  The new contactgroup to modify into the monitoring
 *                  engine.
 */
void applier::contactgroup::modify_object(
                              configuration::contactgroup const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying contactgroup '" << obj.contactgroup_name() << "'";

  // Find old configuration.
  set_contactgroup::iterator
    it_cfg(config->contactgroups_find(obj.key()));
  if (it_cfg == config->contactgroups().end())
    throw (engine_error() << "Error: Could not modify non-existing "
           << "contact group '" << obj.contactgroup_name() << "'");

  // Find contact group object.
  contactgroup_map::iterator
    it_obj(engine::contactgroup::contactgroups.find(obj.key()));
  if (it_obj == engine::contactgroup::contactgroups.end())
    throw (engine_error() << "Error: Could not modify non-existing "
           << "contact group object '" << obj.contactgroup_name() << "'");

  // Update the global configuration set.
  configuration::contactgroup old_cfg(*it_cfg);
  config->contactgroups().erase(it_cfg);
  config->contactgroups().insert(obj);

  // Modify properties.
  if (it_obj->second->get_alias() != obj.alias())
    it_obj->second->set_alias(obj.alias());

  if (obj.members() != old_cfg.members()) {
    //delete all old contact group members
    it_obj->second->clear_members();

    for (set_string::const_iterator
         it(obj.members().begin()),
         end(obj.members().end());
       it != end;
       ++it) {
      contact_map::const_iterator
        ct_it{engine::contact::contacts.find(*it)};
      if (ct_it == engine::contact::contacts.end()) {
        logger(log_verification_error, basic)
          << "Error: Contact '" << *it
          << "' specified in contact group '" << it_obj->second->get_name()
          << "' is not defined anywhere!";
        throw engine_error() << "Error: Cannot resolve contact group "
           << obj.contactgroup_name() << "'";
      }
      else {
        it_obj->second->get_members().insert({ct_it->first, ct_it->second.get()});
        timeval tv(get_broker_timestamp(nullptr));
        broker_group(NEBTYPE_CONTACTGROUP_ADD,
            NEBFLAG_NONE,
            NEBATTR_NONE,
            it_obj->second.get(),
            &tv);
      }
    }
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_group(
    NEBTYPE_CONTACTGROUP_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    it_obj->second.get(),
    &tv);
}

/**
 *  Remove old contactgroup.
 *
 *  @param[in] obj  The new contactgroup to remove from the monitoring
 *                  engine.
 */
void applier::contactgroup::remove_object(
                              configuration::contactgroup const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing contactgroup '" << obj.contactgroup_name() << "'";

  // Find contact group.
  contactgroup_map::iterator
    it(engine::contactgroup::contactgroups.find(obj.key()));
  if (it != engine::contactgroup::contactgroups.end()) {

    // Remove contact group from its list.
    //unregister_object<contactgroup>(&contactgroup_list, grp);

    // Notify event broker.
    timeval tv(get_broker_timestamp(nullptr));
    broker_group(
      NEBTYPE_CONTACTGROUP_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      it->second.get(),
      &tv);

    // Remove contact group (this will effectively delete the object).
    engine::contactgroup::contactgroups.erase(it);
  }

  // Remove contact group from the global configuration set.
  config->contactgroups().erase(obj);
}

/**
 *  Resolve a contact group.
 *
 *  @param[in] obj  Contact group object.
 */
void applier::contactgroup::resolve_object(
                              configuration::contactgroup const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving contact group '" << obj.contactgroup_name() << "'";

  // Find contact group.
  contactgroup_map::iterator
    it{engine::contactgroup::contactgroups.find(obj.key())};
  if (engine::contactgroup::contactgroups.end() == it || !it->second)
    throw engine_error() << "Error: Cannot resolve non-existing "
           << "contact group '" << obj.contactgroup_name() << "'";

  // Resolve contact group.
  it->second->resolve(config_warnings, config_errors);
}

/**
 *  Resolve members of a contact group.
 *
 *  @param[in,out] s    Configuration being applied.
 *  @param[in]     obj  Object that should be processed.
 */
void applier::contactgroup::_resolve_members(
                              configuration::state& s,
                              configuration::contactgroup const& obj) {
  // Only process if contactgroup has not been resolved already.
  if (_resolved.find(obj.key()) == _resolved.end()) {
    // Logging.
    logger(logging::dbg_config, logging::more)
      << "Resolving members of contact group '"
      << obj.contactgroup_name() << "'";

    // Mark object as resolved.
    configuration::contactgroup& resolved_obj(_resolved[obj.key()]);

    // Insert base members.
    resolved_obj = obj;
    resolved_obj.contactgroup_members().clear();

    // Add contactgroup members.
    for (set_string::const_iterator
           it(obj.contactgroup_members().begin()),
           end(obj.contactgroup_members().end());
         it != end;
         ++it) {
      // Find contactgroup entry.
      set_contactgroup::iterator it2(s.contactgroups_find(*it));
      if (it2 == s.contactgroups().end())
        throw engine_error()
               << "Error: Could not add non-existing contact group member '"
               << *it << "' to contactgroup '"
               << obj.contactgroup_name() << "'";

      // Resolve contactgroup member.
      _resolve_members(s, *it2);

      // Add contactgroup member members to members.
      configuration::contactgroup& resolved_group(_resolved[*it]);
      resolved_obj.members().insert(
                               resolved_group.members().begin(),
                               resolved_group.members().end());
    }
  }
}

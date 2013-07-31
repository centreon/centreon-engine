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

#include <algorithm>
#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/commandsmember.hh"
#include "com/centreon/engine/deleter/customvariablesmember.hh"
#include "com/centreon/engine/deleter/listmember.hh"
#include "com/centreon/engine/deleter/objectlist.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

/**
 *  Check if the contact group name matches the configuration object.
 */
class         contactgroup_name_comparator {
public:
              contactgroup_name_comparator(
                std::string const& contactgroup_name) {
    _contactgroup_name = contactgroup_name;
  }

  bool        operator()(shared_ptr<configuration::contactgroup> cg) {
    return (_contactgroup_name == cg->contactgroup_name());
 }

private:
  std::string _contactgroup_name;
};

/**
 *  Default constructor.
 */
applier::contact::contact() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::contact::contact(applier::contact const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::contact::~contact() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::contact& applier::contact::operator=(
                                      applier::contact const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new contact.
 *
 *  @param[in] obj The new contact to add into the monitoring engine.
 */
void applier::contact::add_object(
                         shared_ptr<configuration::contact> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new contact '" << obj->contact_name() << "'.";

  // Add contact to the global configuration set.
  config->contacts().insert(obj);

  // Create address list.
  char const* addresses[MAX_CONTACT_ADDRESSES];
  memset(addresses, 0, sizeof(addresses));
  {
    unsigned int i(0);
    for (tab_string::const_iterator
           it(obj->address().begin()),
           end(obj->address().end());
         it != end;
         ++it, ++i)
      addresses[i] = NULL_IF_EMPTY(*it);
  }

  // Create contact.
  contact_struct*
    c(add_contact(
        obj->contact_name().c_str(),
        NULL_IF_EMPTY(obj->alias()),
        NULL_IF_EMPTY(obj->email()),
        NULL_IF_EMPTY(obj->pager()),
        addresses,
        NULL_IF_EMPTY(obj->service_notification_period()),
        NULL_IF_EMPTY(obj->host_notification_period()),
        static_cast<bool>(
          obj->service_notification_options() & service::ok),
        static_cast<bool>(
          obj->service_notification_options() & service::critical),
        static_cast<bool>(
          obj->service_notification_options() & service::warning),
        static_cast<bool>(
          obj->service_notification_options() & service::unknown),
        static_cast<bool>(
          obj->service_notification_options() & service::flapping),
        static_cast<bool>(
          obj->service_notification_options() & service::downtime),
        static_cast<bool>(
          obj->host_notification_options() & host::up),
        static_cast<bool>(
          obj->host_notification_options() & host::down),
        static_cast<bool>(
          obj->host_notification_options() & host::unreachable),
        static_cast<bool>(
          obj->host_notification_options() & host::flapping),
        static_cast<bool>(
          obj->host_notification_options() & host::downtime),
        obj->host_notifications_enabled(),
        obj->service_notifications_enabled(),
        obj->can_submit_commands(),
        obj->retain_status_information(),
        obj->retain_nonstatus_information()));
  if (!c)
    throw (engine_error() << "Error: Could not register contact '"
           << obj->contact_name() << "'.");

  // Add all the host notification commands.
  for (list_string::const_iterator
         it(obj->host_notification_commands().begin()),
         end(obj->host_notification_commands().end());
       it != end;
       ++it)
    if (!add_host_notification_command_to_contact(
           c,
           it->c_str()))
      throw (engine_error()
             << "Error: Could not add host notification command '"
             << *it << "' to contact '" << obj->contact_name() << "'.");

  // Add all the service notification commands.
  for (list_string::const_iterator
	 it(obj->service_notification_commands().begin()),
	 end(obj->service_notification_commands().end());
       it != end;
       ++it)
    if (!add_service_notification_command_to_contact(
           c,
	   it->c_str()))
      throw (engine_error()
	     << "Error: Could not add service notification command '"
	     << *it << "' to contact '" << obj->contact_name() << "'.");

  // Add all custom variables.
  for (map_customvar::const_iterator
           it(obj->customvariables().begin()),
           end(obj->customvariables().end());
       it != end;
       ++it)
    if (!add_custom_variable_to_contact(
           c,
	   it->first.c_str(),
	   it->second.c_str()))
      throw (engine_error()
	     << "Error: Could not add custom variable '" << it->first
	     << "' to contact '" << obj->contact_name() << "'.");

  return ;
}

/**
 *  @brief Expand a contact.
 *
 *  During expansion, the contact will be added to its contact groups.
 *  These will be modified in the state.
 *
 *  @param[in]      obj Contact to expand.
 *  @param[int,out] s   Configuration state.
 */
void applier::contact::expand_object(
                         shared_ptr<configuration::contact> obj,
                         configuration::state& s) {
  // Browse contact groups.
  for (list_string::const_iterator
         it(obj->contactgroups().begin()),
         end(obj->contactgroups().end());
       it != end;
       ++it) {
    // Find contact group.
    std::set<shared_ptr<configuration::contactgroup> >::iterator
      it_group(std::find_if(
                      s.contactgroups().begin(),
                      s.contactgroups().end(),
                      contactgroup_name_comparator(*it)));
    if (it_group == s.contactgroups().end())
      throw (engine_error() << "Error: Could not add contact '"
             << obj->contact_name()
             << "' to non-existing contact group '" << *it << "'.");

    // Remove contact group from state.
    shared_ptr<configuration::contactgroup> backup(*it_group);
    s.contactgroups().erase(it_group);

    // Add contact to group members.
    backup->members().push_back(obj->contact_name());

    // Reinsert contact group.
    s.contactgroups().insert(backup);
  }

  // We do not need to reinsert the contact in the set, as no
  // modification was applied on the contact.

  return ;
}

/**
 *  Modified contact.
 *
 *  @param[in] obj The new contact to modify into the monitoring engine.
 */
void applier::contact::modify_object(
                         shared_ptr<configuration::contact> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying contact '" << obj-> contact_name() << "'.";

  // Find old configuration.
  set_contact::iterator it_cfg(config->contacts_find(obj->key()));
  if (it_cfg == config->contacts().end())
    throw (engine_error() << "Error: Cannot modify non-existing "
           << "contact '" << obj->contact_name() << "'.");

  // Find contact object.
  umap<std::string, shared_ptr<contact_struct> >::iterator
    it_obj(applier::state::instance().contacts_find(obj->key()));
  if (it_obj == applier::state::instance().contacts().end())
    throw (engine_error() << "Error: Could not modify non-existing "
           << "contact object '" << obj->contact_name() << "'.");
  contact_struct* c(it_obj->second.get());

  // Update the global configuration set.
  shared_ptr<configuration::contact> old_cfg(*it_cfg);
  config->contacts().erase(it_cfg);
  config->contacts().insert(obj);

  // Modify contact.
  modify_if_different(c->alias, NULL_IF_EMPTY(obj->alias()));
  modify_if_different(c->email, NULL_IF_EMPTY(obj->email()));
  modify_if_different(c->pager, NULL_IF_EMPTY(obj->pager()));
  modify_if_different(
    c->address,
    obj->address(),
    MAX_CONTACT_ADDRESSES);
  modify_if_different(
    c->notify_on_service_unknown,
    static_cast<int>(static_cast<bool>(
      obj->service_notification_options() & service::unknown)));
  modify_if_different(
    c->notify_on_service_warning,
    static_cast<int>(static_cast<bool>(
      obj->service_notification_options() & service::warning)));
  modify_if_different(
    c->notify_on_service_critical,
    static_cast<int>(static_cast<bool>(
      obj->service_notification_options() & service::critical)));
  modify_if_different(
    c->notify_on_service_recovery,
    static_cast<int>(static_cast<bool>(
      obj->service_notification_options() & service::ok)));
  modify_if_different(
    c->notify_on_service_flapping,
    static_cast<int>(static_cast<bool>(
      obj->service_notification_options() & service::flapping)));
  modify_if_different(
    c->notify_on_service_downtime,
    static_cast<int>(static_cast<bool>(
      obj->service_notification_options() & service::downtime)));
  modify_if_different(
    c->notify_on_host_down,
    static_cast<int>(static_cast<bool>(
      obj->host_notification_options() & host::down)));
  modify_if_different(
    c->notify_on_host_unreachable,
    static_cast<int>(static_cast<bool>(
      obj->host_notification_options() & host::unreachable)));
  modify_if_different(
    c->notify_on_host_recovery,
    static_cast<int>(static_cast<bool>(
      obj->host_notification_options() & host::up)));
  modify_if_different(
    c->notify_on_host_flapping,
    static_cast<int>(static_cast<bool>(
      obj->host_notification_options() & host::flapping)));
  modify_if_different(
    c->notify_on_host_downtime,
    static_cast<int>(static_cast<bool>(
      obj->host_notification_options() & host::downtime)));
  modify_if_different(
    c->host_notification_period,
    NULL_IF_EMPTY(obj->host_notification_period()));
  modify_if_different(
    c->service_notification_period,
    NULL_IF_EMPTY(obj->service_notification_period()));
  modify_if_different(
    c->host_notifications_enabled,
    static_cast<int>(obj->host_notifications_enabled()));
  modify_if_different(
    c->service_notifications_enabled,
    static_cast<int>(obj->service_notifications_enabled()));
  modify_if_different(
    c->can_submit_commands,
    static_cast<int>(obj->can_submit_commands()));
  modify_if_different(
    c->retain_status_information,
    static_cast<int>(obj->retain_status_information()));
  modify_if_different(
    c->retain_nonstatus_information,
    static_cast<int>(obj->retain_nonstatus_information()));

  // Host notification commands.
  if (obj->host_notification_commands()
      != old_cfg->host_notification_commands()) {
    deleter::listmember(
      c->host_notification_commands,
      &deleter::commandsmember);

    for (list_string::const_iterator
           it(obj->host_notification_commands().begin()),
           end(obj->host_notification_commands().end());
         it != end;
         ++it)
      if (!add_host_notification_command_to_contact(
             c,
             it->c_str()))
        throw (engine_error()
               << "Error: Could not add host notification command '"
               << *it << "' to contact '" << obj->contact_name()
               << "'.");
  }

  // Service notification commands.
  if (obj->service_notification_commands()
      != old_cfg->service_notification_commands()) {
    deleter::listmember(
      c->service_notification_commands,
      &deleter::commandsmember);

    for (list_string::const_iterator
           it(obj->service_notification_commands().begin()),
           end(obj->service_notification_commands().end());
         it != end;
         ++it)
      if (!add_service_notification_command_to_contact(
             c,
             it->c_str()))
        throw (engine_error()
               << "Error: Could not add service notification command '"
               << *it << "' to contact '" << obj->contact_name()
               << "'.");
  }

  // Custom variables.
  if (std::operator!=(obj->customvariables(), old_cfg->customvariables())) {
    deleter::listmember(
      c->custom_variables,
      &deleter::customvariablesmember);

    for (map_customvar::const_iterator
           it(obj->customvariables().begin()),
           end(obj->customvariables().end());
         it != end;
         ++it)
      if (!add_custom_variable_to_contact(
             c,
             it->first.c_str(),
             it->second.c_str()))
        throw (engine_error()
               << "Error: Could not add custom variable '" << it->first
               << "' to contact '" << obj->contact_name() << "'.");
  }

  // Notify event broker.
  timeval tv(get_broker_timestamp(NULL));
  broker_adaptive_contact_data(
    NEBTYPE_CONTACT_UPDATE,
    NEBFLAG_NONE,
    NEBATTR_NONE,
    c,
    CMD_NONE,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    MODATTR_ALL,
    &tv);

  return ;
}

/**
 *  Remove old contact.
 *
 *  @param[in] obj The new contact to remove from the monitoring engine.
 */
void applier::contact::remove_object(
                         shared_ptr<configuration::contact> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing contact '" << obj->contact_name() << "'.";

  // Find contact.
  umap<std::string, shared_ptr<contact_struct> >::iterator
    it(applier::state::instance().contacts_find(obj->key()));
  if (it != applier::state::instance().contacts().end()) {
    contact_struct* cntct(it->second.get());

    // Remove contact from its list.
    unregister_object<contact_struct>(&contact_list, cntct);

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_adaptive_contact_data(
      NEBTYPE_CONTACT_DELETE,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      cntct,
      CMD_NONE,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      MODATTR_ALL,
      &tv);

    // Erase contact object (this will effectively delete the object).
    applier::state::instance().contacts().erase(it);
  }

  // Remove contact from the global configuration set.
  config->contacts().erase(obj);

  return ;
}

/**
 *  Resolve a contact.
 *
 *  @param[in,out] obj Object to resolve.
 */
void applier::contact::resolve_object(
                         shared_ptr<configuration::contact> obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving contact '" << obj->contact_name() << "'.";

  // Find contact.
  umap<std::string, shared_ptr<contact_struct> >::iterator
    it(applier::state::instance().contacts().find(obj->contact_name()));
  if (applier::state::instance().contacts().end() == it)
    throw (engine_error()
           << "Error: Cannot resolve non-existing contact '"
           << obj->contact_name() << "'.");

  // Remove contact group links.
  deleter::listmember(
    it->second->contactgroups_ptr,
    &deleter::objectlist);

  // Resolve contact.
  if (!check_contact(it->second.get(), NULL, NULL))
    throw (engine_error() << "Error: Cannot resolve contact '"
           << obj->contact_name() << "'.");

  return ;
}

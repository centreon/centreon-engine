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

#include "com/centreon/engine/config.hh"
#include "com/centreon/engine/configuration/applier/command.hh"
#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/customvariable.hh"
#include "com/centreon/engine/configuration/applier/object.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

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
 *  @param[in] s   Configuration being applied.
 */
void applier::contact::add_object(
                         configuration::contact const& obj,
                         configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new contact '" << obj.contact_name() << "'.";

  // Create address list.
  char const* addresses[MAX_CONTACT_ADDRESSES];
  memset(addresses, 0, sizeof(addresses));
  {
    unsigned int i(0);
    for (tab_string::const_iterator
           it(obj.address().begin()),
           end(obj.address().end());
         it != end;
         ++it, ++i)
      addresses[i] = NULL_IF_EMPTY(*it);
  }

  // Create contact.
  contact_struct*
    c(add_contact(
        obj.contact_name().c_str(),
        NULL_IF_EMPTY(obj.alias()),
        NULL_IF_EMPTY(obj.email()),
        NULL_IF_EMPTY(obj.pager()),
        addresses,
        NULL_IF_EMPTY(obj.service_notification_period()),
        NULL_IF_EMPTY(obj.host_notification_period()),
        static_cast<bool>(
          obj.service_notification_options() & service::ok),
        static_cast<bool>(
          obj.service_notification_options() & service::critical),
        static_cast<bool>(
          obj.service_notification_options() & service::warning),
        static_cast<bool>(
          obj.service_notification_options() & service::unknown),
        static_cast<bool>(
          obj.service_notification_options() & service::flapping),
        static_cast<bool>(
          obj.service_notification_options() & service::downtime),
        static_cast<bool>(
          obj.host_notification_options() & host::up),
        static_cast<bool>(
          obj.host_notification_options() & host::down),
        static_cast<bool>(
          obj.host_notification_options() & host::unreachable),
        static_cast<bool>(
          obj.host_notification_options() & host::flapping),
        static_cast<bool>(
          obj.host_notification_options() & host::downtime),
        obj.host_notifications_enabled(),
        obj.service_notifications_enabled(),
        obj.can_submit_commands(),
        obj.retain_status_information(),
        obj.retain_nonstatus_information()));
  if (!c)
    throw (engine_error() << "Error: Could not register contact '"
           << obj.contact_name() << "'.");

  // Add all the host notification commands.
  for (list_string::const_iterator
         it(obj.host_notification_commands().begin()),
         end(obj.host_notification_commands().end());
       it != end;
       ++it)
    if (!add_host_notification_command_to_contact(
           c,
           it->c_str()))
      throw (engine_error()
             << "Error: Could not add host notification command '"
             << *it << "' to contact '" << obj.contact_name() << "'.");

  // Add all the service notification commands.
  for (list_string::const_iterator
	 it(obj.service_notification_commands().begin()),
	 end(obj.service_notification_commands().end());
       it != end;
       ++it)
    if (!add_service_notification_command_to_contact(
           c,
	   it->c_str()))
      throw (engine_error()
	     << "Error: Could not add service notification command '"
	     << *it << "' to contact '" << obj.contact_name() << "'.");

  // Add all custom variables.
  for (properties::const_iterator
           it(obj.customvariables().begin()),
           end(obj.customvariables().end());
       it != end;
       ++it)
    if (!add_custom_variable_to_contact(
           c,
	   it->first.c_str(),
	   it->second.c_str()))
      throw (engine_error()
	     << "Error: Could not add custom variable '" << it->first
	     << "' to contact '" << obj.contact_name() << "'.");

  // Add contact to contactgroups.
  for (list_string::const_iterator
         it_group(obj.contactgroups().begin()),
         end_group(obj.contactgroups().end());
       it_group != end_group;
       ++it_group) {
    umap<std::string, shared_ptr<contactgroup_struct> >::iterator
      it(applier::state::instance().contactgroups().find(*it_group));
    if (it == applier::state::instance().contactgroups().end())
      throw (engine_error()
             << "Error: Could not add contact '" << obj.contact_name()
             << "' to non-existing contact group '" << *it_group
             << "'.");
    if (!add_contact_to_contactgroup(
           it->second.get(),
           obj.contact_name().c_str()))
      throw (engine_error()
             << "Error: Could not add contact '" << obj.contact_name()
             << "' to contact group '" << *it_group << "'.");
  }

  // XXX : c->last_host_notification = obj.last_host_notification();
  // XXX : c->last_service_notification = obj.last_service_notification();
  // XXX : c->modified_attributes = obj.modified_attributes();
  // XXX : c->modified_host_attributes = obj.modified_host_attributes();
  // XXX : c->modified_service_attributes = obj.modified_service_attributes();

  return ;
}

/**
 *  Modified contact.
 *
 *  @param[in] obj The new contact to modify into the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::contact::modify_object(
                         configuration::contact const& obj,
                         configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying contact '" << obj. contact_name() << "'.";

  // Modify command.
  shared_ptr<contact_struct>&
    c(applier::state::instance().contacts()[obj.contact_name()]);
  modify_if_different(c->alias, NULL_IF_EMPTY(obj.alias()));
  modify_if_different(c->email, NULL_IF_EMPTY(obj.email()));
  modify_if_different(c->pager, NULL_IF_EMPTY(obj.pager()));
  modify_if_different(
    c->address,
    obj.address(),
    MAX_CONTACT_ADDRESSES);
  modify_if_different(
    c->notify_on_service_unknown,
    static_cast<int>(static_cast<bool>(
      obj.service_notification_options() & service::unknown)));
  modify_if_different(
    c->notify_on_service_warning,
    static_cast<int>(static_cast<bool>(
      obj.service_notification_options() & service::warning)));
  modify_if_different(
    c->notify_on_service_critical,
    static_cast<int>(static_cast<bool>(
      obj.service_notification_options() & service::critical)));
  modify_if_different(
    c->notify_on_service_recovery,
    static_cast<int>(static_cast<bool>(
      obj.service_notification_options() & service::ok)));
  modify_if_different(
    c->notify_on_service_flapping,
    static_cast<int>(static_cast<bool>(
      obj.service_notification_options() & service::flapping)));
  modify_if_different(
    c->notify_on_service_downtime,
    static_cast<int>(static_cast<bool>(
      obj.service_notification_options() & service::downtime)));
  modify_if_different(
    c->notify_on_host_down,
    static_cast<int>(static_cast<bool>(
      obj.host_notification_options() & host::down)));
  modify_if_different(
    c->notify_on_host_unreachable,
    static_cast<int>(static_cast<bool>(
      obj.host_notification_options() & host::unreachable)));
  modify_if_different(
    c->notify_on_host_recovery,
    static_cast<int>(static_cast<bool>(
      obj.host_notification_options() & host::up)));
  modify_if_different(
    c->notify_on_host_flapping,
    static_cast<int>(static_cast<bool>(
      obj.host_notification_options() & host::flapping)));
  modify_if_different(
    c->notify_on_host_downtime,
    static_cast<int>(static_cast<bool>(
      obj.host_notification_options() & host::downtime)));
  modify_if_different(
    c->host_notification_period,
    NULL_IF_EMPTY(obj.host_notification_period()));
  modify_if_different(
    c->service_notification_period,
    NULL_IF_EMPTY(obj.service_notification_period()));
  modify_if_different(
    c->host_notifications_enabled,
    static_cast<int>(obj.host_notifications_enabled()));
  modify_if_different(
    c->service_notifications_enabled,
    static_cast<int>(obj.service_notifications_enabled()));
  modify_if_different(
    c->can_submit_commands,
    static_cast<int>(obj.can_submit_commands()));
  modify_if_different(
    c->retain_status_information,
    static_cast<int>(obj.retain_status_information()));
  modify_if_different(
    c->retain_nonstatus_information,
    static_cast<int>(obj.retain_nonstatus_information()));
  if (c->host_notification_commands // Overloaded operator.
      != obj.host_notification_commands()) {
    for (commandsmember* m(c->host_notification_commands); m;) {
      commandsmember* to_delete(m);
      m = m->next;
      delete [] to_delete->cmd;
      delete to_delete;
    }
    c->host_notification_commands = NULL;
    for (list_string::const_iterator
           it(obj.host_notification_commands().begin()),
           end(obj.host_notification_commands().end());
         it != end;
         ++it)
      if (!add_host_notification_command_to_contact(
             c.get(),
             it->c_str()))
        throw (engine_error()
               << "Error: Could not add host notification command '"
               << *it << "' to contact '" << obj.contact_name()
               << "'.");
  }
  if (c->service_notification_commands // Overloaded operator.
      != obj.service_notification_commands()) {
    for (commandsmember* m(c->service_notification_commands); m;) {
      commandsmember* to_delete(m);
      m = m->next;
      delete [] to_delete->cmd;
      delete to_delete;
    }
    c->service_notification_commands = NULL;
    for (list_string::const_iterator
           it(obj.service_notification_commands().begin()),
           end(obj.service_notification_commands().end());
         it != end;
         ++it)
      if (!add_service_notification_command_to_contact(
             c.get(),
             it->c_str()))
        throw (engine_error()
               << "Error: Could not add service notification command '"
               << *it << "' to contact '" << obj.contact_name()
               << "'.");
  }
  if (c->custom_variables // Overloaded operator.
      != obj.customvariables()) {
    for (customvariablesmember* cv(c->custom_variables); cv; ) {
      customvariablesmember* to_delete(cv);
      cv = cv->next;
      delete [] to_delete->variable_name;
      delete [] to_delete->variable_value;
      delete to_delete;
    }
    c->custom_variables = NULL;
    for (properties::const_iterator
           it(obj.customvariables().begin()),
           end(obj.customvariables().end());
         it != end;
         ++it)
      if (!add_custom_variable_to_contact(
             c.get(),
             it->first.c_str(),
             it->second.c_str()))
        throw (engine_error()
               << "Error: Could not add custom variable '" << it->first
               << "' to contact '" << obj.contact_name() << "'.");
  }

  // XXX : c->last_host_notification = obj.last_host_notification();
  // XXX : c->last_service_notification = obj.last_service_notification();
  // XXX : c->modified_attributes = obj.modified_attributes();
  // XXX : c->modified_host_attributes = obj.modified_host_attributes();
  // XXX : c->modified_service_attributes = obj.modified_service_attributes();

  return ;
}

/**
 *  Remove old contact.
 *
 *  @param[in] obj The new contact to remove from the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::contact::remove_object(
                         configuration::contact const& obj,
                         configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing contact '" << obj.contact_name() << "'.";

  // Unregister contact.
  unregister_object<contact_struct, &contact_struct::name>(
    &contact_list,
    obj.contact_name().c_str());
  applier::state::instance().contacts().erase(obj.contact_name());

  return ;
}

/**
 *  Resolve a contact.
 *
 *  @param[in,out] obj Object to resolve.
 *  @param[in]     s   Configuration being applied.
 */
void applier::contact::resolve_object(
                         configuration::contact const& obj,
                         configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving contact '" << obj.contact_name() << "'.";

  // Find contact.
  umap<std::string, shared_ptr<contact_struct> >::iterator
    it(applier::state::instance().contacts().find(obj.contact_name()));
  if (applier::state::instance().contacts().end() == it)
    throw (engine_error()
           << "Error: Cannot resolve non-existing contact '"
           << obj.contact_name() << "'.");

  // Resolve contact.
  if (!check_contact(it->second.get(), NULL, NULL))
    throw (engine_error() << "Error: Cannot resolve contact '"
           << obj.contact_name() << "'.");

  return ;
}

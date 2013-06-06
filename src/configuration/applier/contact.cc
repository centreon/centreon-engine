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

#include "com/centreon/engine/configuration/applier/contact.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/member.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration;

static applier::contact* _instance = NULL;

/**
 *  Apply new configuration.
 *
 *  @param[in] config The new configuration.
 */
void applier::contact::apply(configuration::state const& config) {
  _diff(::config->contacts(), config.contacts());
}

/**
 *  Get the singleton instance of contact applier.
 *
 *  @return Singleton instance.
 */
applier::contact& applier::contact::instance() {
  return (*_instance);
}

/**
 *  Load contact applier singleton.
 */
void applier::contact::load() {
  if (!_instance)
    _instance = new applier::contact;
}

/**
 *  Unload contact applier singleton.
 */
void applier::contact::unload() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Default constructor.
 */
applier::contact::contact() {

}

/**
 *  Destructor.
 */
applier::contact::~contact() throw () {

}

/**
 *  Add new contact.
 *
 *  @param[in] obj The new contact to add into the monitoring engine.
 */
void applier::contact::_add_object(contact_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new contact '" << obj->contact_name() << "'.";

  // Create contact.
  shared_ptr<contact_struct> c(new contact_struct);
  memset(c.get(), 0, sizeof(*c));
  c->name = my_strdup(obj->contact_name().c_str());
  c->alias = obj->alias().empty() ? NULL : my_strdup(obj->alias().c_str());
  c->email = obj->email().empty() ? NULL : my_strdup(obj->email().c_str());
  c->pager = obj->pager().empty() ? NULL : my_strdup(obj->pager().c_str());
  {
    memset(c->address, 0, sizeof(c->address));
    unsigned int i(0);
    for (tab_string::const_iterator
	   it(obj->address().begin()),
	   end(obj->address().end());
	 it != end;
	 ++it, ++i)
      c->address[i] = my_strdup(it->c_str());
  }
  add_members<command_struct, commandsmember_struct>(
    applier::state::instance().commands(),
    obj->host_notification_commands(),
    c->host_notification_commands);
  add_members<command_struct, commandsmember_struct>(
    applier::state::instance().commands(),
    obj->service_notification_commands(),
    c->service_notification_commands);
  c->notify_on_service_unknown = static_cast<bool>(obj->service_notification_options() & service::unknown);
  c->notify_on_service_warning = static_cast<bool>(obj->service_notification_options() & service::warning);
  c->notify_on_service_critical = static_cast<bool>(obj->service_notification_options() & service::critical);
  c->notify_on_service_recovery = static_cast<bool>(obj->service_notification_options() & service::recovery);
  c->notify_on_service_flapping = static_cast<bool>(obj->service_notification_options() & service::flapping);
  c->notify_on_service_downtime = static_cast<bool>(obj->service_notification_options() & service::downtime);
  c->notify_on_host_down = static_cast<bool>(obj->host_notification_options() & host::down);
  c->notify_on_host_unreachable = static_cast<bool>(obj->host_notification_options() & host::unreachable);
  c->notify_on_host_recovery = static_cast<bool>(obj->host_notification_options() & host::recovery);
  c->notify_on_host_flapping = static_cast<bool>(obj->host_notification_options() & host::flapping);
  c->notify_on_host_downtime = static_cast<bool>(obj->host_notification_options() & host::downtime);
  c->host_notification_period = my_strdup(obj->host_notification_period().c_str());
  c->service_notification_period = my_strdup(obj->service_notification_period().c_str());
  c->host_notifications_enabled = obj->host_notifications_enabled();
  c->service_notifications_enabled = obj->service_notifications_enabled();
  c->can_submit_commands = obj->can_submit_commands();
  c->retain_status_information = obj->retain_status_information();
  c->retain_nonstatus_information = obj->retain_nonstatus_information();
  for (properties::const_iterator
	 it(obj->customvariables().begin()),
	 end(obj->customvariables().end());
       it != end;
       ++it)
    add_custom_variable_to_contact(c.get(), it->first.c_str(), it->second.c_str());
  // XXX : c->last_host_notification = obj->last_host_notification();
  // XXX : c->last_service_notification = obj->last_service_notification();
  // XXX : c->modified_attributes = obj->modified_attributes();
  // XXX : c->modified_host_attributes = obj->modified_host_attributes();
  // XXX : c->modified_service_attributes = obj->modified_service_attributes();
  // XXX : host_notification_period_ptr
  // XXX : service_notification_period_ptr
  // XXX : contactgroups_ptr

  // Register contact.
  c->next = contact_list;
  applier::state::instance().contacts()[obj->contact_name()] = c;
  contact_list = c.get();

  return ;
}

/**
 *  Modified contact.
 *
 *  @param[in] obj The new contact to modify into the monitoring engine.
 */
void applier::contact::_modify_object(contact_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying contact '" << obj-> contact_name() << "'.";

  // Modify command.
  shared_ptr<contact_struct>&
    c(applier::state::instance().contacts()[obj->contact_name()]);
  modify_if_different(c->alias, obj->alias().c_str());
  modify_if_different(c->email, obj->email().c_str());
  modify_if_different(c->pager, obj->pager().c_str());
  modify_if_different(c->address, obj->address(), MAX_CONTACT_ADDRESSES);
  // XXX : host_notification_commands
  // XXX : service_notification_commands
  modify_if_different(
    c->notify_on_service_unknown,
    static_cast<int>(obj->service_notification_options() & service::unknown));
  modify_if_different(
    c->notify_on_service_warning,
    static_cast<int>(obj->service_notification_options() & service::warning));
  modify_if_different(
    c->notify_on_service_critical,
    static_cast<int>(obj->service_notification_options() & service::critical));
  modify_if_different(
    c->notify_on_service_recovery,
    static_cast<int>(obj->service_notification_options() & service::recovery));
  modify_if_different(
    c->notify_on_service_flapping,
    static_cast<int>(obj->service_notification_options() & service::flapping));
  modify_if_different(
    c->notify_on_service_downtime,
    static_cast<int>(obj->service_notification_options() & service::downtime));
  modify_if_different(
    c->notify_on_host_down,
    static_cast<int>(obj->host_notification_options() & host::down));
  modify_if_different(
    c->notify_on_host_unreachable,
    static_cast<int>(obj->host_notification_options() & host::unreachable));
  modify_if_different(
    c->notify_on_host_recovery,
    static_cast<int>(obj->host_notification_options() & host::recovery));
  modify_if_different(
    c->notify_on_host_flapping,
    static_cast<int>(obj->host_notification_options() & host::flapping));
  modify_if_different(
    c->notify_on_host_downtime,
    static_cast<int>(obj->host_notification_options() & host::downtime));
  modify_if_different(c->host_notification_period, obj->host_notification_period().c_str());
  c->host_notification_period_ptr = applier::state::instance().timeperiods()[c->host_notification_period].get();
  modify_if_different(c->service_notification_period, obj->service_notification_period().c_str());
  c->service_notification_period_ptr = applier::state::instance().timeperiods()[c->service_notification_period].get();
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
  // XXX : custom_variable
  // XXX : c->last_host_notification = obj->last_host_notification();
  // XXX : c->last_service_notification = obj->last_service_notification();
  // XXX : c->modified_attributes = obj->modified_attributes();
  // XXX : c->modified_host_attributes = obj->modified_host_attributes();
  // XXX : c->modified_service_attributes = obj->modified_service_attributes();
  // XXX : contactgroups_ptr

  return ;
}

/**
 *  Remove old contact.
 *
 *  @param[in] obj The new contact to remove from the monitoring engine.
 */
void applier::contact::_remove_object(contact_ptr obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing contact '" << obj->contact_name() << "'.";

  // Unregister contact.
  for (contact_struct** cs(&contact_list); *cs; cs = &(*cs)->next)
    if (!strcmp((*cs)->name, obj->contact_name().c_str())) {
      (*cs) = (*cs)->next;
      break ;
    }
  applier::state::instance().contacts().erase(obj->contact_name());

  return ;
}

/*
** Copyright 2011-2015,2017-2019 Centreon
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
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/listmember.hh"
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

  bool        operator()(std::shared_ptr<configuration::contactgroup> cg) {
    return _contactgroup_name == cg->contactgroup_name();
 }

private:
  std::string _contactgroup_name;
};

/**
 *  Default constructor.
 */
applier::contact::contact() {}

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
//applier::contact& applier::contact::operator=(
//                                      applier::contact const& right) {
//  (void)right;
//  return *this;
//}

/**
 *  Add new contact.
 *
 *  @param[in] obj  The new contact to add into the monitoring engine.
 */
void applier::contact::add_object(configuration::contact const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new contact '" << obj.contact_name() << "'.";

  // Add contact to the global configuration set.
  config->contacts().insert(obj);

  // Create address list.
  std::array<std::string, MAX_CONTACT_ADDRESSES> addresses;
  {
    unsigned int i{0};
    for (tab_string::const_iterator
           it(obj.address().begin()),
           end(obj.address().end());
         it != end;
         ++it, ++i)
      addresses[i] = *it;
  }

  // Create contact.
  std::shared_ptr<com::centreon::engine::contact>
    c(add_contact(
        obj.contact_name(),
        obj.alias(),
        obj.email(),
        obj.pager(),
        addresses,
        obj.service_notification_period(),
        obj.host_notification_period(),
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
    throw (engine_error() << "Could not register contact '"
           << obj.contact_name() << "'");
  c->set_timezone(obj.timezone());

  // Add new items to the configuration state.
  engine::contact::contacts.insert({c->get_name(), c});

  // Add all custom variables.
  for (map_customvar::const_iterator
           it(obj.customvariables().begin()),
           end(obj.customvariables().end());
       it != end;
       ++it)
    c->custom_variables.insert({it->first, it->second});
}

/**
 *  @brief Expand a contact.
 *
 *  During expansion, the contact will be added to its contact groups.
 *  These will be modified in the state.
 *
 *  @param[in,out] s  Configuration state.
 */
void applier::contact::expand_objects(configuration::state& s) {
  // Browse all contacts.
  for (configuration::set_contact::iterator
         it_contact(s.contacts().begin()),
         end_contact(s.contacts().end());
       it_contact != end_contact;
       ++it_contact) {
    // Should custom variables be sent to broker ?
    for (map_customvar::const_iterator
           it(it_contact->customvariables().begin()),
           end(it_contact->customvariables().end());
         it != end;
         ++it) {
      if (!s.enable_macros_filter()
          || s.macros_filter().find(it->first) != s.macros_filter().end()) {
        customvariable& cv(const_cast<customvariable&>(it->second));
        cv.set_sent(true);
      }
    }

    // Browse current contact's groups.
    for (set_string::const_iterator
           it_group(it_contact->contactgroups().begin()),
           end_group(it_contact->contactgroups().end());
         it_group != end_group;
         ++it_group) {
      // Find contact group.
      configuration::set_contactgroup::iterator
        group(s.contactgroups_find(*it_group));
      if (group == s.contactgroups().end())
        throw (engine_error() << "Could not add contact '"
               << it_contact->contact_name()
               << "' to non-existing contact group '" << *it_group
               << "'");

      // Remove contact group from state.
      configuration::contactgroup backup(*group);
      s.contactgroups().erase(group);

      // Add contact to group members.
      backup.members().insert(it_contact->contact_name());

      // Reinsert contact group.
      s.contactgroups().insert(backup);
    }
  }
}

/**
 *  Modified contact.
 *
 *  @param[in] obj  The new contact to modify into the monitoring engine.
 */
void applier::contact::modify_object(configuration::contact const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying contact '" << obj.contact_name() << "'.";

  // Find old configuration.
  set_contact::iterator it_cfg(config->contacts_find(obj.key()));
  if (it_cfg == config->contacts().end())
    throw engine_error() << "Cannot modify non-existing contact '"
           << obj.contact_name() << "'";

  // Find contact object.
  contact_map::iterator it_obj(engine::contact::contacts.find(obj.key()));
  if (it_obj == engine::contact::contacts.end())
    throw engine_error() << "Could not modify non-existing "
           << "contact object '" << obj.contact_name() << "'";
  engine::contact* c(it_obj->second.get());

  // Update the global configuration set.
  configuration::contact old_cfg(*it_cfg);
  config->contacts().erase(it_cfg);
  config->contacts().insert(obj);

  // Modify contact.
  std::string const& tmp(obj.alias().empty() ? obj.contact_name() : obj.alias());
  if (c->get_alias() != tmp)
    c->set_alias(tmp);
  if (c->get_email() != obj.email())
    c->set_email(obj.email());
  if (c->get_pager() != obj.pager())
    c->set_pager(obj.pager());
  if (c->get_addresses() != obj.address())
    c->set_addresses(obj.address());
  c->set_notify_on(
      notifier::service_notification,
      (obj.service_notification_options() & service::unknown ? notifier::unknown
                                                             : notifier::none) |
          (obj.service_notification_options() & service::warning
               ? notifier::warning
               : notifier::none) |
          (obj.service_notification_options() & service::critical
               ? notifier::critical
               : notifier::none) |
          (obj.service_notification_options() & service::ok ? notifier::recovery
                                                            : notifier::none) |
          (obj.service_notification_options() & service::flapping
               ? (notifier::flappingstart | notifier::flappingstop |
                  notifier::flappingdisabled)
               : notifier::none) |
          (obj.service_notification_options() & service::downtime
               ? notifier::downtime
               : notifier::none));
  c->set_notify_on(
      notifier::host_notification,
      (obj.host_notification_options() & host::down ? notifier::down
                                                    : notifier::none) |
          (obj.host_notification_options() & host::unreachable
               ? notifier::unreachable
               : notifier::none) |
          (obj.host_notification_options() & host::up ? notifier::recovery
                                                      : notifier::none) |
          (obj.host_notification_options() & host::flapping
               ? (notifier::flappingstart | notifier::flappingstop |
                  notifier::flappingdisabled)
               : notifier::none) |
          (obj.host_notification_options() & host::downtime ? notifier::downtime
                                                            : notifier::none));
  if (c->get_host_notification_period() != obj.host_notification_period())
    c->set_host_notification_period(obj.host_notification_period());
  if (c->get_service_notification_period() != obj.service_notification_period())
    c->set_service_notification_period(obj.service_notification_period());
  if (c->get_host_notifications_enabled() != obj.host_notifications_enabled())
    c->set_host_notifications_enabled(obj.host_notifications_enabled());
  if (c->get_service_notifications_enabled() != obj.service_notifications_enabled())
    c->set_service_notifications_enabled(obj.service_notifications_enabled());
  if (c->get_can_submit_commands() != obj.can_submit_commands())
    c->set_can_submit_commands(obj.can_submit_commands());
  if (c->get_retain_status_information() != obj.retain_status_information())
    c->set_retain_status_information(obj.retain_status_information());
  if (c->get_retain_nonstatus_information() != obj.retain_nonstatus_information())
    c->set_retain_nonstatus_information(obj.retain_nonstatus_information());
  c->set_timezone(obj.timezone());

  // Host notification commands.
  if (obj.host_notification_commands()
      != old_cfg.host_notification_commands()) {
    c->get_host_notification_commands().clear();

    for (list_string::const_iterator
           it(obj.host_notification_commands().begin()),
           end(obj.host_notification_commands().end());
         it != end;
         ++it) {
      command_map::const_iterator itt(commands::command::commands.find(*it));
      if (itt != commands::command::commands.end())
        c->get_host_notification_commands().push_back(itt->second);
      else
        throw (engine_error()
               << "Could not add host notification command '"
               << *it << "' to contact '" << obj.contact_name()
               << "': the command does not exist");
    }
  }

  // Service notification commands.
  if (obj.service_notification_commands()
      != old_cfg.service_notification_commands()) {
    c->get_service_notification_commands().clear();

    for (list_string::const_iterator
           it(obj.service_notification_commands().begin()),
           end(obj.service_notification_commands().end());
         it != end;
         ++it) {
      command_map::const_iterator itt(commands::command::commands.find(*it));
      if (itt != commands::command::commands.end())
        c->get_service_notification_commands().push_back(itt->second);
      else
        throw (engine_error()
               << "Could not add service notification command '"
               << *it << "' to contact '" << obj.contact_name()
               << "': the command does not exist");
    }
  }

  // Custom variables.
  if (std::operator!=(obj.customvariables(), old_cfg.customvariables()))
    c->custom_variables = obj.customvariables();

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
}

/**
 *  Remove old contact.
 *
 *  @param[in] obj  The new contact to remove from the monitoring engine.
 */
void applier::contact::remove_object(
                         configuration::contact const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing contact '" << obj.contact_name() << "'.";

  // Find contact.
  contact_map::iterator it{engine::contact::contacts.find(obj.key())};
  if (it != engine::contact::contacts.end()) {
    engine::contact* cntct(it->second.get());

    // Notify event broker.
    timeval tv(get_broker_timestamp(nullptr));
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
    engine::contact::contacts.erase(it);
  }

  // Remove contact from the global configuration set.
  config->contacts().erase(obj);
}

/**
 *  Resolve a contact.
 *
 *  @param[in,out] obj  Object to resolve.
 */
void applier::contact::resolve_object(
                         configuration::contact const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving contact '" << obj.contact_name() << "'.";

  // Find contact.
  contact_map::const_iterator ct_it{engine::contact::contacts.find(obj.contact_name())};
  if (ct_it == engine::contact::contacts.end() || !ct_it->second)
    throw (engine_error()
           << "Cannot resolve non-existing contact '"
           << obj.contact_name() << "'");

  ct_it->second->get_host_notification_commands().clear();

  // Add all the host notification commands.
  for (list_string::const_iterator
         it(obj.host_notification_commands().begin()),
         end(obj.host_notification_commands().end());
       it != end;
       ++it) {
    command_map::const_iterator itt(commands::command::commands.find(*it));
    if (itt != commands::command::commands.end())
      ct_it->second->get_host_notification_commands().push_back(itt->second);
    else {
      ++config_errors;
      throw (engine_error()
          << "Could not add host notification command '"
          << *it << "' to contact '" << obj.contact_name()
          << "': the command does not exist");
    }
  }

  ct_it->second->get_service_notification_commands().clear();

  // Add all the service notification commands.
  for (list_string::const_iterator
	 it(obj.service_notification_commands().begin()),
	 end(obj.service_notification_commands().end());
       it != end;
       ++it) {
    command_map::const_iterator itt(commands::command::commands.find(*it));
    if (itt != commands::command::commands.end())
      ct_it->second->get_service_notification_commands().push_back(itt->second);
    else {
      ++config_errors;
      throw(
        engine_error() << "Could not add service notification command '" << *it
                       << "' to contact '" << obj.contact_name()
                       << "': the command does not exist");
    }
  }

  // Remove contact group links.
  ct_it->second->get_parent_groups().clear();

  // Resolve contact.
  ct_it->second->resolve(config_warnings, config_errors);
}

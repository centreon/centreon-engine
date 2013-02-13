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

#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/command.hh"
#include "com/centreon/engine/objects/commandsmember.hh"
#include "com/centreon/engine/objects/contact.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"
#include "com/centreon/engine/objects/objectlist.hh"
#include "com/centreon/engine/objects/utils.hh"
#include "com/centreon/engine/skiplist.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::objects::utils;

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::link
 */
bool link_contact(
       contact* obj,
       timeperiod* host_notification_period,
       timeperiod* service_notification_period,
       contactgroup** contactgroups,
       command** host_notification_commands,
       command** service_notification_commands,
       char** custom_variables) {
  try {
    objects::link(
               obj,
               host_notification_period,
               service_notification_period,
               tab2vec(contactgroups),
               tab2vec(host_notification_commands),
               tab2vec(service_notification_commands),
               tab2vec(custom_variables));
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << "error: " << e.what();
    return (false);
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << "error: link_contact: unknow exception";
    return (false);
  }
  return (true);
}


/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::release
 */
void release_contact(contact const* obj) {
  try {
    objects::release(obj);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << "error: " << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic)
      << "error: release_contact: unknow exception";
  }
  return;
}

/**
 *  Link a contact with commands, contacts, notification period and custom variable into the engine.
 *
 *  @param[in,out] obj                           Object to link with a correct name.
 *  @param[in]     host_notification_period      Host timeperiod name.
 *  @param[in]     service_notification_period   Service timeperiod name.
 *  @param[in]     contactgroups                 Table with contact groups member name.
 *  @param[in]     host_notification_commands    Table with host notification commands name.
 *  @param[in]     service_notification_commands Table with service notification commands name.
 *  @param[in]     custom_variables              Table with custom variables.
 */
void objects::link(
                contact* obj,
                timeperiod* host_notification_period,
                timeperiod* service_notification_period,
                std::vector<contactgroup*> const& contactgroups,
                std::vector<command*> const& host_notification_commands,
                std::vector<command*> const& service_notification_commands,
                std::vector<std::string> const& custom_variables) {
  // check object contents.
  if (obj == NULL)
    throw (engine_error() << "contact is a NULL pointer.");
  if (obj->name == NULL)
    throw (engine_error() << "contact invalid name.");

  if ((obj->host_notification_period_ptr = host_notification_period) == NULL)
    throw (engine_error() << "contact '" << obj->name
           << "' invalid host notification period.");

  if ((obj->service_notification_period_ptr = service_notification_period) == NULL)
    throw (engine_error() << "contact '" << obj->name
           << "' invalid service notification period.");

  for (std::vector<contactgroup*>::const_iterator
         it = contactgroups.begin(), end = contactgroups.end();
       it != end;
       ++it) {
    if (*it == NULL)
      throw (engine_error() << "contact '" << obj->name
             << "' invalid contactgroup.");
    add_object_to_objectlist(&obj->contactgroups_ptr, *it);
  }

  if (add_commands_to_object(
        host_notification_commands,
        &obj->host_notification_commands) == false)
    throw (engine_error() << "contact '" << obj->name
           << "' invalid host notification commands.");

  if (add_commands_to_object(
        service_notification_commands,
        &obj->service_notification_commands) == false)
    throw (engine_error() << "contact '" << obj->name
           << "' invalid service notification commands.");

  if (add_custom_variables_to_object(
        custom_variables,
        &obj->custom_variables) == false)
    throw (engine_error() << "contact '" << obj->name
           << "' invalid custom variable.");
  return;
}

/**
 *  Cleanup memory of contact.
 *
 *  @param[in] obj The contact to cleanup memory.
 */
void objects::release(contact const* obj) {
  if (obj == NULL)
    return;

  commandsmember const* cmdmember = obj->host_notification_commands;
  while ((cmdmember = release(cmdmember))) {}

  cmdmember = obj->service_notification_commands;
  while ((cmdmember = release(cmdmember))) {}

  customvariablesmember const* varmember = obj->custom_variables;
  while ((varmember = release(varmember))) {}

  release(obj->contactgroups_ptr);
  skiplist_delete(object_skiplists[CONTACT_SKIPLIST], obj);
  remove_object_list(obj, &contact_list, &contact_list_tail);

  // host_notification_period_ptr not free.
  // service_notification_period_ptr not free.

  delete[] obj->name;
  delete[] obj->alias;
  delete[] obj->email;
  delete[] obj->pager;
  for (unsigned int i = 0; i < MAX_CONTACT_ADDRESSES; ++i)
    delete[] obj->address[i];
  delete[] obj->host_notification_period;
  delete[] obj->service_notification_period;
  delete obj;
  return;
}

/**
 *  Add somme contacts to a generic object with contacts member list.
 *
 *  @param[in]  contacts     The contacts to insert.
 *  @param[out] list_contact The object contact.
 *
 *  @return True if insert sucessfuly, false otherwise.
 */
bool objects::add_contacts_to_object(
                std::vector<contact*> const& contacts,
                contactsmember** list_contact) {
  if (list_contact == NULL)
    return (false);

  for (std::vector<contact*>::const_iterator
         it = contacts.begin(), end = contacts.end();
       it != end;
       ++it) {
    if (*it == NULL)
      return (false);

    // create contactsmember and add it into the contact list.
    contactsmember* member = add_contact_to_object(
                               list_contact,
                               (*it)->name);
    if (member == NULL)
      return (false);

    // add contact to the contactsmember.
    member->contact_ptr = *it;
  }
  return (true);
}

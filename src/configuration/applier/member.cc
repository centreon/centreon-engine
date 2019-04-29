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

#include <memory>
#include "com/centreon/engine/configuration/applier/member.hh"
#include "com/centreon/engine/deleter/contactsmember.hh"
#include "com/centreon/engine/objects/command.hh"
#include "com/centreon/engine/contact.hh"
#include "com/centreon/engine/objects/contactsmember.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::configuration;

/**
 *  Add new contact into contact member struct.
 *
 *  @param[in] contacts The contacts list.
 *  @param[in] name     The contacts name to add.
 *  @param[in] members  The contact members to fill.
 */
void applier::add_member(
       umap<std::string,
           std::shared_ptr<com::centreon::engine::contact> > const& contacts,
       std::string const& name,
       contactsmember_struct*& members) {
  // Find contact to add.
  umap<std::string,
       std::shared_ptr<com::centreon::engine::contact> >::const_iterator
    it(contacts.find(name));
  if (it == contacts.end()) {
    logger(log_config_error, basic)
      << "Error: Cannot add contact member: contact '"
      << name << "' not found";
    return ;
  }

  // Create and fill the new member.
  std::unique_ptr<contactsmember_struct> obj(new contactsmember_struct);
  memset(obj.get(), 0, sizeof(*obj));
  obj->contact_name = string::dup(name);
  obj->contact_ptr = &(*it->second);
  obj->next = members;
  members = obj.release();
}

/**
 *  Add new command into command member struct.
 *
 *  @param[in] contacts The command list.
 *  @param[in] name     The commands name to add.
 *  @param[in] members  The command members to fill.
 */

void applier::update_members(
       umap<std::string,
            std::shared_ptr<com::centreon::engine::contact> > const& contacts,
       std::list<std::string> const& lst,
       contactsmember_struct*& members) {
  deleter::contactsmember(members);
  members = NULL;
  add_members(contacts, lst, members);
}

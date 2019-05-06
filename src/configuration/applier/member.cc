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
#include "com/centreon/engine/objects/commandsmember.hh"
#include "com/centreon/engine/objects/contact.hh"
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
       umap<std::string, std::shared_ptr<contact_struct> > const& contacts,
       std::string const& name,
       contactsmember_struct*& members) {
  // Find contact to add.
  umap<std::string, std::shared_ptr<contact_struct> >::const_iterator
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
void applier::add_member(
       umap<std::string, std::shared_ptr<command_struct> > const& commands,
       std::string const& name,
       commandsmember_struct*& members) {
  // Find command to add.
  umap<std::string, std::shared_ptr<command_struct> >::const_iterator
    it(commands.find(name));
  if (it == commands.end()) {
    logger(log_config_error, basic)
      << "Error: Cannot add command member: command '"
      << name << "' not found";
    return ;
  }

  // Create and fill the new member.
  std::unique_ptr<commandsmember_struct> obj(new commandsmember_struct);
  memset(obj.get(), 0, sizeof(*obj));
  obj->cmd = string::dup(name);
  obj->command_ptr = &(*it->second);
  obj->next = members;
  members = obj.release();
}

void applier::update_members(
       umap<std::string, std::shared_ptr<contact_struct> > const& contacts,
       std::list<std::string> const& lst,
       contactsmember_struct*& members) {
  deleter::contactsmember(members);
  members = NULL;
  add_members(contacts, lst, members);
}

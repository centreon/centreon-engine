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
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::configuration;

/**
 *  Add new contact into contact memeber struct.
 *
 *  @param[in] contacts The contacts list.
 *  @param[in] name     The contacts name to add.
 *  @param[in] memebers The contact memebers to fill.
 */
void applier::add_member(
       umap<std::string, shared_ptr<contact_struct> > const& contacts,
       std::string const& name,
       contactsmember_struct*& members) {
  // Find contact to add.
  umap<std::string, shared_ptr<contact_struct> >::const_iterator
    it(contacts.find(name));
  if (it == contacts.end()) {
    logger(log_config_error, basic)
      << "configuration: add contact into contactsmember failed: "
      "contact name '" << name << "' not found";
    return;
  }

  // Create and fill the new memeber.
  std::auto_ptr<contactsmember_struct> obj(new contactsmember_struct);
  memset(obj.get(), 0, sizeof(*obj));
  obj->contact_name = my_strdup(name.c_str());
  obj->contact_ptr = &(*it->second);
  obj->next = members;
  members = obj.get();

  // Notify event broker.
  // XXX
}

void applier::update_members(
       umap<std::string, shared_ptr<contact_struct> > const& contacts,
       std::list<std::string> const& lst,
       contactsmember_struct*& members) {
  // XXX: call broker callback to notify del.
  deleter::contactsmember(members);
  members = NULL;
  add_members(contacts, lst, members);
  // XXX: call broker callback to notify add.
}

/*
** Copyright 2011-2013,2015 Merethis
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
#include "com/centreon/engine/objects/command.hh"
#include "com/centreon/engine/objects/commandsmember.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::configuration;

/**
 *  Add new command into command member struct.
 *
 *  @param[in] commands  The command list.
 *  @param[in] name      The commands name to add.
 *  @param[in] members   The command members to fill.
 */
void applier::add_member(
       umap<std::string, shared_ptr<command_struct> > const& commands,
       std::string const& name,
       commandsmember_struct*& members) {
  // Find command to add.
  umap<std::string, shared_ptr<command_struct> >::const_iterator
    it(commands.find(name));
  if (it == commands.end()) {
    logger(log_config_error, basic)
      << "Error: Cannot add command member: command '"
      << name << "' not found";
    return ;
  }

  // Create and fill the new member.
  std::auto_ptr<commandsmember_struct> obj(new commandsmember_struct);
  memset(obj.get(), 0, sizeof(*obj));
  obj->cmd = string::dup(name);
  obj->command_ptr = &(*it->second);
  obj->next = members;
  members = obj.release();
}

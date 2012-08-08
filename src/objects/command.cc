/*
** Copyright 2011-2012 Merethis
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

#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/commands/raw.hh"
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/command.hh"
#include "com/centreon/engine/objects/utils.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::objects::utils;

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::link
 */
bool link_command(command const* obj) {
  try {
    objects::link(obj);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << e.what();
    return (false);
  }
  catch (...) {
    logger(log_runtime_error, basic) << __func__ << " unknow exception";
    return (false);
  }
  return (true);
}

/**
 *  Wrapper C
 *
 *  @see com::centreon::engine::objects::release
 */
void release_command(command const* obj) {
  try {
    objects::release(obj);
  }
  catch (std::exception const& e) {
    logger(log_runtime_error, basic) << e.what();
  }
  catch (...) {
    logger(log_runtime_error, basic) << __func__ << " unknow exception";
  }
}

/**
 *  Link a command into the command set.
 *
 *  @param[in,out] obj Object to link with a correct name.
 */
void objects::link(command const* obj) {
  // check object contents.
  if (obj == NULL)
    throw (engine_error() << "command is a NULL pointer.");
  if (obj->name == NULL)
    throw (engine_error() << "command invalid name.");
  if (obj->command_line == NULL)
    throw (engine_error() << "command invalid command line.");

  // update command executon system.
  commands::set& cmd_set = commands::set::instance();
  com::centreon::shared_ptr<commands::command>
    new_command(new commands::raw(
                                obj->name,
                                obj->command_line,
                                &checks::checker::instance()));
  cmd_set.add_command(new_command);
}

/**
 *  Cleanup memory of command.
 *
 *  @param[in] obj The command to cleanup memory.
 */
void objects::release(command const* obj) {
  if (obj == NULL)
    return;

  skiplist_delete(object_skiplists[COMMAND_SKIPLIST], obj);
  remove_object_list(obj, &command_list, &command_list_tail);

  // update command executon system.
  commands::set::instance().remove_command(obj->name);

  delete[] obj->name;
  delete[] obj->command_line;
  delete obj;
}

/**
 *  Add somme commands to a generic object with commands member list.
 *
 *  @param[in]  commands     The commands to insert.
 *  @param[out] list_command The object command.
 *
 *  @return True if insert sucessfuly, false otherwise.
 */
bool objects::add_commands_to_object(std::vector<command*> const& commands,
                                     commandsmember** list_command) {
  if (list_command == NULL)
    return (false);

  for (std::vector<command*>::const_iterator it = commands.begin(),
	 end = commands.end();
       it != end;
       ++it) {
    if (*it == NULL)
      return (false);

    // create a new commandsmember and add it into the command list.
    commandsmember* member = new commandsmember;
    memset(member, 0, sizeof(*member));

    member->cmd = my_strdup((*it)->name);
    member->next = *list_command;
    *list_command = member;

    // add command to the commandsmember.
    member->command_ptr = *it;
  }
  return (true);
}

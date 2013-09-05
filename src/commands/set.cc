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

#include <cstdlib>
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::commands;

// Class instance.
static set* _instance = NULL;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Add (or replace) a new command.
 *
 *  @param[in] cmd The new command.
 */
void set::add_command(command const& cmd) {
  add_command(shared_ptr<command>(cmd.clone()));
  return;
}

/**
 *  Add (or replace) a new command.
 *
 *  @param[in] cmd The new command.
 */
void set::add_command(shared_ptr<command> cmd) {
  _list[cmd->get_name()] = cmd;
  logger(dbg_commands, basic)
    << "added command " << cmd->get_name();
  return;
}

/**
 *  Get a command.
 *
 *  @param[in] cmd_name The command name.
 *
 *  @return The shared pointer on a command object.
 */
shared_ptr<commands::command> set::get_command(
                                     std::string const& cmd_name) {
  umap<std::string, shared_ptr<command> >::iterator
    it(_list.find(cmd_name));
  if (it == _list.end())
    throw (engine_error()
           << "Command '" << cmd_name << "' was not found");
  return (it->second);
}

/**
 *  Get instance of the commands set singleton.
 *
 *  @return This singleton.
 */
set& set::instance() {
  return (*_instance);
}

/**
 *  Load singleton.
 */
void set::load() {
  if (!_instance)
    _instance = new set;
  return;
}

/**
 *  Remove a command.
 *
 *  @param[in] cmd_name The command name.
 */
void set::remove_command(std::string const& cmd_name) {
  _list.erase(cmd_name);
  logger(dbg_commands, basic)
    << "remove command " << cmd_name;
  return;
}

/**
 *  Cleanup the set singleton.
 */
void set::unload() {
  delete _instance;
  _instance = NULL;
  return;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
set::set() {

}

/**
 *  Destructor.
 */
set::~set() throw () {

}

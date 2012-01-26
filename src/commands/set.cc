/*
** Copyright 2011      Merethis
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

#include "error.hh"
#include "logging/logger.hh"
#include "commands/set.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::commands;

set* set::_instance = NULL;

/**
 *  Get instance of the commands set singleton.
 *
 *  @return This singleton.
 */
set& set::instance() {
  if (_instance == NULL)
    _instance = new set();
  return (*_instance);
}

/**
 *  Cleanup the set singleton.
 */
void set::cleanup() {
  delete _instance;
  _instance = NULL;
}

/**
 *  Add (or replace) a new command.
 *
 *  @param[in] cmd The new command.
 */
void set::add_command(command const& cmd) {
  add_command(QSharedPointer<command>(cmd.clone()));
}

/**
 *  Add (or replace) a new command.
 *
 *  @param[in] cmd The new command.
 */
void set::add_command(QSharedPointer<command> cmd) {
  if (connect(&(*cmd),
	      SIGNAL(name_changed(std::string const&, std::string const&)),
	      this,
	      SLOT(command_name_changed(std::string const&, std::string const&)),
              Qt::DirectConnection) == false) {
    throw (engine_error() << "connect command to set failed.");
  }
  // XXX: todo.
  // _list[cmd->get_name()] = cmd;
  logger(dbg_commands, basic) << "add command " << cmd->get_name();
}

/**
 *  Remove a command.
 *
 *  @param[in] cmd_name The command name.
 */
void set::remove_command(std::string const& cmd_name) throw() {
  // XXX: todo.
  // _list.remove(cmd_name);
  logger(dbg_commands, basic) << "remove command " << cmd_name;
}

/**
 *  Get a command.
 *
 *  @param[in] cmd_name The command name.
 *
 *  @return The shared pointer on a command object.
 */
QSharedPointer<commands::command> set::get_command(std::string const& cmd_name) {
  /*
  // XXX: todo.
  std::map<std::string, QSharedPointer<command> >::iterator it = _list.find(cmd_name);
  if (it == _list.end()) {
    throw (engine_error() << "command '" << cmd_name << "' not found.");
  }
  return (it.value());
  */
}

/**
 *  Slot for notify when the commend name changed.
 *
 *  @param[in] old_name The old name of the command.
 *  @param[in] new_name The new name of the command.
 */
void set::command_name_changed(std::string const& old_name,
			       std::string const& new_name) {
  (void)new_name;

  QSharedPointer<commands::command> cmd = get_command(old_name);
  remove_command(old_name);
  add_command(cmd);
}

/**
 *  Default constructor.
 */
set::set() {

}

/**
 *  Default destructor.
 */
set::~set() throw() {

}



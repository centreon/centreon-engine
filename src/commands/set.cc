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
#include "commands/set.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::commands;

/**
 *  Get instance of the commands set singleton.
 *
 *  @return This singleton.
 */
set& set::instance() {
  static set instance;
  return (instance);
}

/**
 *  Cleanup the set singleton.
 */
void set::cleanup() {
  set& instance = set::instance();
  instance._list.clear();
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
	      SIGNAL(name_changed(QString const&, QString const&)),
	      this,
	      SLOT(command_name_changed(QString const&, QString const&))) == false) {
    throw (engine_error() << "connect command to set failed.");
  }
  _list[cmd->get_name()] = cmd;
}

/**
 *  Remove a command.
 *
 *  @param[in] cmd_name The command name.
 */
void set::remove_command(QString const& cmd_name) throw() {
  _list.remove(cmd_name);
}

/**
 *  Get a command.
 *
 *  @param[in] cmd_name The command name.
 *
 *  @return The shared pointer on a command object.
 */
QSharedPointer<commands::command> set::get_command(QString const& cmd_name) {
  QHash<QString, QSharedPointer<command> >::iterator it = _list.find(cmd_name);
  if (it == _list.end()) {
    throw (engine_error() << "command '" << cmd_name << "' not found.");
  }
  return (it.value());
}

/**
 *  Slot for notify when the commend name changed.
 *
 *  @param[in] old_name The old name of the command.
 *  @param[in] new_name The new name of the command.
 */
void set::command_name_changed(QString const& old_name,
			       QString const& new_name) {
  (void)new_name;

  QSharedPointer<commands::command> cmd = get_command(old_name);
  _list.remove(old_name);
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



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

#include <assert.h>
#include <stdlib.h>
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::commands;

// Class instance.
std::auto_ptr<set> set::_instance;

/**************************************
*                                     *
*           Public Methods            *
*                                     *
**************************************/

/**
 *  Destructor.
 */
set::~set() throw () {}

/**
 *  Add (or replace) a new command.
 *
 *  @param[in] cmd The new command.
 */
void set::add_command(command const& cmd) {
  add_command(QSharedPointer<command>(cmd.clone()));
  return ;
}

/**
 *  Add (or replace) a new command.
 *
 *  @param[in] cmd The new command.
 */
void set::add_command(QSharedPointer<command> cmd) {
  if (connect(
        &(*cmd),
        SIGNAL(name_changed(QString const&, QString const&)),
        this,
        SLOT(command_name_changed(QString const&, QString const&)),
        Qt::DirectConnection) == false)
    throw (engine_error() << "connect command to set failed.");
  _list[cmd->get_name()] = cmd;
  logger(dbg_commands, basic) << "added command " << cmd->get_name();
  return ;
}

/**
 *  Get a command.
 *
 *  @param[in] cmd_name The command name.
 *
 *  @return The shared pointer on a command object.
 */
QSharedPointer<commands::command> set::get_command(
                                         QString const& cmd_name) {
  QHash<QString, QSharedPointer<command> >::iterator
    it(_list.find(cmd_name));
  if (it == _list.end())
    throw (engine_error() << "command '" << cmd_name << "' not found");
  return (it.value());
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
  if (!_instance.get())
    _instance.reset(new set);
  return ;
}

/**
 *  Remove a command.
 *
 *  @param[in] cmd_name The command name.
 */
void set::remove_command(QString const& cmd_name) {
  _list.remove(cmd_name);
  logger(dbg_commands, basic) << "remove command " << cmd_name;
  return ;
}

/**
 *  Cleanup the set singleton.
 */
void set::unload() {
  _instance.reset();
}

/**
 *  Slot to get notified when the commend name change.
 *
 *  @param[in] old_name The old name of the command.
 *  @param[in] new_name The new name of the command.
 */
void set::command_name_changed(
            QString const& old_name,
            QString const& new_name) {
  (void)new_name;
  QSharedPointer<commands::command> cmd(get_command(old_name));
  remove_command(old_name);
  add_command(cmd);
  return ;
}

/**************************************
*                                     *
*           Private Methods           *
*                                     *
**************************************/

/**
 *  Default constructor.
 */
set::set() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
set::set(set const& right) : QObject() {
  _internal_copy(right);
}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
set& set::operator=(set const& right) {
  _internal_copy(right);
  return (*this);
}

/**
 *  Copy internal data members.
 *
 *  @param[in] right Object to copy.
 */
void set::_internal_copy(set const& right) {
  (void)right;
  assert(!"command set is not copyable");
  abort();
  return ;
}

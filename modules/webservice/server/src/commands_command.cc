/*
** Copyright 2012 Merethis
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

#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/modules/webservice/commands.hh"
#include "com/centreon/engine/modules/webservice/create_object.hh"
#include "com/centreon/engine/modules/webservice/sync_lock.hh"
#include "com/centreon/engine/objects.hh"
#include "com/centreon/engine/objects/command.hh"
#include "soapH.h"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::modules;
using namespace com::centreon::engine::modules::webservice;

/**
 *  Create a new command into Engine.
 *
 *  @param[in] cmd New command information.
 */
void webservice::create_command(ns1__commandType const& cmd) {
  command* obj(add_command(cmd.name.c_str(), cmd.commandLine.c_str()));
  if (!obj)
    throw (engine_error() << "could not create command '"
           << cmd.name.c_str() << "': invalid name or command line");
  objects::link(obj);
  return ;
}

/**
 *  Add a new command.
 *
 *  @param[in]  s       SOAP object.
 *  @param[in]  command Command to add.
 *  @param[out] res     Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__commandAdd(
      soap* s,
      ns1__commandType* command,
      centreonengine__commandAddResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(command->name << ", " << command->commandLine)

  // Create command.
  create_command(*command);

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Modify an existing command.
 *
 *  @param[in]  s   SOAP object.
 *  @param[in]  cmd New command parameters.
 *  @param[out] res Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__commandModify(
      soap* s,
      ns1__commandType* cmd,
      centreonengine__commandModifyResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(cmd->name << ", " << cmd->commandLine)

  // Modify command.
  QSharedPointer<commands::command>
    modified_cmd(commands::set::instance().get_command(cmd->name.c_str()));
  modified_cmd->set_command_line(cmd->commandLine.c_str());
  modify_command(cmd->name.c_str(), cmd->commandLine.c_str());

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

/**
 *  Remove command.
 *
 *  @param[in]  s           SOAP object.
 *  @param[in]  command_id  Command to remove.
 *  @param[out] res         Unused.
 *
 *  @return SOAP_OK on success.
 */
int centreonengine__commandRemove(
      soap* s,
      ns1__commandIDType* command_id,
      centreonengine__commandRemoveResponse& res) {
  (void)res;

  // Begin try block.
  COMMAND_BEGIN(command_id->command)

  // Remove command.
  int ret(remove_command_by_id(command_id->command.c_str()));
  if (ret != 1) {
    std::string* error(soap_new_std__string(s, 1));
    if (!ret)
      *error = "Command '" + command_id->command + "' not found";
    else
      *error = "Command '"
               + command_id->command
               + "' is still in use, removal refused";
    logger(dbg_commands, most)
      << "Webservice: " << __func__ << " failed: " << *error;
    return (soap_receiver_fault(
              s,
              "Invalid arguments",
              error->c_str()));
  }

  // Exception handling.
  COMMAND_END()

  return (SOAP_OK);
}

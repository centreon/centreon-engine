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

#include "com/centreon/engine/checks/checker.hh"
#include "com/centreon/engine/commands/connector.hh"
#include "com/centreon/engine/commands/set.hh"
#include "com/centreon/engine/configuration/applier/connector.hh"
#include "com/centreon/engine/configuration/applier/difference.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/globals.hh"

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
applier::connector::connector() {}

/**
 *  Copy constructor.
 *
 *  @param[in] right Object to copy.
 */
applier::connector::connector(applier::connector const& right) {
  (void)right;
}

/**
 *  Destructor.
 */
applier::connector::~connector() throw () {}

/**
 *  Assignment operator.
 *
 *  @param[in] right Object to copy.
 *
 *  @return This object.
 */
applier::connector& applier::connector::operator=(
                      applier::connector const& right) {
  (void)right;
  return (*this);
}

/**
 *  Add new connector.
 *
 *  @param[in] obj The new connector to add into the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::connector::add_object(
                           configuration::connector const& obj,
                           configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new connector '" << obj.connector_name() << "'.";

  return ;
}

/**
 *  Modified connector.
 *
 *  @param[in] obj The new connector to modify into the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::connector::modify_object(
                           configuration::connector const& obj,
                           configuration::state const& s) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying connector '" << obj.connector_name() << "'.";

  // XXX : cannot modify a connector because
  //       1) unmodified commands might hold a pointer on the connector
  //       2) even if we cast the command, there's no public API to
  //          restart a connector

  return ;
}

/**
 *  Remove old connector.
 *
 *  @param[in] obj The new connector to remove from the monitoring engine.
 *  @param[in] s   Configuration being applied.
 */
void applier::connector::remove_object(
                           configuration::connector const& obj,
                           configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing connector '" << obj.connector_name() << "'.";

  // Remove connector.
  commands::set::instance().remove_command(obj.connector_name());

  return ;
}

/**
 *  Resolve a connector.
 *
 *  @param[in,out] obj Connector object.
 *  @param[in] s       Configuration being applied.
 */
void applier::connector::resolve_object(
                           configuration::connector const& obj,
                           configuration::state const& s) {
  (void)s;

  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Resolving connector '" << obj.connector_name() << "'.";

  // Expand command line.
  nagios_macros* macros(get_global_macros());
  char* command_line(NULL);
  process_macros_r(
    macros,
    obj.connector_line().c_str(),
    &command_line,
    0);
  std::string processed_cmd(command_line);
  delete [] command_line;

  // Create connector.
  shared_ptr<commands::command>
    cmd(new commands::connector(
                        obj.connector_name(),
                        processed_cmd,
                        &checks::checker::instance()));
  commands::set::instance().add_command(cmd);

  return ;
}

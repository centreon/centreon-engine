/*
** Copyright 2011-2013,2017 Centreon
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
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/macros/misc.hh"
#include "com/centreon/engine/macros/process.hh"

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
 *  @param[in] obj  The new connector to add into the monitoring engine.
 */
void applier::connector::add_object(
                           configuration::connector const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Creating new connector '" << obj.connector_name() << "'.";

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

  // Add connector to the global configuration set.
  config->connectors().insert(obj);

  // Create connector.
  std::shared_ptr<commands::connector>
    cmd(new commands::connector(
                        obj.connector_name(),
                        processed_cmd,
                        &checks::checker::instance()));
  state::instance().connectors()[obj.connector_name()] = cmd;
  commands::set::instance().add_command(cmd);
  return ;
}

/**
 *  @brief Expand connector.
 *
 *  Connector configuration objects do not need expansion. Therefore
 *  this method only copy obj to expanded.
 *
 *  @param[in] s  Unused.
 */
void applier::connector::expand_objects(configuration::state& s) {
  (void)s;
  return ;
}

/**
 *  Modify connector.
 *
 *  @param[in] obj  The connector to modify in the monitoring engine.
 */
void applier::connector::modify_object(
                           configuration::connector const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Modifying connector '" << obj.connector_name() << "'.";

  // Find old configuration.
  set_connector::iterator it_cfg(config->connectors_find(obj.key()));
  if (it_cfg == config->connectors().end())
    throw (engine_error() << "Cannot modify non-existing connector '"
           << obj.connector_name() << "'");

  // Find connector object.
  umap<std::string, std::shared_ptr<commands::connector> >::iterator
    it_obj(applier::state::instance().connectors().find(obj.key()));
  if (it_obj == applier::state::instance().connectors().end())
    throw (engine_error() << "Could not modify non-existing "
           << "connector object '" << obj.connector_name() << "'");
  commands::connector* c(it_obj->second.get());

  // Update the global configuration set.
  config->connectors().erase(it_cfg);
  config->connectors().insert(obj);

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

  // Set the new command line.
  c->set_command_line(processed_cmd);
  return ;
}

/**
 *  Remove old connector.
 *
 *  @param[in] obj  The new connector to remove from the monitoring
 *                  engine.
 */
void applier::connector::remove_object(
                           configuration::connector const& obj) {
  // Logging.
  logger(logging::dbg_config, logging::more)
    << "Removing connector '" << obj.connector_name() << "'.";

  // Find connector.
  umap<std::string, std::shared_ptr<commands::connector> >::iterator
    it(applier::state::instance().connectors().find(obj.key()));
  if (it != applier::state::instance().connectors().end()) {
    // Remove connector object.
    commands::set::instance().remove_command(obj.connector_name());
    state::instance().connectors().erase(it);
  }

  // Remove connector from the global configuration set.
  config->connectors().erase(obj);

  return ;
}

/**
 *  @brief Resolve a connector.
 *
 *  Connector objects do not need resolution. Therefore this method does
 *  nothing.
 *
 *  @param[in] obj Unused.
 */
void applier::connector::resolve_object(
                           configuration::connector const& obj) {
  (void)obj;
  return ;
}

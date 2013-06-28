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

#include "com/centreon/engine/broker.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/deleter/command.hh"
#include "com/centreon/engine/globals.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/command.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/shared_ptr.hh"

using namespace com::centreon;
using namespace com::centreon::engine;
using namespace com::centreon::engine::configuration::applier;
using namespace com::centreon::engine::logging;
using namespace com::centreon::engine::string;

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       command const& obj1,
       command const& obj2) throw () {
  return (is_equal(obj1.name, obj2.name)
          && is_equal(obj1.command_line, obj2.command_line));
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool operator!=(
       command const& obj1,
       command const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump command content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The command to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, command const& obj) {
  os << "command {\n"
    "  name:         " << chkstr(obj.name) << "\n"
    "  command_line: " << chkstr(obj.command_line) << "\n"
    "}\n";
  return (os);
}

/**
 *  Add a new command to the list in memory.
 *
 *  @param[in] name  Command name.
 *  @param[in] value Command itself.
 *
 *  @return New command object.
 */
command* add_command(char const* name, char const* value) {
  // Make sure we have the data we need.
  if (!name || !name[0] || !value || !value[0]) {
    logger(log_config_error, basic)
      << "Error: Command name or command line is NULL";
    return (NULL);
  }

  // Allocate memory for the new command.
  shared_ptr<command> obj(new command, deleter::command);
  memset(obj.get(), 0, sizeof(*obj));

  try {
    // Duplicate vars.
    obj->name = string::dup(name);
    obj->command_line = string::dup(value);

    // Add new command to the monitoring engine.
    std::string id(name);
    umap<std::string, shared_ptr<command_struct> >::const_iterator
      it(state::instance().commands().find(id));
    if (it != state::instance().commands().end()) {
      logger(log_config_error, basic)
        << "Error: Command '" << name << "' has already been defined";
      return (NULL);
    }

    // Add new items to the configuration state.
    state::instance().commands()[id] = obj;

    // Add new items to the list.
    obj->next = command_list;
    command_list = obj.get();

    // Notify event broker.
    timeval tv(get_broker_timestamp(NULL));
    broker_command_data(
      NEBTYPE_COMMAND_ADD,
      NEBFLAG_NONE,
      NEBATTR_NONE,
      name,
      value,
      &tv);
  }
  catch (...) {
    obj.clear();
  }

  return (obj.get());
}

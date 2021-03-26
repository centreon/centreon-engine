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

#include "com/centreon/engine/commands/command.hh"
#include <atomic>
#include <memory>
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/macros/grab.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

static std::atomic<uint64_t> _id{0};

command_map commands::command::commands;

/**
 *  Default constructor
 *
 *  @param[in] name         The command name.
 *  @param[in] command_line The command line.
 *  @param[in] listener     The command listener to catch events.
 */
commands::command::command(const std::string& name,
                           const std::string& command_line,
                           command_listener* listener)
    : _command_line(command_line), _listener{listener}, _name(name) {
  if (_name.empty())
    throw engine_error() << "Could not create a command with an empty name";
  if (_listener) {
    std::function<void()> f = [this] { _listener = nullptr; };
    _listener->reg(this, f);
  }
}

/**
 *  Destructor.
 */
commands::command::~command() noexcept {
  if (_listener) {
    _listener->unreg(this);
  }
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the same value.
 */
bool commands::command::operator==(command const& right) const noexcept {
  return _name == right._name && _command_line == right._command_line;
}

/**
 *  Compare two result.
 *
 *  @param[in] right The object to compare.
 *
 *  @return True if object have the different value.
 */
bool commands::command::operator!=(command const& right) const noexcept {
  return !operator==(right);
}

/**
 *  Get the command line.
 *
 *  @return The command line.
 */
const std::string& commands::command::get_command_line() const noexcept {
  return _command_line;
}

/**
 *  Get the command name.
 *
 *  @return The command name.
 */
const std::string& commands::command::get_name() const noexcept {
  return _name;
}

/**
 *  Set the command line.
 *
 *  @param[in] command_line The command line.
 */
void commands::command::set_command_line(const std::string& command_line) {
  _command_line = command_line;
}

/**
 *  Set the command listener.
 *
 *  @param[in] listener  The listener who catch events.
 */
void commands::command::set_listener(
    commands::command_listener* listener) noexcept {
  if (_listener)
    _listener->unreg(this);
  _listener = listener;
  if (_listener) {
    std::function<void()> f([this] { _listener = nullptr; });
    _listener->reg(this, f);
  }
}

/**
 *  Get the processed command line.
 *
 *  @param[in] macros The macros list.
 *
 *  @return The processed command line.
 */
std::string commands::command::process_cmd(nagios_macros* macros) const {
  std::string command_line;
  process_macros_r(macros, _command_line, command_line, 0);
  return command_line;
}

/**
 *  Get the unique command id.
 *
 *  @return The unique command id.
 */
uint64_t commands::command::get_uniq_id() {
  return ++_id;
}

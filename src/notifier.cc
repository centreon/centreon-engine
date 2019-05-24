/*
** Copyright 2011-2019 Centreon
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

#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/notifier.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

notifier::notifier(std::string const& display_name,
                   std::string const& check_command,
                   int initial_state,
                   double check_interval)
    : _display_name{display_name},
      _check_command{check_command},
      _initial_state{initial_state},
      _check_interval{check_interval} {
  if (check_interval < 0) {
    logger(log_config_error, basic)
        << "Error: Invalid check_interval value for notifier '" << display_name
        << "'";
    throw engine_error() << "Could not register notifier '" << display_name
                         << "'";
  }
}

std::string const& notifier::get_display_name() const {
  return _display_name;
}

void notifier::set_display_name(std::string const& display_name) {
  _display_name = display_name;
}

std::string const& notifier::get_check_command() const {
  return _check_command;
}

void notifier::set_check_command(std::string const& check_command) {
  _check_command = check_command;
}

int notifier::get_initial_state() const {
  return _initial_state;
}

void notifier::set_initial_state(int initial_state) {
  _initial_state = initial_state;
}

double notifier::get_check_interval() const {
  return _check_interval;
}

void notifier::set_check_interval(double check_interval) {
  _check_interval = check_interval;
}

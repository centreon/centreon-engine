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

#include "com/centreon/engine/retention/applier/utils.hh"
#include "com/centreon/engine/common.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::retention::applier;

/**
 *  Check if a command exist with the command line.
 *
 *  @param[in] command_line The full command line.
 *
 *  @return True if the command is find, otherwise false.
 */
bool utils::is_command_exist(std::string const& command_line) {
  std::size_t pos(command_line.find('!'));
  if (pos != std::string::npos) {
    std::string name(command_line.substr(0, pos));
    if (is_command_exist(name))
      return (true);
  }
  return (false);
}

/**
 *  Set the state history array.
 *
 *  @param[in]  values        The values to set.
 *  @param[out] state_history The array to fill.
 */
void utils::set_state_history(
    std::vector<int> const& values,
    std::array<int, MAX_STATE_HISTORY_ENTRIES>& state_history) {
  size_t end{MAX_STATE_HISTORY_ENTRIES};
  if (end > values.size())
    end = values.size();
  for (unsigned int i{0}; i < end; ++i)
    state_history[i] = values[i];
}

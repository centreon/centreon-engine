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

#ifndef CCE_RETENTION_APPLIER_UTILS_HH
#define CCE_RETENTION_APPLIER_UTILS_HH

#include <array>
#include <string>
#include <vector>
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace retention {
namespace applier {
namespace utils {
bool is_command_exist(std::string const& command_line);
void set_state_history(
    std::vector<int> const& values,
    std::array<int, MAX_STATE_HISTORY_ENTRIES>& state_history);
}  // namespace utils
}  // namespace applier
}  // namespace retention

CCE_END()

#endif  // !CCE_RETENTION_APPLIER_UTILS_HH

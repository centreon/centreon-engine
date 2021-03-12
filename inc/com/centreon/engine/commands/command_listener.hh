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

#ifndef CCE_COMMANDS_COMMAND_LISTENER_HH
#define CCE_COMMANDS_COMMAND_LISTENER_HH

#include <functional>
#include <unordered_map>

#include "com/centreon/engine/commands/result.hh"
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace commands {
  class command;
/**
 *  @class command_listener command_listener.hh
 *  @brief Notify command events.
 *
 *  This class provide interface to notify command events.
 */
class command_listener {
  std::unordered_map<command*, std::function<void()>> _clean_callbacks;

 public:
  virtual ~command_listener() noexcept {
    for (auto it = _clean_callbacks.begin(), end = _clean_callbacks.end();
        it != end; ++it) {
      (it->second)();
    }
  }

  virtual void finished(result const& res) noexcept = 0;
  void reg(command* const ptr, std::function<void()>& regf) {
    _clean_callbacks.insert({ptr, regf});
  }
  void unreg(command* const ptr) {
    _clean_callbacks.erase(ptr);
  }
};
}  // namespace commands

CCE_END()

#endif  // !CCE_COMMANDS_COMMAND_LISTENER_HH

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

#include "com/centreon/engine/commands/result.hh"
#include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace commands {
/**
 *  @class command_listener command_listener.hh
 *  @brief Notify command events.
 *
 *  This class provide interface to notify command events.
 */
class command_listener {
 public:
  virtual ~command_listener() throw() {}
  virtual void finished(result const& res) throw() = 0;
};
}  // namespace commands

CCE_END()

#endif  // !CCE_COMMANDS_COMMAND_LISTENER_HH

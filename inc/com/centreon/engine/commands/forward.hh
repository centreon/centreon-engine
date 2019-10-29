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

#ifndef CCE_COMMANDS_FORWARD_HH
#  define CCE_COMMANDS_FORWARD_HH

#  include <string>
#  include "com/centreon/engine/commands/command.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace              commands {
  /**
   *  @class forward commands/forward.hh
   *  @brief Command is a specific implementation of commands::command.
   *
   *  Command is a specific implementation of commands::command who
   *  provide forward, is more efficiente that a raw command.
   */
  class                forward
    : public command {
  public:
                       forward(
                         std::string const& command_name,
                         std::string const& command_line,
                         command& cmd);
                       forward(forward const& right);
                       ~forward() throw() override;
    forward&           operator=(forward const& right);
    commands::command* clone() const override;
    uint64_t           run(
                         std::string const& processed_cmd,
                         nagios_macros& macros,
                         uint32_t timeout) override;
    void               run(
                         std::string const& processed_cmd,
                         nagios_macros& macros,
                         uint32_t timeout,
                         result& res) override;

  private:
    void               _internal_copy(forward const& right);

    command*           _command;
  };
}

CCE_END()

#endif // !CCE_COMMANDS_FORWARD_HH

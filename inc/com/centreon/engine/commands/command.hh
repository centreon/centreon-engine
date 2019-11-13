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

#ifndef CCE_COMMANDS_COMMAND_HH
#define CCE_COMMANDS_COMMAND_HH

#include "com/centreon/engine/commands/command_listener.hh"
#include "com/centreon/engine/commands/result.hh"
#include "com/centreon/engine/macros/defines.hh"

CCE_BEGIN()
namespace commands {
class command;
}
CCE_END()

typedef std::unordered_map<
    std::string,
    std::shared_ptr<com::centreon::engine::commands::command> >
    command_map;

CCE_BEGIN()

namespace commands {
/**
 *  @class command command.hh
 *  @brief Execute command and send the result.
 *
 *  Command execute a command line with their arguments and
 *  notify listener at the end of the command.
 */
class command {
 public:
  command(std::string const& name,
          std::string const& command_line,
          command_listener* listener = NULL);
  virtual ~command() noexcept;
  bool operator==(command const& right) const noexcept;
  bool operator!=(command const& right) const noexcept;
  virtual command* clone() const = 0;
  virtual std::string const& get_command_line() const noexcept;
  virtual std::string const& get_name() const noexcept;
  virtual std::string process_cmd(nagios_macros* macros) const;
  virtual uint64_t run(std::string const& processed_cmd,
                       nagios_macros& macors,
                       uint32_t timeout) = 0;
  virtual void run(std::string const& process_cmd,
                   nagios_macros& macros,
                   uint32_t timeout,
                   result& res) = 0;
  virtual void set_command_line(std::string const& command_line);
  void set_listener(command_listener* listener) noexcept;
  static command_map commands;

 protected:
  command(command const& right);
  command& operator=(command const& right);
  static uint64_t get_uniq_id();

  std::string _command_line;
  command_listener* _listener;
  std::string _name;
};
}  // namespace commands

CCE_END()

#endif  // !CCE_COMMANDS_COMMAND_HH

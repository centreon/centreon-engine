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
#  define CCE_COMMANDS_COMMAND_HH

#  include "com/centreon/concurrency/mutex.hh"
#  include "com/centreon/engine/commands/command_listener.hh"
#  include "com/centreon/engine/commands/result.hh"
#  include "com/centreon/engine/macros/defines.hh"

CCE_BEGIN()

namespace                      commands {
  /**
   *  @class command command.hh
   *  @brief Execute command and send the result.
   *
   *  Command execute a command line with their arguments and
   *  notify listener at the end of the command.
   */
  class                        command {
  public:
    static command*            add_command(commands::command* obj);
                               command(
                                 std::string const& name,
                                 std::string const& command_line,
                                 command_listener* listener = NULL);
    virtual                    ~command() throw ();
    bool                       operator==(
                                 command const& right) const throw ();
    bool                       operator!=(
                                 command const& right) const throw ();
    virtual command*           clone() const = 0;
    virtual std::string const& get_command_line() const throw ();
    command_listener*          get_listener() const throw ();
    virtual std::string const& get_name() const throw ();
    virtual std::string        process_cmd(nagios_macros* macros) const;
    virtual unsigned long      run(
                                 std::string const& processed_cmd,
                                 nagios_macros& macors,
                                 unsigned int timeout) = 0;
    virtual void               run(
                                 std::string const& process_cmd,
                                 nagios_macros& macros,
                                 unsigned int timeout,
                                 result& res) = 0;
    virtual void               set_command_line(
                                 std::string const& command_line);
    void                       set_listener(
                                 command_listener* listener) throw ();

  protected:
                               command(command const& right);
    command&                   operator=(command const& right);
    static unsigned long       get_uniq_id();

    std::string                _command_line;
    command_listener*          _listener;
    std::string                _name;
  };
}

CCE_END()

#endif // !CCE_COMMANDS_COMMAND_HH

/*
** Copyright 2011-2012 Merethis
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

#ifndef CCE_COMMANDS_RESULT_HH
#  define CCE_COMMANDS_RESULT_HH

#  include <string>
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/timestamp.hh"

CCE_BEGIN()

namespace              commands {
  /**
   *  @class result result.hh
   *  @brief Result contain the result of execution process.
   *
   *  Result contain the result of execution process (output, retvalue,
   *  execution time).
   */
  class                result {
  public:
                       result(
                         unsigned long cmd_id = 0,
                         std::string const& stdout = "",
                         std::string const& stderr = "",
                         timestamp const& start_time = timestamp(),
                         timestamp const& end_time = timestamp(),
                         int retval = 0,
                         bool is_timeout = false,
                         bool is_executed = true);
                       result(result const& right);
                       ~result() throw ();
    result&            operator=(result const& right);
    bool               operator==(result const& right) const throw ();
    bool               operator!=(result const& right) const throw ();
    unsigned long      get_command_id() const throw ();
    timestamp const&   get_end_time() const throw ();
    unsigned int       get_execution_time() const throw ();
    int                get_exit_code() const throw ();
    bool               get_is_executed() const throw ();
    bool               get_is_timeout() const throw ();
    timestamp const&   get_start_time() const throw ();
    std::string const& get_stderr() const throw ();
    std::string const& get_stdout() const throw ();
    void               set_command_id(unsigned long id) throw ();
    void               set_end_time(timestamp const& time) throw ();
    void               set_exit_code(int retval) throw ();
    void               set_is_executed(bool value) throw ();
    void               set_is_timeout(bool value) throw ();
    void               set_start_time(timestamp const& time) throw ();
    void               set_stderr(std::string const& str);
    void               set_stdout(std::string const& str);

  private:
    void               _internal_copy(result const& right);

    unsigned long      _cmd_id;
    timestamp          _end_time;
    int                _exit_code;
    bool               _is_executed;
    bool               _is_timeout;
    timestamp          _start_time;
    std::string        _stderr;
    std::string        _stdout;
  };
}

CCE_END()

typedef com::centreon::engine::commands::result cce_commands_result;

#endif // !CCE_COMMANDS_RESULT_HH

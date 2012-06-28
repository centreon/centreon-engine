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

#ifndef CCE_COMMANDS_CONNECTOR_EXECUTE_QUERY_HH
#  define CCE_COMMANDS_CONNECTOR_EXECUTE_QUERY_HH

#  include <list>
#  include <string>
#  include "com/centreon/engine/commands/connector/request.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/timestamp.hh"

CCE_BEGIN()

namespace                commands {
  namespace              connector {
    /**
     *  @class execute_query commands/connector/execute_query.hh
     *  @brief Execute query send a command to the connector.
     *
     *  Execution query is a request to send a command to the
     *  connector.
     */
    class                execute_query : public request {
    public:
                         execute_query(
                           unsigned long cmd_id = 0,
                           std::string const& cmd = "",
                           timestamp const& start_time = timestamp(),
                           unsigned int timeout = 0);
                         execute_query(execute_query const& right);
                         ~execute_query() throw ();
      execute_query&     operator=(execute_query const& right);
      bool               operator==(
                           execute_query const& right) const throw ();
      bool               operator!=(
                           execute_query const& right) const throw ();
      std::string        build();
      request*           clone() const;
      std::list<std::string>
                         get_args() const throw ();
      std::string const& get_command() const throw ();
      unsigned long      get_command_id() const throw ();
      timestamp const&   get_start_time() const throw ();
      unsigned int       get_timeout() const throw ();
      void               restore(std::string const& data);

    private:
      void               _internal_copy(execute_query const& right);

      std::string        _cmd;
      unsigned long      _cmd_id;
      timestamp          _start_time;
      unsigned int       _timeout;
    };
  }
}

CCE_END()

#endif // !CCE_COMMANDS_CONNECTOR_EXECUTE_QUERY_HH

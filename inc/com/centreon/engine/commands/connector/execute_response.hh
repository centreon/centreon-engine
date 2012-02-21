/*
** Copyright 2011 Merethis
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

#ifndef CCE_COMMANDS_CONNECTOR_EXECUTE_RESPONSE_HH
# define CCE_COMMANDS_CONNECTOR_EXECUTE_RESPONSE_HH

# include <string>
# include <QDateTime>
# include "com/centreon/engine/commands/connector/request.hh"

namespace                     com {
  namespace                   centreon {
    namespace                 engine {
      namespace               commands {
	namespace             connector {
	/**
	 *  @class execute_response commands/connector/execute_response.hh
	 *  @brief Execute response is the result of the command execution.
	 *
	 *  Execute reponse is a request who send the result of the command
	 *  execution.
	 */
	  class               execute_response : public request {
	  public:
	                      execute_response(unsigned long cmd_id = 0,
					       bool is_executed = false,
					       int exit_code = 0,
					       QDateTime const& end_time = QDateTime(),
					       std::string const& stderr = "",
					       std::string const& stdout = "");
	                      execute_response(execute_response const& right);
	                      ~execute_response() throw();

	    execute_response& operator=(execute_response const& right);
	    bool              operator==(execute_response const& right) const throw();
	    bool              operator!=(execute_response const& right) const throw();

	    request*          clone() const;

	    std::string        build();
	    void              restore(std::string const& data);

	    std::string const&    get_stderr() const throw();
	    std::string const&    get_stdout() const throw();
	    QDateTime const&  get_end_time() const throw();
	    unsigned long     get_command_id() const throw();
	    int               get_exit_code() const throw();
	    bool              get_is_executed() const throw();

	  private:
	    std::string           _stderr;
	    std::string           _stdout;
	    QDateTime         _end_time;
	    unsigned long     _cmd_id;
	    int               _exit_code;
	    bool              _is_executed;
	  };
	}
      }
    }
  }
}

#endif // !CCE_COMMANDS_CONNECTOR_EXECUTE_RESPONSE_HH

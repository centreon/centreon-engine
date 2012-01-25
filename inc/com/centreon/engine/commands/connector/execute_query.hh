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

#ifndef CCE_COMMANDS_CONNECTOR_EXECUTE_QUERY_HH
# define CCE_COMMANDS_CONNECTOR_EXECUTE_QUERY_HH

# include <string>
# include <QStringList>
# include <QDateTime>
# include "com/centreon/engine/commands/connector/request.hh"

namespace                    com {
  namespace                  centreon {
    namespace                engine {
      namespace              commands {
	namespace            connector {
	/**
	 *  @class execute_query commands/connector/execute_query.hh
	 *  @brief Execute query send a command to the connector.
	 *
	 *  Execution query is a request to send a command to the
	 *  connector.
	 */
	  class              execute_query : public request {
	  public:
	                     execute_query(unsigned long cmd_id = 0,
					   std::string const& cmd = "",
					   QDateTime const& start_time = QDateTime(),
					   unsigned int timeout = 0);
	                     execute_query(execute_query const& right);
	                     ~execute_query() throw();

	    execute_query&   operator=(execute_query const& right);
	    bool             operator==(execute_query const& right) const throw();
	    bool             operator!=(execute_query const& right) const throw();

	    request*         clone() const;

	    QByteArray       build();
	    void             restore(QByteArray const& data);

	    std::string const&   get_command() const throw();
	    QStringList      get_args() const throw();
	    QDateTime const& get_start_time() const throw();
	    unsigned long    get_command_id() const throw();
	    unsigned int     get_timeout() const throw();

	  private:
	    std::string          _cmd;
	    QDateTime        _start_time;
	    unsigned long    _cmd_id;
	    unsigned int     _timeout;
	  };
	}
      }
    }
  }
}

#endif // !CCE_COMMANDS_CONNECTOR_EXECUTE_QUERY_HH

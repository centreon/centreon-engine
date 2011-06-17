/*
** Copyright 2011      Merethis
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

#ifndef CCE_COMMANDS_CONNECTOR_QUIT_RESPONSE_HH
# define CCE_COMMANDS_CONNECTOR_QUIT_RESPONSE_HH

# include "commands/connector/request.hh"

namespace                  com {
  namespace                centreon {
    namespace              engine {
      namespace            commands {
	namespace          connector {
	/**
	 *  @class quit_response commands/connector/quit_response.hh
	 *  @brief Quit response notify engine of when the connector finish.
	 */
	  class            quit_response : public request {
	  public:
	                   quit_response();
	                   quit_response(quit_response const& right);
	                   ~quit_response() throw();

	    quit_response& operator=(quit_response const& right);
	    bool           operator==(quit_response const& right) const throw();
	    bool           operator!=(quit_response const& right) const throw();

	    request*       clone() const;

	    QByteArray     build();
	    void           restore(QByteArray const& data);
	  };
	}
      }
    }
  }
}

#endif // !CCE_COMMANDS_CONNECTOR_QUIT_RESPONSE_HH

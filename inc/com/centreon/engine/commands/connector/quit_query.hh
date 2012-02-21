/*
** Copyright 2011 Merethis
**
** This file is part of Centreon Engine.
**
** Centreon Engine is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License version_query 2
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

#ifndef CCE_COMMANDS_CONNECTOR_QUIT_QUERY_HH
# define CCE_COMMANDS_CONNECTOR_QUIT_QUERY_HH

# include "com/centreon/engine/commands/connector/request.hh"

namespace               com {
  namespace             centreon {
    namespace           engine {
      namespace         commands {
	namespace       connector {
	/**
	 *  @class quit_query commands/connector/.hh
	 *  @brief Quit query ask connector to exit.
	 */
	  class         quit_query : public request {
	  public:
	                quit_query();
	                quit_query(quit_query const& right);
	                ~quit_query() throw();

	    quit_query& operator=(quit_query const& right);
	    bool        operator==(quit_query const& right) const throw();
	    bool        operator!=(quit_query const& right) const throw();

	    request*    clone() const;

	    std::string  build();
	    void        restore(std::string const& data);
	  };
	}
      }
    }
  }
}

#endif // !CCE_COMMANDS_CONNECTOR_QUIT_QUERY_HH

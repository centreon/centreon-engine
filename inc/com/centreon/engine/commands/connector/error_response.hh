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

#ifndef CCE_COMMANDS_CONNECTOR_ERROR_RESPONSE_HH
# define CCE_COMMANDS_CONNECTOR_ERROR_RESPONSE_HH

# include "com/centreon/engine/commands/connector/request.hh"

namespace                     com {
  namespace                   centreon {
    namespace                 engine {
      namespace               commands {
	namespace             connector {
	/**
	 *  @class error_response commands/connector/error_response.hh
	 *
	 *  Error response is the response request, who give the information
	 *  on error of connector.
	 */
	  class               error_response : public request {
	  public:
	    enum              e_code {
	      info = 0,
	      warning = 1,
	      error = 2
	    };

	                      error_response(std::string const& message = "",
					     e_code code = info);
	                      error_response(error_response const& right);
	                      ~error_response() throw();

	    error_response&   operator=(error_response const& right);
	    bool              operator==(error_response const& right) const throw();
	    bool              operator!=(error_response const& right) const throw();

	    request*          clone() const;

	    std::string        build();
	    void              restore(std::string const& data);

	    std::string const&    get_message() const throw();
	    e_code            get_code() const throw();

	  private:
	    std::string           _message;
	    e_code            _code;
	  };
	}
      }
    }
  }
}

#endif // !CCE_COMMANDS_CONNECTOR_ERROR_RESPONSE_HH

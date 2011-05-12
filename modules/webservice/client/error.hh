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

#ifndef CCE_MOD_WS_CLIENT_ERROR_HH
# define CCE_MOD_WS_CLIENT_ERROR_HH

# include <exception>

namespace                com {
  namespace              centreon {
    namespace            engine {
      namespace          modules {
	namespace        client {
	  /**
	   *  @class error error.hh
	   *  @brief Base exception class.
	   *
	   *  Simple exception class containing an error message and a flag to
	   *  determine if the error that generated the exception was either fatal
	   *  or not.
	   */
	  class          error : public std::exception {
	  public:
	                 error(char const* message) throw();
	                 error(error const& e) throw ();
	                 ~error() throw ();
	    error&       operator=(error const& e) throw ();
	    char const*  what() const throw ();

	  private:
	    char         _message[4096];
	  };
	}
      }
    }
  }
}

#endif // !CCE_MOD_WS_CLIENT_ERROR_HH

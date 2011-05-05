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

#ifndef TEST_LOGGING_COMMON_HH
# define TEST_LOGGING_COMMON_HH

# include "logging/object.hh"

namespace com {
  namespace centreon {
    namespace engine {
      namespace logging {
	/**
	 *  @class test test.hh
	 *  @brief Class test for testing logging system.
	 *
	 *  Simple Class for testing logging system.
	 */
	class test : public object {
	public:
	  test(std::string const& msg,
	       unsigned long long type,
	       unsigned int verbosity,
	       unsigned int total_call);

	  ~test();

	  void log(char const* message,
		   unsigned long long type,
		   unsigned int verbosity) throw();

	  static unsigned int get_nb_instance();

	private:
	  static unsigned int _nb_instance;

	  std::string         _msg;
	  unsigned long long  _type;
	  unsigned int        _verbosity;
	  unsigned int        _total_call;
	  unsigned int        _nb_call;
	};
      }
    }
  }
}

#endif // !TEST_LOGGING_COMMON_HH

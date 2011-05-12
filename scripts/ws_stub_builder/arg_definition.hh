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

#ifndef CCE_SCRIPT_ARG_DEFINITION_HH
# define CCE_SCRIPT_ARG_DEFINITION_HH

# include "argument.hh"

namespace                        com {
  namespace                      centreon {
    namespace                    engine {
      namespace                  script {
	/**
	 *  @class arg_definition arg_definition.hh
	 *  @brief Singleton to contain all arguments definition.
	 *
	 *  This class is a singleton with all arguments definition.
	 */
	class                    arg_definition {
	public:
	  static arg_definition& instance();


	  bool                   exist_argument(QString const& type) const;
	  argument const&        find_argument(QString const& type) const;
	  QList<argument> const& get_arguments() const throw();

	private:
	                         arg_definition();
	                         arg_definition(arg_definition const& right);
	                         ~arg_definition() throw();

	  arg_definition&        operator=(arg_definition const& right);

	  QList<argument>        _list;
	};
      }
    }
  }
}

#endif // !CCE_SCRIPT_ARG_DEFINITION_HH

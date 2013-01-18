/*
** Copyright 2011-2013 Merethis
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
#  define CCE_SCRIPT_ARG_DEFINITION_HH

#  include <list>
#  include "argument.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                      script {
  /**
   *  @class arg_definition arg_definition.hh
   *  @brief Singleton to contain all arguments definition.
   *
   *  This class is a singleton with all arguments definition.
   */
  class                        arg_definition {
  public:
    bool                       exist_argument(std::string const& type) const;
    argument const&            find_argument(std::string const& type) const;
    std::list<argument> const& get_arguments() const throw ();
    static arg_definition&     instance();

  private:
                               arg_definition();
                               arg_definition(arg_definition const& right);
                               ~arg_definition() throw ();
    arg_definition&            operator=(arg_definition const& right);

    std::list<argument>        _list;
  };
}

CCE_END()

#endif // !CCE_SCRIPT_ARG_DEFINITION_HH

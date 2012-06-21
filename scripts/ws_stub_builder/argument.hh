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

#ifndef CCE_SCRIPT_ARGUMENT_HH
#  define CCE_SCRIPT_ARGUMENT_HH

#  include <list>
#  include <string>
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                  script {
  /**
   *  @class argument argument.hh
   *  @brief Argument information class.
   *
   *  Argument provide basic information on variable type.
   */
  class                    argument {
  public:
                           argument(
                             std::string const& type = "",
                             std::string const& name = "",
                             std::string const& help = "",
                             bool is_optional = false,
                             bool is_array = false);
                           argument(argument const& right);
                           ~argument() throw ();
    argument&              operator=(argument const& right);
    bool                   operator==(
                             argument const& right) const throw ();
    bool                   operator!=(
                             argument const& right) const throw ();
    argument&              add(argument const& arg);
    std::list<argument> const&
                           get_args() const throw ();
    std::string const&     get_help() const throw ();
    std::string const&     get_name() const throw ();
    std::string const&     get_type() const throw ();
    bool                   is_array() const throw ();
    bool                   is_optional() const throw ();
    bool                   is_primitive() const throw ();
    argument&              set_help(std::string const& help);
    argument&              set_is_array(bool value) throw ();
    argument&              set_is_optional(bool value) throw ();
    argument&              set_name(std::string const& name);

  private:
    std::string            _help;
    bool                   _is_array;
    bool                   _is_optional;
    std::list<argument>    _list;
    std::string            _name;
    std::string            _type;
  };
}

CCE_END()

#endif // !CCE_SCRIPT_ARGUMENT_HH

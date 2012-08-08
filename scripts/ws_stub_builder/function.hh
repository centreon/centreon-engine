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

#ifndef CCE_SCRIPT_FUNCTION_HH
#  define CCE_SCRIPT_FUNCTION_HH

#  include <list>
#  include <string>
#  include "argument.hh"
#  include "arg_definition.hh"
#  include "com/centreon/engine/namespace.hh"

CCE_BEGIN()

namespace                 script {
  /**
   *  @class function function.hh
   *  @brief The function builder class.
   *
   *  This class build functions (help and execute) with their prototype.
   */
  class                   function {
  public:
                          function(std::string const& data = "");
                          function(function const& right);
                          ~function() throw ();
    function&             operator=(function const& right);
    void                  build();
    std::string const&    get_exec_function() const throw ();
    std::string           get_exec_name() const throw ();
    std::string const&    get_exec_prototype() const throw ();
    std::string const&    get_help_function() const throw ();
    std::string           get_help_name() const throw ();
    std::string const&    get_help_prototype() const throw ();
    std::string const&    get_name() const throw ();
    static bool           is_valid(std::string const& data) throw ();

  private:
    struct                arg_info {
      std::string         type;
      std::string         name;
      bool                is_pointer;
      bool                is_ref;
    };

    void                  _build_args_info(
                            std::string const& args_list);
    std::string           _build_exec_delete(
                            std::string const& base,
                            argument const& arg);
    void                  _build_exec_function();
    std::string           _build_exec_new(
                            std::string const& base,
                            argument const& arg);
    void                  _build_exec_prototype();
    std::string           _build_exec_struct(
                            std::string const& base,
                            argument const& arg);
    std::string           _build_help_args(argument const& arg) const;
    void                  _build_help_function();
    void                  _build_help_prototype();
    static std::string    _clean_function_name(std::string const& name);
    std::string           _get_string_method(std::string type) const;
    static std::string&   _replace(
                            std::string& str,
                            std::string const& old_str,
                            std::string const& new_str);

    std::list<arg_info>   _args_info;
    std::string           _data;
    arg_definition const& _def;
    std::string           _exec_function;
    std::string           _exec_prototype;
    std::string           _function;
    std::string           _help_function;
    std::string           _help_prototype;
    unsigned int          _list_pos;
    std::string           _new_function;
    static char const*    _pattern;
  };
}

CCE_END()

#endif // !CCE_SCRIPT_FUNCTION_HH

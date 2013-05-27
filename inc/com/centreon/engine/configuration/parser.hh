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

#ifndef CCE_CONFIGURATION_PARSER_HH
#  define CCE_CONFIGURATION_PARSER_HH

#  include <fstream>
#  include <string>
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/configuration/state.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/unordered_hash.hh"

CCE_BEGIN()

namespace       configuration {
  class         parser {
  public:
                parser();
                ~parser() throw ();
    void        parse(std::string const& path, state& config);

  private:
                parser(parser const& right);
    parser&     operator=(parser const& right);
    void        _apply(
                  std::list<std::string> const& lst,
                  void (parser::*pfunc)(std::string const&));
    template<typename T>
    static void _insert(
                  umap<std::size_t, shared_ptr<object> > const& from,
                  umap<std::size_t, shared_ptr<T> >& to);
    bool        _get_data(
                  std::string const& line,
                  std::string& key,
                  std::string& value,
                  char const* delim);
    bool        _get_next_line(
                  std::ifstream& stream,
                  std::string& line,
                  unsigned int& current_pos);
    void        _parse_directory_configuration(std::string const& path);
    void        _parse_global_configuration(std::string const& path);
    void        _parse_object_definitions(std::string const& path);
    void        _parse_resource_file(std::string const& path);
    void        _resolve_template();

    state*      _config;
    umap<std::size_t, umap<std::size_t, shared_ptr<object> > >
                _objects;
    umap<std::size_t, umap<std::string, shared_ptr<object> > >
                _templates;
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_PARSER_HH

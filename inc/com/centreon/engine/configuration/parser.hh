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
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/unordered_hash.hh"

CCE_BEGIN()

namespace             configuration {
  typedef umap<std::string, std::string> properties;

  class               parser {
  public:
                      parser();
                      ~parser() throw ();
    void              parse(std::string const& path);
    objects const&    get_objects(std::string const& type) const;
    properties const& get_globals() const throw ();

  private:
                      parser(parser const& right);
    parser&           operator=(parser const& right);
    bool              _get_next_line(
                        std::ifstream& stream,
                        std::string& line,
                        unsigned int& current_pos);
    void              _parse_main_configuration(
                        std::string const& path);
    void              _parse_object_definitions(
                        std::string const& path);
    void              _resolve_template();

    umap<std::string, objects>
                      _objects[2];
    properties        _properties;
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_PARSER_HH

/*
** Copyright 2011-2015 Merethis
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
#  include "com/centreon/engine/configuration/command.hh"
#  include "com/centreon/engine/configuration/connector.hh"
#  include "com/centreon/engine/configuration/file_info.hh"
#  include "com/centreon/engine/configuration/hostdependency.hh"
#  include "com/centreon/engine/configuration/host.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/configuration/servicedependency.hh"
#  include "com/centreon/engine/configuration/service.hh"
#  include "com/centreon/engine/configuration/state.hh"
#  include "com/centreon/engine/configuration/timeperiod.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/unordered_hash.hh"

CCE_BEGIN()

namespace              configuration {
  class                parser {
  public:
    enum               read_options {
      read_commands = (1 << 0),
      read_connector = (1 << 1),
      read_host = (1 << 2),
      read_hostdependency = (1 << 3),
      read_service = (1 << 4),
      read_servicedependency = (1 << 5),
      read_timeperiod = (1 << 6),
      read_all = (~0)
    };

                       parser(unsigned int read_options = read_all);
                       ~parser() throw ();
    void               parse(std::string const& path, state& config);

  private:
    typedef void (parser::*store)(object_ptr obj);

                       parser(parser const& right);
    parser&            operator=(parser const& right);
    void               _add_object(object_ptr obj);
    void               _add_template(object_ptr obj);
    void               _apply(
                         std::list<std::string> const& lst,
                         void (parser::*pfunc)(std::string const&));
    file_info const&   _get_file_info(object* obj) const;
    template<typename T>
    void               _get_objects_by_list_name(
                         list_string const& lst,
                         map_object& objects,
                         std::list<shared_ptr<T> >& out);


    template<typename T>
    static void        _insert(
                         list_object const& from,
                         std::set<shared_ptr<T> >& to);
    template<typename T>
    static void        _insert(
                         map_object const& from,
                         std::set<shared_ptr<T> >& to);
    std::string const& _map_object_type(
                         map_object const& objects) const throw ();
    void               _parse_directory_configuration(
                         std::string const& path);
    void               _parse_global_configuration(
                         std::string const& path,
                         bool is_main_file);
    void               _parse_object_definitions(std::string const& path);
    void               _resolve_template();
    void               _store_into_list(object_ptr obj);
    template<typename T, std::string const& (T::*ptr)() const throw ()>
    void               _store_into_map(object_ptr obj);

    state*             _config;
    unsigned int       _current_line;
    std::string        _current_path;
    list_object        _lst_objects[16];
    map_object         _map_objects[16];
    umap<object*, file_info>
                       _objects_info;
    unsigned int       _read_options;
    static store       _store[];
    map_object         _templates[16];
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_PARSER_HH

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
#  include "com/centreon/engine/configuration/command.hh"
#  include "com/centreon/engine/configuration/connector.hh"
#  include "com/centreon/engine/configuration/contact.hh"
#  include "com/centreon/engine/configuration/hostdependency.hh"
#  include "com/centreon/engine/configuration/hostescalation.hh"
#  include "com/centreon/engine/configuration/hostextinfo.hh"
#  include "com/centreon/engine/configuration/host.hh"
#  include "com/centreon/engine/configuration/object.hh"
#  include "com/centreon/engine/configuration/servicedependency.hh"
#  include "com/centreon/engine/configuration/serviceescalation.hh"
#  include "com/centreon/engine/configuration/serviceextinfo.hh"
#  include "com/centreon/engine/configuration/service.hh"
#  include "com/centreon/engine/configuration/state.hh"
#  include "com/centreon/engine/configuration/timeperiod.hh"
#  include "com/centreon/engine/namespace.hh"
#  include "com/centreon/unordered_hash.hh"

CCE_BEGIN()

namespace              configuration {
  class                parser {
  public:
                       parser();
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
    void               _apply_hostextinfo();
    void               _apply_serviceextinfo();
    void               _check_command_dependencies();
    void               _check_contact_dependencies();
    void               _check_contactgroup_dependencies();
    void               _check_dependencies();
    void               _check_hostdependency_dependencies();
    void               _check_hostescalation_dependencies();
    void               _check_hostextinfo_dependencies();
    void               _check_hostgroup_dependencies();
    template<typename T, list_string const& (T::*get_list)() const throw (), std::string const& (T::*get_name)() const throw ()>
    void               _check_objects_dependencies(
                         map_object const& objects,
                         map_object const& dependencies);
    template<typename T, std::string const& (T::*get_data)() const throw (), std::string const& (T::*get_name)() const throw ()>
    void               _check_objects_dependencies(
                         map_object const& objects,
                         map_object const& dependencies);
    template<typename T, list_string const& (T::*get_list)() const throw ()>
    void               _check_objects_dependencies(
                         list_object const& objects,
                         map_object const& dependencies);
    template<typename T, std::string const& (T::*get_data)() const throw ()>
    void               _check_objects_dependencies(
                         list_object const& objects,
                         map_object const& dependencies);
    void               _check_servicedependency_dependencies();
    void               _check_serviceescalation_dependencies();
    void               _check_serviceextinfo_dependencies();
    void               _check_servicegroup_dependencies();
    void               _get_hosts_by_hostgroups(
                         hostgroup_ptr const& hostgroups,
                         list_host& hosts);
    void               _get_hosts_by_hostgroups_name(
                         list_string const& lst_group,
                         list_host& hosts);
    template<typename T>
    void               _get_objects_by_list_name(
                         list_string const& lst,
                         map_object& objects,
                         std::list<shared_ptr<T> >& out);


    template<typename T>
    static void        _insert(
                         list_object const& from,
                         std::list<shared_ptr<T> >& to);
    template<typename T>
    static void        _insert(
                         map_object const& from,
                         std::list<shared_ptr<T> >& to);
    bool               _get_data(
                         std::string const& line,
                         std::string& key,
                         std::string& value,
                         char const* delim);
    bool               _get_next_line(
                         std::ifstream& stream,
                         std::string& line);
    std::string const& _map_object_type(
                         map_object const& objects) const throw ();
    void               _parse_directory_configuration(std::string const& path);
    void               _parse_global_configuration(std::string const& path);
    void               _parse_object_definitions(std::string const& path);
    void               _parse_resource_file(std::string const& path);
    void               _resolve_template();
    void               _store_into_list(object_ptr obj);
    template<typename T, std::string const& (T::*ptr)() const throw ()>
    void               _store_into_map(object_ptr obj);

    state*             _config;
    unsigned int       _current_line;
    std::string        _current_path;
    list_object        _lst_objects[15];
    map_object         _map_objects[15];
    static store       _store[];
    map_object         _templates[15];
  };
}

CCE_END()

#endif // !CCE_CONFIGURATION_PARSER_HH

/*
** Copyright 2011-2014,2017 Centreon
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

#include <memory>
#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/string.hh"
#include "com/centreon/io/directory_entry.hh"

using namespace com::centreon;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::io;

parser::store parser::_store[] = {
  &parser::_store_into_map<command, &command::command_name>,
  &parser::_store_into_map<connector, &connector::connector_name>,
  &parser::_store_into_map<contact, &contact::contact_name>,
  &parser::_store_into_map<contactgroup, &contactgroup::contactgroup_name>,
  &parser::_store_into_map<host, &host::host_name>,
  &parser::_store_into_list,
  &parser::_store_into_list,
  &parser::_store_into_list,
  &parser::_store_into_map<hostgroup, &hostgroup::hostgroup_name>,
  &parser::_store_into_list,
  &parser::_store_into_list,
  &parser::_store_into_list,
  &parser::_store_into_list,
  &parser::_store_into_map<servicegroup, &servicegroup::servicegroup_name>,
  &parser::_store_into_map<timeperiod, &timeperiod::timeperiod_name>
};

/**
 *  Default constructor.
 *
 *  @param[in] read_options Configuration file reading options
 *             (use to skip some object type).
 */
parser::parser(unsigned int read_options)
  : _config(NULL),
    _read_options(read_options) {}

/**
 *  Destructor.
 */
parser::~parser() throw () {}

/**
 *  Parse configuration file.
 *
 *  @param[in] path   The configuration file path.
 *  @param[in] config The state configuration to fill.
 */
void parser::parse(std::string const& path, state& config) {
  _config = &config;

  // parse the global configuration file.
  _parse_global_configuration(path);

  // parse configuration files.
  _apply(config.cfg_file(), &parser::_parse_object_definitions);
  // parse resource files.
  _apply(config.resource_file(), &parser::_parse_resource_file);
  // parse configuration directories.
  _apply(config.cfg_dir(), &parser::_parse_directory_configuration);

  // Apply template.
  _resolve_template();

  // Apply extended info.
  _apply_hostextinfo();
  _apply_serviceextinfo();

  // Fill state.
  _insert(_map_objects[object::command], config.commands());
  _insert(_map_objects[object::connector], config.connectors());
  _insert(_map_objects[object::contact], config.contacts());
  _insert(_map_objects[object::contactgroup], config.contactgroups());
  _insert(_lst_objects[object::hostdependency], config.hostdependencies());
  _insert(_lst_objects[object::hostescalation], config.hostescalations());
  _insert(_map_objects[object::hostgroup], config.hostgroups());
  _insert(_map_objects[object::host], config.hosts());
  _insert(_lst_objects[object::servicedependency], config.servicedependencies());
  _insert(_lst_objects[object::serviceescalation], config.serviceescalations());
  _insert(_map_objects[object::servicegroup], config.servicegroups());
  _insert(_lst_objects[object::service], config.services());
  _insert(_map_objects[object::timeperiod], config.timeperiods());

  // cleanup.
  _objects_info.clear();
  for (unsigned int i(0);
       i < sizeof(_lst_objects) / sizeof(_lst_objects[0]);
       ++i) {
    _lst_objects[i].clear();
    _map_objects[i].clear();
    _templates[i].clear();
  }
}

/**
 *  Add object into the list.
 *
 *  @param[in] obj The object to add into the list.
 */
void parser::_add_object(object_ptr obj) {
  if (obj->should_register())
    (this->*_store[obj->type()])(obj);
  return ;
}

/**
 *  Add template into the list.
 *
 *  @param[in] obj The tempalte to add into the list.
 */
void parser::_add_template(object_ptr obj) {
  std::string const& name(obj->name());
  if (name.empty())
    throw (engine_error() << "Parsing of "
           << obj->type_name() << " failed "
           << _get_file_info(obj.get()) << ": Property 'name' "
           "is missing");
  map_object& tmpl(_templates[obj->type()]);
  if (tmpl.find(name) != tmpl.end())
    throw (engine_error() << "Parsing of "
           << obj->type_name() << " failed "
           << _get_file_info(obj.get()) << ": " << name
           << " already exists");
  tmpl[name] = obj;
}

/**
 *  Apply parse method into list.
 *
 *  @param[in] lst   The list to apply action.
 *  @param[in] pfunc The method to apply.
 */
void parser::_apply(
       std::list<std::string> const& lst,
       void (parser::*pfunc)(std::string const&)) {
  for (std::list<std::string>::const_iterator
         it(lst.begin()), end(lst.end());
       it != end;
       ++it)
    (this->*pfunc)(*it);
}

/**
 *  Apply the host extended info.
 *
 *  @warning This function is for compatibility and has very
 *           poor performance. Didn't use extended info. If you
 *           want to use the generic template system.
 */
void parser::_apply_hostextinfo() {
  map_object& gl_hosts(_map_objects[object::host]);
  list_object const& hostextinfos(_lst_objects[object::hostextinfo]);
  for (list_object::const_iterator
         it(hostextinfos.begin()), end(hostextinfos.end());
       it != end;
       ++it) {
    // Get the current hostextinfo to check.
    hostextinfo_ptr obj(std::static_pointer_cast<hostextinfo>(*it));

    list_host hosts;
    _get_objects_by_list_name(obj->hosts(), gl_hosts, hosts);
    _get_hosts_by_hostgroups_name(obj->hostgroups(), hosts);

    for (list_host::iterator it(hosts.begin()), end(hosts.end());
         it != end;
         ++it)
      it->merge(*obj);
  }
}

/**
 *  Apply the service extended info.
 *
 *  @warning This function is for compatibility and has very
 *           poor performance. Didn't use extended info. If you
 *           want to use the generic template system.
 */
void parser::_apply_serviceextinfo() {
  map_object& gl_hosts(_map_objects[object::host]);
  list_object& gl_services(_lst_objects[object::service]);
  list_object const& serviceextinfos(_lst_objects[object::serviceextinfo]);
  for (list_object::const_iterator
         it(serviceextinfos.begin()), end(serviceextinfos.end());
       it != end;
       ++it) {
    // Get the current serviceextinfo to check.
    serviceextinfo_ptr obj(std::static_pointer_cast<serviceextinfo>(*it));

    list_host hosts;
    _get_objects_by_list_name(obj->hosts(), gl_hosts, hosts);
    _get_hosts_by_hostgroups_name(obj->hostgroups(), hosts);

    for (list_object::iterator
           it(gl_services.begin()), end(gl_services.end());
         it != end;
         ++it) {
      service_ptr svc(std::static_pointer_cast<service>(*it));
      if (svc->service_description() != obj->service_description())
        continue;

      list_host svc_hosts;
      _get_objects_by_list_name(svc->hosts(), gl_hosts, svc_hosts);
      _get_hosts_by_hostgroups_name(svc->hostgroups(), svc_hosts);

      bool found(false);
      for (list_host::const_iterator
             it_host(hosts.begin()),
             end_host(hosts.end());
           !found && it_host != end_host;
           ++it_host) {
        for (list_host::const_iterator
               it_svc_host(svc_hosts.begin()),
               end_svc_host(svc_hosts.end());
             it_svc_host != end_svc_host;
             ++it_svc_host) {
          if (it_host->host_name() == it_svc_host->host_name()) {
            svc->merge(*obj);
            found = true;
          }
        }
      }
    }
  }
}

/**
 *  Get the file information.
 *
 *  @param[in] obj The object to get file informations.
 *
 *  @return The file informations object.
 */
file_info const& parser::_get_file_info(object* obj) const {
  if (obj) {
    std::unordered_map<object*, file_info>::const_iterator it(_objects_info.find(obj));
    if (it != _objects_info.end())
      return (it->second);
  }
  throw (engine_error() << "Parsing failed: Object not "
                           "found into the file information cache");
}

/**
 *  Build the hosts list with hostgroups.
 *
 *  @param[in]     hostgroups The hostgroups.
 *  @param[in,out] hosts      The host list to fill.
 */
void parser::_get_hosts_by_hostgroups(
       hostgroup const& hostgroups,
       list_host& hosts) {
  _get_objects_by_list_name(
    hostgroups.members(),
    _map_objects[object::host],
    hosts);
  _get_hosts_by_hostgroups_name(hostgroups.hostgroup_members(), hosts);
}

/**
 *  Build the hosts list with list of hostgroups.
 *
 *  @param[in]     hostgroups The hostgroups list.
 *  @param[in,out] hosts      The host list to fill.
 */
void parser::_get_hosts_by_hostgroups_name(
       set_string const& lst_group,
       list_host& hosts) {
  map_object& gl_hostgroups(_map_objects[object::hostgroup]);
  for (set_string::const_iterator
         it(lst_group.begin()),
         end(lst_group.end());
       it != end;
       ++it) {
    map_object::iterator it_hostgroups(gl_hostgroups.find(*it));
    if (it_hostgroups != gl_hostgroups.end())
      _get_hosts_by_hostgroups(
        *static_cast<configuration::hostgroup*>(
           it_hostgroups->second.get()),
        hosts);
  }
}

/**
 *  Build the object list with list of object name.
 *
 *  @param[in]     lst     The object name list.
 *  @param[in]     objects The object map to find object name.
 *  @param[in,out] out     The list to fill.
 */
template<typename T>
void parser::_get_objects_by_list_name(
       set_string const& lst,
       map_object& objects,
       std::list<T>& out) {
  for (set_string::const_iterator it(lst.begin()), end(lst.end());
       it != end;
       ++it) {
    map_object::iterator it_obj(objects.find(*it));
    if (it_obj != objects.end())
      out.push_back(*static_cast<T*>(it_obj->second.get()));
  }
}

/**
 *  Insert objects into type T list and sort the new list by object id.
 *
 *  @param[in]  from The objects source.
 *  @param[out] to   The objects destination.
 */
template<typename T>
void parser::_insert(
       list_object const& from,
       std::set<T>& to) {
  for (list_object::const_iterator it(from.begin()), end(from.end());
       it != end;
       ++it)
    to.insert(*static_cast<T const*>(it->get()));
  return ;
}

/**
 *  Insert objects into type T list and sort the new list by object id.
 *
 *  @param[in]  from The objects source.
 *  @param[out] to   The objects destination.
 */
template<typename T>
void parser::_insert(
       map_object const& from,
       std::set<T>& to) {
  for (map_object::const_iterator it(from.begin()), end(from.end());
       it != end;
       ++it)
    to.insert(*static_cast<T*>(it->second.get()));
  return ;
}

/**
 *  Get the map object type name.
 *
 *  @param[in] objects  The map object.
 *
 *  @return The type name.
 */
std::string const& parser::_map_object_type(
                     map_object const& objects) const throw () {
  static std::string const empty("");
  map_object::const_iterator it(objects.begin());
  if (it == objects.end())
    return (empty);
  return (it->second->type_name());
}

/**
 *  Parse the directory configuration.
 *
 *  @param[in] path The directory path.
 */
void parser::_parse_directory_configuration(std::string const& path) {
  directory_entry dir(path);
  std::list<file_entry> const& lst(dir.entry_list("*.cfg"));
  for (std::list<file_entry>::const_iterator
         it(lst.begin()), end(lst.end());
       it != end;
       ++it)
    _parse_object_definitions(it->path());
}

/**
 *  Parse the global configuration file.
 *
 *  @param[in] path The configuration path.
 */
void parser::_parse_global_configuration(std::string const& path) {
  logger(logging::log_info_message, logging::most)
    << "Reading main configuration file '" << path << "'.";

  std::ifstream stream(path.c_str(), std::ios::binary);
  if (!stream.is_open())
    throw (engine_error() << "Parsing of global "
           "configuration failed: Can't open file '" << path << "'");

  _config->cfg_main(path);

  _current_line = 0;
  _current_path = path;

  std::string input;
  while (string::get_next_line(stream, input, _current_line)) {
    char const* key;
    char const* value;
    if (!string::split(input, &key, &value, '=')
        || !_config->set(key, value))
      throw (engine_error() << "Parsing of global "
                "configuration failed in file '" << path << "' on line "
             << _current_line << ": Invalid line '" << input << "'");
  }
}

/**
 *  Parse the object definition file.
 *
 *  @param[in] path The object definitions path.
 */
void parser::_parse_object_definitions(std::string const& path) {
  logger(logging::log_info_message, logging::basic)
    << "Processing object config file '" << path << "'";

  std::ifstream stream(path.c_str(), std::ios::binary);
  if (!stream.is_open())
    throw (engine_error() << "Parsing of object definition failed: "
           << "Can't open file '" << path << "'");

  _current_line = 0;
  _current_path = path;

  bool parse_object(false);
  object_ptr obj;
  std::string input;
  while (string::get_next_line(stream, input, _current_line)) {
    // Multi-line.
    while ('\\' == input[input.size() - 1]) {
      input.resize(input.size() - 1);
      std::string addendum;
      if (!string::get_next_line(stream, addendum, _current_line))
        break ;
      input.append(addendum);
    }

    // Check if is a valid object.
    if (obj == nullptr) {
      if (input.find("define") || !std::isspace(input[6]))
        throw (engine_error() << "Parsing of object definition failed "
               << "in file '" << _current_path << "' on line "
               << _current_line << ": Unexpected start definition");
      string::trim_left(input.erase(0, 6));
      std::size_t last(input.size() - 1);
      if (input.empty() || input[last] != '{')
        throw (engine_error() << "Parsing of object definition failed "
               << "in file '" << _current_path << "' on line "
               << _current_line << ": Unexpected start definition");
      std::string const& type(string::trim_right(input.erase(last)));
      obj = object::create(type);
      if (obj == nullptr)
        throw (engine_error() << "Parsing of object definition failed "
               << "in file '" << _current_path << "' on line "
               << _current_line << ": Unknown object type name '"
               << type << "'");
      parse_object = (_read_options & (1 << obj->type()));
      _objects_info[obj.get()] = file_info(path, _current_line);
    }
    // Check if is the not the end of the current object.
    else if (input != "}") {
      if (parse_object) {
        if (!obj->parse(input))
          throw (engine_error() << "Parsing of object definition "
                 << "failed in file '" << _current_path << "' on line "
                 << _current_line << ": Invalid line '"
                 << input << "'");
      }
    }
    // End of the current object.
    else {
      if (parse_object) {
        if (!obj->name().empty())
          _add_template(obj);
        if (obj->should_register())
          _add_object(obj);
      }
      obj.reset();
    }
  }
}

/**
 *  Parse the resource file.
 *
 *  @param[in] path The resource file path.
 */
void parser::_parse_resource_file(std::string const& path) {
  logger(logging::log_info_message, logging::most)
    << "Reading resource file '" << path << "'";

  std::ifstream stream(path.c_str(), std::ios::binary);
  if (!stream.is_open())
    throw (engine_error() << "Parsing of resource file failed: "
           << "can't open file '" << path << "'");

  _current_line = 0;
  _current_path = path;

  std::string input;
  while (string::get_next_line(stream, input, _current_line)) {
    try {
      std::string key;
      std::string value;
      if (!string::split(input, key, value, '='))
        throw (engine_error() << "Parsing of resource file '"
               << _current_path << "' failed on line " << _current_line
               << ": Invalid line '" << input << "'");
      _config->user(key, value);
    }
    catch (std::exception const& e) {
      (void)e;
      throw (engine_error() << "Parsing of resource file '"
             << _current_path << "' failed on line " << _current_line
             << ": Invalid line '" << input << "'");
    }
  }
}

/**
 *  Resolve template for register objects.
 */
void parser::_resolve_template() {
  for (unsigned int i(0);
       i < sizeof(_templates) / sizeof(_templates[0]);
       ++i) {
    map_object& templates(_templates[i]);
    for (map_object::iterator
           it(_templates[i].begin()), end(_templates[i].end());
         it != end;
         ++it)
      it->second->resolve_template(templates);
  }

  for (unsigned int i(0);
       i < sizeof(_lst_objects) / sizeof(_lst_objects[0]);
       ++i) {
    map_object& templates(_templates[i]);
    for (list_object::iterator
           it(_lst_objects[i].begin()), end(_lst_objects[i].end());
         it != end;
         ++it) {
      (*it)->resolve_template(templates);
      try {
        (*it)->check_validity();
      }
      catch (std::exception const& e) {
        throw (engine_error() << "Configuration parsing failed "
               << _get_file_info(it->get()) << ": " << e.what());
      }
    }
  }

  for (unsigned int i(0);
       i < sizeof(_map_objects) / sizeof(_map_objects[0]);
       ++i) {
    map_object& templates(_templates[i]);
    for (map_object::iterator
           it(_map_objects[i].begin()), end(_map_objects[i].end());
         it != end;
         ++it) {
      it->second->resolve_template(templates);
      try {
        it->second->check_validity();
      }
      catch (std::exception const& e) {
        throw (engine_error() << "Configuration parsing failed "
               << _get_file_info(it->second.get()) << ": " << e.what());
      }
    }
  }
}

/**
 *  Store object into the list.
 *
 *  @param[in] obj The object to store.
 */
void parser::_store_into_list(object_ptr obj) {
  _lst_objects[obj->type()].push_back(obj);
}

/**
 *  Store object into the map.
 *
 *  @param[in] obj The object to store.
 */
template<typename T, std::string const& (T::*ptr)() const throw ()>
void parser::_store_into_map(object_ptr obj) {
  std::shared_ptr<T> real(std::static_pointer_cast<T>(obj));
  map_object::iterator
    it(_map_objects[obj->type()].find((real.get()->*ptr)()));
  if (it != _map_objects[obj->type()].end())
    throw (engine_error() << "Parsing of " << obj->type_name()
           << " failed " << _get_file_info(obj.get()) << ": "
           << obj->name() << " alrealdy exists");
  _map_objects[obj->type()][(real.get()->*ptr)()] = real;
}

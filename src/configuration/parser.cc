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

#include "com/centreon/engine/configuration/parser.hh"
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/io/directory_entry.hh"

using namespace com::centreon;
using namespace com::centreon::engine::configuration;
using namespace com::centreon::io;

/**
 *  Default constructor.
 */
parser::parser()
  : _config(NULL) {

}

/**
 *  Destructor.
 */
parser::~parser() throw () {

}

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

  _insert(_objects[object::command], config.commands());
  _insert(_objects[object::connector], config.connectors());
  _insert(_objects[object::contact], config.contacts());
  _insert(_objects[object::contactgroup], config.contactgroups());
  _insert(_objects[object::hostdependency], config.hostdependencies());
  _insert(_objects[object::hostescalation], config.hostescalations());
  _insert(_objects[object::hostextinfo], config.hostextinfos());
  _insert(_objects[object::hostgroup], config.hostgroups());
  _insert(_objects[object::host], config.hosts());
  _insert(_objects[object::servicedependency], config.servicedependencies());
  _insert(_objects[object::serviceescalation], config.serviceescalations());
  _insert(_objects[object::serviceextinfo], config.serviceextinfos());
  _insert(_objects[object::servicegroup], config.servicegroups());
  _insert(_objects[object::service], config.services());
  _insert(_objects[object::timeperiod], config.timeperiods());

  // cleanup.
  _objects.clear();
  _templates.clear();
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
 *  Insert objects into type T map.
 *
 *  @param[in]  from The objects source.
 *  @param[out] to   The objects destination.
 */
template<typename T>
void parser::_insert(
       map_object const& from,
       umap<std::size_t, shared_ptr<T> >& to) {
  for (map_object::const_iterator it(from.begin()), end(from.end());
       it != end;
       ++it)
    to[it->first] = it->second;
}

/**
 *  Get key and value from line.
 *
 *  @param[in]  line  The line to extract data.
 *  @param[out] key   The key to fill.
 *  @param[out] value The value to fill.
 *  @param[in]  delim The delimiter.
 */
bool parser::_get_data(
       std::string const& line,
       std::string& key,
       std::string& value,
       char const* delim) {
  std::size_t pos(line.find_first_of(delim, 0));
  if (pos == std::string::npos)
    return (false);

  key = line.substr(0, pos);
  misc::trim(key);

  value = line.substr(pos + 1);
  misc::trim(value);
  return (true);
}

/**
 *  Get the next valid line.
 *
 *  @param[in, out] stream      The current stream to read new line.
 *  @param[out]     line        The line to fill.
 *  @param[out]     current_pos The current line.
 *
 *  @return True if data is available, false if no data.
 */
bool parser::_get_next_line(
       std::ifstream& stream,
       std::string& line,
       unsigned int& current_pos) {
  while (std::getline(stream, line, '\n')) {
    ++current_pos;
    misc::trim(line);
    if (!line.empty()) {
      char c(line[0]);
      if (c != '#' && c != ';' && c != '\x0')
        return (true);
    }
  }
  return (false);
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
  std::ifstream stream(path.c_str());
  if (!stream.is_open())
    throw (engine_error() << "configuration: parse global "
           "configuration failed: can't open file '" << path << "'");

  _config->cfg_main(path);

  unsigned int current_line(0);
  std::string input;
  while (_get_next_line(stream, input, current_line)) {
    std::string key;
    std::string value;
    if (!_get_data(input, key, value, "=") || !_config->set(key, value))
      throw (engine_error() << "configuration: parse global "
             "configuration failed: invalid line "
             "'" << input << "' in file '" << path << "' "
             "on line " << current_line);
  }
}

/**
 *  Parse the object definition file.
 *
 *  @param[in] path The object definitions path.
 */
void parser::_parse_object_definitions(std::string const& path) {
  std::ifstream stream(path.c_str());
  if (!stream.is_open())
    throw (engine_error() << "configuration: parse object "
           "definitions failed: can't open file '" << path << "'");

  shared_ptr<object> obj;
  unsigned int current_line(0);
  std::string input;
  while (_get_next_line(stream, input, current_line)) {
    // Check if is a valid object.
    if (obj.is_null()) {
      if (input.find("define") || !std::isspace(input[6]))
        throw (engine_error() << "configuration: parse object "
               "definitions failed: unexpected start definition in "
               "file '" << path << "' on line " << current_line);
      misc::trim_left(input.erase(0, 6));
      std::size_t last(input.size() - 1);
      if (input.empty() || input[last] != '{')
        throw (engine_error() << "configuration: parse object "
               "definitions failed: unexpected start definition in "
               "file '" << path << "' on line " << current_line);
      std::string const& type(misc::trim_right(input.erase(last)));
      obj = object::create(type);
      if (obj.is_null())
        throw (engine_error() << "configuration: parse object "
               "definitions failed: unknown object type name "
               "'" << type << "' in file '" << path << "' on line "
               << current_line);
    }
    // Check if is the not the end of the current object.
    else if (input != "}") {
      if (!obj->parse(input))
        throw (engine_error() << "configuration: parse object "
               "definitions failed: invalid line "
               "'" << input << "' in file '" << path << "' "
               "on line " << current_line);
    }
    // End of the current object.
    else {
      object::object_type type(obj->type());
      // Add template.
      if (obj->is_template()) {
        std::string const& name(obj->name());
        if (name.empty())
          throw (engine_error() << "configuration: parse object "
                 "definition failed: property 'name' is missing in "
                 "file '" << path << "' on line " << current_line);
        _templates[type][name] = obj;
      }
      // Add object.
      else {
        std::size_t id(obj->id());
        if (!id)
          throw (engine_error() << "configuration: parse object "
                 "definition failed: property missing in "
                 "file '" << path << "' on line " << current_line);
        _objects[type][id] = obj;
      }
      obj.clear();
    }
  }
}

/**
 *  Parse the resource file.
 *
 *  @param[in] path The resource file path.
 */
void parser::_parse_resource_file(std::string const& path) {
  std::ifstream stream(path.c_str());
  if (!stream.is_open())
    throw (engine_error() << "configuration: parse resources "
           "configuration failed: can't open file '" << path << "'");

  unsigned int current_line(0);
  std::string input;
  while (_get_next_line(stream, input, current_line)) {
    try {
      std::string key;
      std::string value;
      if (!_get_data(input, key, value, "="))
        throw (engine_error());
      _config->user(key, value);
    }
    catch (std::exception const& e) {
      (void)e;
      throw (engine_error() << "configuration: parse resources "
             "configuration failed: invalid line "
             "'" << input << "' in file '" << path << "' "
             "on line " << current_line);
    }
  }
}

/**
 *  Resolve template for register objects.
 */
void parser::_resolve_template() {
  for (umap<std::size_t, map_object>::iterator
         it(_objects.begin()), end(_objects.end());
       it != end;
       ++it) {
    map_template& templates(_templates[it->first]);
    map_object& objects(it->second);
    for (map_object::iterator it(objects.begin()), end(objects.end());
         it != end;
         ++it)
      it->second->resolve_template(templates);
  }
}

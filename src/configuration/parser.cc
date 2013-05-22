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

using namespace com::centreon::engine::configuration;

/**
 *  Default constructor.
 */
parser::parser() {

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
  // parse the global configuration file.
  _parse_global_configuration(path, config);

  // parse all configuration file define into the global configuration.
  std::list<std::string> const& cfg_file(config.cfg_file());
  for (std::list<std::string>::const_iterator
         it(cfg_file.begin()), end(cfg_file.end());
       it != end;
       ++it)
    _parse_object_definitions(*it);

  // parse all configuration directory define into the
  // global configuration.
  std::list<std::string> const& cfg_dir(config.cfg_dir());
  for (std::list<std::string>::const_iterator
         it(cfg_dir.begin()), end(cfg_dir.end());
       it != end;
       ++it)
    ; // XXX: _parse_object_definitions(*it);

  // parse all resource file define into the global configuration.
  std::list<std::string> const& cfg_resource(config.resource_file());
  for (std::list<std::string>::const_iterator
         it(cfg_resource.begin()), end(cfg_resource.end());
       it != end;
       ++it)
    _parse_resource_file(*it);

  // Apply template.
  _resolve_template();

  // cleanup.
  for (unsigned int i(0);
       i < sizeof(_objects) / sizeof(_objects[0]);
       ++i)
    _objects[i].clear();
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
 *  Parse the global configuration file.
 *
 *  @param[in] path   The configuration path.
 *  @param[in] config The state configuration to fill.
 */
void parser::_parse_global_configuration(
       std::string const& path,
       state& config) {
  std::ifstream stream(path.c_str());
  if (!stream.is_open())
    throw (engine_error() << "configuration: parse global "
           "configuration failed: can't open file '" << path << "'");

  unsigned int current_line(0);
  std::string input;
  while (_get_next_line(stream, input, current_line)) {
    std::size_t pos(input.find_first_of("=", 0));
    if (pos == std::string::npos)
      throw (engine_error() << "configuration: parse global "
             "configuration failed: invalid line "
             "'" << input << "' in file '" << path << "' "
             "on line " << current_line);
    std::string key(input.substr(0, pos));
    std::string value(input.substr(pos + 1));
    if (!config.set(misc::trim(key), misc::trim(value)))
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
      std::string const& name(obj->name());
      if (name.empty())
        throw (engine_error() << "configuration: parse object "
               "definition failed: property 'name' is missing in "
               "file '" << path << "' on line " << current_line);

      std::string const& type(obj->type());
      bool is_template(obj->is_template());
      _objects[is_template][type][name] = obj;
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
  // XXX:
}

/**
 *  Resolve template for register objects.
 */
void parser::_resolve_template() {
  for (umap<std::string, objects>::iterator
         it(_objects[1].begin()), end(_objects[1].end());
         ++it != end;
         ++it) {
    objects& templates(_objects[0][it->first]);
    objects& objects(it->second);
    for (objects::iterator it(objects.begin()), end(objects.end());
         it != end;
         ++it)
      it->second->resolve_template(templates);
  }
}

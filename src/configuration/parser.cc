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
 *  @param[in]  path The configuration file path.
 */
void parser::parse(std::string const& path) {
  _parse_main_configuration(path);
}

/**
 *  Get object by type name.
 *
 *  @param[in] type The type name (service, host, command...).
 *
 *  @return Map object with properties or throw exception.
 */
objects const& parser::get_objects(std::string const& type) const {
  umap<std::string, objects>::const_iterator
    it(_objects[1].find(type));
  if (it != _objects[1].end())
    throw (engine_error() << "configuration: get objects failed: "
           "unknown type '" << type << "'");
  return (it->second);
}

/**
 *  Get global properties.
 *
 *  @return Global properties.
 */
properties const& parser::get_globals() const throw () {
  return (_properties);
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
 *  Parse the main configuration file.
 *
 *  @param[in] path The configuration path.
 */
void parser::_parse_main_configuration(std::string const& path) {
  std::ifstream stream(path.c_str());
  if (!stream.is_open())
    throw (engine_error() << "configuration: parse main "
           "configuration failed: can't open file '" << path << "'");

  unsigned int current_line(0);
  std::string input;
  while (_get_next_line(stream, input, current_line)) {
    _parse_object_definitions(input);
  }

  // _resolve_template();
}

/**
 *  Parse the object definition file.
 *
 *  @param[in] path The object definitions path.
 */
#include <iostream>
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
      // XXX:
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
 *  Resolve template for register objects.
 */
void parser::_resolve_template() {
  /*
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
  */
}

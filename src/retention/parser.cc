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

#include <fstream>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/retention/parser.hh"
#include "com/centreon/engine/retention/state.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine::retention;

parser::store parser::_store[] = {
  &parser::_store_into_list<list_comment, &state::comments>,
  &parser::_store_into_list<list_contact, &state::contacts>,
  &parser::_store_into_list<list_downtime, &state::downtimes>,
  &parser::_store_into_list<list_host, &state::hosts>,
  &parser::_store_object<info, &state::informations>,
  &parser::_store_object<program, &state::globals>,
  &parser::_store_into_list<list_service, &state::services>
};

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
 *  @param[in] path The configuration file path.
 */
void parser::parse(std::string const& path, state& retention) {
  std::ifstream stream(path.c_str());
  if (!stream.is_open())
    throw (engine_error() << "retention: parse retention "
           "failed: can't open file '" << path << "'");

  shared_ptr<object> obj;
  std::string input;
  unsigned int current_line(0);
  while (string::get_next_line(stream, input, current_line)) {
    if (obj.is_null()) {
      std::size_t pos(input.find_first_of(" \t"));
      if (pos == std::string::npos)
        continue;
      obj = object::create(input.substr(0, pos));
    }
    else if (input != "}") {
      char const* key;
      char const* value;
      if (string::split(input, &key, &value, '='))
        obj->set(key, value);
    }
    else {
      (this->*_store[obj->type()])(retention, obj);
      obj.clear();
    }
  }
}

/**
 *  Store object into the state list.
 *
 *  @param[in] retention The state to fill.
 *  @param[in] obj       The object to store.
 */
template<typename T, T& (state::*ptr)() throw ()>
void parser::_store_into_list(state& retention, object_ptr obj) {
  (retention.*ptr)().push_back(obj);
}

/**
 *  Store object into the state retention.
 *
 *  @param[in] retention The state to fill.
 *  @param[in] obj       The object to store.
 */
template<typename T, T& (state::*ptr)() throw ()>
void parser::_store_object(state& retention, object_ptr obj) {
  (retention.*ptr)() = *shared_ptr<T>(obj);
}

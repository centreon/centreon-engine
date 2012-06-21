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

#include <cstdlib>
#include <exception>
#include <iostream>
#include "com/centreon/engine/error.hh"
#include "com/centreon/engine/macros.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"
#include "com/centreon/engine/utils.hh"
#include "test/objects/create_object.hh"

using namespace com::centreon::engine::objects;
using namespace test::objects;

void add_with_null_member() {
  std::vector<std::string> objs;
  if (add_custom_variables_to_object(objs, NULL) == true)
    throw (engine_error() << __func__ << " failed: invalid return");
}

void add_without_objects() {
  customvariablesmember* head = NULL;
  std::vector<std::string> objs;
  if (add_custom_variables_to_object(objs, &head) == false)
    throw (engine_error() << __func__ << " failed: invalid return");
}

void add_with_objects(unsigned int id) {
  init_object_skiplists();
  customvariablesmember* head = NULL;
  std::vector<std::string> objs;
  for (unsigned int i(0); i < id; ++i) {
    std::ostringstream oss;
    oss << "_key" << id << "=value" << id;
    objs.push_back(oss.str());
  }

  if (add_custom_variables_to_object(objs, &head) == false)
    throw (engine_error() << __func__ << " failed: invalid return");

  customvariablesmember const* member = head;
  while ((member = release(member)));
  free_object_skiplists();
}

/**
 *  Check that custom variable can be added to member.
 *
 *  @return EXIT_SUCCESS on success.
 */
int main() {
  try {
    // Tests.
    add_with_null_member();
    add_without_objects();
    add_with_objects(1);
    add_with_objects(10);
  }
  catch (std::exception const& e) {
    // Exception handling.
    std::cerr << "error: " << e.what() << std::endl;
    free_memory(get_global_macros());
    return (EXIT_FAILURE);
  }

  return (EXIT_SUCCESS);
}

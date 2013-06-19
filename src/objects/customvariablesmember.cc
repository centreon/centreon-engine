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

#include "com/centreon/engine/misc/object.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/objects/customvariablesmember.hh"

using namespace com::centreon::engine::misc;

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       customvariablesmember const& obj1,
       customvariablesmember const& obj2) throw () {
  if (is_equal(obj1.variable_name, obj2.variable_name)
      && is_equal(obj1.variable_value, obj2.variable_value)
      && obj1.has_been_modified == obj2.has_been_modified) {
    if (!obj1.next && !obj2.next)
      return (*obj1.next == *obj2.next);
    if (obj1.next == obj2.next)
      return (true);
  }
  return (false);
}

/**
 *  Not equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is not the same object, otherwise false.
 */
bool operator!=(
       customvariablesmember const& obj1,
       customvariablesmember const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump customvariablesmember content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The customvariablesmember to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, customvariablesmember const& obj) {
  for (customvariablesmember const* m(&obj); m; m = m->next)
    os << "  " << chkstr(m->variable_name) << ": " << chkstr(m->variable_value) << "\n";
  return (os);
}


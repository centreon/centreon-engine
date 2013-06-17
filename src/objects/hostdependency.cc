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

#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/objects/hostdependency.hh"

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
       hostdependency const& obj1,
       hostdependency const& obj2) throw () {
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
       hostdependency const& obj1,
       hostdependency const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump hostdependency content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The hostdependency to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, hostdependency const& obj) {
  os << "hostdependency {\n"
    "  dependency_type:        " << obj.dependency_type << "\n"
    "  dependent_host_name:    " << chkstr(obj.dependent_host_name) << "\n"
    "  host_name:              " << chkstr(obj.host_name) << "\n"
    "  dependency_period:      " << chkstr(obj.dependency_period) << "\n"
    "  inherits_parent:        " << obj.inherits_parent << "\n"
    "  fail_on_up:             " << obj.fail_on_up << "\n"
    "  fail_on_down:           " << obj.fail_on_down << "\n"
    "  fail_on_unreachable:    " << obj.fail_on_unreachable << "\n"
    "  fail_on_pending:        " << obj.fail_on_pending << "\n"
    "  circular_path_checked:  " << obj.circular_path_checked << "\n"
    "  contains_circular_path: " << obj.contains_circular_path << "\n"
    "  master_host_ptr:        " << obj.master_host_ptr << "\n"
    "  dependent_host_ptr:     " << obj.dependent_host_ptr << "\n"
    "  dependency_period_ptr:  " << obj.dependency_period_ptr << "\n"
    "}\n";
  return (os);
}


/*
** Copyright 2011-2013,2015 Merethis
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

#include "com/centreon/engine/deleter/commandsmember.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/commandsmember.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       commandsmember const& obj1,
       commandsmember const& obj2) throw () {
  if (is_equal(obj1.cmd, obj2.cmd)) {
    if (!obj1.next || !obj2.next)
      return (!obj1.next && !obj2.next);
    return (*obj1.next == *obj2.next);
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
       commandsmember const& obj1,
       commandsmember const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump commandsmember content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The commandsmember to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, commandsmember const& obj) {
  for (commandsmember const* m(&obj); m; m = m->next)
    os << string::chkstr(m->cmd) << (m->next ? ", " : "");
  return (os);
}

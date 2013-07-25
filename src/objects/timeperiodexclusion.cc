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

#include "com/centreon/engine/deleter/timeperiodexclusion.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/objects/timeperiodexclusion.hh"
#include "com/centreon/engine/objects/tool.hh"
#include "com/centreon/engine/shared.hh"
#include "com/centreon/engine/string.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::string;

/**
 *  Equal operator.
 *
 *  @param[in] obj1 The first object to compare.
 *  @param[in] obj2 The second object to compare.
 *
 *  @return True if is the same object, otherwise false.
 */
bool operator==(
       timeperiodexclusion const& obj1,
       timeperiodexclusion const& obj2) throw () {
  if (is_equal(obj1.timeperiod_name, obj2.timeperiod_name)) {
    if (!obj1.next || !obj2.next)
      return (!obj1.next && !obj2.next);
    else
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
       timeperiodexclusion const& obj1,
       timeperiodexclusion const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump timeperiodexclusion content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The timeperiodexclusion to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, timeperiodexclusion const& obj) {
  for (timeperiodexclusion const* m(&obj); m; m = m->next)
    os << chkstr(m->timeperiod_name) << (m->next ? ", " : "");
  return (os);
}

/**
 *  Adds a new exclusion to a timeperiod.
 *
 *  @param[in] period Base timeperiod.
 *  @param[in] name   Exclusion timeperiod name.
 *
 *  @return Timeperiod exclusion object.
 */
timeperiodexclusion* add_exclusion_to_timeperiod(
                       timeperiod* period,
                       char const* name) {
  // Make sure we have enough data.
  if (!period || !name)
    return (NULL);

  // Allocate memory.
  timeperiodexclusion* obj(new timeperiodexclusion);
  memset(obj, 0, sizeof(*obj));

  try {
    // Set exclusion properties.
    obj->timeperiod_name = string::dup(name);
    obj->next = period->exclusions;
    period->exclusions = obj;
  }
  catch (...) {
    deleter::timeperiodexclusion(obj);
    obj = NULL;
  }

  return (obj);
}

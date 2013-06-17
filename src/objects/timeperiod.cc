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

#include <cstring>
#include "com/centreon/engine/misc/object.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/objects/daterange.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/objects/timeperiodexclusion.hh"
#include "com/centreon/engine/objects/timerange.hh"

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
       timeperiod const& obj1,
       timeperiod const& obj2) throw () {
  if (strcmp(obj1.name, obj2.name)
      || strcmp(obj1.alias, obj2.alias))
    return (false);
  // XXX: todo
  return (true);
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
       timeperiod const& obj1,
       timeperiod const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump timeperiod content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The timeperiod to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, timeperiod const& obj) {
  os << "timeperiod {\n"
     << "  name:  " << chkstr(obj.name) << "\n"
     << "  alias: " << chkstr(obj.alias) << "\n"
     << "  exclusions: " << chkobj(os, obj.exclusions) << "\n";

  for (unsigned int i(0); i < sizeof(obj.days) / sizeof(obj.days[0]); ++i)
    os << get_weekday_name(i) << ": " << *obj.days[i] << "\n";

  for (unsigned int i(0); i < DATERANGE_TYPES; ++i)
    for (daterange const* it(obj.exceptions[i]); it; it = it->next)
      os << *it << "\n";
  os << "}\n";
  return (os);
}


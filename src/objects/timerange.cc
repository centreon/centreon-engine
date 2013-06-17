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

#include <iomanip>
#include "com/centreon/engine/misc/string.hh"
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
       timerange const& obj1,
       timerange const& obj2) throw () {
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
       timerange const& obj1,
       timerange const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump timerange content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The timerange to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, timerange const& obj) {
  for (timerange const* r(&obj); r; r = r->next) {
    unsigned int start_hours(r->range_start / 3600);
    unsigned int start_minutes((r->range_start % 3600) / 60);
    unsigned int end_hours(r->range_end / 3600);
    unsigned int end_minutes((r->range_end % 3600) / 60);
    os << std::setfill('0') << std::setw(2) << start_hours << ":"
       << std::setfill('0') << std::setw(2) << start_minutes << "-"
       << std::setfill('0') << std::setw(2) << end_hours << ":"
       << std::setfill('0') << std::setw(2) << end_minutes
       << (r->next ? ", " : "");
  }
  return (os);
}


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
#include "com/centreon/engine/deleter/timerange.hh"
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/objects/daterange.hh"
#include "com/centreon/engine/objects/timerange.hh"
#include "com/centreon/engine/objects/timeperiod.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;
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
  if (obj1.range_start == obj2.range_start
      && obj1.range_end == obj2.range_end) {
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

/**
 *  Add a new timerange to a daterange.
 *
 *  @param[in,out] drange     Target date range.
 *  @param[in]     start_time Range start time.
 *  @param[in]     end_time   Range end time.
 *
 *  @return New timerange.
 */
timerange* add_timerange_to_daterange(
             daterange* drange,
             unsigned long start_time,
             unsigned long end_time) {
  // Make sure we have the data we need.
  if (!drange)
    return (NULL);
  if (start_time > 86400) {
    logger(log_config_error, basic)
      << "Error: Start time " << start_time
      << " is not valid for timeperiod";
    return (NULL);
  }
  if (end_time > 86400) {
    logger(log_config_error, basic)
      << "Error: End time " << end_time
      << " is not value for timeperiod";
    return (NULL);
  }

  // Allocate memory for the new time range.
  timerange* obj(new timerange);
  memset(obj, 0, sizeof(*obj));

  try {
    obj->range_start = start_time;
    obj->range_end = end_time;

    // Add the new time range to the head of the range
    // list for this date range.
    obj->next = drange->times;
    drange->times = obj;

    // Notify event broker.
    // XXX
  }
  catch (...) {
    deleter::timerange(obj);
    obj = NULL;
  }

  return (obj);
}

/**
 *  Add a new timerange to a timeperiod.
 *
 *  @param[in,out] period     Target period.
 *  @param[in]     day        Day of the range ([0-6]).
 *  @param[in]     start_time Range start time.
 *  @param[in]     end_time   Range end time.
 *
 *  @return New time range.
 */
timerange* add_timerange_to_timeperiod(
             timeperiod* period,
             int day,
             unsigned long start_time,
             unsigned long end_time) {
  // Make sure we have the data we need.
  if (!period)
    return (NULL);
  if (day < 0 || day > 6) {
    logger(log_config_error, basic)
      << "Error: Day " << day
      << " is not valid for timeperiod '" << period->name << "'";
    return (NULL);
  }
  if (start_time > 86400) {
    logger(log_config_error, basic)
      << "Error: Start time " << start_time << " on day " << day
      << " is not valid for timeperiod '" << period->name << "'";
    return (NULL);
  }
  if (end_time > 86400) {
    logger(log_config_error, basic)
      << "Error: End time " << end_time << " on day " << day
      << " is not value for timeperiod '" << period->name << "'";
    return (NULL);
  }

  // Allocate memory for the new time range.
  timerange* obj(new timerange);
  memset(obj, 0, sizeof(*obj));

  try {
    obj->range_start = start_time;
    obj->range_end = end_time;

    // Add the new time range to the head of the range list for this day.
    obj->next = period->days[day];
    period->days[day] = obj;

    // Notify event broker.
    // XXX
  }
  catch (...) {
    deleter::timerange(obj);
    obj = NULL;
  }

  return (obj);
}

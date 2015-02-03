/*
** Copyright 2011-2014 Merethis
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
#include "com/centreon/engine/objects/daterange.hh"
#include "com/centreon/engine/objects/timerange.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
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
       timerange const& obj1,
       timerange const& obj2) throw () {
  if ((obj1.start_hour == obj2.start_hour)
      && (obj1.start_minute == obj2.start_minute)
      && (obj1.end_hour == obj2.end_hour)
      && (obj1.end_minute == obj2.end_minute)) {
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
    os << std::setfill('0') << std::setw(2) << obj.start_hour << ":"
       << std::setfill('0') << std::setw(2) << obj.start_minute << "-"
       << std::setfill('0') << std::setw(2) << obj.end_hour << ":"
       << std::setfill('0') << std::setw(2) << obj.end_minute
       << (r->next ? ", " : "");
  }
  return (os);
}

/**
 *  Add a new timerange to a daterange.
 *
 *  @param[in,out] drange        Target date range.
 *  @param[in]     start_hour    Range start hour.
 *  @param[in]     start_minute  Range start minute.
 *  @param[in]     end_hour      Range end hour.
 *  @param[in]     end_minute    Range end minute.
 *
 *  @return New timerange.
 */
timerange* add_timerange_to_daterange(
             daterange* drange,
             int start_hour,
             int start_minute,
             int end_hour,
             int end_minute) {
  // Make sure we have the data we need.
  if (!drange)
    return (NULL);
  if (start_hour * 60 * 60 + start_minute * 60 > 24 * 60 * 60) {
    logger(log_config_error, basic)
      << "Error: Start time " << start_hour << ":" << start_minute
      << " is not valid for timeperiod";
    return (NULL);
  }
  if (end_hour * 60 * 60 + end_minute * 60 > 24 * 60 * 60) {
    logger(log_config_error, basic)
      << "Error: End time " << end_hour << ":" << end_minute
      << " is not valid for timeperiod";
    return (NULL);
  }

  // Allocate memory for the new time range.
  timerange* obj(new timerange);
  memset(obj, 0, sizeof(*obj));

  try {
    obj->start_hour = start_hour;
    obj->start_minute = start_minute;
    obj->end_hour = end_hour;
    obj->end_minute = end_minute;

    // Add the new time range to the head of the range
    // list for this date range.
    obj->next = drange->times;
    drange->times = obj;
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
 *  @param[in,out] period        Target period.
 *  @param[in]     day           Day of the range ([0-6]).
 *  @param[in]     start_hour    Range start hour.
 *  @param[in]     start_minute  Range start minute.
 *  @param[in]     end_hour      Range end hour.
 *  @param[in]     end_minute    Range end minute.
 *
 *  @return New time range.
 */
timerange* add_timerange_to_timeperiod(
             timeperiod* period,
             int day,
             int start_hour,
             int start_minute,
             int end_hour,
             int end_minute) {
  // Make sure we have the data we need.
  if (!period)
    return (NULL);
  if ((day < 0) || (day > 6)) {
    logger(log_config_error, basic)
      << "Error: Day " << day
      << " is not valid for timeperiod '" << period->name << "'";
    return (NULL);
  }
  if (start_hour * 60 * 60 + start_minute * 60 > 24 * 60 * 60) {
    logger(log_config_error, basic)
      << "Error: Start time " << start_hour << ":" << start_minute
      << " on day " << day << " is not valid for timeperiod '"
      << period->name << "'";
    return (NULL);
  }
  if (end_hour * 60 * 60 + end_minute * 60 > 24 * 60 * 60) {
    logger(log_config_error, basic)
      << "Error: End time " << end_hour << ":" << end_minute
      << " on day " << day << " is not value for timeperiod '"
      << period->name << "'";
    return (NULL);
  }

  // Allocate memory for the new time range.
  timerange* obj(new timerange);
  memset(obj, 0, sizeof(*obj));

  try {
    obj->start_hour = start_hour;
    obj->start_minute = start_minute;
    obj->end_hour = end_hour;
    obj->end_minute = end_minute;

    // Add the new time range to the head of the range list for this day.
    obj->next = period->days[day];
    period->days[day] = obj;
  }
  catch (...) {
    deleter::timerange(obj);
    obj = NULL;
  }

  return (obj);
}

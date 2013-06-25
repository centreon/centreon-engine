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
#include "com/centreon/engine/common.hh"
#include "com/centreon/engine/deleter/daterange.hh"
#include "com/centreon/engine/misc/object.hh"
#include "com/centreon/engine/misc/string.hh"
#include "com/centreon/engine/objects/daterange.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/objects/timerange.hh"

using namespace com::centreon::engine;
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
       daterange const& obj1,
       daterange const& obj2) throw () {
  if (obj1.type == obj2.type
      && obj1.syear == obj2.syear
      && obj1.smon == obj2.smon
      && obj1.smday == obj2.smday
      && obj1.swday == obj2.swday
      && obj1.swday_offset == obj2.swday_offset
      && obj1.eyear == obj2.eyear
      && obj1.emon == obj2.emon
      && obj1.emday == obj2.emday
      && obj1.ewday == obj2.ewday
      && obj1.ewday_offset == obj2.ewday_offset
      && obj1.skip_interval == obj2.skip_interval
      && is_equal(obj1.times, obj2.times)) {
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
       daterange const& obj1,
       daterange const& obj2) throw () {
  return (!operator==(obj1, obj2));
}

/**
 *  Dump the daterange value into the calendar date format.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The daterange to dump.
 *
 *  @return The output stream.
 */
static std::ostream& _dump_calendar_date(std::ostream& os, daterange const& obj) {
  os << std::setfill('0') << std::setw(2) << obj.syear << "-"
     << std::setfill('0') << std::setw(2) << obj.smon + 1 << "-"
     << std::setfill('0') << std::setw(2) << obj.smday;
  if (obj.syear != obj.eyear
      || obj.smon != obj.emon
      || obj.smday != obj.emday)
    os << " - "
       << std::setfill('0') << std::setw(2) << obj.eyear << "-"
       << std::setfill('0') << std::setw(2) << obj.emon + 1 << "-"
       << std::setfill('0') << std::setw(2) << obj.emday;
  if (obj.skip_interval)
    os << " / " << obj.skip_interval;
  return (os);
}

/**
 *  Dump the daterange value into the month date format.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The daterange to stringify.
 *
 *  @return The output stream.
 */
static std::ostream& _dump_month_date(std::ostream& os, daterange const& obj) {
  std::string const& smon(get_month_name(obj.smon));
  std::string const& emon(get_month_name(obj.emon));
  os << smon << " " << obj.smday;
  if (smon != emon)
    os << " - " << emon << " " << obj.emday;
  else if (obj.smday != obj.emday)
    os << " - " << obj.emday;
  if (obj.skip_interval)
    os << " / " << obj.skip_interval;
  return (os);
}

/**
 *  Dump the daterange value into the month day format.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The daterange to stringify.
 *
 *  @return The output stream.
 */
static std::ostream& _dump_month_day(std::ostream& os, daterange const& obj) {
  os << "day " << obj.smday;
  if (obj.smday != obj.emday)
    os << " - " << obj.emday;
  if (obj.skip_interval)
    os << " / " << obj.skip_interval;
  return (os);
}

/**
 *  Dump the daterange value into the month week day
 *  format.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The daterange to stringify.
 *
 *  @return The output stream.
 */
static std::ostream& _dump_month_week_day(std::ostream& os, daterange const& obj) {
  os << get_weekday_name(obj.swday) << " " << obj.swday_offset << " "
     << get_month_name(obj.smon);
  if (obj.swday != obj.ewday
      || obj.swday_offset != obj.ewday_offset
      || obj.smon != obj.emon)
    os << " - " << get_weekday_name(obj.ewday)
       << " " << obj.ewday_offset << " " << get_month_name(obj.emon);
  if (obj.skip_interval)
    os << " / " << obj.skip_interval;
  return (os);
}

/**
 *  Dump the daterange value into the week day format.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The daterange to stringify.
 *
 *  @return The output stream.
 */
static std::ostream& _dump_week_day(std::ostream& os, daterange const& obj) {
  os << get_weekday_name(obj.swday) << " " << obj.swday_offset;
  if (obj.swday != obj.ewday || obj.swday_offset != obj.ewday_offset)
    os << " - " << get_weekday_name(obj.ewday) << " " << obj.ewday_offset;
  if (obj.skip_interval)
    os << " / " << obj.skip_interval;
  return (os);
}

/**
 *  Dump daterange content into the stream.
 *
 *  @param[out] os  The output stream.
 *  @param[in]  obj The daterange to dump.
 *
 *  @return The output stream.
 */
std::ostream& operator<<(std::ostream& os, daterange const& obj) {
  typedef std::ostream& (*func)(std::ostream&, daterange const&);
  static func tab[] = {
    &_dump_calendar_date,
    &_dump_month_date,
    &_dump_month_day,
    &_dump_month_week_day,
    &_dump_week_day,
  };

  if (obj.type < 0 || obj.type >= DATERANGE_TYPES)
    os << "unknown type " << obj.type;
  else {
    (*(tab[obj.type]))(os, obj);
    os << " " << chkobj(obj.times);
  }
  return (os);
}

/**
 *  Get the month name.
 *
 *  @param[in] index  The month position.
 *
 *  @return The month name.
 */
std::string const& get_month_name(unsigned int index) {
  static std::string const unknown("unknown");
  static std::string const month[] = {
    "january",
    "february",
    "march",
    "april",
    "may",
    "june",
    "july",
    "august",
    "september",
    "october",
    "november",
    "december"
  };
  if (index >= sizeof(month) / sizeof(*month))
    return (unknown);
  return (month[index]);
}

/**
 *  Get the weekday name.
 *
 *  @param[in] index  The weekday position.
 *
 *  @return The weekday name.
 */
std::string const& get_weekday_name(unsigned int index) {
  static std::string const unknown("unknown");
  static std::string const days[] = {
    "sunday",
    "monday",
    "tuesday",
    "wednesday",
    "thursday",
    "friday",
    "saturday"
  };
  if (index >= sizeof(days) / sizeof(*days))
    return (unknown);
  return (days[index]);
}

/**
 *  Add a new exception to a timeperiod.
 *
 *  @param[in] period        Base period.
 *  @param[in] type
 *  @param[in] syear
 *  @param[in] smon
 *  @param[in] smday
 *  @param[in] swday
 *  @param[in] swday_offset
 *  @param[in] eyear
 *  @param[in] emon
 *  @param[in] emday
 *  @param[in] ewday
 *  @param[in] ewday_offset
 *  @param[in] skip_interval
 *
 *  @return Excluded date range.
 */
daterange* add_exception_to_timeperiod(
             timeperiod* period,
             int type,
             int syear,
             int smon,
             int smday,
             int swday,
             int swday_offset,
             int eyear,
             int emon,
             int emday,
             int ewday,
             int ewday_offset,
             int skip_interval) {
  // Make sure we have the data we need.
  if (!period)
    return (NULL);

  // Allocate memory for the date range range.
  daterange* obj(new daterange);
  memset(obj, 0, sizeof(*obj));

  try {
    // Set daterange properties.
    obj->type = type;
    obj->syear = syear;
    obj->smon = smon;
    obj->smday = smday;
    obj->swday = swday;
    obj->swday_offset = swday_offset;
    obj->eyear = eyear;
    obj->emon = emon;
    obj->emday = emday;
    obj->ewday = ewday;
    obj->ewday_offset = ewday_offset;
    obj->skip_interval = skip_interval;

    // Add the new date range to the head of the range
    // list for this exception type.
    obj->next = period->exceptions[type];
    period->exceptions[type] = obj;

    // Notify event broker.
    // XXX
  }
  catch (...) {
    deleter::daterange(obj);
    obj = NULL;
  }

  return (obj);
}

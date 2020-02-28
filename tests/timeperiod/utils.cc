/*
** Copyright 2016 Centreon
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

#include "tests/timeperiod/utils.hh"
#include <cstring>
#include <ctime>
#include <memory>
#include "com/centreon/engine/exceptions/error.hh"
#include "com/centreon/engine/timerange.hh"

using namespace com::centreon::engine;
// Global time.
static time_t gl_now((time_t)-1);

/**
 *  Create new timeperiod creator.
 */
timeperiod_creator::timeperiod_creator() {}

/**
 *  Delete timeperiod creator and associated timeperiods.
 */
timeperiod_creator::~timeperiod_creator() {
  _timeperiods.clear();
}

/**
 *  Get generated timeperiods.
 *
 *  @return Timeperiods list.
 */
timeperiod* timeperiod_creator::get_timeperiods() {
  return (_timeperiods.begin()->get());
}

std::shared_ptr<timeperiod> timeperiod_creator::get_timeperiods_shared() {
  return (*_timeperiods.begin());
}

/**
 *  Create a new timeperiod.
 *
 *  @return The newly created timeperiod.
 */
timeperiod* timeperiod_creator::new_timeperiod() {
  std::shared_ptr<timeperiod> tp{new timeperiod("test", "test")};
  _timeperiods.push_front(tp);
  return (tp.get());
}

/**
 *  Create a new exclusion on the timeperiod.
 *
 *  @param[in]  excluded  Excluded timeperiod.
 *  @param[out] target    Target timeperiod.
 */
void timeperiod_creator::new_exclusion(std::shared_ptr<timeperiod> excluded,
                                       timeperiod* target) {
  if (!target)
    target = _timeperiods.begin()->get();

  target->get_exclusions().insert({excluded->get_name(), excluded.get()});
}

/**
 *  Create a new calendar date range.
 *
 *  @param[in]  start_year   Start year.
 *  @param[in]  start_month  Start month.
 *  @param[in]  start_day    Start day.
 *  @param[in]  end_year     End year.
 *  @param[in]  end_month    End month.
 *  @param[in]  end_day      End day.
 *  @param[out] target       Target timeperiod.
 *
 *  @return The newly created daterange.
 */
daterange* timeperiod_creator::new_calendar_date(int start_year,
                                                 int start_month,
                                                 int start_day,
                                                 int end_year,
                                                 int end_month,
                                                 int end_day,
                                                 timeperiod* target) {
  if (!target)
    target = _timeperiods.begin()->get();

  std::shared_ptr<daterange> dr{
      new daterange(DATERANGE_CALENDAR_DATE, start_year, start_month, start_day,
                    0, 0, end_year, end_month, end_day, 0, 0, 0)};

  target->exceptions[DATERANGE_CALENDAR_DATE].push_back(dr);
  return dr.get();
}

/**
 *  Create a new specific month date range.
 *
 *  @param[in]  start_month  Start month.
 *  @param[in]  start_day    Start day.
 *  @param[in]  end_month    End month.
 *  @param[in]  end_day      End day.
 *  @param[out] target       Target timeperiod.
 *
 *  @return The newly created daterange.
 */
daterange* timeperiod_creator::new_specific_month_date(int start_month,
                                                       int start_day,
                                                       int end_month,
                                                       int end_day,
                                                       timeperiod* target) {
  if (!target)
    target = _timeperiods.begin()->get();

  std::shared_ptr<daterange> dr{new daterange(DATERANGE_MONTH_DATE, 0,
                                              start_month, start_day, 0, 0, 0,
                                              end_month, end_day, 0, 0, 0)};

  target->exceptions[DATERANGE_MONTH_DATE].push_back(dr);
  return dr.get();
}

/**
 *  Create a new generic month date.
 *
 *  @param[in]  start_day  Start day.
 *  @param[in]  end_day    End day.
 *  @param[out] target     Target timeperiod.
 *
 *  @return The newly created daterange.
 */
daterange* timeperiod_creator::new_generic_month_date(int start_day,
                                                      int end_day,
                                                      timeperiod* target) {
  if (!target)
    target = _timeperiods.begin()->get();

  std::shared_ptr<daterange> dr{new daterange(
      DATERANGE_MONTH_DAY, 0, 0, start_day, 0, 0, 0, 0, end_day, 0, 0, 0)};

  target->exceptions[DATERANGE_MONTH_DAY].push_back(dr);
  return dr.get();
}

/**
 *  Create a new offset weekday daterange.
 *
 *  @param[in]  start_month   Start month.
 *  @param[in]  start_wday    Start week day.
 *  @param[in]  start_offset  Start offset.
 *  @param[in]  end_month     End month.
 *  @param[in]  end_wday      End week day.
 *  @param[in]  end_offset    End offset.
 *  @param[out] timeperiod    Target timeperiod.
 *
 *  @return The newly created daterange.
 */
daterange* timeperiod_creator::new_offset_weekday_of_specific_month(
    int start_month,
    int start_wday,
    int start_offset,
    int end_month,
    int end_wday,
    int end_offset,
    timeperiod* target) {
  if (!target)
    target = _timeperiods.begin()->get();

  std::shared_ptr<daterange> dr{
      new daterange(DATERANGE_MONTH_WEEK_DAY, 0, start_month, 0, start_wday,
                    start_offset, 0, end_month, 0, end_wday, end_offset, 0)};

  target->exceptions[DATERANGE_MONTH_WEEK_DAY].push_back(dr);
  return dr.get();
}

/**
 *  Create a new offset weekday daterange.
 *
 *  @param[in]  start_wday    Start week day.
 *  @param[in]  start_offset  Start offset.
 *  @param[in]  end_wday      End week day.
 *  @param[in]  end_offset    End offset.
 *  @param[out] timeperiod    Target timeperiod.
 *
 *  @return The newly created daterange.
 */
daterange* timeperiod_creator::new_offset_weekday_of_generic_month(
    int start_wday,
    int start_offset,
    int end_wday,
    int end_offset,
    timeperiod* target) {
  if (!target)
    target = _timeperiods.begin()->get();

  std::shared_ptr<daterange> dr{new daterange(DATERANGE_WEEK_DAY, 0, 0, 0,
                                              start_wday, start_offset, 0, 0, 0,
                                              end_wday, end_offset, 0)};

  target->exceptions[DATERANGE_WEEK_DAY].push_back(dr);
  return dr.get();
}

/**
 *  Create a new timerange in a daterange.
 *
 *  @param[in]  start_hour    Start hour.
 *  @param[in]  start_minute  Start minute.
 *  @param[in]  end_hour      End hour.
 *  @param[in]  end_minute    End minute.
 *  @param[out] target        Target daterange.
 */
void timeperiod_creator::new_timerange(int start_hour,
                                       int start_minute,
                                       int end_hour,
                                       int end_minute,
                                       daterange* target) {
  if (!target)
    return;

  std::shared_ptr<timerange> tr{new timerange(hmtos(start_hour, start_minute),
                                              hmtos(end_hour, end_minute))};

  target->times.push_back(tr);
}

/**
 *  Create a new weekday timerange.
 *
 *  @param[in]  start_hour    Start hour.
 *  @param[in]  start_minute  Start minute.
 *  @param[in]  end_hour      End hour.
 *  @param[in]  end_minute    End minute.
 *  @param[in]  day           Day.
 *  @param[out] target        Target timeperiod.
 */
void timeperiod_creator::new_timerange(int start_hour,
                                       int start_minute,
                                       int end_hour,
                                       int end_minute,
                                       int day,
                                       timeperiod* target) {
  if (!target)
    target = _timeperiods.begin()->get();

  std::shared_ptr<timerange> tr{new timerange(hmtos(start_hour, start_minute),
                                              hmtos(end_hour, end_minute))};

  target->days[day].push_back(tr);
}

/**
 *  Convert hour and minutes to a number of seconds.
 *
 *  @param[in] h  Hours.
 *  @param[in] m  Minutes.
 *
 *  @return The number of seconds.
 */
int hmtos(int h, int m) {
  return h * 60 * 60 + m * 60;
}

/**
 *  Set system time for testing purposes.
 *
 *  The real system time is not changed but time() returns the requested
 *  value.
 *
 *  @param now  New system time.
 */
void set_time(time_t now) {
  gl_now = now;
}

/**
 *  Convert a string to time_t.
 *
 *  @param str  String to convert.
 *
 *  @return The converted string.
 */
time_t strtotimet(std::string const& str) {
  tm t;
  memset(&t, 0, sizeof(t));
  if (!strptime(str.c_str(), "%Y-%m-%d %H:%M:%S", &t))
    throw(engine_error() << "invalid date format");
  t.tm_isdst = -1;
  return (mktime(&t));
}

/**
 *  Overload of libc time function.
 */

#ifndef __THROW
#define __THROW
#endif  // !__THROW

extern "C" time_t time(time_t* t) __THROW {
  if (t)
    *t = gl_now;
  return (gl_now);
}

extern "C" int gettimeofday(struct timeval* tv, struct timezone *tz) __THROW {
  if (tv) {
    tv->tv_sec = gl_now;
    tv->tv_usec = 0;
  }
  return 0;
}

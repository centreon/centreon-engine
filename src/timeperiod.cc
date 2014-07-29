/*
** Copyright 1999-2009 Ethan Galstad
** Copyright 2009-2012 Icinga Development Team (http://www.icinga.org)
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

#include <ctime>
#include "com/centreon/engine/logging/logger.hh"
#include "com/centreon/engine/objects/daterange.hh"
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/objects/timeperiodexclusion.hh"
#include "com/centreon/engine/objects/timerange.hh"
#include "com/centreon/engine/timeperiod.hh"

using namespace com::centreon::engine;
using namespace com::centreon::engine::logging;

/**
 *  Add a round number of days (expressed in seconds) to a date.
 *
 *  @param[in] middnight  Midnight of base day.
 *  @param[in] skip       Number of days to skip (in seconds).
 *
 *  @return Midnight of the day in skip seconds.
 */
static time_t _add_round_days_to_midnight(time_t midnight, time_t skip) {
  // Compute expected time with no DST.
  time_t next_day_time(midnight + skip);
  struct tm next_day;
  localtime_r(&next_day_time, &next_day);

  // There was a DST shift in between.
  if (next_day.tm_hour || next_day.tm_min || next_day.tm_sec) {
    /*
    ** The trick here is to move from midnight to noon, add the skip
    ** seconds and break time down in a tm structure. We're now sure to
    ** be in the proper day (DST shift is +-1h) we only have to reset
    ** time to midnight, convert back and we're done.
    */
    next_day_time += 12 * 60 * 60 + skip;
    localtime_r(&next_day_time, &next_day);
    next_day.tm_hour = 0;
    next_day.tm_min = 0;
    next_day.tm_sec = 0;
    next_day_time = mktime(&next_day);
  }

  return (next_day_time);
}

/**
 *  Returns a time (midnight) of particular (3rd, last) day in a given
 *  month.
 *
 *  @param[in] year      Year.
 *  @param[in] month     Month.
 *  @param[in] monthday  Day in month.
 *
 *  @return Requested timestamp, (time_t)-1 if conversion failed.
 */
static time_t calculate_time_from_day_of_month(
                int year,
                int month,
                int monthday) {
  time_t midnight;
  tm t;

  // Positive day (3rd day).
  if (monthday > 0) {
    t.tm_sec = 0;
    t.tm_min = 0;
    t.tm_hour = 0;
    t.tm_year = year;
    t.tm_mon = month;
    t.tm_mday = monthday;
    t.tm_isdst = -1;
    midnight = mktime(&t);

    // If we rolled over to the next month, time is invalid, assume the
    // user's intention is to keep it in the current month.
    if (t.tm_mon != month)
      midnight = (time_t)-1;
  }
  // Negative offset (last day, 3rd to last day).
  else {
    // Find last day in the month.
    int day(32);
    do {
      // Back up a day.
      --day;

      // Make the new time.
      t.tm_mon = month;
      t.tm_year = year;
      t.tm_mday = day;
      t.tm_isdst = -1;
      midnight = mktime(&t);
    } while ((midnight == (time_t)-1)
             || (t.tm_mon != month));

    // Now that we know the last day, back up more.
    t.tm_mon = month;
    t.tm_year = year;
    // Beware to roll over the whole month.
    if (-monthday >= t.tm_mday)
      t.tm_mday = 1;
    // -1 means last day of month, so add one to make this correct.
    else
      t.tm_mday += monthday + 1;
    t.tm_isdst = -1;
    midnight = mktime(&t);
  }

  return (midnight);
}

/**
 *  Returns a time (midnight) of particular (3rd, last) weekday in a
 *  given month.
 *
 *  @param[in] year            Year.
 *  @param[in] month           Month.
 *  @param[in] weekday         Target weekday.
 *  @param[in] weekday_offset  Weekday offset (1 is first, 2 is second,
 *                             -1 is last).
 *
 *  @return Requested timestamp, (time_t)-1 if conversion failed.
 */
static time_t calculate_time_from_weekday_of_month(
                int year,
                int month,
                int weekday,
                int weekday_offset) {
  // Compute first day of month (to get weekday).
  tm t;
  t.tm_sec = 0;
  t.tm_min = 0;
  t.tm_hour = 0;
  t.tm_year = year;
  t.tm_mon = month;
  t.tm_mday = 1;
  t.tm_isdst = -1;
  time_t midnight(mktime(&t));

  // How many days must we advance to reach the first instance of the
  // weekday this month ?
  int days(weekday - (t.tm_wday));
  if (days < 0)
    days += 7;

  // Positive offset (3rd thursday).
  if (weekday_offset > 0) {
    // How many weeks must we advance (no more than 5 possible).
    int weeks((weekday_offset > 5) ? 5 : weekday_offset);
    days += ((weeks - 1) * 7);

    // Make the new time.
    t.tm_mon = month;
    t.tm_year = year;
    t.tm_mday = days + 1;
    t.tm_isdst = -1;
    midnight = mktime(&t);

    // If we rolled over to the next month, time is invalid, assume the
    // user's intention is to keep it in the current month.
    if (t.tm_mon != month)
      midnight = (time_t)-1;
  }
  // Negative offset (last thursday, 3rd to last tuesday).
  else {
    // Find last instance of weekday in the month.
    days += (5 * 7);
    do {
      // Back up a week.
      days -= 7;

      // Make the new time.
      t.tm_mon = month;
      t.tm_year = year;
      t.tm_mday = days + 1;
      t.tm_isdst = -1;
      midnight = mktime(&t);
    } while ((midnight == (time_t)-1)
             || (t.tm_mon != month));

    // Now that we know the last instance of the weekday, back up more.
    days = ((weekday_offset + 1) * 7);
    t.tm_mon = month;
    t.tm_year = year;
    // Beware to roll over the whole month.
    if (-days >= t.tm_mday)
      t.tm_mday = t.tm_mday % 7;
    else
      t.tm_mday += days;
    t.tm_isdst = -1;
    midnight = mktime(&t);
  }

  return (midnight);
}

// tests if a date range covers just a single day
int is_daterange_single_day(daterange const* dr) {
  if (!dr)
    return (false);

  if (dr->syear != dr->eyear)
    return (false);
  if (dr->smon != dr->emon)
    return (false);
  if (dr->smday != dr->emday)
    return (false);
  if (dr->swday != dr->ewday)
    return (false);
  if (dr->swday_offset != dr->ewday_offset)
    return (false);

  return (true);
}

/**
 *  Internal struct time information.
 */
struct   time_info {
  time_t current_time;
  tm     curtime;
  time_t midnight;
  time_t preferred_time;
  tm     preftime;
};

// Checks if the given time is in daylight time saving period.
static int is_dst_time(time_t const* time) {
  tm t;
  localtime_r(time, &t);
  return (t.tm_isdst);
}

// Returns the shift in seconds if the given times are across the
// daylight time saving period change.
static int get_dst_shift(time_t const* start, time_t const* end) {
  int dst_start(is_dst_time(start));
  int dst_end(is_dst_time(end));
  if (dst_start < dst_end)
    return (3600);
  if (dst_start > dst_end)
    return (-3600);
  return (0);
}

/**
 *  Calculate start time and end time for date range calendar date.
 *
 *  @param[in]  r      Range to calculate start time and end time.
 *  @param[in]  ti     Time informations.
 *  @param[out] start  Variable to fill start time.
 *  @param[out] end    Variable to fill end time.
 *
 *  @return True on success, otherwise false.
 */
static bool _daterange_calendar_date_to_time_t(
              daterange const& r,
              time_info const& ti,
              time_t& start,
              time_t& end) {
  (void)ti;

  tm t;
  t.tm_sec = 0;
  t.tm_min = 0;
  t.tm_hour = 0;
  t.tm_isdst = -1;

  t.tm_mday = r.smday;
  t.tm_mon = r.smon;
  t.tm_year = r.syear - 1900;
  if ((start = mktime(&t)) == (time_t)-1)
    return (false);

  if (r.eyear) {
    t.tm_mday = r.emday;
    t.tm_mon = r.emon;
    t.tm_year = r.eyear - 1900;
    t.tm_hour = 23;
    t.tm_min = 59;
    t.tm_sec = 59;
    /*
    ** We don't have to care about DST even if it would occur at
    ** midnight because end time is used as the upper bound of the date
    ** range. This make it invalid for use as midnight but perfectly
    ** valid to check that we're less than or equal to 23:59:59, which
    ** value is provided by mktime().
    */
    if ((end = mktime(&t)) == (time_t)-1)
      return (false);
    ++end;
  }
  else
    end = (time_t)-1;

  return (true);
}

/**
 *  Calculate start time and end time for date range month date.
 *
 *  @param[in]  r      Range to calculate start time and end time.
 *  @param[in]  ti     Time informations.
 *  @param[out] start  Variable to fill start time.
 *  @param[out] end    Variable to fill end time.
 *
 *  @return True on success, otherwise false.
 */
static bool _daterange_month_date_to_time_t(
              daterange const& r,
              time_info const& ti,
              time_t& start,
              time_t& end) {
  // what year should we use?
  int year(std::max(ti.preftime.tm_year, ti.curtime.tm_year));
  // advance an additional year if we already passed
  // the end month date
  if (r.emon < ti.curtime.tm_mon
      || (r.emon == ti.curtime.tm_mon
          && r.emday < ti.curtime.tm_mday))
    ++year;
  start = calculate_time_from_day_of_month(year, r.smon, r.smday);

  // start date was bad.
  if (!start)
    return (false);

  // use same year as was calculated for start time above
  end = calculate_time_from_day_of_month(year, r.emon, r.emday);
  // advance a year if necessary: august 5 - february 2
  if (end < start)
    end = calculate_time_from_day_of_month( ++year, r.emon, r.emday);

  // end date was bad - see if we can handle the error
  if (!end) {
    // end date can't be helped, so skip it
    if (r.emday < 0)
      return (false);
    // else end date slipped past end of month, so use last
    // day of month as end date
    end = calculate_time_from_day_of_month(year, r.emon, -1);
  }
  return (true);
}

/**
 *  Calculate start time and end time for date range month day.
 *
 *  @param[in]  r      Range to calculate start time and end time.
 *  @param[in]  ti     Time informations.
 *  @param[out] start  Variable to fill start time.
 *  @param[out] end    Variable to fill end time.
 *
 *  @return True on success, otherwise false.
 */
static bool _daterange_month_day_to_time_t(
              daterange const& r,
              time_info const& ti,
              time_t& start,
              time_t& end) {
  // What year/month should we use ?
  int year;
  int month;
  if (ti.preferred_time > ti.current_time) {
    year = ti.preftime.tm_year;
    month = ti.preftime.tm_mon;
    // Advance an additional month (and possibly the year) if
    // we already passed the end day of month.
    if (ti.preftime.tm_mday > r.emday) {
      if (month != 11)
        ++month;
      else {
        month = 0;
        ++year;
      }
    }
  }
  else {
    year = ti.curtime.tm_year;
    month = ti.curtime.tm_mon;
    // Advance an additional month (and possibly the year) if
    // we already passed the end day of month.
    if (ti.curtime.tm_mday > r.emday) {
      if (month != 11)
        ++month;
      else {
        month = 0;
        ++year;
      }
    }
  }

  // Compute start date.
  start = calculate_time_from_day_of_month(year, month, r.smday);
  if (start == (time_t)-1)
    return (false);

  // Use same year and month as was calculated for start time above.
  end = calculate_time_from_day_of_month(year, month, r.emday);
  if (end == (time_t)-1) {
    // End date can't be helped, so skip it.
    if (r.emday < 0)
      return (false);

    // Else end date slipped past end of month,
    // so use first day of next month.
    if (month != 11)
      ++month;
    else {
      month = 0;
      ++year;
    }
    end = calculate_time_from_day_of_month(year, month, 0);
  }
  else
    end = _add_round_days_to_midnight(end, 24 * 60 * 60);
  return (true);
}

/**
 *  Calculate start time and end time for date range month week day.
 *
 *  @param[in]  r      Range to calculate start time and end time.
 *  @param[in]  ti     Time informations.
 *  @param[out] start  Variable to fill start time.
 *  @param[out] end    Variable to fill end time.
 *
 *  @return True on success, otherwise false.
 */
static bool _daterange_month_week_day_to_time_t(
              daterange const& r,
              time_info const& ti,
              time_t& start,
              time_t& end) {
  // What year should we use?
  int year(std::max(ti.preftime.tm_year, ti.curtime.tm_year));

  while (true) {
    // Calculate time of specified weekday of specific month.
    start = calculate_time_from_weekday_of_month(
              year,
              r.smon,
              r.swday,
              r.swday_offset);
    if ((time_t)-1 == start)
      return (false);

    // Use same year as was calculated for start time above.
    end = calculate_time_from_weekday_of_month(
            year,
            r.emon,
            r.ewday,
            r.ewday_offset);

    // Advance a year if necessary :
    // thursday 2 august - monday 3 february
    if ((end != (time_t)-1) && (end < start))
      end = calculate_time_from_weekday_of_month(
              year + 1,
              r.emon,
              r.ewday,
              r.ewday_offset);

    if ((time_t)-1 == end) {
      // End date can't be helped, so skip it.
      if (r.ewday_offset < 0)
        return (false);

      // Else end date slipped past end of month, so use last day
      // of month as end date.
      int end_month;
      int end_year;
      if (r.emon != 11) {
        end_month = r.emon + 1;
        end_year = year;
      }
      else {
        end_month = 0;
        end_year = year + 1;
      }
      end = calculate_time_from_day_of_month(
              end_year,
              end_month,
              0);
      if ((time_t)-1 == end)
        return (false);
    }
    else
      end = _add_round_days_to_midnight(end, 24 * 60 * 60);

    // We should have an interval that includes or is above
    // preferred time.
    if (ti.preferred_time < end)
      break ;
    // Advance to next year if we've passed this month
    // weekday already this year.
    else
      ++year;
  }

  return (true);
}

/**
 *  Calculate start time and end time for date range week day.
 *
 *  @param[in]  r      Range to calculate start time and end time.
 *  @param[in]  ti     Time informations.
 *  @param[out] start  Variable to fill start time.
 *  @param[out] end    Variable to fill end time.
 *
 *  @return True on success, otherwise false.
 */
static bool _daterange_week_day_to_time_t(
              daterange const& r,
              time_info const& ti,
              time_t& start,
              time_t& end) {
  // What year/month should we use ?
  int year;
  int month;
  if (ti.preferred_time > ti.current_time) {
    year = ti.preftime.tm_year;
    month = ti.preftime.tm_mon;
  }
  else {
    year = ti.curtime.tm_year;
    month = ti.curtime.tm_mon;
  }

  while (true) {
    // Calculate time of specified weekday of month.
    start = calculate_time_from_weekday_of_month(
              year,
              month,
              r.swday,
              r.swday_offset);

    // Use same year and month as was calculated for start time above.
    end = calculate_time_from_weekday_of_month(
            year,
            month,
            r.ewday,
            r.ewday_offset);
    if (end == (time_t)-1) {
      // End date can't be helped, so skip it.
      if (r.ewday_offset < 0)
        return (false);

      // Else end date slipped past end of month, so use last day
      // of month as end date.
      int end_month;
      int end_year;
      if (month != 11) {
        end_month = month + 1;
        end_year = year;
      }
      else {
        end_month = 0;
        end_year = year + 1;
      }
      end = calculate_time_from_day_of_month(end_year, end_month, 0);
    }
    else
      end = _add_round_days_to_midnight(end, 24 * 60 * 60);

    // Error checking.
    if (((time_t)-1 == start)
        || ((time_t)-1 == end)
        || (start > end))
      return (false);

    // We should have an interval that includes or is above
    // preferred time.
    if (ti.preferred_time < end)
      break ;
    // Advance to next month (or year) if we've passed this weekday of
    // this month already.
    else {
      month = ti.preftime.tm_mon;
      if (month != 11)
        ++month;
      else {
        month = 0;
        ++year;
      }
    }
  }

  return (true);
}

/**
 *  Calculate start time and end time for date range.
 *
 *  @param[in]  r      Range to calculate start time and end time.
 *  @param[in]  type   Date range type.
 *  @param[in]  ti     Time informations.
 *  @param[out] start  Variable to fill start time.
 *  @param[out] end    Variable to fill end time.
 *
 *  @return True on success, otherwise false.
 */
static bool _daterange_to_time_t(
              daterange const* r,
              unsigned int type,
              time_info const* ti,
              time_t& start,
              time_t& end) {
  typedef bool (*pfunc)(
                   daterange const&,
                   time_info const&,
                   time_t&,
                   time_t&);
  static pfunc tabfunc[] = {
    &_daterange_calendar_date_to_time_t,   // 2009-08-11
    &_daterange_month_date_to_time_t,      // january 1
    &_daterange_month_day_to_time_t,       // day 3
    &_daterange_month_week_day_to_time_t,  // thursday 2 april
    &_daterange_week_day_to_time_t         // wednesday 1
  };

  if (type >= sizeof(tabfunc) / sizeof(*tabfunc))
    return (false);
  if (!(*tabfunc[type])(*r, *ti, start, end))
    return (false);

  // if skipping days...
  if (r->skip_interval > 1) {
    // advance to the next possible skip date
    if (start < ti->preferred_time) {
      // check if interval is across dlst change and
      // gets the compensation
      int shift(get_dst_shift(&start, &ti->midnight));

      // how many days have passed between skip start date
      // and preferred time?
      unsigned long days((shift + ti->midnight
                          - (unsigned long)start) / (3600 * 24));

      // advance start date to next skip day
      if (!(days % r->skip_interval))
        start += (days * 3600 * 24);
      else
        start += ((days - (days % r->skip_interval)
                   + r->skip_interval) * 3600 * 24);
    }
  }

  return (true);
}

/**
 *  Get the earliest midnight of day that includes preferred time or
 *  occurs later.
 *
 *  @param[in] preferred_time     Preferred time.
 *  @param[in] drange             Date range.
 *  @param[in] drange_start_time  Date range start time.
 *  @param[in] drange_end_time    Date range end time.
 *
 *  @return Earliest midnight.
 */
static time_t _earliest_midnight_in_daterange(
                time_t preferred_time,
                daterange* drange,
                time_t drange_start_time,
                time_t drange_end_time) {
  // XXX : handle full day skipping directly (from preferred_time to next midnight)
  while ((drange_start_time < drange_end_time)
         || (drange_end_time == (time_t)-1)) {
    // Next day at midnight.
    time_t next_day(_add_round_days_to_midnight(
                      drange_start_time,
                      24 * 60 * 60));

    // Check range.
    if ((preferred_time < drange_start_time)
        || ((preferred_time >= drange_start_time)
            && (preferred_time < next_day)))
      return (drange_start_time);

    // Move to next day (if range is skipable).
    if (!drange->skip_interval)
      break ;
    else if (drange->skip_interval == 1)
      drange_start_time = next_day;
    else
      drange_start_time = _add_round_days_to_midnight(
                            drange_start_time,
                            drange->skip_interval * 24 * 60 * 60);
  }
  return ((time_t)-1);
}

/**
 *  Get time range limits.
 *
 *  @param[in]  trange       Time range.
 *  @param[in]  midnight     Midnight of day.
 *  @param[out] range_start  Start of time range in this specific day.
 *  @param[out] range_end    End of time range in this specific day.
 *
 *  @return True upon successful conversion.
 */
static bool _timerange_to_time_t(
              timerange* trange,
              struct tm const* midnight,
              time_t& range_start,
              time_t& range_end) {
  struct tm my_tm;
  memcpy(&my_tm, midnight, sizeof(my_tm));
  my_tm.tm_hour = trange->start_hour;
  my_tm.tm_min = trange->start_minute;
  range_start = mktime(&my_tm);
  my_tm.tm_hour = trange->end_hour;
  my_tm.tm_min = trange->end_minute;
  range_end = mktime(&my_tm);
  return (true);
}

// see if the specified time falls into a valid time range in the
// given time period
int check_time_against_period(time_t test_time, timeperiod* tperiod) {
  logger(dbg_functions, basic)
    << "check_time_against_period()";

  // if no period was specified, assume the time is good
  if (!tperiod)
    return (OK);

  // test exclusions first - if exclusions match current time,
  // bail out with an error
  // clear exclusions list before recursing (and restore afterwards)
  // to prevent endless loops...
  timeperiodexclusion* first_exclusion(tperiod->exclusions);
  tperiod->exclusions = NULL;
  for (timeperiodexclusion* exclusion(first_exclusion);
       exclusion;
       exclusion = exclusion->next) {
    if (!check_time_against_period(
          test_time,
          exclusion->timeperiod_ptr)) {
      tperiod->exclusions = first_exclusion;
      return (ERROR);
    }
  }
  tperiod->exclusions = first_exclusion;

  // save values for later
  tm t;
  localtime_r(&test_time, &t);
  int test_time_year(t.tm_year);
  int test_time_mon(t.tm_mon);
  // int test_time_mday(t.tm_mday);
  int test_time_wday(t.tm_wday);

  // calculate the start of the day (midnight, 00:00 hours)
  // when the specified test time occurs
  t.tm_sec = 0;
  t.tm_min = 0;
  t.tm_hour = 0;
  time_t midnight(mktime(&t));

  bool found_match(false);

  // check exceptions first
  for (unsigned int daterange_type(0);
       daterange_type < DATERANGE_TYPES;
       ++daterange_type) {

    for (daterange* drange(tperiod->exceptions[daterange_type]);
         drange;
         drange = drange->next) {

#ifdef TEST_TIMEPERIODS_A
      logger(dbg_events, most)
        << "TYPE: " << daterange_type
        << "\nTEST: " << test_time << " = " << ctime(&test_time)
        << "MIDNIGHT: " << midnight << " = " << ctime(&midnight);
#endif

      time_t start_time(0);
      time_t end_time(0);
      int year(0);

      // get the start time and end time.
      switch (daterange_type) {
      case DATERANGE_CALENDAR_DATE:
        t.tm_sec = 0;
        t.tm_min = 0;
        t.tm_hour = 0;
        t.tm_wday = 0;
        t.tm_isdst = -1;

        t.tm_mday = drange->smday;
        t.tm_mon = drange->smon;
        t.tm_year = drange->syear - 1900;
        if (!(start_time = mktime(&t)))
          continue;

        t.tm_mday = drange->emday;
        t.tm_mon = drange->emon;
        t.tm_year = drange->eyear - 1900;
        if (!(end_time = mktime(&t)))
          continue;
        break;

      case DATERANGE_MONTH_DATE:
        start_time = calculate_time_from_day_of_month(
                       test_time_year,
                       drange->smon,
                       drange->smday);
        if (!start_time)
          continue;

        year = test_time_year;
        end_time = calculate_time_from_day_of_month(
                     year,
                     drange->emon,
                     drange->emday);
        // advance a year if necessary: august 2 - february 5
        if (end_time < start_time)
          end_time = calculate_time_from_day_of_month(
                       ++year,
                       drange->emon,
                       drange->emday);

        // end date was bad - see if we can handle the error
        if (!end_time) {
          // end date can't be helped, so skip it
          if (drange->emday < 0)
            continue;

          // else end date slipped past end of month, so use
          // last day of month as end date
          // use same year calculated above
          end_time = calculate_time_from_day_of_month(
                       year,
                       drange->emon,
                       -1);
        }
        break;

      case DATERANGE_MONTH_DAY:
        start_time = calculate_time_from_day_of_month(
                       test_time_year,
                       test_time_mon,
                       drange->smday);
        if (!start_time)
          continue;

        end_time = calculate_time_from_day_of_month(
                     test_time_year,
                     test_time_mon,
                     drange->emday);

        // end date was bad - see if we can handle the error
        if (!end_time) {
          // end date can't be helped, so skip it
          if (drange->emday < 0)
            continue;

          // else end date slipped past end of month, so use
          // last day of month as end date
          end_time = calculate_time_from_day_of_month(
                       test_time_year,
                       test_time_mon,
                       -1);
        }
        break;

      case DATERANGE_MONTH_WEEK_DAY:
        start_time = calculate_time_from_weekday_of_month(
                       test_time_year,
                       drange->smon,
                       drange->swday,
                       drange->swday_offset);
        if (!start_time)
          continue;

        year = test_time_year;
        end_time = calculate_time_from_weekday_of_month(
                     year,
                     drange->emon,
                     drange->ewday,
                     drange->ewday_offset);
        // advance a year if necessary:
        // thursday 2 august - monday 3 february
        if (end_time < start_time)
          end_time = calculate_time_from_weekday_of_month(
                       ++year,
                       drange->emon,
                       drange->ewday,
                       drange->ewday_offset);

        // end date was bad - see if we can handle the error
        if (!end_time) {
          // end date can't be helped, so skip it
          if (drange->ewday_offset < 0)
            continue;

          // else end date slipped past end of month, so use
          // last day of month as end date
          // use same year calculated above
          end_time = calculate_time_from_day_of_month(
                       year,
                       test_time_mon,
                       -1);
        }
        break;

      case DATERANGE_WEEK_DAY:
        start_time = calculate_time_from_weekday_of_month(
                       test_time_year,
                       test_time_mon,
                       drange->swday,
                       drange->swday_offset);
        if (!start_time)
          continue;

        end_time = calculate_time_from_weekday_of_month(
                     test_time_year,
                     test_time_mon,
                     drange->ewday,
                     drange->ewday_offset);

        // end date was bad - see if we can handle the error
        if (!end_time) {
          // end date can't be helped, so skip it
          if (drange->ewday_offset < 0)
            continue;

          // else end date slipped past end of month, so use
          // last day of month as end date
          end_time = calculate_time_from_day_of_month(
                       test_time_year,
                       test_time_mon,
                       -1);
        }
        break;

      default:
        continue;
      }

#ifdef TEST_TIMEPERIODS_A
      logger(dbg_events, most)
        << "START: " << start_time << " = " << ctime(&start_time)
        << "END: " << end_time << " = " << ctime(&end_time);
#endif

      // calculate skip date start (and end)
      if (drange->skip_interval > 1) {
        // skip start date must be before test time
        if (start_time > test_time)
          continue;

        // check if interval is accress dlst change
        // and gets the compensation.
        int shift(get_dst_shift(&start_time, &midnight));

        // how many days have passed between skip
        // start date and test time?
        unsigned long days((shift + (unsigned long)midnight
                            - (unsigned long)start_time) / (3600 * 24));

        // if test date doesn't fall on a skip
        // interval day, bail out early
        if ((days % drange->skip_interval))
          continue;

        // use midnight of test date as start time
        start_time = midnight;

        // if skipping range has no end, use test date as end
        if (daterange_type == DATERANGE_CALENDAR_DATE
            && is_daterange_single_day(drange))
          end_time = midnight;
      }

#ifdef TEST_TIMEPERIODS_A
      logger(dbg_events, most)
        << "NEW START: " << start_time << " = " << ctime(&start_time)
        << "NEW END: " << end_time << " = " << ctime(&end_time)
        << days << " DAYS PASSED\n"
        "DLST SHIFT: " << shift;
#endif

      // time falls into the range of days
      if (midnight >= start_time && midnight <= end_time)
        found_match = true;

      // found a day match, so see if time ranges are good
      if (found_match) {
        for (timerange* trange(drange->times);
             trange;
             trange = trange->next) {

//           // ranges with start/end of zero mean exlude this day
//           if (!trange->range_start && !trange->range_end) {
// #ifdef TEST_TIMEPERIODS_A
//             logger(dbg_events, most)
//               << "0 MINUTE RANGE EXCLUSION";
// #endif
//             continue;
//           }

          time_t day_range_start(midnight + trange->start_hour * 60 * 60 + trange->start_minute * 60);
          time_t day_range_end(midnight + trange->end_hour * 60 * 60 + trange->end_minute * 60);

#ifdef TEST_TIMEPERIODS_A
          logger(dbg_events, most)
            << "  RANGE START: " << trange->range_start
            << " (" << day_range_start << ") = "
            << ctime(&day_range_start)
            << "  RANGE END: " << trange->range_end
            << " (" << day_range_end << ") = "
            << ctime(&day_range_end);
#endif

          // if the user-specified time falls in this range,
          // return with a positive result
          if (test_time >= day_range_start
              && test_time <= day_range_end)
            return (OK);
        }

        // no match, so bail with error
        return (ERROR);
      }
    }
  }

  // check normal, weekly rotating schedule last

  // check weekday time ranges
  for (timerange* trange(tperiod->days[test_time_wday]);
       trange;
       trange = trange->next) {

    time_t day_range_start(midnight + trange->start_hour * 60 * 60 + trange->start_minute * 60);
    time_t day_range_end(midnight + trange->end_hour * 60 * 60 + trange->end_minute * 60);

    // if the user-specified time falls in this range,
    // return with a positive result
    if (test_time >= day_range_start && test_time <= day_range_end)
      return (OK);
  }

  return (ERROR);
}

/**
 *  This function is for timeperiod exclusions,
 *  Icinga special from #459
 *
 *  @param[in]  preferred_time  The preferred time to check.
 *  @param[out] valid_time      Variable to fill.
 *  @param[in]  current_time    The current time.
 *  @param[in]  tperiod         The time period to use.
 */
static void _get_min_invalid_time_per_timeperiod(
       time_t preferred_time,
       time_t* valid_time,
       time_t current_time,
       timeperiod* tperiod) {
  logger(dbg_functions, basic)
    << "get_min_valid_time_per_timeperiod()";

  // if no timeperiod, go with preferred time
  if (!tperiod) {
    *valid_time = preferred_time;
    return;
  }

  time_info ti;
  ti.preferred_time = preferred_time;
  ti.current_time = current_time;
  localtime_r(&current_time, &ti.curtime);

  // calculate the start of the day (midnight, 00:00 hours)
  // of preferred time
  localtime_r(&preferred_time, &ti.preftime);
  ti.preftime.tm_sec = 0;
  ti.preftime.tm_min = 0;
  ti.preftime.tm_hour = 0;
  ti.midnight = mktime(&ti.preftime);

#ifdef TEST_TIMEPERIODS_B
  logger(dbg_events, most)
    << "PREF TIME: " << preferred_time
    << " = " << ctime(&preferred_time)
    << "CURRENT TIME: " << current_time
    << " = " << ctime(&current_time)
    << "PREF YEAR: " << ti.preftime.tm_year
    << ", MON: " << ti.preftime.tm_mon
    << ", MDAY: " << ti.preftime.tm_mday
    << ", WDAY: " << ti.preftime.tm_wday
    << "\nCURRENT YEAR: " << ti.curtime.tm_year
    << ", MON: " << ti.curtime.tm_mon
    << ", MDAY: " << ti.curtime.tm_mday
    << ", WDAY: " << ti.curtime.tm_wday;
#endif // TEST_TIMEPERIODS_B

  bool have_latest_time(false);
  time_t latest_time(0);
  time_t earliest_day(0);

  // check exceptions (in this timeperiod definition) first
  for (unsigned int daterange_type(0);
       daterange_type < DATERANGE_TYPES;
       ++daterange_type) {

#ifdef TEST_TIMEPERIODS_B
    logger(dbg_events, most)
      << "TYPE: " << daterange_type;
#endif // TEST_TIMEPERIODS_B

    for (daterange* drange(tperiod->exceptions[daterange_type]);
         drange;
         drange = drange->next) {

      time_t start_time(0);
      time_t end_time(0);
      if (!_daterange_to_time_t(
             drange,
             daterange_type,
             &ti,
             start_time,
             end_time))
        continue;

      // skip this date range its out of bounds with what we want
      if (preferred_time > end_time)
        continue;

      // how many days at a time should we advance?
      unsigned long advance_interval(1);
      if (drange->skip_interval > 1)
        advance_interval = drange->skip_interval;

      // advance through the date range
      for (time_t day_start(start_time);
           day_start <= end_time;
           day_start += advance_interval * 3600 * 24) {

        // we already found a time from a higher-precendence
        // date range exception
        // here we use have_latest_time instead of have_earliest_time
        if (day_start >= earliest_day && have_latest_time)
          continue;

        for (timerange* trange(drange->times);
             trange;
             trange = trange->next) {

          // REMOVED
          // ranges with start/end of zero mean exlude this day
          // if (!trange->range_start && !trange->range_end)
          //    continue;

          time_t day_range_end(day_start + trange->end_hour * 60 * 60 + trange->end_minute * 60);
#ifdef TEST_TIMEPERIODS_B
          time_t day_range_start(day_start + trange->range_start);
          logger(dbg_events, most)
            << "  RANGE START: " << trange->range_start << " ("
            << day_range_start << ") = " << ctime(&day_range_start)
            << "  RANGE END: " << trange->range_end
            << " (" << day_range_end << ") = "
            << ctime(&day_range_end);
#endif // TEST_TIMEPERIODS_B

          // range is out of bounds
          if (day_range_end < preferred_time)
            continue;

          // is this the earliest time found thus far?
          if (!have_latest_time || day_range_end < latest_time) {
            // save it as latest_time instead of earliest_time
            have_latest_time = true;
            latest_time = day_range_end;
            earliest_day = day_start;
          }
        }
      }
    }
  }

  // find next available time from normal, weekly rotating schedule
  // (in this timeperiod definition)

  // check a one week rotation of time
  bool has_looped(false);
  for (int weekday(ti.preftime.tm_wday), days_into_the_future(0);
       ;
       ++weekday, ++days_into_the_future) {

    // break out of the loop if we have checked an entire week already
    if (has_looped && weekday >= ti.preftime.tm_wday)
      break;

    if (weekday >= 7) {
      weekday -= 7;
      has_looped = true;
    }

    // calculate start of this future weekday
    time_t day_start((time_t)(ti.midnight + (days_into_the_future * 3600 * 24)));

    // we already found a time from a higher-precendence
    // date range exception
    if (day_start == earliest_day)
      continue;

    // check all time ranges for this day of the week
    for (timerange* trange(tperiod->days[weekday]);
         trange;
         trange = trange->next) {

      // calculate day_range_end to assign to lastest_time again
      time_t day_range_end((time_t)(day_start + trange->end_hour * 60 * 60 + trange->end_minute * 60));

      if ((!have_latest_time
           || day_range_end < latest_time)
          && day_range_end >= preferred_time) {
        have_latest_time = true;
        latest_time = day_range_end;
        earliest_day = day_start;
      }
    }
  }

  // if we couldn't find a time period there must be none defined
  if (!have_latest_time || !latest_time)
    *valid_time = preferred_time;
  // else use the calculated time
  else
    *valid_time = latest_time;
  return;
}

/**
 *  Get the next valid time within a time period.
 *
 *  @param[in]  preferred_time  The preferred time to check.
 *  @param[out] valid_time      Variable to fill.
 *  @param[in]  current_time    The current time.
 *  @param[in]  tperiod         The time period to use.
 */
#include <iostream>
static void _get_next_valid_time_per_timeperiod(
              time_t preferred_time,
              time_t* valid_time,
              time_t current_time,
              timeperiod* tperiod) {
  logger(dbg_functions, basic)
    << "get_next_valid_time_per_timeperiod()";

  // If no timeperiod, go with preferred time.
  if (!tperiod) {
    *valid_time = preferred_time;
    return ;
  }

  // If no time can be found, the original preferred time will be set
  // in valid_time at the end of the loop.
  time_t original_preferred_time(preferred_time);

  // Do not compute more than one year ahead (we might compute forever).
  time_t earliest_time((time_t)-1);
  time_t in_one_year(preferred_time + 366 * 24 * 60 * 60);
  while ((earliest_time == (time_t)-1)
         && (preferred_time < in_one_year)) {
    std::cout << "LOOP\n";
    // Compute time information.
    time_info ti;
    ti.preferred_time = preferred_time;
    ti.current_time = current_time;
    localtime_r(&current_time, &ti.curtime);
    localtime_r(&preferred_time, &ti.preftime);
    ti.preftime.tm_sec = 0;
    ti.preftime.tm_min = 0;
    ti.preftime.tm_hour = 0;
    ti.midnight = mktime(&ti.preftime);

    // XXX : handle range end reached
    // Browse all date range.
    for (unsigned int daterange_type(0);
         daterange_type < DATERANGE_TYPES;
         ++daterange_type) {
      for (daterange* drange(tperiod->exceptions[daterange_type]);
           drange;
           drange = drange->next) {
        // Get range limits.
        time_t daterange_start_time(0);
        time_t daterange_end_time(0);
        if (_daterange_to_time_t(
              drange,
              daterange_type,
              &ti,
              daterange_start_time,
              daterange_end_time)
            && ((preferred_time < daterange_end_time)
                || (daterange_end_time == (time_t)-1))) {
          std::cout << "DATERANGE IS " << ctime(&daterange_start_time);
          std::cout << "-" << ctime(&daterange_end_time) << "\n";
          // Check that date if within range.
          time_t earliest_midnight(_earliest_midnight_in_daterange(
                                     preferred_time,
                                     drange,
                                     daterange_start_time,
                                     daterange_end_time));
          if (earliest_midnight != (time_t)-1) {
            std::cout << "GOT EARLIEST MIDNIGHT\n";
            // Midnight.
            struct tm midnight;
            localtime_r(&earliest_midnight, &midnight);

            // Browse all time range of date range.
            for (timerange* trange(drange->times);
                 trange;
                 trange = trange->next) {
              // Get range limits.
              time_t range_start(0);
              time_t range_end(0);
              if (_timerange_to_time_t(
                    trange,
                    &midnight,
                    range_start,
                    range_end)) {
                time_t potential_time((time_t)-1);
                // Range is out of bound.
                if (preferred_time <= range_end) {
                  // Preferred time occurs before range start, so use
                  // range start time as earliest potential time.
                  if (range_start >= preferred_time)
                    potential_time = range_start;
                  // Preferred time occurs between range start/end, so
                  // use preferred time as earliest potential time.
                  else
                    potential_time = preferred_time;

                  // Is this the earliest time found thus far ?
                  if ((earliest_time == (time_t)-1)
                      || (potential_time < earliest_time))
                    earliest_time = potential_time;
                }
              }
            }
          }
        }
      }
    }

    // Check week day.
    // XXX

    // Check exclusions.
    // XXX : reset earliest time + move preferred time past exclusion

    // Increment preferred time.
    if (earliest_time == (time_t)-1)
      preferred_time = _add_round_days_to_midnight(
                         ti.midnight,
                         24 * 60 * 60);
  }

  // If we couldn't find a time period there must be none defined.
  if (earliest_time == (time_t)-1)
    *valid_time = original_preferred_time;
  // Else use the calculated time.
  else
    *valid_time = earliest_time;

  return ;

//   // find next available time from normal, weekly
//   // rotating schedule (in this timeperiod definition)

//   // check a one week rotation of time
//   bool has_looped(false);
//   for (int weekday(ti.preftime.tm_wday), days_into_the_future(0);
//        ;
//        ++weekday, ++days_into_the_future) {

//     // Break out of the loop if we have checked an entire
//     // week already (8 days total).
//     if (has_looped && days_into_the_future > 7)
//       break ;

//     if (weekday >= 7) {
//       weekday -= 7;
//       has_looped = true;
//     }

//     // calculate start of this future weekday
//     time_t day_start((time_t)(ti.midnight + (days_into_the_future * 3600 * 24)));

//     // we already found a time from a higher-precendence
//     // date range exception
//     if (day_start == earliest_day)
//       continue;

//     // check all time ranges for this day of the week
//     for (timerange* trange(tperiod->days[weekday]);
//          trange;
//          trange = trange->next) {

//       // calculate the time for the start of this time range
//       time_t day_range_start(day_start + trange->range_start);

//       if ((!have_earliest_time
//            || day_range_start < earliest_time)
//           && day_range_start >= preferred_time) {
//         have_earliest_time = true;
//         earliest_time = day_range_start;
//         earliest_day = day_start;
//       }
//     }
//   }

}

/**
 *  Calculate the earliest time possible including checks
 *  for timepriod exclusion
 *
 *  @param[in]  preferred_time  The preferred time to check.
 *  @param[out] valid_time      Variable to fill.
 *  @param[in]  current_time    The current time.
 *  @param[in]  tperiod         The time period to use.
 *  @param[in]  level           The recursivity level.
 */
static void get_earliest_time(
              time_t preferred_time,
              time_t* valid_time,
              time_t current_time,
              timeperiod* tperiod,
              int level) {
  time_t earliest_time(0);

  /*
   * function can get called recursivly, pushing alternating level
   * see below in timeperiod exclusion loop
   */
  if (!(level % 2)) {
    // initial hit here, and then alternating by 2 the timeperiod exclusions
    _get_next_valid_time_per_timeperiod(
      preferred_time,
      &earliest_time,
      current_time,
      tperiod);
    if (*valid_time == 0)
      *valid_time = earliest_time;
    else if (earliest_time < *valid_time)
      *valid_time = earliest_time;
  }
  else {
    // first timeperiod exclusion hits here, alternating by 2
    _get_min_invalid_time_per_timeperiod(
      preferred_time,
      &earliest_time,
      current_time,
      tperiod);
    if (*valid_time == 0)
      *valid_time = earliest_time;
    else if (earliest_time < *valid_time)
      *valid_time = earliest_time + 1;
  }

  // loop through all available exclusions in this timeperiod and alternate level
  timeperiodexclusion* first_exclusion(tperiod->exclusions);
  tperiod->exclusions = NULL;
  for (timeperiodexclusion* exclusion(first_exclusion);
       exclusion;
       exclusion = exclusion->next)
    get_earliest_time(
      preferred_time,
      valid_time,
      current_time,
      exclusion->timeperiod_ptr,
      level + 1);
  tperiod->exclusions = first_exclusion;
}

/**
 *  Given a preferred time, get the next valid time within a time
 *  period.
 *
 *  @param[in]  preferred_time  The preferred time to check.
 *  @param[out] valid_time      Variable to fill.
 *  @param[in]  tperiod         The time period to use.
 */
void get_next_valid_time(
       time_t pref_time,
       time_t* valid_time,
       timeperiod* tperiod) {
  logger(dbg_functions, basic) << "get_next_valid_time()";

  // Preferred time must be now or in the future.
  time_t current_time(time(NULL));
  time_t preferred_time(std::max(pref_time, current_time));

  // If no timeperiod, go with the preferred time.
  if (!tperiod) {
    *valid_time = preferred_time;
    return ;
  }
  // First check for possible timeperiod exclusions
  // before getting a valid_time.
  else {
    *valid_time = 0;
    get_earliest_time(
      preferred_time,
      valid_time,
      current_time,
      tperiod,
      0);
  }

  return ;
}

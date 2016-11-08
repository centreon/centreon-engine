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

#include <gtest/gtest.h>
#include "com/centreon/engine/objects/timeperiod.hh"
#include "com/centreon/engine/timeperiod.hh"
#include "tests/timeperiod/utils.hh"

using namespace com::centreon::engine;

class        GetNextValidTimeBackwardDST : public ::testing::Test {
 public:
  void       SetUp() {
    _creator.new_timeperiod();
    _computed = (time_t)-1;
  }

  //
  // DATE RANGES
  //

  daterange* dst_calendar_date() {
    return (_creator.new_calendar_date(2016, 9, 30, 2016, 9, 30));
  }

  daterange* dst_specific_month_date() {
    return (_creator.new_specific_month_date(9, 30, 9, 30));
  }

  daterange* dst_generic_month_date() {
    return (_creator.new_generic_month_date(30, 30));
  }

  daterange* dst_offset_weekday_of_specific_month() {
    return (_creator.new_offset_weekday_of_specific_month(
                       9,
                       0,
                       5,
                       9,
                       0,
                       5));
  }

  daterange* dst_offset_weekday_of_generic_month() {
    return (_creator.new_offset_weekday_of_generic_month(0, 5, 0, 5));
  }

  //
  // TIME RANGES
  //

  void       timerange_includes_dst(daterange* dr) {
    _creator.new_timerange(1, 0, 4, 0, dr);
  }

  void       timerange_excludes_dst(daterange* dr) {
    _creator.new_timerange(1, 0, 2, 0, dr);
    _creator.new_timerange(3, 0, 4, 0, dr);
  }

  void       timerange_within_dst(daterange* dr) {
    _creator.new_timerange(2, 15, 2, 45, dr);
  }

  void       timerange_stop_in_dst(daterange* dr) {
    _creator.new_timerange(1, 0, 2, 30, dr);
  }

  //
  // CURRENT TIME
  //

  void       beginning_of_overlapped_hour() {
    _now = strtotimet("2016-10-30 01:59:59") + 1;
    set_time(_now);
  }

  void       at_dst() {
    _now = strtotimet("2016-10-30 03:00:00") - 3600;
    set_time(_now);
  }

  void       end_of_timerange() {
    _now = strtotimet("2016-10-30 01:59:59") + 1 + 45 * 60;
    set_time(_now);
  }

 protected:
  time_t             _computed;
  timeperiod_creator _creator;
  time_t             _now;
};

//
// RESULTS WILL BE MOSTLY THE SAME FOR ALL DATE RANGES
//
// So instead of writing all Given/When/Then, here are the common cases.
//
// Given a timeperiod covering a backward DST change
// And the timeranges includes the DST change
// And we are at the beginning of the hour that will be overlapped, still in the original timezone
// When get_next_valid_time() is called
// Then the next valid time is now
//
// Given a timeperiod covering a backward DST change
// And the timeranges excludes the DST change
// And we are at the beginning of the hour that will be overlapped, still in the original timezone
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the timerange in the new timezone
//
// Given a timeperiod covering a backward DST change
// And the timeranges are within the overlapped hour
// And we are at the beginning of the hour that will be overlapped, still in the original timezone
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the timerange in the original timezone
//
// Given a timeperiod covering a backward DST change
// And the timeranges are within the overlapped hour
// And we are at the end of the timerange in the original timezone
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the timerange in the new timezone
//
// Given a timeperiod covering a backward DST change
// And the timeranges includes the DST range
// And we are at DST
// When get_next_valid_time() is called
// Then the next valid time is now
//
// Given a timeperiod covering a backward DST change
// And the timeranges excludes the DST range
// And we are at DST
// When get_next_valid_time() is callled
// Then the next valid time is the beginning of the timerange in the new timezone
//
// Given a timeperiod covering a backward DST change
// And the timeranges are within the DST range
// And we are at DST
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the timerange in the new timezone
//
// Given a timeperiod covering a backward DST change
// And the timeranges stop during the overlapped hour
// And we are at the end of the timerange in the original timezone
// When get_next_valid_time() is called
// Then the next valid time is the DST change


//
// CALENDAR DATE TESTS
//

TEST_F(GetNextValidTimeBackwardDST, CalendarDateIncludeDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_calendar_date());
  timerange_includes_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 01:59:59") + 1);
}

TEST_F(GetNextValidTimeBackwardDST, CalendarDateExcludeDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_calendar_date());
  timerange_excludes_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00"));
}

TEST_F(GetNextValidTimeBackwardDST, CalendarDateWithinDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_calendar_date());
  timerange_within_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 01:59:59") + 1 + 15 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, CalendarDateWithinDSTNowAtEndOfTimerange) {
  daterange* dr(dst_calendar_date());
  timerange_within_dst(dr);
  end_of_timerange();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 45 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, CalendarDateIncludeDSTNowAtDST) {
  daterange* dr(dst_calendar_date());
  timerange_includes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 3600);
}

TEST_F(GetNextValidTimeBackwardDST, CalendarDateExcludeDSTNowAtDST) {
  daterange* dr(dst_calendar_date());
  timerange_excludes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00"));
}

TEST_F(GetNextValidTimeBackwardDST, CalendarDateWithinDSTNowAtDST) {
  daterange* dr(dst_calendar_date());
  timerange_within_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 45 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, CalendarDateStopInDSTNowAtEndOfTimerange) {
  daterange* dr(dst_calendar_date());
  timerange_stop_in_dst(dr);
  end_of_timerange();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 3600);
}

//
// SPECIFIC MONTH DATE TESTS
//

TEST_F(GetNextValidTimeBackwardDST, SpecificMonthDateIncludeDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_specific_month_date());
  timerange_includes_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 01:59:59") + 1);
}

TEST_F(GetNextValidTimeBackwardDST, SpecificMonthDateExcludeDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_specific_month_date());
  timerange_excludes_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00"));
}

TEST_F(GetNextValidTimeBackwardDST, SpecificMonthDateWithinDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_specific_month_date());
  timerange_within_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 01:59:59") + 1 + 15 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, SpecificMonthDateWithinDSTNowAtEndOfTimerange) {
  daterange* dr(dst_specific_month_date());
  timerange_within_dst(dr);
  end_of_timerange();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 45 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, SpecificMonthDateIncludeDSTNowAtDST) {
  daterange* dr(dst_specific_month_date());
  timerange_includes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 3600);
}

TEST_F(GetNextValidTimeBackwardDST, SpecificMonthDateExcludeDSTNowAtDST) {
  daterange* dr(dst_specific_month_date());
  timerange_excludes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00"));
}

TEST_F(GetNextValidTimeBackwardDST, SpecificMonthDateWithinDSTNowAtDST) {
  daterange* dr(dst_specific_month_date());
  timerange_within_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 45 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, SpecificMonthDateStopInDSTNowAtEndOfTimerange) {
  daterange* dr(dst_specific_month_date());
  timerange_stop_in_dst(dr);
  end_of_timerange();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 3600);
}

//
// GENERIC MONTH DATE TESTS
//

TEST_F(GetNextValidTimeBackwardDST, GenericMonthDateIncludeDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_generic_month_date());
  timerange_includes_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 01:59:59") + 1);
}

TEST_F(GetNextValidTimeBackwardDST, GenericMonthDateExcludeDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_generic_month_date());
  timerange_excludes_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00"));
}

TEST_F(GetNextValidTimeBackwardDST, GenericMonthDateWithinDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_generic_month_date());
  timerange_within_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 01:59:59") + 1 + 15 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, GenericMonthDateWithinDSTNowAtEndOfTimerange) {
  daterange* dr(dst_generic_month_date());
  timerange_within_dst(dr);
  end_of_timerange();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 45 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, GenericMonthDateIncludeDSTNowAtDST) {
  daterange* dr(dst_generic_month_date());
  timerange_includes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 3600);
}

TEST_F(GetNextValidTimeBackwardDST, GenericMonthDateExcludeDSTNowAtDST) {
  daterange* dr(dst_generic_month_date());
  timerange_excludes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00"));
}

TEST_F(GetNextValidTimeBackwardDST, GenericMonthDateWithinDSTNowAtDST) {
  daterange* dr(dst_generic_month_date());
  timerange_within_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 45 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, GenericMonthDateStopInDSTNowAtEndOfTimerange) {
  daterange* dr(dst_generic_month_date());
  timerange_stop_in_dst(dr);
  end_of_timerange();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 3600);
}

//
// OFFSET WEEKDAY OF SPECIFIC MONTH TESTS
//

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfSpecificMonthIncludeDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_offset_weekday_of_specific_month());
  timerange_includes_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 01:59:59") + 1);
}

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfSpecificMonthExcludeDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_offset_weekday_of_specific_month());
  timerange_excludes_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00"));
}

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfSpecificMonthWithinDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_offset_weekday_of_specific_month());
  timerange_within_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 01:59:59") + 1 + 15 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfSpecificMonthWithinDSTNowAtEndOfTimerange) {
  daterange* dr(dst_offset_weekday_of_specific_month());
  timerange_within_dst(dr);
  end_of_timerange();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 45 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfSpecificMonthIncludeDSTNowAtDST) {
  daterange* dr(dst_offset_weekday_of_specific_month());
  timerange_includes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 3600);
}

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfSpecificMonthExcludeDSTNowAtDST) {
  daterange* dr(dst_offset_weekday_of_specific_month());
  timerange_excludes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00"));
}

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfSpecificMonthWithinDSTNowAtDST) {
  daterange* dr(dst_offset_weekday_of_specific_month());
  timerange_within_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 45 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfSpecificMonthStopInDSTNowAtEndOfTimerange) {
  daterange* dr(dst_offset_weekday_of_specific_month());
  timerange_stop_in_dst(dr);
  end_of_timerange();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 3600);
}

//
// OFFSET WEEKDAY OF GENERIC MONTH TESTS
//

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfGenericMonthIncludeDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_offset_weekday_of_generic_month());
  timerange_includes_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 01:59:59") + 1);
}

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfGenericMonthExcludeDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_offset_weekday_of_generic_month());
  timerange_excludes_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00"));
}

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfGenericMonthWithinDSTNowAtBeginningOfOverlappedHour) {
  daterange* dr(dst_offset_weekday_of_generic_month());
  timerange_within_dst(dr);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 01:59:59") + 1 + 15 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfGenericMonthWithinDSTNowAtEndOfTimerange) {
  daterange* dr(dst_offset_weekday_of_generic_month());
  timerange_within_dst(dr);
  end_of_timerange();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 45 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfGenericMonthIncludeDSTNowAtDST) {
  daterange* dr(dst_offset_weekday_of_generic_month());
  timerange_includes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 3600);
}

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfGenericMonthExcludeDSTNowAtDST) {
  daterange* dr(dst_offset_weekday_of_generic_month());
  timerange_excludes_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00"));
}

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfGenericMonthWithinDSTNowAtDST) {
  daterange* dr(dst_offset_weekday_of_generic_month());
  timerange_within_dst(dr);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 45 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, OffsetWeekdayOfGenericMonthStopInDSTNowAtEndOfTimerange) {
  daterange* dr(dst_offset_weekday_of_generic_month());
  timerange_stop_in_dst(dr);
  end_of_timerange();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 3600);
}

//
// NORMAL WEEKDAY TESTS
//

TEST_F(GetNextValidTimeBackwardDST, NormalWeekdayIncludeDSTNowAtBeginningOfOverlappedHour) {
  _creator.new_timerange(1, 0, 4, 0, 0);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 01:59:59") + 1);
}

TEST_F(GetNextValidTimeBackwardDST, NormalWeekdayExcludeDSTNowAtBeginningOfOverlappedHour) {
  _creator.new_timerange(1, 0, 2, 0, 0);
  _creator.new_timerange(3, 0, 4, 0, 0);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00"));
}

TEST_F(GetNextValidTimeBackwardDST, NormalWeekdayWithinDSTNowAtBeginningOfOverlappedHour) {
  _creator.new_timerange(2, 15, 2, 45, 0);
  beginning_of_overlapped_hour();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 01:59:59") + 1 + 15 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, NormalWeekdayWithinDSTNowAtEndOfTimerange) {
  _creator.new_timerange(2, 15, 2, 45, 0);
  end_of_timerange();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 45 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, NormalWeekdayIncludeDSTNowAtDST) {
  _creator.new_timerange(1, 0, 4, 0, 0);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 3600);
}

TEST_F(GetNextValidTimeBackwardDST, NormalWeekdayExcludeDSTNowAtDST) {
  _creator.new_timerange(1, 0, 2, 0, 0);
  _creator.new_timerange(3, 0, 4, 0, 0);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00"));
}

TEST_F(GetNextValidTimeBackwardDST, NormalWeekdayWithinDSTNowAtDST) {
  _creator.new_timerange(2, 15, 2, 45, 0);
  at_dst();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 45 * 60);
}

TEST_F(GetNextValidTimeBackwardDST, NormalWeekdayStopInDSTNowAtEndOfTimerange) {
  _creator.new_timerange(1, 0, 2, 30, 0);
  end_of_timerange();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-10-30 03:00:00") - 3600);
}

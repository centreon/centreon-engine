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
#include "com/centreon/clib.hh"
#include "com/centreon/engine/configuration/applier/state.hh"
#include "com/centreon/engine/timeperiod.hh"
#include "tests/timeperiod/utils.hh"

using namespace com::centreon;
using namespace com::centreon::engine;

class GetNextValidTimeEarliestDaterangeFirstTest : public testing::Test {
 public:
  void SetUp() override {
    // All dateranges are based on the same day : 2016-11-07.
    _creator.new_timeperiod();
    _now = strtotimet("2016-11-07 00:00:00");
    set_time(_now);
  }

  // monday 06:00-07:00
  void normal_weekday_and_later() {
    _creator.new_timerange(6, 0, 7, 0, 1);
    offset_weekday_of_generic_month_and_later();
  }

  // tuesday 2 05:00-06:00
  void offset_weekday_of_generic_month_and_later() {
    daterange* dr(_creator.new_offset_weekday_of_generic_month(2, 2, 2, 2));
    _creator.new_timerange(5, 0, 6, 0, dr);
    offset_weekday_of_specific_month_and_later();
  }

  // wednesday 2 november 04:00-05:00
  void offset_weekday_of_specific_month_and_later() {
    daterange* dr(
        _creator.new_offset_weekday_of_specific_month(10, 3, 2, 10, 3, 2));
    _creator.new_timerange(4, 0, 5, 0, dr);
    generic_month_date_and_later();
  }

  // day 10 03:00-04:00
  void generic_month_date_and_later() {
    daterange* dr(_creator.new_generic_month_date(10, 10));
    _creator.new_timerange(3, 0, 4, 0, dr);
    specific_month_date_and_later();
  }

  // november 11 02:00-03:00
  void specific_month_date_and_later() {
    daterange* dr(_creator.new_specific_month_date(10, 11, 10, 11));
    _creator.new_timerange(2, 0, 3, 0, dr);
    calendar_date_and_later();
  }

  // 2016-11-12 01:00-02:00
  // saturday 00:00-01:00
  void calendar_date_and_later() {
    daterange* dr(_creator.new_calendar_date(2016, 10, 12, 2016, 10, 12));
    _creator.new_timerange(1, 0, 2, 0, dr);
    _creator.new_timerange(0, 0, 1, 0, 6);
  }

 protected:
  timeperiod_creator _creator;
  time_t _now;
};

// Given a timeperiod configured with multiple date ranges
// And a normal weekday daterange occurs first
// When get_next_valid_time() is called
// Then the next valid time is based on the normal weekday's timeranges
TEST_F(GetNextValidTimeEarliestDaterangeFirstTest, NormalWeekdayFirst) {
  normal_weekday_and_later();
  time_t computed((time_t)-1);
  get_next_valid_time(_now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-07 06:00:00"));
}

// Given a timeperiod configured with multiple date ranges
// And an offset weekday of generic month daterange occurs first
// When get_next_valid_time() is called
// Then the next valid time is based on the offset weekday of generic month's
// timeranges
TEST_F(GetNextValidTimeEarliestDaterangeFirstTest,
       OffsetWeekdayOfGenericMonthFirst) {
  offset_weekday_of_generic_month_and_later();
  time_t computed((time_t)-1);
  get_next_valid_time(_now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-08 05:00:00"));
}

// Given a timeperiod configured with multiple date ranges
// And an offset weekday of specific month daterange occurs first
// When get_next_valid_time() is called
// Then the next valid time is based on the offset weekday of specific month's
// timeranges
TEST_F(GetNextValidTimeEarliestDaterangeFirstTest,
       OffsetWeekdayOfSpecificMonthFirst) {
  offset_weekday_of_specific_month_and_later();
  time_t computed((time_t)-1);
  get_next_valid_time(_now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-09 04:00:00"));
}

// Given a timeperiod configured with multiple date ranges
// And a generic month date daterange occurs first
// When get_next_valid_time() is called
// Then the next valid time is based on the generic month date's timeranges
TEST_F(GetNextValidTimeEarliestDaterangeFirstTest, GenericMonthDateFirst) {
  generic_month_date_and_later();
  time_t computed((time_t)-1);
  get_next_valid_time(_now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-10 03:00:00"));
}

// Given a timeperiod configured with multiple date ranges
// And a specific month date daterange occurs first
// When get_next_valid_time() is called
// Then the next valid time is based on the specific month date's timeranges
TEST_F(GetNextValidTimeEarliestDaterangeFirstTest, SpecificMonthDateFirst) {
  specific_month_date_and_later();
  time_t computed((time_t)-1);
  get_next_valid_time(_now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-11 02:00:00"));
}

// Given a timeperiod configured with multiple date ranges
// And a calendar date daterange occurs first
// When get_next_valid_time() is called
// Then the next valid time is based on the calendar date's timeranges
TEST_F(GetNextValidTimeEarliestDaterangeFirstTest, CalendarDateFirst) {
  calendar_date_and_later();
  time_t computed((time_t)-1);
  get_next_valid_time(_now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-12 01:00:00"));
}

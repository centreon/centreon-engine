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

class GetNextValidTimeBetweenTwoYears : public ::testing::Test {
 public:
  void SetUp() override {
    _creator.new_timeperiod();
    _computed = (time_t)-1;
  }

  //
  // DATE RANGES
  //

  void calendar_date() {
    daterange* dr(_creator.new_calendar_date(2016, 11, 20, 2017, 0, 10));
    _creator.new_timerange(0, 0, 24, 0, dr);
    dr = _creator.new_calendar_date(2017, 11, 20, 2018, 0, 10);
    _creator.new_timerange(0, 0, 24, 0, dr);
  }

  void specific_month_date() {
    daterange* dr(_creator.new_specific_month_date(11, 20, 0, 10));
    _creator.new_timerange(0, 0, 24, 0, dr);
  }

  void generic_month_date() {
    daterange* dr(_creator.new_generic_month_date(20, 10));
    _creator.new_timerange(0, 0, 24, 0, dr);
  }

  void offset_weekday_of_specific_month() {
    daterange* dr(
        _creator.new_offset_weekday_of_specific_month(11, 2, 3, 0, 2, 2));
    _creator.new_timerange(0, 0, 24, 0, dr);
  }

  //
  // CURRENT TIME
  //

  void before_timeperiod() {
    _now = strtotimet("2016-12-15 12:00:00");
    set_time(_now);
  }

  void before_new_year() {
    _now = strtotimet("2016-12-25 12:00:00");
    set_time(_now);
  }

  void at_new_year() {
    _now = strtotimet("2017-01-01 00:00:00");
    set_time(_now);
  }

  void after_new_year() {
    _now = strtotimet("2017-01-02 12:00:00");
    set_time(_now);
  }

  void after_timeperiod() {
    _now = strtotimet("2017-01-15 12:00:00");
    set_time(_now);
  }

 protected:
  time_t _computed;
  timeperiod_creator _creator;
  time_t _now;
};

//
// RESULTS WILL BE MOSTLY THE SAME FOR ALL DATE RANGES
//
// So instead of writing all Given/When/Then, here are the common cases.
// A notable exception is the offset weekdays of generic months that
// cannot span on multiple months.
//
// Given a timeperiod covering a year change
// And we are before the timeperiod
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the timeperiod
//
// Given a timeperiod covering a year change
// And we are in the timeperiod before the new year
// When get_next_valid_time() is called
// Then the next valid time is now
//
// Given a timeperiod covering a year change
// And we are at the year change
// When get_next_valid_time() is called
// Then the next valid time is now
//
// Given a timeperiod covering a year change
// And we are in the timeperiod after the new year
// When get_next_valid_time() is called
// Then the next valid time is now
//
// Given a timeperiod covering a year change
// And we are after the timeperiod
// When get_next_valid_time() is called
// Then the next valid time is the beginning of the timeperiod in the new year

//
// CALENDAR DATE TESTS
//

TEST_F(GetNextValidTimeBetweenTwoYears, CalendarDateBeforeTimeperiod) {
  calendar_date();
  before_timeperiod();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-12-20 00:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears, CalendarDateBeforeNewYear) {
  calendar_date();
  before_new_year();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-12-25 12:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears, CalendarDateAtNewYear) {
  calendar_date();
  at_new_year();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-01-01 00:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears, CalendarDateAfterNewYear) {
  calendar_date();
  after_new_year();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-01-02 12:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears, CalendarDateAfterTimeperiod) {
  calendar_date();
  after_timeperiod();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-12-20 00:00:00"));
}

//
// SPECIFIC MONTH DATE TESTS
//

TEST_F(GetNextValidTimeBetweenTwoYears, SpecificMonthDateBeforeTimeperiod) {
  specific_month_date();
  before_timeperiod();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-12-20 00:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears, SpecificMonthDateBeforeNewYear) {
  specific_month_date();
  before_new_year();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-12-25 12:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears, SpecificMonthDateAtNewYear) {
  specific_month_date();
  at_new_year();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-01-01 00:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears, SpecificMonthDateAfterNewYear) {
  specific_month_date();
  after_new_year();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-01-02 12:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears, SpecificMonthDateAfterTimeperiod) {
  specific_month_date();
  after_timeperiod();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-12-20 00:00:00"));
}

//
// GENERIC MONTH DATE TESTS
//

TEST_F(GetNextValidTimeBetweenTwoYears, GenericMonthDateBeforeTimeperiod) {
  generic_month_date();
  before_timeperiod();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-12-20 00:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears, GenericMonthDateBeforeNewYear) {
  generic_month_date();
  before_new_year();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-12-25 12:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears, GenericMonthDateAtNewYear) {
  generic_month_date();
  at_new_year();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-01-01 00:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears, GenericMonthDateAfterNewYear) {
  generic_month_date();
  after_new_year();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-01-02 12:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears, GenericMonthDateAfterTimeperiod) {
  generic_month_date();
  after_timeperiod();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-01-20 00:00:00"));
}

//
// OFFSET WEEKDAY OF SPECIFIC MONTH TESTS
//

TEST_F(GetNextValidTimeBetweenTwoYears,
       OffsetWeekdayOfSpecificMonthBeforeTimeperiod) {
  offset_weekday_of_specific_month();
  before_timeperiod();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-12-20 00:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears,
       OffsetWeekdayOfSpecificMonthBeforeNewYear) {
  offset_weekday_of_specific_month();
  before_new_year();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2016-12-25 12:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears, OffsetWeekdayOfSpecificMonthAtNewYear) {
  offset_weekday_of_specific_month();
  at_new_year();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-01-01 00:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears,
       OffsetWeekdayOfSpecificMonthAfterNewYear) {
  offset_weekday_of_specific_month();
  after_new_year();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-01-02 12:00:00"));
}

TEST_F(GetNextValidTimeBetweenTwoYears,
       OffsetWeekdayOfSpecificMonthAfterTimeperiod) {
  offset_weekday_of_specific_month();
  after_timeperiod();
  get_next_valid_time(_now, &_computed, _creator.get_timeperiods());
  ASSERT_EQ(_computed, strtotimet("2017-12-19 00:00:00"));
}

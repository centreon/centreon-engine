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

class GetNextValidTimePrecedenceTest : public testing::Test {
 public:
  void SetUp() override {
    clib::load();
    com::centreon::logging::engine::load();
    // All dateranges are based on the same day : 2016-11-07.
    configuration::applier::state::load();
    _creator.new_timeperiod();
    _now = strtotimet("2016-11-07 00:00:00");
    set_time(_now);
  }

  void TearDown() override {
    configuration::applier::state::unload();
    com::centreon::logging::engine::unload();
    clib::unload();
  }

  // 2016-11-07 06:00-07:00
  void calendar_date_and_lower() {
    daterange* dr(_creator.new_calendar_date(2016, 10, 7, 2016, 10, 7));
    _creator.new_timerange(6, 0, 7, 0, dr);
    specific_month_date_and_lower();
    configuration::applier::state::unload();
  }

  // November 7 05:00-06:00
  void specific_month_date_and_lower() {
    daterange* dr(_creator.new_specific_month_date(10, 7, 10, 7));
    _creator.new_timerange(5, 0, 6, 0, dr);
    generic_month_date_and_lower();
  }

  // day 7 04:00-05:00
  void generic_month_date_and_lower() {
    daterange* dr(_creator.new_generic_month_date(7, 7));
    _creator.new_timerange(4, 0, 5, 0, dr);
    offset_weekday_of_specific_month_and_lower();
  }

  // monday 1 november 03:00-04:00
  void offset_weekday_of_specific_month_and_lower() {
    daterange* dr(
        _creator.new_offset_weekday_of_specific_month(10, 1, 1, 10, 1, 1));
    _creator.new_timerange(3, 0, 4, 0, dr);
    offset_weekday_of_generic_month_and_lower();
  }

  // monday 1 02:00-03:00
  // monday   01:00-02:00
  void offset_weekday_of_generic_month_and_lower() {
    daterange* dr(_creator.new_offset_weekday_of_generic_month(1, 1, 1, 1));
    _creator.new_timerange(2, 0, 3, 0, dr);
    _creator.new_timerange(1, 0, 2, 0, 1);
  }

 protected:
  timeperiod_creator _creator;
  time_t _now;
};

// Given a timeperiod configured with calendar dates
// And other overlapping date ranges of lower precedence
// When get_next_valid_time() is called
// Then the next valid time is on the calendar dates' timeranges for the
// overlapping days
TEST_F(GetNextValidTimePrecedenceTest, CalendarDatePrecedence) {
  calendar_date_and_lower();
  time_t computed((time_t)-1);
  get_next_valid_time(_now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-07 06:00:00"));
}

// Given a timeperiod configured with specific month dates
// And other overlapping date ranges of lower precedence
// When get_next_valid_time() is called
// Then the next valid time is on the specific month dates' timeranges for the
// overlapping days
TEST_F(GetNextValidTimePrecedenceTest, SpecificMonthDatePrecedence) {
  specific_month_date_and_lower();
  time_t computed((time_t)-1);
  get_next_valid_time(_now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-07 05:00:00"));
}

// Given a timeperiod configured with generic month dates
// And other overlapping date ranges of lower precedence
// When get_next_valid_time() is called
// Then the next valid time is on the generic month date' timeranges for the
// overlapping days
TEST_F(GetNextValidTimePrecedenceTest, GenericMonthDatePrecedence) {
  generic_month_date_and_lower();
  time_t computed((time_t)-1);
  get_next_valid_time(_now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-07 04:00:00"));
}

// Given a timeperiod configured with offset weekdays of specific month
// And other overlapping date ranges of lower precedence
// When get_next_valid_time() is called
// Then the next valid time is on the offset weekdays of specific month'
// timeranges for the overlapping days
TEST_F(GetNextValidTimePrecedenceTest, OffsetWeekdayOfSpecificMonthPrecedence) {
  offset_weekday_of_specific_month_and_lower();
  time_t computed((time_t)-1);
  get_next_valid_time(_now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-07 03:00:00"));
}

// Given a timeperiod configured with offset weekdays of generic month
// And other overlapping date ranges of lower precedence
// When get_next_valid_time() is called
// Then the next valid time is on the offset weekdays of generic month'
// timeranges for the overlapping days
TEST_F(GetNextValidTimePrecedenceTest, OffsetWeekdayOfGenericMonthPrecedence) {
  offset_weekday_of_generic_month_and_lower();
  time_t computed((time_t)-1);
  get_next_valid_time(_now, &computed, _creator.get_timeperiods());
  ASSERT_EQ(computed, strtotimet("2016-11-07 02:00:00"));
}
